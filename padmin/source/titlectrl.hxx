/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: titlectrl.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 16:30:29 $
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

#ifndef _PAD_TITLECTRL_HXX_
#define _PAD_TITLECTRL_HXX_

#ifndef _SV_CTRL_HXX
#include <vcl/ctrl.hxx>
#endif
#ifndef _SV_IMAGE_HXX
#include <vcl/image.hxx>
#endif

namespace padmin
{

class TitleImage : public Control
{
    Image               m_aImage;
    String              m_aText;
    Color               m_aBGColor;
    Point               m_aImagePos;
    Point               m_aTextPos;

    bool                m_bArranged;

    void arrange();
public:
    TitleImage( Window* pParent, const ResId& rResId );
    ~TitleImage();

    virtual void Paint( const Rectangle& rRect );

    void SetImage( const Image& rImage );
    const Image& GetImage() const { return m_aImage; }

    virtual void SetText( const String& rText );
    virtual String GetText() const { return m_aText; }

    void SetBackgroundColor( const Color& rColor );
    const Color& GetBackgroundColor() const { return m_aBGColor; }
};

}

#endif // _PAD_TITLECTRL_HXX_
