/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



#ifndef _SVX_SVDOTABLE_HXX
#define _SVX_SVDOTABLE_HXX

#include <com/sun/star/text/WritingMode.hpp>
#include <com/sun/star/container/XIndexAccess.hpp>
#include <com/sun/star/table/XTable.hpp>
#include <rtl/ref.hxx>
#include "svdotext.hxx"
#include "svx/svxdllapi.h"

//------------------------------------------------------------------------

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

//------------------------------------------------------------------------
// SdrTableHitKind
//------------------------------------------------------------------------

enum TableHitKind
{
    SDRTABLEHIT_NONE,
    SDRTABLEHIT_CELL,
    SDRTABLEHIT_CELLTEXTAREA,
    SDRTABLEHIT_HORIZONTAL_BORDER,
    SDRTABLEHIT_VERTICAL_BORDER
};

//------------------------------------------------------------------------

struct CellPos
{
    sal_Int32 mnCol;
    sal_Int32 mnRow;

    CellPos() : mnCol( 0 ), mnRow( 0 ) {}
    CellPos( sal_Int32 nCol, sal_Int32 nRow ) { mnCol = nCol; mnRow = nRow; }

    bool operator==( const CellPos& r ) const { return (r.mnCol == mnCol) && (r.mnRow == mnRow); }
    bool operator!=( const CellPos& r ) const { return (r.mnCol != mnCol) || (r.mnRow != mnRow); }
};

//------------------------------------------------------------------------
// TableStyleSettings
//------------------------------------------------------------------------

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

//------------------------------------------------------------------------
//   SdrTableObj
//------------------------------------------------------------------------

class SdrTableObjImpl;

class SVX_DLLPUBLIC SdrTableObj : public ::SdrTextObj
{
private:
    friend class Cell;
    friend class SdrTableObjImpl;
    friend class SdrTableModifyGuard;

protected:
    virtual ~SdrTableObj();

    /// method to copy all data from given source
    virtual void copyDataFromSdrObject(const SdrObject& rSource);

public:
    /// create a copy, evtl. with a different target model (if given)
    virtual SdrObject* CloneSdrObject(SdrModel* pTargetModel = 0) const;

    SdrTableObj(
        SdrModel& rSdrModel,
        const basegfx::B2DHomMatrix& rTransform = basegfx::B2DHomMatrix(),
        sal_Int32 nColumns = 1,
        sal_Int32 nRows = 1);
    virtual bool IsClosedObj() const;

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

    TableHitKind CheckTableHit( const basegfx::B2DPoint& rPos, sal_Int32& rnX, sal_Int32& rnY, int nTol ) const;

    void uno_lock();
    void uno_unlock();

    /** the active table has the focus or is currently edited */
    const ::sdr::table::CellRef& getActiveCell() const;

    void setActiveCell( const sdr::table::CellPos& rPos );
    void getActiveCellPos( sdr::table::CellPos& rPos ) const;

    sal_Int32 getRowCount() const;
    sal_Int32 getColumnCount() const;

    void getCellBounds( const sdr::table::CellPos& rPos, ::Rectangle& rCellRect );

    const SfxItemSet& GetActiveCellItemSet() const;

     void InsertRows( sal_Int32 nIndex, sal_Int32 nCount = 1 );
     void InsertColumns( sal_Int32 nIndex, sal_Int32 nCount = 1 );
     void DeleteRows( sal_Int32 nIndex, sal_Int32 nCount = 1 );
     void DeleteColumns( sal_Int32 nIndex, sal_Int32 nCount = 1 );

     void setTableStyle( const ::com::sun::star::uno::Reference< ::com::sun::star::container::XIndexAccess >& xAutoFormatStyle );
     const ::com::sun::star::uno::Reference< ::com::sun::star::container::XIndexAccess >& getTableStyle() const;

     // text stuff

    /** returns the currently active text. */
    virtual SdrText* getActiveText() const;

    /** returns the nth available text. */
    virtual SdrText* getText( sal_Int32 nIndex ) const;

    /** returns the number of texts available for this object. */
    virtual sal_Int32 getTextCount() const;

    /** changes the current active text */
    virtual void setActiveText( sal_Int32 nIndex );

    /** returns the index of the text that contains the given point or -1 */
    virtual sal_Int32 CheckTextHit(const basegfx::B2DPoint& rPnt) const;

    virtual bool HasText() const;
    bool IsTextEditActive() const { return (pEdtOutl != 0L); }
    bool IsTextEditActive( const sdr::table::CellPos& rPos );

    /** returns true only if we are in edit mode and the user actually changed anything */
    virtual bool IsRealyEdited() const;

    void FitFrameToTextSize();

    SdrOutliner* GetCellTextEditOutliner( const sdr::table::CellPos& rPos ) const;

