//
//  main.c
//  cJson_add
//
//  Created by 凶手 on 2020/12/7.
//  Copyright © 2020 凶手. All rights reserved.
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <limits.h>
#include <float.h>
#include <ctype.h>

#define cJson_h
#define cJSON_False       0
#define cJSON_True        1
#define cJSON_NULL        2
#define cJSON_Number      3
#define cJSON_String      4
#define cJSON_Array       5
#define cJSON_Object      6


/*
功能：创建一个string值为name的cJSON_False节点，并添加到object
*/
#define cJSON_AddFalseToObject(object, name) \
    cJSON_AddItemToObject(object, name, cJSON_CreateFalse())

/*创建一个string值为name的cJSON_True节点,并添加到object节点*/
#define cJSON_AddTrueToObject(object,name) \
    cJSON_AddItemToObject(object, name, cJSON_CreateTrue())

/*创建一个string值为name的cJSON_Bool/False节点,并添加到object节点*/
#define cJSON_AddBoolToObject(object,name, b) \
    cJSON_AddItemToObject(object, name, cJSON_CreateBool(b))

/*创建一个string值为name的cJSON_NULL节点,并添加到object节点*/
#define cJSON_AddNULLToObject(object,name) \
    cJSON_AddItemToObject(object, name, cJSON_CreateNull())

/*创建一个string值为name的cJSON_Number节点,并添加到object节点*/
#define cJSON_AddNumberToObject(object,name, number) \
    cJSON_AddItemToObject(object, name, cJSON_CreateNumber(number))

/*创建一个string值为name的cJSON_String节点,并添加到object节点*/
#define cJSON_AddStringToObject(object,name, s) \
    cJSON_AddItemToObject(object, name, cJSON_CreateString(s))

#define cJSON_SetIntValue(object, val)\
    ((object)?((object)->valueInt=(object)->valueDouble=(val)):(val))

#define cJSON_SetNumberValue(object, val)\
    ((object)?((object)->valueInt=(object)->valueDouble=(val)):(val))

typedef struct cJSON
{
    struct cJSON *next;
    struct cJSON *child;
    int type;
    int valueInt;
    double valueDouble;
    char *string;
    char *valueString;
}cJSON;

/*常用解析函数*/

cJSON *cJSON_Parse(const char *value);

char *cJSON_Print(cJSON *item);

void cJSON_Delete(cJSON *c);

cJSON *cJSON_CreateObject(void);

cJSON *cJSON_CreateString(const char *string);

cJSON *cJSON_CreateNumber(double num);

cJSON *cJSON_CreateArray(void);

cJSON *cJSON_CreateBool(int b);

cJSON *cJSON_CreateTrue(void);

cJSON *cJSON_CreateFalse(void);

cJSON *cJSON_CreateNull(void);

void cJSON_AddItemToObject(cJSON *object, const char *string, cJSON *item);

void cJSON_AddItemToArray(cJSON *array, cJSON *item);
cJSON *cJSON_ParseWithOpts(const char *value, const char **return_parse_end, int require_null_terminated);
cJSON *cJSON_New_Item();
const char *skip(const char *in);
char *cJSON_GetEndPosition(void);
const char *parse_value(cJSON *item, const char *value);
const char* parse_string(cJSON *item, const char *str);
const char* parse_number(cJSON *item, const char *num);
const char* parse_Array(cJSON *item, const char *value);
const char* parse_Object(cJSON *item, const char *value);
char *cJSON_strdup(const char *str);
char *print_number(cJSON *item);
char *print_string(cJSON *item,int name);
//["OSCAR",123,XXX]
char *print_array(cJSON *item, int depth, int fmt);
char *print_object(cJSON *item, int depth, int fmt);
char *print_value(cJSON *item,int depth, int fmt);
cJSON *cJSON_DetachItemFromArray(cJSON *array, int which);
void cJSON_DeleteItemFromArray(cJSON *array, int which);
int cJSON_strcasecmp(const char *s1, const char *s2); //strcmp()
cJSON *cJSON_DetachItemFromObject(cJSON *object, const char *string);
void cJSON_DeleteItemFromObject(cJSON *object, int which);
void cJSON_InsertItemInArray(cJSON *array, int which, cJSON *newItem);
void cJSON_ReplaceItemInArray(cJSON *array, int which, cJSON *newItem);
void cJSON_ReplaceItemInObject(cJSON *object, const char *string, cJSON *newItem);


