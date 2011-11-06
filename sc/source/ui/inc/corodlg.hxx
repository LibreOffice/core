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



#ifndef SC_CORODLG_HXX
#define SC_CORODLG_HXX

#ifndef _DIALOG_HXX //autogen
#include <vcl/dialog.hxx>
#endif
#ifndef _BUTTON_HXX //autogen
#include <vcl/button.hxx>
#endif
#ifndef _FIXED_HXX //autogen
#include <vcl/fixed.hxx>
#endif

#include "sc.hrc"
#include "scresid.hxx"

//------------------------------------------------------------------------

class ScColRowLabelDlg : public ModalDialog
{
public:
            ScColRowLabelDlg( Window* pParent,
                              sal_Bool bCol = sal_False,
                              sal_Bool bRow = sal_False )
                : ModalDialog( pParent, ScResId( RID_SCDLG_CHARTCOLROW ) ),
                  aFlColRow  ( this, ScResId(6) ),
                  aBtnRow    ( this, ScResId(2) ),
                  aBtnCol    ( this, ScResId(1) ),
                  aBtnOk     ( this, ScResId(3) ),
                  aBtnCancel ( this, ScResId(4) ),
                  aBtnHelp   ( this, ScResId(5) )
                {
                    FreeResource();
                    aBtnCol.Check( bCol );
                    aBtnRow.Check( bRow );
                }

    sal_Bool IsCol() { return aBtnCol.IsChecked(); }
    sal_Bool IsRow() { return aBtnRow.IsChecked(); }

private:
    FixedLine       aFlColRow;
    CheckBox        aBtnRow;
    CheckBox        aBtnCol;
    OKButton        aBtnOk;
    CancelButton    aBtnCancel;
    HelpButton      aBtnHelp;
};


#endif



