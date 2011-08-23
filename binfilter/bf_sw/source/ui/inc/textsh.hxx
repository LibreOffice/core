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

#ifndef _SWTEXTSH_HXX
#define _SWTEXTSH_HXX

#ifndef _SWBASESH_HXX
#include <basesh.hxx>
#endif
class Button; 
namespace binfilter {


class SvxHyperlinkItem;

class SwTextShell: public SwBaseShell
{


public:
    SFX_DECL_INTERFACE(SW_TEXTSHELL);
    TYPEINFO();


    void	GetState(SfxItemSet &);

    void	ExecInsert(SfxRequest &){DBG_BF_ASSERT(0, "STRIP");} ;//STRIP001 	void	ExecInsert(SfxRequest &);
    void	StateInsert(SfxItemSet&);
    void	ExecEnterNum(SfxRequest &){DBG_BF_ASSERT(0, "STRIP");} ;//STRIP001 	void	ExecEnterNum(SfxRequest &);
    void	ExecBasicMove(SfxRequest &){DBG_BF_ASSERT(0, "STRIP");} ;//STRIP001 	void	ExecBasicMove(SfxRequest &);
    void	ExecMove(SfxRequest &){DBG_BF_ASSERT(0, "STRIP");} ;//STRIP001 	void	ExecMove(SfxRequest &);
    void	ExecMovePage(SfxRequest &){DBG_BF_ASSERT(0, "STRIP");} ;//STRIP001 	void	ExecMovePage(SfxRequest &);
    void	ExecMoveCol(SfxRequest &){DBG_BF_ASSERT(0, "STRIP");} ;//STRIP001 	void	ExecMoveCol(SfxRequest &);
    void	ExecMoveLingu(SfxRequest &){DBG_BF_ASSERT(0, "STRIP");} ;//STRIP001 	void	ExecMoveLingu(SfxRequest &);
    void	ExecMoveMisc(SfxRequest &){DBG_BF_ASSERT(0, "STRIP");} ;//STRIP001 	void	ExecMoveMisc(SfxRequest &);
    void	StateField(SfxItemSet &);
    void	ExecIdx(SfxRequest &){DBG_BF_ASSERT(0, "STRIP");}  ;//STRIP001 	void	ExecIdx(SfxRequest &);
    void	GetIdxState(SfxItemSet &);
    void	ExecGlossary(SfxRequest &);//STRIP001 	void	ExecGlossary(SfxRequest &);

    void	ExecCharAttr(SfxRequest &){DBG_BF_ASSERT(0, "STRIP");} ;//STRIP001 	void	ExecCharAttr(SfxRequest &);
    void	ExecCharAttrArgs(SfxRequest &){DBG_BF_ASSERT(0, "STRIP");} ;//STRIP001 	void	ExecCharAttrArgs(SfxRequest &);
    void	ExecParaAttr(SfxRequest &){DBG_BF_ASSERT(0, "STRIP");} ;//STRIP001 	void	ExecParaAttr(SfxRequest &);
    void	ExecParaAttrArgs(SfxRequest &){DBG_BF_ASSERT(0, "STRIP");} ;//STRIP001 	void	ExecParaAttrArgs(SfxRequest &);
    void	ExecAttr(SfxRequest &);
    void	ExecDB(SfxRequest &){DBG_BF_ASSERT(0, "STRIP");} ;//STRIP001 	void	ExecDB(SfxRequest &);
    void 	ExecTransliteration(SfxRequest &){DBG_BF_ASSERT(0, "STRIP");} ;//STRIP001 	void 	ExecTransliteration(SfxRequest &);

    void	GetAttrState(SfxItemSet &);

             SwTextShell(SwView &rView);
    virtual ~SwTextShell();
};

} //namespace binfilter
#endif
