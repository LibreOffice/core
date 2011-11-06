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



#ifndef SC_MVTABDLG_HXX
#define SC_MVTABDLG_HXX


#include "address.hxx"
#include <vcl/dialog.hxx>
#include <vcl/button.hxx>
#include <vcl/lstbox.hxx>
#include <vcl/fixed.hxx>

#include <layout/layout.hxx>
#include <layout/layout-pre.hxx>

//------------------------------------------------------------------------

class ScMoveTableDlg : public ModalDialog
{
public:
                    ScMoveTableDlg( Window* pParent );
                    ~ScMoveTableDlg();

    sal_uInt16  GetSelectedDocument     () const;
    SCTAB   GetSelectedTable        () const;
    sal_Bool    GetCopyTable            () const;
    void    SetCopyTable            (sal_Bool bFlag=sal_True);
    void    EnableCopyTable         (sal_Bool bFlag=sal_True);

private:
    FixedText       aFtDoc;
    ListBox         aLbDoc;
    FixedText       aFtTable;
    ListBox         aLbTable;
    CheckBox        aBtnCopy;
    OKButton        aBtnOk;
    CancelButton    aBtnCancel;
    HelpButton      aBtnHelp;

    sal_uInt16          nDocument;
    SCTAB           nTable;
    sal_Bool            bCopyTable;
    //--------------------------------------
    void    Init            ();
    void    InitDocListBox  ();
    DECL_LINK( OkHdl, void * );
    DECL_LINK( SelHdl, ListBox * );
};

#include <layout/layout-post.hxx>

#endif // SC_MVTABDLG_HXX


