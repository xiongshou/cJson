



# my_cJson

## 介绍

cJson01:双链表实现

cJson02:单链表实现

关于Json解析器的代码，大多使用左子树右兄弟表示Json树，其中兄弟之间的逻辑关系用双链表表示，可以支持删除、插入操作。emmmm我们可以稍微改改，兄弟之间用单链表表示！！！类似写算法题经常用到的链式前向星存图的操作

> 插入的话，先写头部插入吧，慢慢把指定节点插入加上。
>
> 删除节点：双链表通过一个前取节点+后继节点实现删除节点的操作，其实单链表也可以在O(1)的时间复杂度删除一个节点，单链表的删除节点操作，使用了一个取巧的办法，用下一个节点取代当前节点，来删除当前节点。

尽量使用指针以及深拷贝实现c语言中字符串相关函数功能，实现了由字符串转化为Json格式，由Json格式转化字符串的功能。简单地基于C语言的JSON解析器，包含了基本的节点创建、解析、封装、输出等等功能。

## GitHub地址：

https://github.com/xiongshou/cJSON

GitHub地址中单链表表示兄弟节点的代码，正在写。



## 关于Json介绍

在整体逻辑结构使用树来存储Json，使用左孩子右兄弟存储树。孩子节点由一个指针指向，兄弟节点使用双链表链接，可以快速增删改兄弟节点内容。

```cpp
// 兄弟节点用双链表
typedef struct cJSON
{
    struct cJSON *next, *prev;
    struct cJSON *child;
    /*array以及obect类型需要设立子节点*/
    int type;
    char *valueString;// 值
    int valueInt;
    double valueDouble;
    char *string; // 可能作为键
}cJSON;
// 兄弟节点用单链表表示
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

```

## 测试函数

```cpp
  // insert code here...
    cJSON *root = cJSON_CreateObject();
    cJSON *node1 = cJSON_CreateObject();
    cJSON *node2 = cJSON_CreateObject();
    cJSON *node3 = cJSON_CreateObject();
    cJSON *node4 = cJSON_CreateObject();
    // 添加兄弟节点：双链表中的节点
     cJSON_AddFalseToObject(node1, "node1_key1");
     cJSON_AddTrueToObject(node1, "node1_key2");
     cJSON_AddStringToObject(node2, "node2_key1", "node2_value1");
     cJSON_AddStringToObject(node2, "node2_key2", "node2_value2");
     cJSON_AddNumberToObject(node3, "node3_key1", 1000);
     cJSON_AddNumberToObject(node3, "node3_key2", 2000);
     cJSON_AddStringToObject(node4, "node4_key1", "node4_value1");
     cJSON_AddStringToObject(node4, "node4_key4", "node4_value2");

    // 添加子类节点，逻辑关系中的指针
     cJSON_AddItemToObject(root, "root_node1", node1);
     cJSON_AddItemToObject(root, "root_node2", node2);
     cJSON_AddItemToObject(node1, "node1_node3", node3);
     cJSON_AddItemToObject(node3, "node1_node3", node4);

     char *buf = cJSON_Print(root);
     printf("json:\n%s\n", buf);

     cJSON *parse_node = cJSON_Parse(buf);

     cJSON_Delete(root);
     cJSON_Delete(parse_node);
     free(buf);
    
```

## 运行结果

![image-20201206142633741](https://tva1.sinaimg.cn/large/0081Kckwly1gle4b5c8ntj310i0scgr3.jpg)

## 



## 解析函数（字符串解析json树）

### cJson_Parse函数

功能：解析字符串，返回一颗Json树

基本逻辑：先创建这颗节点，然后skip一下字符串（跳过空格、换行）

之后，执行parse_value(json树 ， 字符串)，把字符串放进json树节点中，并且返回一个字符串地址。

### parse_value()函数

功能：把value字符串放入item节点中

基本逻辑：先匹配一下这个字符串什么类型，先确定json树类型

false、 true、 null这些直接在json树节点确定一下就可以了

若是：string类型，调用parse_string()把这个字符串解析到树中。

若是：number类型，掉用parse_number()把这个数字解析到树中

若是：数组类型，调用parse_array()把这个数组解析到json树中

若是：json对象{},调用parse_object()把这个对象解析到json树中



### parse_string（）函数

功能：把字符串解析到json节点中 ：这个字符串是“XXX”这种形式

基本逻辑：使用俩个指针，找出字符串的长度，这个长度，不包括转义符号

然后，使用深拷贝，用字符指针取出这个字符串，

然后：Json树中的->valueString,指向这个字符串，Json树中的->type类型设置为c JSON_String

这里需要注意，字符串json节点中，保存值都是valueString，之后遇见json对象之后，string作为键，valuestring作为值，构成键值对

### parse_number()函数

功能：把表示数字的字符串转化为数字，放入json节点中。

基本逻辑：考虑一下正负数、科学计数法，然后把值存入节点中，存一个double类型，一个int类型

### parse_Array()函数

功能：处理一下：["Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"]，引号里面有可能是数字。

基本逻辑：找出引号里面的字符串，把字符串解析到一个新节点，要么是字符串、要么是数字，调用之前的函数：parse_value()函数处理。

数组中里面的字符串、数字 用双向链表以及“头插法”插入json树节点



### parse_Object()函数

功能：把表示键值对的字符串，放入json节点中。

基本逻辑：把{}里面到1键值对处理了，先申请一个孩子节点，每个孩子节点存一对“键：值”，然后使用头插法，处理下一下键值对





## 打印函数（把json树打印为字符串）

### print_value（）

功能：把json节点转化为字符串，

基本逻辑：判断这个树什么类型，使用print_value打印

其他功能函数逻辑和parse_value()一样



## 删除函数

删除json节点的第i个子节点



## 添加函数

### cJSON_AddItemToObject（）

