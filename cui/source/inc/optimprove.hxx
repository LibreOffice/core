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
#ifndef _SVX_OPTIMPROVE_HXX
#define _SVX_OPTIMPROVE_HXX

// include ---------------------------------------------------------------

#include <vcl/tabpage.hxx>
#include <vcl/fixed.hxx>
#include <vcl/button.hxx>
#include <svtools/fixedhyper.hxx>
#include <sfx2/basedlgs.hxx>
#include <sfx2/tabdlg.hxx>

// class SvxImprovementPage ----------------------------------------------

class SvxImprovementPage : public TabPage
{
private:
    FixedLine       m_aImproveFL;
    FixedText       m_aInvitationFT;
    RadioButton     m_aYesRB;
    RadioButton     m_aNoRB;
    FixedLine       m_aDataFL;
    FixedText       m_aNumberOfReportsFT;
    FixedText       m_aNumberOfReportsValueFT;
    FixedText       m_aNumberOfActionsFT;
    FixedText       m_aNumberOfActionsValueFT;
    PushButton      m_aShowDataPB;

    String          m_sInfo;
    String          m_sMoreInfo;

public:
    SvxImprovementPage( Window* pParent );
    ~SvxImprovementPage();

    inline bool     IsYesChecked() const { return m_aYesRB.IsChecked() != sal_False; }

    inline String   GetPageText() const { return GetText(); }
    inline String   GetInvitationText() const { return m_aInvitationFT.GetText(); }
    inline String   GetYesButtonText() const { return m_aYesRB.GetText(); }
    inline String   GetNoButtonText() const { return m_aNoRB.GetText(); }
    inline String   GetInfoText() const { return m_sInfo; }
    inline String   GetTitleText() const { return m_aImproveFL.GetText(); }
};

class SvxImprovementOptionsPage : public SfxTabPage
{
private:
    FixedLine                   m_aImproveFL;
    FixedText                   m_aInvitationFT;
    RadioButton                 m_aYesRB;
    RadioButton                 m_aNoRB;
    svt::FixedHyperlinkImage    m_aInfoFI;
    FixedLine                   m_aDataFL;
    FixedText                   m_aNumberOfReportsFT;
    FixedText                   m_aNumberOfReportsValueFT;
    FixedText                   m_aNumberOfActionsFT;
    FixedText                   m_aNumberOfActionsValueFT;
    PushButton                  m_aShowDataPB;

    String                      m_sInfo;
    String                      m_sMoreInfo;
    ::rtl::OUString             m_sLogPath;

    SvxImprovementOptionsPage( Window* pParent, const SfxItemSet& rSet );

    DECL_LINK( HandleHyperlink, svt::FixedHyperlinkImage * );
    DECL_LINK( HandleShowData, PushButton * );

public:
    virtual             ~SvxImprovementOptionsPage();

    static SfxTabPage*  Create( Window* pParent, const SfxItemSet& rSet );

    virtual sal_Bool    FillItemSet( SfxItemSet& rSet );
    virtual void        Reset( const SfxItemSet& rSet );
};

class SvxImprovementDialog : public SfxSingleTabDialog
{
private:
    SvxImprovementPage*     m_pPage;

    DECL_LINK( HandleOK, OKButton * );
    DECL_LINK( HandleHyperlink, svt::FixedHyperlinkImage * );

public:
    SvxImprovementDialog( Window* pParent, const String& rInfoURL );
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
