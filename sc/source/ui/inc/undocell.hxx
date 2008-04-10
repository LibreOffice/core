/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: undocell.hxx,v $
 * $Revision: 1.5 $
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

#include "undobase.hxx"
#include "postit.hxx"

class ScDocShell;
class ScBaseCell;
class ScPatternAttr;
class EditTextObject;
class SdrUndoAction;
class ScDetOpList;
class ScDetOpData;
class ScRangeName;

//----------------------------------------------------------------------------

class ScUndoCursorAttr: public ScSimpleUndo
{
public:
                    TYPEINFO();
                    ScUndoCursorAttr( ScDocShell* pNewDocShell,
                            SCCOL nNewCol, SCROW nNewRow, SCTAB nNewTab,
                            const ScPatternAttr* pOldPat, const ScPatternAttr* pNewPat,
                            const ScPatternAttr* pApplyPat, BOOL bAutomatic );
    virtual         ~ScUndoCursorAttr();

    virtual void    Undo();
    virtual void    Redo();
    virtual void    Repeat(SfxRepeatTarget& rTarget);
    virtual BOOL    CanRepeat(SfxRepeatTarget& rTarget) const;

    virtual String  GetComment() const;

private:
    SCCOL           nCol;
    SCROW           nRow;
    SCTAB           nTab;
    ScPatternAttr*  pOldPattern;
    ScPatternAttr*  pNewPattern;
    ScPatternAttr*  pApplyPattern;
    BOOL            bIsAutomatic;

    void            DoChange( const ScPatternAttr* pWhichPattern ) const;
};


class ScUndoEnterData: public ScSimpleUndo
{
public:
                    TYPEINFO();
                    ScUndoEnterData( ScDocShell* pNewDocShell,
                            SCCOL nNewCol, SCROW nNewRow, SCTAB nNewTab,
                            SCTAB nNewCount, SCTAB* pNewTabs,
                            ScBaseCell** ppOldData, BOOL* pHasForm, ULONG* pOldForm,
                            const String& rNewStr, EditTextObject* pObj = NULL );
    virtual         ~ScUndoEnterData();

    virtual void    Undo();
    virtual void    Redo();
    virtual void    Repeat(SfxRepeatTarget& rTarget);
    virtual BOOL    CanRepeat(SfxRepeatTarget& rTarget) const;

    virtual String  GetComment() const;

private:
    String          aNewString;
    SCTAB*          pTabs;
    ScBaseCell**    ppOldCells;
    BOOL*           pHasFormat;
    ULONG*          pOldFormats;
    EditTextObject* pNewEditData;
    ULONG           nEndChangeAction;
    SCCOL           nCol;
    SCROW           nRow;
    SCTAB           nTab;
    SCTAB           nCount;             //  markierte Tabellen

    void            DoChange() const;
    void            SetChangeTrack();
};


class ScUndoEnterValue: public ScSimpleUndo
{
public:
                    TYPEINFO();
                    ScUndoEnterValue( ScDocShell* pNewDocShell,
                            const ScAddress& rNewPos,
                            ScBaseCell* pUndoCell, double nVal, BOOL bHeight );
    virtual         ~ScUndoEnterValue();

    virtual void    Undo();
    virtual void    Redo();
    virtual void    Repeat(SfxRepeatTarget& rTarget);
    virtual BOOL    CanRepeat(SfxRepeatTarget& rTarget) const;

    virtual String  GetComment() const;

private:
    ScAddress       aPos;
    ScBaseCell*     pOldCell;
    double          nValue;
    ULONG           nEndChangeAction;
    BOOL            bNeedHeight;

    void            SetChangeTrack();
};


class ScUndoPutCell: public ScSimpleUndo
{
public:
                    TYPEINFO();
                    ScUndoPutCell( ScDocShell* pNewDocShell,
                            const ScAddress& rNewPos,
                            ScBaseCell* pUndoCell, ScBaseCell* pRedoCell, BOOL bHeight );
    virtual         ~ScUndoPutCell();

    virtual void    Undo();
    virtual void    Redo();
    virtual void    Repeat(SfxRepeatTarget& rTarget);
    virtual BOOL    CanRepeat(SfxRepeatTarget& rTarget) const;

    virtual String  GetComment() const;

private:
    ScAddress       aPos;
    ScBaseCell*     pOldCell;
    ScBaseCell*     pEnteredCell;
    ULONG           nEndChangeAction;
    BOOL            bNeedHeight;

    void            SetChangeTrack();
};


class ScUndoPageBreak: public ScSimpleUndo
{
public:
                    TYPEINFO();
                    ScUndoPageBreak( ScDocShell* pNewDocShell,
                            SCCOL nNewCol, SCROW nNewRow, SCTAB nNewTab,
                            BOOL bNewColumn, BOOL bNewInsert );
    virtual         ~ScUndoPageBreak();

    virtual void    Undo();
    virtual void    Redo();
    virtual void    Repeat(SfxRepeatTarget& rTarget);
    virtual BOOL    CanRepeat(SfxRepeatTarget& rTarget) const;

