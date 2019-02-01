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

#include <com/sun/star/text/XTextTable.hpp>
#include <com/sun/star/text/XTextSection.hpp>

#include <hintids.hxx>
#include <rtl/ustrbuf.hxx>
#include <xmloff/xmlnmspe.hxx>
#include <xmloff/xmltoken.hxx>
#include <xmloff/xmluconv.hxx>
#include <xmloff/numehelp.hxx>
#include <svl/zforlist.hxx>
#include <editeng/brushitem.hxx>
#include <editeng/boxitem.hxx>
#include <editeng/xmlcnitm.hxx>
#include <fmtrowsplt.hxx>
#include <editeng/frmdiritem.hxx>
#include <list>
#include <swtable.hxx>
#include <doc.hxx>
#include <pam.hxx>
#include <frmfmt.hxx>
#include <wrtswtbl.hxx>
#include <fmtfsize.hxx>
#include <fmtornt.hxx>
#include <cellatr.hxx>
#include <ddefld.hxx>
#include <swddetbl.hxx>
#include <ndole.hxx>
#include <xmloff/nmspmap.hxx>
#include <sfx2/linkmgr.hxx>
#include <unotbl.hxx>
#include "xmltexte.hxx"
#include "xmlexp.hxx"
#include <o3tl/any.hxx>
#include <o3tl/sorted_vector.hxx>
#include <textboxhelper.hxx>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::text;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::container;
using namespace ::xmloff::token;
using table::XCell;
using std::vector;
using std::advance;


class SwXMLTableColumn_Impl : public SwWriteTableCol
{
    OUString    sStyleName;
    sal_uInt32  nRelWidth;

public:

    explicit SwXMLTableColumn_Impl(sal_uInt32 nPosition)
        : SwWriteTableCol(nPosition)
        , nRelWidth(0)
    {};

    void SetStyleName( const OUString& rName ) { sStyleName = rName; }
    const OUString& GetStyleName() const { return sStyleName; }

    void SetRelWidth( sal_uInt32 nSet ) { nRelWidth = nSet; }
    sal_uInt32 GetRelWidth() const { return nRelWidth; }
};

struct SwXMLTableColumnCmpWidth_Impl
{
    bool operator()( SwXMLTableColumn_Impl* const& lhs, SwXMLTableColumn_Impl* const& rhs ) const
    {
        sal_Int32 n = static_cast<sal_Int32>(lhs->GetWidthOpt()) - static_cast<sal_Int32>(rhs->GetWidthOpt());
        if( !n )
            n = static_cast<sal_Int32>(lhs->GetRelWidth()) - static_cast<sal_Int32>(rhs->GetRelWidth());
        return n < 0;
    }
};

class SwXMLTableColumns_Impl : public o3tl::sorted_vector<std::unique_ptr<SwXMLTableColumn_Impl>, o3tl::less_uniqueptr_to<SwXMLTableColumn_Impl> > {
};

class SwXMLTableColumnsSortByWidth_Impl : public o3tl::sorted_vector<SwXMLTableColumn_Impl*, SwXMLTableColumnCmpWidth_Impl> {};

class SwXMLTableLines_Impl
{
    SwXMLTableColumns_Impl  aCols;
    const SwTableLines      *pLines;
    sal_uInt32              nWidth;

public:

    explicit SwXMLTableLines_Impl( const SwTableLines& rLines );

    sal_uInt32 GetWidth() const { return nWidth; }
    const SwTableLines *GetLines() const { return pLines; }

    const SwXMLTableColumns_Impl& GetColumns() const { return aCols; }
};

SwXMLTableLines_Impl::SwXMLTableLines_Impl( const SwTableLines& rLines ) :
    pLines( &rLines ),
    nWidth( 0 )
{
#if OSL_DEBUG_LEVEL > 0
    sal_uInt32 nEndCPos = 0U;
#endif
    const size_t nLines = rLines.size();
    for( size_t nLine=0U; nLine<nLines; ++nLine )
    {
        const SwTableLine *pLine = rLines[nLine];
        const SwTableBoxes& rBoxes = pLine->GetTabBoxes();
        const size_t nBoxes = rBoxes.size();

        sal_uInt32 nCPos = 0U;
        for( size_t nBox=0U; nBox<nBoxes; ++nBox )
        {
            const SwTableBox *pBox = rBoxes[nBox];

            if( nBox < nBoxes-1U || nWidth==0 )
            {
                nCPos = nCPos + SwWriteTable::GetBoxWidth( pBox );
                std::unique_ptr<SwXMLTableColumn_Impl> pCol(
                    new SwXMLTableColumn_Impl( nCPos ));

                aCols.insert( std::move(pCol) );

                if( nBox==nBoxes-1U )
                {
                    OSL_ENSURE( nLine==0U && nWidth==0,
                            "parent width will be lost" );
                    nWidth = nCPos;
                }
            }
            else
            {
#if OSL_DEBUG_LEVEL > 0
                sal_uInt32 nCheckPos =
                    nCPos + SwWriteTable::GetBoxWidth( pBox );
                if( !nEndCPos )
                {
                    nEndCPos = nCheckPos;
                }
#endif
                nCPos = nWidth;
#if OSL_DEBUG_LEVEL > 0
                SwXMLTableColumn_Impl aCol( nWidth );
                OSL_ENSURE( aCols.find(&aCol) != aCols.end(), "couldn't find last column" );
                OSL_ENSURE( SwXMLTableColumn_Impl(nCheckPos) ==
                                            SwXMLTableColumn_Impl(nCPos),
                        "rows have different total widths" );
#endif
            }
        }
    }
}

