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


#ifndef _SVX_INSRC_HXX
#define _SVX_INSRC_HXX

#include <svx/stddlg.hxx>

#include <vcl/fixed.hxx>
#include <vcl/field.hxx>
#include <vcl/button.hxx>
#include <vcl/group.hxx>
#include <vcl/button.hxx>

#include <tools/string.hxx>

class SvxInsRowColDlg : public SvxAbstractInsRowColDlg, public ModalDialog
{
    FixedText       aCount;
    NumericField    aCountEdit;
    FixedLine        aInsFL;

    RadioButton     aBeforeBtn;
    RadioButton     aAfterBtn;
    FixedLine        aPosFL;

    String          aRow;
    String          aCol;

    OKButton        aOKBtn;
    CancelButton    aCancelBtn;
    HelpButton      aHelpBtn;

    bool            bColumn;

public:
    SvxInsRowColDlg( Window* pParent, bool bCol, const rtl::OString& sHelpId );

    virtual short Execute(void);

    virtual bool isInsertBefore() const;
    virtual sal_uInt16 getInsertCount() const;
};

#endif

