/*************************************************************************
 *
 *  $RCSfile: tphfedit.hxx,v $
 *
 *  $Revision: 1.9 $
 *
 *  last change: $Author: rt $ $Date: 2004-07-23 14:26:00 $
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

#ifndef SC_TPHFEDIT_HXX
#define SC_TPHFEDIT_HXX

#ifndef _SFXTABDLG_HXX //autogen
#include <sfx2/tabdlg.hxx>
#endif

#ifndef _SVX_PAGEITEM_HXX //autogen
#include <svx/pageitem.hxx>
#endif

#ifndef _STDCTRL_HXX //autogen
#include <svtools/stdctrl.hxx>
#endif

#ifndef _SV_GROUP_HXX //autogen
#include <vcl/group.hxx>
#endif

#ifndef _LSTBOX_HXX //autogen
#include <vcl/lstbox.hxx>
#endif

#ifndef SC_ITEMS_HXX
#include "scitems.hxx"          // wegen enum SvxNumType
#endif

#ifndef _SV_TIMER_HXX //autogen
#include <vcl/timer.hxx>
#endif

#ifndef _SV_VIRDEV_HXX //autogen
#include <vcl/virdev.hxx>
#endif

#ifndef SC_POPMENU_HXX
#include <popmenu.hxx>
#endif

#ifndef _COM_SUN_STAR_ACCESSIBILITY_XACCESSIBLE_HPP_
#include <com/sun/star/accessibility/XAccessible.hpp>
#endif
#ifndef _CPPUHELPER_WEAKREF_HXX_
#include <cppuhelper/weakref.hxx>
#endif

//===================================================================

class ScHeaderEditEngine;
class ScPatternAttr;
class EditView;
class EditTextObject;
class SvxFieldItem;
class ScAccessibleEditObject;
class ScEditWindow;

extern ScEditWindow* GetScEditWindow (); //CHINA001
enum ScEditWindowLocation
{
    Left,
    Center,
    Right
};

class ScEditWindow : public Control
{
public:
            ScEditWindow( Window* pParent, const ResId& rResId, ScEditWindowLocation eLoc );
            ~ScEditWindow();

    void            SetFont( const ScPatternAttr& rPattern );
    void            SetText( const EditTextObject& rTextObject );
    EditTextObject* CreateTextObject();
    void            SetCharAttriutes();

    void            InsertField( const SvxFieldItem& rFld );

    void            SetNumType(SvxNumType eNumType);

    virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible > CreateAccessible();

    inline ScHeaderEditEngine*  GetEditEngine() const {return pEdEngine;}
protected:
    virtual void    Paint( const Rectangle& rRec );
    virtual void    MouseMove( const MouseEvent& rMEvt );
    virtual void    MouseButtonDown( const MouseEvent& rMEvt );
    virtual void    MouseButtonUp( const MouseEvent& rMEvt );
    virtual void    KeyInput( const KeyEvent& rKEvt );
    virtual void    Command( const CommandEvent& rCEvt );
    virtual void    GetFocus();
    virtual void    LoseFocus();

private:
    ScHeaderEditEngine* pEdEngine;
    EditView*           pEdView;
    ScEditWindowLocation eLocation;

    com::sun::star::uno::WeakReference< ::com::sun::star::accessibility::XAccessible > xAcc;
    ScAccessibleEditObject* pAcc;
};

//===================================================================
class ScExtIButton : public ImageButton
{
private:

    Timer           aTimer;
    ScPopupMenu*    pPopupMenu;
    Link            aMLink;
    USHORT          nSelected;

    DECL_LINK( TimerHdl, Timer*);

//  void            DrawArrow();

protected:

    virtual void    MouseButtonDown( const MouseEvent& rMEvt );
    virtual void    MouseButtonUp( const MouseEvent& rMEvt);
    virtual void    Click();

    virtual void    StartPopup();

public:

    ScExtIButton(Window* pParent, const ResId& rResId );

    void            SetPopupMenu(ScPopupMenu* pPopUp);

    USHORT          GetSelected();

    void            SetMenuHdl( const Link& rLink ) { aMLink = rLink; }
    const Link&     GetMenuHdl() const { return aMLink; }

    virtual long    PreNotify( NotifyEvent& rNEvt );
};


//===================================================================
//CHINA001
//CHINA001 class ScHFEditPage : public SfxTabPage
//CHINA001 {
//CHINA001 public:
//CHINA001 virtual  BOOL    FillItemSet ( SfxItemSet& rCoreSet );
//CHINA001 virtual  void    Reset       ( const SfxItemSet& rCoreSet );
//CHINA001
//CHINA001 void         SetNumType(SvxNumType eNumType);
//CHINA001
//CHINA001 protected:
//CHINA001 ScHFEditPage( Window*            pParent,
//CHINA001 USHORT           nResId,
//CHINA001 const SfxItemSet&    rCoreSet,
//CHINA001 USHORT           nWhich );
//CHINA001 virtual      ~ScHFEditPage();
//CHINA001
//CHINA001 private:
//CHINA001 FixedText        aFtLeft;
//CHINA001 ScEditWindow aWndLeft;
//CHINA001 FixedText        aFtCenter;
//CHINA001 ScEditWindow aWndCenter;
//CHINA001 FixedText        aFtRight;
//CHINA001 ScEditWindow aWndRight;
//CHINA001 ImageButton      aBtnText;
//CHINA001 ScExtIButton aBtnFile;
//CHINA001 ImageButton      aBtnTable;
//CHINA001 ImageButton      aBtnPage;
//CHINA001 ImageButton      aBtnLastPage;
//CHINA001 ImageButton      aBtnDate;
//CHINA001 ImageButton      aBtnTime;
//CHINA001 FixedLine        aFlInfo;
//CHINA001 FixedInfo        aFtInfo;
//CHINA001 ScPopupMenu      aPopUpFile;
//CHINA001
//CHINA001 USHORT           nWhich;
//CHINA001 String           aCmdArr[6];
//CHINA001
//CHINA001 private:
//CHINA001 #ifdef _TPHFEDIT_CXX
//CHINA001 void FillCmdArr();
//CHINA001 DECL_LINK( ClickHdl, ImageButton* );
//CHINA001 DECL_LINK( MenuHdl, ScExtIButton* );
//CHINA001 #endif
//CHINA001 };
//CHINA001
//CHINA001 //===================================================================
//CHINA001
//CHINA001 class ScRightHeaderEditPage : public ScHFEditPage
//CHINA001 {
//CHINA001 public:
//CHINA001 static SfxTabPage*   Create( Window* pParent, const SfxItemSet& rCoreSet );
//CHINA001 static USHORT*       GetRanges();
//CHINA001
//CHINA001 private:
//CHINA001 ScRightHeaderEditPage( Window* pParent, const SfxItemSet& rSet );
//CHINA001 };
//CHINA001
//CHINA001 //===================================================================
//CHINA001
//CHINA001 class ScLeftHeaderEditPage : public ScHFEditPage
//CHINA001 {
//CHINA001 public:
//CHINA001 static SfxTabPage*   Create( Window* pParent, const SfxItemSet& rCoreSet );
//CHINA001 static USHORT*       GetRanges();
//CHINA001
//CHINA001 private:
//CHINA001 ScLeftHeaderEditPage( Window* pParent, const SfxItemSet& rSet );
//CHINA001 };
//CHINA001
//CHINA001 //===================================================================
//CHINA001
//CHINA001 class ScRightFooterEditPage : public ScHFEditPage
//CHINA001 {
//CHINA001 public:
//CHINA001 static SfxTabPage*   Create( Window* pParent, const SfxItemSet& rCoreSet );
//CHINA001 static USHORT*       GetRanges();
//CHINA001
//CHINA001 private:
//CHINA001 ScRightFooterEditPage( Window* pParent, const SfxItemSet& rSet );
//CHINA001 };
//CHINA001
//CHINA001 //===================================================================
//CHINA001
//CHINA001 class ScLeftFooterEditPage : public ScHFEditPage
//CHINA001 {
//CHINA001 public:
//CHINA001 static SfxTabPage*   Create( Window* pParent, const SfxItemSet& rCoreSet );
//CHINA001 static USHORT*       GetRanges();
//CHINA001
//CHINA001 private:
//CHINA001 ScLeftFooterEditPage( Window* pParent, const SfxItemSet& rSet );
//CHINA001 };



#endif // SC_TPHFEDIT_HXX

