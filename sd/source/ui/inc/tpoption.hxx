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

#ifndef INCLUDED_SD_SOURCE_UI_INC_TPOPTION_HXX
#define INCLUDED_SD_SOURCE_UI_INC_TPOPTION_HXX

#include <vcl/lstbox.hxx>
#include <vcl/group.hxx>
#include <vcl/fixed.hxx>
#include <vcl/field.hxx>
#include <vcl/button.hxx>
#include <svtools/stdctrl.hxx>
#include <sfx2/tabdlg.hxx>
#include <svx/optgrid.hxx>

/**
 * Option-Tab-Page: Snap
 */
class SdTpOptionsSnap : public SvxGridTabPage
{
public:
            SdTpOptionsSnap( vcl::Window* pParent, const SfxItemSet& rInAttrs  );
            virtual ~SdTpOptionsSnap();

    static  VclPtr<SfxTabPage> Create( vcl::Window*, const SfxItemSet* );
    virtual bool FillItemSet( SfxItemSet* ) SAL_OVERRIDE;
    virtual void Reset( const SfxItemSet * ) SAL_OVERRIDE;

};

/**
 * Option-Tab-Page: Contents
 */
class SdTpOptionsContents : public SfxTabPage
{
private:
    VclPtr<CheckBox>    m_pCbxRuler;
    VclPtr<CheckBox>    m_pCbxDragStripes;
    VclPtr<CheckBox>    m_pCbxHandlesBezier;
    VclPtr<CheckBox>    m_pCbxMoveOutline;

public:
            SdTpOptionsContents( vcl::Window* pParent, const SfxItemSet& rInAttrs  );
            virtual ~SdTpOptionsContents();
    virtual void dispose() SAL_OVERRIDE;

    static  VclPtr<SfxTabPage> Create( vcl::Window*, const SfxItemSet* );
    virtual bool FillItemSet( SfxItemSet* ) SAL_OVERRIDE;
    virtual void Reset( const SfxItemSet * ) SAL_OVERRIDE;
};

/**
 * Option-Tab-Page: View
 */
class SdModule;
class SdTpOptionsMisc : public SfxTabPage
{
 friend class SdModule;

private:
    VclPtr<CheckBox>    m_pCbxQuickEdit;
    VclPtr<CheckBox>    m_pCbxPickThrough;

    VclPtr<VclFrame>    m_pNewDocumentFrame;
    VclPtr<CheckBox>    m_pCbxStartWithTemplate;

    VclPtr<CheckBox>    m_pCbxMasterPageCache;
    VclPtr<CheckBox>    m_pCbxCopy;
    VclPtr<CheckBox>    m_pCbxMarkedHitMovesAlways;
    VclPtr<VclFrame>    m_pPresentationFrame;

    VclPtr<ListBox>     m_pLbMetric;
    VclPtr<MetricField> m_pMtrFldTabstop;

    VclPtr<CheckBox>    m_pCbxEnableSdremote;
    VclPtr<CheckBox>    m_pCbxEnablePresenterScreen;
    VclPtr<CheckBox>    m_pCbxUsePrinterMetrics;
    VclPtr<CheckBox>    m_pCbxCompatibility;

    //Scale
    VclPtr<VclFrame>    m_pScaleFrame;
    VclPtr<ComboBox>    m_pCbScale;
    VclPtr<FixedText>   m_pNewDocLb;
    VclPtr<FixedText>   m_pFiInfo1;
    VclPtr<MetricField> m_pMtrFldOriginalWidth;
    VclPtr<FixedText>   m_pWidthLb;
    VclPtr<FixedText>   m_pHeightLb;
    VclPtr<FixedText>   m_pFiInfo2;
    VclPtr<MetricField> m_pMtrFldOriginalHeight;
    VclPtr<CheckBox>    m_pCbxDistrot;
    VclPtr<MetricField> m_pMtrFldInfo1;
    VclPtr<MetricField> m_pMtrFldInfo2;

    sal_uInt32          nWidth;
    sal_uInt32          nHeight;
    OUString aInfo1;
    OUString aInfo2;

    SfxMapUnit          ePoolUnit;

    static OUString        GetScale( sal_Int32 nX, sal_Int32 nY );
    static bool            SetScale( const OUString& aScale, sal_Int32& rX, sal_Int32& rY );

    DECL_LINK_TYPED( SelectMetricHdl_Impl, ListBox&, void );

    /** Enable or disable the controls in the compatibility section of the
        'general' tab page depending on whether there is at least one
        document.
    */
    void UpdateCompatibilityControls();

protected:
    virtual void ActivatePage( const SfxItemSet& rSet ) SAL_OVERRIDE;
    virtual sfxpg DeactivatePage( SfxItemSet* pSet ) SAL_OVERRIDE;

public:
            SdTpOptionsMisc( vcl::Window* pParent, const SfxItemSet& rInAttrs  );
            virtual ~SdTpOptionsMisc();
    virtual void dispose() SAL_OVERRIDE;

    static  VclPtr<SfxTabPage> Create( vcl::Window*, const SfxItemSet* );
    virtual bool FillItemSet( SfxItemSet* ) SAL_OVERRIDE;
    virtual void Reset( const SfxItemSet * ) SAL_OVERRIDE;

    /** Hide Impress specific controls, make Draw specific controls visible
        and arrange the visible controls.  Do not call this method or the
        <member>SetImpressMode()</member> method more than once.
    */
    void SetDrawMode();

    /** Hide Draw specific controls, make Impress specific controls visible
        and arrange the visible controls.  Do not call this method or the
        <member>SetDrawMode()</member> method more than once.
    */
    void SetImpressMode();
    virtual void        PageCreated(const SfxAllItemSet& aSet) SAL_OVERRIDE;

    using TabPage::ActivatePage;
    using TabPage::DeactivatePage;
    using OutputDevice::SetDrawMode;

};

#endif // INCLUDED_SD_SOURCE_UI_INC_TPOPTION_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
