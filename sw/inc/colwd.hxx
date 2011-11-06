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


#ifndef _COLWD_HXX
#define _COLWD_HXX


#include <svx/stddlg.hxx>
#include <vcl/fixed.hxx>
#include <vcl/field.hxx>
#ifndef _SV_BUTTON_HXX //autogen
#include <vcl/button.hxx>
#endif



class SwTableFUNC;

class SwTableWidthDlg : public SvxStandardDialog
{
    FixedLine        aWidthFL;
    FixedText       aColFT;
    NumericField    aColEdit;
    FixedText       aWidthFT;
    MetricField     aWidthEdit;
    OKButton        aOKBtn;
    CancelButton    aCancelBtn;
    HelpButton      aHelpBtn;
    SwTableFUNC     &rFnc;

protected:
    virtual void    Apply();
    DECL_LINK( LoseFocusHdl, Edit* pEdt=0 );

public:
    SwTableWidthDlg(Window *pParent, SwTableFUNC &rFnc );
};

#endif
