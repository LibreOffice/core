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



#ifndef SC_REDCOM_HXX
#define SC_REDCOM_HXX

//CHINA001 #ifndef _SVX_POSTDLG_HXX //autogen
//CHINA001 #include <svx/postdlg.hxx>
//CHINA001 #endif

#include "chgtrack.hxx"

class ScDocShell;
class AbstractSvxPostItDialog; //CHINA001

class ScRedComDialog
{
private:

    ScChangeAction  *pChangeAction;
    ScDocShell      *pDocShell;
    String          aComment;
    AbstractSvxPostItDialog* pDlg;

    DECL_LINK( PrevHdl, AbstractSvxPostItDialog* );
    DECL_LINK( NextHdl, AbstractSvxPostItDialog* );

protected:

    void    ReInit(ScChangeAction *);
    void    SelectCell();

    ScChangeAction *FindPrev(ScChangeAction *pAction);
    ScChangeAction *FindNext(ScChangeAction *pAction);

public:

    ScRedComDialog( Window* pParent, const SfxItemSet& rCoreSet,
                    ScDocShell *,ScChangeAction *,sal_Bool bPrevNext = sal_False);
    ~ScRedComDialog();

    short Execute();
};

#endif

