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
            SdTpOptionsSnap( Window* pParent, const SfxItemSet& rInAttrs  );
            virtual ~SdTpOptionsSnap();

    static  SfxTabPage* Create( Window*, const SfxItemSet& );
    virtual bool FillItemSet( SfxItemSet& ) SAL_OVERRIDE;
    virtual void Reset( const SfxItemSet & ) SAL_OVERRIDE;

};

/**
 * Option-Tab-Page: Contents
 */
class SdTpOptionsContents : public SfxTabPage
{
private:
    CheckBox*    m_pCbxRuler;
    CheckBox*    m_pCbxDragStripes;
    CheckBox*    m_pCbxHandlesBezier;
    CheckBox*    m_pCbxMoveOutline;


public:
            SdTpOptionsContents( Window* pParent, const SfxItemSet& rInAttrs  );
            virtual ~SdTpOptionsContents();

    static  SfxTabPage* Create( Window*, const SfxItemSet& );
    virtual bool FillItemSet( SfxItemSet& ) SAL_OVERRIDE;
    virtual void Reset( const SfxItemSet & ) SAL_OVERRIDE;
};

/**
 * Option-Tab-Page: View
 */
class SdModule;
class SdTpOptionsMisc : public SfxTabPage
{
 friend class SdModule;

private:
    CheckBox    *m_pCbxQuickEdit;
    CheckBox    *m_pCbxPickThrough;

    VclFrame    *m_pNewDocumentFrame;
    CheckBox    *m_pCbxStartWithTemplate;

    CheckBox    *m_pCbxMasterPageCache;
    CheckBox    *m_pCbxCopy;
    CheckBox    *m_pCbxMarkedHitMovesAlways;
    VclFrame    *m_pPresentationFrame;

    ListBox     *m_pLbMetric;
    MetricField *m_pMtrFldTabstop;

    CheckBox    *m_pCbxEnableSdremote;
    CheckBox    *m_pCbxEnablePresenterScreen;
    CheckBox    *m_pCbxUsePrinterMetrics;
    CheckBox    *m_pCbxCompatibility;

    //Scale
    VclFrame    *m_pScaleFrame;
    ComboBox    *m_pCbScale;
    FixedText   *m_pNewDocLb;
    FixedText   *m_pFiInfo1;
    MetricField *m_pMtrFldOriginalWidth;
    FixedText   *m_pWidthLb;
    FixedText   *m_pHeightLb;
    FixedText   *m_pFiInfo2;
    MetricField *m_pMtrFldOriginalHeight;
    CheckBox    *m_pCbxDistrot;
    MetricField *m_pMtrFldInfo1;
    MetricField *m_pMtrFldInfo2;

    sal_uInt32          nWidth;
    sal_uInt32          nHeight;
    OUString aInfo1;
    OUString aInfo2;

    SfxMapUnit          ePoolUnit;

    OUString        GetScale( sal_Int32 nX, sal_Int32 nY );
    sal_Bool            SetScale( const OUString& aScale, sal_Int32& rX, sal_Int32& rY );

    DECL_LINK( SelectMetricHdl_Impl, void * );

    /** Enable or disable the controls in the compatibility section of the
        'general' tab page depending on whether there is at least one
        document.
    */
    void UpdateCompatibilityControls (void);

protected:
    virtual void ActivatePage( const SfxItemSet& rSet ) SAL_OVERRIDE;
    virtual int DeactivatePage( SfxItemSet* pSet ) SAL_OVERRIDE;

public:
            SdTpOptionsMisc( Window* pParent, const SfxItemSet& rInAttrs  );
            virtual ~SdTpOptionsMisc();

    static  SfxTabPage* Create( Window*, const SfxItemSet& );
    virtual bool FillItemSet( SfxItemSet& ) SAL_OVERRIDE;
    virtual void Reset( const SfxItemSet & ) SAL_OVERRIDE;

    /** Hide Impress specific controls, make Draw specific controls visible
        and arrange the visible controls.  Do not call this method or the
        <member>SetImpressMode()</member> method more than once.
    */
    void SetDrawMode (void);

    /** Hide Draw specific controls, make Impress specific controls visible
        and arrange the visible controls.  Do not call this method or the
        <member>SetDrawMode()</member> method more than once.
    */
    void SetImpressMode (void);
    virtual void        PageCreated (SfxAllItemSet aSet) SAL_OVERRIDE;

    using TabPage::ActivatePage;
    using TabPage::DeactivatePage;
    using OutputDevice::SetDrawMode;

};


#endif // INCLUDED_SD_SOURCE_UI_INC_TPOPTION_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
