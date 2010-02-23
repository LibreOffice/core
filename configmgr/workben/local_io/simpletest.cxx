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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_configmgr.hxx"

#include <memory>
#include <vector>
#include <stack>
#include<osl/file.hxx>

#include <rtl/ustring.hxx>
#include <rtl/string.hxx>
#include <com/sun/star/uno/Sequence.hxx>
#include <com/sun/star/uno/Any.hxx>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <cppuhelper/implbase1.hxx>
#include <cppuhelper/implbase2.hxx>
#include <cppuhelper/servicefactory.hxx>

#ifndef _COM_SUN_STAR_LANG_XCOMPONENT_HDL_
#include <com/sun/star/lang/XComponent.hpp>
#endif
#ifndef _COM_SUN_STAR_IO_XACTIVEDATASOURCE_HDL_
#include <com/sun/star/io/XActiveDataSource.hpp>
#endif
#ifndef _COM_SUN_STAR_IO_XACTIVEDATASINK_HDL_
#include <com/sun/star/io/XActiveDataSink.hpp>
#endif
#ifndef _COM_SUN_STAR_IO_XACTIVEDATACONTROL_HDL_
#include <com/sun/star/io/XActiveDataControl.hpp>
#endif
#ifndef _COM_SUN_STAR_IO_XDATATRANSFEREVENTLISTENER_HDL_
#include <com/sun/star/io/XDataTransferEventListener.hpp>
#endif
#ifndef _COM_SUN_STAR_IO_XDATAIMPORTER_HDL_
#include <com/sun/star/io/XDataImporter.hpp>
#endif

#ifndef _COM_SUN_STAR_IO_XINPUTSTREAM_HDL_
#include <com/sun/star/io/XInputStream.hpp>
#endif
#include <com/sun/star/xml/sax/XParser.hpp>
#include <com/sun/star/xml/sax/SAXParseException.hpp>
#include <com/sun/star/xml/sax/XExtendedDocumentHandler.hpp>

#include <com/sun/star/xml/sax/InputSource.hpp>
#include <vos/thread.hxx>

#include <vos/pipe.hxx>
#include <osl/diagnose.h>
#include "oslstream.hxx"
#include <com/sun/star/xml/sax/XAttributeList.hpp>

#include <com/sun/star/container/XHierarchicalNameAccess.hpp>
#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/container/XHierarchicalName.hpp>

#include <com/sun/star/container/XNameReplace.hpp>
#include <com/sun/star/util/XChangesBatch.hpp>
#include <com/sun/star/script/XTypeConverter.hpp>
#include <com/sun/star/container/XNameContainer.hpp>
#include <osl/conditn.hxx>

#include "xmltreebuilder.hxx"

#include "dataimport.hxx"

#include "createpropertyvalue.hxx"
#include "strdecl.hxx"

#include "confname.hxx"

#include "FileHelper.hxx"

// -----------------------------------------------------------------------------
// --------------------------------- namespaces ---------------------------------
// -----------------------------------------------------------------------------
namespace uno = com::sun::star::uno;
namespace lang = com::sun::star::lang;
namespace io = com::sun::star::io;
namespace sax = com::sun::star::xml::sax;
namespace script = com::sun::star::script;

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::util;
using namespace ::com::sun::star::xml::sax;
using namespace ::com::sun::star::io;
using namespace rtl;
using namespace osl;

// -----------------------------------------------------------------------------
// ---------------------------------- defines ----------------------------------
// -----------------------------------------------------------------------------
#ifndef ASCII
#define ASCII(x) OUString::createFromAscii(x)
#endif

ostream& operator << (ostream& out, rtl::OUString const& aStr)
{
    sal_Unicode const* const pStr = aStr.getStr();
    sal_Unicode const* const pEnd = pStr + aStr.getLength();
    for (sal_Unicode const* p = pStr; p < pEnd; ++p)
        if (0 < *p && *p < 127) // ASCII
            out << char(*p);
        else
            out << "[\\u" << hex << *p << "]";
    return out;
}


// -----------------------------------------------------------------------------
// ---------------------------------- a Class ----------------------------------
// -----------------------------------------------------------------------------
// <Name a="xyz" b="bar">

//==========================================================================
//= Visitors
//==========================================================================


namespace configmgr
{

    class XMLSimpleDocHandler : public ::cppu::WeakImplHelper1<sax::XDocumentHandler>
    {
        // uno::Reference< sax::XDocumentHandler > m_xWriter;         // the service object for writing XML code

