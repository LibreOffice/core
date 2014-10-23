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


// SdrTableHitKind


enum TableHitKind
{
    SDRTABLEHIT_NONE,
    SDRTABLEHIT_CELL,
    SDRTABLEHIT_CELLTEXTAREA,
    SDRTABLEHIT_HORIZONTAL_BORDER,
    SDRTABLEHIT_VERTICAL_BORDER
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


// TableStyleSettings


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


//   SdrTableObj


class SdrTableObjImpl;

class SVX_DLLPUBLIC SdrTableObj : public ::SdrTextObj
{
    friend class Cell;
    friend class SdrTableObjImpl;

public:
    SdrTableObj(SdrModel* _pModel);
    SdrTableObj(SdrModel* _pModel, const ::Rectangle& rNewRect, sal_Int32 nColumns, sal_Int32 nRows);
    virtual ~SdrTableObj();

    TYPEINFO_OVERRIDE();

    // table stuff

    SdrTableObj* CloneRange( const CellPos& rStartPos, const CellPos& rEndPos );
    void DistributeColumns( sal_Int32 nFirstColumn, sal_Int32 nLastColumn );
    void DistributeRows( sal_Int32 nFirstRow, sal_Int32 nLastRow );

    com::sun::star::uno::Reference< com::sun::star::table::XTable > getTable() const;

    bool isValid( const sdr::table::CellPos& rPos ) const;
    CellPos getFirstCell() const;
    CellPos getLastCell() const;
    CellPos getLeftCell( const CellPos& rPos, bool bEdgeTravel ) const;
    CellPos getRightCell( const CellPos& rPos, bool bEdgeTravel  ) const;
    CellPos getUpCell( const CellPos& rPos, bool bEdgeTravel ) const;
    CellPos getDownCell( const CellPos& rPos, bool bEdgeTravel ) const;
    CellPos getPreviousCell( const CellPos& rPos, bool bEdgeTravel ) const;
    CellPos getNextCell( const CellPos& rPos, bool bEdgeTravel ) const;
    CellPos getPreviousRow( const CellPos& rPos, bool bEdgeTravel ) const;
    CellPos getNextRow( const CellPos& rPos, bool bEdgeTravel ) const;

    void createCell( ::sdr::table::CellRef& xCell );

    const ::sdr::table::TableStyleSettings& getTableStyleSettings() const;
    void setTableStyleSettings( const ::sdr::table::TableStyleSettings& rStyle );

    TableHitKind CheckTableHit( const Point& rPos, sal_Int32& rnX, sal_Int32& rnY, int nTol ) const;

    void uno_lock();
    void uno_unlock();

    /** the active table has the focus or is currently edited */
    const ::sdr::table::CellRef& getActiveCell() const;

    void setActiveCell( const sdr::table::CellPos& rPos );
    void getActiveCellPos( sdr::table::CellPos& rPos ) const;
    sal_Int32 getColumnCount() const;
    void getCellBounds( const sdr::table::CellPos& rPos, ::Rectangle& rCellRect );

    const SfxItemSet& GetActiveCellItemSet() const;

     void setTableStyle( const ::com::sun::star::uno::Reference< ::com::sun::star::container::XIndexAccess >& xAutoFormatStyle );
     const ::com::sun::star::uno::Reference< ::com::sun::star::container::XIndexAccess >& getTableStyle() const;

     // text stuff

    /** returns the currently active text. */
    virtual SdrText* getActiveText() const SAL_OVERRIDE;

    /** returns the nth available text. */
    virtual SdrText* getText( sal_Int32 nIndex ) const SAL_OVERRIDE;

    /** returns the number of texts available for this object. */
    virtual sal_Int32 getTextCount() const SAL_OVERRIDE;

    /** changes the current active text */
    virtual void setActiveText( sal_Int32 nIndex ) SAL_OVERRIDE;

    /** returns the index of the text that contains the given point or -1 */
    virtual sal_Int32 CheckTextHit(const Point& rPnt) const SAL_OVERRIDE;

    // #i121917#
    virtual bool HasText() const SAL_OVERRIDE;

    bool IsTextEditActive() const { return (pEdtOutl != 0L); }
    bool IsTextEditActive( const sdr::table::CellPos& rPos );

    /** returns true only if we are in edit mode and the user actually changed anything */
    virtual bool IsRealyEdited() const SAL_OVERRIDE;

