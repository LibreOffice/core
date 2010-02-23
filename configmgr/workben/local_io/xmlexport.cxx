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

#include<iostream>
#include<osl/file.hxx>

#include <rtl/ustring.hxx>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <cppuhelper/implbase1.hxx>
#include <cppuhelper/implbase2.hxx>
#include <cppuhelper/servicefactory.hxx>
/*
#include <cppuhelper/implbase1.hxx>
*/
#ifndef _COM_SUN_STAR_LANG_XCOMPONENT_HDL_
#include <com/sun/star/lang/XComponent.hpp>
#endif
#ifndef _COM_SUN_STAR_IO_XDATATRANSFEREVENTLISTENER_HDL_
#include <com/sun/star/io/XDataTransferEventListener.hpp>
#endif
#ifndef _COM_SUN_STAR_IO_XDATAIMPORTER_HDL_
#include <com/sun/star/io/XDataExporter.hpp>
#endif
#ifndef _COM_SUN_STAR_IO_XINPUTSTREAM_HDL_
#include <com/sun/star/io/XOutputStream.hpp>
#endif
#include <com/sun/star/io/XActiveDataSource.hpp>
#include <com/sun/star/xml/sax/SAXParseException.hpp>
#include <com/sun/star/xml/sax/XExtendedDocumentHandler.hpp>
#include <osl/diagnose.h>
#include <com/sun/star/xml/sax/XAttributeList.hpp>

#include "oslstream.hxx"
#include "attributes.hxx"
#include "typeconverter.hxx"
#include "xmlformater.hxx"
#include "filehelper.hxx"


#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/lang/IllegalArgumentException.hpp>

#include "createpropertyvalue.hxx"
// -----------------------------------------------------------------------------
// --------------------------------- namespaces ---------------------------------
// -----------------------------------------------------------------------------
namespace configmgr
{

    namespace uno = com::sun::star::uno;
    namespace lang = com::sun::star::lang;
    namespace io = com::sun::star::io;
    namespace sax = com::sun::star::xml::sax;
    namespace beans = com::sun::star::beans;

    using ::rtl::OUString;
    using ::osl::File;

    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::lang;
    using namespace ::com::sun::star::container;

// -----------------------------------------------------------------------------
// ---------------------------------- defines ----------------------------------
// -----------------------------------------------------------------------------
#define ASCII(x) OUString::createFromAscii(x)

// -----------------------------------------------------------------------------
    class Listener : public ::cppu::WeakImplHelper1<io::XDataTransferEventListener>
    {
        virtual void SAL_CALL disposing( const lang::EventObject& Source )
            throw(::com::sun::star::uno::RuntimeException)
            {
                OSL_ENSURE(0, "disposing");
            }


        virtual void SAL_CALL finished( const io::DataTransferEvent& aEvent )
            throw(uno::RuntimeException)
            {
                OSL_ENSURE(0, "finished");
            }

        virtual void SAL_CALL cancelled( const io::DataTransferEvent& aEvent )
            throw(uno::RuntimeException)
            {
                OSL_ENSURE(0, "cancelled");
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





#define ASCII_STRING(rtlOUString)   rtl::OUStringToOString(rtlOUString, RTL_TEXTENCODING_ASCII_US).getStr()
    static ::rtl::OUString  sRegistry = ::rtl::OUString::createFromAscii("applicat.rdb");








// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
    void exportTest()
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
        OUString sFilename = ASCII("exported_config");
        OUString aExtension = ASCII("xml");

        OUString aFullname = aPath + ASCII("/") + sFilename + ASCII(".") + aExtension;

        // Filename convertieren
        OUString aURL;
        File aConvert(ASCII(""));
        aConvert.normalizePath(aFullname, aURL);

        FileHelper::tryToRemoveFile(aURL);

        rtl::OUString sError;

        // File oeffnen
        File aFile(aURL);
        osl::FileBase::RC eError = aFile.open(OpenFlag_Write | OpenFlag_Create);

        if (eError != osl_File_E_None)
        {
            sError = ASCII("XMLExportTest:");
            rtl::OString aStr = rtl::OUStringToOString(sError,RTL_TEXTENCODING_ASCII_US);
            OSL_ENSURE(0, aStr.getStr());
            return;
        }

        // create an outputstream
        uno::Reference<io::XOutputStream> xOutputStream = new configmgr::OSLOutputStreamWrapper(aFile);

        // Listener & Component
        uno::Reference<io::XDataTransferEventListener>  rListener = new Listener();
        uno::Reference<lang::XComponent> rComponent = new Component();


        OUString sPath = ASCII("com.sun.star.ucb.Hierarchy");

        // Create a TypeConverter
        uno::Reference<script::XTypeConverter> aConverter;
        aConverter = aConverter.query(xMSF->createInstance(ASCII( "com.sun.star.script.Converter" )) );

        // prepare parameters for DataExport
        Sequence< uno::Any > aArgs(4);
        aArgs[0] <<= createPropertyValue(ASCII("PackageName"), sFilename);
        aArgs[1] <<= createPropertyValue(ASCII("Path"), sPath); // multiple occur possible
        aArgs[2] <<= createPropertyValue(ASCII("TypeConverter"), aConverter); // optional
        aArgs[3] <<= createPropertyValue(ASCII("Path"), ASCII("com.sun.star.office.Setup"));    // multiple occur possible

        // important: createInstanceWithArguments will throw an IllegalArgumentException() if
        // parameters are wrong...
        Reference< XInterface > xDataExport = xMSF->createInstanceWithArguments(
            OUString::createFromAscii("com.sun.star.configuration.DataExport"),
            aArgs);

        uno::Reference<io::XDataExporter>  rExporter(xDataExport, UNO_QUERY);

        // Export Data
        rExporter->exportData(xOutputStream, rComponent, rListener);
    }

} // namespace configmgr
