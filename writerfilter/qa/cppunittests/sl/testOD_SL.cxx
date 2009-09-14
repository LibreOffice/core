/*  Copyright 2005 Sun Microsystems, Inc. */

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
