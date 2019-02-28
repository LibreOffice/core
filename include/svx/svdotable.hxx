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

#ifndef INCLUDED_SVX_SVDOTABLE_HXX
#define INCLUDED_SVX_SVDOTABLE_HXX

#include <com/sun/star/text/WritingMode.hpp>
#include <com/sun/star/container/XIndexAccess.hpp>
#include <com/sun/star/table/XTable.hpp>
#include <rtl/ref.hxx>
#include <svx/svdotext.hxx>
#include <svx/svxdllapi.h>


class SvStream;
class SfxStyleSheet;
class SdrUndoAction;

namespace sdr { namespace contact {
    class ViewContactOfTableObj;
}}

namespace sdr { namespace table {

class TableLayouter;
struct ImplTableShadowPaintInfo;

#ifndef CellRef
    class Cell;
    typedef rtl::Reference< Cell > CellRef;
#endif


/// SdrTableHitKind
enum class TableHitKind
{
    NONE,
    Cell,
    CellTextArea,
    HorizontalBorder,
    VerticallBorder
};


struct CellPos
{
    sal_Int32 mnCol;
    sal_Int32 mnRow;

    CellPos() : mnCol( 0 ), mnRow( 0 ) {}
    CellPos( sal_Int32 nCol, sal_Int32 nRow ) { mnCol = nCol; mnRow = nRow; }

    bool operator==( const CellPos& r ) const { return (r.mnCol == mnCol) && (r.mnRow == mnRow); }
    bool operator!=( const CellPos& r ) const { return (r.mnCol != mnCol) || (r.mnRow != mnRow); }
};


/// TableStyleSettings
struct SVX_DLLPUBLIC TableStyleSettings
{
    bool mbUseFirstRow;
    bool mbUseLastRow;
    bool mbUseFirstColumn;
    bool mbUseLastColumn;
    bool mbUseRowBanding;
    bool mbUseColumnBanding;

    TableStyleSettings();
    TableStyleSettings( const TableStyleSettings& rStyle );
    TableStyleSettings& operator=(const TableStyleSettings& rStyle);

    bool operator==( const TableStyleSettings& r ) const;
};


/// SdrTableObj
class SdrTableObjImpl;

class SVX_DLLPUBLIC SdrTableObj : public ::SdrTextObj
{
    friend class Cell;
    friend class SdrTableObjImpl;

protected:
    // protected destructor
    virtual ~SdrTableObj() override;

public:
    SdrTableObj(SdrModel& rSdrModel);
    SdrTableObj(
        SdrModel& rSdrModel,
        const ::tools::Rectangle& rNewRect,
        sal_Int32 nColumns,
        sal_Int32 nRows);

    // helper to limit existing TableModel to a given selection
    void CropTableModelToSelection(const CellPos& rStart, const CellPos& rEnd);

    // Table stuff
    void DistributeColumns( sal_Int32 nFirstColumn, sal_Int32 nLastColumn, const bool bOptimize, const bool bMinimize );
    void DistributeRows( sal_Int32 nFirstRow, sal_Int32 nLastRow, const bool bOptimize, const bool bMinimize );

    css::uno::Reference< css::table::XTable > getTable() const;

    bool isValid( const sdr::table::CellPos& rPos ) const;
    static CellPos getFirstCell();
    CellPos getLastCell() const;
    CellPos getLeftCell( const CellPos& rPos, bool bEdgeTravel ) const;
    CellPos getRightCell( const CellPos& rPos, bool bEdgeTravel  ) const;
    CellPos getUpCell( const CellPos& rPos, bool bEdgeTravel ) const;
    CellPos getDownCell( const CellPos& rPos, bool bEdgeTravel ) const;
    CellPos getPreviousCell( const CellPos& rPos, bool bEdgeTravel ) const;
    CellPos getNextCell( const CellPos& rPos, bool bEdgeTravel ) const;
    CellPos getPreviousRow( const CellPos& rPos, bool bEdgeTravel ) const;
    CellPos getNextRow( const CellPos& rPos, bool bEdgeTravel ) const;