// 创建函数-------------//
static void *(*cJSON_malloc)(size_t sz) = malloc;
static void(*cJSON_free)(void *ptr) = free;

// 解析到出错的位置
static const char *ep;

cJSON *cJSON_New_Item()
{
    cJSON * node = (cJSON *)cJSON_malloc(sizeof(cJSON));
    if (node) { memset(node, 0, sizeof(cJSON)); }
    return node;
}

cJSON *cJSON_CreateNull(void)
{
    cJSON *item = cJSON_New_Item();
    if (item) { item->type = cJSON_NULL; }
    return item;
}

cJSON *cJSON_CreateTrue(void)
{
    cJSON *item = cJSON_New_Item();
    if (item)item->type = cJSON_True;
    return item;
}

cJSON *cJSON_CreateFalse(void)
{
    cJSON *item = cJSON_New_Item();
    if (item)item->type = cJSON_False;
    return item;
}

cJSON *cJSON_CreateBool(int b)
{
    cJSON *item = cJSON_New_Item();
    if (item)item->type = b ? cJSON_True : cJSON_False;
    return item;
}

cJSON *cJSON_CreateNumber(double num)
{
    cJSON *item = cJSON_New_Item();
    if (item)
    {
        item->type = cJSON_Number;
        item->valueDouble = num;
        item->valueInt = (int)num;
    }
    return item;
}

cJSON *cJSON_CreateString(const char *string)
{
    cJSON *item = cJSON_New_Item();
    if (item)
    {
        item->type = cJSON_String;
        item->valueString = cJSON_strdup(string);
    }
    return item;
}

cJSON *cJSON_CreateArray(void)
{
    cJSON *item = cJSON_New_Item();
    if (item)item->type = cJSON_Array;
    return item;
}

cJSON *cJSON_CreateObject()
{
    cJSON *item = cJSON_New_Item();
    if (item)item->type = cJSON_Object;
    return item;
}
/* 解析函数*/
const char *skip(const char *in)
{
    while (in && *in && ((unsigned char)*in<=32))
    {
        in++;
    }
    return in;
}
// 都是返回字符串的结尾
const char* parse_string(cJSON *item, const char *str)
{
    // ptr 尾指针，不断后移保证len++
    const char *ptr = str + 1;
    char *ptr2;
    char *out;

    int len = 0;
    // 不是一个字符串：字符串要双引号起来
    if (*str != '\"')
    {
        ep = str;
        return NULL;
    }

    while (*ptr != '\"' && *ptr && ++len)
    {
        if (*ptr++ == '\\') { ptr++; }
    }

    out = (char *)cJSON_malloc(len + 1);
    if (!out) return NULL;

    ptr = str + 1;
    ptr2 = out;

    // 深拷贝 + 处理转义符号
    while (*ptr != '\"' && *ptr)
    {
        if (*ptr != '\\') { *ptr2++ = *ptr++; }
        else
        {
            ptr++;
            switch (*ptr)
            {
            case 'b':*ptr2++ = '\b'; break;
            case 'f':*ptr2++ = '\f'; break;
            case 'n':*ptr2++ = '\n'; break;
            case 'r':*ptr2++ = '\r'; break;
            case 't':*ptr2++ = '\t'; break;
            default:
                *ptr2++ = *ptr; break;
            }
            ptr++;
        }
    }
    *ptr2 = 0;

    if (*ptr == '\"') { ptr++; }
    
    item->valueString = out;
    item->type = cJSON_String;

    return ptr;
}

const char* parse_number(cJSON *item, const char *num)
{
    double n = 0;
    int sign = 1;
    int signSubScale = 1;
    int scale = 0; // 小数 多少位
    int subscale = 0; // 科学计数法 多少位

    if (*num == '-')
    {
        sign = -1;
        num++;
    }

    if (*num == '0') { num++; }

    if (*num >= '0' && *num <= '9')
    {
        //1920
        do
        {
            n = (n*10.0) + (*num++ - '0');
        } while (*num >= '0' && *num <= '9');
    }

    if (*num == '.' && num[1] >= '0' && num[1] <= '9')
    {
        //12345.6789
        num++;
        do
        {
            n = (n*10.0) + (*num++ - '0');
            scale--;
        } while (*num >= '0' && *num <= '9');
    }

    if (*num == 'e' || *num == 'E')
    {
        num++;
        if (*num == '+') num++;
        else if (*num == '-')
        {
            signSubScale = -1;
            num++;
        }
        do
        {
            subscale = (subscale*10.0) + (*num++ - '0');
        } while (*num >= '0' && *num <= '9');
    }

    //number = +/- number.fraction * 10 ^(+/-)exp
    n = sign * n * pow(10.0, (scale + signSubScale*subscale));
    item->valueDouble = n;
    item->valueInt = (int)n;
    item->type = cJSON_Number;
    return num;
}

