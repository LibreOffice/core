/*************************************************************************
 *
 *  $RCSfile: xmlimport.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: armin $ $Date: 2001-03-08 09:08:52 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#include <memory>
#include <vector>
#include <stack>

#ifndef _OSL_FILE_HXX_
#include<osl/file.hxx>
#endif

#include <rtl/ustring.hxx>

#ifndef _COM_SUN_STAR_UNO_SEQUENCE_HXX_
#include <com/sun/star/uno/Sequence.hxx>
#endif

#ifndef _COM_SUN_STAR_UNO_ANY_HXX_
#include <com/sun/star/uno/Any.hxx>
#endif

#ifndef _COM_SUN_STAR_LANG_XMULTISERVICEFACTORY_HPP_
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#endif
#ifndef _CPPUHELPER_IMPLBASE1_HXX_
#include <cppuhelper/implbase1.hxx>
#endif
#ifndef _CPPUHELPER_IMPLBASE2_HXX_
#include <cppuhelper/implbase2.hxx>
#endif

#ifndef _CPPUHELPER_SERVICEFACTORY_HXX_
#include <cppuhelper/servicefactory.hxx>
#endif

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

#ifndef _COM_SUN_STAR_XML_SAX_XPARSER_HPP_
#include <com/sun/star/xml/sax/XParser.hpp>
#endif

#ifndef _COM_SUN_STAR_XML_SAX_SAXPARSEEXCEPTION_HPP_
#include <com/sun/star/xml/sax/SAXParseException.hpp>
#endif

#ifndef _COM_SUN_STAR_XML_SAX_XEXTENDEDDOCUMENTHANDLER_HPP_
#include <com/sun/star/xml/sax/XExtendedDocumentHandler.hpp>
#endif

#include <com/sun/star/xml/sax/InputSource.hpp>

#ifndef _VOS_THREAD_HXX_
#include <vos/thread.hxx>
#endif

#include <vos/pipe.hxx>

#ifndef _OSL_DIAGNOSE_H_
#include <osl/diagnose.h>
#endif
#include "oslstream.hxx"

#ifndef _COM_SUN_STAR_XML_SAX_XATTRIBUTELIST_HPP_
#include <com/sun/star/xml/sax/XAttributeList.hpp>
#endif

#include <com/sun/star/container/XHierarchicalNameAccess.hpp>
#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/container/XHierarchicalName.hpp>

#include <com/sun/star/container/XNameReplace.hpp>
#include <com/sun/star/util/XChangesBatch.hpp>

#ifndef _COM_SUN_STAR_SCRIPT_XTYPECONVERTER_HPP_
#include <com/sun/star/script/XTypeConverter.hpp>
#endif

#ifndef _COM_SUN_STAR_CONTAINER_XNAMECONTAINER_HPP_
#include <com/sun/star/container/XNameContainer.hpp>
#endif

#ifndef _OSL_CONDITN_HXX_
#include <osl/conditn.hxx>
#endif

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
            OSL_ENSHURE(0, "disposing");
            m_aCondition.set();              // will not block
        }

    virtual void SAL_CALL finished( const io::DataTransferEvent& aEvent )
        throw(uno::RuntimeException)
        {
            OSL_ENSHURE(0, "finished");
            m_aCondition.set();              // will not block
        }

    virtual void SAL_CALL cancelled( const io::DataTransferEvent& aEvent )
        throw(uno::RuntimeException)
        {
            OSL_ENSHURE(0, "cancelled");
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
            OSL_ENSHURE(0, "dispose");

        }
    virtual void SAL_CALL addEventListener( const uno::Reference< lang::XEventListener >& xListener )
        throw(uno::RuntimeException)
        {
            OSL_ENSHURE(0, "addEventListener");
        }
    virtual void SAL_CALL removeEventListener( const uno::Reference< lang::XEventListener >& aListener )
        throw(uno::RuntimeException)
        {
            OSL_ENSHURE(0, "removeEventListener");
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
        OSL_ENSHURE(xPump.is(), "there is no pump");

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
        OSL_ENSHURE(0, "No Configuration Provider");
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
