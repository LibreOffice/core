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
#ifndef INCLUDED_CUI_SOURCE_INC_TRANSFRM_HXX
#define INCLUDED_CUI_SOURCE_INC_TRANSFRM_HXX

#include <svx/dlgctrl.hxx>
#include <svx/dialcontrol.hxx>
#include <svx/anchorid.hxx>

#include <vcl/fixed.hxx>

#include <basegfx/range/b2drange.hxx>

// predefines
class SdrView;

/*************************************************************************
|*
|* Transform-Tab-Dialog
|*
\************************************************************************/

struct SvxSwFrameValidation;
class SvxTransformTabDialog : public SfxTabDialog
{
    sal_uInt16         nPosSize;
    sal_uInt16         nSWPosSize;
    sal_uInt16         nRotation;
    sal_uInt16         nSlant;
private:
    const SdrView*      pView;

    SvxAnchorIds        nAnchorCtrls;
    Link<SvxSwFrameValidation&,void> aValidateLink;

    virtual void        PageCreated( sal_uInt16 nId, SfxTabPage &rPage ) override;

public:

            SvxTransformTabDialog( vcl::Window* pParent, const SfxItemSet* pAttr,
                            const SdrView* pView,
                            SvxAnchorIds nAnchorTypes);

            //link for the Writer to validate positions
            void SetValidateFramePosLink( const Link<SvxSwFrameValidation&,void>& rLink );
};

/*************************************************************************
|*
|* position and size tab page
|*
\************************************************************************/

class SvxPositionSizeTabPage : public SvxTabPage
{
    using TabPage::ActivatePage;
    using TabPage::DeactivatePage;
    static const sal_uInt16 pPosSizeRanges[];

private:
    const SfxItemSet&   mrOutAttrs;

    const SdrView*      mpView;

    // #i75273#
    basegfx::B2DRange   maRange;
    basegfx::B2DRange   maWorkRange;
    basegfx::B2DPoint   maAnchor;

    MapUnit             mePoolUnit;
    FieldUnit           meDlgUnit;
    TriState            mnProtectSizeState;
    bool                mbPageDisabled;
    bool                mbProtectDisabled;
    bool                mbSizeDisabled;
    bool                mbAdjustDisabled;

    // from size
    // #i75273#
    double              mfOldWidth;
    double              mfOldHeight;
    RectPoint          meRP;

    RectCtl          m_aCtlPos;
    RectCtl          m_aCtlSize;

    // position
    std::unique_ptr<weld::Widget> m_xFlPosition;
    std::unique_ptr<weld::MetricSpinButton> m_xMtrPosX;
    std::unique_ptr<weld::MetricSpinButton> m_xMtrPosY;
    std::unique_ptr<weld::CustomWeld> m_xCtlPos;

    // size
    std::unique_ptr<weld::Widget> m_xFlSize;
    std::unique_ptr<weld::Label> m_xFtWidth;
    std::unique_ptr<weld::MetricSpinButton> m_xMtrWidth;
    std::unique_ptr<weld::Label> m_xFtHeight;
    std::unique_ptr<weld::MetricSpinButton> m_xMtrHeight;
    std::unique_ptr<weld::CheckButton> m_xCbxScale;
    std::unique_ptr<weld::CustomWeld> m_xCtlSize;

    // protect
    std::unique_ptr<weld::Widget> m_xFlProtect;
    std::unique_ptr<weld::CheckButton> m_xTsbPosProtect;
    std::unique_ptr<weld::CheckButton> m_xTsbSizeProtect;

    // adjust
    std::unique_ptr<weld::Widget> m_xFlAdjust;
    std::unique_ptr<weld::CheckButton> m_xTsbAutoGrowWidth;
    std::unique_ptr<weld::CheckButton> m_xTsbAutoGrowHeight;

    DECL_LINK(ChangePosProtectHdl, weld::ToggleButton&, void);
    DECL_LINK(ChangeSizeProtectHdl, weld::ToggleButton&, void);

    void SetMinMaxPosition();
    void GetTopLeftPosition(double& rfX, double& rfY, const basegfx::B2DRange& rRange);

    DECL_LINK( ChangeWidthHdl, weld::MetricSpinButton&, void );
    DECL_LINK( ChangeHeightHdl, weld::MetricSpinButton&, void );
    DECL_LINK( ClickSizeProtectHdl, weld::ToggleButton&, void );
    DECL_LINK( ClickAutoHdl, weld::ToggleButton&, void );

public:
    SvxPositionSizeTabPage(TabPageParent pParent, const SfxItemSet& rInAttrs);
    virtual ~SvxPositionSizeTabPage() override;

    static VclPtr<SfxTabPage> Create( TabPageParent, const SfxItemSet* );
    static const sal_uInt16* GetRanges() {  return pPosSizeRanges; }

