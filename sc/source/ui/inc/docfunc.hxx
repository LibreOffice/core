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

#ifndef SC_DOCFUNC_HXX
#define SC_DOCFUNC_HXX

#include <tools/link.hxx>
#include "global.hxx"
#include "formula/grammar.hxx"
#include "tabbgcolor.hxx"
#include "token.hxx"
#include "rangenam.hxx"

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

// ---------------------------------------------------------------------------

class ScDocFunc
{
protected:
    ScDocShell&        rDocShell;

    sal_Bool        AdjustRowHeight( const ScRange& rRange, sal_Bool bPaint = true );
    void            CreateOneName( ScRangeName& rList,
                                    SCCOL nPosX, SCROW nPosY, SCTAB nTab,
                                    SCCOL nX1, SCROW nY1, SCCOL nX2, SCROW nY2,
                                    sal_Bool& rCancel, sal_Bool bApi );
    void            NotifyInputHandler( const ScAddress& rPos );

                        ScDocFunc( ScDocShell& rDocSh ): rDocShell(rDocSh) {}
public:
    virtual             ~ScDocFunc() {}

    DECL_LINK( NotifyDrawUndo, SdrUndoAction* );

    // for grouping multiple operations into one with a new name
    virtual void            EnterListAction( sal_uInt16 nNameResId );
    virtual void            EndListAction();

    virtual sal_Bool        DetectiveAddPred(const ScAddress& rPos);
    virtual sal_Bool        DetectiveDelPred(const ScAddress& rPos);
    virtual sal_Bool        DetectiveAddSucc(const ScAddress& rPos);
    virtual sal_Bool        DetectiveDelSucc(const ScAddress& rPos);
    virtual sal_Bool        DetectiveAddError(const ScAddress& rPos);
    virtual sal_Bool        DetectiveMarkInvalid(SCTAB nTab);
    virtual sal_Bool        DetectiveDelAll(SCTAB nTab);
    virtual sal_Bool        DetectiveRefresh(sal_Bool bAutomatic = false);
    virtual void            DetectiveCollectAllPreds(const ScRangeList& rSrcRanges, ::std::vector<ScTokenRef>& rRefTokens);
    virtual void            DetectiveCollectAllSuccs(const ScRangeList& rSrcRanges, ::std::vector<ScTokenRef>& rRefTokens);

    virtual sal_Bool        DeleteContents( const ScMarkData& rMark, sal_uInt16 nFlags,
                                            sal_Bool bRecord, sal_Bool bApi );

    virtual sal_Bool        TransliterateText( const ScMarkData& rMark, sal_Int32 nType,
                                               sal_Bool bRecord, sal_Bool bApi );

    virtual sal_Bool        SetNormalString( bool& o_rbNumFmtSet, const ScAddress& rPos, const String& rText, sal_Bool bApi );
    virtual bool SetValueCell( const ScAddress& rPos, double fVal, bool bInteraction );
    virtual bool SetStringCell( const ScAddress& rPos, const OUString& rStr, bool bInteraction );
    virtual bool SetEditCell( const ScAddress& rPos, const EditTextObject& rStr, bool bInteraction );

    bool SetStringOrEditCell( const ScAddress& rPos, const OUString& rStr, bool bInteraction );

    /**
     * This method takes ownership of the formula cell instance. The caller
     * must not delete it after passing it to this call.
     */
    virtual bool SetFormulaCell( const ScAddress& rPos, ScFormulaCell* pCell, bool bInteraction );
    virtual bool PutData( const ScAddress& rPos, ScEditEngineDefaulter& rEngine, bool bApi );
    virtual bool SetCellText(
        const ScAddress& rPos, const OUString& rText, bool bInterpret, bool bEnglish, bool bApi,
        const formula::FormulaGrammar::Grammar eGrammar );

    virtual bool            ShowNote( const ScAddress& rPos, bool bShow = true );
    inline bool             HideNote( const ScAddress& rPos ) { return ShowNote( rPos, false ); }

