/*************************************************************************
 *
 *  $RCSfile: xmlexport.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:13:43 $
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

#include<iostream>

#ifndef _OSL_FILE_HXX_
#include<osl/file.hxx>
#endif

#include <rtl/ustring.hxx>

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
/*
#ifndef _CPPUHELPER_IMPLBASE1_HXX_
#include <cppuhelper/implbase1.hxx>
#endif
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
#ifndef _COM_SUN_STAR_IO_XACTIVEDATASOURCE_HPP_
#include <com/sun/star/io/XActiveDataSource.hpp>
#endif
#ifndef _COM_SUN_STAR_XML_SAX_SAXPARSEEXCEPTION_HPP_
#include <com/sun/star/xml/sax/SAXParseException.hpp>
#endif

#ifndef _COM_SUN_STAR_XML_SAX_XEXTENDEDDOCUMENTHANDLER_HPP_
#include <com/sun/star/xml/sax/XExtendedDocumentHandler.hpp>
#endif

#ifndef _OSL_DIAGNOSE_H_
#include <osl/diagnose.h>
#endif
#ifndef _COM_SUN_STAR_XML_SAX_XATTRIBUTELIST_HPP_
#include <com/sun/star/xml/sax/XAttributeList.hpp>
#endif

#include "oslstream.hxx"
#include "attributes.hxx"
#include "typeconverter.hxx"
#include "xmlformater.hxx"

#ifndef _CONFIGMGR_FILEHELPER_HXX_
#include "filehelper.hxx"
#endif


#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>

#ifndef _COM_SUN_STAR_BEANS_PROPERTYVALUE_HPP_
#include <com/sun/star/beans/PropertyValue.hpp>
#endif

#ifndef _COM_SUN_STAR_LANG_ILLEGALARGUMENTEXCEPTION_HPP_
#include <com/sun/star/lang/IllegalArgumentException.hpp>
#endif

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
                OSL_ENSHURE(0, "disposing");
            }


        virtual void SAL_CALL finished( const io::DataTransferEvent& aEvent )
            throw(uno::RuntimeException)
            {
                OSL_ENSHURE(0, "finished");
            }

        virtual void SAL_CALL cancelled( const io::DataTransferEvent& aEvent )
            throw(uno::RuntimeException)
            {
                OSL_ENSHURE(0, "cancelled");
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
            OSL_ENSHURE(0, aStr.getStr());
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
