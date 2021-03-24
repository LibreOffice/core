/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#pragma once

#include <svtools/valueset.hxx>
#include <vcl/weld.hxx>
#include <vcl/weldutils.hxx>
#include <pres.hxx>
#include "assclass.hxx"

#include <memory>
#include <vector>

class ListBox;
class Edit;
class SdHtmlAttrPreview;
class SdPublishingDesign;
class ButtonSet;

namespace com::sun::star::beans
{
struct PropertyValue;
}
namespace com::sun::star::uno
{
template <class E> class Sequence;
}

// *********************************************************************
// Html-Export Autopilot
// *********************************************************************
// should turn this into a wizard
class SdPublishingDlg : public weld::GenericDialogController
{
private:
    // page 1 controls
    std::unique_ptr<weld::Container> m_xPage1;
    std::unique_ptr<weld::Label> m_xPage1_Title;
    std::unique_ptr<weld::RadioButton> m_xPage1_NewDesign;
    std::unique_ptr<weld::RadioButton> m_xPage1_OldDesign;
    std::unique_ptr<weld::TreeView> m_xPage1_Designs;
    std::unique_ptr<weld::Button> m_xPage1_DelDesign;
    std::unique_ptr<weld::Label> m_xPage1_Desc;

    // page 2 controls
    std::unique_ptr<weld::Container> m_xPage2;
    std::unique_ptr<weld::Container> m_xPage2Frame2;
    std::unique_ptr<weld::Container> m_xPage2Frame3;
    std::unique_ptr<weld::Container> m_xPage2Frame4;
    std::unique_ptr<weld::Label> m_xPage2_Title;
    std::unique_ptr<weld::RadioButton> m_xPage2_Standard;
    std::unique_ptr<weld::RadioButton> m_xPage2_Frames;
    std::unique_ptr<weld::RadioButton> m_xPage2_SingleDocument;
    std::unique_ptr<weld::RadioButton> m_xPage2_Kiosk;
    std::unique_ptr<weld::RadioButton> m_xPage2_WebCast;
    std::unique_ptr<weld::Image> m_xPage2_Standard_FB;
    std::unique_ptr<weld::Image> m_xPage2_Frames_FB;
    std::unique_ptr<weld::Image> m_xPage2_Kiosk_FB;
    std::unique_ptr<weld::Image> m_xPage2_WebCast_FB;

    std::unique_ptr<weld::Label> m_xPage2_Title_Html;
    std::unique_ptr<weld::CheckButton> m_xPage2_Content;
    std::unique_ptr<weld::CheckButton> m_xPage2_Notes;

    std::unique_ptr<weld::Label> m_xPage2_Title_WebCast;
    std::unique_ptr<weld::RadioButton> m_xPage2_ASP;
    std::unique_ptr<weld::RadioButton> m_xPage2_PERL;
    std::unique_ptr<weld::Label> m_xPage2_URL_txt;
    std::unique_ptr<weld::Entry> m_xPage2_URL;
    std::unique_ptr<weld::Label> m_xPage2_CGI_txt;
    std::unique_ptr<weld::Entry> m_xPage2_CGI;
    std::unique_ptr<weld::Label> m_xPage2_Index_txt;
    std::unique_ptr<weld::Entry> m_xPage2_Index;
    std::unique_ptr<weld::Label> m_xPage2_Title_Kiosk;
    std::unique_ptr<weld::RadioButton> m_xPage2_ChgDefault;
    std::unique_ptr<weld::RadioButton> m_xPage2_ChgAuto;
    std::unique_ptr<weld::Label> m_xPage2_Duration_txt;
    std::unique_ptr<weld::FormattedSpinButton> m_xPage2_Duration;
    std::unique_ptr<weld::TimeFormatter> m_xFormatter;
    std::unique_ptr<weld::CheckButton> m_xPage2_Endless;

    // page 3 controls
    std::unique_ptr<weld::Container> m_xPage3;
    std::unique_ptr<weld::Label> m_xPage3_Title1;
    std::unique_ptr<weld::RadioButton> m_xPage3_Png;
    std::unique_ptr<weld::RadioButton> m_xPage3_Gif;
    std::unique_ptr<weld::RadioButton> m_xPage3_Jpg;
    std::unique_ptr<weld::Label> m_xPage3_Quality_txt;
    std::unique_ptr<weld::ComboBox> m_xPage3_Quality;
    std::unique_ptr<weld::Label> m_xPage3_Title2;
    std::unique_ptr<weld::RadioButton> m_xPage3_Resolution_1;
    std::unique_ptr<weld::RadioButton> m_xPage3_Resolution_2;
    std::unique_ptr<weld::RadioButton> m_xPage3_Resolution_3;
    std::unique_ptr<weld::RadioButton> m_xPage3_Resolution_4;
    std::unique_ptr<weld::Label> m_xPage3_Title3;
    std::unique_ptr<weld::CheckButton> m_xPage3_SldSound;
    std::unique_ptr<weld::CheckButton> m_xPage3_HiddenSlides;

