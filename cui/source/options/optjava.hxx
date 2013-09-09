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
#ifndef _SVX_OPTJAVA_HXX
#define _SVX_OPTJAVA_HXX

#include <config_features.h>

#include <vector>
#include <ucbhelper/content.hxx>
#include <vcl/button.hxx>
#include <vcl/fixed.hxx>
#include <vcl/lstbox.hxx>
#include <vcl/timer.hxx>
#include <sfx2/tabdlg.hxx>
#include <com/sun/star/ui/dialogs/XFolderPicker2.hpp>
#include <svtools/dialogclosedlistener.hxx>
#include <svtools/simptabl.hxx>
#include "radiobtnbox.hxx"

// forward ---------------------------------------------------------------

#if HAVE_FEATURE_JAVA
typedef struct _JavaInfo JavaInfo;
#else
typedef void* JavaInfo;
#endif

class   SvxJavaParameterDlg;
class   SvxJavaClassPathDlg;
class   SvxJavaListBox;

// class SvxJavaOptionsPage ----------------------------------------------

class SvxJavaOptionsPage : public SfxTabPage
{
private:
    CheckBox*                   m_pJavaEnableCB;
    VclContainer*               m_pJavaBox;
    SvxJavaListBox*             m_pJavaList;
    FixedText*                  m_pJavaPathText;
    PushButton*                 m_pAddBtn;
    PushButton*                 m_pParameterBtn;
    PushButton*                 m_pClassPathBtn;

    SvxJavaParameterDlg*    m_pParamDlg;
    SvxJavaClassPathDlg*    m_pPathDlg;

#if HAVE_FEATURE_JAVA
    JavaInfo**              m_parJavaInfo;
    rtl_uString**           m_parParameters;
    rtl_uString*            m_pClassPath;
    sal_Int32               m_nInfoSize;
    sal_Int32               m_nParamSize;
#endif
    String                  m_sInstallText;
    String                  m_sAccessibilityText;
    String                  m_sAddDialogText;
    Timer                   m_aResetTimer;

    CheckBox*               m_pExperimentalCB;
    CheckBox*               m_pExpSidebarCB;
    CheckBox*               m_pMacroCB;

    ::std::vector< JavaInfo* >
                            m_aAddedInfos;

    ::com::sun::star::uno::Reference< ::svt::DialogClosedListener > xDialogListener;
    ::com::sun::star::uno::Reference< ::com::sun::star::ui::dialogs::XFolderPicker2 > xFolderPicker;

    DECL_LINK(EnableHdl_Impl, void *);
    DECL_LINK(              CheckHdl_Impl, SvSimpleTable * );
    DECL_LINK(SelectHdl_Impl, void *);
    DECL_LINK(AddHdl_Impl, void *);
    DECL_LINK(ParameterHdl_Impl, void *);
    DECL_LINK(ClassPathHdl_Impl, void *);
    DECL_LINK(ResetHdl_Impl, void *);

    DECL_LINK(              StartFolderPickerHdl, void * );
    DECL_LINK(              DialogClosedHdl, ::com::sun::star::ui::dialogs::DialogClosedEvent* );

    void                    ClearJavaInfo();
    void                    ClearJavaList();
    void                    LoadJREs();
    void                    AddJRE( JavaInfo* _pInfo );
    void                    HandleCheckEntry( SvTreeListEntry* _pEntry );
    void                    AddFolder( const OUString& _rFolder );

public:
    SvxJavaOptionsPage( Window* pParent, const SfxItemSet& rSet );
    ~SvxJavaOptionsPage();

    static SfxTabPage*      Create( Window* pParent, const SfxItemSet& rSet );

    virtual sal_Bool            FillItemSet( SfxItemSet& rSet );
    virtual void            Reset( const SfxItemSet& rSet );
    virtual void            FillUserData();
};

// class SvxJavaParameterDlg ---------------------------------------------

class SvxJavaParameterDlg : public ModalDialog
{
private:
    Edit*                   m_pParameterEdit;
    PushButton*             m_pAssignBtn;

    ListBox*                m_pAssignedList;
    PushButton*             m_pRemoveBtn;

    DECL_LINK(ModifyHdl_Impl, void *);
    DECL_LINK(AssignHdl_Impl, void *);
    DECL_LINK(SelectHdl_Impl, void *);
    DECL_LINK(DblClickHdl_Impl, void *);
    DECL_LINK(RemoveHdl_Impl, void *);

    inline void             EnableRemoveButton()
                                { m_pRemoveBtn->Enable(
                                    m_pAssignedList->GetSelectEntryPos()
                                    != LISTBOX_ENTRY_NOTFOUND ); }


public:
    SvxJavaParameterDlg( Window* pParent );
    ~SvxJavaParameterDlg();

    virtual short           Execute();

    ::com::sun::star::uno::Sequence< OUString > GetParameters() const;
    void SetParameters( ::com::sun::star::uno::Sequence< OUString >& rParams );
};

// class SvxJavaClassPathDlg ---------------------------------------------

class SvxJavaClassPathDlg : public ModalDialog
{
private:
    ListBox*                 m_pPathList;
    PushButton*              m_pAddArchiveBtn;
    PushButton*              m_pAddPathBtn;
    PushButton*              m_pRemoveBtn;

    String                  m_sOldPath;

    DECL_LINK(AddArchiveHdl_Impl, void *);
    DECL_LINK(AddPathHdl_Impl, void *);
    DECL_LINK(RemoveHdl_Impl, void *);
    DECL_LINK(SelectHdl_Impl, void *);

    bool                    IsPathDuplicate( const String& _rPath );
    inline void             EnableRemoveButton()
                                { m_pRemoveBtn->Enable(
                                    m_pPathList->GetSelectEntryPos() != LISTBOX_ENTRY_NOTFOUND ); }

public:
    SvxJavaClassPathDlg( Window* pParent );
    ~SvxJavaClassPathDlg();

    inline const String&    GetOldPath() const { return m_sOldPath; }
    inline void             SetFocus() { m_pPathList->GrabFocus(); }

    String                  GetClassPath() const;
    void                    SetClassPath( const String& _rPath );
};

#endif // #ifndef _SVX_OPTJAVA_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
