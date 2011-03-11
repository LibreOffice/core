/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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

#ifndef _PAD_PRTSETUP_HXX_
#define _PAD_PRTSETUP_HXX_

#include "helper.hxx"

#include "tools/link.hxx"

#include "vcl/tabdlg.hxx"
#include "vcl/tabpage.hxx"
#include "vcl/tabctrl.hxx"
#include "vcl/button.hxx"
#include "vcl/fixed.hxx"
#include "vcl/lstbox.hxx"
#include "vcl/field.hxx"
#include "vcl/combobox.hxx"
#include "vcl/ppdparser.hxx"
#include "vcl/printerinfomanager.hxx"

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
    void insertAllPPDValues( ListBox&, const psp::PPDParser*, const psp::PPDKey* );
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

    DECL_LINK( SelectHdl, ListBox* );
public:
    RTSPaperPage( RTSDialog* );
    ~RTSPaperPage();

    void update();

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

    sal_uLong getLevel();
    sal_uLong getPDFDevice();
    sal_uLong getDepth() { return m_aDepthBox.GetSelectEntry().ToInt32(); }
    sal_uLong getColorDevice()
    {
        String aSpace( m_aSpaceBox.GetSelectEntry() );
        return aSpace == m_aSpaceColor ? 1 : ( aSpace == m_aSpaceGray ? -1 : 0 );
    }
};

class RTSOtherPage : public TabPage
{
    RTSDialog*          m_pParent;

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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
