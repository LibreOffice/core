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


#ifndef RPTUI_DLGPAGE_HXX
#define RPTUI_DLGPAGE_HXX

#include <sfx2/tabdlg.hxx>

namespace rptui
{
/*************************************************************************
|*
|* Seite einrichten-Tab-Dialog
|*
\************************************************************************/
class ORptPageDialog : public SfxTabDialog
{
private:
    const SfxItemSet&   rOutAttrs;

    ORptPageDialog(const ORptPageDialog&);
    void operator =(const ORptPageDialog&);
public:

    ORptPageDialog( Window* pParent, const SfxItemSet* pAttr,sal_uInt16 _nPageId);
    virtual ~ORptPageDialog() {}
};
// =============================================================================
} // namespace rptui
// =============================================================================
#endif // RPTUI_DLGPAGE_HXX