typedef vector< SwFrameFormat* > SwXMLFrameFormats_Impl;

class SwXMLTableFrameFormatsSort_Impl
{
private:
    SwXMLFrameFormats_Impl aFormatList;
public:
    bool AddRow( SwFrameFormat& rFrameFormat, const OUString& rNamePrefix, sal_uInt32 nLine );
    bool AddCell( SwFrameFormat& rFrameFormat, const OUString& rNamePrefix,
                  sal_uInt32 nCol, sal_uInt32 nRow, bool bTop );
};

bool SwXMLTableFrameFormatsSort_Impl::AddRow( SwFrameFormat& rFrameFormat,
                                         const OUString& rNamePrefix,
                                            sal_uInt32 nLine )
{
    const SwFormatFrameSize *pFrameSize = nullptr;
    const SwFormatRowSplit* pRowSplit = nullptr;
    const SvxBrushItem *pBrush = nullptr;

    const SfxItemSet& rItemSet = rFrameFormat.GetAttrSet();
    const SfxPoolItem *pItem;
    if( SfxItemState::SET == rItemSet.GetItemState( RES_FRM_SIZE, false, &pItem ) )
        pFrameSize = static_cast<const SwFormatFrameSize *>(pItem);

    if( SfxItemState::SET == rItemSet.GetItemState( RES_ROW_SPLIT, false, &pItem ) )
        pRowSplit = static_cast<const SwFormatRowSplit *>(pItem);

    if( SfxItemState::SET == rItemSet.GetItemState( RES_BACKGROUND, false, &pItem ) )
        pBrush = static_cast<const SvxBrushItem *>(pItem);

    // empty styles have not to be exported
    if( !pFrameSize && !pBrush && !pRowSplit )
        return false;

    // order is: -/brush, size/-, size/brush
    bool bInsert = true;
    SwXMLFrameFormats_Impl::iterator i;
    for( i = aFormatList.begin(); i < aFormatList.end(); ++i )
    {
        const SwFormatFrameSize *pTestFrameSize = nullptr;
        const SwFormatRowSplit* pTestRowSplit = nullptr;
        const SvxBrushItem *pTestBrush = nullptr;
        const SwFrameFormat *pTestFormat = *i;
        const SfxItemSet& rTestSet = pTestFormat->GetAttrSet();
        if( SfxItemState::SET == rTestSet.GetItemState( RES_FRM_SIZE, false,
                                                  &pItem ) )
        {
            if( !pFrameSize )
                break;

            pTestFrameSize = static_cast<const SwFormatFrameSize *>(pItem);
        }
        else
        {
            if( pFrameSize )
                continue;
        }

        if( SfxItemState::SET == rTestSet.GetItemState( RES_BACKGROUND, false,
                                                  &pItem ) )
        {
            if( !pBrush )
                break;

            pTestBrush = static_cast<const SvxBrushItem *>(pItem);
        }
        else
        {
            if( pBrush )
                continue;
        }

        if( SfxItemState::SET == rTestSet.GetItemState( RES_ROW_SPLIT, false,
                                                  &pItem ) )
        {
            if( !pRowSplit )
                break;

            pTestRowSplit = static_cast<const SwFormatRowSplit *>(pItem);
        }
        else
        {
            if( pRowSplit )
                continue;
        }

        if( pFrameSize &&
            ( pFrameSize->GetHeightSizeType() != pTestFrameSize->GetHeightSizeType() ||
              pFrameSize->GetHeight() != pTestFrameSize->GetHeight() ) )
            continue;

        if( pBrush && (*pBrush != *pTestBrush) )
            continue;

        if( pRowSplit && (!pRowSplit->GetValue() != !pTestRowSplit->GetValue()) )
            continue;

        // found!
        rFrameFormat.SetName( pTestFormat->GetName() );
        bInsert = false;
        break;
    }

    if( bInsert )
    {
        rFrameFormat.SetName( rNamePrefix + "." + OUString::number(nLine+1) );
        if ( i != aFormatList.end() ) ++i;
        aFormatList.insert( i, &rFrameFormat );
    }

    return bInsert;
}

static OUString lcl_xmltble_appendBoxPrefix(const OUString& rNamePrefix,
                                  sal_uInt32 nCol, sal_uInt32 nRow, bool bTop )
{
    if( bTop )
    {
        OUString sTmp;
        sw_GetTableBoxColStr( static_cast<sal_uInt16>(nCol), sTmp );
        return rNamePrefix + "." + sTmp + OUString::number(nRow + 1);
    }
    return rNamePrefix
        + "." + OUString::number(nCol + 1)
        + "." + OUString::number(nRow + 1);
}

