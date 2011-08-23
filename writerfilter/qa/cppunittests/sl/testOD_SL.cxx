/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
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
#include <odiapi/props/Properties.hxx>
#include <odiapi/sl/od_sl.hxx>

#include "../odiapi/FileLoggerImpl.hxx"
#include "../odiapi/ExternalViewLogger.hxx"

#include <osl/file.hxx>
#include <osl/thread.hxx>

using namespace writerfilter;
using namespace osl;
using namespace rtl;
using namespace util;

class MyHandler : public xxml::ContentHandler
{
public:
    virtual void startDocument()
    {
    }
    virtual void endDocument()
    {
    }
    virtual void startElement(QName_t name, QName_t attrName[], const xxml::Value *attrValue[], int attrs)
    {
        printf("<{%s}:%s>\n", QName::serializer().getNamespaceUri(name), QName::serializer().getLocalName(name));
        for(int i=0;i<attrs;i++)
        {
            printf("@{%s}:%s=\"%s\"\n", QName::serializer().getNamespaceUri(attrName[i]), QName::serializer().getLocalName(attrName[i]), attrValue[i]->getOString().getStr());
        }
    }
    virtual void endElement(QName_t name)
    {
        printf("</{%s}:%s>\n", QName::serializer().getNamespaceUri(name), QName::serializer().getLocalName(name));
    }
    virtual void characters(const xxml::Value &value)
    {
        printf("\"%s\"\n", value.getOString().getStr());
    }

};

OString getTempFileName(const OUString& fileName)
{
  OUString ousTmpUrl;
  FileBase::getTempDirURL(ousTmpUrl);
  if (!ousTmpUrl.endsWithIgnoreAsciiCaseAsciiL("/", 1))
    ousTmpUrl += OUString::createFromAscii("/");
  ousTmpUrl += fileName;

  OUString sysTmpPath;
  FileBase::getSystemPathFromFileURL(ousTmpUrl, sysTmpPath);

  return OUStringToOString(sysTmpPath, osl_getThreadTextEncoding());
}

class TestXXML : public CppUnit::TestFixture
{
public:
    void test()
    {
        odiapi::props::PropertyPool::Pointer_t propertyPool=odiapi::props::createPropertyPool();
        std::auto_ptr<ODSLHandler> handler=ODSLHandler::createODSLHandler(propertyPool);
        std::auto_ptr<xxml::XXmlReader> reader=xxml::XXmlReader::createXXmlReader(*handler);
        reader->read("helloworld.odt.flat.xml");

        OString tmpFileName = getTempFileName(OUString::createFromAscii("dumpSlPool_int.dot"));
        printf("Pool dump: %s\n", tmpFileName.getStr());
        FileLoggerImpl fl(tmpFileName.getStr());
        propertyPool->dump(&fl);

        OString tmpFileName2 = getTempFileName(OUString::createFromAscii("dumpSlPool_ext.dot"));
        printf("Pool dump: %s\n", tmpFileName2.getStr());
        ExternalViewLoggerImpl evl(tmpFileName2.getStr());
        propertyPool->dump(&evl);
    }

    CPPUNIT_TEST_SUITE(TestXXML);
    CPPUNIT_TEST(test);


    CPPUNIT_TEST_SUITE_END();
};

//#####################################
// register test suites
CPPUNIT_TEST_SUITE_NAMED_REGISTRATION(TestXXML, "TestXXML");

NOADDITIONAL;

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
