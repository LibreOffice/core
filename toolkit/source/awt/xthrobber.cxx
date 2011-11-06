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
#include "toolkit/awt/xthrobber.hxx"
#include "toolkit/helper/property.hxx"
#include <toolkit/helper/tkresmgr.hxx>

#ifndef _TOOLKIT_AWT_XTHROBBER_HRC_
#include "xthrobber.hrc"
#endif
#include <tools/debug.hxx>
#include <vcl/fixed.hxx>
#include <vcl/timer.hxx>
#include <vcl/throbber.hxx>

//........................................................................
namespace toolkit
{
//........................................................................

    using namespace ::com::sun::star;

    //====================================================================
    //= XThrobber
    //====================================================================
    DBG_NAME( XThrobber )

    //--------------------------------------------------------------------
    XThrobber::XThrobber()
    {
        DBG_CTOR( XThrobber, NULL );
    }

    //--------------------------------------------------------------------
    XThrobber::~XThrobber()
    {
        DBG_DTOR( XThrobber, NULL );
    }

    //--------------------------------------------------------------------
    void SAL_CALL XThrobber::start() throw ( uno::RuntimeException )
    {
        ::vos::OGuard aGuard( GetMutex() );
        Throbber* pThrobber( dynamic_cast< Throbber* >( GetWindow() ) );
        if ( pThrobber != NULL)
            pThrobber->start();
    }

    //--------------------------------------------------------------------
    void SAL_CALL XThrobber::stop() throw ( uno::RuntimeException )
    {
        ::vos::OGuard aGuard( GetMutex() );
        Throbber* pThrobber( dynamic_cast< Throbber* >( GetWindow() ) );
        if ( pThrobber != NULL)
            pThrobber->stop();
    }

    //--------------------------------------------------------------------
    void XThrobber::SetWindow( Window* pWindow )
    {
        XThrobber_Base::SetWindow( pWindow );
        InitImageList();
    }

    //--------------------------------------------------------------------
    void SAL_CALL XThrobber::InitImageList()
        throw( uno::RuntimeException )
    {
        ::vos::OGuard aGuard( GetMutex() );

        Throbber* pThrobber( dynamic_cast< Throbber* >( GetWindow() ) );
        if ( pThrobber == NULL)
            return;

        uno::Sequence< uno::Reference< graphic::XGraphic > > aImageList(12);
        sal_uInt16 nIconIdStart = RID_TK_ICON_THROBBER_START;

        if ( pThrobber->GetSettings().GetStyleSettings().GetHighContrastMode() )
            nIconIdStart = RID_TK_HC_ICON_THROBBER_START;

        for ( sal_uInt16 i=0; i<12; i++ )
        {
            Image aImage = TK_RES_IMAGE( nIconIdStart + i );
            aImageList[i] = aImage.GetXGraphic();
        }

        pThrobber->setImageList( aImageList );
    }

//........................................................................
}   // namespace toolkit
//........................................................................
