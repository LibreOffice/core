/*************************************************************************
 *
 *  $RCSfile: undocell.hxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:45:02 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef SC_UNDOCELL_HXX
#define SC_UNDOCELL_HXX

#ifndef SC_UNDOBASE_HXX
#include "undobase.hxx"
#endif

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
                            USHORT nNewCol, USHORT nNewRow, USHORT nNewTab,
                            const ScPatternAttr* pOldPat, const ScPatternAttr* pNewPat,
                            const ScPatternAttr* pApplyPat, BOOL bAutomatic );
    virtual         ~ScUndoCursorAttr();

    virtual void    Undo();
    virtual void    Redo();
    virtual void    Repeat(SfxRepeatTarget& rTarget);
    virtual BOOL    CanRepeat(SfxRepeatTarget& rTarget) const;

    virtual String  GetComment() const;

private:
    USHORT          nCol;
    USHORT          nRow;
    USHORT          nTab;
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
                            USHORT nNewCol, USHORT nNewRow, USHORT nNewTab,
                            USHORT nNewCount, USHORT* pNewTabs,
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
    USHORT*         pTabs;
    ScBaseCell**    ppOldCells;
    BOOL*           pHasFormat;
    ULONG*          pOldFormats;
    EditTextObject* pNewEditData;
    ULONG           nEndChangeAction;
    USHORT          nCol;
    USHORT          nRow;
    USHORT          nTab;
    USHORT          nCount;             //  markierte Tabellen

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
                            USHORT nNewCol, USHORT nNewRow, USHORT nNewTab,
                            BOOL bNewColumn, BOOL bNewInsert );
    virtual         ~ScUndoPageBreak();

    virtual void    Undo();
    virtual void    Redo();
    virtual void    Repeat(SfxRepeatTarget& rTarget);
    virtual BOOL    CanRepeat(SfxRepeatTarget& rTarget) const;

    virtual String  GetComment() const;

private:
    USHORT          nCol;
    USHORT          nRow;
    USHORT          nTab;
    BOOL            bColumn;        // Spalten- oder Zeilenumbruch
    BOOL            bInsert;        // Einfuegen oder Loeschen

    void            DoChange( BOOL bInsert ) const;
};

class ScUndoPrintZoom: public ScSimpleUndo
{
public:
                    TYPEINFO();
                    ScUndoPrintZoom( ScDocShell* pNewDocShell, USHORT nT,
                                    USHORT nOS, USHORT nOP, USHORT nNS, USHORT nNP );
    virtual         ~ScUndoPrintZoom();

    virtual void    Undo();
    virtual void    Redo();
    virtual void    Repeat(SfxRepeatTarget& rTarget);
    virtual BOOL    CanRepeat(SfxRepeatTarget& rTarget) const;

    virtual String  GetComment() const;

private:
    USHORT          nTab;
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
                            USHORT nNewCol, USHORT nNewRow, USHORT nNewTab,
                            const String& rNewUndoStr, const EditTextObject* pUndoTObj,
                            const String& rNewRedoStr, const EditTextObject* pRedoTObj);
    virtual         ~ScUndoThesaurus();

    virtual void    Undo();
    virtual void    Redo();
    virtual void    Repeat(SfxRepeatTarget& rTarget);
    virtual BOOL    CanRepeat(SfxRepeatTarget& rTarget) const;

    virtual String  GetComment() const;

private:
    USHORT          nCol;
    USHORT          nRow;
    USHORT          nTab;
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

