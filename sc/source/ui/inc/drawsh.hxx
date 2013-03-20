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

#pragma once
#if 1

#include <sfx2/shell.hxx>
#include "shellids.hxx"
#include <sfx2/module.hxx>
#include <svx/svdmark.hxx>
#include <tools/link.hxx>

class AbstractSvxNameDialog;
class ScViewData;
class ScDrawView;

class ScDrawShell : public SfxShell
{
    ScViewData* pViewData;

    DECL_LINK( NameObjectHdl, AbstractSvxNameDialog* );

    void SetHlinkForObject( SdrObject* pObj, const rtl::OUString& rHlnk );

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
};



#endif


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