bool SwXMLTableFrameFormatsSort_Impl::AddCell( SwFrameFormat& rFrameFormat,
                                         const OUString& rNamePrefix,
                                            sal_uInt32 nCol, sal_uInt32 nRow, bool bTop )
{
    const SwFormatVertOrient *pVertOrient = nullptr;
    const SvxBrushItem *pBrush = nullptr;
    const SvxBoxItem *pBox = nullptr;
    const SwTableBoxNumFormat *pNumFormat = nullptr;
    const SvxFrameDirectionItem *pFrameDir = nullptr;
    const SvXMLAttrContainerItem *pAttCnt = nullptr;

    const SfxItemSet& rItemSet = rFrameFormat.GetAttrSet();
    const SfxPoolItem *pItem;
    if( SfxItemState::SET == rItemSet.GetItemState( RES_VERT_ORIENT, false,
                                               &pItem ) )
        pVertOrient = static_cast<const SwFormatVertOrient *>(pItem);

    if( SfxItemState::SET == rItemSet.GetItemState( RES_BACKGROUND, false, &pItem ) )
        pBrush = static_cast<const SvxBrushItem *>(pItem);

    if( SfxItemState::SET == rItemSet.GetItemState( RES_BOX, false, &pItem ) )
        pBox = static_cast<const SvxBoxItem *>(pItem);

    if ( SfxItemState::SET == rItemSet.GetItemState( RES_BOXATR_FORMAT,
                                                false, &pItem ) )
        pNumFormat = static_cast<const SwTableBoxNumFormat *>(pItem);
    if ( SfxItemState::SET == rItemSet.GetItemState( RES_FRAMEDIR,
                                                false, &pItem ) )
        pFrameDir = static_cast<const SvxFrameDirectionItem *>(pItem);
    if ( SfxItemState::SET == rItemSet.GetItemState( RES_UNKNOWNATR_CONTAINER,
                                                false, &pItem ) )
        pAttCnt = static_cast<const SvXMLAttrContainerItem *>(pItem);

    // empty styles have not to be exported
    if( !pVertOrient && !pBrush && !pBox && !pNumFormat && !pFrameDir && !pAttCnt )
        return false;

    // order is: -/-/-/num,
    //           -/-/box/-, -/-/box/num,
    //           -/brush/-/-, -/brush/-/num, -/brush/box/-, -/brush/box/num,
    //           vert/-/-/-, vert/-/-/num, vert/-/box/-, ver/-/box/num,
    //           vert/brush/-/-, vert/brush/-/num, vert/brush/box/-,
    //           vert/brush/box/num
    bool bInsert = true;
    SwXMLFrameFormats_Impl::iterator i;
    for( i = aFormatList.begin(); i < aFormatList.end(); ++i )
    {
        const SwFormatVertOrient *pTestVertOrient = nullptr;
        const SvxBrushItem *pTestBrush = nullptr;
        const SvxBoxItem *pTestBox = nullptr;
        const SwTableBoxNumFormat *pTestNumFormat = nullptr;
        const SvxFrameDirectionItem *pTestFrameDir = nullptr;
        const SvXMLAttrContainerItem *pTestAttCnt = nullptr;
        const SwFrameFormat* pTestFormat = *i;
        const SfxItemSet& rTestSet = pTestFormat->GetAttrSet();
        if( SfxItemState::SET == rTestSet.GetItemState( RES_VERT_ORIENT, false,
                                                  &pItem ) )
        {
            if( !pVertOrient )
                break;

            pTestVertOrient = static_cast<const SwFormatVertOrient *>(pItem);
        }
        else
        {
            if( pVertOrient )
                continue;
        }

        if( SfxItemState::SET == rTestSet.GetItemState( RES_BACKGROUND, false,
                                                  &pItem ) )
        {
            if( !pBrush )
                break;

            pTestBrush = static_cast<const SvxBrushItem *>(pItem);
        }
        else
        {
            if( pBrush )
                continue;
        }

        if( SfxItemState::SET == rTestSet.GetItemState( RES_BOX, false, &pItem ) )
        {
            if( !pBox )
                break;

            pTestBox = static_cast<const SvxBoxItem *>(pItem);
        }
        else
        {
            if( pBox )
                continue;
        }

        if ( SfxItemState::SET == rTestSet.GetItemState( RES_BOXATR_FORMAT,
                                                false, &pItem ) )
        {
            if( !pNumFormat )
                break;

            pTestNumFormat = static_cast<const SwTableBoxNumFormat *>(pItem);
        }
        else
        {
            if( pNumFormat )
                continue;

        }

        if ( SfxItemState::SET == rTestSet.GetItemState( RES_FRAMEDIR,
                                                false, &pItem ) )
        {
            if( !pFrameDir )
                break;

            pTestFrameDir = static_cast<const SvxFrameDirectionItem *>(pItem);
        }
        else
        {
            if( pFrameDir )
                continue;

        }

        if ( SfxItemState::SET == rTestSet.GetItemState( RES_UNKNOWNATR_CONTAINER,
                                                false, &pItem ) )
        {
             if( !pAttCnt )
                 break;

             pTestAttCnt = static_cast<const SvXMLAttrContainerItem *>(pItem);
        }
        else
        {
             if ( pAttCnt )
                 continue;

        }

        if( pVertOrient &&
            pVertOrient->GetVertOrient() != pTestVertOrient->GetVertOrient() )
            continue;

        if( pBrush && ( *pBrush != *pTestBrush ) )
            continue;

        if( pBox && ( *pBox != *pTestBox ) )
            continue;

        if( pNumFormat && pNumFormat->GetValue() != pTestNumFormat->GetValue() )
            continue;

        if( pFrameDir && pFrameDir->GetValue() != pTestFrameDir->GetValue() )
            continue;

        if( pAttCnt && ( *pAttCnt != *pTestAttCnt ) )
            continue;

        // found!
        rFrameFormat.SetName( pTestFormat->GetName() );
        bInsert = false;
        break;
    }

    if( bInsert )
    {
        rFrameFormat.SetName( lcl_xmltble_appendBoxPrefix( rNamePrefix, nCol, nRow, bTop ) );
        if ( i != aFormatList.end() ) ++i;
        aFormatList.insert( i, &rFrameFormat );
    }

    return bInsert;
}

class SwXMLTableInfo_Impl
{
    const SwTable *m_pTable;
    Reference<XTextSection> m_xBaseSection;
    bool m_bBaseSectionValid;
    sal_uInt32 const m_nPrefix;

public:

    inline SwXMLTableInfo_Impl( const SwTable *pTable, sal_uInt16 nPrefix );

