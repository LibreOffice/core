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

#ifndef INCLUDED_SC_SOURCE_UI_INC_UNDOCELL_HXX
#define INCLUDED_SC_SOURCE_UI_INC_UNDOCELL_HXX

#include "undobase.hxx"
#include "postit.hxx"
#include "cellvalue.hxx"
#include <cellvalues.hxx>

#include <memory>

class ScDocShell;
class ScPatternAttr;
class EditTextObject;
class SdrUndoAction;
class ScDetOpList;
class ScDetOpData;
class ScRangeName;
class ScDocument;

namespace sc {

class CellValues;

}

class ScUndoCursorAttr: public ScSimpleUndo
{
public:
                    ScUndoCursorAttr( ScDocShell* pNewDocShell,
                            SCCOL nNewCol, SCROW nNewRow, SCTAB nNewTab,
                            const ScPatternAttr* pOldPat, const ScPatternAttr* pNewPat,
                            const ScPatternAttr* pApplyPat, bool bAutomatic );
    virtual         ~ScUndoCursorAttr();

    virtual void    Undo() override;
    virtual void    Redo() override;
    virtual void    Repeat(SfxRepeatTarget& rTarget) override;
    virtual bool    CanRepeat(SfxRepeatTarget& rTarget) const override;

    virtual OUString GetComment() const override;

    /** once the objects are passed to this class, their life-cycle is
        managed by this class; the calling function must pass new'ed
        objects to this method. */
    void            SetEditData( EditTextObject* pOld, EditTextObject* pNew );

private:
    SCCOL           nCol;
    SCROW           nRow;
    SCTAB           nTab;
    ScPatternAttr*  pOldPattern;
    ScPatternAttr*  pNewPattern;
    ScPatternAttr*  pApplyPattern;
    std::shared_ptr<EditTextObject> pOldEditData;
    std::shared_ptr<EditTextObject> pNewEditData;
    bool            bIsAutomatic;

    void            DoChange( const ScPatternAttr* pWhichPattern, const std::shared_ptr<EditTextObject>& pEditData ) const;
};

class ScUndoEnterData: public ScSimpleUndo
{
public:

    struct Value
    {
        SCTAB mnTab;
        bool mbHasFormat;
        sal_uInt32 mnFormat;
        ScCellValue maCell;

        Value();
    };

    typedef std::vector<Value> ValuesType;

    ScUndoEnterData(
        ScDocShell* pNewDocShell, const ScAddress& rPos,
        ValuesType& rOldValues, const OUString& rNewStr, EditTextObject* pObj = nullptr );

    virtual ~ScUndoEnterData();

    virtual void    Undo() override;
    virtual void    Redo() override;
    virtual void    Repeat(SfxRepeatTarget& rTarget) override;
    virtual bool    CanRepeat(SfxRepeatTarget& rTarget) const override;

    virtual OUString GetComment() const override;

private:
    ValuesType maOldValues;

    OUString  maNewString;
    std::unique_ptr<EditTextObject> mpNewEditData;
    sal_uLong mnEndChangeAction;
    ScAddress maPos;

    void            DoChange() const;
    void            SetChangeTrack();
};

class ScUndoEnterValue: public ScSimpleUndo
{
public:
    ScUndoEnterValue(
        ScDocShell* pNewDocShell, const ScAddress& rNewPos,
        const ScCellValue& rUndoCell, double nVal );

    virtual         ~ScUndoEnterValue();

    virtual void    Undo() override;
    virtual void    Redo() override;
    virtual void    Repeat(SfxRepeatTarget& rTarget) override;
    virtual bool    CanRepeat(SfxRepeatTarget& rTarget) const override;

    virtual OUString GetComment() const override;

private:
    ScAddress       aPos;
    ScCellValue maOldCell;
    double          nValue;
    sal_uLong       nEndChangeAction;

    void            SetChangeTrack();
};

class ScUndoSetCell : public ScSimpleUndo
{
public:
    ScUndoSetCell( ScDocShell* pDocSh, const ScAddress& rPos, const ScCellValue& rOldVal, const ScCellValue& rNewVal );

    virtual ~ScUndoSetCell();

    virtual void Undo() override;
    virtual void Redo() override;
    virtual void Repeat( SfxRepeatTarget& rTarget ) override;
    virtual bool CanRepeat( SfxRepeatTarget& rTarget ) const override;
    virtual OUString GetComment() const override;

private:
    void SetChangeTrack();
    void SetValue( const ScCellValue& rVal );

private:
    ScAddress maPos;
    ScCellValue maOldValue;
    ScCellValue maNewValue;
    sal_uLong mnEndChangeAction;
};

class ScUndoPageBreak: public ScSimpleUndo
{
public:
                    ScUndoPageBreak( ScDocShell* pNewDocShell,
                            SCCOL nNewCol, SCROW nNewRow, SCTAB nNewTab,
                            bool bNewColumn, bool bNewInsert );
    virtual         ~ScUndoPageBreak();

    virtual void    Undo() override;
    virtual void    Redo() override;
    virtual void    Repeat(SfxRepeatTarget& rTarget) override;
    virtual bool    CanRepeat(SfxRepeatTarget& rTarget) const override;

    virtual OUString GetComment() const override;

private:
    SCCOL           nCol;
    SCROW           nRow;
    SCTAB           nTab;
    bool            bColumn;        // Column or row break
    bool            bInsert;        // Insert or Delete

    void            DoChange( bool bInsert ) const;
};

class ScUndoPrintZoom: public ScSimpleUndo
{
public:
                    ScUndoPrintZoom( ScDocShell* pNewDocShell, SCTAB nT,
                                    sal_uInt16 nOS, sal_uInt16 nOP, sal_uInt16 nNS, sal_uInt16 nNP );
    virtual         ~ScUndoPrintZoom();