        sal_Int32 m_nElementDepth;
        sal_Int32 m_nIgnoreLevel;
    public:
        XMLSimpleDocHandler::XMLSimpleDocHandler()
            {}


        // DECLARE_UNO3_DEFAULTS(XMLReadFilter, CmDocumentHandler_BASE);

        // XDocumentHandler
        virtual void SAL_CALL startDocument(void)
            throw (sax::SAXException, uno::RuntimeException)
            {
            }

        virtual void SAL_CALL endDocument(void)
            throw(sax::SAXException, uno::RuntimeException)
            {
            }

        virtual void SAL_CALL startElement(const rtl::OUString& aName,
                                           const uno::Reference< sax::XAttributeList > &_xAttrList)
            throw(sax::SAXException, uno::RuntimeException)
            {
                sal_Int16 nAttrCount = _xAttrList.is() ? _xAttrList->getLength() : 0;
                for( sal_Int16 i=0; i < nAttrCount; i++ )
                {
                    OUString aParamName( _xAttrList->getNameByIndex( i ) );
                    OUString aParamValue( _xAttrList->getValueByIndex( i ) );
                    volatile int dummy = 0;
                }
            }

        virtual void SAL_CALL endElement(const rtl::OUString& aName)
            throw(sax::SAXException, uno::RuntimeException)
            {
            }

        virtual void SAL_CALL characters(const rtl::OUString& aChars)
            throw(sax::SAXException, uno::RuntimeException)
            {}


        virtual void SAL_CALL ignorableWhitespace(const rtl::OUString& aWhitespaces)
            throw(sax::SAXException, uno::RuntimeException)
            {}


        virtual void SAL_CALL processingInstruction(const rtl::OUString& aTarget,
                                                    const rtl::OUString& aData)
            throw(sax::SAXException, uno::RuntimeException)
            {}

        virtual void SAL_CALL setDocumentLocator(const uno::Reference< sax::XLocator > &xLocator)
            throw(sax::SAXException, uno::RuntimeException)
            {}
    };

// -----------------------------------------------------------------------------
// ------------------------------------ Test ------------------------------------
// -----------------------------------------------------------------------------
#define ASCII_STRING(rtlOUString)   rtl::OUStringToOString(rtlOUString, RTL_TEXTENCODING_ASCII_US).getStr()
    static ::rtl::OUString  sRegistry = ::rtl::OUString::createFromAscii("applicat.rdb");

    void simpleTest()
    {
        uno::Reference< lang::XMultiServiceFactory > xMSF;
        try
        {
            xMSF = cppu::createRegistryServiceFactory(sRegistry, ::rtl::OUString());
        }
        catch (uno::Exception& e)
        {
            cout << "could not bootstrap the services from " << ASCII_STRING(sRegistry) << endl ;
            cout << "    (error message : " << ASCII_STRING(e.Message) << ")" << endl;
        }

        if (!xMSF.is())
        {
            cerr << "could not create the service factory !" << endl;
            return;
        }

        OUString aPath = ASCII("e:/temp/Test");
        OUString aFilename = ASCII("calc");
        OUString aExtension = ASCII("xml");

        OUString aFullname = aPath + ASCII("/") + aFilename + ASCII(".") + aExtension;

        // Filename convertieren
        OUString aURL;
        File aConvert(ASCII(""));
        aConvert.normalizePath(aFullname, aURL);

        // File oeffnen
        File aFile(aURL);
        aFile.open(osl_File_OpenFlag_Read);

        uno::Reference<io::XInputStream> xInputStream =
            new configmgr::OSLInputStreamWrapper(aFile);

        // connect stream to input stream to the parser
        InputSource aInputSource;
        Reference<XInputStream> xPipeInput( xInputStream, UNO_QUERY );
        aInputSource.aInputStream = xPipeInput;

        Reference< sax::XParser > xParser;
        xParser = Reference< sax::XParser > (
            xMSF->createInstance(
                ::rtl::OUString::createFromAscii("com.sun.star.xml.sax.Parser")), UNO_QUERY);

        XMLSimpleDocHandler *pTest = new XMLSimpleDocHandler();

        // get filter
        Reference<XDocumentHandler> xFilter = pTest;

        // connect parser and filter
        xParser->setDocumentHandler( xFilter );

        // parse
        sal_Int16 nRet = 0;
        OUString sError;
        try
        {
            xParser->parseStream( aInputSource );
        }
        catch( SAXParseException &e )
        {
            OUString sLine = OUString::valueOf(e.LineNumber);
            OUString aStr = ASCII("SAXParseException occured in ");
            sError = aStr + ASCII(" Line: (") + sLine + ASCII(")");

            OSL_ENSURE(0, rtl::OUStringToOString(sError,RTL_TEXTENCODING_ASCII_US).getStr());
            nRet = 3;
        }
        catch( SAXException &e )
        {
            sError = e.Message;
            OSL_ENSURE(0, rtl::OUStringToOString(sError,RTL_TEXTENCODING_ASCII_US).getStr());
            nRet = 4;
        }
        catch( IOException &e )
        {
            sError = e.Message;
            OSL_ENSURE(0, rtl::OUStringToOString(sError,RTL_TEXTENCODING_ASCII_US).getStr());
            nRet = 5;
        }
    }


#include <osl/time.h>
#include <rtl/string.hxx>

class TimeTest
{
    TimeValue m_aStartTime, m_aEndTime;
    bool m_bStarted;
public:
    TimeTest()
        :m_bStarted(false)
        {
        }

