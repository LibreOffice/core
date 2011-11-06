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
#include "precompiled_toolkit.hxx"
#include <toolkit/helper/tkresmgr.hxx>
#include <tools/simplerm.hxx>
#include <comphelper/processfactory.hxx>
#include <comphelper/componentcontext.hxx>
#include <comphelper/namedvaluecollection.hxx>
#include <com/sun/star/graphic/XGraphicProvider.hpp>
#ifndef  _TOOLS_RESMGR_HXX_
#include <tools/resmgr.hxx>
#endif
#include <tools/diagnose_ex.h>

#include <vcl/svapp.hxx>

using ::com::sun::star::uno::Reference;
using ::com::sun::star::graphic::XGraphic;
using ::com::sun::star::graphic::XGraphicProvider;
using namespace ::com::sun::star;
// -----------------------------------------------------------------------------
// TkResMgr
// -----------------------------------------------------------------------------

SimpleResMgr*   TkResMgr::m_pSimpleResMgr = NULL;
ResMgr*         TkResMgr::m_pResMgr = NULL;

// -----------------------------------------------------------------------------

TkResMgr::EnsureDelete::~EnsureDelete()
{
    delete TkResMgr::m_pSimpleResMgr;
//    delete TkResMgr::m_pResMgr;
}

// -----------------------------------------------------------------------------

void TkResMgr::ensureImplExists()
{
    if (m_pSimpleResMgr)
        return;

    ::com::sun::star::lang::Locale aLocale = Application::GetSettings().GetUILocale();

    ByteString sResMgrName( "tk" );

    m_pSimpleResMgr = SimpleResMgr::Create( sResMgrName.GetBuffer(), aLocale );
    m_pResMgr = ResMgr::CreateResMgr( sResMgrName.GetBuffer() );

    if (m_pSimpleResMgr)
    {
        // now that we have a impl class, make sure it's deleted on unloading the library
        static TkResMgr::EnsureDelete s_aDeleteTheImplClass;
    }
}

// -----------------------------------------------------------------------------
::rtl::OUString TkResMgr::loadString( sal_uInt16 nResId )
{
    ::rtl::OUString sReturn;

    ensureImplExists();
    if ( m_pSimpleResMgr )
        sReturn = m_pSimpleResMgr->ReadString( nResId );

    return sReturn;
}

// -----------------------------------------------------------------------------
Image TkResMgr::loadImage( sal_uInt16 nResId )
{
    Image aReturn;

    ensureImplExists();
    if ( m_pResMgr )
        aReturn = Image( ResId( nResId, *m_pResMgr ) );

    return aReturn;
}

// -----------------------------------------------------------------------------
Image TkResMgr::getImageFromURL( const ::rtl::OUString& i_rImageURL )
{
    if ( !i_rImageURL.getLength() )
        return Image();

    try
    {
        ::comphelper::ComponentContext aContext( ::comphelper::getProcessServiceFactory() );
        Reference< XGraphicProvider > xProvider;
        if ( aContext.createComponent( "com.sun.star.graphic.GraphicProvider", xProvider ) )
        {
            ::comphelper::NamedValueCollection aMediaProperties;
            aMediaProperties.put( "URL", i_rImageURL );
            Reference< XGraphic > xGraphic = xProvider->queryGraphic( aMediaProperties.getPropertyValues() );
            return Image( xGraphic );
        }
    }
    catch( const uno::Exception& )
    {
        DBG_UNHANDLED_EXCEPTION();
    }
    return Image();
}

