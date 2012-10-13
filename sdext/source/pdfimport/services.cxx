/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */


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
