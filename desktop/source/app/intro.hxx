/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: intro.hxx,v $
 * $Revision: 1.7 $
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
#ifndef _DESKTOP_INTRO_HXX
#define _DESKTOP_INTRO_HXX

// include ---------------------------------------------------------------

#ifndef _SV_WRKWIN_HXX
#include <vcl/introwin.hxx>
#endif
#include <vcl/bitmap.hxx>

// class IntroWindow_Impl ------------------------------------------------
namespace desktop
{

class IntroWindow_Impl : public IntroWindow
{
private:
    Bitmap          m_aIntroBmp;

    void            Init();
    DECL_LINK( AppEventListenerHdl, VclWindowEvent * );

public:
    IntroWindow_Impl( const Bitmap& aInfoBitmap );
    ~IntroWindow_Impl();

    virtual void    Paint( const Rectangle& );

    void            Slide();
};

}

#endif // #ifndef _DESKTOP_INTRO_HXX
