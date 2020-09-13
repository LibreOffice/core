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
#include <sfx2/tabdlg.hxx>
#include <svx/sxctitm.hxx>
#include <svx/sxcecitm.hxx>
#include <svx/anchorid.hxx>
#include <vcl/image.hxx>

class SdrView;

// class SvxCaptionTabPage -----------------------------------------------

const sal_uInt16 CAPTYPE_BITMAPS_COUNT = 3;

class SvxCaptionTabPage : public SfxTabPage
{
private:
    static const sal_uInt16 pCaptionRanges[];

    Image           m_aBmpCapTypes[CAPTYPE_BITMAPS_COUNT];

    std::vector<OUString> m_aStrHorzList;
    std::vector<OUString> m_aStrVertList;

    SdrCaptionType      nCaptionType;
    sal_Int32           nGap;
    SdrCaptionEscDir    nEscDir;
    bool                bEscRel;
    sal_Int32           nEscAbs;
    sal_Int32           nEscRel;
    sal_Int32           nLineLen;
    bool                bFitLineLen;

    sal_uInt16          nPosition;
    sal_uInt16          nExtension;

    const SfxItemSet&   rOutAttrs;
    const SdrView*      pView;

    std::unique_ptr<weld::MetricSpinButton> m_xMF_SPACING;
    std::unique_ptr<weld::ComboBox> m_xLB_EXTENSION;
    std::unique_ptr<weld::Label> m_xFT_BYFT;
    std::unique_ptr<weld::MetricSpinButton> m_xMF_BY;
    std::unique_ptr<weld::Label> m_xFT_POSITIONFT;
    std::unique_ptr<weld::ComboBox> m_xLB_POSITION;
    std::unique_ptr<weld::ComboBox> m_xLineTypes;
    std::unique_ptr<weld::Label> m_xFT_LENGTHFT;
    std::unique_ptr<weld::MetricSpinButton> m_xMF_LENGTH;
    std::unique_ptr<weld::CheckButton> m_xCB_OPTIMAL;
    std::unique_ptr<ValueSet> m_xCT_CAPTTYPE;
    std::unique_ptr<weld::CustomWeld> m_xCT_CAPTTYPEWin;

    void            SetupExtension_Impl( sal_uInt16 nType );
    void            SetupType_Impl( SdrCaptionType nType );
    DECL_LINK(ExtensionSelectHdl_Impl, weld::ComboBox&, void);
    DECL_LINK(PositionSelectHdl_Impl, weld::ComboBox&, void);
    DECL_LINK(LineOptHdl_Impl, weld::ToggleButton&, void);
    DECL_LINK(SelectCaptTypeHdl_Impl, ValueSet*, void);

public:
    SvxCaptionTabPage(weld::Container* pPage, weld::DialogController* pController, const SfxItemSet& rInAttrs);
    virtual ~SvxCaptionTabPage() override;

    static std::unique_ptr<SfxTabPage>  Create( weld::Container* pPage, weld::DialogController* pController, const SfxItemSet* );
    static const sal_uInt16*  GetRanges() { return pCaptionRanges; }

    virtual bool        FillItemSet( SfxItemSet* ) override;
    virtual void        Reset( const SfxItemSet * ) override;
    void                Construct();
    void                SetView( const SdrView* pSdrView )
                            { pView = pSdrView; }

    void FillValueSet();
};

// class SvxCaptionTabDialog ---------------------------------------------
struct SvxSwFrameValidation;
class SvxCaptionTabDialog : public SfxTabDialogController
{
private:
    const SdrView* pView;
    SvxAnchorIds nAnchorCtrls;

    Link<SvxSwFrameValidation&,void> aValidateLink;

    virtual void PageCreated(const OString& rId, SfxTabPage &rPage) override;

public:
    SvxCaptionTabDialog(weld::Window* pParent, const SdrView* pView,
                            SvxAnchorIds nAnchorTypes);

    /// link for the Writer to validate positions
    void SetValidateFramePosLink( const Link<SvxSwFrameValidation&,void>& rLink );
};


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
