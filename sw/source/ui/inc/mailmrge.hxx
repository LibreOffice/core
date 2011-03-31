/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/
#ifndef _MAILMRGE_HXX
#define _MAILMRGE_HXX

#include <svx/stddlg.hxx>

#ifndef _BUTTON_HXX //autogen
#include <vcl/button.hxx>
#endif

#ifndef _FIELD_HXX //autogen
#include <vcl/field.hxx>
#endif

#ifndef _FIXED_HXX //autogen
#include <vcl/fixed.hxx>
#endif
#ifndef _EDIT_HXX //autogen
#include <vcl/edit.hxx>
#endif
#include <svtools/stdctrl.hxx>

#ifndef _LSTBOX_HXX //autogen
#include <vcl/lstbox.hxx>
#endif
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
    DECL_LINK( InsertPathHdl, PushButton * );
    DECL_LINK( AttachFileHdl, PushButton * );
    DECL_LINK( OutputTypeHdl, RadioButton* pBtn );
    DECL_LINK( FilenameHdl, CheckBox* pBtn );
    DECL_LINK( ModifyHdl, NumericField* pLB );
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
/* -----------------27.11.2002 12:05-----------------
 *
 * --------------------------------------------------*/
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
/* -----------------04.02.2003 13:41-----------------
 *
 * --------------------------------------------------*/
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

