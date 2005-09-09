/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: htmlattr.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 03:21:33 $
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

#ifndef _SD_HTMLATTR_HXX
#define _SD_HTMLATTR_HXX

#ifndef _SV_CTRL_HXX
#include <vcl/ctrl.hxx>
#endif
#ifndef _TOOLS_COLOR_HXX
#include <tools/color.hxx>
#endif

class SdHtmlAttrPreview : public Control
{
protected:

    Color   m_aBackColor, m_aTextColor, m_aLinkColor;
    Color   m_aVLinkColor, m_aALinkColor;

public:
    SdHtmlAttrPreview( Window* pParent, const ResId& rResId );
    ~SdHtmlAttrPreview();

    virtual void    Paint( const Rectangle& rRect );

    void    SetColors( Color& aBack, Color& aText, Color& aLink,
                       Color& aVLink, Color& aALink );
    void    GetColors( Color& aBack, Color& aText, Color& aLink,
                       Color& aVLink, Color& aALink ) const;
};

#endif // _SD_HTMLATTR_HXX
