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



#ifndef SC_TPHFEDIT_HXX
#define SC_TPHFEDIT_HXX

#include <sfx2/tabdlg.hxx>
#include <svx/pageitem.hxx>
#include <svtools/stdctrl.hxx>
#include <vcl/group.hxx>
#ifndef _LSTBOX_HXX //autogen
#include <vcl/lstbox.hxx>
#endif
#include <vcl/timer.hxx>
#include <vcl/virdev.hxx>
#include "scdllapi.h"
#include "scitems.hxx"          // wegen enum SvxNumType
#include "popmenu.hxx"
#include <com/sun/star/accessibility/XAccessible.hpp>
#include <cppuhelper/weakref.hxx>

//===================================================================

class ScHeaderEditEngine;
class ScPatternAttr;
class EditView;
class EditTextObject;
class SvxFieldItem;
class ScAccessibleEditObject;
class ScEditWindow;

SC_DLLPUBLIC ScEditWindow* GetScEditWindow (); //CHINA001

enum ScEditWindowLocation
{
    Left,
    Center,
    Right
};

class SC_DLLPUBLIC ScEditWindow : public Control
{
public:
            ScEditWindow( Window* pParent, const ResId& rResId, ScEditWindowLocation eLoc );
            ~ScEditWindow();

    using Control::SetFont;
    void            SetFont( const ScPatternAttr& rPattern );
    using Control::SetText;
    void            SetText( const EditTextObject& rTextObject );
    EditTextObject* CreateTextObject();
    void            SetCharAttriutes();

    void            InsertField( const SvxFieldItem& rFld );

    void            SetNumType(SvxNumType eNumType);

    virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible > CreateAccessible();

    inline ScHeaderEditEngine*  GetEditEngine() const {return pEdEngine;}
    //IAccessibility2 Implementation 2009-----
    void            SetObjectSelectHdl( const Link& aLink){ aObjectSelectLink = aLink; };
    //-----IAccessibility2 Implementation 2009
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
    bool mbRTL;

    com::sun::star::uno::WeakReference< ::com::sun::star::accessibility::XAccessible > xAcc;
    ScAccessibleEditObject* pAcc;

    //IAccessibility2 Implementation 2009-----
    Link                aObjectSelectLink;
    //-----IAccessibility2 Implementation 2009

};

//===================================================================
class SC_DLLPUBLIC ScExtIButton : public ImageButton
{
private:

    Timer           aTimer;
    ScPopupMenu*    pPopupMenu;
    Link            aMLink;
    sal_uInt16          nSelected;

    SC_DLLPRIVATE  DECL_LINK( TimerHdl, Timer*);

//  void            DrawArrow();

protected:

    virtual void    MouseButtonDown( const MouseEvent& rMEvt );
    virtual void    MouseButtonUp( const MouseEvent& rMEvt);
    virtual void    Click();

    virtual void    StartPopup();

public:

    ScExtIButton(Window* pParent, const ResId& rResId );

    void            SetPopupMenu(ScPopupMenu* pPopUp);

    sal_uInt16          GetSelected();

    void            SetMenuHdl( const Link& rLink ) { aMLink = rLink; }
    const Link&     GetMenuHdl() const { return aMLink; }

    virtual long    PreNotify( NotifyEvent& rNEvt );
};


//===================================================================
//CHINA001
//CHINA001 class ScHFEditPage : public SfxTabPage
//CHINA001 {
//CHINA001 public:
//CHINA001 virtual  sal_Bool    FillItemSet ( SfxItemSet& rCoreSet );
//CHINA001 virtual  void    Reset       ( const SfxItemSet& rCoreSet );
//CHINA001
//CHINA001 void         SetNumType(SvxNumType eNumType);
//CHINA001
//CHINA001 protected:
//CHINA001 ScHFEditPage( Window*            pParent,
//CHINA001 sal_uInt16           nResId,
//CHINA001 const SfxItemSet&    rCoreSet,
//CHINA001 sal_uInt16           nWhich );
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
//CHINA001 sal_uInt16           nWhich;
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
//CHINA001 static sal_uInt16*       GetRanges();
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
//CHINA001 static sal_uInt16*       GetRanges();
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
//CHINA001 static sal_uInt16*       GetRanges();
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
//CHINA001 static sal_uInt16*       GetRanges();
//CHINA001
//CHINA001 private:
//CHINA001 ScLeftFooterEditPage( Window* pParent, const SfxItemSet& rSet );
//CHINA001 };



#endif // SC_TPHFEDIT_HXX