    virtual String  GetComment() const;

private:
    SCCOL           nCol;
    SCROW           nRow;
    SCTAB           nTab;
    BOOL            bColumn;        // Spalten- oder Zeilenumbruch
    BOOL            bInsert;        // Einfuegen oder Loeschen

    void            DoChange( BOOL bInsert ) const;
};

class ScUndoPrintZoom: public ScSimpleUndo
{
public:
                    TYPEINFO();
                    ScUndoPrintZoom( ScDocShell* pNewDocShell, SCTAB nT,
                                    USHORT nOS, USHORT nOP, USHORT nNS, USHORT nNP );
    virtual         ~ScUndoPrintZoom();

    virtual void    Undo();
    virtual void    Redo();
    virtual void    Repeat(SfxRepeatTarget& rTarget);
    virtual BOOL    CanRepeat(SfxRepeatTarget& rTarget) const;

    virtual String  GetComment() const;

private:
    SCTAB           nTab;
    USHORT          nOldScale;
    USHORT          nOldPages;
    USHORT          nNewScale;
    USHORT          nNewPages;

    void            DoChange( BOOL bUndo );
};

class ScUndoThesaurus: public ScSimpleUndo
{
public:
                    TYPEINFO();
                    ScUndoThesaurus( ScDocShell* pNewDocShell,
                            SCCOL nNewCol, SCROW nNewRow, SCTAB nNewTab,
                            const String& rNewUndoStr, const EditTextObject* pUndoTObj,
                            const String& rNewRedoStr, const EditTextObject* pRedoTObj);
    virtual         ~ScUndoThesaurus();

    virtual void    Undo();
    virtual void    Redo();
    virtual void    Repeat(SfxRepeatTarget& rTarget);
    virtual BOOL    CanRepeat(SfxRepeatTarget& rTarget) const;

    virtual String  GetComment() const;

private:
    SCCOL           nCol;
    SCROW           nRow;
    SCTAB           nTab;
    String          aUndoStr;           // Daten bei StringZelle
    EditTextObject* pUndoTObject;       //       bei EditZelle
    String          aRedoStr;
    EditTextObject* pRedoTObject;
    ULONG           nEndChangeAction;

    void            DoChange( BOOL bUndo, const String& rStr,
                                const EditTextObject* pTObj );
    void            SetChangeTrack( ScBaseCell* pOldCell );
};


class ScUndoNote: public ScSimpleUndo
{
public:
                    TYPEINFO();
                    ScUndoNote( ScDocShell* pNewDocShell,
                                BOOL bShow, const ScAddress& rNewPos,
                                SdrUndoAction* pDraw );
    virtual         ~ScUndoNote();

    virtual void    Undo();
    virtual void    Redo();
    virtual void    Repeat(SfxRepeatTarget& rTarget);
    virtual BOOL    CanRepeat(SfxRepeatTarget& rTarget) const;

    virtual String  GetComment() const;

private:
    BOOL            bIsShow;
    ScAddress       aPos;
    SdrUndoAction*  pDrawUndo;
};


class ScUndoEditNote: public ScSimpleUndo
{
public:
                    TYPEINFO();
                    ScUndoEditNote( ScDocShell* pNewDocShell,
                                    const ScAddress& rNewPos,
                                    const ScPostIt& rOld,
                                    const ScPostIt& rNew );
    virtual         ~ScUndoEditNote();

    virtual void    Undo();
    virtual void    Redo();
    virtual void    Repeat(SfxRepeatTarget& rTarget);
    virtual BOOL    CanRepeat(SfxRepeatTarget& rTarget) const;

    virtual String  GetComment() const;

private:
    ScAddress       aPos;
    ScPostIt        aOldNote;
    ScPostIt        aNewNote;
};


class ScUndoDetective: public ScSimpleUndo
{
public:
                    TYPEINFO();
                    ScUndoDetective( ScDocShell* pNewDocShell,
                                    SdrUndoAction* pDraw, const ScDetOpData* pOperation,
                                    ScDetOpList* pUndoList = NULL );
    virtual         ~ScUndoDetective();

    virtual void    Undo();
    virtual void    Redo();
    virtual void    Repeat(SfxRepeatTarget& rTarget);
    virtual BOOL    CanRepeat(SfxRepeatTarget& rTarget) const;

    virtual String  GetComment() const;

private:
    BOOL            bIsDelete;
    ScDetOpList*    pOldList;
    USHORT          nAction;
    ScAddress       aPos;
    SdrUndoAction*  pDrawUndo;
};


class ScUndoRangeNames: public ScSimpleUndo
{
public:
                    TYPEINFO();
                    ScUndoRangeNames( ScDocShell* pNewDocShell,
                                        ScRangeName* pOld, ScRangeName* pNew );
    virtual         ~ScUndoRangeNames();

    virtual void    Undo();
    virtual void    Redo();
    virtual void    Repeat(SfxRepeatTarget& rTarget);
    virtual BOOL    CanRepeat(SfxRepeatTarget& rTarget) const;

    virtual String  GetComment() const;

private:
    ScRangeName*    pOldRanges;
    ScRangeName*    pNewRanges;

    void            DoChange( BOOL bUndo );
};



#endif

