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



#ifndef SC_DELCODLG_HXX
#define SC_DELCODLG_HXX

#include <vcl/dialog.hxx>
#include <vcl/fixed.hxx>
#include <vcl/button.hxx>
#include "global.hxx"

//------------------------------------------------------------------------

class ScDeleteContentsDlg : public ModalDialog
{
private:
    FixedLine       aFlFrame;
    CheckBox        aBtnDelAll;
    CheckBox        aBtnDelStrings;
    CheckBox        aBtnDelNumbers;
    CheckBox        aBtnDelDateTime;
    CheckBox        aBtnDelFormulas;
    CheckBox        aBtnDelNotes;
    CheckBox        aBtnDelAttrs;
    CheckBox        aBtnDelObjects;
    OKButton        aBtnOk;
    CancelButton    aBtnCancel;
    HelpButton      aBtnHelp;

    sal_Bool            bObjectsDisabled;

    static sal_Bool     bPreviousAllCheck;
    static sal_uInt16   nPreviousChecks;

    void DisableChecks( sal_Bool bDelAllChecked = sal_True );
    DECL_LINK( DelAllHdl, void * );

public:
            ScDeleteContentsDlg( Window* pParent,
                                 sal_uInt16  nCheckDefaults = 0 );
            ~ScDeleteContentsDlg();
    void    DisableObjects();

    sal_uInt16  GetDelContentsCmdBits() const;
};


#endif // SC_DELCODLG_HXX