// 数组这里可能是字符串、可能是数字
const char* parse_Array(cJSON *item, const char *value)
{
    //["Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"]
    //   [],
    cJSON *child;
    // 不是数组 【】
    if (*value != '[')
    {
        ep = value;
        return NULL;
    }

    item->type = cJSON_Array;
    value = skip(value + 1);
    if (*value == ']') return value + 1;

    item->child = child = cJSON_New_Item();
    if (!item->child) return NULL;

    value = skip(parse_value(child, skip(value)));
    if (!value) return NULL;
    // 循环处理 兄弟数组节点
    while (*value == ',')
    {
        cJSON *new_item;
        if (!(new_item = cJSON_New_Item())) return NULL;
        // 插头法，插入节点 item -> child  item ->new_item->cild
        child->next = new_item;
        // new_item->prev = child;
        // new_item->next = child;
        // item->next=new_item;
        child = new_item;
        value = skip(parse_value(child, skip(value + 1)));
        if (!value) return NULL;
    }

    if (*value == ']') return value + 1;
    ep = value;

    return NULL;
}

// 处理对象 大括号里面包括的东西
const char* parse_Object(cJSON *item, const char *value)
{
    cJSON *child;
    // 不合法的情况
    if (*value != '{')
    {
        ep = value;
        return NULL;
    }

    item->type = cJSON_Object;
    value = skip(value + 1);
    // 大括号里面没有东西，空对象
    if (*value == '}') return value + 1;

    item->child = child = cJSON_New_Item();
    if (!item->child) return NULL;

    value = skip(parse_string(child, skip(value)));
    if (!value) return NULL;

    child->string = child->valueString;
    child->valueString = NULL;
    if (*value != ':')
    {
        ep = value;
        return NULL;
    }

    value = skip(parse_value(child, skip(value + 1)));
    if (!value) return NULL;

    while (*value == ',')
    {
        cJSON *new_item;
        if (!(new_item = cJSON_New_Item())) return NULL;
        
        // item ->chile  item->new_item ->chil
        child->next = new_item;
        // new_item->prev = child;
        child = new_item;
        

        value = skip(parse_string(child, skip(value + 1)));
        if (!value) return NULL;
        // string : valueString  = 键值对，前者是键
        // 前者的键=后者匹配的valustring
        // 前者的键对应的值为 应该为空
        child->string = child->valueString;
        child->valueString = NULL;

        // 正常情况下 键值后是： ， 若不是，设置一下ep
        if (*value != ':')
        {
            ep = value;
            return NULL;
        }

        value = skip(parse_value(child, skip(value + 1)));
        if (!value) return NULL;
    }

    if (*value == '}') return value + 1;

    ep = value;
    return NULL;


}

// 把解析的添加到item，在value解析了节点，并且添加item节点，
// 返回那个字符串的指针，把双引号中间的那一段深拷贝一下
const char *parse_value(cJSON *item, const char *value)
{
    if (!value) return NULL;
    // 相同返回0，改一下item类型
    if (!strncmp(value, "false", 5))
    {
        item->type = cJSON_False;
        return value + 5;
    }

    if (!strncmp(value, "true", 4))
    {
        item->type = cJSON_True;
        item->valueInt = 1;
        return value + 4;
    }

    if (!strncmp(value, "null", 4))
    {
        item->type = cJSON_NULL;
        return value + 4;
    }
    // 把字符串、数字、数组 添加到item，然后返回一个“处理完value的下一个指针
    // 遇到 “ 就是处理字符串
    if (*value == '\"') {
        return parse_string(item, value);
    }
    // 遇到数字（正数或者负数）
    if (*value == '-' || (*value>='0' && *value <= '9')) { return parse_number(item, value); }
    // 数组
    if (*value == '[') { return parse_Array(item, value); }
    // 处理object
    if (*value == '{') { return parse_Object(item, value); }

    ep = value;
    return NULL;
}

