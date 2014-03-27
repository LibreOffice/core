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

#include <vcl/fixed.hxx>

// #i75273#
#include <basegfx/range/b2drange.hxx>

// predefines
class SdrView;

/*************************************************************************
|*
|* Transform-Tab-Dialog
|*
\************************************************************************/

/** put this into the nAnchorTypes parameter of the SvxTransformTabDialog c'tor
    to disable the size controls */
const sal_uInt16 SVX_OBJ_NORESIZE = 0x0100;

/** put this into the nAnchorTypes parameter of the SvxTransformTabDialog c'tor
    to disable the protect controls */
const sal_uInt16 SVX_OBJ_NOPROTECT = 0x0200;

class SvxTransformTabDialog : public SfxTabDialog
{
    sal_uInt16         nPosSize;
    sal_uInt16         nSWPosSize;
    sal_uInt16         nRotation;
    sal_uInt16         nSlant;
private:
    const SdrView*      pView;

    sal_uInt16          nAnchorCtrls;
    Link                aValidateLink;

    virtual void        PageCreated( sal_uInt16 nId, SfxTabPage &rPage ) SAL_OVERRIDE;

public:

            SvxTransformTabDialog( Window* pParent, const SfxItemSet* pAttr,
                            const SdrView* pView,
                            sal_uInt16 nAnchorTypes = 0);
            ~SvxTransformTabDialog();

            //link for the Writer to validate positions
            void SetValidateFramePosLink( const Link& rLink );
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

private:
    // position
    VclFrame*            m_pFlPosition;
    MetricField*         m_pMtrPosX;
    MetricField*         m_pMtrPosY;
    SvxRectCtl*          m_pCtlPos;

    // size
    VclFrame*            m_pFlSize;
    FixedText*           m_pFtWidth;
    MetricField*         m_pMtrWidth;
    FixedText*           m_pFtHeight;
    MetricField*         m_pMtrHeight;
    CheckBox*            m_pCbxScale;
    SvxRectCtl*          m_pCtlSize;

    // protect
    VclFrame*            m_pFlProtect;
    TriStateBox*         m_pTsbPosProtect;
    TriStateBox*         m_pTsbSizeProtect;

    // adjust
    VclFrame*            m_pFlAdjust;
    TriStateBox*         m_pTsbAutoGrowWidth;
    TriStateBox*         m_pTsbAutoGrowHeight;

private:
    const SfxItemSet&   mrOutAttrs;

    const SdrView*      mpView;

    // #i75273#
    basegfx::B2DRange   maRange;
    basegfx::B2DRange   maWorkRange;
    basegfx::B2DPoint   maAnchor;

    SfxMapUnit          mePoolUnit;
    FieldUnit           meDlgUnit;
    MapUnit             meMapUnit;
    TriState            mnProtectSizeState;
    bool                mbPageDisabled;
    bool                mbProtectDisabled;
    bool                mbSizeDisabled;
    bool                mbAdjustDisabled;

    // frome size
    // #i75273#
    double              mfOldWidth;
    double              mfOldHeight;
    RECT_POINT          meRP;


    DECL_LINK( ChangePosProtectHdl, void * );
    DECL_LINK( ChangeSizeProtectHdl, void * );

    void SetMinMaxPosition();
    void GetTopLeftPosition(double& rfX, double& rfY, const basegfx::B2DRange& rRange);

    DECL_LINK( ChangeWidthHdl, void * );
    DECL_LINK( ChangeHeightHdl, void * );
    DECL_LINK( ClickSizeProtectHdl, void * );
    DECL_LINK( ClickAutoHdl, void * );

    void        SetMaxSize( Rectangle aRect );

public:
    SvxPositionSizeTabPage( Window* pParent, const SfxItemSet& rInAttrs  );

    static SfxTabPage* Create( Window*, const SfxItemSet& );
    static sal_uInt16*     GetRanges();

    virtual bool FillItemSet( SfxItemSet& ) SAL_OVERRIDE;
    virtual void Reset( const SfxItemSet & ) SAL_OVERRIDE;

    virtual void ActivatePage( const SfxItemSet& rSet ) SAL_OVERRIDE;
    virtual int  DeactivatePage( SfxItemSet* pSet ) SAL_OVERRIDE;

    virtual void PointChanged( Window* pWindow, RECT_POINT eRP ) SAL_OVERRIDE;

    void         Construct();
    void         SetView( const SdrView* pSdrView ) { mpView = pSdrView; }

    virtual void FillUserData() SAL_OVERRIDE;

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

private:
    VclFrame*            m_pFlPosition;
    MetricField*         m_pMtrPosX;
    MetricField*         m_pMtrPosY;
    SvxRectCtl*          m_pCtlRect;

    VclFrame*            m_pFlAngle;
    NumericField*        m_pNfAngle;
    svx::DialControl*    m_pCtlAngle;

    const SfxItemSet&   rOutAttrs;
    const SdrView*      pView;

    // #i75273#
    basegfx::B2DRange   maRange;
    basegfx::B2DPoint   maAnchor;

    SfxMapUnit          ePoolUnit;
    FieldUnit           eDlgUnit;

public:
         SvxAngleTabPage( Window* pParent, const SfxItemSet& rInAttrs  );

    static SfxTabPage* Create( Window*, const SfxItemSet& );
    static sal_uInt16*     GetRanges();

    virtual bool FillItemSet( SfxItemSet& ) SAL_OVERRIDE;
    virtual void Reset( const SfxItemSet & ) SAL_OVERRIDE;

    virtual void ActivatePage( const SfxItemSet& rSet ) SAL_OVERRIDE;
    virtual int  DeactivatePage( SfxItemSet* pSet ) SAL_OVERRIDE;

    virtual void PointChanged( Window* pWindow, RECT_POINT eRP ) SAL_OVERRIDE;

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

private:
    VclFrame*            m_pFlRadius;
    MetricField*         m_pMtrRadius;
    VclFrame*            m_pFlAngle;
    MetricField*         m_pMtrAngle;

    const SfxItemSet&   rOutAttrs;

    const SdrView*      pView;

    // #i75273#
    basegfx::B2DRange   maRange;

    SfxMapUnit          ePoolUnit;
    FieldUnit           eDlgUnit;

public:
         SvxSlantTabPage( Window* pParent, const SfxItemSet& rInAttrs  );

    static SfxTabPage* Create( Window*, const SfxItemSet& );
    static sal_uInt16*     GetRanges();

    virtual bool FillItemSet( SfxItemSet& ) SAL_OVERRIDE;
    virtual void Reset( const SfxItemSet & ) SAL_OVERRIDE;

    virtual void ActivatePage( const SfxItemSet& rSet ) SAL_OVERRIDE;
    virtual int  DeactivatePage( SfxItemSet* pSet ) SAL_OVERRIDE;

    virtual void PointChanged( Window* pWindow, RECT_POINT eRP ) SAL_OVERRIDE;

    void         Construct();
    void         SetView( const SdrView* pSdrView ) { pView = pSdrView; }
};



#endif // INCLUDED_CUI_SOURCE_INC_TRANSFRM_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
