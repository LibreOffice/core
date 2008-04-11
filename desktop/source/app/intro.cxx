/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: intro.cxx,v $
 * $Revision: 1.10 $
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
#include "precompiled_desktop.hxx"


#include "intro.hxx"

#include <tools/stream.hxx>
#include <tools/urlobj.hxx>
#include <vos/process.hxx>
#include <rtl/logfile.hxx>
#include <vcl/svapp.hxx>

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
