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


#ifndef _SVX_SPLITCELLDLG_HXX
#define _SVX_SPLITCELLDLG_HXX

#include <vcl/fixed.hxx>
#include <vcl/field.hxx>
#include <vcl/button.hxx>
#include <vcl/button.hxx>

#include <svx/stddlg.hxx>
#include <svx/svxdlg.hxx>

class SvxSplitTableDlg : public SvxAbstractSplittTableDialog, public SvxStandardDialog
{
    FixedLine           maCountFL;
    FixedText           maCountLbl;
    NumericField        maCountEdit;
    FixedLine           maDirFL;
    ImageRadioButton    maHorzBox;
    ImageRadioButton    maVertBox;
    CheckBox            maPropCB;
    OKButton            maOKBtn;
    CancelButton        maCancelBtn;
    HelpButton          maHelpBtn;

    long                mnMaxVertical;
    long                mnMaxHorizontal;
protected:
    virtual void Apply();

public:
    SvxSplitTableDlg(Window *pParent, bool bIsTableVertical, long nMaxVertical, long nMaxHorizontal );
    virtual ~SvxSplitTableDlg();

    DECL_LINK( ClickHdl, Button * );

    virtual bool IsHorizontal() const;
    virtual bool IsProportional() const;
    virtual long GetCount() const;

    virtual short Execute();
};

#endif