    void start()
        {
            m_bStarted = true;
            osl_getSystemTime(&m_aStartTime);
        }
    void stop()
        {
            osl_getSystemTime(&m_aEndTime);
            OSL_ENSURE(m_bStarted, "Not Started.");
            m_bStarted = false;
        }
    void  showTime(const rtl::OString & aWhatStr)
        {
            OSL_ENSURE(!m_bStarted, "Not Stopped.");

            sal_Int32 nSeconds = m_aEndTime.Seconds - m_aStartTime.Seconds;
            sal_Int32 nNanoSec = m_aEndTime.Nanosec - m_aStartTime.Nanosec;
            if (nNanoSec < 0)
            {
                nNanoSec = 1000000000 - nNanoSec;
                nSeconds++;
            }
            rtl::OString aStr = "Time for ";
            aStr += aWhatStr;
            aStr += " : ";
            aStr += rtl::OString::valueOf(nSeconds);
            aStr += ".";
            aStr += rtl::OString::valueOf(nNanoSec);

            cout << aStr.getStr() << endl;
        }

};

// -----------------------------------------------------------------------------
// -------------------------------- Mapping Test --------------------------------
// -----------------------------------------------------------------------------
// Simple Map created with a stl::vector

typedef ::std::pair< rtl::OUString, rtl::OUString > Assoc;
// typedef ::std::set<Assoc, ltNode> MappingTable;
typedef std::vector<Assoc> MappingTable;

rtl::OUString mapTo(const rtl::OUString& aFrom, bool bToNew)
{
    static MappingTable aMap;
    if (aMap.empty())
    {
        // Fill Map old, new

        aMap.push_back(Assoc(ASCII("value"),    TAG_VALUE));
        aMap.push_back(Assoc(ASCII("type"),     ATTR_TYPE));
        aMap.push_back(Assoc(ASCII("instance"), ATTR_INSTANCE));
    }
    if (bToNew)
    {
        // check, if we should convert first to second
        for (std::vector<Assoc>::const_iterator it = aMap.begin();it != aMap.end();++it)
        {
            if ((*it).first.equals(aFrom))
                return (*it).second;
        }
    }
    else
    {
        // check if we should convert second to first
        for (std::vector<Assoc>::const_iterator it = aMap.begin();it != aMap.end();++it)
        {
            if ((*it).second.equals(aFrom))
                return (*it).first;
        }
    }

    // do nothing!
    return aFrom;
}
// -----------------------------------------------------------------------------
void simpleMappingTest()
{
    OUString aValue = ASCII("value");
    OUString aNew;

    aNew = mapTo(aValue, true);                  // true for ToNew
    aNew = mapTo(aNew, false);
    aNew = mapTo(aNew, false);

    volatile int dummy = 0;
}

// -----------------------------------------------------------------------------
    void speedTest()
    {
        // check speed of:
        // ASCII("value")
        // TAG_VALUE
        // ...

        sal_Int32 nCount = 1000 * 1000 * 100;
        sal_Int32 n;

        cout << "Starting Timetest" << endl;
        TimeTest tt;
        tt.start();
        for(n=0;n<nCount;n++)
        {
        }
        tt.stop();
        tt.showTime("Leere Schleife: ");


        nCount = 1000 * 1000 * 5;
        rtl::OUString aStr;

        tt.start();
        for(n=0;n<nCount;n++)
        {
            aStr = ASCII("value");
        }
        tt.stop();
        tt.showTime("ASCII() ");


        OUString aValue = ASCII("value");
        tt.start();
        for(n=0;n<nCount;n++)
        {
            aStr = TAG_VALUE;
        }
        tt.stop();
        tt.showTime("TAG_VALUE: ");

    }

OUString changeToComSunStarPath(const OUString &aPath)
{
    static OUString aOO  = ASCII("org.OpenOffice");
    static OUString aCSS = ASCII("com.sun.star");
    OUString aNewPath;

    // compare
    if (aPath.compareTo( aOO, aOO.getLength() ) == 0)
    {
        aNewPath = aCSS;
        aNewPath += aPath.copy(aOO.getLength());
        return aNewPath;
    }
    return aPath;
}

void stringTest2()
{
    OUString aPath = ASCII("org.OpenOffice.Setup/blah/blub");

    OUString aNewPath = changeToComSunStarPath(aPath);
    volatile int dummy = 0;
}

/*

class A
{
public:
    static void run() {
        cout << "This is A::run();" << endl;
    }

};

class B : public A
{
public:

    static void run() {
        cout << "This is B::run();" << endl;
    }
};

void classTest()
{
    A a;
    B b;
    B::run();
}
*/

// -----------------------------------------------------------------------------
// ------------------------------------ Map ------------------------------------
// -----------------------------------------------------------------------------
struct ltstr
{
    bool operator()(const rtl::OUString &s1, const rtl::OUString &s2) const
        {
            return s1.compareTo(s2) < 0 ? true : false;
        }
};

void stringTest()
{
    map<const OUString, int, ltstr> months;

    months[ASCII("january")] = 31;
    months[ASCII("february")] = 28;
    months[ASCII("march")] = 31;
    months[ASCII("april")] = 30;
    months[ASCII("may")] = 31;
    months[ASCII("june")] = 30;
    months[ASCII("july")] = 31;
    months[ASCII("august")] = 31;
    months[ASCII("september")] = 30;
    months[ASCII("october")] = 31;
    months[ASCII("november")] = 30;
    months[ASCII("december")] = 31;

    cout << "june -> " << months[ASCII("june")] << endl;
    map<const OUString, int, ltstr>::iterator cur  = months.find(ASCII("april"));
    // map<const OUString, int, ltstr>::iterator prev = cur;
    map<const OUString, int, ltstr>::iterator next = cur;
    // ++next;
    // --prev;
    // cout << "Previous (in alphabetical order) is " << (*prev).first << endl;
    for(int i=0;i<12;i++)
    {
        cout << "Next (in alphabetical order) is " << (*next).first << " days " << (*next).second << endl;
        ++next;
    }
}

// -----------------------------------------------------------------------------
// ---------------------------------- HashMap ----------------------------------
// -----------------------------------------------------------------------------
#include <hash_map>


namespace test {

struct eqstr
{
    bool operator()(const rtl::OUString &s1, const rtl::OUString &s2) const
        {
            return s1.equals(s2) == sal_True ? true : false;
        }
};

struct hash_oustring
{
    // This hash funktion is a copy of hash<char*> from SGI-STL
    size_t operator()(const rtl::OUString &_s) const
        {
            sal_Int64 nStrLen = _s.getLength();
            const sal_Unicode *pStr = _s.getStr();
            unsigned long h = 0;
            for (sal_Int64 i=0;i<nStrLen; ++i)
                h = 5*h + *pStr++;

            return size_t(h);
        }
};

void hash_test()
{
    hash_map<const rtl::OUString, rtl::OUString, hash_oustring, eqstr> months;

    rtl::OUString sJanuary = ASCII("january");
    months[sJanuary]   = ASCII("31");
    months[ASCII("february")]  = ASCII("28");
    months[ASCII("march")]     = ASCII("31");
    months[ASCII("april")]     = ASCII("30");
    months[ASCII("may")]       = ASCII("31");
    months[ASCII("june")]      = ASCII("30");
    months[ASCII("july")]      = ASCII("31");
    months[ASCII("august")]    = ASCII("31");
    months[ASCII("september")] = ASCII("30");
    months[ASCII("october")]   = ASCII("31");
    months[ASCII("november")]  = ASCII("30");
    months[ASCII("december")]  = ASCII("31");

    cout << "september -> " << months[ASCII("september")] << endl;
    cout << "april     -> " << months[ASCII("april")] << endl;
    cout << "june      -> " << months[ASCII("june")] << endl;
    cout << "november  -> " << months[ASCII("november")] << endl;
}
}


// -----------------------------------------------------------------------------
// Fri Nov 10 15:10:45 2000
// -----------------------------------------------------------------------------

#include <vos/ref.hxx>

class Options : public vos::OReference
{
    int m_aValue;
public:
    int getValue() {return m_aValue;}
    void setValue(int _aValue) {m_aValue = _aValue;}


};

class A
{
    vos::ORef<Options> m_aOptions;
public:

