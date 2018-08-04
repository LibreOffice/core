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
#ifndef INCLUDED_CUI_SOURCE_INC_LABDLG_HXX
#define INCLUDED_CUI_SOURCE_INC_LABDLG_HXX

#include <vcl/field.hxx>
#include <vcl/lstbox.hxx>
#include <vcl/fixed.hxx>
#include <svtools/valueset.hxx>
#include <sfx2/tabdlg.hxx>
#include <svx/sxctitm.hxx>
#include <svx/sxcecitm.hxx>
#include <svx/anchorid.hxx>


class SdrView;

// class SvxCaptionTabPage -----------------------------------------------

const sal_uInt16 CAPTYPE_BITMAPS_COUNT = 3;

class SvxCaptionTabPage : public SfxTabPage
{
private:
    static const sal_uInt16 pCaptionRanges[];
    VclPtr<ValueSet>       m_pCT_CAPTTYPE;
    VclPtr<MetricField>    m_pMF_SPACING;
    VclPtr<ListBox>        m_pLB_EXTENSION;
    VclPtr<FixedText>      m_pFT_BYFT;
    VclPtr<MetricField>    m_pMF_BY;
    VclPtr<FixedText>      m_pFT_POSITIONFT;
    VclPtr<ListBox>        m_pLB_POSITION;
    VclPtr<FixedText>      m_pFT_LENGTHFT;
    VclPtr<MetricField>    m_pMF_LENGTH;
    VclPtr<CheckBox>       m_pCB_OPTIMAL;

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

    void            SetupExtension_Impl( sal_uInt16 nType );
    void            SetupType_Impl( SdrCaptionType nType );
    DECL_LINK( ExtensionSelectHdl_Impl, ListBox&, void );
    DECL_LINK( PositionSelectHdl_Impl, ListBox&, void );
    DECL_LINK( LineOptHdl_Impl, Button *, void );
    DECL_LINK( SelectCaptTypeHdl_Impl, ValueSet*, void );

    const SfxItemSet&   rOutAttrs;
    const SdrView*      pView;

public:
    SvxCaptionTabPage( vcl::Window* pParent, const SfxItemSet& rInAttrs  );
    virtual ~SvxCaptionTabPage() override;
    virtual void dispose() override;

    static VclPtr<SfxTabPage>  Create( TabPageParent, const SfxItemSet* );
    static const sal_uInt16*  GetRanges() { return pCaptionRanges; }

    virtual bool        FillItemSet( SfxItemSet* ) override;
    virtual void        Reset( const SfxItemSet * ) override;
    void                Construct();
    void                SetView( const SdrView* pSdrView )
                            { pView = pSdrView; }

    virtual void DataChanged( const DataChangedEvent& rDCEvt ) override;
    void FillValueSet();
};

// class SvxCaptionTabDialog ---------------------------------------------
struct SvxSwFrameValidation;
class SvxCaptionTabDialog : public SfxTabDialog
{
private:
    const SdrView* pView;
    SvxAnchorIds nAnchorCtrls;
    sal_uInt16 m_nSwPosSizePageId;
    sal_uInt16 m_nPositionSizePageId;
    sal_uInt16 m_nCaptionPageId;

    Link<SvxSwFrameValidation&,void> aValidateLink;

    virtual void        PageCreated( sal_uInt16 nId, SfxTabPage &rPage ) override;

public:
    SvxCaptionTabDialog(vcl::Window* pParent, const SdrView* pView,
                            SvxAnchorIds nAnchorTypes);

    /// link for the Writer to validate positions
    void SetValidateFramePosLink( const Link<SvxSwFrameValidation&,void>& rLink );
};


#endif // INCLUDED_CUI_SOURCE_INC_LABDLG_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
