/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/


#ifndef _SVX_TRANSFRM_HXX
#define _SVX_TRANSFRM_HXX

// include ---------------------------------------------------------------

#include <svx/dlgctrl.hxx>

#ifndef _FIXED_HXX //autogen
#include <vcl/fixed.hxx>
#endif

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

    // from size
    // #i75273#
    double              mfOldWidth;
    double              mfOldHeight;
    RECT_POINT          meRP;

    //------------------------------------
#if _SOLAR__PRIVATE
    DECL_LINK( ChangePosProtectHdl, void * );
    DECL_LINK( ChangeSizeProtectHdl, void * );
    DECL_LINK( ChangePosXHdl, void * );
    DECL_LINK( ChangePosYHdl, void * );
//  DECL_LINK( SetAnchorHdl, ListBox * );
//  DECL_LINK( SetOrientHdl, ListBox * );

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

//  void         ShowAnchorCtrls(sal_uInt16 nAnchorCtrls); // Writer-spezifische Controls anzeigen
    virtual void FillUserData();

    void        DisableResize();
    void        DisableProtect();

    void        UpdateControlStates();
};

/*************************************************************************
|*
|* Drehwinkel-Tab-Page
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
|* Schraegstellen/Eckenradius-Tab-Page
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
    //TriStateBox           aTsbVertical;
    FixedLine           aFlAngle;
    FixedText           aFtAngle;
    MetricField         aMtrAngle;
    //SvxRectCtl            aCtlAngle;

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