// 字符串解析成cjson树的函数
cJSON *cJSON_Parse(const char *value)
{
    return cJSON_ParseWithOpts(value, 0, 0);
}
// 2:给出出错的位置， 3： 需不需要用一次就删除掉
cJSON *cJSON_ParseWithOpts(const char *value, const char **return_parse_end, int require_null_terminated)
{
    const char *end = NULL;
    cJSON *c = cJSON_New_Item();
    ep = NULL;
    if (!c) return NULL;
    // 解析一下类型
    end = parse_value(c, skip(value));
    if (!end)
    {
        cJSON_Delete(c);
        return NULL;
    }
    // 判断解析字符串有没有错，这一需求
    if (require_null_terminated)
    {
        end = skip(end);
        if (*end)
        {
            cJSON_Delete(c);
            ep = end;
            return NULL;
        }
    }

    if (return_parse_end)
    {
        *return_parse_end = end;
    }

    return c;

}

// 打印函数
char *cJSON_strdup(const char *str)
{
    char *copy = NULL;
    size_t len;

    len = strlen(str);
    copy =(char *) cJSON_malloc(len + 1);
    if (!copy) return NULL;
    memcpy(copy, str, len + 1);

    return copy;
}

char *print_number(cJSON *item)
{
    char *str = NULL;
    double d = item->valueDouble;
    if (d == 0)
    {
        str = (char *)cJSON_malloc(2);
        if (str) strcpy(str, "0");
    }


    else if ((fabs((double)(item->valueInt) - d) <= DBL_EPSILON) && d <= INT_MAX && d >= INT_MIN)
    {
        str = (char *)cJSON_malloc(21);
        if (str) sprintf(str, "%d", item->valueInt);
    }
    else
    {
        str = (char *)cJSON_malloc(64);
        if (str)
        {
            //1234xxxx.0
            if ((fabs(floor(d) - d) <= DBL_EPSILON) && fabs(d) < 1.0e60) { sprintf(str, "%.0f", d); }
            else if (fabs(d) < 1.0e-6 || fabs(1.0e9)) { sprintf(str, "%e", d); }
            else { sprintf(str, "%f", d); }
        }
    }

    return str;

}

char *print_string(cJSON *item, int isName)
{
    char *str = NULL;
    if(isName){ str = item->string; }
    else{ str = item->valueString; }
    char *ptr, *ptr2, *out;
    unsigned char token;
    int flag = 0, len = 0;

    for (ptr = str; *ptr; ptr++)
    {
        flag |= ((*ptr > 0 && *ptr < 32) || (*ptr == '\"') || (*ptr == '\\')) ? 1 : 0;
    }
    // 字符串合法
    if (!flag)
    {
        len = ptr - str;
        out = (char *)cJSON_malloc(len + 2 + 1); // 外加申请2个双引号 + \0
        if (!out) return NULL;
        // 通过指针修改字符串
        ptr2 = out;
        *ptr2++ = '\"';
        strcpy(ptr2, str);
        //"abcde"\0
        ptr2[len] = '\"';
        ptr2[len + 1] = '\0';

        return out;
    }

    if (!str)
    {
        out = (char *)cJSON_malloc(3);
        if (!out) return NULL;
        strcpy(out, "\"\"");
        return out;
    }
    // 字符串不合法
    ptr = str;
    while ((token = *ptr) && ++len)
    {
        if (strchr("\"\\\b\f\n\r\t", token)) { len++; }
        else if (token < 32) { len += 5; ptr++; }
    }

    out = (char *)cJSON_malloc(len + 3);
    if (!out) return NULL;

    ptr2 = out;
    ptr = str;
    *ptr2++ = '\"';
    while (*ptr)
    {
        if ((unsigned char)*ptr > 31 && *ptr != '\"' && *ptr != '\\')
        {
            *ptr2++ = *ptr++;
        }
        else
        {
            *ptr2++ = '\\';
            switch (token = *ptr)
            {
            case '\\':*ptr2++ = '\\'; break;
            case '\"':*ptr2++ = '\"'; break;
            case '\b':*ptr2++ = '\b'; break;
            case '\f':*ptr2++ = '\f'; break;
            case '\n':*ptr2++ = '\n'; break;
            case '\r':*ptr2++ = '\r'; break;
            case '\t':*ptr2++ = '\t'; break;
            default:
                sprintf(str, "u%04x", token);
                ptr2++;
                break;
            }
        }
    }
    *ptr2++ = '\"';
    *ptr2++ = '\0';
    return out;
}

