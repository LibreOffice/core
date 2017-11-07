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

#ifndef INCLUDED_SD_SOURCE_UI_INC_PUBDLG_HXX
#define INCLUDED_SD_SOURCE_UI_INC_PUBDLG_HXX

#include <com/sun/star/beans/PropertyValue.hpp>
#include <vcl/fixed.hxx>
#include <vcl/group.hxx>
#include <vcl/button.hxx>
#include <vcl/field.hxx>
#include <vcl/dialog.hxx>
#include <vcl/layout.hxx>
#include <resltn.hxx>
#include <pres.hxx>
#include "assclass.hxx"

#include <memory>
#include <vector>

class FixedText;
class RadioButton;
class ListBox;
class ComboBox;
class Edit;
class MultiLineEdit;
class ValueSet;
class SdHtmlAttrPreview;
class SdPublishingDesign;
class ButtonSet;

// *********************************************************************
// Html-Export Autopilot
// *********************************************************************

class SdPublishingDlg : public ModalDialog
{
private:
    // page 1 controls
    VclPtr<VclContainer>   pPage1;
    VclPtr<FixedText>      pPage1_Titel;
    VclPtr<RadioButton>    pPage1_NewDesign;
    VclPtr<RadioButton>    pPage1_OldDesign;
    VclPtr<ListBox>        pPage1_Designs;
    VclPtr<PushButton>     pPage1_DelDesign;
    VclPtr<FixedText>      pPage1_Desc;

    // page 2 controls
    VclPtr<VclContainer>   pPage2;
    VclPtr<VclContainer>   pPage2Frame2;
    VclPtr<VclContainer>   pPage2Frame3;
    VclPtr<VclContainer>   pPage2Frame4;
    VclPtr<FixedText>      pPage2_Titel;
    VclPtr<RadioButton>    pPage2_Standard;
    VclPtr<RadioButton>    pPage2_Frames;
    VclPtr<RadioButton>    pPage2_SingleDocument;
    VclPtr<RadioButton>    pPage2_Kiosk;
    VclPtr<RadioButton>    pPage2_WebCast;
    VclPtr<FixedImage>    pPage2_Standard_FB;
    VclPtr<FixedImage>    pPage2_Frames_FB;
    VclPtr<FixedImage>    pPage2_Kiosk_FB;
    VclPtr<FixedImage>    pPage2_WebCast_FB;

    VclPtr<FixedText>      pPage2_Titel_Html;
    VclPtr<CheckBox>       pPage2_Content;
    VclPtr<CheckBox>       pPage2_Notes;

    VclPtr<FixedText>      pPage2_Titel_WebCast;
    VclPtr<RadioButton>    pPage2_ASP;
    VclPtr<RadioButton>    pPage2_PERL;
    VclPtr<FixedText>      pPage2_URL_txt;
    VclPtr<Edit>           pPage2_URL;
    VclPtr<FixedText>      pPage2_CGI_txt;
    VclPtr<Edit>           pPage2_CGI;
    VclPtr<FixedText>      pPage2_Index_txt;
    VclPtr<Edit>           pPage2_Index;
    VclPtr<FixedText>      pPage2_Titel_Kiosk;
    VclPtr<RadioButton>    pPage2_ChgDefault;
    VclPtr<RadioButton>    pPage2_ChgAuto;
    VclPtr<FixedText>      pPage2_Duration_txt;
    VclPtr<TimeField>      pPage2_Duration;
    VclPtr<CheckBox>       pPage2_Endless;

    // page 3 controls
    VclPtr<VclContainer>   pPage3;
    VclPtr<FixedText>      pPage3_Titel1;
    VclPtr<RadioButton>    pPage3_Png;
    VclPtr<RadioButton>    pPage3_Gif;
    VclPtr<RadioButton>    pPage3_Jpg;
    VclPtr<FixedText>      pPage3_Quality_txt;
    VclPtr<ComboBox>       pPage3_Quality;
    VclPtr<FixedText>      pPage3_Titel2;
    VclPtr<RadioButton>    pPage3_Resolution_1;
    VclPtr<RadioButton>    pPage3_Resolution_2;
    VclPtr<RadioButton>    pPage3_Resolution_3;
    VclPtr<FixedText>      pPage3_Titel3;
    VclPtr<CheckBox>       pPage3_SldSound;
    VclPtr<CheckBox>       pPage3_HiddenSlides;