    const SwTable *GetTable() const { return m_pTable; }
    const SwFrameFormat *GetTableFormat() const { return m_pTable->GetFrameFormat(); }

    bool IsBaseSectionValid() const { return m_bBaseSectionValid; }
    const Reference<XTextSection>& GetBaseSection() const { return m_xBaseSection; }
    inline void SetBaseSection( const Reference < XTextSection >& rBase );
    /// The namespace (table or loext) that should be used for the elements.
    sal_uInt16 GetPrefix() const { return m_nPrefix; }
};

inline SwXMLTableInfo_Impl::SwXMLTableInfo_Impl(const SwTable *pTable, sal_uInt16 nPrefix) :
    m_pTable(pTable),
    m_bBaseSectionValid(false),
    m_nPrefix(nPrefix)
{
}

inline void SwXMLTableInfo_Impl::SetBaseSection(
        const Reference < XTextSection >& rBaseSection )
{
    m_xBaseSection = rBaseSection;
    m_bBaseSectionValid = true;
}

void SwXMLExport::ExportTableColumnStyle( const SwXMLTableColumn_Impl& rCol )
{
    // <style:style ...>
    CheckAttrList();

    // style:name="..."
    bool bEncoded = false;
    AddAttribute( XML_NAMESPACE_STYLE, XML_NAME,
                    EncodeStyleName( rCol.GetStyleName(), &bEncoded ) );
    if( bEncoded )
        AddAttribute( XML_NAMESPACE_STYLE, XML_DISPLAY_NAME, rCol.GetStyleName() );

    // style:family="table-column"
    AddAttribute( XML_NAMESPACE_STYLE, XML_FAMILY, XML_TABLE_COLUMN );

    {
        SvXMLElementExport aElem( *this, XML_NAMESPACE_STYLE, XML_STYLE, true,
                                  true );
        OUStringBuffer sValue;
        if( rCol.GetWidthOpt() )
        {
            GetTwipUnitConverter().convertMeasureToXML( sValue,
                    rCol.GetWidthOpt() );
            AddAttribute( XML_NAMESPACE_STYLE, XML_COLUMN_WIDTH,
                          sValue.makeStringAndClear() );
        }
        if( rCol.GetRelWidth() )
        {
            sValue.append( static_cast<sal_Int32>(rCol.GetRelWidth()) );
            sValue.append( '*' );
            AddAttribute( XML_NAMESPACE_STYLE, XML_REL_COLUMN_WIDTH,
                          sValue.makeStringAndClear() );
        }

        {
            SvXMLElementExport aElemExport( *this, XML_NAMESPACE_STYLE,
                                      XML_TABLE_COLUMN_PROPERTIES,
                                      true, true );
        }
    }
}

