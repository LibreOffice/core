/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: throbberimpl.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: obo $ $Date: 2007-01-23 08:04:37 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/
#include "precompiled_toolkit.hxx"
#ifndef  _TOOLKIT_HELPER_THROBBERIMPL_HXX_
#include <toolkit/helper/throbberimpl.hxx>
#endif

#include <vcl/svapp.hxx>
#include <vcl/fixed.hxx>

//........................................................................
namespace toolkit
//........................................................................
{
    using namespace ::com::sun::star;
    using namespace ::com::sun::star::graphic;
    using namespace ::com::sun::star::uno;

    //--------------------------------------------------------------------
    Throbber_Impl::Throbber_Impl( Reference< VCLXWindow > xParent,
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
    void Throbber_Impl::start() throw (RuntimeException)
    {
        ::vos::OGuard aGuard( GetMutex() );

        mnCurStep = 0;
        maWaitTimer.Start();
    }

    //--------------------------------------------------------------------
    void Throbber_Impl::stop() throw (RuntimeException)
    {
        ::vos::OGuard aGuard( GetMutex() );

        maWaitTimer.Stop();
    }

    //--------------------------------------------------------------------
    void Throbber_Impl::setImageList( const Sequence< Reference< XGraphic > >& rImageList )
        throw (RuntimeException)
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
        throw (RuntimeException)
    {
        FixedImage* pImage = static_cast< FixedImage* >( mxParent->GetWindow() );
        if ( pImage )
            pImage->SetImage( maImageList[ 0 ] );
    }

    //--------------------------------------------------------------------
    sal_Bool Throbber_Impl::isHCMode()
        throw (RuntimeException)
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

        if ( !pImage )
            return 0;

        if ( mnCurStep < 11 )
            mnCurStep += 1;
        else
            mnCurStep = 0;

        pImage->SetImage( maImageList[ mnCurStep ] );

        return 0;
    }

//........................................................................
} // namespacetoolkit
//........................................................................

