/*************************************************************************
 *
 *  $RCSfile: prtsetup.hxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: pl $ $Date: 2001-05-08 11:56:36 $
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

#ifndef _PAD_PRTSETUP_HXX_
#define _PAD_PRTSETUP_HXX_

#ifndef _LINK_HXX
#include <tools/link.hxx>
#endif
#ifndef _SV_TABDLG_HXX
#include <vcl/tabdlg.hxx>
#endif
#ifndef _SV_TABPAGE_HXX
#include <vcl/tabpage.hxx>
#endif
#ifndef _SV_TABCTRL_HXX
#include <vcl/tabctrl.hxx>
#endif
#ifndef _SV_BUTTON_HXX
#include <vcl/button.hxx>
#endif
#ifndef _SV_FIXED_HXX
#include <vcl/fixed.hxx>
#endif
#ifndef _SV_LSTBOX_HXX
#include <vcl/lstbox.hxx>
#endif
#ifndef _SV_FIELD_HXX
#include <vcl/field.hxx>
#endif
#ifndef _SV_COMBOBOX_HXX
#include <vcl/combobox.hxx>
#endif
#ifndef _PSPPRINT_PPDPARSER_HXX_
#include <psprint/ppdparser.hxx>
#endif
#ifndef _PSPRINT_PRINTERINFOMANAGER_HXX_
#include <psprint/printerinfomanager.hxx>
#endif
#ifndef _PAD_HELPER_HXX_
#include <helper.hxx>
#endif

namespace padmin {

class RTSPaperPage;
class RTSDevicePage;
class RTSOtherPage;
class RTSFontSubstPage;
class RTSCommandPage;

class RTSDialog : public TabDialog
{
    friend class RTSPaperPage;
    friend class RTSDevicePage;
    friend class RTSOtherPage;
    friend class RTSFontSubstPage;
    friend class RTSCommandPage;

    ::psp::PrinterInfo      m_aJobData;
    String                  m_aPrinter;

    // controls
    TabControl              m_aTabControl;
    OKButton                m_aOKButton;
    CancelButton            m_aCancelButton;

    // pages
    RTSPaperPage*           m_pPaperPage;
    RTSDevicePage*          m_pDevicePage;
    RTSOtherPage*           m_pOtherPage;
    RTSFontSubstPage*       m_pFontSubstPage;
    RTSCommandPage*         m_pCommandPage;

    // some resources
    String                  m_aInvalidString;
    String                  m_aFromDriverString;

    DECL_LINK( ActivatePage, TabControl* );
    DECL_LINK( ClickButton, Button* );

    // helper functions
    void insertAllPPDValues( ListBox&, const ::psp::PPDKey* );

    String getPaperSize();
public:
    RTSDialog( const ::psp::PrinterInfo& rJobData, const String& rPrinter, bool bAllPages, Window* pParent = NULL );
    ~RTSDialog();

    const ::psp::PrinterInfo& getSetup() const { return m_aJobData; }
};

class RTSPaperPage : public TabPage
{
    RTSDialog*          m_pParent;

    FixedText           m_aPaperText;
    ListBox             m_aPaperBox;

    FixedText           m_aOrientText;
    ListBox             m_aOrientBox;

    FixedText           m_aDuplexText;
    ListBox             m_aDuplexBox;

    FixedText           m_aSlotText;
    ListBox             m_aSlotBox;

    FixedText           m_aScaleText;
    MetricField         m_aScaleBox;

    DECL_LINK( SelectHdl, ListBox* );
public:
    RTSPaperPage( RTSDialog* );
    ~RTSPaperPage();

    void update();

    ULONG getScale() { return m_aScaleBox.GetValue(); }
    String getOrientation() { return m_aOrientBox.GetSelectEntry(); }
};

class RTSDevicePage : public TabPage
{
    RTSDialog*          m_pParent;

    String              m_aSpaceColor;
    String              m_aSpaceGray;

    FixedText           m_aPPDKeyText;
    ListBox             m_aPPDKeyBox;

    FixedText           m_aPPDValueText;
    ListBox             m_aPPDValueBox;

    FixedText           m_aLevelText;
    ListBox             m_aLevelBox;

    FixedText           m_aSpaceText;
    ListBox             m_aSpaceBox;

    FixedText           m_aDepthText;
    ListBox             m_aDepthBox;

    void FillValueBox( const ::psp::PPDKey* );

    DECL_LINK( SelectHdl, ListBox* );
public:
    RTSDevicePage( RTSDialog* );
    ~RTSDevicePage();

    void update();

    ULONG getLevel() { return m_aLevelBox.GetSelectEntry().ToInt32(); }
    ULONG getDepth() { return m_aDepthBox.GetSelectEntry().ToInt32(); }
    ULONG getColorDevice()
    {
        String aSpace( m_aSpaceBox.GetSelectEntry() );
        return aSpace == m_aSpaceColor ? 1 : ( aSpace == m_aSpaceGray ? -1 : 0 );
    }
};

class RTSOtherPage : public TabPage
{
    RTSDialog*          m_pParent;

    ULONG               m_nMarginLeft;
    ULONG               m_nMarginTop;
    ULONG               m_nMarginRight;
    ULONG               m_nMarginBottom;

    FixedText           m_aLeftTxt;
    MetricField         m_aLeftLB;
    FixedText           m_aTopTxt;
    MetricField         m_aTopLB;
    FixedText           m_aRightTxt;
    MetricField         m_aRightLB;
    FixedText           m_aBottomTxt;
    MetricField         m_aBottomLB;
    FixedText           m_aCommentTxt;
    Edit                m_aCommentEdt;
    PushButton          m_aDefaultBtn;

    void initValues();

    DECL_LINK( ClickBtnHdl, Button *);

public:
    RTSOtherPage( RTSDialog* );
    ~RTSOtherPage();

    void save();
};

class RTSFontSubstPage : public TabPage
{
    RTSDialog*          m_pParent;

    FixedText           m_aSubstitutionsText;
    DelMultiListBox     m_aSubstitutionsBox;
    FixedText           m_aFromFontText;
    ComboBox            m_aFromFontBox;
    FixedText           m_aToFontText;
    ListBox             m_aToFontBox;

    PushButton          m_aAddButton;
    PushButton          m_aRemoveButton;
    CheckBox            m_aEnableBox;

    DECL_LINK( ClickBtnHdl, Button* );
    DECL_LINK( SelectHdl, ListBox* );
    DECL_LINK( DelPressedHdl, ListBox* );

    void update();
public:
    RTSFontSubstPage( RTSDialog* );
    ~RTSFontSubstPage();
};

} // namespace

#endif // _PAD_PRTSETUP_HXX
