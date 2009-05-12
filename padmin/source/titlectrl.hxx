/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: titlectrl.hxx,v $
 * $Revision: 1.3 $
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

#ifndef _PAD_TITLECTRL_HXX_
#define _PAD_TITLECTRL_HXX_

#include <vcl/ctrl.hxx>
#include <vcl/image.hxx>

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
