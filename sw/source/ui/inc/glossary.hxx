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


#ifndef _GLOSSARY_HXX
#define _GLOSSARY_HXX

#ifndef _EDIT_HXX //autogen
#include <vcl/edit.hxx>
#endif
#include <svtools/svtreebx.hxx>
#include <svx/stddlg.hxx>

#ifndef _BUTTON_HXX //autogen
#include <vcl/button.hxx>
#endif
#include <vcl/fixed.hxx>

#ifndef _BUTTON_HXX //autogen
#include <vcl/button.hxx>
#endif

#ifndef _FIXED_HXX //autogen
#include <vcl/fixed.hxx>
#endif

#ifndef _COMBOBOX_HXX //autogen
#include <vcl/combobox.hxx>
#endif

#ifndef _MENUBTN_HXX //autogen
#include <vcl/menubtn.hxx>
#endif
#include <com/sun/star/container/XEnumerationAccess.hpp>
#include <com/sun/star/container/XHierarchicalNameAccess.hpp>
#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/container/XContentEnumerationAccess.hpp>
#include <com/sun/star/container/XEnumeration.hpp>
#include <com/sun/star/container/XElementAccess.hpp>
#include <com/sun/star/container/XIndexAccess.hpp>

#include <actctrl.hxx>

class SwGlossaryHdl;
class SwNewGlosNameDlg;
class SwWrtShell;
class SfxViewFrame;
class PopupMenu;
class Menu;

const short RET_EDIT = 100;

//------------------------------------------------------------------

class SwGlTreeListBox : public SvTreeListBox
{
    const String    sReadonly;

    SvLBoxEntry*  pDragEntry;

    virtual DragDropMode NotifyStartDrag( TransferDataContainer& rContainer,
                                            SvLBoxEntry* );
    virtual sal_Bool    NotifyAcceptDrop( SvLBoxEntry* );

    virtual sal_Bool    NotifyMoving(   SvLBoxEntry*  pTarget,
                                    SvLBoxEntry*  pEntry,
                                    SvLBoxEntry*& rpNewParent,
                                    sal_uLong&        rNewChildPos
                                );
    virtual sal_Bool    NotifyCopying(  SvLBoxEntry*  pTarget,
                                    SvLBoxEntry*  pEntry,
                                    SvLBoxEntry*& rpNewParent,
                                    sal_uLong&        rNewChildPos);
public:
    SwGlTreeListBox(Window* pParent, const ResId& rResId);

    virtual void    RequestHelp( const HelpEvent& rHEvt );
    void            Clear();
};

//------------------------------------------------------------------
class SwOneExampleFrame;
class SwGlossaryDlg : public SvxStandardDialog
{
    friend class SwNewGlosNameDlg;
    friend class SwGlTreeListBox;

    CheckBox        aInsertTipCB;
    FixedText       aNameLbl;
    Edit            aNameED;
    FixedText       aShortNameLbl;
    NoSpaceEdit     aShortNameEdit;
    SwGlTreeListBox aCategoryBox;
    FixedLine       aRelativeFL;
    CheckBox        aFileRelCB;
    CheckBox        aNetRelCB;
    Window          aExampleWIN;
    Window          aExampleDummyWIN;
    CheckBox        aShowExampleCB;
    OKButton        aInsertBtn;
    CancelButton    aCloseBtn;
    HelpButton      aHelpBtn;
    MenuButton      aEditBtn;
    PushButton      aBibBtn;
    PushButton      aPathBtn;

    String          sReadonlyPath;

    ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess >        _xAutoText;
    SwOneExampleFrame*  pExampleFrame;

    PopupMenu*      pMenu;
    SwGlossaryHdl*  pGlossaryHdl;

    String          sResumeGroup;
    String          sResumeShortName;
    sal_Bool            bResume;


    const sal_Bool      bSelection : 1;
    sal_Bool            bReadOnly : 1;
    sal_Bool            bIsOld : 1;
    sal_Bool            bIsDocReadOnly:1;

    SwWrtShell*     pSh;

    void EnableShortName(sal_Bool bOn = sal_True);

    DECL_LINK( NameModify, Edit * );
    DECL_LINK( NameDoubleClick, SvTreeListBox * );
    DECL_LINK( GrpSelect, SvTreeListBox * );
    DECL_LINK( MenuHdl, Menu * );
    DECL_LINK( EnableHdl, Menu * );
    DECL_LINK( BibHdl, Button * );
    DECL_LINK( EditHdl, Button * );
    DECL_LINK( PathHdl, Button * );
    DECL_LINK( CheckBoxHdl, CheckBox * );
    DECL_LINK( ShowPreviewHdl, CheckBox * );
    DECL_LINK( PreviewLoadedHdl, void * );


    virtual void    Apply();
    void            Init();
    SvLBoxEntry*    DoesBlockExist(const String& sBlock, const String& rShort);
    void            ShowAutoText(const String& rGroup, const String& rShortName);
    void            ResumeShowAutoText();

    sal_Bool            GetResumeData(String& rGroup, String& rShortName)
                        {rGroup = sResumeGroup; rShortName = sResumeShortName; return bResume;}
    void            SetResumeData(const String& rGroup, const String& rShortName)
                        {sResumeGroup = rGroup; sResumeShortName = rShortName; bResume = sal_True;}
    void            ResetResumeData() {bResume = sal_False;}
public:
    SwGlossaryDlg(SfxViewFrame* pViewFrame, SwGlossaryHdl* pGlosHdl, SwWrtShell *pWrtShell);
    ~SwGlossaryDlg();
    String          GetCurrGrpName() const;
    inline String   GetCurrLongName() const;
    inline String   GetCurrShortName() const;
    static String   GetCurrGroup();
    static void     SetActGroup(const String& rNewGroup);
    static String   GetExtension();
};

inline String SwGlossaryDlg::GetCurrLongName() const
{
    return aNameED.GetText();
}
inline String SwGlossaryDlg::GetCurrShortName() const
{
    return aShortNameEdit.GetText();
}


#endif

