/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/
#ifndef _SVX_TRANSFRM_HXX
#define _SVX_TRANSFRM_HXX

#include <svx/dlgctrl.hxx>

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
private:
    const SdrView*      pView;

    sal_uInt16              nAnchorCtrls;
    Link                aValidateLink;

    virtual void        PageCreated( sal_uInt16 nId, SfxTabPage &rPage );

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
    FixedLine           maFlPosition;
    FixedText           maFtPosX;
    MetricField         maMtrPosX;
    FixedText           maFtPosY;
    MetricField         maMtrPosY;
    FixedText           maFtPosReference;
    SvxRectCtl          maCtlPos;

    // size
    FixedLine           maFlSize;
    FixedText           maFtWidth;
    MetricField         maMtrWidth;
    FixedText           maFtHeight;
    MetricField         maMtrHeight;
    CheckBox            maCbxScale;
    FixedText           maFtSizeReference;
    SvxRectCtl          maCtlSize;

    // protect
    FixedLine           maFlProtect;
    TriStateBox         maTsbPosProtect;
    TriStateBox         maTsbSizeProtect;

    // adjust
    FixedLine           maFlAdjust;
    TriStateBox         maTsbAutoGrowWidth;
    TriStateBox         maTsbAutoGrowHeight;

    FixedLine           maFlDivider;

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

    //------------------------------------
#if _SOLAR__PRIVATE
    DECL_LINK( ChangePosProtectHdl, void * );
    DECL_LINK( ChangeSizeProtectHdl, void * );

    void SetMinMaxPosition();
    void GetTopLeftPosition(double& rfX, double& rfY, const basegfx::B2DRange& rRange);
#endif

#if _SOLAR__PRIVATE
    DECL_LINK( ChangeWidthHdl, void * );
    DECL_LINK( ChangeHeightHdl, void * );
    DECL_LINK( ClickSizeProtectHdl, void * );
    DECL_LINK( ClickAutoHdl, void * );

    void        SetMaxSize( Rectangle aRect );
    Rectangle   GetRect();
#endif

public:
    SvxPositionSizeTabPage( Window* pParent, const SfxItemSet& rInAttrs  );

    static SfxTabPage* Create( Window*, const SfxItemSet& );
    static sal_uInt16*     GetRanges();

    virtual sal_Bool FillItemSet( SfxItemSet& );
    virtual void Reset( const SfxItemSet & );

    virtual void ActivatePage( const SfxItemSet& rSet );
    virtual int  DeactivatePage( SfxItemSet* pSet );

    virtual void PointChanged( Window* pWindow, RECT_POINT eRP );

    void         Construct();
    void         SetView( const SdrView* pSdrView ) { mpView = pSdrView; }

    virtual void FillUserData();

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
    FixedLine           aFlPosition;
    FixedText           aFtPosX;
    MetricField         aMtrPosX;
    FixedText           aFtPosY;
    MetricField         aMtrPosY;
    FixedText           aFtPosPresets;
    SvxRectCtl          aCtlRect;

    FixedLine           aFlAngle;
    FixedText           aFtAngle;
    MetricField         aMtrAngle;
    FixedText           aFtAnglePresets;
    SvxRectCtl          aCtlAngle;

    const SfxItemSet&   rOutAttrs;
    const SdrView*      pView;

    // #i75273#
    basegfx::B2DRange   maRange;
    basegfx::B2DPoint   maAnchor;

    SfxMapUnit          ePoolUnit;
    FieldUnit           eDlgUnit;
    MapUnit             eMapUnit;
    //------------------------------------
#if _SOLAR__PRIVATE
    DECL_LINK( ModifiedHdl, void * );
#endif
public:
         SvxAngleTabPage( Window* pParent, const SfxItemSet& rInAttrs  );

    static SfxTabPage* Create( Window*, const SfxItemSet& );
    static sal_uInt16*     GetRanges();

    virtual sal_Bool FillItemSet( SfxItemSet& );
    virtual void Reset( const SfxItemSet & );

    virtual void ActivatePage( const SfxItemSet& rSet );
    virtual int  DeactivatePage( SfxItemSet* pSet );

    virtual void PointChanged( Window* pWindow, RECT_POINT eRP );

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
    FixedLine           aFlRadius;
    FixedText           aFtRadius;
    MetricField         aMtrRadius;
    FixedLine           aFlAngle;
    FixedText           aFtAngle;
    MetricField         aMtrAngle;

    const SfxItemSet&   rOutAttrs;

    const SdrView*      pView;

    // #i75273#
    basegfx::B2DRange   maRange;

    SfxMapUnit          ePoolUnit;
    FieldUnit           eDlgUnit;
    MapUnit             eMapUnit;
    //------------------------------------
public:
         SvxSlantTabPage( Window* pParent, const SfxItemSet& rInAttrs  );

    static SfxTabPage* Create( Window*, const SfxItemSet& );
    static sal_uInt16*     GetRanges();

    virtual sal_Bool FillItemSet( SfxItemSet& );
    virtual void Reset( const SfxItemSet & );

    virtual void ActivatePage( const SfxItemSet& rSet );
    virtual int  DeactivatePage( SfxItemSet* pSet );

    virtual void PointChanged( Window* pWindow, RECT_POINT eRP );

    void         Construct();
    void         SetView( const SdrView* pSdrView ) { pView = pSdrView; }
};



#endif // _SVX_TRANSFRM_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
