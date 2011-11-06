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


#ifndef _SVX_MULTIPAT_HXX
#define _SVX_MULTIPAT_HXX

// include ---------------------------------------------------------------

#include <vcl/dialog.hxx>
#include <vcl/lstbox.hxx>
#ifndef _SV_BUTTON_HXX
#include <vcl/button.hxx>
#endif
#include <vcl/fixed.hxx>

#include "radiobtnbox.hxx"

// define ----------------------------------------------------------------

// different delimiter for Unix (:) and Windows (;)

#ifdef UNX
#define CLASSPATH_DELIMITER ':'
#else
#define CLASSPATH_DELIMITER ';'
#endif

// forward ---------------------------------------------------------------

struct MultiPath_Impl;

// class SvxMultiPathDialog ----------------------------------------------

class SvxMultiPathDialog : public ModalDialog
{
protected:
    FixedLine                   aPathFL;
    ListBox                     aPathLB;
    svx::SvxRadioButtonListBox  aRadioLB;
    FixedText                   aRadioFT;
    PushButton                  aAddBtn;
    PushButton                  aDelBtn;
    OKButton                    aOKBtn;
    CancelButton                aCancelBtn;
    HelpButton                  aHelpButton;

    MultiPath_Impl* pImpl;

    DECL_LINK( AddHdl_Impl, PushButton * );
    DECL_LINK( DelHdl_Impl, PushButton * );
    DECL_LINK( SelectHdl_Impl, void * );
    DECL_LINK( CheckHdl_Impl, svx::SvxRadioButtonListBox * );

public:
    SvxMultiPathDialog( Window* pParent, sal_Bool bEmptyAllowed = sal_False );
    ~SvxMultiPathDialog();

    String          GetPath() const;
    void            SetPath( const String& rPath );
    void            SetClassPathMode();
    sal_Bool        IsClassPathMode() const;
    void            EnableRadioButtonMode();
};

#endif // #ifndef _SVX_MULTIPAT_HXX

