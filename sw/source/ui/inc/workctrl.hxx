/*************************************************************************
 *
 *  $RCSfile: workctrl.hxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 17:14:43 $
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

#ifndef _VALUESET_HXX //autogen
#include <svtools/valueset.hxx>
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

    SwTbxInsertCtrl( USHORT nId, ToolBox& rTbx, SfxBindings& rBind );
    ~SwTbxInsertCtrl();

    virtual SfxPopupWindowType  GetPopupWindowType() const;
    virtual SfxPopupWindow*     CreatePopupWindow();
    virtual void                StateChanged( USHORT nSID,
                                              SfxItemState eState,
                                              const SfxPoolItem* pState );

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

    SwTbxAutoTextCtrl( USHORT nId, ToolBox& rTbx, SfxBindings& rBind );
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

class SwScrollNaviValueSet : public ValueSet
{
    virtual void    MouseButtonUp( const MouseEvent& rMEvt );
    virtual void    RequestHelp( const HelpEvent& rHEvt );

    public:
        SwScrollNaviValueSet(Window* pParent, WinBits nWinStyle ) :
            ValueSet(pParent, nWinStyle ) {}
};

class SwScrollNaviPopup : public SfxPopupWindow
{
    SwScrollNaviValueSet    aVSet;
    ImageList       aIList;
    const ResId&    rResId;

    String          sQuickHelp[2 * NID_COUNT];

    USHORT          nFwdId;
    USHORT          nBackId;

    protected:
        DECL_LINK(SelectHdl, ValueSet*);

    public:
        SwScrollNaviPopup( USHORT nId, const ResId &rId, SfxBindings & );
        ~SwScrollNaviPopup();

    static String           GetQuickHelpText(BOOL bNext);
    virtual void            PopupModeEnd();
    virtual SfxPopupWindow* Clone() const;
};

//----------------------------------------------------------------------------
//
//----------------------------------------------------------------------------

class SwNaviImageButton : public ImageButton
{
        SwScrollNaviPopup*  pPopup;
        Image               aImage;
        String              sQuickText;

    protected:
        virtual void    MouseButtonDown( const MouseEvent& rMEvt );
        virtual void    MouseButtonUp( const MouseEvent& rMEvt );
    public:
        SwNaviImageButton(Window* pParent);
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
#endif