    void createCell( sdr::table::CellRef& xCell );

    const sdr::table::TableStyleSettings& getTableStyleSettings() const;
    void setTableStyleSettings( const sdr::table::TableStyleSettings& rStyle );

    TableHitKind CheckTableHit( const Point& rPos, sal_Int32& rnX, sal_Int32& rnY, const sal_uInt16 aTol = 0 ) const;

    void uno_lock();
    void uno_unlock();

    /** The active table has the focus or is currently edited */
    const sdr::table::CellRef& getActiveCell() const;

    void setActiveCell( const sdr::table::CellPos& rPos );
    void getActiveCellPos( sdr::table::CellPos& rPos ) const;
    sal_Int32 getColumnCount() const;
    void getCellBounds( const sdr::table::CellPos& rPos, ::tools::Rectangle& rCellRect );

    const SfxItemSet& GetActiveCellItemSet() const;

     void setTableStyle( const css::uno::Reference< css::container::XIndexAccess >& xAutoFormatStyle );
     const css::uno::Reference< css::container::XIndexAccess >& getTableStyle() const;

    // Text stuff

    /** Returns the currently active text */
    virtual SdrText* getActiveText() const override;

    /** Returns the nth available text */
    virtual SdrText* getText( sal_Int32 nIndex ) const override;

    /** Returns the number of texts available for this object */
    virtual sal_Int32 getTextCount() const override;

    /** Changes the current active text */
    virtual void setActiveText( sal_Int32 nIndex ) override;

    /** Returns the index of the text that contains the given point or -1 */
    virtual sal_Int32 CheckTextHit(const Point& rPnt) const override;

    // #i121917#
    virtual bool HasText() const override;

    bool IsTextEditActive() const { return (pEdtOutl != nullptr); }
    bool IsTextEditActive( const sdr::table::CellPos& rPos );

    /** Returns true only if we are in edit mode and the user actually changed anything */
    virtual bool IsReallyEdited() const override;

    /** At the same time, we set the text in the outliner (if applicable the EditOutliners')
     * as well as the PaperSize
     */
    void TakeTextRect( const sdr::table::CellPos& rPos, SdrOutliner& rOutliner, ::tools::Rectangle& rTextRect, bool bNoEditText, ::tools::Rectangle* pAnchorRect ) const;
    virtual void TakeTextRect( SdrOutliner& rOutliner, tools::Rectangle& rTextRect, bool bNoEditText, tools::Rectangle* pAnchorRect, bool bLineWidth = true ) const override;
    void TakeTextAnchorRect(const sdr::table::CellPos& rPos, ::tools::Rectangle& rAnchorRect ) const;
    virtual void TakeTextAnchorRect(::tools::Rectangle& rAnchorRect) const override;

    virtual bool IsAutoGrowHeight() const override;
    virtual bool IsAutoGrowWidth() const override;

    virtual bool IsFontwork() const override;

    virtual void TakeObjInfo(SdrObjTransformInfoRec& rInfo) const override;
    virtual sal_uInt16 GetObjIdentifier() const override;
    virtual void SetChanged() override;

    virtual bool AdjustTextFrameWidthAndHeight(tools::Rectangle& rR, bool bHgt = true, bool bWdt = true) const override;
    virtual bool AdjustTextFrameWidthAndHeight() override;
    virtual OUString TakeObjNameSingul() const override;
    virtual OUString TakeObjNamePlural() const override;
    virtual SdrTableObj* CloneSdrObject(SdrModel& rTargetModel) const override;
    SdrTableObj& operator=(const SdrTableObj& rObj);
    virtual void RecalcSnapRect() override;
    virtual const tools::Rectangle& GetSnapRect() const override;
    virtual void NbcSetSnapRect(const tools::Rectangle& rRect) override;

    virtual const tools::Rectangle& GetLogicRect() const override;
    virtual void NbcSetLogicRect(const tools::Rectangle& rRect) override;
    virtual void AdjustToMaxRect( const tools::Rectangle& rMaxRect, bool bShrinkOnly = false ) override;

