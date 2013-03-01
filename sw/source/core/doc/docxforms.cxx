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

#include <doc.hxx>
#include <docsh.hxx>
#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/container/XNameContainer.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/frame/XModule.hpp>
#include <com/sun/star/xforms/XModel.hpp>
#include <com/sun/star/xforms/XFormsUIHelper1.hpp>
#include <comphelper/processfactory.hxx>
#include <tools/diagnose_ex.h>


using namespace ::com::sun::star;

using uno::Reference;
using uno::XInterface;
using uno::UNO_QUERY;
using uno::makeAny;
using uno::Exception;
using container::XNameContainer;
using xforms::XModel;
using frame::XModule;
using xforms::XFormsUIHelper1;
using ::rtl::OUString;


Reference<XNameContainer> SwDoc::getXForms() const
{
    return mxXForms;
}

bool SwDoc::isXForms() const
{
    return mxXForms.is();
}

static Reference<XInterface> lcl_createInstance( const sal_Char* pServiceName )
{
    OSL_ENSURE( pServiceName != NULL, "no service name" );
    return comphelper::getProcessServiceFactory()->createInstance(
        OUString::createFromAscii( pServiceName  ) );
}

void SwDoc::initXForms( bool bCreateDefaultModel )
{
    OSL_ENSURE( ! isXForms(), "please initialize only once" );

    try
    {
        // create XForms components
        mxXForms.set( lcl_createInstance( "com.sun.star.xforms.XForms" ),
                    UNO_QUERY );
        OSL_ENSURE( mxXForms.is(), "can't create XForms container" );

        // change our module identifier, to be able to have a dedicated UI
        Reference< XModule > xModule;
        SwDocShell* pShell( GetDocShell() );
        if ( pShell )
            xModule = xModule.query( pShell->GetModel() );
        OSL_ENSURE( xModule.is(), "SwDoc::initXForms: no XModule at the document!" );
        if ( xModule.is() )
            xModule->setIdentifier( OUString( "com.sun.star.xforms.XMLFormDocument" ) );

        // create default model
        if( bCreateDefaultModel && mxXForms.is() )
        {
            OUString sName("Model 1");
            Reference<XModel> xModel(
                lcl_createInstance( "com.sun.star.xforms.Model" ),
                UNO_QUERY );
            OSL_ENSURE( xModel.is(), "no model?" );
            if( xModel.is() )
            {
                xModel->setID( sName );
                Reference<XFormsUIHelper1>( xModel, UNO_QUERY )->newInstance(
                    OUString("Instance 1"),
                    OUString(), sal_True );
                xModel->initialize();
                mxXForms->insertByName( sName, makeAny( xModel ) );
            }
            OSL_ENSURE( mxXForms->hasElements(), "can't create XForms model" );
        }

        OSL_ENSURE( isXForms(), "initialization failed" );
    }
    catch( const Exception& )
    {
        DBG_UNHANDLED_EXCEPTION();
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
