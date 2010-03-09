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
// -----------------------------------------------------------------------------
// --------------------------------- namespaces ---------------------------------
// -----------------------------------------------------------------------------
namespace uno = com::sun::star::uno;
namespace lang = com::sun::star::lang;
namespace io = com::sun::star::io;
namespace sax = com::sun::star::xml::sax;
namespace script = com::sun::star::script;

using ::rtl::OUString;
using ::osl::File;

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::util;

// -----------------------------------------------------------------------------
// ---------------------------------- defines ----------------------------------
// -----------------------------------------------------------------------------
#ifndef ASCII
#define ASCII(x) OUString::createFromAscii(x)
#endif


// -----------------------------------------------------------------------------
// ---------------------------------- a Class ----------------------------------
// -----------------------------------------------------------------------------
// <Name a="xyz" b="bar">

//==========================================================================
//= Visitors
//==========================================================================


// -----------------------------------------------------------------------------
class Listener : public ::cppu::WeakImplHelper1<io::XDataTransferEventListener>
{
    osl::Condition m_aCondition;
public:
    Listener()
        {
            m_aCondition.reset();                // will block
        }

    virtual void SAL_CALL disposing( const lang::EventObject& Source )
        throw(::com::sun::star::uno::RuntimeException)
        {
            OSL_ENSURE(0, "disposing");
            m_aCondition.set();              // will not block
        }

    virtual void SAL_CALL finished( const io::DataTransferEvent& aEvent )
        throw(uno::RuntimeException)
        {
            OSL_ENSURE(0, "finished");
            m_aCondition.set();              // will not block
        }

    virtual void SAL_CALL cancelled( const io::DataTransferEvent& aEvent )
        throw(uno::RuntimeException)
        {
            OSL_ENSURE(0, "cancelled");
            m_aCondition.set();              // will not block
        }
public:
    void wait()
        {
            m_aCondition.wait();
        }
};

class Component: public ::cppu::WeakImplHelper1<lang::XComponent>
{
    virtual void SAL_CALL dispose(  )
        throw(uno::RuntimeException)
        {
            OSL_ENSURE(0, "dispose");

        }
    virtual void SAL_CALL addEventListener( const uno::Reference< lang::XEventListener >& xListener )
        throw(uno::RuntimeException)
        {
            OSL_ENSURE(0, "addEventListener");
        }
    virtual void SAL_CALL removeEventListener( const uno::Reference< lang::XEventListener >& aListener )
        throw(uno::RuntimeException)
        {
            OSL_ENSURE(0, "removeEventListener");
        }
};



namespace configmgr
{

// -----------------------------------------------------------------------------
// ------------------------------------ Test ------------------------------------
// -----------------------------------------------------------------------------
#define ASCII_STRING(rtlOUString)   rtl::OUStringToOString(rtlOUString, RTL_TEXTENCODING_ASCII_US).getStr()
    static ::rtl::OUString  sRegistry = ::rtl::OUString::createFromAscii("applicat.rdb");

    void importTest()
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

        OUString aPath = ASCII("l:/SRC601/configmgr/workben/local_io");
        OUString aFilename = ASCII("hierarchy");
        OUString aExtension = ASCII("xml");

        OUString aFullname = aPath + ASCII("/") + aFilename + ASCII(".") + aExtension;

        // Filename convertieren
        OUString aURL;
        File aConvert(ASCII(""));
        aConvert.normalizePath(aFullname, aURL);

        // File oeffnen
        File aFile(aURL);
        aFile.open(osl_File_OpenFlag_Read);

        // sal_uInt64 nBytesRead;
        // uno::Sequence< sal_Int8 > aBufferSeq(2000);
        // sal_Int8 *pBuff = aBufferSeq.getArray();
        // aFile.read(pBuff, 2000, nBytesRead);
        //
        // aFile.close();

        uno::Reference<io::XInputStream> xInputStream = new configmgr::OSLInputStreamWrapper(aFile);

        uno::Reference <uno::XInterface> xPump = xMSF->createInstance( L"com.sun.star.io.Pump" );
        OSL_ENSURE(xPump.is(), "there is no pump");

        uno::Reference<io::XActiveDataSink> xPumpSink(xPump, uno::UNO_QUERY);
        xPumpSink->setInputStream(xInputStream);

        uno::Reference<io::XActiveDataSource> xPumpSource(xPump, uno::UNO_QUERY);

        Listener *pListener = new Listener();
        uno::Reference<io::XDataTransferEventListener>  rListener = pListener;
        uno::Reference<lang::XComponent> rComponent = new Component();

        // Create a TypeConverter
        uno::Reference<script::XTypeConverter> aConverter;
        aConverter = aConverter.query(xMSF->createInstance(ASCII( "com.sun.star.script.Converter" )) );

        Sequence< uno::Any > aArgs(1);           // optional arg.
        aArgs[0] <<= configmgr::createPropertyValue(ASCII("TypeConverter"), aConverter);

        Reference< XInterface > xDataImport = xMSF->createInstanceWithArguments(
            OUString::createFromAscii("com.sun.star.configuration.DataImport"),
            aArgs);

        if (xDataImport.is())
        {
            uno::Reference<io::XDataImporter>  rImporter(xDataImport, UNO_QUERY);

            // Import Data
            rImporter->importData(xPumpSource, rComponent, rListener);

            // lets pump
            uno::Reference<io::XActiveDataControl> xControl(xPump, uno::UNO_QUERY);
            xControl->start();

            // TEST:
            // rImporter->cancel();

            // pImporter->wait();
            // Wait until the listener send ready
            pListener->wait();
        }
        return;
    }

