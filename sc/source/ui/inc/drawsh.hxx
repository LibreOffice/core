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



#ifndef SC_DRAWSH_HXX
#define SC_DRAWSH_HXX

#include <sfx2/shell.hxx>
#include "shellids.hxx"
#include <sfx2/module.hxx>
#include <sfx2/sidebar/EnumContext.hxx>
#include <svx/svdmark.hxx>
#include <tools/link.hxx>
#include <rtl/ref.hxx>

class AbstractSvxNameDialog; //CHINA001 class SvxNameDialog;
class ScViewData;
class ScDrawView;
namespace svx { namespace sidebar {
class SelectionChangeHandler;
} }

class ScDrawShell : public SfxShell
{
    ScViewData* pViewData;
    ::rtl::Reference<svx::sidebar::SelectionChangeHandler> mpSelectionChangeHandler;

    DECL_LINK( NameObjectHdl, AbstractSvxNameDialog* );

#ifdef ISSUE66550_HLINK_FOR_SHAPES
    void SetHlinkForObject( SdrObject* pObj, const rtl::OUString& rHlnk );
#endif

protected:
    ScViewData* GetViewData()   { return pViewData; }

public:
    TYPEINFO();
    SFX_DECL_INTERFACE(SCID_DRAW_SHELL)

                    ScDrawShell(ScViewData* pData);
                    ~ScDrawShell();

    void    StateDisableItems( SfxItemSet &rSet );

    void    ExecDrawAttr(SfxRequest& rReq);
    void    GetDrawAttrState(SfxItemSet &rSet);
    void    GetAttrFuncState(SfxItemSet &rSet);

    void    ExecDrawFunc(SfxRequest& rReq);
    void    GetDrawFuncState(SfxItemSet &rSet);
    void    GetState(SfxItemSet &rSet);

    void    ExecFormText(SfxRequest& rReq);     // StarFontWork
    void    GetFormTextState(SfxItemSet& rSet);

    void    ExecuteHLink(SfxRequest& rReq);     // Hyperlink
    void    GetHLinkState(SfxItemSet& rSet);

    void    ExecFormatPaintbrush(SfxRequest& rReq);
    void    StateFormatPaintbrush(SfxItemSet& rSet);

    void    ExecuteMacroAssign( SdrObject* pObj, Window* pWin );
    void    ExecuteLineDlg( SfxRequest& rReq, sal_uInt16 nTabPage = 0xffff );
    void    ExecuteAreaDlg( SfxRequest& rReq, sal_uInt16 nTabPage = 0xffff );
    void    ExecuteTextAttrDlg( SfxRequest& rReq, sal_uInt16 nTabPage = 0xffff );

    ScDrawView* GetDrawView();

    sal_Bool    AreAllObjectsOnLayer(sal_uInt16 nLayerNo,const SdrMarkList& rMark);

    void GetDrawAttrStateForIFBX( SfxItemSet& rSet );
    ::sfx2::sidebar::EnumContext::Context GetContextForSelection (void);
};



#endif