void SwXMLExport::ExportTableLinesAutoStyles( const SwTableLines& rLines,
                                    sal_uInt32 nAbsWidth, sal_uInt32 nBaseWidth,
                                    const OUString& rNamePrefix,
                                    SwXMLTableColumnsSortByWidth_Impl& rExpCols,
                                    SwXMLTableFrameFormatsSort_Impl& rExpRows,
                                    SwXMLTableFrameFormatsSort_Impl& rExpCells,
                                    SwXMLTableInfo_Impl& rTableInfo,
                                    bool bTop )
{
    // pass 1: calculate columns
    SwXMLTableLines_Impl *pLines = new SwXMLTableLines_Impl( rLines );
    if( !m_pTableLines )
        m_pTableLines.reset(new SwXMLTableLinesCache_Impl);

    m_pTableLines->push_back( pLines );

    // pass 2: export column styles
    {
        const SwXMLTableColumns_Impl& rCols = pLines->GetColumns();
        sal_uInt32 nCPos = 0U;
        const size_t nColumns = rCols.size();
        for( size_t nColumn=0U; nColumn<nColumns; ++nColumn )
        {
            SwXMLTableColumn_Impl *pColumn = rCols[nColumn].get();

            sal_uInt32 nOldCPos = nCPos;
            nCPos = pColumn->GetPos();

            sal_uInt32 nWidth = nCPos - nOldCPos;

            // If a base width is given, the table has either an automatic
            // or margin alignment, or an percentage width. In either case,
            // relative widths should be exported.
            if( nBaseWidth )
            {
                pColumn->SetRelWidth( nWidth );
            }

            // If an absolute width is given, the table either has a fixed
            // width, or the current width is known from the layout. In the
            // later case, a base width is set in addition and must be used
            // to "absolutize" the relative column width.
            if( nAbsWidth )
            {
                sal_uInt32 nColAbsWidth = nWidth;
                if( nBaseWidth )
                {
                    nColAbsWidth *= nAbsWidth;
                    nColAbsWidth += (nBaseWidth/2UL);
                    nColAbsWidth /= nBaseWidth;
                }
                pColumn->SetWidthOpt( nColAbsWidth, false );
            }

            SwXMLTableColumnsSortByWidth_Impl::const_iterator it = rExpCols.find( pColumn );
            if( it != rExpCols.end() )
            {
                pColumn->SetStyleName( (*it)->GetStyleName() );
            }
            else
            {
                if( bTop )
                {
                    OUString sTmp;
                    sw_GetTableBoxColStr( nColumn, sTmp );
                    pColumn->SetStyleName( rNamePrefix + "." + sTmp );
                }
                else
                {
                    pColumn->SetStyleName( rNamePrefix + "." + OUString::number(nColumn + 1U) );
                }
                ExportTableColumnStyle( *pColumn );
                rExpCols.insert( pColumn );
            }
        }
    }

    // pass 3: export line/rows
    const size_t nLines = rLines.size();
    for( size_t nLine=0U; nLine<nLines; ++nLine )
    {
        SwTableLine *pLine = rLines[nLine];

        SwFrameFormat *pFrameFormat = pLine->GetFrameFormat();
        if( rExpRows.AddRow( *pFrameFormat, rNamePrefix, nLine ) )
            ExportFormat( *pFrameFormat, XML_TABLE_ROW );

        const SwTableBoxes& rBoxes = pLine->GetTabBoxes();
        const size_t nBoxes = rBoxes.size();

        sal_uInt32 nCPos = 0U;
        size_t nCol = 0U;
        for( size_t nBox=0U; nBox<nBoxes; nBox++ )
        {
            SwTableBox *pBox = rBoxes[nBox];

            if( nBox < nBoxes-1U )
                nCPos = nCPos + SwWriteTable::GetBoxWidth( pBox );
            else
                nCPos = pLines->GetWidth();

            // and their index
            const size_t nOldCol = nCol;
            SwXMLTableColumn_Impl aCol( nCPos );
            SwXMLTableColumns_Impl::const_iterator it = pLines->GetColumns().find( &aCol );
            OSL_ENSURE( it != pLines->GetColumns().end(), "couldn't find column" );
            nCol = it - pLines->GetColumns().begin();

            const SwStartNode *pBoxSttNd = pBox->GetSttNd();
            if( pBoxSttNd )
            {
                SwFrameFormat *pFrameFormat2 = pBox->GetFrameFormat();
                if( rExpCells.AddCell( *pFrameFormat2, rNamePrefix, nOldCol, nLine,
                                       bTop) )
                    ExportFormat( *pFrameFormat2, XML_TABLE_CELL );

                Reference < XCell > xCell = SwXCell::CreateXCell(
                                                const_cast<SwFrameFormat *>(rTableInfo.GetTableFormat()),
                                                  pBox,
                                                 const_cast<SwTable *>(rTableInfo.GetTable()) );
                if (xCell.is())
                {
                    Reference < XText > xText( xCell, UNO_QUERY );
                    if( !rTableInfo.IsBaseSectionValid() )
                    {
                        Reference<XPropertySet> xCellPropertySet( xCell,
                                                                 UNO_QUERY );
                        Any aAny = xCellPropertySet->getPropertyValue("TextSection");
                        Reference < XTextSection > xTextSection;
                        aAny >>= xTextSection;
                        rTableInfo.SetBaseSection( xTextSection );
                    }

                    const bool bExportContent = bool(getExportFlags() & SvXMLExportFlags::CONTENT );
                    if ( !bExportContent )
                    {
                        // AUTOSTYLES - not needed anymore if we are currently exporting content.xml
                        GetTextParagraphExport()->collectTextAutoStyles(
                            xText, rTableInfo.GetBaseSection(), IsShowProgress() );
                    }
                }
                else {
                    OSL_FAIL("here should be a XCell");
                }
            }
            else
            {
                ExportTableLinesAutoStyles( pBox->GetTabLines(),
                                            nAbsWidth, nBaseWidth,
                                            lcl_xmltble_appendBoxPrefix( rNamePrefix,
                                                                         nOldCol, nLine, bTop ),
                                            rExpCols, rExpRows, rExpCells,
                                            rTableInfo );
            }

            nCol++;
        }
    }
}

void SwXMLExport::ExportTableAutoStyles( const SwTableNode& rTableNd )
{
    const SwTable& rTable = rTableNd.GetTable();
    const SwFrameFormat *pTableFormat = rTable.GetFrameFormat();

    if( !pTableFormat )
        return;

    sal_Int16 eTabHoriOri = pTableFormat->GetHoriOrient().GetHoriOrient();
    const SwFormatFrameSize& rFrameSize = pTableFormat->GetFrameSize();

    sal_uInt32 nAbsWidth = rFrameSize.GetSize().Width();
    sal_uInt32 nBaseWidth = 0;
    sal_Int8 nPrcWidth = rFrameSize.GetWidthPercent();

    bool bFixAbsWidth = nPrcWidth != 0 || /*text::*/HoriOrientation::NONE == eTabHoriOri
                                       || /*text::*/HoriOrientation::FULL == eTabHoriOri;
    if( bFixAbsWidth )
    {
        nBaseWidth = nAbsWidth;
        nAbsWidth = pTableFormat->FindLayoutRect(true).Width();
        if( !nAbsWidth )
        {
            // TODO?
        }
    }
    ExportTableFormat( *pTableFormat, nAbsWidth );

    SwXMLTableColumnsSortByWidth_Impl aExpCols;
    SwXMLTableFrameFormatsSort_Impl aExpRows;
    SwXMLTableFrameFormatsSort_Impl aExpCells;
    SwXMLTableInfo_Impl aTableInfo( &rTable, XML_NAMESPACE_TABLE );
    ExportTableLinesAutoStyles( rTable.GetTabLines(), nAbsWidth, nBaseWidth,
                                pTableFormat->GetName(), aExpCols, aExpRows, aExpCells,
                                aTableInfo, true);

}

