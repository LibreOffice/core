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


#ifndef _IDXMRK_HXX
#define _IDXMRK_HXX

#include <sfx2/childwin.hxx>

#include "swabstdlg.hxx"

class SwWrtShell;

/* -----------------07.09.99 08:02-------------------

 --------------------------------------------------*/
class SwInsertIdxMarkWrapper : public SfxChildWindow
{
    AbstractMarkFloatDlg*   pAbstDlg;
protected:
    SwInsertIdxMarkWrapper( Window *pParentWindow,
                            sal_uInt16 nId,
                            SfxBindings* pBindings,
                            SfxChildWinInfo* pInfo );

    SFX_DECL_CHILDWINDOW(SwInsertIdxMarkWrapper);

public:
    void    ReInitDlg(SwWrtShell& rWrtShell);
};

/* -----------------07.09.99 08:02-------------------

 --------------------------------------------------*/
class SwInsertAuthMarkWrapper : public SfxChildWindow
{
    AbstractMarkFloatDlg*   pAbstDlg;
protected:
    SwInsertAuthMarkWrapper(    Window *pParentWindow,
                            sal_uInt16 nId,
                            SfxBindings* pBindings,
                            SfxChildWinInfo* pInfo );

    SFX_DECL_CHILDWINDOW(SwInsertAuthMarkWrapper);

public:
    void    ReInitDlg(SwWrtShell& rWrtShell);
};

#endif // _IDXMRK_HXX

