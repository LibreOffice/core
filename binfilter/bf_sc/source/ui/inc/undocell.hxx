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

#ifndef SC_UNDOCELL_HXX
#define SC_UNDOCELL_HXX

#ifndef SC_UNDOBASE_HXX
#include "undobase.hxx"
#endif
namespace binfilter {

class ScDocShell;
class ScBaseCell;
class ScPatternAttr;
class EditTextObject;
class SdrUndoAction;
class ScDetOpList;
class ScDetOpData;
class ScRangeName;

//----------------------------------------------------------------------------







class ScUndoPutCell: public ScSimpleUndo
{
public:
                    ScUndoPutCell( ScDocShell* pNewDocShell,
                            const ScAddress& rNewPos,
                            ScBaseCell* pUndoCell, ScBaseCell* pRedoCell, BOOL bHeight );
    virtual 		~ScUndoPutCell();

    virtual BOOL	CanRepeat(SfxRepeatTarget& rTarget) const;

    virtual String	GetComment() const;

private:
    ScAddress		aPos;
    ScBaseCell*		pOldCell;
    ScBaseCell*		pEnteredCell;
    ULONG			nEndChangeAction;
    BOOL			bNeedHeight;

    void			SetChangeTrack();
};






class ScUndoNote: public ScSimpleUndo
{
public:
                    ScUndoNote( ScDocShell* pNewDocShell,
                                BOOL bShow, const ScAddress& rNewPos,
                                SdrUndoAction* pDraw );
    virtual 		~ScUndoNote();



private:
    BOOL			bIsShow;
    ScAddress		aPos;
    SdrUndoAction*	pDrawUndo;
};






class ScUndoRangeNames: public ScSimpleUndo
{
public:
                    ScUndoRangeNames( ScDocShell* pNewDocShell,
                                        ScRangeName* pOld, ScRangeName* pNew );
    virtual 		~ScUndoRangeNames();

    virtual BOOL	CanRepeat(SfxRepeatTarget& rTarget) const;

    virtual String	GetComment() const;

private:
    ScRangeName*	pOldRanges;
    ScRangeName*	pNewRanges;

};



} //namespace binfilter
#endif

