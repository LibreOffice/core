/*************************************************************************
 *
 *  $RCSfile: animobjs.hxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:48:37 $
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


#ifndef _SD_ANIMOBJS_HXX
#define _SD_ANIMOBJS_HXX

#ifndef _SFXDOCKWIN_HXX //autogen
#include <sfx2/dockwin.hxx>
#endif
#ifndef _SV_FIXED_HXX //autogen
#include <vcl/fixed.hxx>
#endif
#ifndef _STDCTRL_HXX //autogen
#include <svtools/stdctrl.hxx>
#endif
#ifndef _SV_GROUP_HXX //autogen
#include <vcl/group.hxx>
#endif
#ifndef _SFXCTRLITEM_HXX //autogen
#include <sfx2/ctrlitem.hxx>
#endif

#ifndef _SV_BUTTON_HXX //autogen
#include <vcl/button.hxx>
#endif

#ifndef _SV_FIELD_HXX //autogen
#include <vcl/field.hxx>
#endif

#ifndef _SVX_DLG_CTRL_HXX //autogen
#include <svx/dlgctrl.hxx>
#endif


#ifndef _SV_LSTBOX_HXX //autogen
#include <vcl/lstbox.hxx>
#endif

#ifndef _SD_SDRESID_HXX
#include "sdresid.hxx"
#endif

class SdDrawDocument;
class SdView;
class BitmapEx;

//------------------------------------------------------------------------

enum BitmapAdjustment
{
    BA_LEFT_UP,
    BA_LEFT,
    BA_LEFT_DOWN,
    BA_UP,
    BA_CENTER,
    BA_DOWN,
    BA_RIGHT_UP,
    BA_RIGHT,
    BA_RIGHT_DOWN
};

//------------------------------------------------------------------------

class SdDisplay : public Control
{
private:
    BitmapEx*   pBitmapEx;
    Fraction    aScale;

public:
    SdDisplay( Window* pWin, SdResId Id );
    ~SdDisplay();

    virtual void Paint( const Rectangle& rRect );

    void    SetBitmapEx( BitmapEx* pBmpEx ) { pBitmapEx = pBmpEx; }
    void    SetScale( const Fraction& rFrac );
};

//------------------------------------------------------------------------

class SdAnimationWin : public SfxDockingWindow
{
 friend class SdAnimationChildWindow;
 friend class SdAnimationControllerItem;

private:
    SdDisplay       aCtlDisplay;
    ImageButton     aBtnFirst;
    ImageButton     aBtnReverse;
    ImageButton     aBtnStop;
    ImageButton     aBtnPlay;
    ImageButton     aBtnLast;
    NumericField    aNumFldBitmap;
    TimeField       aTimeField;
    ListBox         aLbLoopCount;

    ImageButton     aBtnGetOneObject;
    ImageButton     aBtnGetAllObjects;
    ImageButton     aBtnRemoveBitmap;
    ImageButton     aBtnRemoveAll;
    FixedText       aFtCount;
    FixedInfo       aFiCount;
    GroupBox        aGrpBitmap;

    RadioButton     aRbtGroup;
    RadioButton     aRbtBitmap;
    FixedText       aFtAdjustment;
    ListBox         aLbAdjustment;
    FixedText       aFtColor;
    ColorLB         aLbColor;
    PushButton      aBtnCreateGroup;
    GroupBox        aGrpAnimation;

    Window*         pWin;
    List            aBmpExList;
    List            aTimeList;
    SdDrawDocument* pMyDoc;
    BitmapEx*       pBitmapEx;

    Size            aSize;
    Size            aFltWinSize;
    Size            aDisplaySize;
    Size            aBmpSize;
    BOOL            bMovie;
    BOOL            bAllObjects;

    SfxBindings*                pBindings;
    SdAnimationControllerItem*  pControllerItem;

    //------------------------------------

    DECL_LINK( ClickFirstHdl, void * );
    DECL_LINK( ClickStopHdl, void * );
    DECL_LINK( ClickPlayHdl, void * );
    DECL_LINK( ClickLastHdl, void * );
    DECL_LINK( ClickGetObjectHdl, void * );
    DECL_LINK( ClickRemoveBitmapHdl, void * );
    DECL_LINK( ClickRbtHdl, void * );
    DECL_LINK( ClickCreateGroupHdl, void * );
    DECL_LINK( ModifyBitmapHdl, void * );
    DECL_LINK( ModifyTimeHdl, void * );

    void            UpdateControl( ULONG nPos, BOOL bDisableCtrls = FALSE );
    void            ResetAttrs();
    void            WaitInEffect( ULONG nMilliSeconds ) const;
    void            WaitInEffect( ULONG nMilliSeconds, ULONG nTime,
                                        SfxProgress* pStbMgr ) const;
    Fraction        GetScale();

protected:
    virtual BOOL    Close();
    virtual void    Resize();
    virtual void    FillInfo( SfxChildWinInfo& ) const;

public:
            SdAnimationWin( SfxBindings* pBindings, SfxChildWindow *pCW,
                        Window* pParent, const SdResId& rSdResId );
            ~SdAnimationWin();

    void    InitColorLB( const SdDrawDocument* pDoc );
    void    AddObj( SdView& rView );
    void    CreateAnimObj( SdView& rView );
};

/*************************************************************************
|*
|* Ableitung vom SfxChildWindow als "Behaelter" fuer Animator
|*
\************************************************************************/

class SdAnimationChildWindow : public SfxChildWindow
{
 public:
    SdAnimationChildWindow( Window*, USHORT, SfxBindings*,
                            SfxChildWinInfo*);

    SFX_DECL_CHILDWINDOW(SdAnimationChildWindow);
};

/*************************************************************************
|*
|* ControllerItem fuer Animator
|*
\************************************************************************/

class SdAnimationControllerItem : public SfxControllerItem
{
    SdAnimationWin* pAnimationWin;

 protected:
    virtual void StateChanged( USHORT nSId, SfxItemState eState,
                                const SfxPoolItem* pState );

 public:
    SdAnimationControllerItem( USHORT, SdAnimationWin*, SfxBindings* );
};

#endif      // _SD_ANIMOBJS_HXX

