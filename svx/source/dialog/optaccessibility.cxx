/*************************************************************************
 *
 *  $RCSfile: optaccessibility.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: obo $ $Date: 2004-07-05 09:27:00 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#pragma hdrstop

#ifndef _SVX_OPTACCESSIBILITY_HXX
#include <optaccessibility.hxx>
#endif
#include <optaccessibility.hrc>
#include <dialmgr.hxx>
#include "dialogs.hrc"
#ifndef INCLUDED_SVTOOLS_ACCESSIBILITYOPTIONS_HXX
#include <svtools/accessibilityoptions.hxx>
#endif
#ifndef _SV_SETTINGS_HXX
#include <vcl/settings.hxx>
#endif
#ifndef _SV_SVAPP_HXX
#include <vcl/svapp.hxx>
#endif

static void MovePosY( Window& _rWin, long _nDelta )
{
    Point   aPoint = _rWin.GetPosPixel();
    aPoint.Y() += _nDelta;

    _rWin.SetPosPixel( aPoint );
}

struct SvxAccessibilityOptionsTabPage_Impl
{
    SvtAccessibilityOptions     m_aConfig;
    SvxAccessibilityOptionsTabPage_Impl()
            : m_aConfig(){}
};

SvxAccessibilityOptionsTabPage::SvxAccessibilityOptionsTabPage( Window* pParent, const SfxItemSet& rSet )
    :SfxTabPage(pParent, ResId( RID_SVXPAGE_ACCESSIBILITYCONFIG, DIALOG_MGR() ), rSet)
    ,m_aMiscellaneousLabel      (this, ResId(FL_MISCELLANEOUS       ))
    ,m_aAccessibilityTool       (this, ResId(CB_ACCESSIBILITY_TOOL  ))
    ,m_aTextSelectionInReadonly (this, ResId(CB_TEXTSELECTION       ))
    ,m_aAnimatedGraphics        (this, ResId(CB_ANIMATED_GRAPHICS   ))
    ,m_aAnimatedTexts           (this, ResId(CB_ANIMATED_TEXTS      ))
    ,m_aTipHelpCB               (this, ResId(CB_TIPHELP             ))
    ,m_aTipHelpNF               (this, ResId(NF_TIPHELP             ))
    ,m_aTipHelpFT               (this, ResId(FT_TIPHELP             ))
    ,m_aHCOptionsLabel          (this, ResId(FL_HC_OPTIONS          ))
    ,m_aAutoDetectHC            (this, ResId(CB_AUTO_DETECT_HC      ))
    ,m_aAutomaticFontColor      (this, ResId(CB_AUTOMATIC_FONT_COLOR))
    ,m_aPagePreviews            (this, ResId(CB_PAGE_PREVIEWS       ))
    ,m_pImpl(new SvxAccessibilityOptionsTabPage_Impl)
{
    FreeResource();
    m_aTipHelpCB.SetClickHdl(LINK(this, SvxAccessibilityOptionsTabPage, TipHelpHdl));

    long nHeightDelta = 0;      // to correct positions _under_ m_aAccessibilityTool

#ifdef UNX
    {
        // UNIX: read the gconf2 setting instead to use the checkbox
        m_aAccessibilityTool.Hide();
        nHeightDelta = -( ROWA_2 - ROWA_1 );
    }
#else
    // calculate the height of the checkbox. Do we need two (default in resource) or only one line
    String aText = m_aAccessibilityTool.GetText();
    long nWidth = m_aAccessibilityTool.GetTextWidth( aText );
    long nCtrlWidth = m_aAccessibilityTool.GetSizePixel().Width() - ( COL2 - COL1 );
    if ( nWidth > nCtrlWidth )
    {
        long nDelta = 2 * RSC_CD_FIXEDLINE_HEIGHT + LINESPACE - RSC_CD_CHECKBOX_HEIGHT;
        nHeightDelta = nDelta;
        Size aSize = m_aAccessibilityTool.LogicToPixel( Size( 0, nDelta ), MAP_APPFONT );
        nDelta = aSize.Height();
        aSize = m_aAccessibilityTool.GetSizePixel();
        aSize.Height() += nDelta;
        m_aAccessibilityTool.SetSizePixel( aSize );
    }
#endif

    if( nHeightDelta )
    {   //adjust positions of controls under m_aAccessibilityTool
        Size aSize = m_aAccessibilityTool.LogicToPixel( Size( 0, nHeightDelta ), MAP_APPFONT );
        nHeightDelta = aSize.Height();

        MovePosY( m_aTextSelectionInReadonly, nHeightDelta );
        MovePosY( m_aAnimatedGraphics, nHeightDelta );
        MovePosY( m_aAnimatedTexts, nHeightDelta );
        MovePosY( m_aTipHelpCB, nHeightDelta );
        MovePosY( m_aTipHelpNF, nHeightDelta );
        MovePosY( m_aTipHelpFT, nHeightDelta );
        MovePosY( m_aHCOptionsLabel, nHeightDelta );
        MovePosY( m_aAutoDetectHC, nHeightDelta );
        MovePosY( m_aAutomaticFontColor, nHeightDelta );
        MovePosY( m_aPagePreviews, nHeightDelta );
    }
}

SvxAccessibilityOptionsTabPage::~SvxAccessibilityOptionsTabPage()
{
    delete m_pImpl;
}

SfxTabPage* SvxAccessibilityOptionsTabPage::Create( Window* pParent, const SfxItemSet& rAttrSet )
{
    return new SvxAccessibilityOptionsTabPage(pParent, rAttrSet);
}

BOOL SvxAccessibilityOptionsTabPage::FillItemSet( SfxItemSet& rSet )
{
    //aConfig.Set... from controls

    m_pImpl->m_aConfig.SetIsForPagePreviews( m_aPagePreviews.IsChecked() );
    m_pImpl->m_aConfig.SetIsHelpTipsDisappear( m_aTipHelpCB.IsChecked() );
    m_pImpl->m_aConfig.SetHelpTipSeconds( (short)m_aTipHelpNF.GetValue() );
    m_pImpl->m_aConfig.SetIsAllowAnimatedGraphics( m_aAnimatedGraphics.IsChecked() );
    m_pImpl->m_aConfig.SetIsAllowAnimatedText( m_aAnimatedTexts.IsChecked() );
    m_pImpl->m_aConfig.SetIsAutomaticFontColor( m_aAutomaticFontColor.IsChecked() );
    m_pImpl->m_aConfig.SetSelectionInReadonly( m_aTextSelectionInReadonly.IsChecked());
    m_pImpl->m_aConfig.SetAutoDetectSystemHC( m_aAutoDetectHC.IsChecked());

    if(m_pImpl->m_aConfig.IsModified())
        m_pImpl->m_aConfig.Commit();

#ifndef UNX
    AllSettings aAllSettings = Application::GetSettings();
    MiscSettings aMiscSettings = aAllSettings.GetMiscSettings();
    aMiscSettings.SetEnableATToolSupport( m_aAccessibilityTool.IsChecked() );
    aAllSettings.SetMiscSettings(aMiscSettings);
    Application::SetSettings(aAllSettings);
    Application::MergeSystemSettings( aAllSettings );
#endif

    return FALSE;
}

void SvxAccessibilityOptionsTabPage::Reset( const SfxItemSet& rSet )
{
    //set controls from aConfig.Get...

    m_aPagePreviews.Check(            m_pImpl->m_aConfig.GetIsForPagePreviews() );
    EnableTipHelp(                    m_pImpl->m_aConfig.GetIsHelpTipsDisappear() );
    m_aTipHelpNF.SetValue(            m_pImpl->m_aConfig.GetHelpTipSeconds() );
    m_aAnimatedGraphics.Check(        m_pImpl->m_aConfig.GetIsAllowAnimatedGraphics() );
    m_aAnimatedTexts.Check(           m_pImpl->m_aConfig.GetIsAllowAnimatedText() );
    m_aAutomaticFontColor.Check(      m_pImpl->m_aConfig.GetIsAutomaticFontColor() );
//  m_aSystemFont.Check(              m_pImpl->m_aConfig.GetIsSystemFont() );
    m_aTextSelectionInReadonly.Check( m_pImpl->m_aConfig.IsSelectionInReadonly() );
    m_aAutoDetectHC.Check(            m_pImpl->m_aConfig.GetAutoDetectSystemHC() );


    AllSettings aAllSettings = Application::GetSettings();
    MiscSettings aMiscSettings = aAllSettings.GetMiscSettings();
    m_aAccessibilityTool.Check( aMiscSettings.GetEnableATToolSupport() );
}

IMPL_LINK(SvxAccessibilityOptionsTabPage, TipHelpHdl, CheckBox*, pBox)
{
    sal_Bool bChecked = pBox->IsChecked();
    m_aTipHelpNF.Enable(bChecked);
    return 0;
}

void SvxAccessibilityOptionsTabPage::EnableTipHelp(sal_Bool bCheck)
{
    m_aTipHelpCB.Check(bCheck);
    m_aTipHelpNF.Enable(bCheck);
}