    // page 4 controls
    std::unique_ptr<weld::Container> m_xPage4;
    std::unique_ptr<weld::Label> m_xPage4_Title1;
    std::unique_ptr<weld::Label> m_xPage4_Author_txt;
    std::unique_ptr<weld::Entry> m_xPage4_Author;
    std::unique_ptr<weld::Label> m_xPage4_Email_txt;
    std::unique_ptr<weld::Entry> m_xPage4_Email;
    std::unique_ptr<weld::Label> m_xPage4_WWW_txt;
    std::unique_ptr<weld::Entry> m_xPage4_WWW;
    std::unique_ptr<weld::Label> m_xPage4_Title2;
    std::unique_ptr<weld::TextView> m_xPage4_Misc;
    std::unique_ptr<weld::CheckButton> m_xPage4_Download;

    // page 5 controls
    std::unique_ptr<weld::Container> m_xPage5;
    std::unique_ptr<weld::Label> m_xPage5_Title;
    std::unique_ptr<weld::CheckButton> m_xPage5_TextOnly;
    std::unique_ptr<ValueSet> m_xPage5_Buttons;
    std::unique_ptr<weld::CustomWeld> m_xPage5_ButtonsWnd;

    // page 6 controls
    std::unique_ptr<weld::Container> m_xPage6;
    std::unique_ptr<weld::Label> m_xPage6_Title;
    std::unique_ptr<weld::RadioButton> m_xPage6_Default;
    std::unique_ptr<weld::RadioButton> m_xPage6_User;
    std::unique_ptr<weld::Button> m_xPage6_Back;
    std::unique_ptr<weld::Button> m_xPage6_Text;
    std::unique_ptr<weld::Button> m_xPage6_Link;
    std::unique_ptr<weld::Button> m_xPage6_VLink;
    std::unique_ptr<weld::Button> m_xPage6_ALink;
    std::unique_ptr<weld::RadioButton> m_xPage6_DocColors;
    std::unique_ptr<SdHtmlAttrPreview> m_xPage6_Preview;
    std::unique_ptr<weld::CustomWeld> m_xPage6_PreviewWnd;

    std::unique_ptr<ButtonSet> m_xButtonSet;

    // standard controls
    std::unique_ptr<weld::Button> m_xLastPageButton;
    std::unique_ptr<weld::Button> m_xNextPageButton;
    std::unique_ptr<weld::Button> m_xFinishButton;

    Assistent aAssistentFunc;

    bool m_bImpress;
    bool m_bButtonsDirty;

    void SetDefaults();
    void CreatePages();

    Color m_aBackColor, m_aTextColor, m_aLinkColor;
    Color m_aVLinkColor, m_aALinkColor;

    void ChangePage();
    void UpdatePage();

    std::vector<SdPublishingDesign> m_aDesignList;
    bool m_bDesignListDirty;
    SdPublishingDesign* m_pDesign;
    void Load();
    bool Save();

    void GetDesign(SdPublishingDesign* pDesign);
    void SetDesign(SdPublishingDesign const* pDesign);

    void LoadPreviewButtons();

    DECL_LINK(FinishHdl, weld::Button&, void);
    DECL_LINK(NextPageHdl, weld::Button&, void);
    DECL_LINK(LastPageHdl, weld::Button&, void);

    DECL_LINK(DesignHdl, weld::Button&, void);
    DECL_LINK(DesignSelectHdl, weld::TreeView&, void);
    DECL_LINK(DesignDeleteHdl, weld::Button&, void);
    DECL_LINK(BaseHdl, weld::Button&, void);
    DECL_LINK(ContentHdl, weld::Button&, void);
    DECL_LINK(GfxFormatHdl, weld::Button&, void);
    DECL_LINK(ResolutionHdl, weld::Button&, void);
    DECL_LINK(ButtonsHdl, ValueSet*, void);
    DECL_LINK(ColorHdl, weld::Button&, void);
    DECL_LINK(WebServerHdl, weld::Button&, void);
    DECL_LINK(SlideChgHdl, weld::Button&, void);

public:
    SdPublishingDlg(weld::Window* pWindow, DocumentType eDocType);
    virtual ~SdPublishingDlg() override;

    void GetParameterSequence(css::uno::Sequence<css::beans::PropertyValue>& rParams);
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
