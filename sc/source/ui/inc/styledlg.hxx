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



#ifndef SC_STYLEDLG_HXX
#define SC_STYLEDLG_HXX

#ifndef _SFX_HXX
#endif

#include <sfx2/styledlg.hxx>

//==================================================================

class SfxStyleSheetBase;

class ScStyleDlg : public SfxStyleDialog
{
public:
                ScStyleDlg( Window*             pParent,
                            SfxStyleSheetBase&  rStyleBase,
                            sal_uInt16              nRscId );
                ~ScStyleDlg();

protected:
    virtual void                PageCreated( sal_uInt16 nPageId, SfxTabPage& rTabPage );
    virtual const SfxItemSet*   GetRefreshedSet();

private:
    sal_uInt16 nDlgRsc;
};


#endif // SC_STYLEDLG_HXX


