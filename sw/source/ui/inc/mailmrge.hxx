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
#ifndef _MAILMRGE_HXX
#define _MAILMRGE_HXX

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
        class XFrame2;
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
    ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame2 > m_xFrame;

    Size            m_aDialogSize;
    OUString m_sSaveFilter;


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

    const OUString& GetSaveFilter() const {return m_sSaveFilter;}
    inline const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any > GetSelection() const { return m_aSelection; }
    ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XResultSet> GetResultSet() const;

};

class SwMailMergeCreateFromDlg : public ModalDialog
{
    RadioButton* m_pThisDocRB;
public:
    SwMailMergeCreateFromDlg(Window* pParent);
    bool IsThisDocument() const
    {
        return m_pThisDocRB->IsChecked();
    }
};

class SwMailMergeFieldConnectionsDlg : public ModalDialog
{
    RadioButton* m_pUseExistingRB;
public:
    SwMailMergeFieldConnectionsDlg(Window* pParent);
    bool IsUseExistingConnections() const
    {
        return m_pUseExistingRB->IsChecked();
    }
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
