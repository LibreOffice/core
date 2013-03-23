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
#pragma once
#if 1

#include <svx/stddlg.hxx>

#include <vcl/button.hxx>

#include <vcl/field.hxx>

#include <vcl/fixed.hxx>
#include <vcl/edit.hxx>
#include <svtools/stdctrl.hxx>

#include <vcl/lstbox.hxx>
#include <com/sun/star/uno/Sequence.h>
#include <com/sun/star/uno/Reference.h>

class SwWrtShell;
class SwModuleOptions;
class SwXSelChgLstnr_Impl;
struct SwMailMergeDlg_Impl;
namespace com{namespace sun{namespace star{
    namespace frame{
        class XFrame;
    }
    namespace sdbc{
        class XResultSet;
        class XConnection;
    }
}}}

class SwMailMergeDlg : public SvxStandardDialog
{
    friend class SwXSelChgLstnr_Impl;

    Window*         pBeamerWin;

    RadioButton     aAllRB;
    RadioButton     aMarkedRB;
    RadioButton     aFromRB;
    NumericField    aFromNF;
    FixedText       aBisFT;
    NumericField    aToNF;
    FixedLine       aRecordFL;

    FixedLine       aSeparatorFL;

    RadioButton     aPrinterRB;
    RadioButton     aMailingRB;
    RadioButton     aFileRB;

    CheckBox        aSingleJobsCB;

    FixedLine       aSaveMergedDocumentFL;
    RadioButton     aSaveSingleDocRB;
    RadioButton     aSaveIndividualRB;

    CheckBox        aGenerateFromDataBaseCB;

    FixedText       aColumnFT;
    ListBox         aColumnLB;
    FixedText       aPathFT;
    Edit            aPathED;
    PushButton      aPathPB;
    FixedText       aFilterFT;
    ListBox         aFilterLB;

    ListBox         aAddressFldLB;
    FixedText       aSubjectFT;
    Edit            aSubjectED;
    FixedText       aFormatFT;
    FixedText       aAttachFT;
    Edit            aAttachED;
    PushButton      aAttachPB;
    CheckBox        aFormatHtmlCB;
    CheckBox        aFormatRtfCB;
    CheckBox        aFormatSwCB;
    FixedLine       aDestFL;

    FixedLine       aBottomSeparatorFL;

    OKButton        aOkBTN;
    CancelButton    aCancelBTN;
    HelpButton      aHelpBTN;

    SwMailMergeDlg_Impl* pImpl;

    SwWrtShell&     rSh;
    SwModuleOptions* pModOpt;
    const String&   rDBName;
    const String&   rTableName;

    sal_uInt16          nMergeType;
    ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any >       m_aSelection;
    ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame > xFrame;

    Size            m_aDialogSize;
    ::rtl::OUString m_sSaveFilter;


    DECL_LINK( ButtonHdl, Button* pBtn );
    DECL_LINK(InsertPathHdl, void *);
    DECL_LINK(AttachFileHdl, void *);
    DECL_LINK( OutputTypeHdl, RadioButton* pBtn );
    DECL_LINK( FilenameHdl, CheckBox* pBtn );
    DECL_LINK(ModifyHdl, void *);
    DECL_LINK( SaveTypeHdl, RadioButton* pBtn );

    virtual void    Apply();
    virtual void    Resize();
    bool            ExecQryShell();

public:
     SwMailMergeDlg(Window* pParent, SwWrtShell& rSh,
         const String& rSourceName,
        const String& rTblName,
        sal_Int32 nCommandType,
        const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection>& xConnection,
        ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any >* pSelection = 0);
    ~SwMailMergeDlg();

    inline sal_uInt16   GetMergeType() { return nMergeType; }

    bool IsSaveIndividualDocs() const { return aSaveIndividualRB.IsChecked(); }
    bool IsGenerateFromDataBase() const { return aGenerateFromDataBaseCB.IsChecked(); }
    String GetColumnName() const { return aColumnLB.GetSelectEntry();}
    String GetPath() const { return aPathED.GetText();}

    const ::rtl::OUString& GetSaveFilter() const {return m_sSaveFilter;}
    inline const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any > GetSelection() const { return m_aSelection; }
    ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XResultSet> GetResultSet() const;

};

class SwMailMergeCreateFromDlg : public ModalDialog
{
    FixedLine       aCreateFromFL;
    RadioButton     aThisDocRB;
    RadioButton     aUseTemplateRB;

    OKButton        aOK;
    CancelButton    aCancel;
    HelpButton      aHelp;
public:
    SwMailMergeCreateFromDlg(Window* pParent);
    ~SwMailMergeCreateFromDlg();

    sal_Bool    IsThisDocument() const {return aThisDocRB.IsChecked();}
};

class SwMailMergeFieldConnectionsDlg : public ModalDialog
{
    FixedLine       aConnectionsFL;
    RadioButton     aUseExistingRB;
    RadioButton     aCreateNewRB;

    FixedInfo       aInfoFI;

    OKButton        aOK;
    CancelButton    aCancel;
    HelpButton      aHelp;
public:
    SwMailMergeFieldConnectionsDlg(Window* pParent);
    ~SwMailMergeFieldConnectionsDlg();

    sal_Bool    IsUseExistingConnections() const {return aUseExistingRB.IsChecked();}
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
