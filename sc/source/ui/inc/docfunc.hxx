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

#ifndef INCLUDED_SC_SOURCE_UI_INC_DOCFUNC_HXX
#define INCLUDED_SC_SOURCE_UI_INC_DOCFUNC_HXX

#include <tools/solar.h>
#include <global.hxx>
#include <formula/grammar.hxx>
#include <tabbgcolor.hxx>

#include <memory>
#include <vector>
#include <map>

class ScEditEngineDefaulter;
class SdrUndoAction;
class ScAddress;
class ScDocShell;
class ScMarkData;
class ScPatternAttr;
class ScRange;
class ScRangeList;
class ScFormulaCell;
class ScTokenArray;
struct ScTabOpParam;
class ScTableProtection;
struct ScCellMergeOption;
class ScConditionalFormat;
class ScConditionalFormatList;
class ScUndoRemoveMerge;
class ScRangeName;

enum class TransliterationFlags;
enum class CreateNameFlags;
namespace sc {
    struct ColRowSpan;
}

class ScDocFunc
{
protected:
    ScDocShell&     rDocShell;

    bool            AdjustRowHeight( const ScRange& rRange, bool bPaint = true );
    void            CreateOneName( ScRangeName& rList,
                                    SCCOL nPosX, SCROW nPosY, SCTAB nTab,
                                    SCCOL nX1, SCROW nY1, SCCOL nX2, SCROW nY2,
                                    bool& rCancel, bool bApi );
    void            NotifyInputHandler( const ScAddress& rPos );

                    ScDocFunc( ScDocShell& rDocSh ): rDocShell(rDocSh) {}
public:
    virtual         ~ScDocFunc() {}

    void            NotifyDrawUndo(std::unique_ptr<SdrUndoAction>);

    // for grouping multiple operations into one with a new name
    void            EnterListAction(const char* pNameResId);
    void            EndListAction();

    bool            DetectiveAddPred(const ScAddress& rPos);
    bool            DetectiveDelPred(const ScAddress& rPos);
    bool            DetectiveAddSucc(const ScAddress& rPos);
    bool            DetectiveDelSucc(const ScAddress& rPos);
    bool            DetectiveAddError(const ScAddress& rPos);
    bool            DetectiveMarkInvalid(SCTAB nTab);
    bool            DetectiveDelAll(SCTAB nTab);
    bool            DetectiveRefresh(bool bAutomatic = false);
    void            DetectiveCollectAllPreds(const ScRangeList& rSrcRanges, ::std::vector<ScTokenRef>& rRefTokens);
    void            DetectiveCollectAllSuccs(const ScRangeList& rSrcRanges, ::std::vector<ScTokenRef>& rRefTokens);

    SC_DLLPUBLIC bool DeleteContents(
        const ScMarkData& rMark, InsertDeleteFlags nFlags, bool bRecord, bool bApi );

    bool DeleteCell(
        const ScAddress& rPos, const ScMarkData& rMark, InsertDeleteFlags nFlags, bool bRecord );

    bool            TransliterateText( const ScMarkData& rMark, TransliterationFlags nType,
                                               bool bApi );

    bool            SetNormalString( bool& o_rbNumFmtSet, const ScAddress& rPos, const OUString& rText, bool bApi );
    bool SetValueCell( const ScAddress& rPos, double fVal, bool bInteraction );
    void SetValueCells( const ScAddress& rPos, const std::vector<double>& aVals, bool bInteraction );
    bool SetStringCell( const ScAddress& rPos, const OUString& rStr, bool bInteraction );
    bool SetEditCell( const ScAddress& rPos, const EditTextObject& rStr, bool bInteraction );

    bool SetStringOrEditCell( const ScAddress& rPos, const OUString& rStr, bool bInteraction );

    /**
     * This method takes ownership of the formula cell instance. The caller
     * must not delete it after passing it to this call.
     */
    bool SetFormulaCell( const ScAddress& rPos, ScFormulaCell* pCell, bool bInteraction );
    void PutData( const ScAddress& rPos, ScEditEngineDefaulter& rEngine, bool bApi );
    bool SetCellText(
        const ScAddress& rPos, const OUString& rText, bool bInterpret, bool bEnglish, bool bApi,
        const formula::FormulaGrammar::Grammar eGrammar );

    bool            ShowNote( const ScAddress& rPos, bool bShow );

    void            SetNoteText( const ScAddress& rPos, const OUString& rNoteText, bool bApi );
    void            ReplaceNote( const ScAddress& rPos, const OUString& rNoteText, const OUString* pAuthor, const OUString* pDate, bool bApi );

    bool            ApplyAttributes( const ScMarkData& rMark, const ScPatternAttr& rPattern,
                                           bool bApi );
    bool            ApplyStyle( const ScMarkData& rMark, const OUString& rStyleName,
                                        bool bApi );

    bool            InsertCells( const ScRange& rRange,const ScMarkData* pTabMark,
                              InsCellCmd eCmd, bool bRecord, bool bApi, bool bPartOfPaste = false );

    bool            DeleteCells( const ScRange& rRange, const ScMarkData* pTabMark,
                              DelCellCmd eCmd, bool bApi );

    bool            MoveBlock( const ScRange& rSource, const ScAddress& rDestPos,
                                       bool bCut, bool bRecord, bool bPaint, bool bApi );

    SC_DLLPUBLIC bool InsertTable( SCTAB nTab, const OUString& rName, bool bRecord, bool bApi );
    bool            RenameTable( SCTAB nTab, const OUString& rName, bool bRecord, bool bApi );
    bool            DeleteTable( SCTAB nTab, bool bRecord );

