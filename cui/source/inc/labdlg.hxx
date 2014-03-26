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
class SdrView;

// class SvxCaptionTabPage -----------------------------------------------

const sal_uInt16 CAPTYPE_BITMAPS_COUNT = 3;

class SvxCaptionTabPage : public SfxTabPage
{
private:
    ValueSet*       m_pCT_CAPTTYPE;
    MetricField*    m_pMF_ABSTAND;
    ListBox*        m_pLB_ANSATZ;
    FixedText*      m_pFT_UM;
    MetricField*    m_pMF_ANSATZ;
    FixedText*      m_pFT_ANSATZ_REL;
    ListBox*        m_pLB_ANSATZ_REL;
    FixedText*      m_pFT_LAENGE;
    MetricField*    m_pMF_LAENGE;
    CheckBox*       m_pCB_LAENGE;

    Image           m_aBmpCapTypes[CAPTYPE_BITMAPS_COUNT];

    std::vector<OUString> m_aStrHorzList;
    std::vector<OUString> m_aStrVertList;
    std::vector<OUString> m_aLineTypes;

    short               nCaptionType;
    sal_Int32           nGap;
    short               nEscDir;
    sal_Bool            bEscRel;
    sal_Int32           nEscAbs;
    sal_Int32           nEscRel;
    sal_Int32           nLineLen;
    sal_Bool            bFitLineLen;

    sal_uInt16          nAnsatzRelPos;
    sal_uInt16          nAnsatzTypePos;

    void            SetupAnsatz_Impl( sal_uInt16 nType );
    void            SetupType_Impl( sal_uInt16 nType );
    DECL_LINK( AnsatzSelectHdl_Impl, ListBox * );
    DECL_LINK( AnsatzRelSelectHdl_Impl, ListBox * );
    DECL_LINK( LineOptHdl_Impl, Button * );
    DECL_LINK( SelectCaptTypeHdl_Impl, void * );

    const SfxItemSet&   rOutAttrs;
    const SdrView*      pView;

public:
    SvxCaptionTabPage( Window* pParent, const SfxItemSet& rInAttrs  );

    static SfxTabPage*  Create( Window*, const SfxItemSet& );
    static sal_uInt16*      GetRanges();

    virtual bool        FillItemSet( SfxItemSet& );
    virtual void        Reset( const SfxItemSet & );
    void                Construct();
    void                SetView( const SdrView* pSdrView )
                            { pView = pSdrView; }

    virtual void DataChanged( const DataChangedEvent& rDCEvt );
    void FillValueSet();
};

// class SvxCaptionTabDialog ---------------------------------------------

class SvxCaptionTabDialog : public SfxTabDialog
{
private:
    const SdrView* pView;
    sal_uInt16 nAnchorCtrls;
    sal_uInt16 m_nSwPosSizePageId;
    sal_uInt16 m_nPositionSizePageId;
    sal_uInt16 m_nCaptionPageId;

    Link                aValidateLink;

    virtual void        PageCreated( sal_uInt16 nId, SfxTabPage &rPage );

public:
    SvxCaptionTabDialog(Window* pParent, const SdrView* pView,
                            sal_uInt16 nAnchorTypes = 0);

    /// link for the Writer to validate positions
    void SetValidateFramePosLink( const Link& rLink );
};


#endif // INCLUDED_CUI_SOURCE_INC_LABDLG_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
