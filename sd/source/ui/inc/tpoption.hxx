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

#include <vcl/layout.hxx>
#include <vcl/lstbox.hxx>
#include <vcl/fixed.hxx>
#include <vcl/field.hxx>
#include <vcl/button.hxx>
#include <sfx2/tabdlg.hxx>
#include <svx/optgrid.hxx>

/**
 * Option-Tab-Page: Snap
 */
class SdTpOptionsSnap : public SvxGridTabPage
{
public:
    SdTpOptionsSnap(TabPageParent pParent, const SfxItemSet& rInAttrs);
    virtual ~SdTpOptionsSnap() override;

    static  VclPtr<SfxTabPage> Create( TabPageParent, const SfxItemSet* );
    virtual bool FillItemSet( SfxItemSet* ) override;
    virtual void Reset( const SfxItemSet * ) override;
};

/**
 * Option-Tab-Page: Contents
 */
class SdTpOptionsContents : public SfxTabPage
{
private:
    std::unique_ptr<weld::CheckButton> m_xCbxRuler;
    std::unique_ptr<weld::CheckButton> m_xCbxDragStripes;
    std::unique_ptr<weld::CheckButton> m_xCbxHandlesBezier;
    std::unique_ptr<weld::CheckButton> m_xCbxMoveOutline;

public:
    SdTpOptionsContents(TabPageParent pParent, const SfxItemSet& rInAttrs);
    virtual ~SdTpOptionsContents() override;

    static  VclPtr<SfxTabPage> Create( TabPageParent, const SfxItemSet* );
    virtual bool FillItemSet( SfxItemSet* ) override;
    virtual void Reset( const SfxItemSet * ) override;
};

/**
 * Option-Tab-Page: View
 */

class SdTpOptionsMisc : public SfxTabPage
{
 friend class SdModule;

private:
    sal_uInt32          nWidth;
    sal_uInt32          nHeight;
    OUString aInfo1;
    OUString aInfo2;

    MapUnit             ePoolUnit;

    std::unique_ptr<weld::CheckButton> m_xCbxQuickEdit;
    std::unique_ptr<weld::CheckButton> m_xCbxPickThrough;

    std::unique_ptr<weld::Frame> m_xNewDocumentFrame;
    std::unique_ptr<weld::CheckButton> m_xCbxStartWithTemplate;

    std::unique_ptr<weld::CheckButton> m_xCbxMasterPageCache;
    std::unique_ptr<weld::CheckButton> m_xCbxCopy;
    std::unique_ptr<weld::CheckButton> m_xCbxMarkedHitMovesAlways;
    std::unique_ptr<weld::Frame> m_xPresentationFrame;

    std::unique_ptr<weld::ComboBox> m_xLbMetric;
    std::unique_ptr<weld::MetricSpinButton> m_xMtrFldTabstop;

    std::unique_ptr<weld::CheckButton> m_xCbxEnableSdremote;
    std::unique_ptr<weld::CheckButton> m_xCbxEnablePresenterScreen;
    std::unique_ptr<weld::CheckButton> m_xCbxUsePrinterMetrics;
    std::unique_ptr<weld::CheckButton> m_xCbxCompatibility;

    //Scale
    std::unique_ptr<weld::Frame> m_xScaleFrame;
    std::unique_ptr<weld::ComboBox> m_xCbScale;
    std::unique_ptr<weld::Label> m_xNewDocLb;
    std::unique_ptr<weld::Label> m_xFiInfo1;
    std::unique_ptr<weld::MetricSpinButton> m_xMtrFldOriginalWidth;
    std::unique_ptr<weld::Label> m_xWidthLb;
    std::unique_ptr<weld::Label> m_xHeightLb;
    std::unique_ptr<weld::Label> m_xFiInfo2;
    std::unique_ptr<weld::MetricSpinButton> m_xMtrFldOriginalHeight;
    std::unique_ptr<weld::CheckButton> m_xCbxDistrot;
    std::unique_ptr<weld::MetricSpinButton> m_xMtrFldInfo1;
    std::unique_ptr<weld::MetricSpinButton> m_xMtrFldInfo2;

    static OUString        GetScale( sal_Int32 nX, sal_Int32 nY );
    static bool            SetScale( const OUString& aScale, sal_Int32& rX, sal_Int32& rY );

    DECL_LINK( SelectMetricHdl_Impl, weld::ComboBox&, void );

    /** Enable or disable the controls in the compatibility section of the
        'general' tab page depending on whether there is at least one
        document.
    */
    void UpdateCompatibilityControls();

protected:
    virtual void ActivatePage( const SfxItemSet& rSet ) override;
    virtual DeactivateRC DeactivatePage( SfxItemSet* pSet ) override;

public:
    SdTpOptionsMisc(TabPageParent pParent, const SfxItemSet& rInAttrs);
    virtual ~SdTpOptionsMisc() override;

    static  VclPtr<SfxTabPage> Create( TabPageParent, const SfxItemSet* );
    virtual bool FillItemSet( SfxItemSet* ) override;
    virtual void Reset( const SfxItemSet * ) override;

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
    virtual void        PageCreated(const SfxAllItemSet& aSet) override;

    using TabPage::ActivatePage;
    using TabPage::DeactivatePage;
    using OutputDevice::SetDrawMode;

};

#endif // INCLUDED_SD_SOURCE_UI_INC_TPOPTION_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
