/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/


#ifndef _SVX_FNTCTRL_HXX
#define _SVX_FNTCTRL_HXX

// include ---------------------------------------------------------------

#include <vcl/window.hxx>
#include <editeng/svxfont.hxx>
#include "svx/svxdllapi.h"

#ifndef _RTL_USTRING_HXX_
#include <rtl/ustring.hxx>
#endif

// forward ---------------------------------------------------------------

class FontPrevWin_Impl;

// class SvxFontPrevWindow -----------------------------------------------

class SVX_DLLPUBLIC SvxFontPrevWindow : public Window
{
    using OutputDevice::SetFont;
private:
    FontPrevWin_Impl*   pImpl;

    SVX_DLLPRIVATE void             InitSettings( sal_Bool bForeground, sal_Bool bBackground );

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
    void                ResetColor();
    void                SetBackColor( const Color& rColor );
    void                UseResourceText( sal_Bool bUse = sal_True );
    void                Paint( const Rectangle& );

    sal_Bool                IsTwoLines() const;
    void                SetTwoLines(sal_Bool bSet);

    void                SetBrackets(sal_Unicode cStart, sal_Unicode cEnd);

    void                SetFontWidthScale( sal_uInt16 nScaleInPercent );

    void                AutoCorrectFontColor( void );

    void                SetPreviewText( const ::rtl::OUString& rString );
    void                SetFontNameAsPreviewText();
};

#endif // #ifndef _SVX_FNTCTRL_HXX

