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

#ifndef SC_PFILTDLG_HXX
#define SC_PFILTDLG_HXX

#include <vcl/morebtn.hxx>
#include <svtools/stdctrl.hxx>
#include <vcl/button.hxx>
#include <vcl/fixed.hxx>
#include <vcl/dialog.hxx>
#include <vcl/lstbox.hxx>
#include <vcl/combobox.hxx>
#include "address.hxx"
#include "queryparam.hxx"

//------------------------------------------------------------------

class ScViewData;
class ScDocument;
class ScQueryItem;
class ScTypedStrData;

//==================================================================

class ScPivotFilterDlg : public ModalDialog
{
public:
                    ScPivotFilterDlg( Window* pParent,
                                      const SfxItemSet& rArgSet, SCTAB nSourceTab );
                    ~ScPivotFilterDlg();

    const ScQueryItem&  GetOutputItem();

private:
    FixedLine       aFlCriteria;
    //----------------------------
    ListBox         aLbField1;
    ListBox         aLbCond1;
    ComboBox        aEdVal1;
    //----------------------------
    ListBox         aLbConnect1;
    ListBox         aLbField2;
    ListBox         aLbCond2;
    ComboBox        aEdVal2;
    //----------------------------
    ListBox         aLbConnect2;
    ListBox         aLbField3;
    ListBox         aLbCond3;
    ComboBox        aEdVal3;
    //----------------------------
    FixedText       aFtConnect;
    FixedText       aFtField;
    FixedText       aFtCond;
    FixedText       aFtVal;

    FixedLine       aFlOptions;
    CheckBox        aBtnCase;
    CheckBox        aBtnRegExp;
    CheckBox        aBtnUnique;
    FixedText       aFtDbAreaLabel;
    FixedInfo       aFtDbArea;
    OKButton        aBtnOk;
    CancelButton    aBtnCancel;
    HelpButton      aBtnHelp;
    MoreButton      aBtnMore;
    const OUString aStrUndefined;
    const OUString aStrNone;
    const OUString aStrEmpty;
    const OUString aStrNotEmpty;
    const OUString aStrRow;
    const OUString aStrColumn;

    const sal_uInt16    nWhichQuery;
    const ScQueryParam  theQueryData;
    ScQueryItem*        pOutItem;
    ScViewData*         pViewData;
    ScDocument*         pDoc;
    SCTAB               nSrcTab;

    sal_uInt16              nFieldCount;
    ComboBox*           aValueEdArr[3];
    ListBox*            aFieldLbArr[3];
    ListBox*            aCondLbArr[3];

    std::vector<ScTypedStrData>* pEntryLists[MAXCOLCOUNT];

#ifdef _PFILTDLG_CXX
private:
    void    Init            ( const SfxItemSet& rArgSet );
    void    FillFieldLists  ();
    void    UpdateValueList ( sal_uInt16 nList );
    void    ClearValueList  ( sal_uInt16 nList );
    sal_uInt16  GetFieldSelPos  ( SCCOL nField );

    // Handler:
    DECL_LINK( LbSelectHdl, ListBox* );
    DECL_LINK( ValModifyHdl, ComboBox* );
    DECL_LINK( CheckBoxHdl,  CheckBox* );
#endif
};


#endif // SC_PFILTDLG_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
