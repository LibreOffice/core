/*************************************************************************
 *
 *  $RCSfile: transfrm.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: hjs $ $Date: 2004-06-28 14:07:02 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/
#ifndef _SVX_TRANSFRM_HXX
#define _SVX_TRANSFRM_HXX

// include ---------------------------------------------------------------

#include "dlgctrl.hxx"

#ifndef _FIXED_HXX //autogen
#include <vcl/fixed.hxx>
#endif
class SdrView;

/*************************************************************************
|*
|* Transform-Tab-Dialog
|*
\************************************************************************/

/** put this into the nAnchorTypes parameter of the SvxTransformTabDialog c'tor
    to disable the size controls */
const USHORT SVX_OBJ_NORESIZE = 0x0100;

/** put this into the nAnchorTypes parameter of the SvxTransformTabDialog c'tor
    to disable the protect controls */
const USHORT SVX_OBJ_NOPROTECT = 0x0200;

class SvxTransformTabDialog : public SfxTabDialog
{
private:
    const SdrView*      pView;

    USHORT              nAnchorCtrls;
    Link                aValidateLink;

    virtual void        PageCreated( USHORT nId, SfxTabPage &rPage );

public:

            SvxTransformTabDialog( Window* pParent, const SfxItemSet* pAttr,
                            const SdrView* pView,
                            USHORT nAnchorTypes = 0);
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

/*  // anchor
    FixedLine           maAnchorBox;
    FixedText           maFtAnchor;
    ListBox             maDdLbAnchor;
*/
    // protect
    FixedLine           maFlProtect;
    TriStateBox         maTsbPosProtect;
    TriStateBox         maTsbSizeProtect;

    // adjust
    FixedLine           maFlAdjust;
    TriStateBox         maTsbAutoGrowWidth;
    TriStateBox         maTsbAutoGrowHeight;

    // ???
//  FixedText           maFtOrient;
//  ListBox             maDdLbOrient;

    FixedLine           maFlDivider;

private:
    const SfxItemSet&   mrOutAttrs;

    const SdrView*      mpView;
    Rectangle           maRect;
    Rectangle           maWorkArea;

    Point               maAnchorPos;
    SfxMapUnit          mePoolUnit;
    FieldUnit           meDlgUnit;
    MapUnit             meMapUnit;
    TriState            mnProtectSizeState;
    bool                mbPageDisabled;
    bool                mbProtectDisabled;
    bool                mbSizeDisabled;

    // frome size
    UINT32              mlOldWidth;
    UINT32              mlOldHeight;
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
    void GetTopLeftPosition( INT32& rX, INT32& rY, const Rectangle& rRect );
#endif

#if _SOLAR__PRIVATE
    DECL_LINK( ChangeWidthHdl, void * );
    DECL_LINK( ChangeHeightHdl, void * );
    DECL_LINK( ClickSizeProtectHdl, void * );
    DECL_LINK( ClickAutoHdl, void * );

    void        DisableSizeControls();
    void        SetMaxSize( Rectangle aRect );
    Rectangle   GetRect();
#endif

public:
    SvxPositionSizeTabPage( Window* pParent, const SfxItemSet& rInAttrs  );

    static SfxTabPage* Create( Window*, const SfxItemSet& );
    static USHORT*     GetRanges();

    virtual BOOL FillItemSet( SfxItemSet& );
    virtual void Reset( const SfxItemSet & );

    virtual void ActivatePage( const SfxItemSet& rSet );
    virtual int  DeactivatePage( SfxItemSet* pSet );

    virtual void PointChanged( Window* pWindow, RECT_POINT eRP );

    void         Construct();
    void         SetView( const SdrView* pSdrView ) { mpView = pSdrView; }

//  void         ShowAnchorCtrls(USHORT nAnchorCtrls); // Writer-spezifische Controls anzeigen
    virtual void FillUserData();

    void        DisableResize();
    void        DisableProtect();
};

/*************************************************************************
|*
|* Drehwinkel-Tab-Page
|*
\************************************************************************/
class SvxAngleTabPage : public SvxTabPage
{
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
    Rectangle           aRect;

    Point               aAnchorPos;
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
    static USHORT*     GetRanges();

    virtual BOOL FillItemSet( SfxItemSet& );
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
    Rectangle           aRect;

    SfxMapUnit          ePoolUnit;
    FieldUnit           eDlgUnit;
    MapUnit             eMapUnit;
    //------------------------------------
#if _SOLAR__PRIVATE
    DECL_LINK( ModifiedHdl, void * );
#endif
public:
         SvxSlantTabPage( Window* pParent, const SfxItemSet& rInAttrs  );

    static SfxTabPage* Create( Window*, const SfxItemSet& );
    static USHORT*     GetRanges();

    virtual BOOL FillItemSet( SfxItemSet& );
    virtual void Reset( const SfxItemSet & );

    virtual void ActivatePage( const SfxItemSet& rSet );
    virtual int  DeactivatePage( SfxItemSet* pSet );

    virtual void PointChanged( Window* pWindow, RECT_POINT eRP );

    void         Construct();
    void         SetView( const SdrView* pSdrView ) { pView = pSdrView; }
};



#endif // _SVX_TRANSFRM_HXX

