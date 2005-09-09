/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: animobjs.hxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 05:21:56 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#ifndef SD_ANIMOBJS_HXX
#define SD_ANIMOBJS_HXX

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
#ifndef _SFX_PROGRESS_HXX
#include <sfx2/progress.hxx>
#endif


#ifndef _SV_LSTBOX_HXX //autogen
#include <vcl/lstbox.hxx>
#endif

#ifndef _SD_SDRESID_HXX
#include "sdresid.hxx"
#endif

class SdDrawDocument;
class BitmapEx;

namespace sd {

class AnimationControllerItem;
class View;

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
    BitmapEx    aBitmapEx;
    Fraction    aScale;

public:
    SdDisplay( ::Window* pWin, SdResId Id );
    ~SdDisplay();

    virtual void Paint( const Rectangle& rRect );

    void    SetBitmapEx( BitmapEx* pBmpEx );
    void    SetScale( const Fraction& rFrac );

    virtual void DataChanged( const DataChangedEvent& rDCEvt );
};

//------------------------------------------------------------------------

class AnimationWindow : public SfxDockingWindow
{
 friend class AnimationChildWindow;
 friend class AnimationControllerItem;

public:
    AnimationWindow( SfxBindings* pBindings, SfxChildWindow *pCW,
        ::Window* pParent, const SdResId& rSdResId );
    virtual ~AnimationWindow();

    void    AddObj( ::sd::View& rView );
    void    CreateAnimObj( ::sd::View& rView );

    virtual void DataChanged( const DataChangedEvent& rDCEvt );

protected:
    virtual BOOL    Close();
    virtual void    Resize();
    virtual void    FillInfo( SfxChildWinInfo& ) const;

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
    FixedLine       aGrpBitmap;

    RadioButton     aRbtGroup;
    RadioButton     aRbtBitmap;
    FixedText       aFtAdjustment;
    ListBox         aLbAdjustment;
    PushButton      aBtnCreateGroup;
    FixedLine       aGrpAnimation;

    ::Window*       pWin;
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
    AnimationControllerItem*    pControllerItem;

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
};

/*************************************************************************
|*
|* ControllerItem fuer Animator
|*
\************************************************************************/

class AnimationControllerItem : public SfxControllerItem
{

public:
    AnimationControllerItem( USHORT, AnimationWindow*, SfxBindings* );

protected:
    virtual void StateChanged( USHORT nSId, SfxItemState eState,
        const SfxPoolItem* pState );
private:
    AnimationWindow* pAnimationWin;
};

} // end of namespace sd

#endif

