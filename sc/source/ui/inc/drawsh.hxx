/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#ifndef INCLUDED_SC_SOURCE_UI_INC_DRAWSH_HXX
#define INCLUDED_SC_SOURCE_UI_INC_DRAWSH_HXX

#include <sfx2/shell.hxx>
#include "shellids.hxx"
#include <sfx2/module.hxx>
#include <svx/svdmark.hxx>
#include <tools/link.hxx>
#include <rtl/ref.hxx>

class AbstractSvxObjectNameDialog;
class ScViewData;
class ScDrawView;
namespace svx { namespace sidebar {
class SelectionChangeHandler;
} }

class ScDrawShell : public SfxShell
{
    ScViewData* pViewData;
    ::rtl::Reference<svx::sidebar::SelectionChangeHandler> mpSelectionChangeHandler;

    DECL_LINK_TYPED( NameObjectHdl, AbstractSvxObjectNameDialog&, bool );

    void SetHlinkForObject( SdrObject* pObj, const OUString& rHlnk );

protected:
    virtual void    Activate(bool bMDI) SAL_OVERRIDE;
    ScViewData* GetViewData()   { return pViewData; }

public:
    TYPEINFO_OVERRIDE();
    SFX_DECL_INTERFACE(SCID_DRAW_SHELL)

private:
    /// SfxInterface initializer.
    static void InitInterface_Impl();

public:
                    ScDrawShell(ScViewData* pData);
                    virtual ~ScDrawShell();

    static void StateDisableItems( SfxItemSet &rSet );

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

    void    ExecuteMacroAssign( SdrObject* pObj, vcl::Window* pWin );
    void    ExecuteLineDlg( SfxRequest& rReq, sal_uInt16 nTabPage = 0xffff );
    void    ExecuteAreaDlg( SfxRequest& rReq, sal_uInt16 nTabPage = 0xffff );
    void    ExecuteTextAttrDlg( SfxRequest& rReq, sal_uInt16 nTabPage = 0xffff );

    ScDrawView* GetDrawView();

    static bool AreAllObjectsOnLayer(sal_uInt16 nLayerNo,const SdrMarkList& rMark);

    void GetDrawAttrStateForIFBX( SfxItemSet& rSet );
    ::rtl::OUString GetSidebarContextName();
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
