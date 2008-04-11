/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: throbberimpl.cxx,v $
 * $Revision: 1.6 $
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
#include "precompiled_toolkit.hxx"
#include <toolkit/helper/throbberimpl.hxx>

#include <vcl/svapp.hxx>
#include <vcl/fixed.hxx>

//........................................................................
namespace toolkit
//........................................................................
{
    using namespace ::com::sun::star;

    //--------------------------------------------------------------------
    Throbber_Impl::Throbber_Impl( uno::Reference< VCLXWindow > xParent,
                                  sal_Int32 nStepTime,
                                  sal_Bool bRepeat )
        :mrMutex( Application::GetSolarMutex() )
    {
        mxParent = xParent;
        mbRepeat = bRepeat;
        mnStepTime = nStepTime;
        maWaitTimer.SetTimeout( mnStepTime );
        maWaitTimer.SetTimeoutHdl( LINK( this, Throbber_Impl, TimeOutHdl ) );
    }

    //--------------------------------------------------------------------
    Throbber_Impl::~Throbber_Impl()
    {
        maWaitTimer.Stop();
        mxParent = NULL;
    }

    //--------------------------------------------------------------------
    void Throbber_Impl::start() throw ( uno::RuntimeException )
    {
        ::vos::OGuard aGuard( GetMutex() );

        mnCurStep = 0;
        maWaitTimer.Start();
    }

    //--------------------------------------------------------------------
    void Throbber_Impl::stop() throw ( uno::RuntimeException )
    {
        ::vos::OGuard aGuard( GetMutex() );

        maWaitTimer.Stop();
    }

    //--------------------------------------------------------------------
    void Throbber_Impl::setImageList( const uno::Sequence< uno::Reference< graphic::XGraphic > >& rImageList )
        throw ( uno::RuntimeException )
    {
        ::vos::OGuard aGuard( GetMutex() );

        maImageList = rImageList;

        mnStepCount = maImageList.getLength();
        FixedImage* pImage = static_cast< FixedImage* >( mxParent->GetWindow() );
        if ( pImage )
            if ( mnStepCount )
                pImage->SetImage( maImageList[ 0 ] );
            else
                pImage->SetImage( Image() );
    }

    //--------------------------------------------------------------------
    void Throbber_Impl::initImage()
        throw ( uno::RuntimeException )
    {
        FixedImage* pImage = static_cast< FixedImage* >( mxParent->GetWindow() );
        if ( pImage && maImageList.getLength() )
            pImage->SetImage( maImageList[ 0 ] );
    }

    //--------------------------------------------------------------------
    sal_Bool Throbber_Impl::isHCMode()
        throw ( uno::RuntimeException )
    {
        FixedImage* pImage = static_cast< FixedImage* >( mxParent->GetWindow() );
        if ( pImage )
            return pImage->GetSettings().GetStyleSettings().GetFaceColor().IsDark();
        else
            return Application::GetSettings().GetStyleSettings().GetFaceColor().IsDark();
    }

    // -----------------------------------------------------------------------
    IMPL_LINK( Throbber_Impl, TimeOutHdl, Throbber_Impl*, EMPTYARG )
    {
        ::vos::OGuard aGuard( GetMutex() );

        FixedImage* pImage = static_cast< FixedImage* >( mxParent->GetWindow() );

        if ( !pImage || !maImageList.getLength() )
            return 0;

        if ( mnCurStep < mnStepCount - 1 )
            mnCurStep += 1;
        else
            mnCurStep = 0;

        pImage->SetImage( maImageList[ mnCurStep ] );

        return 0;
    }

//........................................................................
} // namespacetoolkit
//........................................................................