void SwXMLExport::ExportTableBox( const SwTableBox& rBox,
                                  sal_uInt32 nColSpan,
                                  sal_uInt32 nRowSpan,
                                  SwXMLTableInfo_Impl& rTableInfo )
{
    const SwStartNode *pBoxSttNd = rBox.GetSttNd();
    if( pBoxSttNd )
    {
        const SwFrameFormat *pFrameFormat = rBox.GetFrameFormat();
        if( pFrameFormat )
        {
            const OUString& sName = pFrameFormat->GetName();
            if( !sName.isEmpty() )
            {
                AddAttribute( XML_NAMESPACE_TABLE, XML_STYLE_NAME, EncodeStyleName(sName) );
            }
        }
    }

    if( nRowSpan != 1 )
    {
        AddAttribute( XML_NAMESPACE_TABLE, XML_NUMBER_ROWS_SPANNED,
                      OUString::number(nRowSpan) );
    }

    if( nColSpan != 1 )
    {
        AddAttribute( XML_NAMESPACE_TABLE, XML_NUMBER_COLUMNS_SPANNED,
                      OUString::number(nColSpan) );
    }

    {
        if( pBoxSttNd )
        {
            // start node -> normal cell
            // get cell range for table
            Reference<XCell> xCell = SwXCell::CreateXCell( const_cast<SwFrameFormat *>(rTableInfo.GetTableFormat()),
                                                            const_cast<SwTableBox *>(&rBox),
                                                            const_cast<SwTable *>(rTableInfo.GetTable()) );

            if (xCell.is())
            {
                Reference<XText> xText( xCell, UNO_QUERY );

                // get formula (and protection)
                const OUString sCellFormula = xCell->getFormula();

                // if this cell has a formula, export it
                //     (with value and number format)
                if (!sCellFormula.isEmpty())
                {
                    const OUString sQValue =
                        GetNamespaceMap().GetQNameByKey(
                                XML_NAMESPACE_OOOW, sCellFormula, false );
                    // formula
                    AddAttribute(XML_NAMESPACE_TABLE, XML_FORMULA, sQValue );
                }

                // value and format (if NumberFormat != -1)
                Reference<XPropertySet> xCellPropertySet(xCell,
                                                        UNO_QUERY);
                if (xCellPropertySet.is())
                {
                    sal_Int32 nNumberFormat = 0;
                    Any aAny = xCellPropertySet->getPropertyValue("NumberFormat");
                    aAny >>= nNumberFormat;

                    if (static_cast<sal_Int32>(getSwDefaultTextFormat()) == nNumberFormat)
                    {
                        // text format
                        AddAttribute( XML_NAMESPACE_OFFICE,
                                    XML_VALUE_TYPE, XML_STRING );
                    }
                    else if ( (-1 != nNumberFormat) && !xText->getString().isEmpty() )
                    {
                        // number format key:
                        // (export values only if cell contains text;)
                        XMLNumberFormatAttributesExportHelper::
                            SetNumberFormatAttributes(
                                *this, nNumberFormat, xCell->getValue() );
                    }
                    // else: invalid key; ignore

                    // cell protection
                    aAny = xCellPropertySet->getPropertyValue("IsProtected");
                    if (*o3tl::doAccess<bool>(aAny))
                    {
                        AddAttribute( XML_NAMESPACE_TABLE, XML_PROTECTED,
                                        XML_TRUE );
                    }

                    if( !rTableInfo.IsBaseSectionValid() )
                    {
                        aAny = xCellPropertySet->getPropertyValue("TextSection");
                        Reference < XTextSection > xTextSection;
                        aAny >>= xTextSection;
                        rTableInfo.SetBaseSection( xTextSection );
                    }
                }

                // export cell element
                SvXMLElementExport aElem( *this, rTableInfo.GetPrefix(),
                                        XML_TABLE_CELL, true, true );

                // export cell content
                GetTextParagraphExport()->exportText( xText,
                                                    rTableInfo.GetBaseSection(),
                                                    IsShowProgress() );
            }
            else
            {
                OSL_FAIL("here should be a XCell");
                ClearAttrList();
            }
        }
        else
        {
            // no start node -> merged cells: export subtable in cell
            SvXMLElementExport aElem( *this, XML_NAMESPACE_TABLE,
                                      XML_TABLE_CELL, true, true );
            {
                AddAttribute( XML_NAMESPACE_TABLE, XML_IS_SUB_TABLE,
                                GetXMLToken( XML_TRUE ) );

                SvXMLElementExport aElemExport( *this, XML_NAMESPACE_TABLE,
                                          XML_TABLE, true, true );
                ExportTableLines( rBox.GetTabLines(), rTableInfo );
            }
        }
    }
}

void SwXMLExport::ExportTableLine( const SwTableLine& rLine,
                                   const SwXMLTableLines_Impl& rLines,
                                   SwXMLTableInfo_Impl& rTableInfo )
{
    if( rLine.hasSoftPageBreak() )
    {
        SvXMLElementExport aElem( *this, XML_NAMESPACE_TEXT,
                                  XML_SOFT_PAGE_BREAK, true, true );
    }
    const SwFrameFormat *pFrameFormat = rLine.GetFrameFormat();
    if( pFrameFormat )
    {
        const OUString& sName = pFrameFormat->GetName();
        if( !sName.isEmpty() )
        {
            AddAttribute( XML_NAMESPACE_TABLE, XML_STYLE_NAME, EncodeStyleName(sName) );
        }
    }

    {
        SvXMLElementExport aElem( *this, rTableInfo.GetPrefix(), XML_TABLE_ROW, true, true );
        const SwTableBoxes& rBoxes = rLine.GetTabBoxes();
        const size_t nBoxes = rBoxes.size();

        sal_uInt32 nCPos = 0U;
        size_t nCol = 0U;
        for( size_t nBox=0U; nBox<nBoxes; ++nBox )
        {
            const SwTableBox *pBox = rBoxes[nBox];

            // NEW TABLES
            const long nRowSpan = pBox->getRowSpan();
            if( nRowSpan < 1 )
            {
                SvXMLElementExport aElem2( *this, rTableInfo.GetPrefix(),
                                          XML_COVERED_TABLE_CELL, true,
                                          false );
            }

            if( nBox < nBoxes-1U )
                nCPos = nCPos + SwWriteTable::GetBoxWidth( pBox );
            else
                nCPos = rLines.GetWidth();

            // and their index
            const size_t nOldCol = nCol;
            SwXMLTableColumn_Impl aCol( nCPos );
            SwXMLTableColumns_Impl::const_iterator it = rLines.GetColumns().find( &aCol );
            OSL_ENSURE( it != rLines.GetColumns().end(), "couldn't find column" );
            nCol = it - rLines.GetColumns().begin();

            // #i95726# - Some fault tolerance, if table is somehow corrupted.
            if ( nCol < nOldCol )
            {
                OSL_FAIL( "table and/or table information seems to be corrupted." );
                // NOTE: nOldCol is not necessarily a valid index into
                // GetColumns(), but that doesn't matter here
                nCol = nOldCol;
            }

            const sal_uInt32 nColSpan = nCol - nOldCol + 1U;

            if ( nRowSpan >= 1 )
                ExportTableBox( *pBox, nColSpan, static_cast< sal_uInt32 >(nRowSpan), rTableInfo );

            for( size_t i=nOldCol; i<nCol; ++i )
            {
                SvXMLElementExport aElemExport( *this, rTableInfo.GetPrefix(),
                                          XML_COVERED_TABLE_CELL, true,
                                          false );
            }

            nCol++;
        }
    }
}