    virtual bool            SetNoteText( const ScAddress& rPos, const String& rNoteText, sal_Bool bApi );
    virtual bool            ReplaceNote( const ScAddress& rPos, const OUString& rNoteText, const OUString* pAuthor, const OUString* pDate, sal_Bool bApi );

    virtual sal_Bool        ApplyAttributes( const ScMarkData& rMark, const ScPatternAttr& rPattern,
                                             sal_Bool bRecord, sal_Bool bApi );
    virtual sal_Bool        ApplyStyle( const ScMarkData& rMark, const String& rStyleName,
                                        sal_Bool bRecord, sal_Bool bApi );

    virtual bool InsertCells( const ScRange& rRange,const ScMarkData* pTabMark,
                              InsCellCmd eCmd, bool bRecord, bool bApi, bool bPartOfPaste = false );

    virtual bool DeleteCells( const ScRange& rRange, const ScMarkData* pTabMark,
                              DelCellCmd eCmd, bool bRecord, bool bApi );

    virtual sal_Bool        MoveBlock( const ScRange& rSource, const ScAddress& rDestPos,
                                       sal_Bool bCut, sal_Bool bRecord, sal_Bool bPaint, sal_Bool bApi );

    virtual sal_Bool        InsertTable( SCTAB nTab, const String& rName, sal_Bool bRecord, sal_Bool bApi );
    virtual sal_Bool        RenameTable( SCTAB nTab, const String& rName, sal_Bool bRecord, sal_Bool bApi );
    virtual sal_Bool        DeleteTable( SCTAB nTab, sal_Bool bRecord, sal_Bool bApi );

    virtual bool            SetTabBgColor( SCTAB nTab, const Color& rColor, bool bRecord, bool bApi );
    virtual bool            SetTabBgColor( ScUndoTabColorInfo::List& rUndoTabColorList, bool bRecord, bool bApi );

    virtual sal_Bool        SetTableVisible( SCTAB nTab, bool bVisible, sal_Bool bApi );

    virtual sal_Bool        SetLayoutRTL( SCTAB nTab, bool bRTL, sal_Bool bApi );

    virtual SC_DLLPUBLIC sal_Bool
                            SetWidthOrHeight( sal_Bool bWidth, SCCOLROW nRangeCnt, SCCOLROW* pRanges,
                                              SCTAB nTab, ScSizeMode eMode, sal_uInt16 nSizeTwips,
                                              sal_Bool bRecord, sal_Bool bApi );

    virtual sal_Bool        InsertPageBreak( sal_Bool bColumn, const ScAddress& rPos,
                                             sal_Bool bRecord, sal_Bool bSetModified, sal_Bool bApi );
    virtual sal_Bool        RemovePageBreak( sal_Bool bColumn, const ScAddress& rPos,
                                             sal_Bool bRecord, sal_Bool bSetModified, sal_Bool bApi );

    virtual void            ProtectSheet( SCTAB nTab, const ScTableProtection& rProtect );

    virtual sal_Bool        Protect( SCTAB nTab, const String& rPassword, sal_Bool bApi );
    virtual sal_Bool        Unprotect( SCTAB nTab, const String& rPassword, sal_Bool bApi );

    virtual sal_Bool        ClearItems( const ScMarkData& rMark, const sal_uInt16* pWhich, sal_Bool bApi );
    virtual sal_Bool        ChangeIndent( const ScMarkData& rMark, sal_Bool bIncrement, sal_Bool bApi );
    virtual bool            AutoFormat( const ScRange& rRange, const ScMarkData* pTabMark,
                                        sal_uInt16 nFormatNo, bool bRecord, bool bApi );

    virtual sal_Bool        EnterMatrix( const ScRange& rRange, const ScMarkData* pTabMark,
                                         const ScTokenArray* pTokenArray,
                                         const String& rString, sal_Bool bApi, sal_Bool bEnglish,
                                         const String& rFormulaNmsp,
                                         const formula::FormulaGrammar::Grammar );