// -----------------------------------------------------------------------------
// -------------------------------- Import Test --------------------------------
// -----------------------------------------------------------------------------

    // ----------- TEST ReadAccess -----------
    void showSequence(const Sequence<OUString> &aSeq);

void hierarchyTest()
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


    Reference< XMultiServiceFactory > xCfgProvider( xMSF->createInstance(
        ::rtl::OUString::createFromAscii("com.sun.star.configuration.ConfigurationProvider")),
                                                    UNO_QUERY);

    if (!xCfgProvider.is())
    {
        OSL_ENSURE(0, "No Configuration Provider");
    }

    OUString sPath = ASCII("com.sun.star.ucb.Hierarchy");

    Sequence< Any > aArgs(1);
    aArgs[0] <<= sPath;

    Reference< XInterface > xCfgUpdt = xCfgProvider->createInstanceWithArguments(
        OUString::createFromAscii("com.sun.star.configuration.ConfigurationUpdateAccess"),
        aArgs);

    Reference< XNameAccess > xNameAccess(xCfgUpdt, UNO_QUERY);

    // which Names are exist?
    showSequence(xNameAccess->getElementNames());

    OUString aNamePath = ASCII("Root");
    if (xNameAccess->hasByName(aNamePath))
    {
        Any aAny;
        aAny = xNameAccess->getByName(aNamePath);
        TypeClass aTypeClass = aAny.getValueTypeClass();
        if (aAny.getValueTypeClass() == TypeClass_INTERFACE)
        {
            Reference< XInterface > xInterface;
            aAny >>= xInterface;
            Reference< XNameAccess > xNameAccess2(xInterface, UNO_QUERY);
            if (xNameAccess.is())
            {
                Sequence<OUString> aSeq = xNameAccess2->getElementNames();
                showSequence(aSeq);

                // insert a new set
/*
                Reference<XHierarchicalName> xHierarchical(xNameAccess, UNO_QUERY);
                OUString aHierachicalName = xHierarchical->getHierarchicalName();

                Sequence< Any > aArgs(2);
                aArgs[0] <<= aHierachicalName;
                sal_Int32 nLevels=1;
                aArgs[1] <<= nLevels;

                Reference< XInterface > xCfgNewUpdt = xCfgProvider->createInstanceWithArguments(
                    OUString::createFromAscii("com.sun.star.configuration.ConfigurationUpdateAccess"),
                    aArgs);
*/
                Reference< lang::XSingleServiceFactory > xChildFactory(xNameAccess2, UNO_QUERY);
                if (xChildFactory.is())
                {
                    Reference< XInterface > xChild = xChildFactory->createInstance();
                    if (xChild.is())
                    {
                        Reference<XNameAccess> xObjectOnTheMedow(xChild, UNO_QUERY);




                        // which Names are exist?
                        showSequence(xObjectOnTheMedow->getElementNames());

                        OUString aChildren = ASCII("Children");
                        if (xObjectOnTheMedow->hasByName(aChildren))
                        {
                            Any aAny;
                            aAny = xObjectOnTheMedow->getByName(aChildren);
                            TypeClass aTypeClass = aAny.getValueTypeClass();
                            if (aAny.getValueTypeClass() == TypeClass_INTERFACE)
                            {
                                Reference< XInterface > xInterface;
                                aAny >>= xInterface;
                                Reference< XNameAccess > xChildFromOOM(xInterface, UNO_QUERY);

                                if (xChildFromOOM.is())
                                {
                                    // insert a new Object in the new Object

                                    Reference< lang::XSingleServiceFactory > xChildFactory(xChildFromOOM, UNO_QUERY);
                                    if (xChildFactory.is())
                                    {
                                        Reference< XInterface > xChild = xChildFactory->createInstance();
                                        if (xChild.is())
                                        {
                                            Reference<XNameContainer> xNameContainer(xChildFromOOM, UNO_QUERY);
                                            if (xNameContainer.is())
                                            {
                                                xNameContainer->insertByName(ASCII("Test2"), makeAny(xChild));
                                            }
                                        }
                                    }
                                }
                            }
                        }


                        Reference<XNameContainer> xNameContainer(xNameAccess2, UNO_QUERY);
                        if (xNameContainer.is())
                        {
                            xNameContainer->insertByName(ASCII("Test"), makeAny(xObjectOnTheMedow));
                        }
                    }

                    // commit changes
                    Reference< XChangesBatch > xChangesBatch(xCfgUpdt, UNO_QUERY);
                    if (xChangesBatch.is())
                    {
                        xChangesBatch->commitChanges();
                    }
                }

                Sequence<OUString> aSeq2 = xNameAccess2->getElementNames();
                showSequence(aSeq2);

/*
                OUString *pStr = aSeq.getArray();
                for (int i=0;i<aSeq.getLength();i++)
                {
                    OUString aStr = pStr[i];
                    Any aAny;
                    aAny = xNameAccess->getByName(aStr);
                    TypeClass aTypeClass = aAny.getValueTypeClass();

                    Reference< XNameReplace > xNameReplace(xNameAccess, UNO_QUERY);

                    Any aNewAny;
                    sal_Bool bValue = false;
                    aNewAny <<= bValue;
                    xNameReplace->replaceByName(aStr, aNewAny);

                    volatile int dummy = 0;
                }
*/
            }
        }
        volatile int dummy = 0;
    }
}

// ------------------------------ Helperfunctions ------------------------------

    void showSequence(const Sequence<OUString> &aSeq)
    {
        OUString aArray;
        const OUString *pStr = aSeq.getConstArray();
        for (int i=0;i<aSeq.getLength();i++)
        {
            OUString aStr = pStr[i];
            aArray += aStr + ASCII(", ");
        }
        volatile int dummy = 0;
    }

} // namespace configmgr