void SwXMLExport::ExportTableLines( const SwTableLines& rLines,
                                    SwXMLTableInfo_Impl& rTableInfo,
                                    sal_uInt32 nHeaderRows )
{
    OSL_ENSURE( m_pTableLines && !m_pTableLines->empty(),
            "SwXMLExport::ExportTableLines: table columns infos missing" );
    if( !m_pTableLines || m_pTableLines->empty() )
        return;

    SwXMLTableLines_Impl* pLines = nullptr;
    size_t nInfoPos;
    for( nInfoPos=0; nInfoPos < m_pTableLines->size(); nInfoPos++ )
    {
        if( m_pTableLines->at( nInfoPos )->GetLines() == &rLines )
        {
            pLines = m_pTableLines->at( nInfoPos );
            break;
        }
    }
    OSL_ENSURE( pLines,
            "SwXMLExport::ExportTableLines: table columns info missing" );
    OSL_ENSURE( 0==nInfoPos,
            "SwXMLExport::ExportTableLines: table columns infos are unsorted" );
    if( !pLines )
        return;

    SwXMLTableLinesCache_Impl::iterator it = m_pTableLines->begin();
    advance( it, nInfoPos );
    m_pTableLines->erase( it );

    if( m_pTableLines->empty() )
        m_pTableLines.reset();

    // pass 2: export columns
    const SwXMLTableColumns_Impl& rCols = pLines->GetColumns();
    size_t nColumn = 0U;
    const size_t nColumns = rCols.size();
    sal_Int32 nColRep = 1;
    SwXMLTableColumn_Impl *pColumn = (nColumns > 0) ? rCols.front().get() : nullptr;
    while( pColumn )
    {
        nColumn++;
        SwXMLTableColumn_Impl *pNextColumn =
            (nColumn < nColumns) ? rCols[nColumn].get() : nullptr;
        if( pNextColumn &&
            pNextColumn->GetStyleName() == pColumn->GetStyleName() )
        {
            nColRep++;
        }
        else
        {
            AddAttribute( XML_NAMESPACE_TABLE, XML_STYLE_NAME,
                          EncodeStyleName(pColumn->GetStyleName()) );

            if( nColRep > 1 )
            {
                AddAttribute( XML_NAMESPACE_TABLE, XML_NUMBER_COLUMNS_REPEATED,
                              OUString::number(nColRep) );
            }

            {
                SvXMLElementExport aElem( *this, rTableInfo.GetPrefix(), XML_TABLE_COLUMN, true, true );
            }

            nColRep = 1;
        }
        pColumn = pNextColumn;
    }

    // pass 3: export line/rows
    const size_t nLines = rLines.size();
    // export header rows, if present
    if( nHeaderRows > 0 )
    {
        SvXMLElementExport aElem( *this, XML_NAMESPACE_TABLE,
                                  XML_TABLE_HEADER_ROWS, true, true );

        OSL_ENSURE( nHeaderRows <= nLines, "more headers then lines?" );
        for( size_t nLine = 0U; nLine < nHeaderRows; ++nLine )
            ExportTableLine( *(rLines[nLine]), *pLines, rTableInfo );
    }
    // export remaining rows
    for( size_t nLine = nHeaderRows; nLine < nLines; ++nLine )
    {
        ExportTableLine( *(rLines[nLine]), *pLines, rTableInfo );
    }

    delete pLines;
}

static void lcl_xmltble_ClearName_Line( SwTableLine* pLine );

static void lcl_xmltble_ClearName_Box( SwTableBox* pBox )
{
    if( !pBox->GetSttNd() )
    {
        for( SwTableLine* pLine : pBox->GetTabLines() )
            lcl_xmltble_ClearName_Line( pLine );
    }
    else
    {
        SwFrameFormat *pFrameFormat = pBox->GetFrameFormat();
        if( pFrameFormat && !pFrameFormat->GetName().isEmpty() )
            pFrameFormat->SetName( OUString() );
    }
}

void lcl_xmltble_ClearName_Line( SwTableLine* pLine )
{
    for( SwTableBox* pBox : pLine->GetTabBoxes() )
        lcl_xmltble_ClearName_Box( pBox );
}

