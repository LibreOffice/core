/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: fntctrl.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2007-04-11 15:52:50 $
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
#ifndef _SVX_FNTCTRL_HXX
#define _SVX_FNTCTRL_HXX

// include ---------------------------------------------------------------

#ifndef _WINDOW_HXX //autogen
#include <vcl/window.hxx>
#endif
#ifndef _SVX_SVXFONT_HXX //autogen
#include <svx/svxfont.hxx>
#endif

#ifndef INCLUDED_SVXDLLAPI_H
#include "svx/svxdllapi.h"
#endif

// forward ---------------------------------------------------------------

class FontPrevWin_Impl;

// class SvxFontPrevWindow -----------------------------------------------

class SVX_DLLPUBLIC SvxFontPrevWindow : public Window
{
    using OutputDevice::SetFont;
private:
    FontPrevWin_Impl*   pImpl;

    SVX_DLLPRIVATE void             InitSettings( BOOL bForeground, BOOL bBackground );

public:
                        SvxFontPrevWindow( Window* pParent, const ResId& rId );
    virtual             ~SvxFontPrevWindow();

    virtual void        StateChanged( StateChangedType nStateChange );
    virtual void        DataChanged( const DataChangedEvent& rDCEvt );

    // Aus Effizienz-gr"unden nicht const
    SvxFont&            GetFont();
    const SvxFont&      GetFont() const;
    void                SetFont( const SvxFont& rFont );
    void                SetFont( const SvxFont& rNormalFont, const SvxFont& rCJKFont, const SvxFont& rCTLFont );
    void                SetCJKFont( const SvxFont& rFont );
    void                SetCTLFont( const SvxFont& rFont );
    SvxFont&            GetCJKFont();
    SvxFont&            GetCTLFont();
    void                SetColor( const Color& rColor );
    void                SetBackColor( const Color& rColor );
    void                UseResourceText( BOOL bUse = TRUE );
    void                Paint( const Rectangle& );

    BOOL                IsTwoLines() const;
    void                SetTwoLines(BOOL bSet);

    void                SetBrackets(sal_Unicode cStart, sal_Unicode cEnd);

    void                SetFontWidthScale( UINT16 nScaleInPercent );

    void                AutoCorrectFontColor( void );
};

#endif // #ifndef _SVX_FNTCTRL_HXX

