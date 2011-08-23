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

#ifndef SC_DOCFUNC_HXX
#define SC_DOCFUNC_HXX

#ifndef _LINK_HXX //autogen
#include <tools/link.hxx>
#endif

#ifndef SC_SCGLOB_HXX
#include "global.hxx"
#endif
namespace binfilter {

class SfxUndoAction;
class EditEngine;
class ScAddress;
class ScDocShell;
class ScMarkData;
class ScPatternAttr;
class ScRange;
class ScRangeName;
class ScBaseCell;
struct ScTabOpParam;


// ---------------------------------------------------------------------------

class ScDocFunc
{
private:
    ScDocShell&		rDocShell;

    BOOL			AdjustRowHeight( const ScRange& rRange, BOOL bPaint = TRUE );
    void			CreateOneName( ScRangeName& rList,
                                    USHORT nPosX, USHORT nPosY, USHORT nTab,
                                    USHORT nX1, USHORT nY1, USHORT nX2, USHORT nY2,
                                    BOOL& rCancel, BOOL bApi );
    void			NotifyInputHandler( const ScAddress& rPos );

public:
                    ScDocFunc( ScDocShell& rDocSh ): rDocShell(rDocSh) {}
                    ~ScDocFunc() {}

    DECL_LINK( NotifyDrawUndo, SfxUndoAction* );

    BOOL			DetectiveAddPred(const ScAddress& rPos);
    BOOL			DetectiveDelPred(const ScAddress& rPos);
    BOOL			DetectiveAddSucc(const ScAddress& rPos);
    BOOL			DetectiveDelSucc(const ScAddress& rPos);
    BOOL			DetectiveAddError(const ScAddress& rPos);
    BOOL			DetectiveMarkInvalid(USHORT nTab);
    BOOL			DetectiveDelAll(USHORT nTab);
    BOOL			DetectiveRefresh(BOOL bAutomatic = FALSE);

    BOOL			DeleteContents( const ScMarkData& rMark, USHORT nFlags,
                                    BOOL bRecord, BOOL bApi );


     BOOL			SetNormalString( const ScAddress& rPos, const String& rText, BOOL bApi );
    BOOL			PutCell( const ScAddress& rPos, ScBaseCell* pNewCell, BOOL bApi );
    BOOL			PutData( const ScAddress& rPos, EditEngine& rEngine,
                                BOOL bInterpret, BOOL bApi );
    BOOL			SetCellText( const ScAddress& rPos, const String& rText,
                                    BOOL bInterpret, BOOL bEnglish, BOOL bApi );

                    // creates a new cell for use with PutCell
    ScBaseCell*		InterpretEnglishString( const ScAddress& rPos, const String& rText );

    BOOL			SetNoteText( const ScAddress& rPos, const String& rText, BOOL bApi );

    BOOL			ApplyAttributes( const ScMarkData& rMark, const ScPatternAttr& rPattern,
                                    BOOL bRecord, BOOL bApi );
    BOOL			ApplyStyle( const ScMarkData& rMark, const String& rStyleName,
                                    BOOL bRecord, BOOL bApi );

    BOOL			InsertCells( const ScRange& rRange, InsCellCmd eCmd, BOOL bRecord, BOOL bApi,
                                    BOOL bPartOfPaste = FALSE );
    BOOL			DeleteCells( const ScRange& rRange, DelCellCmd eCmd, BOOL bRecord, BOOL bApi );

    BOOL			MoveBlock( const ScRange& rSource, const ScAddress& rDestPos,
                                BOOL bCut, BOOL bRecord, BOOL bPaint, BOOL bApi );

    BOOL			InsertTable( USHORT nTab, const String& rName, BOOL bRecord, BOOL bApi );
    BOOL			RenameTable( USHORT nTab, const String& rName, BOOL bRecord, BOOL bApi );
    BOOL			DeleteTable( USHORT nTab, BOOL bRecord, BOOL bApi );

    BOOL			SetTableVisible( USHORT nTab, BOOL bVisible, BOOL bApi );

    BOOL			SetWidthOrHeight( BOOL bWidth, USHORT nRangeCnt, USHORT* pRanges,
                                    USHORT nTab, ScSizeMode eMode, USHORT nSizeTwips,
                                    BOOL bRecord, BOOL bApi );

    BOOL			InsertPageBreak( BOOL bColumn, const ScAddress& rPos,
                                    BOOL bRecord, BOOL bSetModified, BOOL bApi );
    BOOL			RemovePageBreak( BOOL bColumn, const ScAddress& rPos,
                                    BOOL bRecord, BOOL bSetModified, BOOL bApi );

    BOOL			Protect( USHORT nTab, const String& rPassword, BOOL bApi );
    BOOL			Unprotect( USHORT nTab, const String& rPassword, BOOL bApi );

    BOOL			ClearItems( const ScMarkData& rMark, const USHORT* pWhich, BOOL bApi );
    BOOL			ChangeIndent( const ScMarkData& rMark, BOOL bIncrement, BOOL bApi );
    BOOL			AutoFormat( const ScRange& rRange, const ScMarkData* pTabMark,
                                    USHORT nFormatNo, BOOL bRecord, BOOL bApi );

    BOOL			EnterMatrix( const ScRange& rRange, const ScMarkData* pTabMark,
                                    const String& rString, BOOL bApi, BOOL bEnglish );

    BOOL			TabOp( const ScRange& rRange, const ScMarkData* pTabMark,
                            const ScTabOpParam& rParam, BOOL bRecord, BOOL bApi );

    BOOL			FillSeries( const ScRange& rRange, const ScMarkData* pTabMark,
                                FillDir	eDir, FillCmd eCmd, FillDateCmd	eDateCmd,
                                double fStart, double fStep, double fMax,
                                BOOL bRecord, BOOL bApi );
                    // FillAuto: rRange wird von Source-Range auf Dest-Range angepasst
    BOOL			FillAuto( ScRange& rRange, const ScMarkData* pTabMark,
                                FillDir eDir, USHORT nCount, BOOL bRecord, BOOL bApi );


    BOOL			MergeCells( const ScRange& rRange, BOOL bContents,
                                BOOL bRecord, BOOL bApi );
    BOOL			UnmergeCells( const ScRange& rRange, BOOL bRecord, BOOL bApi );


    BOOL			ModifyRangeNames( const ScRangeName& rNewRanges, BOOL bApi );

    BOOL			CreateNames( const ScRange& rRange, USHORT nFlags, BOOL bApi );
    BOOL			InsertNameList( const ScAddress& rStartPos, BOOL bApi );

    BOOL			InsertAreaLink( const String& rFile, const String& rFilter,
                                    const String& rOptions, const String& rSource,
                                    const ScRange& rDestRange, ULONG nRefresh,
                                    BOOL bFitBlock, BOOL bApi );
};



} //namespace binfilter
#endif

