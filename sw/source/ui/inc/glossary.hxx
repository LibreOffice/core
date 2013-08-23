/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */
#ifndef _GLOSSARY_HXX
#define _GLOSSARY_HXX

#include <vcl/edit.hxx>
#include <svtools/treelistbox.hxx>
#include <svx/stddlg.hxx>

#include <vcl/button.hxx>
#include <vcl/fixed.hxx>

#include <vcl/combobox.hxx>

#include <vcl/menubtn.hxx>
#include <com/sun/star/container/XEnumerationAccess.hpp>
#include <com/sun/star/container/XHierarchicalNameAccess.hpp>
#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/container/XContentEnumerationAccess.hpp>
#include <com/sun/star/container/XEnumeration.hpp>
#include <com/sun/star/container/XElementAccess.hpp>
#include <com/sun/star/container/XIndexAccess.hpp>
#include <com/sun/star/text/XAutoTextContainer2.hpp>

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

    SvTreeListEntry*  pDragEntry;

    virtual DragDropMode NotifyStartDrag( TransferDataContainer& rContainer,
                                            SvTreeListEntry* );
    virtual sal_Bool    NotifyAcceptDrop( SvTreeListEntry* );

    virtual sal_Bool    NotifyMoving(   SvTreeListEntry*  pTarget,
                                    SvTreeListEntry*  pEntry,
                                    SvTreeListEntry*& rpNewParent,
                                    sal_uLong&        rNewChildPos
                                );
    virtual sal_Bool    NotifyCopying(  SvTreeListEntry*  pTarget,
                                    SvTreeListEntry*  pEntry,
                                    SvTreeListEntry*& rpNewParent,
                                    sal_uLong&        rNewChildPos);

    sal_Bool NotifyCopyingOrMoving( SvTreeListEntry*  pTarget,
                                    SvTreeListEntry*  pEntry,
                                    bool              bIsMove);
public:
    SwGlTreeListBox(Window* pParent, WinBits nBits);

    virtual void RequestHelp( const HelpEvent& rHEvt );
    virtual Size GetOptimalSize() const;
    void Clear();
};

//------------------------------------------------------------------
class SwOneExampleFrame;
class SwGlossaryDlg : public SvxStandardDialog
{
    friend class SwNewGlosNameDlg;
    friend class SwGlTreeListBox;

    CheckBox*       m_pInsertTipCB;
    Edit*           m_pNameED;
    FixedText*      m_pShortNameLbl;
    NoSpaceEdit*    m_pShortNameEdit;
    SwGlTreeListBox* m_pCategoryBox;
    CheckBox*       m_pFileRelCB;
    CheckBox*       m_pNetRelCB;
    Window*         m_pExampleWIN;
    PushButton*     m_pInsertBtn;
    CloseButton*    m_pCloseBtn;
    MenuButton*     m_pEditBtn;
    PushButton*     m_pBibBtn;
    PushButton*     m_pPathBtn;

    String          sReadonlyPath;

    ::com::sun::star::uno::Reference< ::com::sun::star::text::XAutoTextContainer2 > m_xAutoText;
    SwOneExampleFrame*  pExampleFrame;

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
    void ShowPreview();

    DECL_LINK( NameModify, Edit * );
    DECL_LINK( NameDoubleClick, SvTreeListBox * );
    DECL_LINK( GrpSelect, SvTreeListBox * );
    DECL_LINK( MenuHdl, Menu * );
    DECL_LINK( EnableHdl, Menu * );
    DECL_LINK(BibHdl, void *);
    DECL_LINK(EditHdl, void *);
    DECL_LINK(InsertHdl, void *);
    DECL_LINK( PathHdl, Button * );
    DECL_LINK( CheckBoxHdl, CheckBox * );
    DECL_LINK( PreviewLoadedHdl, void * );


    virtual void    Apply();
    void            Init();
    SvTreeListEntry*    DoesBlockExist(const String& sBlock, const String& rShort);
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
    String GetCurrGrpName() const;
    String GetCurrLongName() const
    {
        return m_pNameED->GetText();
    }
    String GetCurrShortName() const
    {
        return m_pShortNameEdit->GetText();
    }
    static String   GetCurrGroup();
    static void     SetActGroup(const String& rNewGroup);
    static String   GetExtension();
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