    virtual sal_uInt32 GetHdlCount() const override;
    virtual void AddToHdlList(SdrHdlList& rHdlList) const override;

    // Special drag methods
    virtual bool hasSpecialDrag() const override;
    virtual bool beginSpecialDrag(SdrDragStat& rDrag) const override;
    virtual bool applySpecialDrag(SdrDragStat& rDrag) override;
    virtual basegfx::B2DPolyPolygon getSpecialDragPoly(const SdrDragStat& rDrag) const override;

    virtual bool BegCreate(SdrDragStat& rStat) override;
    virtual bool MovCreate(SdrDragStat& rStat) override;
    virtual bool EndCreate(SdrDragStat& rStat, SdrCreateCmd eCmd) override;
    virtual bool BckCreate(SdrDragStat& rStat) override;
    virtual void BrkCreate(SdrDragStat& rStat) override;
    virtual basegfx::B2DPolyPolygon TakeCreatePoly(const SdrDragStat& rDrag) const override;
    virtual PointerStyle GetCreatePointer() const override;

    virtual void NbcMove(const Size& rSiz) override;
    virtual void NbcResize(const Point& rRef, const Fraction& xFact, const Fraction& yFact) override;

    virtual bool BegTextEdit(SdrOutliner& rOutl) override;
    virtual void EndTextEdit(SdrOutliner& rOutl) override;
    virtual void TakeTextEditArea(Size* pPaperMin, Size* pPaperMax, tools::Rectangle* pViewInit, tools::Rectangle* pViewMin) const override;
    void TakeTextEditArea(const sdr::table::CellPos& rPos, Size* pPaperMin, Size* pPaperMax, tools::Rectangle* pViewInit, tools::Rectangle* pViewMin) const;
    virtual EEAnchorMode GetOutlinerViewAnchorMode() const override;

    virtual void NbcSetOutlinerParaObject(std::unique_ptr<OutlinerParaObject> pTextObject) override;

    virtual OutlinerParaObject* GetOutlinerParaObject() const override;

    virtual void NbcReformatText() override;

    virtual bool IsVerticalWriting() const override;
    virtual void SetVerticalWriting(bool bVertical) override;

    css::text::WritingMode GetWritingMode() const;

    /// Add an undo action that should be on the undo stack after ending text edit.
    void AddUndo(SdrUndoAction* pUndo);

    /// Next time layouting would be done, skip it (to layout at the end of multiple actions).
    void SetSkipChangeLayout(bool bSkipChangeLayout);

    virtual void onEditOutlinerStatusEvent( EditStatus* pEditStatus ) override;

    /** Hack for clipboard with calc and writer, export and import table content as rtf table */
    static void ExportAsRTF( SvStream& rStrm, SdrTableObj& rObj );
    static void ImportAsRTF( SvStream& rStrm, SdrTableObj& rObj );

    virtual void dumpAsXml(xmlTextWriterPtr pWriter) const override;

private:
    void init( sal_Int32 nColumns, sal_Int32 nRows );

protected:
    virtual std::unique_ptr<sdr::properties::BaseProperties> CreateObjectSpecificProperties() override;
    virtual std::unique_ptr<sdr::contact::ViewContact> CreateObjectSpecificViewContact() override;

    virtual SdrObjGeoData* NewGeoData() const override;
    virtual void SaveGeoData(SdrObjGeoData& rGeo) const override;
    virtual void RestGeoData(const SdrObjGeoData& rGeo) override;

private:
    SdrOutliner* GetCellTextEditOutliner( const sdr::table::Cell& rCell ) const;

private:
    // For the ViewContactOfTableObj to build the primitive representation, it is necessary to access the
    // TableLayouter for position and attribute information
    friend class sdr::contact::ViewContactOfTableObj;
    const TableLayouter& getTableLayouter() const;

    tools::Rectangle   maLogicRect;
private:
    rtl::Reference<SdrTableObjImpl>    mpImpl;
};


} }

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
