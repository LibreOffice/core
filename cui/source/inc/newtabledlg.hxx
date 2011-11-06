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


#ifndef _SVX_NEWTABLEDLG_HXX
#define _SVX_NEWTABLEDLG_HXX

#include <vcl/fixed.hxx>
#include <vcl/field.hxx>
#include <vcl/button.hxx>

#include <svx/stddlg.hxx>
#include <svx/svxdlg.hxx>

class SvxNewTableDialog : public SvxAbstractNewTableDialog, public ModalDialog
{
private:
    FixedText           maFtColumns;
    NumericField        maNumColumns;
    FixedText           maFtRows;
    NumericField        maNumRows;

    FixedLine           maFlSep;
    HelpButton          maHelpButton;
    OKButton            maOkButton;
    CancelButton        maCancelButton;

public:
    SvxNewTableDialog( Window* pWindow );

    virtual short Execute(void);
    virtual void Apply(void);

    virtual sal_Int32 getRows() const;
    virtual sal_Int32 getColumns() const;
};

#endif // _SVX_NEWTABLEDLG_HXX


