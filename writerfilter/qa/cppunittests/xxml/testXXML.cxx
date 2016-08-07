/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



#include <testshl/simpleheader.hxx>
#include <odiapi/xxml/XXmlReader.hxx>
#include <osl/time.h>


using namespace writerfilter;

class Node
{
public:
    QName_t tag;
    Node *next;
    Node *prev;

    Node(QName_t tag) : prev(NULL), next(NULL), tag(tag) {
    };

    void append(Node &node)
    {
        this->next=&node;
        node.prev=this;
    }
};

class Table : public Node
{
public:
    Table(QName_t tag):Node(tag) {};
};

class Row : public Node
{
public:
    Table &parent;

    Row(QName_t tag, Table &parent) : Node(tag), parent(parent) {};
};

class Cell : public Node
{
public:
    Row &parent;

    Cell(QName_t tag, Row &parent) : Node(tag), parent(parent) {};
};


class MyHandler : public xxml::ContentHandler
{
public:
    int events;
    Table *currentTable;
    Row *currentRow;
    Cell *currentCell;

    virtual void startDocument()
    {
        currentTable=NULL;
        currentRow=NULL;
        currentCell=NULL;
        events=1;
    }
    virtual void endDocument()
    {
        events++;
    }
    virtual void startElement(QName_t name, QName_t attrName[], const xxml::Value *attrValue[], int attrs)
    {
        events++;
//      printf("<{%s}:%s>\n", QName::serializer().getNamespaceUri(name), QName::serializer().getLocalName(name));
        for(int i=0;i<attrs;i++)
        {
//          printf("@{%s}:%s=\"%s\"\n", QName::serializer().getNamespaceUri(attrName[i]), QName::serializer().getLocalName(attrName[i]), attrValue[i]->getOString().getStr());
            events++;
        }

        switch(name)
        {
        case NS_table::LN_table:
        case NS_ss11::LN_Table:
            currentTable=new Table(name);
            break;
        case NS_table::LN_table_row:
        case NS_ss11::LN_Row:
            if (currentRow==NULL)
                currentRow=new Row(name, *currentTable);
            else
                currentRow->append(*new Row(name, *currentTable));
            break;
        case NS_table::LN_table_cell:
        case NS_ss11::LN_Cell:
            if (currentCell==NULL)
                currentCell=new Cell(name, *currentRow);
            else
                currentCell->append(*new Cell(name, *currentRow));
            break;

        };

    }
    virtual void endElement(QName_t name)
    {
            //printf("</{%s}:%s>\n", QName::serializer().getNamespaceUri(name), QName::serializer().getLocalName(name));
            events++;
            switch(name)
            {
            case NS_table::LN_table:
            case NS_ss11::LN_Table:
                if (currentTable != NULL)
                {
                    currentRow->append(*currentTable);
                }
                currentRow=NULL;
                break;
            case NS_table::LN_table_row:
            case NS_ss11::LN_Row:
                if (currentRow != NULL)
                    currentCell->append(*currentRow);
                currentCell=NULL;
                break;
            case NS_table::LN_table_cell:
            case NS_ss11::LN_Cell:
                break;

            };
    }
    virtual void characters(const xxml::Value &value)
    {
        //printf("\"%s\"\n", value.getOString().getStr());
        events++;
    }

};

class TestXXML : public CppUnit::TestFixture
{
public:
    void test()
    {
        MyHandler handler;
        std::auto_ptr<xxml::XXmlReader> reader=xxml::XXmlReader::createXXmlReader(handler);
        TimeValue t1; osl_getSystemTime(&t1);

//      reader->read("test.xml");
//      reader->read("C:\\Documents and Settings\\fr156068\\My Documents\\odt\\testfile.xml");
        reader->read("C:\\Documents and Settings\\fr156068\\My Documents\\odt\\testfile\\content.xml");
        TimeValue t2; osl_getSystemTime(&t2);
        printf("Events=%i time=%is time/event=%0.10fs\n", handler.events, t2.Seconds-t1.Seconds, (double)(t2.Seconds-t1.Seconds)/(double)handler.events);
    }

    CPPUNIT_TEST_SUITE(TestXXML);
    CPPUNIT_TEST(test);


    CPPUNIT_TEST_SUITE_END();
};

//#####################################
// register test suites
CPPUNIT_TEST_SUITE_NAMED_REGISTRATION(TestXXML, "TestXXML");

NOADDITIONAL;
