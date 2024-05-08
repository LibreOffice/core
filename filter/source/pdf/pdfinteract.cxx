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


#include "pdfinteract.hxx"
#include "impdialog.hxx"

#include <com/sun/star/task/XInteractionRequest.hpp>
#include <com/sun/star/task/PDFExportException.hpp>
#include <comphelper/namedvaluecollection.hxx>
#include <cppuhelper/supportsservice.hxx>
#include <vcl/svapp.hxx>

PDFInteractionHandler::PDFInteractionHandler()
{
}

PDFInteractionHandler::~PDFInteractionHandler()
{
}

void SAL_CALL PDFInteractionHandler::handle( const Reference< task::XInteractionRequest >& i_xRequest )
{
    handleInteractionRequest( i_xRequest );
}

void SAL_CALL PDFInteractionHandler::initialize(const css::uno::Sequence<css::uno::Any>& rArguments)
{
    comphelper::NamedValueCollection aProperties(rArguments);
    if (aProperties.has(u"Parent"_ustr))
        aProperties.get(u"Parent"_ustr) >>= m_xParent;
}

sal_Bool SAL_CALL PDFInteractionHandler::handleInteractionRequest( const Reference< task::XInteractionRequest >& i_xRequest )
{
    bool bHandled = false;

    Any aRequest( i_xRequest->getRequest() );
    task::PDFExportException aExc;
    if( aRequest >>= aExc )
    {
        std::set< vcl::PDFWriter::ErrorCode > aCodes;
        sal_Int32 nCodes = aExc.ErrorCodes.getLength();
        for( sal_Int32 i = 0; i < nCodes; i++ )
            aCodes.insert( static_cast<vcl::PDFWriter::ErrorCode>(aExc.ErrorCodes.getConstArray()[i]) );

        ImplErrorDialog aDlg(Application::GetFrameWeld(m_xParent), aCodes);
        aDlg.run();
        bHandled = true;
    }
    return bHandled;
}



OUString SAL_CALL PDFInteractionHandler::getImplementationName()
{
    return u"com.sun.star.comp.PDF.PDFExportInteractionHandler"_ustr;
}


sal_Bool SAL_CALL PDFInteractionHandler::supportsService( const OUString& rServiceName )
{
    return cppu::supportsService( this, rServiceName );
}


css::uno::Sequence< OUString > SAL_CALL PDFInteractionHandler::getSupportedServiceNames(  )
{
    return { u"com.sun.star.filter.pdfexport.PDFExportInteractionHandler"_ustr };
}

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface*
filter_PDFExportInteractionHandler_get_implementation(
    css::uno::XComponentContext* , css::uno::Sequence<css::uno::Any> const&)
{
    return cppu::acquire(new PDFInteractionHandler());
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