char *print_array(cJSON *item, int depth, int fmt)
{
    char **entries; //char *entries[];
    char *out = NULL, *ptr, *ret;
    int len = 5, templen = 0, isFail = 0, i = 0; //"[]"\0

    cJSON *child = item->child;
    // 子节点的数量
    int numEntries = 0;
    while (child)
    {
        numEntries++;
        child = child->next;
    }

    if (!numEntries)
    {
        out = (char *)cJSON_malloc(3);
        if (out) strcpy(out, "[]");
        return out;
    }

    entries = (char **)cJSON_malloc(numEntries * sizeof(char *));
    if (!entries) return NULL;
    memset(entries, 0, numEntries * sizeof(char *));

    child = item->child;
    while (child)
    {
        ret = print_value(child, depth + 1, fmt);
        entries[i++] = ret;
        if (ret) { len += strlen(ret) + 2 + (fmt ? 1 : 0); }  // 需要格式化，多加一个\t
        else { isFail = 1; } // isFail 出错

        child = child->next;
    }

    if (!isFail) { out = (char *)cJSON_malloc(len); }
    if (!out) { isFail = 1; }

    if (isFail)
    {
        for (int i = 0; i < numEntries; i++)
        {
            if (entries[i])
            {
                cJSON_free(entries[i]);
            }
        }
        cJSON_free(entries);

        return NULL;
    }

    *out = '[';
    ptr = out + 1;
    *ptr = '\0';
    for (i = 0; i < numEntries; i++)
    {
        templen = strlen(entries[i]);
        memcpy(ptr, entries[i], templen);
        ptr += templen;

        if (i != numEntries - 1)
        {
            *ptr++ = ',';
            if (fmt) { *ptr++ = ' '; }
            *ptr = '\0';
        }

        cJSON_free(entries[i]);
    }
    cJSON_free(entries);

    *ptr++ = ']';
    *ptr++ = '\0';

    return out;
}

void cJSON_Delete(cJSON *c)
{
    cJSON *next;
    while (c)
    {
        next = c->next;
        if (c->child) { cJSON_Delete(c->child); }
        if (c->valueString) { cJSON_free(c->valueString); }
        if (c->string) { cJSON_free(c->string); }
        cJSON_free(c);
        c = next;
    }
}

char *print_object(cJSON *item, int depth, int fmt)
{
    char **entries = NULL, **names = NULL;
    char *out = NULL, *ptr, *ret, *str;
    int len = 7, i = 0, templen = 0;

    cJSON *child = item->child;
    int numEntries = 0, isFail = 0;

    while (child)
    {
        numEntries++;
        child = child->next;
    }

    if (!numEntries)
    {
        out = (char *)cJSON_malloc(fmt ? depth + 4 : 3);
        if (!out) return NULL;

        ptr = out;
        *ptr++ = '{';
        if (fmt)
        {
            *ptr++ = '\n';
            for (i = 0; i < depth - 1; i++)
            {
                *ptr++ = '\t';
            }
        }

        *ptr++ = '}';
        *ptr++ = '\0';

        return out;

    }

    entries = (char **)cJSON_malloc(numEntries * sizeof(char *));
    if (!entries) return NULL;

    names = (char **)cJSON_malloc(numEntries * sizeof(char *));
    if (!names) { cJSON_free(entries); return NULL; }

    memset(entries, 0, numEntries * sizeof(char *));
    memset(names, 0, numEntries * sizeof(char *));

    child = item->child;
    depth++;
    if (fmt) { len += depth; }

    while (child)
    {
        names[i] = str = print_string(child, 1);
        entries[i++] = ret = print_value(child, depth, fmt);
        if (str && ret) { len += strlen(ret) + strlen(str) + 2 + (fmt ? 2 + depth : 0); }
        else { isFail = 1; }

        child = child->next;
    }

    if (!isFail) { out = (char *)cJSON_malloc(len); }
    if (!out) { isFail = 1; }

    if (isFail)
    {
        for (i = 0; i < numEntries; i++)
        {
            if (names[i]) { cJSON_free(names[i]); }
            if (entries[i]) { cJSON_free(entries[i]); }
        }
        cJSON_free(names);
        cJSON_free(entries);

        return NULL;
    }

    *out = '{';
    ptr = out + 1;
    if (fmt) { *ptr++ = '\n'; }
    *ptr = '\0';

    for (i = 0; i < numEntries; i++)
    {
        if (fmt)
        {
            for (int j = 0; j < depth; j++) { *ptr++ = '\t'; }
        }
        templen = strlen(names[i]);
        memcpy(ptr, names[i], templen);
        ptr += templen;
        *ptr++ = ':';
        if (fmt) { *ptr++ = '\t'; }

        strcpy(ptr, entries[i]);
        ptr += strlen(entries[i]);

        if (i != numEntries - 1) { *ptr++ = ','; }
        if (fmt) { *ptr++ = '\n'; }
        *ptr = '\0';

        cJSON_free(names[i]);
        cJSON_free(entries[i]);
    }
    cJSON_free(names);
    cJSON_free(entries);

    if (fmt)
    {
        for (i = 0; i < depth - 1; i++)
        {
            *ptr++ = '\t';
        }
    }

    *ptr++ = '}';
    *ptr++ = '\0';

    return out;
}

