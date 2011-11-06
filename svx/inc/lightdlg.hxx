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




#ifndef _SVX_LIGHT3D_HXX_
#define _SVX_LIGHT3D_HXX_

#include <svx/dlgctl3d.hxx>
#include <vcl/fixed.hxx>
#include <vcl/group.hxx>
#include <sfx2/tabdlg.hxx>
#include <sfx2/basedlgs.hxx>

#ifndef _FIELD_HXX //autogen
#include <vcl/field.hxx>
#endif
#include <svx/dlgctrl.hxx>

/*************************************************************************
|*
|* 3D-Tab-Dialog
|*
\************************************************************************/

class Svx3DTabDialog : public SfxTabDialog
{
private:
    const SfxItemSet&   rOutAttrs;

public:
            Svx3DTabDialog( Window* pParent, const SfxItemSet* pAttr );
            ~Svx3DTabDialog();
};

/*************************************************************************
|*
|* 3D-SingleTab-Dialog
|*
\************************************************************************/
class Svx3DSingleTabDialog : public SfxSingleTabDialog
{
private:
    const SfxItemSet&   rOutAttrs;

public:
        Svx3DSingleTabDialog( Window* pParent, const SfxItemSet* pAttr );
        ~Svx3DSingleTabDialog();
};

#endif // _SVX_LIGHT3D_HXX_