    bool            SetTabBgColor( SCTAB nTab, const Color& rColor, bool bRecord, bool bApi );
    bool            SetTabBgColor( ScUndoTabColorInfo::List& rUndoTabColorList, bool bApi );

    void            SetTableVisible( SCTAB nTab, bool bVisible, bool bApi );

    bool            SetLayoutRTL( SCTAB nTab, bool bRTL );

    SC_DLLPUBLIC bool SetWidthOrHeight(
        bool bWidth, const std::vector<sc::ColRowSpan>& rRanges, SCTAB nTab,
        ScSizeMode eMode, sal_uInt16 nSizeTwips, bool bRecord, bool bApi );

    bool            InsertPageBreak( bool bColumn, const ScAddress& rPos,
                                             bool bRecord, bool bSetModified );
    bool            RemovePageBreak( bool bColumn, const ScAddress& rPos,
                                             bool bRecord, bool bSetModified );

    void            ProtectSheet( SCTAB nTab, const ScTableProtection& rProtect );

    bool            Protect( SCTAB nTab, const OUString& rPassword );
    bool            Unprotect( SCTAB nTab, const OUString& rPassword, bool bApi );

    void            ClearItems( const ScMarkData& rMark, const sal_uInt16* pWhich, bool bApi );
    bool            ChangeIndent( const ScMarkData& rMark, bool bIncrement, bool bApi );
    bool            AutoFormat( const ScRange& rRange, const ScMarkData* pTabMark,
                                        sal_uInt16 nFormatNo, bool bApi );

    SC_DLLPUBLIC bool
                    EnterMatrix( const ScRange& rRange, const ScMarkData* pTabMark,
                                         const ScTokenArray* pTokenArray,
                                         const OUString& rString, bool bApi, bool bEnglish,
                                         const OUString& rFormulaNmsp,
                                         const formula::FormulaGrammar::Grammar );

    bool            TabOp( const ScRange& rRange, const ScMarkData* pTabMark,
                                   const ScTabOpParam& rParam, bool bRecord, bool bApi );

    bool            FillSimple( const ScRange& rRange, const ScMarkData* pTabMark,
                                        FillDir eDir, bool bApi );
    bool            FillSeries( const ScRange& rRange, const ScMarkData* pTabMark,
                                        FillDir    eDir, FillCmd eCmd, FillDateCmd    eDateCmd,
                                        double fStart, double fStep, double fMax,
                                        bool bApi );

    // FillAuto: rRange is change from Source-Range to Dest-Range
    SC_DLLPUBLIC bool
                    FillAuto( ScRange& rRange, const ScMarkData* pTabMark, FillDir eDir, FillCmd eCmd, FillDateCmd  eDateCmd, sal_uLong nCount, double fStep, double fMax, bool bRecord, bool bApi );

    bool            FillAuto( ScRange& rRange, const ScMarkData* pTabMark,
                                      FillDir eDir, sal_uLong nCount, bool bApi );

    void            ResizeMatrix( const ScRange& rOldRange, const ScAddress& rNewEnd );

    bool            MergeCells( const ScCellMergeOption& rOption, bool bContents,
                                        bool bRecord, bool bApi, bool bEmptyMergedCells = false );
    bool            UnmergeCells( const ScRange& rRange, bool bRecord, ScUndoRemoveMerge* pUndoRemoveMerge );
    bool            UnmergeCells( const ScCellMergeOption& rOption, bool bRecord, ScUndoRemoveMerge* pUndoRemoveMerge );

    // takes ownership of pNewRanges, nTab = -1 for local range names
    void            SetNewRangeNames( std::unique_ptr<ScRangeName> pNewRanges, bool bModifyDoc, SCTAB nTab );
    void            ModifyRangeNames( const ScRangeName& rNewRanges, SCTAB nTab = -1 );
    /**
     * Modify all range names, global scope names as well as sheet local ones,
     * in one go.  Note that this method will <b>not</b> destroy the instances
     * passed as arguments (it creates copies); the caller is responsible for
     * destroying them.
     */
    void            ModifyAllRangeNames(const std::map<OUString, std::unique_ptr<ScRangeName>>& rRangeMap);

    bool            CreateNames( const ScRange& rRange, CreateNameFlags nFlags, bool bApi, SCTAB nTab = -1 ); // -1 for global range names
    bool            InsertNameList( const ScAddress& rStartPos, bool bApi );

    void            InsertAreaLink( const OUString& rFile, const OUString& rFilter,
                                            const OUString& rOptions, const OUString& rSource,
                                            const ScRange& rDestRange, sal_uLong nRefresh,
                                            bool bFitBlock, bool bApi );

    /**
     * @param nOldIndex If 0 don't delete an old format
     * @param pFormat if NULL only delete an old format
     */
    void ReplaceConditionalFormat( sal_uLong nOldIndex, std::unique_ptr<ScConditionalFormat> pFormat, SCTAB nTab, const ScRangeList& rRanges );

    /**
     * Sets or replaces the conditional format list of a table
     *
     * @param pList the new ScConditionalFormatList, method takes ownership
     * @param nTab the tab to which the conditional format list belongs
     */
    void SetConditionalFormatList( ScConditionalFormatList* pList, SCTAB nTab );

    void ConvertFormulaToValue( const ScRange& rRange, bool bInteraction );
};

class ScDocFuncDirect : public ScDocFunc
{
public:
            ScDocFuncDirect( ScDocShell& rDocSh ) : ScDocFunc( rDocSh ) {}
};

void VBA_DeleteModule( ScDocShell& rDocSh, const OUString& sModuleName );

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