    vos::ORef<Options> getOptions() {return m_aOptions;}
    void setOptions(vos::ORef<Options>& _aOptions) {
        m_aOptions = _aOptions;
    }
};

void testRefs()
{
    vos::ORef<Options> aO = new Options;
    aO->setValue(10);

    A a,b;
    a.setOptions(aO);
    b.setOptions(aO);
    cout << "Options from a : " << a.getOptions()->getValue() << endl;
    cout << "Options from b : " << b.getOptions()->getValue() << endl;

    aO->setValue(20);
    cout << "Options from a : " << a.getOptions()->getValue() << endl;
    cout << "Options from b : " << b.getOptions()->getValue() << endl;
}



void ConfigName()
{
    // OUString aSubtreePath = ASCII("/org.openoffice.office.common/path/blah/blub");
    OUString aSubtreePath = ASCII("/org.openoffice.office.common");
    ConfigurationName aName(aSubtreePath);
    OUString a = aName.localName();
    OUString b = aName.fullName();
    OUString c = aName.moduleName();
    ConfigurationName aParentName(aName.getParentName());
    OUString d = aParentName.fullName();


    {
        ConfigurationName aName( ASCII("/" ));

        std::stack< rtl::OUString, std::vector<rtl::OUString> > m_aStringStack;

        if (aName.localName().getLength() != 0)
        {
            for (ConfigurationName::Iterator it = aName.begin();
                 it != aName.end();
                 ++it)
            {
                rtl::OUString aName = *it;
                m_aStringStack.push(aName);
                // m_xHandler->startElement(*it, rList);
                volatile int dummy = 0;
            }
        }

        // bBack = writeChanges();

        while(!m_aStringStack.empty())
        {
            OUString aName = m_aStringStack.top();
            m_aStringStack.pop();
        }
    }
}

void ConfigName2()
{
    OUString aSubtreePath = ASCII("/org.openoffice.office.common/path/blah/blub");
    ConfigurationName aName(aSubtreePath);
    ConfigurationName aParent = aName.getParentName();

    for (ConfigurationName::Iterator it = aName.begin();
         it != aName.end();
         ++it)
    {
        rtl::OUString aName = *it;
        volatile int dummy = 0;
    }
}

// -----------------------------------------------------------------------------
inline void operator <<= (rtl::OUString& _rUnicodeString, const sal_Char* _pAsciiString)
{
    _rUnicodeString = ::rtl::OUString::createFromAscii(_pAsciiString);
}

inline void operator <<= (rtl::OUString& _rUnicodeString, const rtl::OString& _rAsciiString)
{
    _rUnicodeString <<= _rAsciiString.getStr();
}

inline void operator <<= (rtl::OString& _rAsciiString, const rtl::OUString& _rUnicodeString)
{
    _rAsciiString = rtl::OUStringToOString(_rUnicodeString,RTL_TEXTENCODING_ASCII_US);
}

// -----------------------------------------------------------------------------
bool isBTimeGreaterATime(TimeValue const& A, TimeValue const& B)
{
    if (B.Seconds > A.Seconds) return true;
    if (B.Nanosec > A.Nanosec) return true;

    // lower or equal
    return false;
}

// -----------------------------------------------------------------------------

void oslTest()
{

    OUString aDirectory(FileHelper::convertFilenameToFileURL(ASCII("c:/temp/file.out.1")));
    TimeValue a = FileHelper::getFileModificationStamp(aDirectory);

    OUString aDirectory2(FileHelper::convertFilenameToFileURL(ASCII("c:/temp/file.out.2")));
    TimeValue b = FileHelper::getFileModificationStamp(aDirectory2);

    if (isBTimeGreaterATime(a,b))
    {
        OSL_ENSURE(false, "FileB ist neuer als FileA");
    }


/*
    OUString aDirectory(FileHelper::convertFilenameToFileURL(ASCII("c:/temp/dies")));
    osl::FileBase::RC eError = osl::Directory::create(aDirectory);
    if (eError != osl::FileBase::E_None)
    {
        OUString aUStr = FileHelper::createOSLErrorString(eError);
        OString aStr;
        aStr <<= aUStr;
        OSL_ENSURE(false, aStr.getStr());
    }
*/
}

} // namespace configmgr
