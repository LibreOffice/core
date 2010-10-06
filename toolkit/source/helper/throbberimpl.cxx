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
#include "precompiled_toolkit.hxx"
#include <toolkit/helper/throbberimpl.hxx>

#include <vcl/svapp.hxx>
#include <vcl/imgctrl.hxx>

//........................................................................
namespace toolkit
//........................................................................
{
    using namespace ::com::sun::star;

    //--------------------------------------------------------------------
    Throbber_Impl::Throbber_Impl( VCLXWindow& i_rParent )
        :mrMutex( Application::GetSolarMutex() )
        ,mrParent( i_rParent )
        ,mbRepeat( sal_True )
        ,mnStepTime( 100 )
        ,mnCurStep( 0 )
        ,mnStepCount( 0 )
    {
        maWaitTimer.SetTimeout( mnStepTime );
        maWaitTimer.SetTimeoutHdl( LINK( this, Throbber_Impl, TimeOutHdl ) );
    }

    //--------------------------------------------------------------------
    Throbber_Impl::~Throbber_Impl()
    {
        maWaitTimer.Stop();
    }

    //--------------------------------------------------------------------
    void Throbber_Impl::start()
    {
        DBG_TESTSOLARMUTEX();
        maWaitTimer.Start();
    }

    //--------------------------------------------------------------------
    void Throbber_Impl::stop()
    {
        DBG_TESTSOLARMUTEX();
        maWaitTimer.Stop();
    }

    //--------------------------------------------------------------------
    bool Throbber_Impl::isRunning() const
    {
        DBG_TESTSOLARMUTEX();
        return maWaitTimer.IsActive();
    }

    //--------------------------------------------------------------------
    namespace
    {
        void lcl_setImage( FixedImage& i_fixedImage, Image const& i_image )
        {
            ImageControl* pImageControl = dynamic_cast< ImageControl* >( &i_fixedImage );
            if ( pImageControl != NULL )
                pImageControl->SetBitmap( i_image.GetBitmapEx() );
            else
                i_fixedImage.SetImage( i_image );
        }
    }

    //--------------------------------------------------------------------
    void Throbber_Impl::setImageList( const uno::Sequence< uno::Reference< graphic::XGraphic > >& rImageList )
    {
        DBG_TESTSOLARMUTEX();

        maImageList = rImageList;

        mnStepCount = maImageList.getLength();
        FixedImage* pFixedImage = dynamic_cast< FixedImage* >( mrParent.GetWindow() );
        if ( pFixedImage )
            lcl_setImage( *pFixedImage, mnStepCount ? maImageList[ 0 ] : Image() );
    }

    //--------------------------------------------------------------------
    sal_Bool Throbber_Impl::isHCMode()
    {
        FixedImage* pImage = dynamic_cast< FixedImage* >( mrParent.GetWindow() );
        if ( pImage )
            return pImage->GetSettings().GetStyleSettings().GetHighContrastMode();
        else
            return Application::GetSettings().GetStyleSettings().GetHighContrastMode();
    }

    // -----------------------------------------------------------------------
    IMPL_LINK( Throbber_Impl, TimeOutHdl, Throbber_Impl*, EMPTYARG )
    {
        ::vos::OGuard aGuard( GetMutex() );
        if ( !maImageList.getLength() )
            return 0;

        FixedImage* pFixedImage = dynamic_cast< FixedImage* >( mrParent.GetWindow() );
        if ( pFixedImage == NULL )
            return 0;

        if ( mnCurStep < mnStepCount - 1 )
            mnCurStep += 1;
        else
        {
            if ( mbRepeat )
            {
                // start over
                mnCurStep = 0;
            }
            else
            {
                stop();
            }
        }

        lcl_setImage( *pFixedImage, maImageList[ mnCurStep ] );

        return 0;
    }

//........................................................................
} // namespacetoolkit
//........................................................................

