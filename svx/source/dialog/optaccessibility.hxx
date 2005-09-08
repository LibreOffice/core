/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: optaccessibility.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 21:39:06 $
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
#ifndef _SVX_OPTACCESSIBILITY_HXX
#define _SVX_OPTACCESSIBILITY_HXX

#ifndef _SFXTABDLG_HXX //autogen
#include <sfx2/tabdlg.hxx>
#endif
#ifndef _SV_FIXED_HXX
#include <vcl/fixed.hxx>
#endif
#ifndef _SV_FIELD_HXX //autogen wg. NumericField
#include <vcl/field.hxx>
#endif
struct SvxAccessibilityOptionsTabPage_Impl;
class SvxAccessibilityOptionsTabPage : public SfxTabPage
{
    FixedLine       m_aMiscellaneousLabel;      // FL_MISCELLANEOUS
    CheckBox        m_aAccessibilityTool;       // CB_ACCESSIBILITY_TOOL
    CheckBox        m_aTextSelectionInReadonly; // CB_TEXTSELECTION
    CheckBox        m_aAnimatedGraphics;        // CB_ANIMATED_GRAPHICS
    CheckBox        m_aAnimatedTexts;           // CB_ANIMATED_TEXTS
    CheckBox        m_aTipHelpCB;               // CB_TIPHELP
    NumericField    m_aTipHelpNF;               // NF_TIPHELP
    FixedText       m_aTipHelpFT;               // FT_TIPHELP
    FixedLine       m_aHCOptionsLabel;          // FL_HC_OPTIONS
    CheckBox        m_aAutoDetectHC;            // CB_AUTO_DETECT_HC
    CheckBox        m_aAutomaticFontColor;      // CB_AUTOMATIC_FONT_COLOR
    CheckBox        m_aPagePreviews;            // CB_PAGEPREVIEWS

    DECL_LINK(TipHelpHdl, CheckBox*);
    void EnableTipHelp(sal_Bool bCheck);

    SvxAccessibilityOptionsTabPage_Impl* m_pImpl;

    SvxAccessibilityOptionsTabPage( Window* pParent, const SfxItemSet& rSet );
public:

    virtual ~SvxAccessibilityOptionsTabPage();

    static SfxTabPage*  Create( Window* pParent, const SfxItemSet& rAttrSet );
    virtual BOOL        FillItemSet( SfxItemSet& rSet );
    virtual void        Reset( const SfxItemSet& rSet );
};

#endif

