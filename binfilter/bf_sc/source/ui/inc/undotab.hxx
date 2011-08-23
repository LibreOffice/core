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

#ifndef SC_UNDOTAB_HXX
#define SC_UNDOTAB_HXX

#ifndef SC_UNDOBASE_HXX
#include "undobase.hxx"
#endif
#ifndef SC_MARKDATA_HXX
#include "markdata.hxx"
#endif

#ifndef _SVSTDARR_USHORTS

#define _SVSTDARR_USHORTS
#include <bf_svtools/svstdarr.hxx>

#endif

#ifndef _SVSTDARR_STRINGS

#define _SVSTDARR_STRINGS
#include <bf_svtools/svstdarr.hxx>

#endif
namespace binfilter {
class ScDocShell;
class ScDocument;
class SdrUndoAction;
class ScPrintRangeSaver;
class SdrObject;

//----------------------------------------------------------------------------

class ScUndoInsertTab : public ScSimpleUndo
{
public:
                    ScUndoInsertTab(
                            ScDocShell* pNewDocShell,
                            USHORT nTabNum,
                            BOOL bApp,
                            const String& rNewName);
    virtual			~ScUndoInsertTab();

    virtual BOOL	CanRepeat(SfxRepeatTarget& rTarget) const;

    virtual String	GetComment() const;

private:
    String			sNewName;
    SdrUndoAction*	pDrawUndo;
    ULONG			nEndChangeAction;
    USHORT			nTab;
    BOOL			bAppend;

    void			SetChangeTrack();
};





class ScUndoRenameTab: public ScSimpleUndo
{
public:
                    ScUndoRenameTab(
                            ScDocShell* pNewDocShell,
                            USHORT nT,
                            const String& rOldName,
                            const String& rNewName);
    virtual			~ScUndoRenameTab();

    virtual BOOL	CanRepeat(SfxRepeatTarget& rTarget) const;

    virtual String	GetComment() const;

private:
    USHORT	nTab;
    String	sOldName;
    String	sNewName;

};












class ScUndoShowHideTab : public ScSimpleUndo
{
public:
                    ScUndoShowHideTab(
                            ScDocShell* pShell,
                            USHORT nNewTab, BOOL bNewShow );
    virtual			~ScUndoShowHideTab();

    virtual BOOL	CanRepeat(SfxRepeatTarget& rTarget) const;

    virtual String	GetComment() const;

private:
    USHORT	nTab;
    BOOL	bShow;

};




class ScUndoPrintRange : public ScSimpleUndo
{
public:
                    ScUndoPrintRange( ScDocShell* pShell, USHORT nNewTab,
                                        ScPrintRangeSaver* pOld, ScPrintRangeSaver* pNew );
    virtual			~ScUndoPrintRange();

private:
    USHORT				nTab;
    ScPrintRangeSaver*	pOldRanges;
    ScPrintRangeSaver*	pNewRanges;

};







} //namespace binfilter
#endif

