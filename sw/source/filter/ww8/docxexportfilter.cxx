/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
 */

#include "docxexportfilter.hxx"
#include "rtfexportfilter.hxx"
#include "docxexport.hxx"

#include <docsh.hxx>
#include <editsh.hxx>
#include <pam.hxx>
#include <unotxdoc.hxx>

#include <cppuhelper/implementationentry.hxx>

using namespace ::comphelper;
using namespace ::com::sun::star;

DocxExportFilter::DocxExportFilter( const uno::Reference< uno::XComponentContext >& xContext )
    : oox::core::XmlFilterBase( xContext )
{
}

bool DocxExportFilter::exportDocument()
{
    OSL_TRACE( "DocxExportFilter::exportDocument()\n" ); 

    
    uno::Reference< uno::XInterface > xIfc( getModel(), uno::UNO_QUERY );
    SwXTextDocument *pTxtDoc = dynamic_cast< SwXTextDocument * >( xIfc.get() );
    if ( !pTxtDoc )
        return false;

    SwDoc *pDoc = pTxtDoc->GetDocShell()->GetDoc();
    if ( !pDoc )
        return false;

    
    SwViewShell* pViewShell = NULL;
    pDoc->GetEditShell(&pViewShell);
    if (pViewShell != NULL)
        pViewShell->CalcLayout();

    
    
    
    
    SwPaM aPam( pDoc->GetNodes().GetEndOfContent() );
    aPam.SetMark();
    aPam.Move( fnMoveBackward, fnGoDoc );

    SwPaM *pCurPam = new SwPaM( *aPam.End(), *aPam.Start() );

    
    
    {
        DocxExport aExport( this, pDoc, pCurPam, &aPam );
        aExport.ExportDocument( true ); 
    }

    commitStorage();

    
    if ( pCurPam )
    {
        while ( pCurPam->GetNext() != pCurPam )
            delete pCurPam->GetNext();
        delete pCurPam;
    }

    return true;
}



#define IMPL_NAME "com.sun.star.comp.Writer.DocxExport"

OUString DocxExport_getImplementationName()
{
    return OUString( IMPL_NAME );
}

OUString DocxExportFilter::implGetImplementationName() const
{
    return DocxExport_getImplementationName();
}

uno::Sequence< OUString > SAL_CALL DocxExport_getSupportedServiceNames() throw()
{
    const OUString aServiceName( "com.sun.star.document.ExportFilter" );
    const uno::Sequence< OUString > aSeq( &aServiceName, 1 );
    return aSeq;
}

uno::Reference< uno::XInterface > SAL_CALL DocxExport_createInstance(const uno::Reference< uno::XComponentContext > & xCtx ) throw( uno::Exception )
{
    return (cppu::OWeakObject*) new DocxExportFilter( xCtx );
}

extern "C"
{

::cppu::ImplementationEntry entries [] =
{
    {
        DocxExport_createInstance, DocxExport_getImplementationName,
        DocxExport_getSupportedServiceNames, ::cppu::createSingleComponentFactory,
        0, 0
    },
    {
        RtfExport_createInstance, RtfExport_getImplementationName,
        RtfExport_getSupportedServiceNames, ::cppu::createSingleComponentFactory,
        0, 0
    },
    { 0, 0, 0, 0, 0, 0 }
};

SAL_DLLPUBLIC_EXPORT void* SAL_CALL msword_component_getFactory( const sal_Char* pImplName, void* pServiceManager, void* pRegistryKey )
{
    return ::cppu::component_getFactoryHelper( pImplName, pServiceManager, pRegistryKey, entries );
}

} 

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
