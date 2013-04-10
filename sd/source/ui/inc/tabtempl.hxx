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




#ifndef SD_TABTEMPL_HXX
#define SD_TABTEMPL_HXX

#include <svx/tabarea.hxx>
#include <sfx2/styledlg.hxx>
class SdrModel;
class SfxObjectShell;
class SdrView;
class XColorList;
class XGradientList;
class XBitmapList;
class XDashList;
class XHatchList;
class XLineEndList;


/*************************************************************************
|*
|* Vorlagen-Tab-Dialog
|*
\************************************************************************/
class SdTabTemplateDlg : public SfxStyleDialog
{
private:
    const SfxObjectShell&   rDocShell;
    SdrView*                pSdrView;

    XColorList*     pColorTab;
    XGradientList*      pGradientList;
    XHatchList*         pHatchingList;
    XBitmapList*        pBitmapList;
    XDashList*          pDashList;
    XLineEndList*       pLineEndList;

    sal_uInt16              nPageType;
    sal_uInt16              nDlgType;
    sal_uInt16              nPos;
    ChangeType          nColorTableState;
    ChangeType          nBitmapListState;
    ChangeType          nGradientListState;
    ChangeType          nHatchingListState;

    virtual void                PageCreated( sal_uInt16 nId, SfxTabPage &rPage );
    virtual const SfxItemSet*   GetRefreshedSet();

public:
                    SdTabTemplateDlg( Window* pParent,
                            const SfxObjectShell* pDocShell,
                            SfxStyleSheetBase& rStyleBase,
                            SdrModel* pModel,
                            SdrView* pView );
                    ~SdTabTemplateDlg();

};


#endif // SD_TABTEMPL_HXX