    // Gleichzeitig wird der Text in den Outliner gesetzt (ggf.
    // der des EditOutliners) und die PaperSize gesetzt.
    virtual void TakeTextRect( const sdr::table::CellPos& rPos, SdrOutliner& rOutliner, ::Rectangle& rTextRect, bool bNoEditText = false, ::Rectangle* pAnchorRect=NULL, bool bLineWidth = true ) const;
    virtual void TakeTextRect( SdrOutliner& rOutliner, Rectangle& rTextRect, bool bNoEditText = false, Rectangle* pAnchorRect=NULL, bool bLineWidth = true ) const SAL_OVERRIDE;
    virtual void TakeTextAnchorRect(const sdr::table::CellPos& rPos, ::Rectangle& rAnchorRect ) const;
    virtual void TakeTextAnchorRect(::Rectangle& rAnchorRect) const SAL_OVERRIDE;

    virtual bool IsAutoGrowHeight() const SAL_OVERRIDE;
    long GetMinTextFrameHeight() const;
    long GetMaxTextFrameHeight() const;
    virtual bool IsAutoGrowWidth() const SAL_OVERRIDE;
    long GetMinTextFrameWidth() const;
    long GetMaxTextFrameWidth() const;

    virtual bool IsFontwork() const SAL_OVERRIDE;

    virtual void SetPage(SdrPage* pNewPage) SAL_OVERRIDE;
    virtual void SetModel(SdrModel* pNewModel) SAL_OVERRIDE;
    virtual void TakeObjInfo(SdrObjTransformInfoRec& rInfo) const SAL_OVERRIDE;
    virtual sal_uInt16 GetObjIdentifier() const SAL_OVERRIDE;
    virtual void SetChanged() SAL_OVERRIDE;

    virtual bool AdjustTextFrameWidthAndHeight(Rectangle& rR, bool bHgt = true, bool bWdt = true) const SAL_OVERRIDE;
    virtual bool AdjustTextFrameWidthAndHeight(bool bHgt = true, bool bWdt = true) SAL_OVERRIDE;
    virtual OUString TakeObjNameSingul() const SAL_OVERRIDE;
    virtual OUString TakeObjNamePlural() const SAL_OVERRIDE;
    virtual SdrTableObj* Clone() const SAL_OVERRIDE;
    SdrTableObj& operator=(const SdrTableObj& rObj);
    virtual basegfx::B2DPolyPolygon TakeXorPoly() const SAL_OVERRIDE;
    virtual basegfx::B2DPolyPolygon TakeContour() const SAL_OVERRIDE;
    virtual void RecalcSnapRect() SAL_OVERRIDE;
    virtual const Rectangle& GetSnapRect() const SAL_OVERRIDE;
    virtual void NbcSetSnapRect(const Rectangle& rRect) SAL_OVERRIDE;

    virtual const Rectangle& GetLogicRect() const SAL_OVERRIDE;
    virtual void NbcSetLogicRect(const Rectangle& rRect) SAL_OVERRIDE;
    virtual void AdjustToMaxRect( const Rectangle& rMaxRect, bool bShrinkOnly = false ) SAL_OVERRIDE;

    virtual sal_uInt32 GetSnapPointCount() const SAL_OVERRIDE;
    virtual Point GetSnapPoint(sal_uInt32 i) const SAL_OVERRIDE;

    virtual sal_uInt32 GetHdlCount() const SAL_OVERRIDE;
    virtual SdrHdl* GetHdl(sal_uInt32 nHdlNum) const SAL_OVERRIDE;
    virtual void AddToHdlList(SdrHdlList& rHdlList) const SAL_OVERRIDE;

    // special drag methods
    virtual bool hasSpecialDrag() const SAL_OVERRIDE;
    virtual bool beginSpecialDrag(SdrDragStat& rDrag) const SAL_OVERRIDE;
    virtual bool applySpecialDrag(SdrDragStat& rDrag) SAL_OVERRIDE;
    virtual OUString getSpecialDragComment(const SdrDragStat& rDrag) const SAL_OVERRIDE;
    virtual basegfx::B2DPolyPolygon getSpecialDragPoly(const SdrDragStat& rDrag) const SAL_OVERRIDE;

    virtual bool BegCreate(SdrDragStat& rStat) SAL_OVERRIDE;
    virtual bool MovCreate(SdrDragStat& rStat) SAL_OVERRIDE;
    virtual bool EndCreate(SdrDragStat& rStat, SdrCreateCmd eCmd) SAL_OVERRIDE;
    virtual bool BckCreate(SdrDragStat& rStat) SAL_OVERRIDE;
    virtual void BrkCreate(SdrDragStat& rStat) SAL_OVERRIDE;
    virtual basegfx::B2DPolyPolygon TakeCreatePoly(const SdrDragStat& rDrag) const SAL_OVERRIDE;
    virtual Pointer GetCreatePointer() const SAL_OVERRIDE;

