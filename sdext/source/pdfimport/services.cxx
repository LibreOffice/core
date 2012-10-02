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


#include "pdfiadaptor.hxx"
#include "filterdet.hxx"
#include "treevisitorfactory.hxx"

#include <cppuhelper/factory.hxx>
#include <cppuhelper/implementationentry.hxx>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::registry;


namespace
{
    static Reference< XInterface > Create_PDFIHybridAdaptor( const Reference< XComponentContext >& _rxContext )
    {
        return *(new pdfi::PDFIHybridAdaptor( _rxContext ));
    }

    static Reference< XInterface > Create_PDFIRawAdaptor_Writer( const Reference< XComponentContext >& _rxContext )
    {
        pdfi::PDFIRawAdaptor* pAdaptor = new pdfi::PDFIRawAdaptor( _rxContext );

        pAdaptor->setTreeVisitorFactory(pdfi::createWriterTreeVisitorFactory());
        pAdaptor->enableToplevelText(); // TEMP! TEMP!

        return uno::Reference<uno::XInterface>(static_cast<xml::XImportFilter*>(pAdaptor));
    }

    static Reference< XInterface > Create_PDFIRawAdaptor_Draw( const Reference< XComponentContext >& _rxContext )
    {
        pdfi::PDFIRawAdaptor* pAdaptor = new pdfi::PDFIRawAdaptor( _rxContext );

        pAdaptor->setTreeVisitorFactory(pdfi::createDrawTreeVisitorFactory());

        return uno::Reference<uno::XInterface>(static_cast<xml::XImportFilter*>(pAdaptor));
    }

    static Reference< XInterface > Create_PDFIRawAdaptor_Impress( const Reference< XComponentContext >& _rxContext )
    {
        pdfi::PDFIRawAdaptor* pAdaptor = new pdfi::PDFIRawAdaptor( _rxContext );

        pAdaptor->setTreeVisitorFactory(pdfi::createImpressTreeVisitorFactory());

        return uno::Reference<uno::XInterface>(static_cast<xml::XImportFilter*>(pAdaptor));
    }

    static Reference< XInterface > Create_PDFDetector( const Reference< XComponentContext >& _rxContext )
    {
        return *(new pdfi::PDFDetector( _rxContext ) );
    }
}

namespace
{
    typedef Reference< XInterface > (SAL_CALL * ComponentFactory)( const Reference< XComponentContext >& );

    struct ComponentDescription
    {
        const sal_Char*     pAsciiServiceName;
        const sal_Char*     pAsciiImplementationName;
        ComponentFactory    pFactory;

        ComponentDescription()
            :pAsciiServiceName( NULL )
            ,pAsciiImplementationName( NULL )
            ,pFactory( NULL )
        {
        }
        ComponentDescription( const sal_Char* _pAsciiServiceName, const sal_Char* _pAsciiImplementationName, ComponentFactory _pFactory )
            :pAsciiServiceName( _pAsciiServiceName )
            ,pAsciiImplementationName( _pAsciiImplementationName )
            ,pFactory( _pFactory )
        {
        }
    };

    static const ComponentDescription* lcl_getComponents()
    {
        static const ComponentDescription aDescriptions[] = {
            ComponentDescription( "com.sun.star.document.ImportFilter", "com.sun.star.comp.documents.HybridPDFImport", Create_PDFIHybridAdaptor ),
            ComponentDescription( "com.sun.star.document.ImportFilter", "com.sun.star.comp.documents.WriterPDFImport", Create_PDFIRawAdaptor_Writer ),
            ComponentDescription( "com.sun.star.document.ImportFilter", "com.sun.star.comp.documents.DrawPDFImport", Create_PDFIRawAdaptor_Draw ),
            ComponentDescription( "com.sun.star.document.ImportFilter", "com.sun.star.comp.documents.ImpressPDFImport", Create_PDFIRawAdaptor_Impress ),
            ComponentDescription( "com.sun.star.document.ImportFilter", "com.sun.star.comp.documents.PDFDetector", Create_PDFDetector ),
            ComponentDescription()
        };
        return aDescriptions;
    }
}

extern "C" SAL_DLLPUBLIC_EXPORT void* SAL_CALL pdfimport_component_getFactory(
    const sal_Char* pImplementationName,
    SAL_UNUSED_PARAMETER void* /*pServiceManager*/,
    SAL_UNUSED_PARAMETER void* /*pRegistryKey*/ )
{
    ::rtl::OUString sImplementationName( ::rtl::OUString::createFromAscii( pImplementationName ) );

    Reference< XSingleComponentFactory > xFactory;

    const ComponentDescription* pComponents = lcl_getComponents();
    while ( pComponents->pAsciiServiceName != NULL )
    {
        if ( 0 == sImplementationName.compareToAscii( pComponents->pAsciiImplementationName ) )
        {
            Sequence< ::rtl::OUString > sServices(1);
            sServices[0] = ::rtl::OUString::createFromAscii( pComponents->pAsciiServiceName );

            xFactory = ::cppu::createSingleComponentFactory(
                pComponents->pFactory,
                sImplementationName,
                sServices,
                NULL
            );
            break;
        }

        ++pComponents;
    }

    // by definition, objects returned via this C API need to ber acquired once
    xFactory->acquire();
    return xFactory.get();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