    virtual sal_Bool        TabOp( const ScRange& rRange, const ScMarkData* pTabMark,
                                   const ScTabOpParam& rParam, sal_Bool bRecord, sal_Bool bApi );

    virtual bool            FillSimple( const ScRange& rRange, const ScMarkData* pTabMark,
                                        FillDir eDir, bool bRecord, bool bApi );
    virtual sal_Bool        FillSeries( const ScRange& rRange, const ScMarkData* pTabMark,
                                        FillDir    eDir, FillCmd eCmd, FillDateCmd    eDateCmd,
                                        double fStart, double fStep, double fMax,
                                        sal_Bool bRecord, sal_Bool bApi );

    // FillAuto: rRange wird von Source-Range auf Dest-Range angepasst
    SC_DLLPUBLIC virtual sal_Bool   FillAuto( ScRange& rRange, const ScMarkData* pTabMark, FillDir eDir, FillCmd eCmd, FillDateCmd  eDateCmd, sal_uLong nCount, double fStep, double fMax, sal_Bool bRecord, sal_Bool bApi );

    virtual sal_Bool        FillAuto( ScRange& rRange, const ScMarkData* pTabMark,
                                      FillDir eDir, sal_uLong nCount, sal_Bool bRecord, sal_Bool bApi );

    virtual sal_Bool        ResizeMatrix( const ScRange& rOldRange, const ScAddress& rNewEnd, sal_Bool bApi );

    virtual sal_Bool        MergeCells( const ScCellMergeOption& rOption, sal_Bool bContents,
                                        sal_Bool bRecord, sal_Bool bApi );
    sal_Bool        UnmergeCells( const ScRange& rRange, sal_Bool bRecord );
    bool            UnmergeCells( const ScCellMergeOption& rOption, sal_Bool bRecord );

    virtual bool            SetNewRangeNames( ScRangeName* pNewRanges, bool bModifyDoc = true, SCTAB nTab = -1 );     // takes ownership of pNewRanges //nTab = -1 for local range names
    virtual bool            ModifyRangeNames( const ScRangeName& rNewRanges, SCTAB nTab = -1 );
    /**
     * Modify all range names, global scope names as well as sheet local ones,
     * in one go.  Note that this method will <b>not</b> destroy the instances
     * passed as arguments (it creates copies); the caller is responsible for
     * destroying them.
     */
    virtual void            ModifyAllRangeNames( const boost::ptr_map<OUString, ScRangeName>& rRangeMap );

    virtual sal_Bool        CreateNames( const ScRange& rRange, sal_uInt16 nFlags, sal_Bool bApi, SCTAB nTab = -1 ); // -1 for global range names
    virtual sal_Bool        InsertNameList( const ScAddress& rStartPos, sal_Bool bApi );

    sal_Bool                InsertAreaLink( const String& rFile, const String& rFilter,
                                            const String& rOptions, const OUString& rSource,
                                            const ScRange& rDestRange, sal_uLong nRefresh,
                                            sal_Bool bFitBlock, sal_Bool bApi );

    /**
     * @param nOldIndex If 0 don't delete an old format
     * @param pFormat if NULL only delete an old format
     */
    virtual void ReplaceConditionalFormat( sal_uLong nOldIndex, ScConditionalFormat* pFormat, SCTAB nTab, const ScRangeList& rRanges );

    /**
     * Sets or replaces the conditional format list of a table
     *
     * @param pList the new ScConditionalFormatList, method takes ownership
     * @param nTab the tab to which the conditional format list belongs
     */
    virtual void SetConditionalFormatList( ScConditionalFormatList* pList, SCTAB nTab );
};

class ScDocFuncDirect : public ScDocFunc
{
public:
             ScDocFuncDirect( ScDocShell& rDocSh ) : ScDocFunc( rDocSh ) {}
    virtual ~ScDocFuncDirect() {}
};


#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