    virtual void NbcMove(const Size& rSiz) SAL_OVERRIDE;
    virtual void NbcResize(const Point& rRef, const Fraction& xFact, const Fraction& yFact) SAL_OVERRIDE;

    virtual bool BegTextEdit(SdrOutliner& rOutl) SAL_OVERRIDE;
    virtual void EndTextEdit(SdrOutliner& rOutl) SAL_OVERRIDE;
    virtual void TakeTextEditArea(Size* pPaperMin, Size* pPaperMax, Rectangle* pViewInit, Rectangle* pViewMin) const SAL_OVERRIDE;
    virtual void TakeTextEditArea(const sdr::table::CellPos& rPos, Size* pPaperMin, Size* pPaperMax, Rectangle* pViewInit, Rectangle* pViewMin) const;
    virtual sal_uInt16 GetOutlinerViewAnchorMode() const SAL_OVERRIDE;

    virtual void NbcSetOutlinerParaObject(OutlinerParaObject* pTextObject) SAL_OVERRIDE;

    virtual OutlinerParaObject* GetOutlinerParaObject() const SAL_OVERRIDE;
    virtual OutlinerParaObject* GetEditOutlinerParaObject() const SAL_OVERRIDE;

    virtual void NbcReformatText() SAL_OVERRIDE;
    virtual void ReformatText() SAL_OVERRIDE;

    void SetTextEditOutliner(SdrOutliner* pOutl) { pEdtOutl=pOutl; }

    virtual bool IsVerticalWriting() const SAL_OVERRIDE;
    virtual void SetVerticalWriting(bool bVertical) SAL_OVERRIDE;

    com::sun::star::text::WritingMode GetWritingMode() const;

    virtual void onEditOutlinerStatusEvent( EditStatus* pEditStatus ) SAL_OVERRIDE;




    // transformation interface for StarOfficeAPI. This implements support for
    // homogen 3x3 matrices containing the transformation of the SdrObject. At the
    // moment it contains a shearX, rotation and translation, but for setting all linear
    // transforms like Scale, ShearX, ShearY, Rotate and Translate are supported.


    // gets base transformation and rectangle of object. If it's an SdrPathObj it fills the PolyPolygon
    // with the base geometry and returns TRUE. Otherwise it returns FALSE.
    virtual bool TRGetBaseGeometry(basegfx::B2DHomMatrix& rMatrix, basegfx::B2DPolyPolygon& rPolyPolygon) const SAL_OVERRIDE;

    // sets the base geometry of the object using infos contained in the homogen 3x3 matrix.
    // If it's an SdrPathObj it will use the provided geometry information. The Polygon has
    // to use (0,0) as upper left and will be scaled to the given size in the matrix.
    virtual void TRSetBaseGeometry(const basegfx::B2DHomMatrix& rMatrix, const basegfx::B2DPolyPolygon& rPolyPolygon) SAL_OVERRIDE;

    // #103836# iterates over the paragraphs of a given SdrObject and removes all
    //          hard set character attributes with the which ids contained in the
    //          given vector
//  virtual void RemoveOutlinerCharacterAttribs( const std::vector<sal_uInt16>& rCharWhichIds );

    /** hack for clipboard with calc and writer, export and import table content as rtf table */
    static void ExportAsRTF( SvStream& rStrm, SdrTableObj& rObj );
    static void ImportAsRTF( SvStream& rStrm, SdrTableObj& rObj );

private:
    void init( sal_Int32 nColumns, sal_Int32 nRows );

protected:
    virtual sdr::properties::BaseProperties* CreateObjectSpecificProperties() SAL_OVERRIDE;
    virtual sdr::contact::ViewContact* CreateObjectSpecificViewContact() SAL_OVERRIDE;

    virtual SdrObjGeoData* NewGeoData() const SAL_OVERRIDE;
    virtual void SaveGeoData(SdrObjGeoData& rGeo) const SAL_OVERRIDE;
    virtual void RestGeoData(const SdrObjGeoData& rGeo) SAL_OVERRIDE;

private:
    SdrOutliner* GetCellTextEditOutliner( const ::sdr::table::Cell& rCell ) const;

private:
    // for the ViewContactOfTableObj to build the primitive representation, it is necessary to access the
    // TableLayouter for position and attribute information
    friend class sdr::contact::ViewContactOfTableObj;
    const TableLayouter& getTableLayouter() const;

    Rectangle   maLogicRect;
private:
    SdrTableObjImpl*    mpImpl;
};



} }

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
