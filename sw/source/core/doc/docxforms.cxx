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
#include <com/sun/star/xforms/Model.hpp>
#include <com/sun/star/xforms/XModel2.hpp>
#include <com/sun/star/xforms/XFormsUIHelper1.hpp>
#include <com/sun/star/xforms/XForms.hpp>
#include <comphelper/processfactory.hxx>
#include <tools/diagnose_ex.h>
#include <com/sun/star/container/XIndexAccess.hpp>

using namespace ::com::sun::star;

using uno::Reference;
using uno::XInterface;
using uno::UNO_QUERY;
using uno::makeAny;
using uno::Exception;
using container::XNameContainer;
using xforms::XModel2;
using frame::XModule;
using xforms::XFormsUIHelper1;
using com::sun::star::container::XIndexAccess;

Reference<XNameContainer> SwDoc::getXForms() const
{
    return mxXForms;
}

bool SwDoc::isXForms() const
{
    return mxXForms.is();
}

void SwDoc::initXForms( bool bCreateDefaultModel )
{
    OSL_ENSURE( ! isXForms(), "please initialize only once" );

    try
    {
        // create XForms components
        mxXForms = xforms::XForms::create( comphelper::getProcessComponentContext() );

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
            Reference<XModel2> xModel = xforms::Model::create( comphelper::getProcessComponentContext() );
            xModel->setID( sName );
            Reference<XFormsUIHelper1>( xModel, uno::UNO_QUERY_THROW )->newInstance(
                OUString("Instance 1"),
                OUString(), sal_True );
            xModel->initialize();
            mxXForms->insertByName( sName, makeAny( xModel ) );
            OSL_ENSURE( mxXForms->hasElements(), "can't create XForms model" );
        }

        OSL_ENSURE( isXForms(), "initialization failed" );
    }
    catch( const Exception& )
    {
        DBG_UNHANDLED_EXCEPTION();
    }
}

// #i113606#, to release the cyclic reference between XFormModel and bindings/submissions.
void SwDoc::disposeXForms( )
{
    // get XForms models
    if( mxXForms.is() )
    {
        // iterate over all models
        uno::Sequence<OUString> aNames = mxXForms->getElementNames();
        const OUString* pNames = aNames.getConstArray();
        sal_Int32 nNames = aNames.getLength();
        for( sal_Int32 n = 0; (n < nNames); n++ )
        {
            Reference< xforms::XModel > xModel(
                mxXForms->getByName( pNames[n] ), UNO_QUERY );

            if( xModel.is() )
            {
                // ask model for bindings
                Reference< XIndexAccess > xBindings(
                         xModel->getBindings(), UNO_QUERY );

                // Then release them one by one
                int nCount = xBindings->getCount();
                for( int i = nCount-1; i >= 0; i-- )
                {
                    xModel->getBindings()->remove(xBindings->getByIndex( i ));
                }

                // ask model for Submissions
                Reference< XIndexAccess > xSubmissions(
                         xModel->getSubmissions(), UNO_QUERY );

                // Then release them one by one
                nCount = xSubmissions->getCount();
                for( int i = nCount-1; i >= 0; i-- )
                {
                    xModel->getSubmissions()->remove(xSubmissions->getByIndex( i ));
                }
            }
        }
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