    // Gleichzeitig wird der Text in den Outliner gesetzt (ggf.
    // der des EditOutliners) und die PaperSize gesetzt.
    void TakeTextRange(const sdr::table::CellPos& rPos, SdrOutliner& rOutliner, basegfx::B2DRange& rTextRange, basegfx::B2DRange& rAnchorRange) const;
    virtual void TakeTextRange(SdrOutliner& rOutliner, basegfx::B2DRange& rTextRange, basegfx::B2DRange& rAnchorRange) const;
    virtual void TakeTextAnchorRangeFromCell(const sdr::table::CellPos& rPos, basegfx::B2DRange& rAnchorRange ) const;
    virtual basegfx::B2DRange getUnifiedTextRange() const;

    virtual bool IsAutoGrowHeight() const;
    long GetMinTextFrameHeight() const;
    long GetMaxTextFrameHeight() const;
    virtual bool IsAutoGrowWidth() const;
    long GetMinTextFrameWidth() const;
    long GetMaxTextFrameWidth() const;

    virtual bool IsFontwork() const;

    virtual void TakeObjInfo(SdrObjTransformInfoRec& rInfo) const;
    virtual sal_uInt16 GetObjIdentifier() const;
    virtual void SetChanged();

    virtual basegfx::B2DRange AdjustTextFrameWidthAndHeight(const basegfx::B2DRange& rRange, bool bHgt = true, bool bWdt = true) const;
    virtual bool AdjustTextFrameWidthAndHeight(bool bHgt = true, bool bWdt = true);
    virtual void TakeObjNameSingul(String& rName) const;
    virtual void TakeObjNamePlural(String& rName) const;
    virtual basegfx::B2DPolyPolygon TakeXorPoly() const;
    virtual void AdjustToMaxRange( const basegfx::B2DRange& rMaxRange, bool bShrinkOnly = false );

    virtual sal_uInt32 GetSnapPointCount() const;
    virtual basegfx::B2DPoint GetSnapPoint(sal_uInt32 i) const;

    virtual void AddToHdlList(SdrHdlList& rHdlList) const;

    // special drag methods
    virtual bool hasSpecialDrag() const;
    virtual bool beginSpecialDrag(SdrDragStat& rDrag) const;
    virtual bool applySpecialDrag(SdrDragStat& rDrag);
    virtual String getSpecialDragComment(const SdrDragStat& rDrag) const;
    virtual basegfx::B2DPolyPolygon getSpecialDragPoly(const SdrDragStat& rDrag) const;

    virtual bool BckCreate(SdrDragStat& rStat);
    virtual void BrkCreate(SdrDragStat& rStat);
    virtual Pointer GetCreatePointer(const SdrView& rSdrView) const;

    virtual bool BegTextEdit(SdrOutliner& rOutl);
    virtual void EndTextEdit(SdrOutliner& rOutl);
    virtual void TakeTextEditArea(basegfx::B2DVector* pPaperMin, basegfx::B2DVector* pPaperMax, basegfx::B2DRange* pViewInit, basegfx::B2DRange* pViewMin) const;
    void TakeTextEditArea(const sdr::table::CellPos& rPos, basegfx::B2DVector* pPaperMin, basegfx::B2DVector* pPaperMax, basegfx::B2DRange* pViewInit, basegfx::B2DRange* pViewMin) const;
    virtual sal_uInt16 GetOutlinerViewAnchorMode() const;

    virtual void SetOutlinerParaObject(OutlinerParaObject* pTextObject);

    virtual OutlinerParaObject* GetOutlinerParaObject() const;
    virtual OutlinerParaObject* GetEditOutlinerParaObject() const;

    // virtual void NbcReformatText();
    virtual void ReformatText();

    void SetTextEditOutliner(SdrOutliner* pOutl) { pEdtOutl=pOutl; }

    virtual bool IsVerticalWriting() const;
    virtual void SetVerticalWriting(bool bVertical);

    com::sun::star::text::WritingMode GetWritingMode() const;

    virtual void onEditOutlinerStatusEvent( EditStatus* pEditStatus );

    /** hack for clipboard with calc and writer, export and import table content as rtf table */
    static void ExportAsRTF( SvStream& rStrm, SdrTableObj& rObj );
    static void ImportAsRTF( SvStream& rStrm, SdrTableObj& rObj );

    virtual void setSdrObjectTransformation(const basegfx::B2DHomMatrix& rTransformation);

protected:
    virtual sdr::properties::BaseProperties* CreateObjectSpecificProperties();
    virtual sdr::contact::ViewContact* CreateObjectSpecificViewContact();

private:
    void init( sal_Int32 nColumns, sal_Int32 nRows );
    SdrOutliner* GetCellTextEditOutliner( const ::sdr::table::Cell& rCell ) const;

    // for the ViewContactOfTableObj to build the primitive representation, it is necessary to access the
    // TableLayouter for position and attribute informations
    friend class sdr::contact::ViewContactOfTableObj;
    const TableLayouter& getTableLayouter() const;

    SdrTableObjImpl*    mpImpl;
};

////////////////////////////////////////////////////////////////////////////////////////////////////

} }

#endif
