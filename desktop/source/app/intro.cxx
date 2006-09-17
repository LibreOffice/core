/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: intro.cxx,v $
 *
 *  $Revision: 1.9 $
 *
 *  last change: $Author: obo $ $Date: 2006-09-17 09:36:33 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_desktop.hxx"


#include "intro.hxx"

#include <tools/stream.hxx>
#include <tools/urlobj.hxx>
#include <vos/process.hxx>
#include <rtl/logfile.hxx>
#ifndef _SV_SVAPP_HXX
#include <vcl/svapp.hxx>
#endif

namespace desktop
{

// -----------------------------------------------------------------------

void IntroWindow_Impl::Init()
{
    RTL_LOGFILE_CONTEXT( aLog, "desktop (cd100003) ::IntroWindow_Impl::Init" );

    Size aSize = m_aIntroBmp.GetSizePixel();
    SetOutputSizePixel( aSize );

    if ( GetColorCount() >= 16 )
    {
        Show();
        Update();
    }
}

// -----------------------------------------------------------------------

IMPL_LINK( IntroWindow_Impl, AppEventListenerHdl, VclWindowEvent *, inEvent )
{
    if ( inEvent != 0 )
    {
        switch ( inEvent->GetId() )
        {
            case VCLEVENT_WINDOW_HIDE:
                Paint( Rectangle() );
                break;

            default:
                break;
        }
    }
    return 0;
}

// -----------------------------------------------------------------------

IntroWindow_Impl::IntroWindow_Impl( const Bitmap& aIntroBitmap ) :
    IntroWindow(),
    m_aIntroBmp( aIntroBitmap )
{
    RTL_LOGFILE_CONTEXT( aLog, "desktop (cd100003) ::IntroWindow_Impl::IntroWindowImpl" );

    Hide();

    Init();
    Application::AddEventListener(
        LINK( this, IntroWindow_Impl, AppEventListenerHdl ) );
}

// -----------------------------------------------------------------------

IntroWindow_Impl::~IntroWindow_Impl()
{
    Application::RemoveEventListener(
        LINK( this, IntroWindow_Impl, AppEventListenerHdl ) );
    Hide();
}

// -----------------------------------------------------------------------

void IntroWindow_Impl::Paint( const Rectangle& )
{
    DrawBitmap( Point(), m_aIntroBmp );
    Flush();
}

// -----------------------------------------------------------------------

void IntroWindow_Impl::Slide()
{
}

}
