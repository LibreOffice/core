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


#ifndef _SWUI_PARDLG_HXX
#define _SWUI_PARDLG_HXX
#include "pardlg.hxx"

class SwParaDlg: public SfxTabDialog
{
    SwView& rView;
    sal_uInt16 nHtmlMode;
    sal_uInt8 nDlgMode;
    sal_Bool bDrawParaDlg;

    void PageCreated(sal_uInt16 nID, SfxTabPage& rPage);

public:
    SwParaDlg(  Window *pParent,
                SwView& rVw,
                const SfxItemSet&,
                sal_uInt8 nDialogMode,
                const String *pCollName = 0,
                sal_Bool bDraw = sal_False,
                sal_uInt16 nDefPage = 0);
    ~SwParaDlg();
};

#endif
