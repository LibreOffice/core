/*************************************************************************
 *
 *  $RCSfile: undoblk.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: dr $ $Date: 2001-04-05 10:49:47 $
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

#ifndef SC_UNDOBLK_HXX
#define SC_UNDOBLK_HXX

#ifndef SC_UNDOBASE_HXX
#include "undobase.hxx"
#endif
#ifndef SC_MARKDATA_HXX
#include "markdata.hxx"
#endif

class ScDocShell;
class ScDocument;
class ScOutlineTable;
class ScRangeName;
class ScRangeList;
class ScDBCollection;
class ScPatternAttr;
class SvxBoxItem;
class SvxBoxInfoItem;
class SvxSearchItem;
class SdrUndoAction;

//----------------------------------------------------------------------------

class ScUndoInsertCells: public ScMoveUndo
{
public:
                    TYPEINFO();
                    ScUndoInsertCells( ScDocShell* pNewDocShell,
                                       const ScRange& rRange, InsCellCmd eNewCmd,
                                       ScDocument* pUndoDocument, ScRefUndoData* pRefData );
    virtual         ~ScUndoInsertCells();

    virtual void    Undo();
    virtual void    Redo();
    virtual void    Repeat( SfxRepeatTarget& rTarget );
    virtual BOOL    CanRepeat( SfxRepeatTarget& rTarget ) const;

    virtual String  GetComment() const;

private:
    ScRange         aEffRange;
    ULONG           nEndChangeAction;
    InsCellCmd      eCmd;

    void            DoChange ( const BOOL bUndo );
    void            SetChangeTrack();
};


class ScUndoDeleteCells: public ScMoveUndo
{
public:
                    TYPEINFO();
                    ScUndoDeleteCells( ScDocShell* pNewDocShell,
                                       const ScRange& rRange, DelCellCmd eNewCmd,
                                       ScDocument* pUndoDocument, ScRefUndoData* pRefData );
    virtual         ~ScUndoDeleteCells();

    virtual void    Undo();
    virtual void    Redo();
    virtual void    Repeat(SfxRepeatTarget& rTarget);
    virtual BOOL    CanRepeat(SfxRepeatTarget& rTarget) const;

    virtual String  GetComment() const;

private:
    ScRange         aEffRange;
    ULONG           nStartChangeAction;
    ULONG           nEndChangeAction;
    DelCellCmd      eCmd;

    void            DoChange ( const BOOL bUndo );
    void            SetChangeTrack();
};


class ScUndoDeleteMulti: public ScMoveUndo
{
public:
                    TYPEINFO();
                    ScUndoDeleteMulti( ScDocShell* pNewDocShell,
                                       BOOL bNewRows, USHORT nNewTab,
                                       const USHORT* pRng, USHORT nRngCnt,
                                       ScDocument* pUndoDocument, ScRefUndoData* pRefData );
    virtual         ~ScUndoDeleteMulti();

    virtual void    Undo();
    virtual void    Redo();
    virtual void    Repeat(SfxRepeatTarget& rTarget);
    virtual BOOL    CanRepeat(SfxRepeatTarget& rTarget) const;

    virtual String  GetComment() const;

private:
    BOOL            bRows;
    USHORT          nTab;
    USHORT*         pRanges;
    USHORT          nRangeCnt;
    ULONG           nStartChangeAction;
    ULONG           nEndChangeAction;

    void            DoChange() const;
    void            SetChangeTrack();
};


class ScUndoCut: public ScBlockUndo
{
public:
                    TYPEINFO();
                    ScUndoCut( ScDocShell* pNewDocShell,
                               ScRange aRange,      // incl. Merge
                               ScAddress aOldEnd,   // Ende ohne Merge
                               ScDocument* pNewUndoDoc );
    virtual         ~ScUndoCut();

    virtual void    Undo();
    virtual void    Redo();
    virtual void    Repeat(SfxRepeatTarget& rTarget);
    virtual BOOL    CanRepeat(SfxRepeatTarget& rTarget) const;

    virtual String  GetComment() const;

private:
    ScDocument*     pUndoDoc;
    ScRange         aExtendedRange;
    ULONG           nStartChangeAction;
    ULONG           nEndChangeAction;

    void            DoChange( const BOOL bUndo );
    void            SetChangeTrack();
};


class ScUndoPaste: public ScSimpleUndo
{
public:
                    TYPEINFO();
                    ScUndoPaste( ScDocShell* pNewDocShell,
                                 USHORT nStartX, USHORT nStartY, USHORT nStartZ,
                                 USHORT nEndX, USHORT nEndY, USHORT nEndZ,
                                 const ScMarkData& rMark,
                                 ScDocument* pNewUndoDoc, ScDocument* pNewRedoDoc,
                                 USHORT nNewFlags,
                                 ScRangeName* pNewUndoRange, ScRangeName* pNewRedoRange,
                                 ScDBCollection* pNewUndoDB, ScDBCollection* pNewRedoDB,
                                 BOOL bRedoIsFilled = TRUE );
    virtual         ~ScUndoPaste();

    virtual void    Undo();
    virtual void    Redo();
    virtual void    Repeat(SfxRepeatTarget& rTarget);
    virtual BOOL    CanRepeat(SfxRepeatTarget& rTarget) const;

    virtual String  GetComment() const;

private:
    ScRange         aRange;
    ScMarkData      aMarkData;
    ScDocument*     pUndoDoc;
    ScDocument*     pRedoDoc;
    USHORT          nFlags;
    ScRangeName*    pUndoRange;
    ScRangeName*    pRedoRange;
    ScDBCollection* pUndoDB;
    ScDBCollection* pRedoDB;
    ULONG           nStartChangeAction;
    ULONG           nEndChangeAction;
    BOOL            bRedoFilled;

    void            DoChange( const BOOL bUndo );
    void            SetChangeTrack();
};


class ScUndoDragDrop: public ScMoveUndo
{
public:
                    TYPEINFO();
                    ScUndoDragDrop( ScDocShell* pNewDocShell,
                                    const ScRange& rRange, ScAddress aNewDestPos, BOOL bNewCut,
                                    ScDocument* pUndoDocument, ScRefUndoData* pRefData,
                                    BOOL bScenario );
    virtual         ~ScUndoDragDrop();

    virtual void    Undo();
    virtual void    Redo();
    virtual void    Repeat(SfxRepeatTarget& rTarget);
    virtual BOOL    CanRepeat(SfxRepeatTarget& rTarget) const;

    virtual String  GetComment() const;

private:
    ScRange         aSrcRange;
    ScRange         aDestRange;
    ULONG           nStartChangeAction;
    ULONG           nEndChangeAction;
    BOOL            bCut;
    BOOL            bKeepScenarioFlags;

    void            PaintArea( ScRange aRange ) const;
    void            DoUndo( ScRange aRange ) const;

    void            SetChangeTrack();
};


class ScUndoDeleteContents: public ScSimpleUndo
{
public:
                    TYPEINFO();
                    ScUndoDeleteContents( ScDocShell* pNewDocShell,
                                          const ScMarkData& rMark,
                                          const ScRange& rRange,
                                          ScDocument* pNewUndoDoc, BOOL bNewMulti,
                                          USHORT nNewFlags, BOOL bObjects );
    virtual         ~ScUndoDeleteContents();

    virtual void    Undo();
    virtual void    Redo();
    virtual void    Repeat(SfxRepeatTarget& rTarget);
    virtual BOOL    CanRepeat(SfxRepeatTarget& rTarget) const;

    virtual String  GetComment() const;

private:
    ScRange         aRange;
    ScMarkData      aMarkData;
    ScDocument*     pUndoDoc;       // Blockmarkierung und geloeschte Daten
    SdrUndoAction*  pDrawUndo;      // geloeschte Objekte
    ULONG           nStartChangeAction;
    ULONG           nEndChangeAction;
    USHORT          nFlags;
    BOOL            bMulti;         // Mehrfachselektion

    void            DoChange( const BOOL bUndo );
    void            SetChangeTrack();
};


class ScUndoFillTable: public ScSimpleUndo
{
public:
                    TYPEINFO();
                    ScUndoFillTable( ScDocShell* pNewDocShell,
                                     const ScMarkData& rMark,
                                     USHORT nStartX, USHORT nStartY, USHORT nStartZ,
                                     USHORT nEndX, USHORT nEndY, USHORT nEndZ,
                                     ScDocument* pNewUndoDoc, BOOL bNewMulti, USHORT nSrc,
                                     USHORT nFlg, USHORT nFunc, BOOL bSkip, BOOL bLink );
    virtual         ~ScUndoFillTable();

    virtual void    Undo();
    virtual void    Redo();
    virtual void    Repeat(SfxRepeatTarget& rTarget);
    virtual BOOL    CanRepeat(SfxRepeatTarget& rTarget) const;

    virtual String  GetComment() const;

private:
    ScRange         aRange;
    ScMarkData      aMarkData;
    ScDocument*     pUndoDoc;       // Blockmarkierung und geloeschte Daten
    ULONG           nStartChangeAction;
    ULONG           nEndChangeAction;
    USHORT          nFlags;
    USHORT          nFunction;
    USHORT          nSrcTab;
    BOOL            bMulti;         // Mehrfachselektion
    BOOL            bSkipEmpty;
    BOOL            bAsLink;

    void            DoChange( const BOOL bUndo );
    void            SetChangeTrack();
};


class ScUndoSelectionAttr: public ScSimpleUndo
{
public:
                    TYPEINFO();
                    ScUndoSelectionAttr( ScDocShell* pNewDocShell,
                                         const ScMarkData& rMark,
                                         USHORT nStartX, USHORT nStartY, USHORT nStartZ,
                                         USHORT nEndX, USHORT nEndY, USHORT nEndZ,
                                         ScDocument* pNewUndoDoc, BOOL bNewMulti,
                                         const ScPatternAttr* pNewApply,
                                         const SvxBoxItem* pNewOuter = NULL,
                                         const SvxBoxInfoItem* pNewInner = NULL );
    virtual         ~ScUndoSelectionAttr();

    virtual void    Undo();
    virtual void    Redo();
    virtual void    Repeat(SfxRepeatTarget& rTarget);
    virtual BOOL    CanRepeat(SfxRepeatTarget& rTarget) const;

    virtual String  GetComment() const;

private:
    ScMarkData      aMarkData;
    ScRange         aRange;
    ScDocument*     pUndoDoc;
    BOOL            bMulti;
    ScPatternAttr*  pApplyPattern;
    SvxBoxItem*     pLineOuter;
    SvxBoxInfoItem* pLineInner;

    void            DoChange( const BOOL bUndo );
};


class ScUndoWidthOrHeight: public ScSimpleUndo
{
public:
                            TYPEINFO();
                            ScUndoWidthOrHeight( ScDocShell* pNewDocShell,
                                    const ScMarkData& rMark,
                                    USHORT nNewStart, USHORT nNewStartTab,
                                    USHORT nNewEnd, USHORT nNewEndTab,
                                    ScDocument* pNewUndoDoc,
                                    USHORT nNewCnt, USHORT* pNewRanges,
                                    ScOutlineTable* pNewUndoTab,
                                    ScSizeMode eNewMode, USHORT nNewSizeTwips,
                                    BOOL bNewWidth );
    virtual                 ~ScUndoWidthOrHeight();

    virtual void            Undo();
    virtual void            Redo();
    virtual void            Repeat(SfxRepeatTarget& rTarget);
    virtual BOOL            CanRepeat(SfxRepeatTarget& rTarget) const;

    virtual String          GetComment() const;

private:
    ScMarkData      aMarkData;
    USHORT          nStart;
    USHORT          nEnd;
    USHORT          nStartTab;
    USHORT          nEndTab;
    ScDocument*     pUndoDoc;
    ScOutlineTable* pUndoTab;
    USHORT          nRangeCnt;
    USHORT*         pRanges;
    USHORT          nNewSize;
    BOOL            bWidth;
    ScSizeMode      eMode;
    SdrUndoAction*  pDrawUndo;
};


class ScUndoAutoFill: public ScBlockUndo
{
public:
                    TYPEINFO();
                    ScUndoAutoFill( ScDocShell* pNewDocShell,
                                    const ScRange& rRange, const ScRange& rSourceArea,
                                    ScDocument* pNewUndoDoc, const ScMarkData& rMark,
                                    FillDir eNewFillDir,
                                    FillCmd eNewFillCmd, FillDateCmd eNewFillDateCmd,
                                    double fNewStartValue, double fNewStepValue, double fNewMaxValue,
                                    USHORT nMaxShIndex );
    virtual         ~ScUndoAutoFill();

    virtual void    Undo();
    virtual void    Redo();
    virtual void    Repeat(SfxRepeatTarget& rTarget);
    virtual BOOL    CanRepeat(SfxRepeatTarget& rTarget) const;

    virtual String  GetComment() const;

private:
    ScRange         aSource;
    ScMarkData      aMarkData;
    ScDocument*     pUndoDoc;
    FillDir         eFillDir;
    FillCmd         eFillCmd;
    FillDateCmd     eFillDateCmd;
    double          fStartValue;
    double          fStepValue;
    double          fMaxValue;
    ULONG           nStartChangeAction;
    ULONG           nEndChangeAction;
    USHORT          nMaxSharedIndex;

    void            SetChangeTrack();
};


class ScUndoMerge: public ScSimpleUndo
{
public:
                    TYPEINFO();
                    ScUndoMerge( ScDocShell* pNewDocShell,
                                 USHORT nStartX, USHORT nStartY, USHORT nStartZ,
                                 USHORT nEndX,   USHORT nEndY,   USHORT nEndZ,
                                 BOOL bNewDoMerge, ScDocument* pNewUndoDoc = NULL );
    virtual         ~ScUndoMerge();

    virtual void    Undo();
    virtual void    Redo();
    virtual void    Repeat(SfxRepeatTarget& rTarget);
    virtual BOOL    CanRepeat(SfxRepeatTarget& rTarget) const;

    virtual String  GetComment() const;

private:
    ScRange         aRange;
    BOOL            bDoMerge;               // Merge oder aufheben
    ScDocument*     pUndoDoc;               // wenn Daten zusammengefasst

    void            DoChange( const BOOL bUndo ) const;
};


class ScUndoAutoFormat: public ScBlockUndo
{
public:
                    TYPEINFO();
                    ScUndoAutoFormat( ScDocShell* pNewDocShell,
                                      const ScRange& rRange, ScDocument* pNewUndoDoc,
                                      const ScMarkData& rMark,
                                      BOOL bNewSize, USHORT nNewFormatNo );
    virtual         ~ScUndoAutoFormat();

    virtual void    Undo();
    virtual void    Redo();
    virtual void    Repeat(SfxRepeatTarget& rTarget);
    virtual BOOL    CanRepeat(SfxRepeatTarget& rTarget) const;

    virtual String  GetComment() const;

private:
    ScDocument*     pUndoDoc;       // geloeschte Daten
    ScMarkData      aMarkData;
    BOOL            bSize;
    USHORT          nFormatNo;
};


class ScUndoReplace: public ScSimpleUndo
{
public:
                    TYPEINFO();
                    ScUndoReplace( ScDocShell* pNewDocShell,
                                   const ScMarkData& rMark,
                                   USHORT nCurX, USHORT nCurY, USHORT nCurZ,
                                   const String& rNewUndoStr, ScDocument* pNewUndoDoc,
                                   const SvxSearchItem* pItem );
    virtual         ~ScUndoReplace();

    virtual void    Undo();
    virtual void    Redo();
    virtual void    Repeat(SfxRepeatTarget& rTarget);
    virtual BOOL    CanRepeat(SfxRepeatTarget& rTarget) const;

    virtual String  GetComment() const;

private:
    ScTripel        aCursorPos;
    ScMarkData      aMarkData;
    String          aUndoStr;           // Daten bei Einfachmarkierung
    ScDocument*     pUndoDoc;           // Blockmarkierung und geloeschte Daten
    SvxSearchItem*  pSearchItem;
    ULONG           nStartChangeAction;
    ULONG           nEndChangeAction;

            void    SetChangeTrack();
};


class ScUndoTabOp: public ScSimpleUndo
{
public:
                    TYPEINFO();
                    ScUndoTabOp( ScDocShell* pNewDocShell,
                                 USHORT nStartX, USHORT nStartY, USHORT nStartZ,
                                 USHORT nEndX,   USHORT nEndY,   USHORT nEndZ,
                                 ScDocument* pNewUndoDoc,
                                 const ScRefTripel& rFormulaCell,
                                 const ScRefTripel& rFormulaEnd,
                                 const ScRefTripel& rRowCell,
                                 const ScRefTripel& rColCell,
                                 BYTE nMode );
    virtual         ~ScUndoTabOp();

    virtual void    Undo();
    virtual void    Redo();
    virtual void    Repeat(SfxRepeatTarget& rTarget);
    virtual BOOL    CanRepeat(SfxRepeatTarget& rTarget) const;

    virtual String  GetComment() const;

private:
    ScRange         aRange;
    ScDocument*     pUndoDoc;       // geloeschte Daten
    ScRefTripel     theFormulaCell;
    ScRefTripel     theFormulaEnd;
    ScRefTripel     theRowCell;
    ScRefTripel     theColCell;
    BYTE            nMode;
};


class ScUndoSpelling: public ScSimpleUndo
{
public:
                    TYPEINFO();
                    ScUndoSpelling( ScDocShell* pNewDocShell,
                                    const ScMarkData& rMark,
                                    USHORT nCurX, USHORT nCurY, USHORT nCurZ,
                                    ScDocument* pNewUndoDoc,
                                    USHORT nNewX, USHORT nNewY, USHORT nNewZ,
                                    ScDocument* pNewRedoDoc);
    virtual         ~ScUndoSpelling();

    virtual void    Undo();
    virtual void    Redo();
    virtual void    Repeat(SfxRepeatTarget& rTarget);
    virtual BOOL    CanRepeat(SfxRepeatTarget& rTarget) const;

    virtual String  GetComment() const;

private:
    ScMarkData      aMarkData;
    ScTripel        aCursorPos;
    ScDocument*     pUndoDoc;           // Blockmarkierung und geloeschte Daten
    ScTripel        aNewCursorPos;
    ScDocument*     pRedoDoc;           // Blockmarkierung und neue Daten
    ULONG           nStartChangeAction;
    ULONG           nEndChangeAction;

    void            DoChange( ScDocument* pRefDoc, const ScTripel& rCursorPos );
    void            SetChangeTrack();
};


class ScUndoListNames: public ScBlockUndo
{
public:
                    TYPEINFO();
                    ScUndoListNames( ScDocShell* pNewDocShell,
                                     const ScRange& rRange,
                                     ScDocument* pNewUndoDoc, ScDocument* pNewRedoDoc );
    virtual         ~ScUndoListNames();

    virtual void    Undo();
    virtual void    Redo();
    virtual void    Repeat(SfxRepeatTarget& rTarget);
    virtual BOOL    CanRepeat(SfxRepeatTarget& rTarget) const;

    virtual String  GetComment() const;

private:
    ScDocument*     pUndoDoc;
    ScDocument*     pRedoDoc;

    void            DoChange( ScDocument* pSrcDoc ) const;
};


class ScUndoUseScenario: public ScSimpleUndo
{
public:
                            TYPEINFO();
                            ScUndoUseScenario( ScDocShell* pNewDocShell,
                                    const ScMarkData& rMark,
                                    const ScArea& rDestArea, ScDocument* pNewUndoDoc,
                                    const String& rNewName );
    virtual                 ~ScUndoUseScenario();

    virtual void            Undo();
    virtual void            Redo();
    virtual void            Repeat(SfxRepeatTarget& rTarget);
    virtual BOOL            CanRepeat(SfxRepeatTarget& rTarget) const;

    virtual String          GetComment() const;

private:
    ScDocument*     pUndoDoc;
    ScRange         aRange;
    ScMarkData      aMarkData;
    String          aName;
};


class ScUndoSelectionStyle: public ScSimpleUndo
{
public:
                    TYPEINFO();
                    ScUndoSelectionStyle( ScDocShell* pNewDocShell,
                                          const ScMarkData& rMark,
                                          const ScRange& rRange,
                                          const String& rName,
                                          ScDocument* pNewUndoDoc );
    virtual         ~ScUndoSelectionStyle();

    virtual void    Undo();
    virtual void    Redo();
    virtual void    Repeat(SfxRepeatTarget& rTarget);
    virtual BOOL    CanRepeat(SfxRepeatTarget& rTarget) const;

    virtual String  GetComment() const;
    virtual USHORT  GetId() const;

private:
    ScMarkData      aMarkData;
    ScDocument*     pUndoDoc;
    String          aStyleName;
    ScRange         aRange;

    void            DoChange( const BOOL bUndo );
};


class ScUndoRefreshLink: public ScSimpleUndo
{
public:
                    TYPEINFO();
                    ScUndoRefreshLink( ScDocShell* pNewDocShell,
                                       ScDocument* pNewUndoDoc );
    virtual         ~ScUndoRefreshLink();

    virtual void    Undo();
    virtual void    Redo();
    virtual void    Repeat(SfxRepeatTarget& rTarget);
    virtual BOOL    CanRepeat(SfxRepeatTarget& rTarget) const;

    virtual String  GetComment() const;

private:
    ScDocument*     pUndoDoc;
    ScDocument*     pRedoDoc;
};


class ScUndoEnterMatrix: public ScBlockUndo
{
public:
                    TYPEINFO();
                    ScUndoEnterMatrix( ScDocShell* pNewDocShell,
                                       const ScRange& rArea,
                                       ScDocument* pNewUndoDoc,
                                       const String& rForm );
    virtual         ~ScUndoEnterMatrix();

    virtual void    Undo();
    virtual void    Redo();
    virtual void    Repeat(SfxRepeatTarget& rTarget);
    virtual BOOL    CanRepeat(SfxRepeatTarget& rTarget) const;

    virtual String  GetComment() const;

private:
    ScDocument*     pUndoDoc;
    String          aFormula;
    ULONG           nStartChangeAction;
    ULONG           nEndChangeAction;

    void            SetChangeTrack();
};


class ScUndoInsertAreaLink : public ScSimpleUndo
{
public:
                    TYPEINFO();
                    ScUndoInsertAreaLink( ScDocShell* pShell,
                                          const String& rDoc,
                                          const String& rFlt, const String& rOpt,
                                          const String& rArea, const ScRange& rDestRange,
                                          ULONG nRefreshDelay );
    virtual         ~ScUndoInsertAreaLink();

    virtual void    Undo();
    virtual void    Redo();
    virtual void    Repeat(SfxRepeatTarget& rTarget);
    virtual BOOL    CanRepeat(SfxRepeatTarget& rTarget) const;

    virtual String  GetComment() const;

private:
    String          aDocName;
    String          aFltName;
    String          aOptions;
    String          aAreaName;
    ScRange         aRange;
    ULONG           nRefreshDelay;
};


class ScUndoRemoveAreaLink : public ScSimpleUndo
{
public:
                    TYPEINFO();
                    ScUndoRemoveAreaLink( ScDocShell* pShell,
                                          const String& rDoc,
                                          const String& rFlt, const String& rOpt,
                                          const String& rArea, const ScRange& rDestRange,
                                          ULONG nRefreshDelay );
    virtual         ~ScUndoRemoveAreaLink();

    virtual void    Undo();
    virtual void    Redo();
    virtual void    Repeat(SfxRepeatTarget& rTarget);
    virtual BOOL    CanRepeat(SfxRepeatTarget& rTarget) const;

    virtual String  GetComment() const;

private:
    String          aDocName;
    String          aFltName;
    String          aOptions;
    String          aAreaName;
    ScRange         aRange;
    ULONG           nRefreshDelay;
};


class ScUndoUpdateAreaLink : public ScSimpleUndo        //! auch BlockUndo umstellen?
{
public:
                    TYPEINFO();
                    ScUndoUpdateAreaLink( ScDocShell* pShell,
                                          const String& rOldD,
                                          const String& rOldF, const String& rOldO,
                                          const String& rOldA, const ScRange& rOldR,
                                          ULONG nOldRD,
                                          const String& rNewD,
                                          const String& rNewF, const String& rNewO,
                                          const String& rNewA, const ScRange& rNewR,
                                          ULONG nNewRD,
                                          ScDocument* pUndo, ScDocument* pRedo,
                                          BOOL bDoInsert );
    virtual         ~ScUndoUpdateAreaLink();

    virtual void    Undo();
    virtual void    Redo();
    virtual void    Repeat(SfxRepeatTarget& rTarget);
    virtual BOOL    CanRepeat(SfxRepeatTarget& rTarget) const;

    virtual String  GetComment() const;

private:
    String          aOldDoc;
    String          aOldFlt;
    String          aOldOpt;
    String          aOldArea;
    ScRange         aOldRange;
    String          aNewDoc;
    String          aNewFlt;
    String          aNewOpt;
    String          aNewArea;
    ScRange         aNewRange;
    ScDocument*     pUndoDoc;
    ScDocument*     pRedoDoc;
    ULONG           nOldRefresh;
    ULONG           nNewRefresh;
    BOOL            bWithInsert;

    void            DoChange( const BOOL bUndo ) const;
};


class ScUndoIndent: public ScBlockUndo
{
public:
                    TYPEINFO();
                    ScUndoIndent( ScDocShell* pNewDocShell, const ScMarkData& rMark,
                                    ScDocument* pNewUndoDoc, BOOL bIncrement );
    virtual         ~ScUndoIndent();

    virtual void    Undo();
    virtual void    Redo();
    virtual void    Repeat(SfxRepeatTarget& rTarget);
    virtual BOOL    CanRepeat(SfxRepeatTarget& rTarget) const;

    virtual String  GetComment() const;

private:
    ScMarkData      aMarkData;
    ScDocument*     pUndoDoc;
    BOOL            bIsIncrement;
};


class ScUndoTransliterate: public ScBlockUndo
{
public:
                    TYPEINFO();
                    ScUndoTransliterate( ScDocShell* pNewDocShell, const ScMarkData& rMark,
                                        ScDocument* pNewUndoDoc, sal_Int32 nType );
    virtual         ~ScUndoTransliterate();

    virtual void    Undo();
    virtual void    Redo();
    virtual void    Repeat(SfxRepeatTarget& rTarget);
    virtual BOOL    CanRepeat(SfxRepeatTarget& rTarget) const;

    virtual String  GetComment() const;

private:
    ScMarkData      aMarkData;
    ScDocument*     pUndoDoc;
    sal_Int32       nTransliterationType;
};


class ScUndoClearItems: public ScBlockUndo
{
public:
                    TYPEINFO();
                    ScUndoClearItems( ScDocShell* pNewDocShell, const ScMarkData& rMark,
                                        ScDocument* pNewUndoDoc, const USHORT* pW );
    virtual         ~ScUndoClearItems();

    virtual void    Undo();
    virtual void    Redo();
    virtual void    Repeat(SfxRepeatTarget& rTarget);
    virtual BOOL    CanRepeat(SfxRepeatTarget& rTarget) const;

    virtual String  GetComment() const;

private:
    ScMarkData      aMarkData;
    ScDocument*     pUndoDoc;
    USHORT*         pWhich;
};


class ScUndoRemoveBreaks: public ScSimpleUndo
{
public:
                    TYPEINFO();
                    ScUndoRemoveBreaks( ScDocShell* pNewDocShell,
                                    USHORT nNewTab, ScDocument* pNewUndoDoc );
    virtual         ~ScUndoRemoveBreaks();

    virtual void    Undo();
    virtual void    Redo();
    virtual void    Repeat(SfxRepeatTarget& rTarget);
    virtual BOOL    CanRepeat(SfxRepeatTarget& rTarget) const;

    virtual String  GetComment() const;

private:
    USHORT          nTab;
    ScDocument*     pUndoDoc;
};


class ScUndoRemoveMerge: public ScBlockUndo
{
public:
                    TYPEINFO();
                    ScUndoRemoveMerge( ScDocShell* pNewDocShell,
                                       const ScRange& rArea,
                                       ScDocument* pNewUndoDoc );
    virtual         ~ScUndoRemoveMerge();

    virtual void    Undo();
    virtual void    Redo();
    virtual void    Repeat(SfxRepeatTarget& rTarget);
    virtual BOOL    CanRepeat(SfxRepeatTarget& rTarget) const;

    virtual String  GetComment() const;

private:
    ScDocument*     pUndoDoc;
};


class ScUndoBorder: public ScBlockUndo
{
public:
                    TYPEINFO();
                    ScUndoBorder( ScDocShell* pNewDocShell,
                                    const ScRangeList& rRangeList,
                                    ScDocument* pNewUndoDoc,
                                    const SvxBoxItem& rNewOuter,
                                    const SvxBoxInfoItem& rNewInner );
    virtual         ~ScUndoBorder();

    virtual void    Undo();
    virtual void    Redo();
    virtual void    Repeat(SfxRepeatTarget& rTarget);
    virtual BOOL    CanRepeat(SfxRepeatTarget& rTarget) const;

    virtual String  GetComment() const;

private:
    ScDocument*     pUndoDoc;
    ScRangeList*    pRanges;
    SvxBoxItem*     pOuter;
    SvxBoxInfoItem* pInner;
};




#endif