void SwXMLExport::ExportTable( const SwTableNode& rTableNd )
{
    const SwTable& rTable = rTableNd.GetTable();
    const SwFrameFormat *pTableFormat = rTable.GetFrameFormat();
    if( pTableFormat && !pTableFormat->GetName().isEmpty() )
    {
        AddAttribute( XML_NAMESPACE_TABLE, XML_NAME, pTableFormat->GetName() );
        AddAttribute( XML_NAMESPACE_TABLE, XML_STYLE_NAME,
                      EncodeStyleName( pTableFormat->GetName() ) );
    }

    // table:template-name=
    if (!rTable.GetTableStyleName().isEmpty())
        AddAttribute(XML_NAMESPACE_TABLE, XML_TEMPLATE_NAME, rTable.GetTableStyleName());

    sal_uInt16 nPrefix = XML_NAMESPACE_TABLE;
    if (const SwFrameFormat* pFlyFormat = rTableNd.GetFlyFormat())
    {
        if (SwTextBoxHelper::isTextBox(pFlyFormat, RES_FLYFRMFMT))
            nPrefix = XML_NAMESPACE_LO_EXT;
    }

    {
        SvXMLElementExport aElem( *this, nPrefix, XML_TABLE, true, true );

        // export DDE source (if this is a DDE table)
        if ( dynamic_cast<const SwDDETable*>( &rTable) !=  nullptr )
        {
            // get DDE Field Type (contains the DDE connection)
            const SwDDEFieldType* pDDEFieldType =
                static_cast<const SwDDETable&>(rTable).GetDDEFieldType();

            // connection name
            AddAttribute( XML_NAMESPACE_OFFICE, XML_NAME,
                          pDDEFieldType->GetName() );

            // DDE command
            const OUString& sCmd = pDDEFieldType->GetCmd();
            sal_Int32 nIdx{ 0 };
            AddAttribute( XML_NAMESPACE_OFFICE, XML_DDE_APPLICATION,
                          sCmd.getToken(0, sfx2::cTokenSeparator, nIdx) );
            AddAttribute( XML_NAMESPACE_OFFICE, XML_DDE_ITEM,
                          sCmd.getToken(0, sfx2::cTokenSeparator, nIdx) );
            AddAttribute( XML_NAMESPACE_OFFICE, XML_DDE_TOPIC,
                          sCmd.getToken(0, sfx2::cTokenSeparator, nIdx) );

            // auto update
            if (pDDEFieldType->GetType() == SfxLinkUpdateMode::ALWAYS)
            {
                AddAttribute( XML_NAMESPACE_OFFICE,
                              XML_AUTOMATIC_UPDATE, XML_TRUE );
            }

            // DDE source element (always empty)
            SvXMLElementExport aSource(*this, XML_NAMESPACE_OFFICE,
                                       XML_DDE_SOURCE, true, false);
        }

        SwXMLTableInfo_Impl aTableInfo( &rTable, nPrefix );
        ExportTableLines( rTable.GetTabLines(), aTableInfo, rTable.GetRowsToRepeat() );

        for( SwTableLine *pLine : const_cast<SwTable &>(rTable).GetTabLines() )
            lcl_xmltble_ClearName_Line( pLine );
    }
}

void SwXMLTextParagraphExport::exportTable(
        const Reference < XTextContent > & rTextContent,
        bool bAutoStyles, bool _bProgress )
{
    bool bOldShowProgress = static_cast<SwXMLExport&>(GetExport()).IsShowProgress();
    static_cast<SwXMLExport&>(GetExport()).SetShowProgress( _bProgress );

    Reference < XTextTable > xTextTable( rTextContent, UNO_QUERY );
    OSL_ENSURE( xTextTable.is(), "text table missing" );
    if( xTextTable.is() )
    {
        SwXTextTable *pXTable = nullptr;
        Reference<XUnoTunnel> xTableTunnel( rTextContent, UNO_QUERY);
        if( xTableTunnel.is() )
        {
            pXTable = reinterpret_cast< SwXTextTable * >(
                    sal::static_int_cast< sal_IntPtr >( xTableTunnel->getSomething( SwXTextTable::getUnoTunnelId() )));
            OSL_ENSURE( pXTable, "SwXTextTable missing" );
        }
        if( pXTable )
        {
            SwFrameFormat *const pFormat = pXTable->GetFrameFormat();
            OSL_ENSURE( pFormat, "table format missing" );
            const SwTable *pTable = SwTable::FindTable( pFormat );
            OSL_ENSURE( pTable, "table missing" );
            const SwTableNode *pTableNd = pTable->GetTableNode();
            OSL_ENSURE( pTableNd, "table node missing" );
            if( bAutoStyles )
            {
                SwNodeIndex aIdx( *pTableNd );
                // AUTOSTYLES: Optimization: Do not export table autostyle if
                // we are currently exporting the content.xml stuff and
                // the table is located in header/footer:
                // During the flat XML export (used e.g. by .sdw-export)
                // ALL flags are set at the same time.
                const bool bExportStyles = bool( GetExport().getExportFlags() & SvXMLExportFlags::STYLES );
                if ( bExportStyles || !pFormat->GetDoc()->IsInHeaderFooter( aIdx ) )
                    static_cast<SwXMLExport&>(GetExport()).ExportTableAutoStyles( *pTableNd );
            }
            else
            {
                static_cast<SwXMLExport&>(GetExport()).ExportTable( *pTableNd );
            }
        }
    }

    static_cast<SwXMLExport&>(GetExport()).SetShowProgress( bOldShowProgress );
}

void SwXMLExport::DeleteTableLines()
{
    if ( m_pTableLines )
    {
        for (SwXMLTableLines_Impl* p : *m_pTableLines)
            delete p;
        m_pTableLines->clear();
        m_pTableLines.reset();
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
