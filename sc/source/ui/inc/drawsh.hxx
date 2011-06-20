/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#ifndef SC_DRAWSH_HXX
#define SC_DRAWSH_HXX

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
