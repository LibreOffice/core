/*************************************************************************
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 * 
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

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
//		printf("<{%s}:%s>\n", QName::serializer().getNamespaceUri(name), QName::serializer().getLocalName(name));
        for(int i=0;i<attrs;i++)
        {
//			printf("@{%s}:%s=\"%s\"\n", QName::serializer().getNamespaceUri(attrName[i]), QName::serializer().getLocalName(attrName[i]), attrValue[i]->getOString().getStr());
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
            currentRow->append(*currentTable);
            currentRow=NULL;
            break;
        case NS_table::LN_table_row:
        case NS_ss11::LN_Row:
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

//		reader->read("test.xml");
//		reader->read("C:\\Documents and Settings\\fr156068\\My Documents\\odt\\testfile.xml");
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
