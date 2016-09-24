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
    Reference< XInterface > Create_PDFIHybridAdaptor( const Reference< XComponentContext >& _rxContext )
    {
        return *(new pdfi::PDFIHybridAdaptor( _rxContext ));
    }

    Reference< XInterface > Create_PDFIRawAdaptor_Writer( const Reference< XComponentContext >& _rxContext )
    {
        pdfi::PDFIRawAdaptor* pAdaptor = new pdfi::PDFIRawAdaptor( "org.libreoffice.comp.documents.WriterPDFImport", _rxContext );

        pAdaptor->setTreeVisitorFactory(pdfi::createWriterTreeVisitorFactory());
        pAdaptor->enableToplevelText(); // TEMP! TEMP!

        return uno::Reference<uno::XInterface>(static_cast<xml::XImportFilter*>(pAdaptor));
    }

    Reference< XInterface > Create_PDFIRawAdaptor_Draw( const Reference< XComponentContext >& _rxContext )
    {
        pdfi::PDFIRawAdaptor* pAdaptor = new pdfi::PDFIRawAdaptor( "org.libreoffice.comp.documents.DrawPDFImport", _rxContext );

        pAdaptor->setTreeVisitorFactory(pdfi::createDrawTreeVisitorFactory());

        return uno::Reference<uno::XInterface>(static_cast<xml::XImportFilter*>(pAdaptor));
    }

    Reference< XInterface > Create_PDFIRawAdaptor_Impress( const Reference< XComponentContext >& _rxContext )
    {
        pdfi::PDFIRawAdaptor* pAdaptor = new pdfi::PDFIRawAdaptor( "org.libreoffice.comp.documents.ImpressPDFImport", _rxContext );

        pAdaptor->setTreeVisitorFactory(pdfi::createImpressTreeVisitorFactory());

        return uno::Reference<uno::XInterface>(static_cast<xml::XImportFilter*>(pAdaptor));
    }

    Reference< XInterface > Create_PDFDetector( const Reference< XComponentContext >& _rxContext )
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
            :pAsciiServiceName( nullptr )
            ,pAsciiImplementationName( nullptr )
            ,pFactory( nullptr )
        {
        }
        ComponentDescription( const sal_Char* _pAsciiServiceName, const sal_Char* _pAsciiImplementationName, ComponentFactory _pFactory )
            :pAsciiServiceName( _pAsciiServiceName )
            ,pAsciiImplementationName( _pAsciiImplementationName )
            ,pFactory( _pFactory )
        {
        }
    };

    const ComponentDescription* lcl_getComponents()
    {
        static const ComponentDescription aDescriptions[] = {
            ComponentDescription( "com.sun.star.document.ImportFilter", "org.libreoffice.comp.documents.HybridPDFImport", Create_PDFIHybridAdaptor ),
            ComponentDescription( "com.sun.star.document.ImportFilter", "org.libreoffice.comp.documents.WriterPDFImport", Create_PDFIRawAdaptor_Writer ),
            ComponentDescription( "com.sun.star.document.ImportFilter", "org.libreoffice.comp.documents.DrawPDFImport", Create_PDFIRawAdaptor_Draw ),
            ComponentDescription( "com.sun.star.document.ImportFilter", "org.libreoffice.comp.documents.ImpressPDFImport", Create_PDFIRawAdaptor_Impress ),
            ComponentDescription( "com.sun.star.document.ImportFilter", "org.libreoffice.comp.documents.PDFDetector", Create_PDFDetector ),
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
    OUString sImplementationName( OUString::createFromAscii( pImplementationName ) );

    Reference< XSingleComponentFactory > xFactory;

    const ComponentDescription* pComponents = lcl_getComponents();
    while ( pComponents->pAsciiServiceName != nullptr )
    {
        if ( sImplementationName.equalsAscii( pComponents->pAsciiImplementationName ) )
        {
            Sequence< OUString > sServices(1);
            sServices[0] = OUString::createFromAscii( pComponents->pAsciiServiceName );

            xFactory = ::cppu::createSingleComponentFactory(
                pComponents->pFactory,
                sImplementationName,
                sServices
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
