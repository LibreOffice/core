/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: intro.hxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 17:09:07 $
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
#ifndef _DESKTOP_INTRO_HXX
#define _DESKTOP_INTRO_HXX

// include ---------------------------------------------------------------

#ifndef _SV_WRKWIN_HXX
#include <vcl/introwin.hxx>
#endif
#ifndef _SV_BITMAP_HXX
#include <vcl/bitmap.hxx>
#endif

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
