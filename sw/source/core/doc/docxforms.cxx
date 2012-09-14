/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



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
#include <com/sun/star/container/XIndexAccess.hpp>

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
using com::sun::star::container::XIndexAccess;

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

//
// #i113606#, to release the cyclic reference between XFormModel and bindings/submissions.
//
void SwDoc::disposeXForms( )
{
    // get XForms models
    if( xXForms.is() )
    {
        // iterate over all models
        uno::Sequence<OUString> aNames = xXForms->getElementNames();
        const OUString* pNames = aNames.getConstArray();
        sal_Int32 nNames = aNames.getLength();
        for( sal_Int32 n = 0; (n < nNames); n++ )
        {
            Reference< xforms::XModel > xModel(
                xXForms->getByName( pNames[n] ), UNO_QUERY );

            if( xModel.is() )
            {
                // ask model for bindings
                Reference< XIndexAccess > xBindings(
                         xModel->getBindings(), UNO_QUERY );

                //
                // Then release them one by one
                //
                int nCount = xBindings->getCount();
                for( int i = nCount-1; i >= 0; i-- )
                {
                    xModel->getBindings()->remove(xBindings->getByIndex( i ));
                }

                // ask model for Submissions
                Reference< XIndexAccess > xSubmissions(
                         xModel->getSubmissions(), UNO_QUERY );

                //
                // Then release them one by one
                //
                nCount = xSubmissions->getCount();
                for( int i = nCount-1; i >= 0; i-- )
                {
                    xModel->getSubmissions()->remove(xSubmissions->getByIndex( i ));
                }
            }
        }
    }
}
