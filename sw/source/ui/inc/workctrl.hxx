/*************************************************************************
 *
 *  $RCSfile: workctrl.hxx,v $
 *
 *  $Revision: 1.8 $
 *
 *  last change: $Author: obo $ $Date: 2004-07-06 11:32:25 $
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
#ifndef _WORKCTRL_HXX
#define _WORKCTRL_HXX

#ifndef _SFXTBXCTRL_HXX //autogen
#include <sfx2/tbxctrl.hxx>
#endif

#ifndef _SV_TOOLBOX_HXX
#include <vcl/toolbox.hxx>
#endif
#ifndef _STDCTRL_HXX
#include <svtools/stdctrl.hxx>
#endif
#ifndef _IMAGEBTN_HXX //autogen
#include <vcl/imagebtn.hxx>
#endif

class PopupMenu;
class SwView;

// doppelter Eintrag! hrc und hxx
// diese Ids bestimmen, was die Buttons unter dem Scrollbar tun
#define NID_START   20000
#define NID_NEXT    20000
#define NID_PREV    20001
#define NID_TBL     20002
#define NID_FRM     20003
#define NID_PGE     20004
#define NID_DRW     20005
#define NID_CTRL    20006
#define NID_REG     20007
#define NID_BKM     20008
#define NID_GRF     20009
#define NID_OLE     20010
#define NID_OUTL    20011
#define NID_SEL     20012
#define NID_FTN     20013
#define NID_MARK    20014
#define NID_POSTIT  20015
#define NID_SRCH_REP 20016
#define NID_INDEX_ENTRY  20017
#define NID_TABLE_FORMULA   20018
#define NID_TABLE_FORMULA_ERROR     20019
#define NID_COUNT  20



//----------------------------------------------------------------------------
//
//----------------------------------------------------------------------------

class SwTbxInsertCtrl : public SfxToolBoxControl
{
    USHORT                  nLastSlotId;

    virtual void            Select( BOOL bMod1 = FALSE );

public:
    SFX_DECL_TOOLBOX_CONTROL();

    SwTbxInsertCtrl( USHORT nSlotId, USHORT nId, ToolBox& rTbx );
    ~SwTbxInsertCtrl();

    virtual SfxPopupWindowType  GetPopupWindowType() const;
    virtual SfxPopupWindow*     CreatePopupWindow();
    virtual void                StateChanged( USHORT nSID,
                                              SfxItemState eState,
                                              const SfxPoolItem* pState );

    virtual void SAL_CALL update() throw (::com::sun::star::uno::RuntimeException);
};

//----------------------------------------------------------------------------
//
//----------------------------------------------------------------------------

class SwTbxAutoTextCtrl : public SfxToolBoxControl
{
    PopupMenu*              pPopup;
    SwView*                 pView;


    void                    DelPopup();
public:
    SFX_DECL_TOOLBOX_CONTROL();

    SwTbxAutoTextCtrl( USHORT nSlotId, USHORT nId, ToolBox& rTbx );
    ~SwTbxAutoTextCtrl();

    virtual SfxPopupWindowType  GetPopupWindowType() const;
    virtual SfxPopupWindow*     CreatePopupWindow();
    virtual void                StateChanged( USHORT nSID,
                                              SfxItemState eState,
                                              const SfxPoolItem* pState );

    DECL_LINK(PopupHdl, PopupMenu*);
};


//----------------------------------------------------------------------------
//
//----------------------------------------------------------------------------

class SwScrollNaviToolBox : public ToolBox
{
    virtual void    MouseButtonUp( const MouseEvent& rMEvt );
    virtual void    RequestHelp( const HelpEvent& rHEvt );

    public:
        SwScrollNaviToolBox(Window* pParent, WinBits nWinStyle ) :
            ToolBox(pParent, nWinStyle ) {}
};

class SwScrollNaviPopup : public SfxPopupWindow
{
    SwScrollNaviToolBox    aToolBox;
    FixedLine       aSeparator;
    FixedInfo       aInfoField;
    ImageList       aIList;
    ImageList       aIListH;

    String          sQuickHelp[2 * NID_COUNT];

    USHORT          nFwdId;
    USHORT          nBackId;

    void            ApplyImageList();
    protected:
        DECL_LINK(SelectHdl, ToolBox*);
        virtual void        DataChanged( const DataChangedEvent& rDCEvt );

    public:
        SwScrollNaviPopup( USHORT nId, const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame >& rFrame );
        ~SwScrollNaviPopup();

    static String           GetQuickHelpText(BOOL bNext);
    virtual SfxPopupWindow* Clone() const;
    void                GrabFocus(){aToolBox.GrabFocus();}
};

//----------------------------------------------------------------------------
//
//----------------------------------------------------------------------------

class SwNaviImageButton : public ImageButton
{
        SwScrollNaviPopup*  pPopup;
        Image               aImage;
        Image               aImageH;
        String              sQuickText;
        SfxPopupWindow*     pPopupWindow;
        SfxPopupWindow*     pFloatingWindow;
        ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame > m_xFrame;

    protected:
        DECL_LINK( PopupModeEndHdl, void * );
        DECL_LINK( ClosePopupWindow, SfxPopupWindow * );

        virtual void    Click();
        virtual void    DataChanged( const DataChangedEvent& rDCEvt );
        void            SetPopupWindow( SfxPopupWindow* pWindow );

    public:
        SwNaviImageButton(Window* pParent, const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame >& rFrame );
};

//----------------------------------------------------------------------------
//  Die ImageButtons muessen sich bei Bedarf den HelpText selbst setzen
//----------------------------------------------------------------------------

class SwHlpImageButton : public ImageButton
{
    BOOL        bUp;
    public:
        SwHlpImageButton(Window* pParent, const ResId& rResId, BOOL bUpBtn) :
            ImageButton(pParent, rResId), bUp(bUpBtn){}

    virtual void    RequestHelp( const HelpEvent& rHEvt );

};
/* -----------------26.11.2002 09:25-----------------
 *
 * --------------------------------------------------*/
class SwPreviewZoomControl : public SfxToolBoxControl
{
public:
    SFX_DECL_TOOLBOX_CONTROL();

    SwPreviewZoomControl( USHORT nSlotId, USHORT nId, ToolBox& rTbx );
    ~SwPreviewZoomControl();

    virtual void            StateChanged( USHORT nSID,
                                              SfxItemState eState,
                                              const SfxPoolItem* pState );

    virtual Window*         CreateItemWindow( Window *pParent );
};
#endif



