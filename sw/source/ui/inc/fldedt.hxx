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


#ifndef _FLDEDT_HXX
#define _FLDEDT_HXX
#include <sfx2/basedlgs.hxx>
#include <vcl/button.hxx>

class SwView;
class SwWrtShell;

/*--------------------------------------------------------------------
   Beschreibung:
 --------------------------------------------------------------------*/

class SwFldEditDlg : public SfxSingleTabDialog
{
    SwWrtShell* pSh;
    ImageButton aPrevBT;
    ImageButton aNextBT;
    PushButton  aAddressBT;

    DECL_LINK( AddressHdl, PushButton *pBt = 0 );
    DECL_LINK( NextPrevHdl, Button *pBt = 0 );

    void            Init();
    SfxTabPage*     CreatePage(sal_uInt16 nGroup);

public:

     SwFldEditDlg(SwView& rVw);
    ~SwFldEditDlg();

    DECL_LINK( OKHdl, Button * );

    virtual short   Execute();

    void            EnableInsert(sal_Bool bEnable);
    void            InsertHdl();
};


#endif
