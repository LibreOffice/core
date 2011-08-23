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
#ifndef _SWDRWTXTSH_HXX
#define _SWDRWTXTSH_HXX


#ifndef _SFX_SHELL_HXX //autogen
#include <bf_sfx2/shell.hxx>
#endif
#include "shellid.hxx"
namespace binfilter {

class OutlinerView;
class SdrOutliner;
class SdrView;
class SwView;
class SfxModule;
class SwWrtShell;

class SwDrawTextShell: public SfxShell
{
    SwView		&rView;

    SdrView		*pSdrView;
    OutlinerView*pOLV;
    SdrOutliner	*pOutliner;

    BOOL 		bRotate : 1;
    BOOL 		bSelMove: 1;

    void 		SetAttrToMarked(const SfxItemSet& rAttr);
    void		InsertSymbol();
    BOOL		IsTextEdit();
public:
    SFX_DECL_INTERFACE(SW_DRWTXTSHELL);
    TYPEINFO();

    SwView	   &GetView() { return rView; }
    SwWrtShell &GetShell();

                 SwDrawTextShell(SwView &rView);

    void		StateDisableItems(SfxItemSet &){DBG_BF_ASSERT(0, "STRIP");}; //STRIP001 void		StateDisableItems(SfxItemSet &);

    void		Execute(SfxRequest &){DBG_BF_ASSERT(0, "STRIP");} ;//STRIP001 	void		Execute(SfxRequest &);
    void		ExecDraw(SfxRequest &){DBG_BF_ASSERT(0, "STRIP");}; //STRIP001 void		ExecDraw(SfxRequest &);
    void		GetState(SfxItemSet &){DBG_BF_ASSERT(0, "STRIP");} ;//STRIP001 	void		GetState(SfxItemSet &);
    void		GetDrawTxtCtrlState(SfxItemSet&){DBG_BF_ASSERT(0, "STRIP");} ;//STRIP001 	void		GetDrawTxtCtrlState(SfxItemSet&);

    void		ExecFontWork(SfxRequest& rReq){DBG_BF_ASSERT(0, "STRIP");} ;//STRIP001 	void		ExecFontWork(SfxRequest& rReq);
    void		StateFontWork(SfxItemSet& rSet){DBG_BF_ASSERT(0, "STRIP");} ;//STRIP001 	void		StateFontWork(SfxItemSet& rSet);
    void		ExecFormText(SfxRequest& rReq){DBG_BF_ASSERT(0, "STRIP");} ;//STRIP001 	void		ExecFormText(SfxRequest& rReq);
    void		GetFormTextState(SfxItemSet& rSet){DBG_BF_ASSERT(0, "STRIP");} ;//STRIP001 	void		GetFormTextState(SfxItemSet& rSet);
    void		ExecDrawLingu(SfxRequest &rReq){DBG_BF_ASSERT(0, "STRIP");} ;//STRIP001 	void		ExecDrawLingu(SfxRequest &rReq);
    void		ExecUndo(SfxRequest &rReq){DBG_BF_ASSERT(0, "STRIP");}; //STRIP001 void		ExecUndo(SfxRequest &rReq);
    void		StateUndo(SfxItemSet &rSet){DBG_BF_ASSERT(0, "STRIP");}; //STRIP001 void		StateUndo(SfxItemSet &rSet);
    void		ExecClpbrd(SfxRequest &rReq){DBG_BF_ASSERT(0, "STRIP");}; //STRIP001 void		ExecClpbrd(SfxRequest &rReq);
    void		StateClpbrd(SfxItemSet &rSet){DBG_BF_ASSERT(0, "STRIP");}; //STRIP001 void		StateClpbrd(SfxItemSet &rSet);
    void		StateInsert(SfxItemSet &rSet){DBG_BF_ASSERT(0, "STRIP");} ;//STRIP001 	void		StateInsert(SfxItemSet &rSet);
    void 		ExecTransliteration(SfxRequest &){DBG_BF_ASSERT(0, "STRIP");} ;//STRIP001 	void 		ExecTransliteration(SfxRequest &);

};

} //namespace binfilter
#endif
