/*************************************************************************
 *
 *  $RCSfile: docfunc.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: nn $ $Date: 2001-03-26 19:21:58 $
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

#ifndef SC_DOCFUNC_HXX
#define SC_DOCFUNC_HXX

#ifndef _LINK_HXX //autogen
#include <tools/link.hxx>
#endif

#ifndef SC_SCGLOB_HXX
#include "global.hxx"
#endif

class EditEngine;
class SfxUndoAction;
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
    ScDocShell&     rDocShell;

    BOOL            AdjustRowHeight( const ScRange& rRange, BOOL bPaint = TRUE );
    void            CreateOneName( ScRangeName& rList,
                                    USHORT nPosX, USHORT nPosY, USHORT nTab,
                                    USHORT nX1, USHORT nY1, USHORT nX2, USHORT nY2,
                                    BOOL& rCancel, BOOL bApi );

public:
                    ScDocFunc( ScDocShell& rDocSh ): rDocShell(rDocSh) {}
                    ~ScDocFunc() {}

    DECL_LINK( NotifyDrawUndo, SfxUndoAction* );

    BOOL            DetectiveAddPred(const ScAddress& rPos);
    BOOL            DetectiveDelPred(const ScAddress& rPos);
    BOOL            DetectiveAddSucc(const ScAddress& rPos);
    BOOL            DetectiveDelSucc(const ScAddress& rPos);
    BOOL            DetectiveAddError(const ScAddress& rPos);
    BOOL            DetectiveMarkInvalid(USHORT nTab);
    BOOL            DetectiveDelAll(USHORT nTab);
    BOOL            DetectiveRefresh(BOOL bAutomatic = FALSE);

    BOOL            DeleteContents( const ScMarkData& rMark, USHORT nFlags,
                                    BOOL bRecord, BOOL bApi );

    BOOL            TransliterateText( const ScMarkData& rMark, sal_Int32 nType,
                                    BOOL bRecord, BOOL bApi );

    BOOL            SetNormalString( const ScAddress& rPos, const String& rText, BOOL bApi );
    BOOL            PutCell( const ScAddress& rPos, ScBaseCell* pNewCell, BOOL bApi );
    BOOL            PutData( const ScAddress& rPos, EditEngine& rEngine,
                                BOOL bInterpret, BOOL bApi );
    BOOL            SetCellText( const ScAddress& rPos, const String& rText,
                                    BOOL bInterpret, BOOL bEnglish, BOOL bApi );

    BOOL            SetNoteText( const ScAddress& rPos, const String& rText, BOOL bApi );

    BOOL            ApplyAttributes( const ScMarkData& rMark, const ScPatternAttr& rPattern,
                                    BOOL bRecord, BOOL bApi );
    BOOL            ApplyStyle( const ScMarkData& rMark, const String& rStyleName,
                                    BOOL bRecord, BOOL bApi );

    BOOL            InsertCells( const ScRange& rRange, InsCellCmd eCmd, BOOL bRecord, BOOL bApi );
    BOOL            DeleteCells( const ScRange& rRange, DelCellCmd eCmd, BOOL bRecord, BOOL bApi );

    BOOL            MoveBlock( const ScRange& rSource, const ScAddress& rDestPos,
                                BOOL bCut, BOOL bRecord, BOOL bPaint, BOOL bApi );

    BOOL            InsertTable( USHORT nTab, const String& rName, BOOL bRecord, BOOL bApi );
    BOOL            RenameTable( USHORT nTab, const String& rName, BOOL bRecord, BOOL bApi );
    BOOL            DeleteTable( USHORT nTab, BOOL bRecord, BOOL bApi );

    BOOL            SetTableVisible( USHORT nTab, BOOL bVisible, BOOL bApi );

    BOOL            SetWidthOrHeight( BOOL bWidth, USHORT nRangeCnt, USHORT* pRanges,
                                    USHORT nTab, ScSizeMode eMode, USHORT nSizeTwips,
                                    BOOL bRecord, BOOL bApi );

    BOOL            InsertPageBreak( BOOL bColumn, const ScAddress& rPos,
                                    BOOL bRecord, BOOL bSetModified, BOOL bApi );
    BOOL            RemovePageBreak( BOOL bColumn, const ScAddress& rPos,
                                    BOOL bRecord, BOOL bSetModified, BOOL bApi );

    BOOL            Protect( USHORT nTab, const String& rPassword, BOOL bApi );
    BOOL            Unprotect( USHORT nTab, const String& rPassword, BOOL bApi );

    BOOL            ClearItems( const ScMarkData& rMark, const USHORT* pWhich, BOOL bApi );
    BOOL            ChangeIndent( const ScMarkData& rMark, BOOL bIncrement, BOOL bApi );
    BOOL            AutoFormat( const ScRange& rRange, const ScMarkData* pTabMark,
                                    USHORT nFormatNo, BOOL bRecord, BOOL bApi );

    BOOL            EnterMatrix( const ScRange& rRange, const ScMarkData* pTabMark,
                                    const String& rString, BOOL bApi );

    BOOL            TabOp( const ScRange& rRange, const ScMarkData* pTabMark,
                            const ScTabOpParam& rParam, BOOL bRecord, BOOL bApi );

    BOOL            FillSimple( const ScRange& rRange, const ScMarkData* pTabMark,
                                FillDir eDir, BOOL bRecord, BOOL bApi );
    BOOL            FillSeries( const ScRange& rRange, const ScMarkData* pTabMark,
                                FillDir eDir, FillCmd eCmd, FillDateCmd eDateCmd,
                                double fStart, double fStep, double fMax,
                                BOOL bRecord, BOOL bApi );
                    // FillAuto: rRange wird von Source-Range auf Dest-Range angepasst
    BOOL            FillAuto( ScRange& rRange, const ScMarkData* pTabMark,
                                FillDir eDir, USHORT nCount, BOOL bRecord, BOOL bApi );

    BOOL            ResizeMatrix( const ScRange& rOldRange, const ScAddress& rNewEnd, BOOL bApi );

    BOOL            MergeCells( const ScRange& rRange, BOOL bContents,
                                BOOL bRecord, BOOL bApi );
    BOOL            UnmergeCells( const ScRange& rRange, BOOL bRecord, BOOL bApi );

    BOOL            SetNote( const ScAddress& rPos, const ScPostIt& rNote, BOOL bApi );

    BOOL            ModifyRangeNames( const ScRangeName& rNewRanges, BOOL bApi );

    BOOL            CreateNames( const ScRange& rRange, USHORT nFlags, BOOL bApi );
    BOOL            InsertNameList( const ScAddress& rStartPos, BOOL bApi );

    BOOL            InsertAreaLink( const String& rFile, const String& rFilter,
                                    const String& rOptions, const String& rSource,
                                    const ScRange& rDestRange, BOOL bFitBlock, BOOL bApi );
};



#endif

