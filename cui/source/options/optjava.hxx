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

#include <vector>
#include <ucbhelper/content.hxx>
#include <vcl/button.hxx>
#include <vcl/fixed.hxx>
#include <vcl/timer.hxx>
#include <sfx2/tabdlg.hxx>
#include <svx/simptabl.hxx>
#include <com/sun/star/ui/dialogs/XFolderPicker.hpp>
#include <svtools/dialogclosedlistener.hxx>
#include "radiobtnbox.hxx"

// forward ---------------------------------------------------------------

typedef struct _JavaInfo JavaInfo;

class   SvxJavaParameterDlg;
class   SvxJavaClassPathDlg;

// class SvxJavaOptionsPage ----------------------------------------------

class SvxJavaOptionsPage : public SfxTabPage
{
private:
    FixedLine                   m_aJavaLine;
    CheckBox                    m_aJavaEnableCB;
    FixedText                   m_aJavaFoundLabel;
    SvxSimpleTableContainer     m_aJavaListContainer;
    svx::SvxRadioButtonListBox  m_aJavaList;
    FixedText                   m_aJavaPathText;
    PushButton                  m_aAddBtn;
    PushButton                  m_aParameterBtn;
    PushButton                  m_aClassPathBtn;

    SvxJavaParameterDlg*    m_pParamDlg;
    SvxJavaClassPathDlg*    m_pPathDlg;

    JavaInfo**              m_parJavaInfo;
    rtl_uString**           m_parParameters;
    rtl_uString*            m_pClassPath;
    sal_Int32               m_nInfoSize;
    sal_Int32               m_nParamSize;
    String                  m_sInstallText;
    String                  m_sAccessibilityText;
    String                  m_sAddDialogText;
    Timer                   m_aResetTimer;

    FixedLine               m_aExperimental;
    CheckBox                m_aExperimentalCB;
    CheckBox                m_aMacroCB;

    ::std::vector< JavaInfo* >
                            m_aAddedInfos;

    ::com::sun::star::uno::Reference< ::svt::DialogClosedListener > xDialogListener;
    ::com::sun::star::uno::Reference< ::com::sun::star::ui::dialogs::XFolderPicker > xFolderPicker;

    DECL_LINK(EnableHdl_Impl, void *);
    DECL_LINK(              CheckHdl_Impl, SvxSimpleTable * );
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
    void                    HandleCheckEntry( SvLBoxEntry* _pEntry );
    void                    AddFolder( const ::rtl::OUString& _rFolder );

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
    FixedText               m_aParameterLabel;
    Edit                    m_aParameterEdit;
    PushButton              m_aAssignBtn;

    FixedText               m_aAssignedLabel;
    ListBox                 m_aAssignedList;
    FixedText               m_aExampleText;
    PushButton              m_aRemoveBtn;

    FixedLine               m_aButtonsLine;
    OKButton                m_aOKBtn;
    CancelButton            m_aCancelBtn;
    HelpButton              m_aHelpBtn;

    DECL_LINK(ModifyHdl_Impl, void *);
    DECL_LINK(AssignHdl_Impl, void *);
    DECL_LINK(SelectHdl_Impl, void *);
    DECL_LINK(DblClickHdl_Impl, void *);
    DECL_LINK(RemoveHdl_Impl, void *);

    inline void             EnableRemoveButton()
                                { m_aRemoveBtn.Enable(
                                    m_aAssignedList.GetSelectEntryPos()
                                    != LISTBOX_ENTRY_NOTFOUND ); }


public:
    SvxJavaParameterDlg( Window* pParent );
    ~SvxJavaParameterDlg();

    virtual short           Execute();

    ::com::sun::star::uno::Sequence< ::rtl::OUString > GetParameters() const;
    void SetParameters( ::com::sun::star::uno::Sequence< ::rtl::OUString >& rParams );
};

// class SvxJavaClassPathDlg ---------------------------------------------

class SvxJavaClassPathDlg : public ModalDialog
{
private:
    FixedText               m_aPathLabel;
    ListBox                 m_aPathList;
    PushButton              m_aAddArchiveBtn;
    PushButton              m_aAddPathBtn;
    PushButton              m_aRemoveBtn;

    FixedLine               m_aButtonsLine;
    OKButton                m_aOKBtn;
    CancelButton            m_aCancelBtn;
    HelpButton              m_aHelpBtn;

    String                  m_sOldPath;

    DECL_LINK(AddArchiveHdl_Impl, void *);
    DECL_LINK(AddPathHdl_Impl, void *);
    DECL_LINK(RemoveHdl_Impl, void *);
    DECL_LINK(SelectHdl_Impl, void *);

    bool                    IsPathDuplicate( const String& _rPath );
    inline void             EnableRemoveButton()
                                { m_aRemoveBtn.Enable(
                                    m_aPathList.GetSelectEntryPos() != LISTBOX_ENTRY_NOTFOUND ); }

public:
    SvxJavaClassPathDlg( Window* pParent );
    ~SvxJavaClassPathDlg();

    inline const String&    GetOldPath() const { return m_sOldPath; }
    inline void             SetFocus() { m_aPathList.GrabFocus(); }

    String                  GetClassPath() const;
    void                    SetClassPath( const String& _rPath );
};

#endif // #ifndef _SVX_OPTJAVA_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