    virtual bool FillItemSet( SfxItemSet* ) override;
    virtual void Reset( const SfxItemSet * ) override;

    virtual void ActivatePage( const SfxItemSet& rSet ) override;
    virtual DeactivateRC DeactivatePage( SfxItemSet* pSet ) override;

    virtual void PointChanged( weld::DrawingArea* pWindow, RectPoint eRP ) override;
    virtual void PointChanged( vcl::Window* pWindow, RectPoint eRP ) override;

    void         Construct();
    void         SetView( const SdrView* pSdrView ) { mpView = pSdrView; }

    virtual void FillUserData() override;

    void        DisableResize();
    void        DisableProtect();

    void        UpdateControlStates();
};

/*************************************************************************
|*
|* rotation angle tab page
|*
\************************************************************************/
class SvxAngleTabPage : public SvxTabPage
{
    using TabPage::ActivatePage;
    using TabPage::DeactivatePage;
    static const sal_uInt16 pAngleRanges[];

private:
    VclPtr<VclFrame>            m_pFlPosition;
    VclPtr<MetricField>         m_pMtrPosX;
    VclPtr<MetricField>         m_pMtrPosY;
    VclPtr<SvxRectCtl>          m_pCtlRect;

    VclPtr<VclFrame>            m_pFlAngle;
    VclPtr<NumericField>        m_pNfAngle;
    VclPtr<svx::DialControl>    m_pCtlAngle;

    const SfxItemSet&   rOutAttrs;
    const SdrView*      pView;

    // #i75273#
    basegfx::B2DRange   maRange;
    basegfx::B2DPoint   maAnchor;

    MapUnit             ePoolUnit;
    FieldUnit           eDlgUnit;

public:
         SvxAngleTabPage( vcl::Window* pParent, const SfxItemSet& rInAttrs  );
    virtual ~SvxAngleTabPage() override;
    virtual void dispose() override;

    static VclPtr<SfxTabPage> Create( TabPageParent, const SfxItemSet* );
    static const sal_uInt16*  GetRanges() { return pAngleRanges; }

    virtual bool FillItemSet( SfxItemSet* ) override;
    virtual void Reset( const SfxItemSet * ) override;

    virtual void ActivatePage( const SfxItemSet& rSet ) override;
    virtual DeactivateRC DeactivatePage( SfxItemSet* pSet ) override;

    virtual void PointChanged( vcl::Window* pWindow, RectPoint eRP ) override;
    virtual void PointChanged( weld::DrawingArea* pWindow, RectPoint eRP ) override;

    void         Construct();
    void         SetView( const SdrView* pSdrView ) { pView = pSdrView; }
};

/*************************************************************************
|*
|* slant/corner radius tab page
|*
\************************************************************************/
class SvxSlantTabPage : public SvxTabPage
{
    using TabPage::ActivatePage;
    using TabPage::DeactivatePage;
    static const sal_uInt16 pSlantRanges[];

private:
    VclPtr<VclFrame>            m_pFlRadius;
    VclPtr<MetricField>         m_pMtrRadius;
    VclPtr<VclFrame>            m_pFlAngle;
    VclPtr<MetricField>         m_pMtrAngle;

    VclPtr<VclFrame>            m_aControlGroups[2];
    VclPtr<VclContainer>        m_aControlGroupX[2];
    VclPtr<MetricField>         m_aControlX[2];
    VclPtr<VclContainer>        m_aControlGroupY[2];
    VclPtr<MetricField>         m_aControlY[2];

    const SfxItemSet&   rOutAttrs;

    const SdrView*      pView;

    // #i75273#
    basegfx::B2DRange   maRange;

    MapUnit             ePoolUnit;
    FieldUnit           eDlgUnit;

public:
         SvxSlantTabPage( vcl::Window* pParent, const SfxItemSet& rInAttrs  );
    virtual ~SvxSlantTabPage() override;
    virtual void dispose() override;

    static VclPtr<SfxTabPage> Create( TabPageParent, const SfxItemSet* );
    static const sal_uInt16* GetRanges() {  return pSlantRanges; }

    virtual bool FillItemSet( SfxItemSet* ) override;
    virtual void Reset( const SfxItemSet * ) override;

    virtual void ActivatePage( const SfxItemSet& rSet ) override;
    virtual DeactivateRC DeactivatePage( SfxItemSet* pSet ) override;

    virtual void PointChanged( vcl::Window* pWindow, RectPoint eRP ) override;
    virtual void PointChanged( weld::DrawingArea* pWindow, RectPoint eRP ) override;

    void         Construct();
    void         SetView( const SdrView* pSdrView ) { pView = pSdrView; }
};


#endif // INCLUDED_CUI_SOURCE_INC_TRANSFRM_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
