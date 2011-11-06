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



#ifndef _SVX_TAB_HYPERLINK_HXX
#define _SVX_TAB_HYPERLINK_HXX

// include ---------------------------------------------------------------

#include <sfx2/childwin.hxx>
#include "svx/svxdllapi.h"

/*************************************************************************
|*
|* Hyperlink-Dialog
|*
\************************************************************************/

class AbstractSvxHpLinkDlg;

class SVX_DLLPUBLIC SvxHlinkDlgWrapper : public SfxChildWindow
{
private:
    AbstractSvxHpLinkDlg*   mpDlg;

public:
    SvxHlinkDlgWrapper( Window*pParent, sal_uInt16 nId,
                        SfxBindings* pBindings, SfxChildWinInfo* pInfo );

    SFX_DECL_CHILDWINDOW(SvxHlinkDlgWrapper);
    virtual sal_Bool    QueryClose();
};

#endif // _SVX_TAB_HYPERLINK_HXX