    virtual void    Undo() override;
    virtual void    Redo() override;
    virtual void    Repeat(SfxRepeatTarget& rTarget) override;
    virtual bool    CanRepeat(SfxRepeatTarget& rTarget) const override;

    virtual OUString GetComment() const override;

private:
    SCTAB           nTab;
    sal_uInt16      nOldScale;
    sal_uInt16      nOldPages;
    sal_uInt16      nNewScale;
    sal_uInt16      nNewPages;

    void            DoChange( bool bUndo );
};

class ScUndoThesaurus: public ScSimpleUndo
{
public:
    ScUndoThesaurus( ScDocShell* pNewDocShell,
                     SCCOL nNewCol, SCROW nNewRow, SCTAB nNewTab,
                     const ScCellValue& rOldText, const ScCellValue& rNewText );
    virtual         ~ScUndoThesaurus();

    virtual void    Undo() override;
    virtual void    Redo() override;
    virtual void    Repeat(SfxRepeatTarget& rTarget) override;
    virtual bool    CanRepeat(SfxRepeatTarget& rTarget) const override;

    virtual OUString GetComment() const override;

private:
    SCCOL           nCol;
    SCROW           nRow;
    SCTAB           nTab;
    sal_uLong       nEndChangeAction;

    ScCellValue maOldText;
    ScCellValue maNewText;

    void DoChange( bool bUndo, const ScCellValue& rText );
    void SetChangeTrack( const ScCellValue& rOldCell );
};

/** Undo action for inserting, removing, and replacing a cell note. */
class ScUndoReplaceNote : public ScSimpleUndo
{
public:

    /** Constructs an undo action for inserting or removing a cell note. */
                    ScUndoReplaceNote(
                        ScDocShell& rDocShell,
                        const ScAddress& rPos,
                        const ScNoteData& rNoteData,
                        bool bInsert,
                        SdrUndoAction* pDrawUndo );

    /** Constructs an undo action for replacing a cell note with another. */
                    ScUndoReplaceNote(
                        ScDocShell& rDocShell,
                        const ScAddress& rPos,
                        const ScNoteData& rOldData,
                        const ScNoteData& rNewData,
                        SdrUndoAction* pDrawUndo );

    virtual         ~ScUndoReplaceNote();

    virtual void    Undo() override;
    virtual void    Redo() override;
    virtual void    Repeat( SfxRepeatTarget& rTarget ) override;
    virtual bool    CanRepeat( SfxRepeatTarget& rTarget ) const override;

    virtual OUString GetComment() const override;

private:
    void            DoInsertNote( const ScNoteData& rNoteData );
    void            DoRemoveNote( const ScNoteData& rNoteData );

private:
    ScAddress       maPos;
    ScNoteData      maOldData;
    ScNoteData      maNewData;
    SdrUndoAction*  mpDrawUndo;
};

/** Undo action for showing or hiding a cell note caption. */
class ScUndoShowHideNote : public ScSimpleUndo
{
public:
                    ScUndoShowHideNote( ScDocShell& rDocShell, const ScAddress& rPos, bool bShow );
    virtual         ~ScUndoShowHideNote();

    virtual void    Undo() override;
    virtual void    Redo() override;
    virtual void    Repeat( SfxRepeatTarget& rTarget ) override;
    virtual bool    CanRepeat( SfxRepeatTarget& rTarget ) const override;

    virtual OUString GetComment() const override;

private:
    ScAddress       maPos;
    bool            mbShown;
};

class ScUndoDetective: public ScSimpleUndo
{
public:
                    ScUndoDetective( ScDocShell* pNewDocShell,
                                    SdrUndoAction* pDraw, const ScDetOpData* pOperation,
                                    ScDetOpList* pUndoList = nullptr );
    virtual         ~ScUndoDetective();

    virtual void    Undo() override;
    virtual void    Redo() override;
    virtual void    Repeat(SfxRepeatTarget& rTarget) override;
    virtual bool    CanRepeat(SfxRepeatTarget& rTarget) const override;

    virtual OUString GetComment() const override;

private:
    bool            bIsDelete;
    ScDetOpList*    pOldList;
    sal_uInt16      nAction;
    ScAddress       aPos;
    SdrUndoAction*  pDrawUndo;
};

class ScUndoRangeNames: public ScSimpleUndo
{
public:
                    //use nTab = -1 for global range names
                    ScUndoRangeNames( ScDocShell* pNewDocShell,
                                        ScRangeName* pOld, ScRangeName* pNew , SCTAB nTab = -1);
    virtual         ~ScUndoRangeNames();

    virtual void    Undo() override;
    virtual void    Redo() override;
    virtual void    Repeat(SfxRepeatTarget& rTarget) override;
    virtual bool    CanRepeat(SfxRepeatTarget& rTarget) const override;

    virtual OUString GetComment() const override;

private:
    ScRangeName*    pOldRanges;
    ScRangeName*    pNewRanges;
    SCTAB           mnTab;

    void            DoChange( bool bUndo );
};

namespace sc {

class UndoSetCells : public ScSimpleUndo
{
    ScAddress maTopPos;
    CellValues maOldValues;
    CellValues maNewValues;

    void DoChange( const CellValues& rValues );

public:
    UndoSetCells( ScDocShell* pDocSh, const ScAddress& rTopPos );
    virtual ~UndoSetCells();

    virtual void Undo() override;
    virtual void Redo() override;

    virtual bool CanRepeat( SfxRepeatTarget& ) const override;
    virtual OUString GetComment() const override;

    CellValues& GetOldValues() { return maOldValues;}
    void SetNewValues( const std::vector<double>& rVals );
};

} // namespace sc

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