// cjson树， 树中的深度，format：是不是带格式的输出
char *print_value(cJSON *item, int depth, int fmt)
{
    char *out = NULL;
    if (!item) return NULL;

    switch ((item->type) & 255)
    {
    case cJSON_NULL: out = cJSON_strdup("null"); break;
    case cJSON_False: out = cJSON_strdup("false"); break;
    case cJSON_True: out = cJSON_strdup("true"); break;
    case cJSON_Number: out = print_number(item); break;
    case cJSON_String: out = print_string(item, 0); break;
    case cJSON_Array: out = print_array(item, depth, fmt); break;
    case cJSON_Object: out = print_object(item, depth, fmt); break;
    }

    return out;
}

// json树解析成字符串的函数
char *cJSON_Print(cJSON *item)
{
    return print_value(item, 0, 1);
}

// 添加节点函数
void cJSON_AddItemToObject(cJSON *object, const char *string, cJSON *item)
{
    if (!item) { return; }
    
    if (!item->string) { cJSON_free(item->string); }
    item->string = cJSON_strdup(string);
    cJSON_AddItemToArray(object, item);
}
void suffix_obect(cJSON *prev, cJSON *item)
{
    prev->next = item;
    //item->prev = prev;
}
void cJSON_AddItemToArray(cJSON *array, cJSON *item)
{
    cJSON *c = array->child;
    if (!item) return;

    if (!c)
    {
        array->child = item;
    }
    else
    {
        while (c && c->next)
        {
            c = c->next;
        }
        suffix_obect(c, item);
    }
}
int main(int argc, const char * argv[])
{

    cJSON *root = cJSON_CreateObject();
    cJSON *node1 = cJSON_CreateObject();
    cJSON *node2 = cJSON_CreateObject();
    cJSON *node3 = cJSON_CreateObject();
    cJSON *node4 = cJSON_CreateObject();
    // 添加双链表中的对象
     cJSON_AddFalseToObject(node1, "node1_key1");
     cJSON_AddTrueToObject(node1, "node1_key2");
     cJSON_AddStringToObject(node2, "node2_key1", "node2_value1");
     cJSON_AddStringToObject(node2, "node2_key2", "node2_value2");
     cJSON_AddNumberToObject(node3, "node3_key1", 1000);
     cJSON_AddNumberToObject(node3, "node3_key2", 2000);
     cJSON_AddStringToObject(node4, "node4_key1", "node4_value1");
     cJSON_AddStringToObject(node4, "node4_key4", "node4_value2");

    // 向根节点添加子类对象
     cJSON_AddItemToObject(root, "root_node1", node1);
     cJSON_AddItemToObject(root, "root_node2", node2);
     cJSON_AddItemToObject(node1, "node1_node3", node3);
     cJSON_AddItemToObject(node3, "node1_node3", node4);

     char *buf = cJSON_Print(root);
     printf("json:\n%s\n", buf);

    
    return 0;
}
