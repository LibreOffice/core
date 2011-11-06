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



#ifndef _MULTMRK_HXX
#define _MULTMRK_HXX


#include <svx/stddlg.hxx>

#ifndef _FIXED_HXX //autogen
#include <vcl/fixed.hxx>
#endif
#include <svtools/stdctrl.hxx>

#ifndef _LSTBOX_HXX //autogen
#include <vcl/lstbox.hxx>
#endif
#include <vcl/fixed.hxx>

#ifndef _BUTTON_HXX //autogen
#include <vcl/button.hxx>
#endif

class SwTOXMgr;

/*--------------------------------------------------------------------
     Beschreibung:  Markierung fuer Verzeichniseintrag einfuegen
 --------------------------------------------------------------------*/

class SwMultiTOXMarkDlg : public SvxStandardDialog
{
    DECL_LINK( SelectHdl, ListBox * );

    FixedLine           aTOXFL;
    FixedText           aEntryFT;
    FixedInfo           aTextFT;
    FixedText           aTOXFT;
    ListBox             aTOXLB;
    OKButton            aOkBT;
    CancelButton        aCancelBT;

    SwTOXMgr           &rMgr;
    sal_uInt16              nPos;

    void                Apply();
public:
    SwMultiTOXMarkDlg( Window* pParent, SwTOXMgr &rTOXMgr );
    ~SwMultiTOXMarkDlg();
};


#endif // _MULTMRK_HXX

