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

#ifndef SC_EDITSH_HXX
#define SC_EDITSH_HXX

#include <bf_sfx2/shell.hxx>
#include <bf_sfx2/module.hxx>
#include <tools/link.hxx>

#include "shellids.hxx"
class TransferableDataHelper;
class TransferableClipboardListener;
namespace binfilter {

class EditView;
class ScViewData;
class ScInputHandler;
class SvxURLField;

class ScEditShell : public SfxShell
{


public:
    SFX_DECL_INTERFACE(SCID_EDIT_SHELL);

    ScEditShell(EditView* pView, ScViewData* pData){DBG_BF_ASSERT(0, "STRIP");} //STRIP001 ScEditShell(EditView* pView, ScViewData* pData);
    void	Execute(SfxRequest& rReq){DBG_BF_ASSERT(0, "STRIP");} //STRIP001 //STRIP001 	void	Execute(SfxRequest& rReq);
    void	ExecuteTrans(SfxRequest& rReq){DBG_BF_ASSERT(0, "STRIP");} //STRIP001 //STRIP001 	void	ExecuteTrans(SfxRequest& rReq);
    void	GetState(SfxItemSet &rSet){DBG_BF_ASSERT(0, "STRIP");} //STRIP001 //STRIP001 	void	GetState(SfxItemSet &rSet);
    void	GetClipState(SfxItemSet& rSet){DBG_BF_ASSERT(0, "STRIP");} //STRIP001 //STRIP001 	void	GetClipState(SfxItemSet& rSet);
    void	ExecuteAttr(SfxRequest& rReq){DBG_BF_ASSERT(0, "STRIP");} //STRIP001 //STRIP001 	void	ExecuteAttr(SfxRequest& rReq);
    void	GetAttrState(SfxItemSet &rSet){DBG_BF_ASSERT(0, "STRIP");} //STRIP001 //STRIP001 	void	GetAttrState(SfxItemSet &rSet);
    void	ExecuteUndo(SfxRequest& rReq){DBG_BF_ASSERT(0, "STRIP");} //STRIP001 //STRIP001 	void	ExecuteUndo(SfxRequest& rReq);
    void	GetUndoState(SfxItemSet &rSet){DBG_BF_ASSERT(0, "STRIP");} //STRIP001 //STRIP001 	void	GetUndoState(SfxItemSet &rSet);
};



} //namespace binfilter
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