    // page 4 controls
    VclPtr<VclContainer>   pPage4;
    VclPtr<FixedText>      pPage4_Titel1;
    VclPtr<FixedText>      pPage4_Author_txt;
    VclPtr<Edit>           pPage4_Author;
    VclPtr<FixedText>      pPage4_Email_txt;
    VclPtr<Edit>           pPage4_Email;
    VclPtr<FixedText>      pPage4_WWW_txt;
    VclPtr<Edit>           pPage4_WWW;
    VclPtr<FixedText>      pPage4_Titel2;
    VclPtr<VclMultiLineEdit>  pPage4_Misc;
    VclPtr<CheckBox>       pPage4_Download;

    // page 5 controls
    VclPtr<VclContainer>   pPage5;
    VclPtr<FixedText>      pPage5_Titel;
    VclPtr<CheckBox>       pPage5_TextOnly;
    VclPtr<ValueSet>       pPage5_Buttons;

    // page 6 controls
    VclPtr<VclContainer>   pPage6;
    VclPtr<FixedText>      pPage6_Titel;
    VclPtr<RadioButton>    pPage6_Default;
    VclPtr<RadioButton>    pPage6_User;
    VclPtr<PushButton>     pPage6_Back;
    VclPtr<PushButton>     pPage6_Text;
    VclPtr<PushButton>     pPage6_Link;
    VclPtr<PushButton>     pPage6_VLink;
    VclPtr<PushButton>     pPage6_ALink;
    VclPtr<RadioButton>    pPage6_DocColors;
    VclPtr<SdHtmlAttrPreview>  pPage6_Preview;

    std::unique_ptr< ButtonSet > mpButtonSet;

    // standard controls
    VclPtr<PushButton>     pLastPageButton;
    VclPtr<PushButton>     pNextPageButton;
    VclPtr<PushButton>     pFinishButton;

    Assistent       aAssistentFunc;

    bool            m_bImpress;
    bool            m_bButtonsDirty;

    void SetDefaults();
    void CreatePages();

    Color m_aBackColor, m_aTextColor, m_aLinkColor;
    Color m_aVLinkColor, m_aALinkColor;

    void    ChangePage();
    void    UpdatePage();

    std::vector<SdPublishingDesign>   m_aDesignList;
    bool    m_bDesignListDirty;
    SdPublishingDesign* m_pDesign;
    void    Load();
    bool    Save();

    void    GetDesign( SdPublishingDesign* pDesign );
    void    SetDesign( SdPublishingDesign const * pDesign );

    void    LoadPreviewButtons();

    DECL_LINK( FinishHdl, Button*, void );
    DECL_LINK( NextPageHdl, Button*, void );
    DECL_LINK( LastPageHdl, Button*, void );

    DECL_LINK( DesignHdl, Button*, void );
    DECL_LINK( DesignSelectHdl, ListBox&, void );
    DECL_LINK( DesignDeleteHdl, Button*, void );
    DECL_LINK( BaseHdl, Button*, void );
    DECL_LINK( ContentHdl, Button*, void );
    DECL_LINK( GfxFormatHdl, Button *, void );
    DECL_LINK( ResolutionHdl, Button*, void );
    DECL_LINK( ButtonsHdl, ValueSet*, void );
    DECL_LINK( ColorHdl, Button*, void );
    DECL_LINK( WebServerHdl, Button *, void );
    DECL_LINK( SlideChgHdl, Button*, void );

public:

    SdPublishingDlg(vcl::Window* pWindow, DocumentType eDocType);
    virtual ~SdPublishingDlg() override;
    virtual void dispose() override;

    void GetParameterSequence( css::uno::Sequence< css::beans::PropertyValue >& rParams );

    // Screenshot interface
    virtual std::vector<OString> getAllPageUIXMLDescriptions() const override;
    virtual bool selectPageByUIXMLDescription(const OString& rUIXMLDescription) override;
};

#endif // INCLUDED_SD_SOURCE_UI_INC_PUBDLG_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
