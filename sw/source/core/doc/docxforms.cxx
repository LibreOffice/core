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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sw.hxx"




#include <doc.hxx>
#include <docsh.hxx>
#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/container/XNameContainer.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/frame/XModule.hpp>
#include <com/sun/star/xforms/XModel.hpp>
#include <com/sun/star/xforms/XFormsUIHelper1.hpp>
#include <unotools/processfactory.hxx>
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
using rtl::OUString;


Reference<XNameContainer> SwDoc::getXForms() const
{
    return xXForms;
}

bool SwDoc::isXForms() const
{
    return xXForms.is();
}

Reference<XInterface> lcl_createInstance( const sal_Char* pServiceName )
{
    DBG_ASSERT( pServiceName != NULL, "no service name" );
    return utl::getProcessServiceFactory()->createInstance(
        OUString::createFromAscii( pServiceName  ) );
}

void SwDoc::initXForms( bool bCreateDefaultModel )
{
    DBG_ASSERT( ! isXForms(), "please initialize only once" );

    try
    {
        // create XForms components
        xXForms.set( lcl_createInstance( "com.sun.star.xforms.XForms" ),
                    UNO_QUERY );
        DBG_ASSERT( xXForms.is(), "can't create XForms container" );

        // change our module identifier, to be able to have a dedicated UI
        Reference< XModule > xModule;
        SwDocShell* pShell( GetDocShell() );
        if ( pShell )
            xModule = xModule.query( pShell->GetModel() );
        DBG_ASSERT( xModule.is(), "SwDoc::initXForms: no XModule at the document!" );
        if ( xModule.is() )
            xModule->setIdentifier( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.xforms.XMLFormDocument" ) ) );

        // create default model
        if( bCreateDefaultModel && xXForms.is() )
        {
            OUString sName(RTL_CONSTASCII_USTRINGPARAM("Model 1"));
            Reference<XModel> xModel(
                lcl_createInstance( "com.sun.star.xforms.Model" ),
                UNO_QUERY );
            DBG_ASSERT( xModel.is(), "no model?" );
            if( xModel.is() )
            {
                xModel->setID( sName );
                Reference<XFormsUIHelper1>( xModel, UNO_QUERY )->newInstance(
                    OUString(RTL_CONSTASCII_USTRINGPARAM("Instance 1")),
                    OUString(), sal_True );
                xModel->initialize();
                xXForms->insertByName( sName, makeAny( xModel ) );
            }
            DBG_ASSERT( xXForms->hasElements(), "can't create XForms model" );
        }

        DBG_ASSERT( isXForms(), "initialization failed" );
    }
    catch( const Exception& )
    {
        DBG_UNHANDLED_EXCEPTION();
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
