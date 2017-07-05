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

#include <memory>
#include "hintids.hxx"
#include <vcl/svapp.hxx>
#include <vcl/wrkwin.hxx>
#include <editeng/boxitem.hxx>
#include <editeng/brushitem.hxx>
#include <editeng/adjustitem.hxx>
#include <editeng/fhgtitem.hxx>
#include <editeng/ulspitem.hxx>
#include <editeng/lrspitem.hxx>
#include <editeng/formatbreakitem.hxx>
#include <editeng/spltitem.hxx>
#include <svtools/htmltokn.h>
#include <svtools/htmlkywd.hxx>
#include <svl/urihelper.hxx>
#include <o3tl/make_unique.hxx>

#include <fmtornt.hxx>
#include <frmfmt.hxx>
#include <fmtfsize.hxx>
#include <fmtsrnd.hxx>
#include <fmtpdsc.hxx>
#include <fmtcntnt.hxx>
#include <fmtanchr.hxx>
#include <fmtlsplt.hxx>
#include "frmatr.hxx"
#include "pam.hxx"
#include "doc.hxx"
#include <IDocumentLayoutAccess.hxx>
#include "ndtxt.hxx"
#include "shellio.hxx"
#include "poolfmt.hxx"
#include "swtable.hxx"
#include "cellatr.hxx"
#include "htmltbl.hxx"
#include "swtblfmt.hxx"
#include "htmlnum.hxx"
#include "swhtml.hxx"
#include "swcss1.hxx"
#include <numrule.hxx>

#define NETSCAPE_DFLT_BORDER 1
#define NETSCAPE_DFLT_CELLSPACING 2

using ::editeng::SvxBorderLine;
using namespace ::com::sun::star;

static HTMLOptionEnum<sal_Int16> aHTMLTableVAlignTable[] =
{
    { OOO_STRING_SVTOOLS_HTML_VA_top,     text::VertOrientation::NONE       },
    { OOO_STRING_SVTOOLS_HTML_VA_middle,  text::VertOrientation::CENTER     },
    { OOO_STRING_SVTOOLS_HTML_VA_bottom,  text::VertOrientation::BOTTOM     },
    { nullptr,                            0               }
};

// table tags options

struct HTMLTableOptions
{
    sal_uInt16 nCols;
    sal_uInt16 nWidth;
    sal_uInt16 nHeight;
    sal_uInt16 nCellPadding;
    sal_uInt16 nCellSpacing;
    sal_uInt16 nBorder;
    sal_uInt16 nHSpace;
    sal_uInt16 nVSpace;

    SvxAdjust eAdjust;
    sal_Int16 eVertOri;
    HTMLTableFrame eFrame;
    HTMLTableRules eRules;

    bool bPrcWidth : 1;
    bool bTableAdjust : 1;
    bool bBGColor : 1;

    Color aBorderColor;
    Color aBGColor;

    OUString aBGImage, aStyle, aId, aClass, aDir;

    HTMLTableOptions( const HTMLOptions& rOptions, SvxAdjust eParentAdjust );
};

class HTMLTableContext
{
    SwHTMLNumRuleInfo aNumRuleInfo; // Numbering valid before the table

    SwTableNode *pTableNd;              // table node
    SwFrameFormat *pFrameFormat;        // the Fly frame::Frame, containing the table
    std::unique_ptr<SwPosition> pPos;   // position behind the table

    size_t nContextStAttrMin;
    size_t nContextStMin;

    bool    bRestartPRE : 1;
    bool    bRestartXMP : 1;
    bool    bRestartListing : 1;

    HTMLTableContext(const HTMLTableContext&) = delete;
    HTMLTableContext& operator=(const HTMLTableContext&) = delete;

public:

    HTMLAttrTable aAttrTab;        // attributes

    HTMLTableContext( SwPosition *pPs, size_t nCntxtStMin,
                       size_t nCntxtStAttrMin ) :
        pTableNd( nullptr ),
        pFrameFormat( nullptr ),
        pPos( pPs ),
        nContextStAttrMin( nCntxtStAttrMin ),
        nContextStMin( nCntxtStMin ),
        bRestartPRE( false ),
        bRestartXMP( false ),
        bRestartListing( false )
    {
        memset( &aAttrTab, 0, sizeof( HTMLAttrTable ));
    }

    void SetNumInfo( const SwHTMLNumRuleInfo& rInf ) { aNumRuleInfo.Set(rInf); }
    const SwHTMLNumRuleInfo& GetNumInfo() const { return aNumRuleInfo; };

    void SavePREListingXMP( SwHTMLParser& rParser );
    void RestorePREListingXMP( SwHTMLParser& rParser );

    SwPosition *GetPos() const { return pPos.get(); }

    void SetTableNode( SwTableNode *pNd ) { pTableNd = pNd; }
    SwTableNode *GetTableNode() const { return pTableNd; }

    void SetFrameFormat( SwFrameFormat *pFormat ) { pFrameFormat = pFormat; }
    SwFrameFormat *GetFrameFormat() const { return pFrameFormat; }

    size_t GetContextStMin() const { return nContextStMin; }
    size_t GetContextStAttrMin() const { return nContextStAttrMin; }
};

// Cell content is a linked list with SwStartNodes and
// HTMLTables.

class HTMLTableCnts
{
    HTMLTableCnts *m_pNext;               // next content

    // Only one of the next two pointers must be set!
    const SwStartNode *m_pStartNode;      // a paragraph
    HTMLTable *m_pTable;                  // a table

    SwHTMLTableLayoutCnts* m_pLayoutInfo;

    bool m_bNoBreak;

    void InitCtor();

public:

    explicit HTMLTableCnts( const SwStartNode* pStNd );
    explicit HTMLTableCnts( HTMLTable* pTab );

    ~HTMLTableCnts();                   // only allowed in ~HTMLTableCell

    // Determine SwStartNode and HTMLTable respectively
    const SwStartNode *GetStartNode() const { return m_pStartNode; }
    const HTMLTable *GetTable() const { return m_pTable; }
    HTMLTable *GetTable() { return m_pTable; }

    // Add a new node at the end of the list
    void Add( HTMLTableCnts* pNewCnts );

    // Determine next node
    const HTMLTableCnts *Next() const { return m_pNext; }
    HTMLTableCnts *Next() { return m_pNext; }

    inline void SetTableBox( SwTableBox *pBox );

    void SetNoBreak() { m_bNoBreak = true; }

    SwHTMLTableLayoutCnts *CreateLayoutInfo();
};

// Cell of a HTML table
class HTMLTableCell
{
    // !!!ATTENTION!!!!! For each new pointer the SetProtected
    // method (and the dtor) has to be executed.
    HTMLTableCnts *pContents;       // cell content
    SvxBrushItem *pBGBrush;         // cell background
    // !!!ATTENTION!!!!!
    std::shared_ptr<SvxBoxItem> m_pBoxItem;

    sal_uInt32 nNumFormat;
    sal_uInt16 nRowSpan;                // cell ROWSPAN
    sal_uInt16 nColSpan;                // cell COLSPAN
    sal_uInt16 nWidth;                  // cell WIDTH
    double nValue;
    sal_Int16 eVertOri;         // vertical alignment of the cell
    bool bProtected : 1;            // cell must not filled
    bool bRelWidth : 1;             // nWidth is given in %
    bool bHasNumFormat : 1;
    bool bHasValue : 1;
    bool bNoWrap : 1;
    bool mbCovered : 1;

public:

    HTMLTableCell();                // new cells always empty

    ~HTMLTableCell();               // only allowed in ~HTMLTableRow

    // Fill a not empty cell
    void Set( HTMLTableCnts *pCnts, sal_uInt16 nRSpan, sal_uInt16 nCSpan,
              sal_Int16 eVertOri, SvxBrushItem *pBGBrush,
              std::shared_ptr<SvxBoxItem> const& rBoxItem,
              bool bHasNumFormat, sal_uInt32 nNumFormat,
              bool bHasValue, double nValue, bool bNoWrap, bool bCovered );

    // Protect an empty 1x1 cell
    void SetProtected();

    // Set/Get cell content
    void SetContents( HTMLTableCnts *pCnts ) { pContents = pCnts; }
    HTMLTableCnts *GetContents() { return pContents; }

    // Set/Get cell ROWSPAN/COLSPAN
    void SetRowSpan( sal_uInt16 nRSpan ) { nRowSpan = nRSpan; }
    sal_uInt16 GetRowSpan() const { return nRowSpan; }

    void SetColSpan( sal_uInt16 nCSpan ) { nColSpan = nCSpan; }
    sal_uInt16 GetColSpan() const { return nColSpan; }

    inline void SetWidth( sal_uInt16 nWidth, bool bRelWidth );

    const SvxBrushItem *GetBGBrush() const { return pBGBrush; }
    const std::shared_ptr<SvxBoxItem>& GetBoxItem() const { return m_pBoxItem; }

    inline bool GetNumFormat( sal_uInt32& rNumFormat ) const;
    inline bool GetValue( double& rValue ) const;

    sal_Int16 GetVertOri() const { return eVertOri; }

    // Is the cell filled or protected ?
    bool IsUsed() const { return pContents!=nullptr || bProtected; }

    std::unique_ptr<SwHTMLTableLayoutCell> CreateLayoutInfo();

    bool IsCovered() const { return mbCovered; }
};

// Row of a HTML table
typedef std::vector<std::unique_ptr<HTMLTableCell>> HTMLTableCells;

class HTMLTableRow
{
    HTMLTableCells *m_pCells;   ///< cells of the row

    bool bIsEndOfGroup : 1;

    sal_uInt16 nHeight;                     // options of <TR>/<TD>
    sal_uInt16 nEmptyRows;                  // number of empty rows are following

    SvxAdjust eAdjust;
    sal_Int16 eVertOri;
    SvxBrushItem *pBGBrush;             // background of cell from STYLE

public:

    bool bBottomBorder;                 // Is there a line after the row?

    explicit HTMLTableRow( sal_uInt16 nCells );    // cells of the row are empty

    ~HTMLTableRow();

    inline void SetHeight( sal_uInt16 nHeight );
    sal_uInt16 GetHeight() const { return nHeight; }

    inline HTMLTableCell *GetCell( sal_uInt16 nCell ) const;

    void SetAdjust( SvxAdjust eAdj ) { eAdjust = eAdj; }
    SvxAdjust GetAdjust() const { return eAdjust; }

    void SetVertOri( sal_Int16 eV) { eVertOri = eV; }
    sal_Int16 GetVertOri() const { return eVertOri; }

    void SetBGBrush( SvxBrushItem *pBrush ) { pBGBrush = pBrush; }
    const SvxBrushItem *GetBGBrush() const { return pBGBrush; }

    void SetEndOfGroup() { bIsEndOfGroup = true; }
    bool IsEndOfGroup() const { return bIsEndOfGroup; }

    void IncEmptyRows() { nEmptyRows++; }
    sal_uInt16 GetEmptyRows() const { return nEmptyRows; }

    // Expand row by adding empty cells
    void Expand( sal_uInt16 nCells, bool bOneCell=false );

    // Shrink row by deleting empty cells
    void Shrink( sal_uInt16 nCells );
};

// Column of a HTML table
class HTMLTableColumn
{
    bool bIsEndOfGroup;

    sal_uInt16 nWidth;                      // options of <COL>
    bool bRelWidth;

    SvxAdjust eAdjust;
    sal_Int16 eVertOri;

    SwFrameFormat *aFrameFormats[6];

    static inline sal_uInt16 GetFrameFormatIdx( bool bBorderLine,
                                sal_Int16 eVertOri );

public:

    bool bLeftBorder;                   // is there a line before the column

    HTMLTableColumn();

    inline void SetWidth( sal_uInt16 nWidth, bool bRelWidth);

    void SetAdjust( SvxAdjust eAdj ) { eAdjust = eAdj; }
    SvxAdjust GetAdjust() const { return eAdjust; }

    void SetVertOri( sal_Int16 eV) { eVertOri = eV; }
    sal_Int16 GetVertOri() const { return eVertOri; }

    void SetEndOfGroup() { bIsEndOfGroup = true; }
    bool IsEndOfGroup() const { return bIsEndOfGroup; }

    inline void SetFrameFormat( SwFrameFormat *pFormat, bool bBorderLine,
                           sal_Int16 eVertOri );
    inline SwFrameFormat *GetFrameFormat( bool bBorderLine,
                                sal_Int16 eVertOri ) const;

    std::unique_ptr<SwHTMLTableLayoutColumn> CreateLayoutInfo();
};

// HTML table
typedef std::vector<std::unique_ptr<HTMLTableRow>> HTMLTableRows;

typedef std::vector<std::unique_ptr<HTMLTableColumn>> HTMLTableColumns;

typedef std::vector<SdrObject *> SdrObjects;

class HTMLTable
{
    OUString m_aId;
    OUString m_aStyle;
    OUString m_aClass;
    OUString m_aDir;

    SdrObjects *m_pResizeDrawObjects;// SDR objects
    std::vector<sal_uInt16> *m_pDrawObjectPrcWidths;   // column of draw object and its rel. width

    HTMLTableRows *m_pRows;         ///< table rows
    HTMLTableColumns *m_pColumns;   ///< table columns

    sal_uInt16 m_nRows;                   // number of rows
    sal_uInt16 m_nCols;                   // number of columns
    sal_uInt16 m_nFilledColumns;             // number of filled columns

    sal_uInt16 m_nCurrentRow;                 // current Row
    sal_uInt16 m_nCurrentColumn;                 // current Column

    sal_uInt16 m_nLeftMargin;             // Space to the left margin (from paragraph edge)
    sal_uInt16 m_nRightMargin;            // Space to the right margin (from paragraph edge)

    sal_uInt16 m_nCellPadding;            // Space from border to Text
    sal_uInt16 m_nCellSpacing;            // Space between two cells
    sal_uInt16 m_nHSpace;
    sal_uInt16 m_nVSpace;

    sal_uInt16 m_nBoxes;                  // number of boxes in the table

    const SwStartNode *m_pPrevStartNode;   // the Table-Node or the Start-Node of the section before
    const SwTable *m_pSwTable;        // SW-Table (only on Top-Level)
    SwTableBox *m_pBox1;              // TableBox, generated when the Top-Level-Table was build
    SwTableBoxFormat *m_pBoxFormat;         // frame::Frame-Format from SwTableBox
    SwTableLineFormat *m_pLineFormat;       // frame::Frame-Format from SwTableLine
    SwTableLineFormat *m_pLineFrameFormatNoHeight;
    SvxBrushItem *m_pBackgroundBrush;         // background of the table
    SvxBrushItem *m_pInheritedBackgroundBrush;      // "inherited" background of the table
    const SwStartNode *m_pCaptionStartNode;   // Start-Node of the table-caption
    //lines for the border
    SvxBorderLine m_aTopBorderLine;
    SvxBorderLine m_aBottomBorderLine;
    SvxBorderLine m_aLeftBorderLine;
    SvxBorderLine m_aRightBorderLine;
    SvxBorderLine m_aBorderLine;
    SvxBorderLine m_aInheritedLeftBorderLine;
    SvxBorderLine m_aInheritedRightBorderLine;
    bool m_bTopBorder;                // is there a line on the top of the table
    bool m_bRightBorder;              // is there a line on the top right of the table
    bool m_bTopAllowed;                  // is it allowed to set the border?
    bool m_bRightAllowed;
    bool m_bFillerTopBorder;          // gets the left/right filler-cell a border on the
    bool m_bFillerBottomBorder;       // top or in the bottom
    bool m_bInheritedLeftBorder;
    bool m_bInheritedRightBorder;
    bool m_bBordersSet;               // the border is set already
    bool m_bForceFrame;
    bool m_bTableAdjustOfTag;         // comes nTableAdjust from <TABLE>?
    sal_uInt32 m_nHeadlineRepeat;         // repeating rows
    bool m_bIsParentHead;
    bool m_bHasParentSection;
    bool m_bHasToFly;
    bool m_bFixedCols;
    bool m_bColSpec;                  // where there COL(GROUP)-elements?
    bool m_bPrcWidth;                 // width is declared in %

    SwHTMLParser *m_pParser;          // the current parser
    HTMLTable *m_pTopTable;           // the table on the Top-Level
    HTMLTableCnts *m_pParentContents;

    HTMLTableContext *m_pContext;    // the context of the table

    SwHTMLTableLayout *m_pLayoutInfo;

    // the following parameters are from the <TABLE>-Tag
    sal_uInt16 m_nWidth;                  // width of the table
    sal_uInt16 m_nHeight;                 // absolute height of the table
    SvxAdjust m_eTableAdjust;             // drawing::Alignment of the table
    sal_Int16 m_eVertOrientation;         // Default vertical direction of the cells
    sal_uInt16 m_nBorder;                 // width of the external border
    HTMLTableFrame m_eFrame;          // frame around the table
    HTMLTableRules m_eRules;          // frame in the table
    bool m_bTopCaption;               // Caption of the table

    void InitCtor( const HTMLTableOptions *pOptions );

    // Correction of the Row-Spans for all cells above the chosen cell and the cell itself for the indicated content. The chosen cell gets the Row-Span 1
    void FixRowSpan( sal_uInt16 nRow, sal_uInt16 nCol, const HTMLTableCnts *pCnts );

    // Protects the chosen cell and the cells among
    void ProtectRowSpan( sal_uInt16 nRow, sal_uInt16 nCol, sal_uInt16 nRowSpan );

    // Looking for the SwStartNodes of the box ahead
    // If nRow==nCell==USHRT_MAX, return the last Start-Node of the table.
    const SwStartNode* GetPrevBoxStartNode( sal_uInt16 nRow, sal_uInt16 nCell ) const;

    sal_uInt16 GetTopCellSpace( sal_uInt16 nRow ) const;
    sal_uInt16 GetBottomCellSpace( sal_uInt16 nRow, sal_uInt16 nRowSpan ) const;

    // Conforming of the frame::Frame-Format of the box
    void FixFrameFormat( SwTableBox *pBox, sal_uInt16 nRow, sal_uInt16 nCol,
                      sal_uInt16 nRowSpan, sal_uInt16 nColSpan,
                      bool bFirstPara=true, bool bLastPara=true ) const;
    void FixFillerFrameFormat( SwTableBox *pBox, bool bRight ) const;

    // Create a table with the content (lines/boxes)
    void MakeTable_( SwTableBox *pUpper );

    // Gernerate a new SwTableBox, which contains a SwStartNode
    SwTableBox *NewTableBox( const SwStartNode *pStNd,
                             SwTableLine *pUpper ) const;

    // Generate a SwTableLine from the cells of the rectangle
    // (nTopRow/nLeftCol) inclusive to (nBottomRow/nRightRow) exclusive
    SwTableLine *MakeTableLine( SwTableBox *pUpper,
                                sal_uInt16 nTopRow, sal_uInt16 nLeftCol,
                                sal_uInt16 nBottomRow, sal_uInt16 nRightCol );

    // Generate a SwTableBox from the content of the cell
    SwTableBox *MakeTableBox( SwTableLine *pUpper,
                              HTMLTableCnts *pCnts,
                              sal_uInt16 nTopRow, sal_uInt16 nLeftCol,
                              sal_uInt16 nBootomRow, sal_uInt16 nRightCol );

    // Autolayout-Algorithm

    // Setting the border with the help of guidelines of the Parent-Table
    void InheritBorders( const HTMLTable *pParent,
                         sal_uInt16 nRow, sal_uInt16 nCol,
                         sal_uInt16 nRowSpan,
                         bool bFirstPara, bool bLastPara );

    // Inherit the left and the right border of the surrounding table
    void InheritVertBorders( const HTMLTable *pParent,
                             sal_uInt16 nCol, sal_uInt16 nColSpan );

    // Set the border with the help of the information from the user
    void SetBorders();

    // is the border already set?
    bool BordersSet() const { return m_bBordersSet; }

    const SvxBrushItem *GetBGBrush() const { return m_pBackgroundBrush; }
    const SvxBrushItem *GetInhBGBrush() const { return m_pInheritedBackgroundBrush; }

    sal_uInt16 GetBorderWidth( const SvxBorderLine& rBLine,
                           bool bWithDistance=false ) const;

public:

    bool m_bFirstCell;                // is there a cell created already?

    HTMLTable( SwHTMLParser* pPars, HTMLTable *pTopTab,
               bool bParHead, bool bHasParentSec,
               bool bHasToFly,
               const HTMLTableOptions *pOptions );

    ~HTMLTable();

    // Identifying of a cell
    inline HTMLTableCell *GetCell( sal_uInt16 nRow, sal_uInt16 nCell ) const;

    // set/determine caption
    inline void SetCaption( const SwStartNode *pStNd, bool bTop );
    const SwStartNode *GetCaptionStartNode() const { return m_pCaptionStartNode; }
    bool IsTopCaption() const { return m_bTopCaption; }

    SvxAdjust GetTableAdjust( bool bAny ) const
    {
        return (m_bTableAdjustOfTag || bAny) ? m_eTableAdjust : SvxAdjust::End;
    }

    sal_uInt16 GetHSpace() const { return m_nHSpace; }
    sal_uInt16 GetVSpace() const { return m_nVSpace; }

    // get inherited drawing::Alignment of rows and column
    SvxAdjust GetInheritedAdjust() const;
    sal_Int16 GetInheritedVertOri() const;

    // Insert a cell on the current position
    void InsertCell( HTMLTableCnts *pCnts, sal_uInt16 nRowSpan, sal_uInt16 nColSpan,
                     sal_uInt16 nWidth, bool bRelWidth, sal_uInt16 nHeight,
                     sal_Int16 eVertOri, SvxBrushItem *pBGBrush,
                     std::shared_ptr<SvxBoxItem> const& rBoxItem,
                     bool bHasNumFormat, sal_uInt32 nNumFormat,
                     bool bHasValue, double nValue, bool bNoWrap );

    // announce the start/end of a new row
    void OpenRow( SvxAdjust eAdjust, sal_Int16 eVertOri,
                  SvxBrushItem *pBGBrush );
    void CloseRow( bool bEmpty );

    // announce the end of a new section
    inline void CloseSection( bool bHead );

    // announce the end of a column-group
    inline void CloseColGroup( sal_uInt16 nSpan, sal_uInt16 nWidth, bool bRelWidth,
                               SvxAdjust eAdjust, sal_Int16 eVertOri );

    // insert a new column
    void InsertCol( sal_uInt16 nSpan, sal_uInt16 nWidth, bool bRelWidth,
                    SvxAdjust eAdjust, sal_Int16 eVertOri );

    // End a table definition (needs to be called for every table)
    void CloseTable();

    // Construct a SwTable (including child tables)
    void MakeTable( SwTableBox *pUpper, sal_uInt16 nAbsAvail,
                    sal_uInt16 nRelAvail=0, sal_uInt16 nAbsLeftSpace=0,
                    sal_uInt16 nAbsRightSpace=0, sal_uInt16 nInhAbsSpace=0 );

    bool IsNewDoc() const { return m_pParser->IsNewDoc(); }

    void SetHasParentSection( bool bSet ) { m_bHasParentSection = bSet; }
    bool HasParentSection() const { return m_bHasParentSection; }

    void SetParentContents( HTMLTableCnts *pCnts ) { m_pParentContents = pCnts; }
    HTMLTableCnts *GetParentContents() const { return m_pParentContents; }

    void MakeParentContents();

    bool GetIsParentHeader() const { return m_bIsParentHead; }

    bool HasToFly() const { return m_bHasToFly; }

    void SetTable( const SwStartNode *pStNd, HTMLTableContext *pCntxt,
                   sal_uInt16 nLeft, sal_uInt16 nRight,
                   const SwTable *pSwTab=nullptr, bool bFrcFrame=false );

    HTMLTableContext *GetContext() const { return m_pContext; }

    SwHTMLTableLayout *CreateLayoutInfo();

    bool HasColTags() const { return m_bColSpec; }

    sal_uInt16 IncGrfsThatResize() { return m_pSwTable ? const_cast<SwTable *>(m_pSwTable)->IncGrfsThatResize() : 0; }

    void RegisterDrawObject( SdrObject *pObj, sal_uInt8 nPrcWidth );

    const SwTable *GetSwTable() const { return m_pSwTable; }

    void SetBGBrush( const SvxBrushItem& rBrush ) { delete m_pBackgroundBrush; m_pBackgroundBrush = new SvxBrushItem( rBrush ); }

    const OUString& GetId() const { return m_aId; }
    const OUString& GetClass() const { return m_aClass; }
    const OUString& GetStyle() const { return m_aStyle; }
    const OUString& GetDirection() const { return m_aDir; }

    void IncBoxCount() { m_nBoxes++; }
    bool IsOverflowing() const { return m_nBoxes > 64000; }
};

void HTMLTableCnts::InitCtor()
{
    m_pNext = nullptr;
    m_pLayoutInfo = nullptr;

    m_bNoBreak = false;
}

HTMLTableCnts::HTMLTableCnts( const SwStartNode* pStNd ):
    m_pStartNode(pStNd), m_pTable(nullptr)
{
    InitCtor();
}

HTMLTableCnts::HTMLTableCnts( HTMLTable* pTab ):
    m_pStartNode(nullptr), m_pTable(pTab)
{
    InitCtor();
}

HTMLTableCnts::~HTMLTableCnts()
{
    delete m_pTable;              // we don't need the tables anymore
    delete m_pNext;
}

void HTMLTableCnts::Add( HTMLTableCnts* pNewCnts )
{
    HTMLTableCnts *pCnts = this;

    while( pCnts->m_pNext )
        pCnts = pCnts->m_pNext;

    pCnts->m_pNext = pNewCnts;
}

inline void HTMLTableCnts::SetTableBox( SwTableBox *pBox )
{
    OSL_ENSURE( m_pLayoutInfo, "There is no layout info" );
    if( m_pLayoutInfo )
        m_pLayoutInfo->SetTableBox( pBox );
}

SwHTMLTableLayoutCnts *HTMLTableCnts::CreateLayoutInfo()
{
    if( !m_pLayoutInfo )
    {
        SwHTMLTableLayoutCnts *pNextInfo = m_pNext ? m_pNext->CreateLayoutInfo() : nullptr;
        SwHTMLTableLayout *pTableInfo = m_pTable ? m_pTable->CreateLayoutInfo() : nullptr;

        m_pLayoutInfo = new SwHTMLTableLayoutCnts( m_pStartNode, pTableInfo,
                                                 m_bNoBreak, pNextInfo );
    }

    return m_pLayoutInfo;
}

HTMLTableCell::HTMLTableCell():
    pContents(nullptr),
    pBGBrush(nullptr),
    nNumFormat(0),
    nRowSpan(1),
    nColSpan(1),
    nWidth( 0 ),
    nValue(0),
    eVertOri( text::VertOrientation::NONE ),
    bProtected(false),
    bRelWidth( false ),
    bHasNumFormat(false),
    bHasValue(false),
    bNoWrap(false),
    mbCovered(false)
{}

HTMLTableCell::~HTMLTableCell()
{
    // the content is in multiple cells but mustn't be deleted more than once
    if( 1==nRowSpan && 1==nColSpan )
    {
        delete pContents;
        delete pBGBrush;
    }
}

void HTMLTableCell::Set( HTMLTableCnts *pCnts, sal_uInt16 nRSpan, sal_uInt16 nCSpan,
                         sal_Int16 eVert, SvxBrushItem *pBrush,
                         std::shared_ptr<SvxBoxItem> const& rBoxItem,
                         bool bHasNF, sal_uInt32 nNF, bool bHasV, double nVal,
                         bool bNWrap, bool bCovered )
{
    pContents = pCnts;
    nRowSpan = nRSpan;
    nColSpan = nCSpan;
    bProtected = false;
    eVertOri = eVert;
    pBGBrush = pBrush;
    m_pBoxItem = rBoxItem;

    bHasNumFormat = bHasNF;
    bHasValue = bHasV;
    nNumFormat = nNF;
    nValue = nVal;

    bNoWrap = bNWrap;
    mbCovered = bCovered;
}

inline void HTMLTableCell::SetWidth( sal_uInt16 nWdth, bool bRelWdth )
{
    nWidth = nWdth;
    bRelWidth = bRelWdth;
}

void HTMLTableCell::SetProtected()
{
    // The content of this cell doesn't have to be anchored anywhere else,
    // since they're not gonna be deleted

    pContents = nullptr;

    // Copy background color
    if( pBGBrush )
        pBGBrush = new SvxBrushItem( *pBGBrush );

    nRowSpan = 1;
    nColSpan = 1;
    bProtected = true;
}

inline bool HTMLTableCell::GetNumFormat( sal_uInt32& rNumFormat ) const
{
    rNumFormat = nNumFormat;
    return bHasNumFormat;
}

inline bool HTMLTableCell::GetValue( double& rValue ) const
{
    rValue = nValue;
    return bHasValue;
}

std::unique_ptr<SwHTMLTableLayoutCell> HTMLTableCell::CreateLayoutInfo()
{
    SwHTMLTableLayoutCnts *pCntInfo = pContents ? pContents->CreateLayoutInfo() : nullptr;

    return std::unique_ptr<SwHTMLTableLayoutCell>(new SwHTMLTableLayoutCell( pCntInfo, nRowSpan, nColSpan, nWidth,
                                      bRelWidth, bNoWrap ));
}

HTMLTableRow::HTMLTableRow(sal_uInt16 const nCells)
    : m_pCells(new HTMLTableCells),
    bIsEndOfGroup(false),
    nHeight(0),
    nEmptyRows(0),
    eAdjust(SvxAdjust::End),
    eVertOri(text::VertOrientation::TOP),
    pBGBrush(nullptr),
    bBottomBorder(false)
{
    for( sal_uInt16 i=0; i<nCells; i++ )
    {
        m_pCells->push_back(o3tl::make_unique<HTMLTableCell>());
    }

    OSL_ENSURE(nCells == m_pCells->size(),
            "wrong Cell count in new HTML table row");
}

HTMLTableRow::~HTMLTableRow()
{
    delete m_pCells;
    delete pBGBrush;
}

inline void HTMLTableRow::SetHeight( sal_uInt16 nHght )
{
    if( nHght > nHeight  )
        nHeight = nHght;
}

inline HTMLTableCell *HTMLTableRow::GetCell( sal_uInt16 nCell ) const
{
    OSL_ENSURE( nCell < m_pCells->size(),
        "invalid cell index in HTML table row" );
    return (*m_pCells)[nCell].get();
}

void HTMLTableRow::Expand( sal_uInt16 nCells, bool bOneCell )
{
    // This row will be filled with a single cell if bOneCell is set.
    // This will only work for rows that don't allow adding cells!

    sal_uInt16 nColSpan = nCells - m_pCells->size();
    for (sal_uInt16 i = m_pCells->size(); i < nCells; ++i)
    {
        std::unique_ptr<HTMLTableCell> pCell(new HTMLTableCell);
        if( bOneCell )
            pCell->SetColSpan( nColSpan );

        m_pCells->push_back(std::move(pCell));
        nColSpan--;
    }

    OSL_ENSURE(nCells == m_pCells->size(),
            "wrong Cell count in expanded HTML table row");
}

void HTMLTableRow::Shrink( sal_uInt16 nCells )
{
    OSL_ENSURE(nCells < m_pCells->size(), "number of cells too large");

#if OSL_DEBUG_LEVEL > 0
     sal_uInt16 const nEnd = m_pCells->size();
#endif
    // The colspan of empty cells at the end has to be fixed to the new
    // number of cells.
    sal_uInt16 i=nCells;
    while( i )
    {
        HTMLTableCell *pCell = (*m_pCells)[--i].get();
        if( !pCell->GetContents() )
        {
#if OSL_DEBUG_LEVEL > 0
            OSL_ENSURE( pCell->GetColSpan() == nEnd - i,
                    "invalid col span for empty cell at row end" );
#endif
            pCell->SetColSpan( nCells-i);
        }
        else
            break;
    }
#if OSL_DEBUG_LEVEL > 0
    for( i=nCells; i<nEnd; i++ )
    {
        HTMLTableCell *pCell = (*m_pCells)[i].get();
        OSL_ENSURE( pCell->GetRowSpan() == 1,
                    "RowSpan of to be deleted cell is wrong" );
        OSL_ENSURE( pCell->GetColSpan() == nEnd - i,
                    "ColSpan of to be deleted cell is wrong" );
        OSL_ENSURE( !pCell->GetContents(), "To be deleted cell has content" );
    }
#endif

    m_pCells->erase( m_pCells->begin() + nCells, m_pCells->end() );
}

HTMLTableColumn::HTMLTableColumn():
    bIsEndOfGroup(false),
    nWidth(0), bRelWidth(false),
    eAdjust(SvxAdjust::End), eVertOri(text::VertOrientation::TOP),
    bLeftBorder(false)
{
    for(SwFrameFormat* & rp : aFrameFormats)
        rp = nullptr;
}

inline void HTMLTableColumn::SetWidth( sal_uInt16 nWdth, bool bRelWdth )
{
    if( bRelWidth==bRelWdth )
    {
        if( nWdth > nWidth )
            nWidth = nWdth;
    }
    else
        nWidth = nWdth;
    bRelWidth = bRelWdth;
}

inline std::unique_ptr<SwHTMLTableLayoutColumn> HTMLTableColumn::CreateLayoutInfo()
{
    return std::unique_ptr<SwHTMLTableLayoutColumn>(new SwHTMLTableLayoutColumn( nWidth, bRelWidth, bLeftBorder ));
}

inline sal_uInt16 HTMLTableColumn::GetFrameFormatIdx( bool bBorderLine,
                                             sal_Int16 eVertOrient )
{
    OSL_ENSURE( text::VertOrientation::TOP != eVertOrient, "Top is not allowed" );
    sal_uInt16 n = bBorderLine ? 3 : 0;
    switch( eVertOrient )
    {
    case text::VertOrientation::CENTER:   n+=1;   break;
    case text::VertOrientation::BOTTOM:   n+=2;   break;
    default:
        ;
    }
    return n;
}

inline void HTMLTableColumn::SetFrameFormat( SwFrameFormat *pFormat, bool bBorderLine,
                                        sal_Int16 eVertOrient )
{
    aFrameFormats[GetFrameFormatIdx(bBorderLine,eVertOrient)] = pFormat;
}

inline SwFrameFormat *HTMLTableColumn::GetFrameFormat( bool bBorderLine,
                                             sal_Int16 eVertOrient ) const
{
    return aFrameFormats[GetFrameFormatIdx(bBorderLine,eVertOrient)];
}

void HTMLTable::InitCtor( const HTMLTableOptions *pOptions )
{
    m_pResizeDrawObjects = nullptr;
    m_pDrawObjectPrcWidths = nullptr;

    m_pRows = new HTMLTableRows;
    m_pColumns = new HTMLTableColumns;
    m_nRows = 0;
    m_nCurrentRow = 0; m_nCurrentColumn = 0;

    m_pBox1 = nullptr;
    m_pBoxFormat = nullptr; m_pLineFormat = nullptr;
    m_pLineFrameFormatNoHeight = nullptr;
    m_pInheritedBackgroundBrush = nullptr;

    m_pPrevStartNode = nullptr;
    m_pSwTable = nullptr;

    m_bTopBorder = false; m_bRightBorder = false;
    m_bTopAllowed = true; m_bRightAllowed = true;
    m_bFillerTopBorder = false; m_bFillerBottomBorder = false;
    m_bInheritedLeftBorder = false; m_bInheritedRightBorder = false;
    m_bBordersSet = false;
    m_bForceFrame = false;
    m_nHeadlineRepeat = 0;

    m_nLeftMargin = 0;
    m_nRightMargin = 0;

    const Color& rBorderColor = pOptions->aBorderColor;

    long nBorderOpt = (long)pOptions->nBorder;
    long nPWidth = nBorderOpt==USHRT_MAX ? NETSCAPE_DFLT_BORDER
                                         : nBorderOpt;
    long nPHeight = nBorderOpt==USHRT_MAX ? 0 : nBorderOpt;
    SvxCSS1Parser::PixelToTwip( nPWidth, nPHeight );

    // nBorder tells the width of the border as it's used in the width calculation of NetScape
    // If pOption->nBorder == USHRT_MAX, there wasn't a BORDER option given
    // Nonetheless, a 1 pixel wide border will be used for width calculation
    m_nBorder = (sal_uInt16)nPWidth;
    if( nBorderOpt==USHRT_MAX )
        nPWidth = 0;

    if ( pOptions->nCellSpacing != 0 )
    {
        m_aTopBorderLine.SetBorderLineStyle(SvxBorderLineStyle::DOUBLE);
    }
    m_aTopBorderLine.SetWidth( nPHeight );
    m_aTopBorderLine.SetColor( rBorderColor );
    m_aBottomBorderLine = m_aTopBorderLine;

    if( nPWidth == nPHeight )
    {
        m_aLeftBorderLine = m_aTopBorderLine;
    }
    else
    {
        if ( pOptions->nCellSpacing != 0 )
        {
            m_aLeftBorderLine.SetBorderLineStyle(SvxBorderLineStyle::DOUBLE);
        }
        m_aLeftBorderLine.SetWidth( nPWidth );
        m_aLeftBorderLine.SetColor( rBorderColor );
    }
    m_aRightBorderLine = m_aLeftBorderLine;

    if( pOptions->nCellSpacing != 0 )
    {
        m_aBorderLine.SetBorderLineStyle(SvxBorderLineStyle::DOUBLE);
        m_aBorderLine.SetWidth( DEF_LINE_WIDTH_0 );
    }
    else
    {
        m_aBorderLine.SetWidth( DEF_LINE_WIDTH_0 );
    }
    m_aBorderLine.SetColor( rBorderColor );

    if( m_nCellPadding )
    {
        if( m_nCellPadding==USHRT_MAX )
            m_nCellPadding = MIN_BORDER_DIST; // default
        else
        {
            m_nCellPadding = SwHTMLParser::ToTwips( m_nCellPadding );
            if( m_nCellPadding<MIN_BORDER_DIST  )
                m_nCellPadding = MIN_BORDER_DIST;
        }
    }
    if( m_nCellSpacing )
    {
        if( m_nCellSpacing==USHRT_MAX )
            m_nCellSpacing = NETSCAPE_DFLT_CELLSPACING;
        m_nCellSpacing = SwHTMLParser::ToTwips( m_nCellSpacing );
    }

    nPWidth = pOptions->nHSpace;
    nPHeight = pOptions->nVSpace;
    SvxCSS1Parser::PixelToTwip( nPWidth, nPHeight );
    m_nHSpace = (sal_uInt16)nPWidth;
    m_nVSpace = (sal_uInt16)nPHeight;

    m_bColSpec = false;

    m_pBackgroundBrush = m_pParser->CreateBrushItem(
                    pOptions->bBGColor ? &(pOptions->aBGColor) : nullptr,
                    pOptions->aBGImage, aEmptyOUStr, aEmptyOUStr, aEmptyOUStr );

    m_pContext = nullptr;
    m_pParentContents = nullptr;

    m_aId = pOptions->aId;
    m_aClass = pOptions->aClass;
    m_aStyle = pOptions->aStyle;
    m_aDir = pOptions->aDir;
}

HTMLTable::HTMLTable( SwHTMLParser* pPars, HTMLTable *pTopTab,
                      bool bParHead,
                      bool bHasParentSec, bool bHasToFlw,
                      const HTMLTableOptions *pOptions ) :
    m_nCols( pOptions->nCols ),
    m_nFilledColumns( 0 ),
    m_nCellPadding( pOptions->nCellPadding ),
    m_nCellSpacing( pOptions->nCellSpacing ),
    m_nBoxes( 1 ),
    m_pCaptionStartNode( nullptr ),
    m_bTableAdjustOfTag( !pTopTab && pOptions->bTableAdjust ),
    m_bIsParentHead( bParHead ),
    m_bHasParentSection( bHasParentSec ),
    m_bHasToFly( bHasToFlw ),
    m_bFixedCols( pOptions->nCols>0 ),
    m_bPrcWidth( pOptions->bPrcWidth ),
    m_pParser( pPars ),
    m_pTopTable( pTopTab ? pTopTab : this ),
    m_pLayoutInfo( nullptr ),
    m_nWidth( pOptions->nWidth ),
    m_nHeight( pTopTab ? 0 : pOptions->nHeight ),
    m_eTableAdjust( pOptions->eAdjust ),
    m_eVertOrientation( pOptions->eVertOri ),
    m_eFrame( pOptions->eFrame ),
    m_eRules( pOptions->eRules ),
    m_bTopCaption( false ),
    m_bFirstCell( !pTopTab )
{
    InitCtor( pOptions );

    for( sal_uInt16 i=0; i<m_nCols; i++ )
        m_pColumns->push_back(o3tl::make_unique<HTMLTableColumn>());
}

HTMLTable::~HTMLTable()
{
    delete m_pResizeDrawObjects;
    delete m_pDrawObjectPrcWidths;

    delete m_pRows;
    delete m_pColumns;
    delete m_pBackgroundBrush;
    delete m_pInheritedBackgroundBrush;

    delete m_pContext;

    // pLayoutInfo has either already been deleted or is now owned by SwTable
}

SwHTMLTableLayout *HTMLTable::CreateLayoutInfo()
{
    sal_uInt16 nW = m_bPrcWidth ? m_nWidth : SwHTMLParser::ToTwips( m_nWidth );

    sal_uInt16 nBorderWidth = GetBorderWidth( m_aBorderLine, true );
    sal_uInt16 nLeftBorderWidth =
        (*m_pColumns)[0]->bLeftBorder ? GetBorderWidth(m_aLeftBorderLine, true) : 0;
    sal_uInt16 nRightBorderWidth =
        m_bRightBorder ? GetBorderWidth( m_aRightBorderLine, true ) : 0;

    m_pLayoutInfo = new SwHTMLTableLayout(
                        m_pSwTable,
                        m_nRows, m_nCols, m_bFixedCols, m_bColSpec,
                        nW, m_bPrcWidth, m_nBorder, m_nCellPadding,
                        m_nCellSpacing, m_eTableAdjust,
                        m_nLeftMargin, m_nRightMargin,
                        nBorderWidth, nLeftBorderWidth, nRightBorderWidth,
                        0/*nInhLeftBorderWidth*/, 0/*nInhRightBorderWidth*/ );

    bool bExportable = true;
    sal_uInt16 i;
    for( i=0; i<m_nRows; i++ )
    {
        HTMLTableRow *const pRow = (*m_pRows)[i].get();
        for( sal_uInt16 j=0; j<m_nCols; j++ )
        {
            m_pLayoutInfo->SetCell( pRow->GetCell(j)->CreateLayoutInfo(), i, j );
            SwHTMLTableLayoutCell* pLayoutCell = m_pLayoutInfo->GetCell(i, j );

            if( bExportable )
            {
                SwHTMLTableLayoutCnts *pLayoutCnts =
                    pLayoutCell->GetContents();
                bExportable = !pLayoutCnts ||
                              ( pLayoutCnts->GetStartNode() &&
                                !pLayoutCnts->GetNext() );
            }
        }
    }

    m_pLayoutInfo->SetExportable( bExportable );

    for( i=0; i<m_nCols; i++ )
        m_pLayoutInfo->SetColumn( (*m_pColumns)[i]->CreateLayoutInfo(), i );

    return m_pLayoutInfo;
}

inline void HTMLTable::SetCaption( const SwStartNode *pStNd, bool bTop )
{
    m_pCaptionStartNode = pStNd;
    m_bTopCaption = bTop;
}

void HTMLTable::FixRowSpan( sal_uInt16 nRow, sal_uInt16 nCol,
                            const HTMLTableCnts *pCnts )
{
    sal_uInt16 nRowSpan=1;
    HTMLTableCell *pCell;
    while( ( pCell=GetCell(nRow,nCol), pCell->GetContents()==pCnts ) )
    {
        pCell->SetRowSpan( nRowSpan );
        if( m_pLayoutInfo )
            m_pLayoutInfo->GetCell(nRow,nCol)->SetRowSpan( nRowSpan );

        if( !nRow ) break;
        nRowSpan++; nRow--;
    }
}

void HTMLTable::ProtectRowSpan( sal_uInt16 nRow, sal_uInt16 nCol, sal_uInt16 nRowSpan )
{
    for( sal_uInt16 i=0; i<nRowSpan; i++ )
    {
        GetCell(nRow+i,nCol)->SetProtected();
        if( m_pLayoutInfo )
            m_pLayoutInfo->GetCell(nRow+i,nCol)->SetProtected();
    }
}

// Search the SwStartNode of the last used predecessor box
const SwStartNode* HTMLTable::GetPrevBoxStartNode( sal_uInt16 nRow, sal_uInt16 nCol ) const
{
    const HTMLTableCnts *pPrevCnts = nullptr;

    if( 0==nRow )
    {
        // always the predecessor cell
        if( nCol>0 )
            pPrevCnts = GetCell( 0, nCol-1 )->GetContents();
        else
            return m_pPrevStartNode;
    }
    else if( USHRT_MAX==nRow && USHRT_MAX==nCol )
        // contents of preceding cell
        pPrevCnts = GetCell( m_nRows-1, m_nCols-1 )->GetContents();
    else
    {
        sal_uInt16 i;
        HTMLTableRow *const pPrevRow = (*m_pRows)[nRow-1].get();

        // maybe a cell in the current row
        i = nCol;
        while( i )
        {
            i--;
            if( 1 == pPrevRow->GetCell(i)->GetRowSpan() )
            {
                pPrevCnts = GetCell(nRow,i)->GetContents();
                break;
            }
        }

        // otherwise the last filled cell of the row before
        if( !pPrevCnts )
        {
            i = m_nCols;
            while( !pPrevCnts && i )
            {
                i--;
                pPrevCnts = pPrevRow->GetCell(i)->GetContents();
            }
        }
    }
    OSL_ENSURE( pPrevCnts, "No previous filled cell found" );
    if( !pPrevCnts )
    {
        pPrevCnts = GetCell(0,0)->GetContents();
        if( !pPrevCnts )
            return m_pPrevStartNode;
    }

    while( pPrevCnts->Next() )
        pPrevCnts = pPrevCnts->Next();

    return ( pPrevCnts->GetStartNode() ? pPrevCnts->GetStartNode()
               : pPrevCnts->GetTable()->GetPrevBoxStartNode( USHRT_MAX, USHRT_MAX ) );
}

static bool IsBoxEmpty( const SwTableBox *pBox )
{
    const SwStartNode *pSttNd = pBox->GetSttNd();
    if( pSttNd &&
        pSttNd->GetIndex() + 2 == pSttNd->EndOfSectionIndex() )
    {
        const SwContentNode *pCNd =
            pSttNd->GetNodes()[pSttNd->GetIndex()+1]->GetContentNode();
        if( pCNd && !pCNd->Len() )
            return true;
    }

    return false;
}

sal_uInt16 HTMLTable::GetTopCellSpace( sal_uInt16 nRow ) const
{
    sal_uInt16 nSpace = m_nCellPadding;

    if( nRow == 0 )
    {
        nSpace += m_nBorder + m_nCellSpacing;
    }

    return nSpace;
}

sal_uInt16 HTMLTable::GetBottomCellSpace( sal_uInt16 nRow, sal_uInt16 nRowSpan ) const
{
    sal_uInt16 nSpace = m_nCellSpacing + m_nCellPadding;

    if( nRow+nRowSpan == m_nRows )
    {
        nSpace = nSpace + m_nBorder;
    }

    return nSpace;
}

void HTMLTable::FixFrameFormat( SwTableBox *pBox,
                             sal_uInt16 nRow, sal_uInt16 nCol,
                             sal_uInt16 nRowSpan, sal_uInt16 nColSpan,
                             bool bFirstPara, bool bLastPara ) const
{
    SwFrameFormat *pFrameFormat = nullptr;      // frame::Frame format
    sal_Int16 eVOri = text::VertOrientation::NONE;
    const SvxBrushItem *pBGBrushItem = nullptr;   // background
    std::shared_ptr<SvxBoxItem> pBoxItem;
    bool bTopLine = false, bBottomLine = false, bLastBottomLine = false;
    bool bReUsable = false;     // Format reusable?
    sal_uInt16 nEmptyRows = 0;
    bool bHasNumFormat = false;
    bool bHasValue = false;
    sal_uInt32 nNumFormat = 0;
    double nValue = 0.0;

    HTMLTableColumn *const pColumn = (*m_pColumns)[nCol].get();

    if( pBox->GetSttNd() )
    {
        // Determine background color/graphic
        const HTMLTableCell *pCell = GetCell( nRow, nCol );
        pBoxItem = pCell->GetBoxItem();
        pBGBrushItem = pCell->GetBGBrush();
        if( !pBGBrushItem )
        {
            // If a cell spans multiple rows, a background to that row should be copied to the cell.
            // If it's a table in a table and that cell goes over the whole height of that table,
            // the row's background has to be copied to the cell as well,
            // since the line is gonna be GC-ed (correctly).
            if( nRowSpan > 1 || (this != m_pTopTable && nRowSpan==m_nRows) )
            {
                pBGBrushItem = (*m_pRows)[nRow]->GetBGBrush();
                if( !pBGBrushItem && this != m_pTopTable )
                {
                    pBGBrushItem = GetBGBrush();
                    if( !pBGBrushItem )
                        pBGBrushItem = GetInhBGBrush();
                }
            }
        }

        bTopLine = 0==nRow && m_bTopBorder && bFirstPara;
        if ((*m_pRows)[nRow+nRowSpan-1]->bBottomBorder && bLastPara)
        {
            nEmptyRows = (*m_pRows)[nRow+nRowSpan-1]->GetEmptyRows();
            if( nRow+nRowSpan == m_nRows )
                bLastBottomLine = true;
            else
                bBottomLine = true;
        }

        eVOri = pCell->GetVertOri();
        bHasNumFormat = pCell->GetNumFormat( nNumFormat );
        if( bHasNumFormat )
            bHasValue = pCell->GetValue( nValue );

        if( nColSpan==1 && !bTopLine && !bLastBottomLine && !nEmptyRows &&
            !pBGBrushItem && !bHasNumFormat && !pBoxItem)
        {
            pFrameFormat = pColumn->GetFrameFormat( bBottomLine, eVOri );
            bReUsable = !pFrameFormat;
        }
    }

    if( !pFrameFormat )
    {
        pFrameFormat = pBox->ClaimFrameFormat();

        // calculate width of the box
        SwTwips nFrameWidth = (SwTwips)m_pLayoutInfo->GetColumn(nCol)
                                                ->GetRelColWidth();
        for( sal_uInt16 i=1; i<nColSpan; i++ )
            nFrameWidth += (SwTwips)m_pLayoutInfo->GetColumn(nCol+i)
                                             ->GetRelColWidth();

        // Only set the border on edit boxes.
        // On setting the upper and lower border, keep in mind if
        // it's the first or the last paragraph of the cell
        if( pBox->GetSttNd() )
        {
            bool bSet = (m_nCellPadding > 0);

            SvxBoxItem aBoxItem( RES_BOX );
            long nInnerFrameWidth = nFrameWidth;

            if( bTopLine )
            {
                aBoxItem.SetLine( &m_aTopBorderLine, SvxBoxItemLine::TOP );
                bSet = true;
            }
            if( bLastBottomLine )
            {
                aBoxItem.SetLine( &m_aBottomBorderLine, SvxBoxItemLine::BOTTOM );
                bSet = true;
            }
            else if( bBottomLine )
            {
                if( nEmptyRows && !m_aBorderLine.GetInWidth() )
                {
                    // For now, empty rows can only be emulated by thick lines, if it's a single line
                    SvxBorderLine aThickBorderLine( m_aBorderLine );

                    sal_uInt16 nBorderWidth = m_aBorderLine.GetOutWidth();
                    nBorderWidth *= (nEmptyRows + 1);
                    aThickBorderLine.SetBorderLineStyle(SvxBorderLineStyle::SOLID);
                    aThickBorderLine.SetWidth( nBorderWidth );
                    aBoxItem.SetLine( &aThickBorderLine, SvxBoxItemLine::BOTTOM );
                }
                else
                {
                    aBoxItem.SetLine( &m_aBorderLine, SvxBoxItemLine::BOTTOM );
                }
                bSet = true;
            }
            if (((*m_pColumns)[nCol])->bLeftBorder)
            {
                const SvxBorderLine& rBorderLine =
                    0==nCol ? m_aLeftBorderLine : m_aBorderLine;
                aBoxItem.SetLine( &rBorderLine, SvxBoxItemLine::LEFT );
                nInnerFrameWidth -= GetBorderWidth( rBorderLine );
                bSet = true;
            }
            if( m_bRightBorder )
            {
                aBoxItem.SetLine( &m_aRightBorderLine, SvxBoxItemLine::RIGHT );
                nInnerFrameWidth -= GetBorderWidth( m_aRightBorderLine );
                bSet = true;
            }

            if (pBoxItem)
            {
                pFrameFormat->SetFormatAttr( *pBoxItem );
            }
            else if (bSet)
            {
                // BorderDist is not part of a cell with fixed width
                sal_uInt16 nBDist = static_cast< sal_uInt16 >(
                    (2*m_nCellPadding <= nInnerFrameWidth) ? m_nCellPadding
                                                      : (nInnerFrameWidth / 2) );
                // We only set the item if there's a border or a border distance
                // If the latter is missing, there's gonna be a border and we'll have to set the distance
                aBoxItem.SetAllDistances((nBDist) ? nBDist : MIN_BORDER_DIST);
                pFrameFormat->SetFormatAttr( aBoxItem );
            }
            else
                pFrameFormat->ResetFormatAttr( RES_BOX );

            if( pBGBrushItem )
            {
                pFrameFormat->SetFormatAttr( *pBGBrushItem );
            }
            else
                pFrameFormat->ResetFormatAttr( RES_BACKGROUND );

            // Only set format if there's a value or the box is empty
            if( bHasNumFormat && (bHasValue || IsBoxEmpty(pBox)) )
            {
                bool bLock = pFrameFormat->GetDoc()->GetNumberFormatter()
                                     ->IsTextFormat( nNumFormat );
                SfxItemSet aItemSet( *pFrameFormat->GetAttrSet().GetPool(),
                                     svl::Items<RES_BOXATR_FORMAT, RES_BOXATR_VALUE>{} );
                SvxAdjust eAdjust = SvxAdjust::End;
                SwContentNode *pCNd = nullptr;
                if( !bLock )
                {
                    const SwStartNode *pSttNd = pBox->GetSttNd();
                    pCNd = pSttNd->GetNodes()[pSttNd->GetIndex()+1]
                                 ->GetContentNode();
                    const SfxPoolItem *pItem;
                    if( pCNd && pCNd->HasSwAttrSet() &&
                        SfxItemState::SET==pCNd->GetpSwAttrSet()->GetItemState(
                            RES_PARATR_ADJUST, false, &pItem ) )
                    {
                        eAdjust = static_cast<const SvxAdjustItem *>(pItem)
                            ->GetAdjust();
                    }
                }
                aItemSet.Put( SwTableBoxNumFormat(nNumFormat) );
                if( bHasValue )
                    aItemSet.Put( SwTableBoxValue(nValue) );

                if( bLock )
                    pFrameFormat->LockModify();
                pFrameFormat->SetFormatAttr( aItemSet );
                if( bLock )
                    pFrameFormat->UnlockModify();
                else if( pCNd && SvxAdjust::End != eAdjust )
                {
                    SvxAdjustItem aAdjItem( eAdjust, RES_PARATR_ADJUST );
                    pCNd->SetAttr( aAdjItem );
                }
            }
            else
                pFrameFormat->ResetFormatAttr( RES_BOXATR_FORMAT );

            OSL_ENSURE( eVOri != text::VertOrientation::TOP, "text::VertOrientation::TOP is not allowed!" );
            if( text::VertOrientation::NONE != eVOri )
            {
                pFrameFormat->SetFormatAttr( SwFormatVertOrient( 0, eVOri ) );
            }
            else
                pFrameFormat->ResetFormatAttr( RES_VERT_ORIENT );

            if( bReUsable )
                pColumn->SetFrameFormat( pFrameFormat, bBottomLine, eVOri );
        }
        else
        {
            pFrameFormat->ResetFormatAttr( RES_BOX );
            pFrameFormat->ResetFormatAttr( RES_BACKGROUND );
            pFrameFormat->ResetFormatAttr( RES_VERT_ORIENT );
            pFrameFormat->ResetFormatAttr( RES_BOXATR_FORMAT );
        }
    }
    else
    {
        OSL_ENSURE( pBox->GetSttNd() ||
                SfxItemState::SET!=pFrameFormat->GetAttrSet().GetItemState(
                                    RES_VERT_ORIENT, false ),
                "Box without content has vertical orientation" );
        pBox->ChgFrameFormat( static_cast<SwTableBoxFormat*>(pFrameFormat) );
    }

}

void HTMLTable::FixFillerFrameFormat( SwTableBox *pBox, bool bRight ) const
{
    SwFrameFormat *pFrameFormat = pBox->ClaimFrameFormat();

    if( m_bFillerTopBorder || m_bFillerBottomBorder ||
        (!bRight && m_bInheritedLeftBorder) || (bRight && m_bInheritedRightBorder) )
    {
        SvxBoxItem aBoxItem( RES_BOX );
        if( m_bFillerTopBorder )
            aBoxItem.SetLine( &m_aTopBorderLine, SvxBoxItemLine::TOP );
        if( m_bFillerBottomBorder )
            aBoxItem.SetLine( &m_aBottomBorderLine, SvxBoxItemLine::BOTTOM );
        if( !bRight && m_bInheritedLeftBorder )
            aBoxItem.SetLine( &m_aInheritedLeftBorderLine, SvxBoxItemLine::LEFT );
        if( bRight && m_bInheritedRightBorder )
            aBoxItem.SetLine( &m_aInheritedRightBorderLine, SvxBoxItemLine::RIGHT );
        aBoxItem.SetAllDistances(MIN_BORDER_DIST);
        pFrameFormat->SetFormatAttr( aBoxItem );
    }
    else
    {
        pFrameFormat->ResetFormatAttr( RES_BOX );
    }

    if( GetInhBGBrush() )
        pFrameFormat->SetFormatAttr( *GetInhBGBrush() );
    else
        pFrameFormat->ResetFormatAttr( RES_BACKGROUND );

    pFrameFormat->ResetFormatAttr( RES_VERT_ORIENT );
    pFrameFormat->ResetFormatAttr( RES_BOXATR_FORMAT );
}

SwTableBox *HTMLTable::NewTableBox( const SwStartNode *pStNd,
                                    SwTableLine *pUpper ) const
{
    SwTableBox *pBox;

    if( m_pTopTable->m_pBox1 &&
        m_pTopTable->m_pBox1->GetSttNd() == pStNd )
    {
        // If the StartNode is the StartNode of the initially created box, we take that box
        pBox = m_pTopTable->m_pBox1;
        pBox->SetUpper( pUpper );
        m_pTopTable->m_pBox1 = nullptr;
    }
    else
        pBox = new SwTableBox( m_pBoxFormat, *pStNd, pUpper );

    return pBox;
}

static void ResetLineFrameFormatAttrs( SwFrameFormat *pFrameFormat )
{
    pFrameFormat->ResetFormatAttr( RES_FRM_SIZE );
    pFrameFormat->ResetFormatAttr( RES_BACKGROUND );
    OSL_ENSURE( SfxItemState::SET!=pFrameFormat->GetAttrSet().GetItemState(
                                RES_VERT_ORIENT, false ),
            "Cell has vertical orientation" );
}

// !!! could be simplified
SwTableLine *HTMLTable::MakeTableLine( SwTableBox *pUpper,
                                       sal_uInt16 nTopRow, sal_uInt16 nLeftCol,
                                       sal_uInt16 nBottomRow, sal_uInt16 nRightCol )
{
    SwTableLine *pLine;
    if( this==m_pTopTable && !pUpper && 0==nTopRow )
        pLine = (m_pSwTable->GetTabLines())[0];
    else
        pLine = new SwTableLine( m_pLineFrameFormatNoHeight ? m_pLineFrameFormatNoHeight
                                                     : m_pLineFormat,
                                 0, pUpper );

    HTMLTableRow *pTopRow = (*m_pRows)[nTopRow].get();
    sal_uInt16 nRowHeight = pTopRow->GetHeight();
    const SvxBrushItem *pBGBrushItem = nullptr;
    if( this == m_pTopTable || nTopRow>0 || nBottomRow<m_nRows )
    {
        // It doesn't make sense to set a color on a line,
        // if it's the outermost and simultaneously sole line of a table in a table
        pBGBrushItem = pTopRow->GetBGBrush();

        if( !pBGBrushItem && this != m_pTopTable )
        {
            // A background that's set on a table in a table is set on the rows.
            // It's the same for the background of the cell where that table is
            pBGBrushItem = GetBGBrush();
            if( !pBGBrushItem )
                pBGBrushItem = GetInhBGBrush();
        }
    }
    if( nTopRow==nBottomRow-1 && (nRowHeight || pBGBrushItem) )
    {
        SwTableLineFormat *pFrameFormat = static_cast<SwTableLineFormat*>(pLine->ClaimFrameFormat());
        ResetLineFrameFormatAttrs( pFrameFormat );

        if( nRowHeight )
        {
            // set table height. Since it's a minimum height it can be calculated like in Netscape,
            // so without considering the actual border width
            nRowHeight += GetTopCellSpace( nTopRow ) +
                       GetBottomCellSpace( nTopRow, 1 );

            pFrameFormat->SetFormatAttr( SwFormatFrameSize( ATT_MIN_SIZE, 0, nRowHeight ) );
        }

        if( pBGBrushItem )
        {
            pFrameFormat->SetFormatAttr( *pBGBrushItem );
        }

    }
    else if( !m_pLineFrameFormatNoHeight )
    {
        // else, we'll have to remove the height from the attribute and remember the format
        m_pLineFrameFormatNoHeight = static_cast<SwTableLineFormat*>(pLine->ClaimFrameFormat());

        ResetLineFrameFormatAttrs( m_pLineFrameFormatNoHeight );
    }

    SwTableBoxes& rBoxes = pLine->GetTabBoxes();

    sal_uInt16 nStartCol = nLeftCol;
    while( nStartCol<nRightCol )
    {
        sal_uInt16 nCol = nStartCol;
        sal_uInt16 nSplitCol = nRightCol;
        bool bSplitted = false;
        while( !bSplitted )
        {
            OSL_ENSURE( nCol < nRightCol, "Gone too far" );

            HTMLTableCell *pCell = GetCell(nTopRow,nCol);
            const bool bSplit = 1 == pCell->GetColSpan();

            OSL_ENSURE((nCol != nRightCol-1) || bSplit, "Split-Flag wrong");
            if( bSplit )
            {
                SwTableBox* pBox = nullptr;
                HTMLTableCell *pCell2 = GetCell( nTopRow, nStartCol );
                if( pCell2->GetColSpan() == (nCol+1-nStartCol) )
                {
                    // The HTML tables represent a box. So we need to split behind that box
                    nSplitCol = nCol + 1;

                    long nBoxRowSpan = pCell2->GetRowSpan();
                    if ( !pCell2->GetContents() || pCell2->IsCovered() )
                    {
                        if ( pCell2->IsCovered() )
                            nBoxRowSpan = -1 * nBoxRowSpan;

                        const SwStartNode* pPrevStartNd =
                            GetPrevBoxStartNode( nTopRow, nStartCol );
                        HTMLTableCnts *pCnts = new HTMLTableCnts(
                            m_pParser->InsertTableSection(pPrevStartNd) );
                        SwHTMLTableLayoutCnts *pCntsLayoutInfo =
                            pCnts->CreateLayoutInfo();

                        pCell2->SetContents( pCnts );
                        SwHTMLTableLayoutCell *pCurrCell = m_pLayoutInfo->GetCell( nTopRow, nStartCol );
                        pCurrCell->SetContents( pCntsLayoutInfo );
                        if( nBoxRowSpan < 0 )
                            pCurrCell->SetRowSpan( 0 );

                        // check COLSPAN if needed
                        for( sal_uInt16 j=nStartCol+1; j<nSplitCol; j++ )
                        {
                            GetCell(nTopRow,j)->SetContents( pCnts );
                            m_pLayoutInfo->GetCell( nTopRow, j )
                                       ->SetContents( pCntsLayoutInfo );
                        }
                    }

                    pBox = MakeTableBox( pLine, pCell2->GetContents(),
                                         nTopRow, nStartCol,
                                         nBottomRow, nSplitCol );

                    if ( 1 != nBoxRowSpan )
                        pBox->setRowSpan( nBoxRowSpan );

                    bSplitted = true;
                }

                OSL_ENSURE( pBox, "Colspan trouble" );

                if( pBox )
                    rBoxes.push_back( pBox );
            }
            nCol++;
        }
        nStartCol = nSplitCol;
    }

    return pLine;
}

SwTableBox *HTMLTable::MakeTableBox( SwTableLine *pUpper,
                                     HTMLTableCnts *pCnts,
                                     sal_uInt16 nTopRow, sal_uInt16 nLeftCol,
                                     sal_uInt16 nBottomRow, sal_uInt16 nRightCol )
{
    SwTableBox *pBox;
    sal_uInt16 nColSpan = nRightCol - nLeftCol;
    sal_uInt16 nRowSpan = nBottomRow - nTopRow;

    if( !pCnts->Next() )
    {
        // just one content section
        if( pCnts->GetStartNode() )
        {
            // ... that's not a table
            pBox = NewTableBox( pCnts->GetStartNode(), pUpper );
            pCnts->SetTableBox( pBox );
        }
        else
        {
            pCnts->GetTable()->InheritVertBorders( this, nLeftCol,
                                                   nRightCol-nLeftCol );
            // ... that's a table. We'll build a new box and put the rows of the table
            // in the rows of the box
            pBox = new SwTableBox( m_pBoxFormat, 0, pUpper );
            sal_uInt16 nAbs, nRel;
            m_pLayoutInfo->GetAvail( nLeftCol, nColSpan, nAbs, nRel );
            sal_uInt16 nLSpace = m_pLayoutInfo->GetLeftCellSpace( nLeftCol, nColSpan );
            sal_uInt16 nRSpace = m_pLayoutInfo->GetRightCellSpace( nLeftCol, nColSpan );
            sal_uInt16 nInhSpace = m_pLayoutInfo->GetInhCellSpace( nLeftCol, nColSpan );
            pCnts->GetTable()->MakeTable( pBox, nAbs, nRel, nLSpace, nRSpace,
                                          nInhSpace );
        }
    }
    else
    {
        // multiple content sections: we'll build a box with rows
        pBox = new SwTableBox( m_pBoxFormat, 0, pUpper );
        SwTableLines& rLines = pBox->GetTabLines();
        bool bFirstPara = true;

        while( pCnts )
        {
            if( pCnts->GetStartNode() )
            {
                // normal paragraphs are gonna be boxes in a row
                SwTableLine *pLine =
                    new SwTableLine( m_pLineFrameFormatNoHeight ? m_pLineFrameFormatNoHeight
                                                         : m_pLineFormat, 0, pBox );
                if( !m_pLineFrameFormatNoHeight )
                {
                    // If there's no line format without height yet, we can use that one
                    m_pLineFrameFormatNoHeight = static_cast<SwTableLineFormat*>(pLine->ClaimFrameFormat());

                    ResetLineFrameFormatAttrs( m_pLineFrameFormatNoHeight );
                }

                SwTableBox* pCntBox = NewTableBox( pCnts->GetStartNode(),
                                                   pLine );
                pCnts->SetTableBox( pCntBox );
                FixFrameFormat( pCntBox, nTopRow, nLeftCol, nRowSpan, nColSpan,
                             bFirstPara, nullptr==pCnts->Next() );
                pLine->GetTabBoxes().push_back( pCntBox );

                rLines.push_back( pLine );
            }
            else
            {
                pCnts->GetTable()->InheritVertBorders( this, nLeftCol,
                                                       nRightCol-nLeftCol );
                // Tables are entered directly
                sal_uInt16 nAbs, nRel;
                m_pLayoutInfo->GetAvail( nLeftCol, nColSpan, nAbs, nRel );
                sal_uInt16 nLSpace = m_pLayoutInfo->GetLeftCellSpace( nLeftCol,
                                                                nColSpan );
                sal_uInt16 nRSpace = m_pLayoutInfo->GetRightCellSpace( nLeftCol,
                                                                 nColSpan );
                sal_uInt16 nInhSpace = m_pLayoutInfo->GetInhCellSpace( nLeftCol, nColSpan );
                pCnts->GetTable()->MakeTable( pBox, nAbs, nRel, nLSpace,
                                              nRSpace, nInhSpace );
            }

            pCnts = pCnts->Next();
            bFirstPara = false;
        }
    }

    FixFrameFormat( pBox, nTopRow, nLeftCol, nRowSpan, nColSpan );

    return pBox;
}

void HTMLTable::InheritBorders( const HTMLTable *pParent,
                                sal_uInt16 nRow, sal_uInt16 nCol,
                                sal_uInt16 nRowSpan,
                                bool bFirstPara, bool bLastPara )
{
    OSL_ENSURE( m_nRows>0 && m_nCols>0 && m_nCurrentRow==m_nRows,
            "Was CloseTable not called?" );

    // The child table needs a border, if the surrounding cell has a margin on that side.
    // The upper/lower border is only set if the table is the first/last paragraph in that cell
    // It can't be determined if a border for that table is needed or possible for the left or right side,
    // since that's depending on if filler cells are gonna be added. We'll only collect info for now

    if( 0==nRow && pParent->m_bTopBorder && bFirstPara )
    {
        m_bTopBorder = true;
        m_bFillerTopBorder = true; // fillers get a border too
        m_aTopBorderLine = pParent->m_aTopBorderLine;
    }
    if ((*pParent->m_pRows)[nRow+nRowSpan-1]->bBottomBorder && bLastPara)
    {
        (*m_pRows)[m_nRows-1]->bBottomBorder = true;
        m_bFillerBottomBorder = true; // fillers get a border too
        m_aBottomBorderLine =
            nRow+nRowSpan==pParent->m_nRows ? pParent->m_aBottomBorderLine
                                          : pParent->m_aBorderLine;
    }

    // The child table mustn't get an upper or lower border, if that's already done by the surrounding table
    // It can get an upper border if the table is not the first paragraph in that cell
    m_bTopAllowed = ( !bFirstPara || (pParent->m_bTopAllowed &&
                 (0==nRow || !((*pParent->m_pRows)[nRow-1])->bBottomBorder)) );

    // The child table has to inherit the color of the cell it's contained in, if it doesn't have one
    const SvxBrushItem *pInhBG = pParent->GetCell(nRow,nCol)->GetBGBrush();
    if( !pInhBG && pParent != m_pTopTable &&
        pParent->GetCell(nRow,nCol)->GetRowSpan() == pParent->m_nRows )
    {
        // the whole surrounding table is a table in a table and consists only of a single line
        // that's gonna be GC-ed (correctly). That's why the background of that line is copied.
        pInhBG = (*pParent->m_pRows)[nRow]->GetBGBrush();
        if( !pInhBG )
            pInhBG = pParent->GetBGBrush();
        if( !pInhBG )
            pInhBG = pParent->GetInhBGBrush();
    }
    if( pInhBG )
        m_pInheritedBackgroundBrush = new SvxBrushItem( *pInhBG );
}

void HTMLTable::InheritVertBorders( const HTMLTable *pParent,
                                 sal_uInt16 nCol, sal_uInt16 nColSpan )
{
    sal_uInt16 nInhLeftBorderWidth = 0;
    sal_uInt16 nInhRightBorderWidth = 0;

    if( nCol+nColSpan==pParent->m_nCols && pParent->m_bRightBorder )
    {
        m_bInheritedRightBorder = true; // just remember for now
        m_aInheritedRightBorderLine = pParent->m_aRightBorderLine;
        nInhRightBorderWidth =
            GetBorderWidth( m_aInheritedRightBorderLine, true ) + MIN_BORDER_DIST;
    }

    if (((*pParent->m_pColumns)[nCol])->bLeftBorder)
    {
        m_bInheritedLeftBorder = true;  // just remember for now
        m_aInheritedLeftBorderLine = 0==nCol ? pParent->m_aLeftBorderLine
                                     : pParent->m_aBorderLine;
        nInhLeftBorderWidth =
            GetBorderWidth( m_aInheritedLeftBorderLine, true ) + MIN_BORDER_DIST;
    }

    if( !m_bInheritedLeftBorder && (m_bFillerTopBorder || m_bFillerBottomBorder) )
        nInhLeftBorderWidth = 2 * MIN_BORDER_DIST;
    if( !m_bInheritedRightBorder && (m_bFillerTopBorder || m_bFillerBottomBorder) )
        nInhRightBorderWidth = 2 * MIN_BORDER_DIST;
    m_pLayoutInfo->SetInhBorderWidths( nInhLeftBorderWidth,
                                     nInhRightBorderWidth );

    m_bRightAllowed = ( pParent->m_bRightAllowed &&
                  (nCol+nColSpan==pParent->m_nCols ||
                   !((*pParent->m_pColumns)[nCol+nColSpan])->bLeftBorder) );
}

void HTMLTable::SetBorders()
{
    sal_uInt16 i;
    for( i=1; i<m_nCols; i++ )
        if( HTMLTableRules::All==m_eRules || HTMLTableRules::Cols==m_eRules ||
            ((HTMLTableRules::Rows==m_eRules || HTMLTableRules::Groups==m_eRules) &&
             ((*m_pColumns)[i-1])->IsEndOfGroup()))
        {
            ((*m_pColumns)[i])->bLeftBorder = true;
        }

    for( i=0; i<m_nRows-1; i++ )
        if( HTMLTableRules::All==m_eRules || HTMLTableRules::Rows==m_eRules ||
            ((HTMLTableRules::Cols==m_eRules || HTMLTableRules::Groups==m_eRules) &&
             (*m_pRows)[i]->IsEndOfGroup()))
        {
            (*m_pRows)[i]->bBottomBorder = true;
        }

    if( m_bTopAllowed && (HTMLTableFrame::Above==m_eFrame || HTMLTableFrame::HSides==m_eFrame ||
                     HTMLTableFrame::Box==m_eFrame) )
        m_bTopBorder = true;
    if( HTMLTableFrame::Below==m_eFrame || HTMLTableFrame::HSides==m_eFrame ||
        HTMLTableFrame::Box==m_eFrame )
    {
        (*m_pRows)[m_nRows-1]->bBottomBorder = true;
    }
    if( HTMLTableFrame::RHS==m_eFrame || HTMLTableFrame::VSides==m_eFrame ||
                      HTMLTableFrame::Box==m_eFrame )
        m_bRightBorder = true;
    if( HTMLTableFrame::LHS==m_eFrame || HTMLTableFrame::VSides==m_eFrame || HTMLTableFrame::Box==m_eFrame )
    {
        ((*m_pColumns)[0])->bLeftBorder = true;
    }

    for( i=0; i<m_nRows; i++ )
    {
        HTMLTableRow *const pRow = (*m_pRows)[i].get();
        for( sal_uInt16 j=0; j<m_nCols; j++ )
        {
            HTMLTableCell *pCell = pRow->GetCell(j);
            if( pCell->GetContents()  )
            {
                HTMLTableCnts *pCnts = pCell->GetContents();
                bool bFirstPara = true;
                while( pCnts )
                {
                    HTMLTable *pTable = pCnts->GetTable();
                    if( pTable && !pTable->BordersSet() )
                    {
                        pTable->InheritBorders( this, i, j,
                                                pCell->GetRowSpan(),
                                                bFirstPara,
                                                nullptr==pCnts->Next() );
                        pTable->SetBorders();
                    }
                    bFirstPara = false;
                    pCnts = pCnts->Next();
                }
            }
        }
    }

    m_bBordersSet = true;
}

sal_uInt16 HTMLTable::GetBorderWidth( const SvxBorderLine& rBLine,
                                  bool bWithDistance ) const
{
    sal_uInt16 nBorderWidth = rBLine.GetWidth();
    if( bWithDistance )
    {
        if( m_nCellPadding )
            nBorderWidth = nBorderWidth + m_nCellPadding;
        else if( nBorderWidth )
            nBorderWidth = nBorderWidth + MIN_BORDER_DIST;
    }

    return nBorderWidth;
}

inline HTMLTableCell *HTMLTable::GetCell( sal_uInt16 nRow,
                                          sal_uInt16 nCell ) const
{
    OSL_ENSURE(nRow < m_pRows->size(), "invalid row index in HTML table");
    return (*m_pRows)[nRow]->GetCell( nCell );
}

SvxAdjust HTMLTable::GetInheritedAdjust() const
{
    SvxAdjust eAdjust = (m_nCurrentColumn<m_nCols ? ((*m_pColumns)[m_nCurrentColumn])->GetAdjust()
                                       : SvxAdjust::End );
    if( SvxAdjust::End==eAdjust )
        eAdjust = (*m_pRows)[m_nCurrentRow]->GetAdjust();

    return eAdjust;
}

sal_Int16 HTMLTable::GetInheritedVertOri() const
{
    // text::VertOrientation::TOP is default!
    sal_Int16 eVOri = (*m_pRows)[m_nCurrentRow]->GetVertOri();
    if( text::VertOrientation::TOP==eVOri && m_nCurrentColumn<m_nCols )
        eVOri = ((*m_pColumns)[m_nCurrentColumn])->GetVertOri();
    if( text::VertOrientation::TOP==eVOri )
        eVOri = m_eVertOrientation;

    OSL_ENSURE( m_eVertOrientation != text::VertOrientation::TOP, "text::VertOrientation::TOP is not allowed!" );
    return eVOri;
}

void HTMLTable::InsertCell( HTMLTableCnts *pCnts,
                            sal_uInt16 nRowSpan, sal_uInt16 nColSpan,
                            sal_uInt16 nCellWidth, bool bRelWidth, sal_uInt16 nCellHeight,
                            sal_Int16 eVertOrient, SvxBrushItem *pBGBrushItem,
                            std::shared_ptr<SvxBoxItem> const& rBoxItem,
                            bool bHasNumFormat, sal_uInt32 nNumFormat,
                            bool bHasValue, double nValue, bool bNoWrap )
{
    if( !nRowSpan || (sal_uInt32)m_nCurrentRow + nRowSpan > USHRT_MAX )
        nRowSpan = 1;

    if( !nColSpan || (sal_uInt32)m_nCurrentColumn + nColSpan > USHRT_MAX )
        nColSpan = 1;

    sal_uInt16 nColsReq = m_nCurrentColumn + nColSpan;
    sal_uInt16 nRowsReq = m_nCurrentRow + nRowSpan;
    sal_uInt16 i, j;

    // if we need more colums than we currently have, we need to add cells for all rows
    if( m_nCols < nColsReq )
    {
        for( i=m_nCols; i<nColsReq; i++ )
            m_pColumns->push_back(o3tl::make_unique<HTMLTableColumn>());
        for( i=0; i<m_nRows; i++ )
            (*m_pRows)[i]->Expand( nColsReq, i<m_nCurrentRow );
        m_nCols = nColsReq;
        OSL_ENSURE(m_pColumns->size() == m_nCols,
                "wrong number of columns after expanding");
    }
    if( nColsReq > m_nFilledColumns )
        m_nFilledColumns = nColsReq;

    // if we need more rows than we currently have, we need to add cells
    if( m_nRows < nRowsReq )
    {
        for( i=m_nRows; i<nRowsReq; i++ )
            m_pRows->push_back(o3tl::make_unique<HTMLTableRow>(m_nCols));
        m_nRows = nRowsReq;
        OSL_ENSURE(m_nRows == m_pRows->size(), "wrong number of rows in Insert");
    }

    // Check if we have an overlap and could remove that
    sal_uInt16 nSpanedCols = 0;
    if( m_nCurrentRow>0 )
    {
        HTMLTableRow *const pCurRow = (*m_pRows)[m_nCurrentRow].get();
        for( i=m_nCurrentColumn; i<nColsReq; i++ )
        {
            HTMLTableCell *pCell = pCurRow->GetCell(i);
            if( pCell->GetContents() )
            {
                // A cell from a row further above overlaps this one.
                // Content and colors are coming from that cell and can be overwritten
                // or deleted (content) or copied (color) by ProtectRowSpan
                nSpanedCols = i + pCell->GetColSpan();
                FixRowSpan( m_nCurrentRow-1, i, pCell->GetContents() );
                if( pCell->GetRowSpan() > nRowSpan )
                    ProtectRowSpan( nRowsReq, i,
                                    pCell->GetRowSpan()-nRowSpan );
            }
        }
        for( i=nColsReq; i<nSpanedCols; i++ )
        {
            // These contents are anchored in the row above in any case
            HTMLTableCell *pCell = pCurRow->GetCell(i);
            FixRowSpan( m_nCurrentRow-1, i, pCell->GetContents() );
            ProtectRowSpan( m_nCurrentRow, i, pCell->GetRowSpan() );
        }
    }

    // Fill the cells
    for( i=nColSpan; i>0; i-- )
    {
        for( j=nRowSpan; j>0; j-- )
        {
            const bool bCovered = i != nColSpan || j != nRowSpan;
            GetCell( nRowsReq-j, nColsReq-i )
                ->Set( pCnts, j, i, eVertOrient, pBGBrushItem, rBoxItem,
                       bHasNumFormat, nNumFormat, bHasValue, nValue, bNoWrap, bCovered );
        }
    }

    Size aTwipSz( bRelWidth ? 0 : nCellWidth, nCellHeight );
    if( (aTwipSz.Width() || aTwipSz.Height()) && Application::GetDefaultDevice() )
    {
        aTwipSz = Application::GetDefaultDevice()
                    ->PixelToLogic( aTwipSz, MapMode( MapUnit::MapTwip ) );
    }

    // Only set width on the first cell!
    if( nCellWidth )
    {
        sal_uInt16 nTmp = bRelWidth ? nCellWidth : (sal_uInt16)aTwipSz.Width();
        GetCell( m_nCurrentRow, m_nCurrentColumn )->SetWidth( nTmp, bRelWidth );
    }

    // Remember height
    if( nCellHeight && 1==nRowSpan )
    {
        (*m_pRows)[m_nCurrentRow]->SetHeight(static_cast<sal_uInt16>(aTwipSz.Height()));
    }

    // Set the column counter behind the new cells
    m_nCurrentColumn = nColsReq;
    if( nSpanedCols > m_nCurrentColumn )
        m_nCurrentColumn = nSpanedCols;

    // and search for the next free cell
    while( m_nCurrentColumn<m_nCols && GetCell(m_nCurrentRow,m_nCurrentColumn)->IsUsed() )
        m_nCurrentColumn++;
}

inline void HTMLTable::CloseSection( bool bHead )
{
    // Close the preceding sections if there's already a row
    OSL_ENSURE( m_nCurrentRow<=m_nRows, "ungeultige aktuelle Zeile" );
    if( m_nCurrentRow>0 && m_nCurrentRow<=m_nRows )
        (*m_pRows)[m_nCurrentRow-1]->SetEndOfGroup();
    if( bHead )
        m_nHeadlineRepeat = m_nCurrentRow;
}

void HTMLTable::OpenRow( SvxAdjust eAdjust, sal_Int16 eVertOrient,
                         SvxBrushItem *pBGBrushItem )
{
    sal_uInt16 nRowsReq = m_nCurrentRow+1;

    // create the next row if it's not there already
    if( m_nRows<nRowsReq )
    {
        for( sal_uInt16 i=m_nRows; i<nRowsReq; i++ )
            m_pRows->push_back(o3tl::make_unique<HTMLTableRow>(m_nCols));
        m_nRows = nRowsReq;
        OSL_ENSURE( m_nRows == m_pRows->size(),
                "Row number in OpenRow is wrong" );
    }

    HTMLTableRow *const pCurRow = (*m_pRows)[m_nCurrentRow].get();
    pCurRow->SetAdjust( eAdjust );
    pCurRow->SetVertOri( eVertOrient );
    if( pBGBrushItem )
        (*m_pRows)[m_nCurrentRow]->SetBGBrush( pBGBrushItem );

    // reset the column counter
    m_nCurrentColumn=0;

    // and search for the next free cell
    while( m_nCurrentColumn<m_nCols && GetCell(m_nCurrentRow,m_nCurrentColumn)->IsUsed() )
        m_nCurrentColumn++;
}

void HTMLTable::CloseRow( bool bEmpty )
{
    OSL_ENSURE( m_nCurrentRow<m_nRows, "current row after table end" );

    // empty cells just get a slightly thicker lower border!
    if( bEmpty )
    {
        if( m_nCurrentRow > 0 )
            (*m_pRows)[m_nCurrentRow-1]->IncEmptyRows();
        return;
    }

    HTMLTableRow *const pRow = (*m_pRows)[m_nCurrentRow].get();

    // modify the COLSPAN of all empty cells at the row end in a way, that they're forming a single cell
    // that can be done here (and not earlier) since there's no more cells in that row
    sal_uInt16 i=m_nCols;
    while( i )
    {
        HTMLTableCell *pCell = pRow->GetCell(--i);
        if( !pCell->GetContents() )
        {
            sal_uInt16 nColSpan = m_nCols-i;
            if( nColSpan > 1 )
                pCell->SetColSpan( nColSpan );
        }
        else
            break;
    }

    m_nCurrentRow++;
}

inline void HTMLTable::CloseColGroup( sal_uInt16 nSpan, sal_uInt16 _nWidth,
                                      bool bRelWidth, SvxAdjust eAdjust,
                                      sal_Int16 eVertOrient )
{
    if( nSpan )
        InsertCol( nSpan, _nWidth, bRelWidth, eAdjust, eVertOrient );

    OSL_ENSURE( m_nCurrentColumn<=m_nCols, "invalid column" );
    if( m_nCurrentColumn>0 && m_nCurrentColumn<=m_nCols )
        ((*m_pColumns)[m_nCurrentColumn-1])->SetEndOfGroup();
}

void HTMLTable::InsertCol( sal_uInt16 nSpan, sal_uInt16 nColWidth, bool bRelWidth,
                           SvxAdjust eAdjust, sal_Int16 eVertOrient )
{
    // #i35143# - no columns, if rows already exist.
    if ( m_nRows > 0 )
        return;

    sal_uInt16 i;

    if( !nSpan )
        nSpan = 1;

    sal_uInt16 nColsReq = m_nCurrentColumn + nSpan;

    if( m_nCols < nColsReq )
    {
        for( i=m_nCols; i<nColsReq; i++ )
            m_pColumns->push_back(o3tl::make_unique<HTMLTableColumn>());
        m_nCols = nColsReq;
    }

    Size aTwipSz( bRelWidth ? 0 : nColWidth, 0 );
    if( aTwipSz.Width() && Application::GetDefaultDevice() )
    {
        aTwipSz = Application::GetDefaultDevice()
                    ->PixelToLogic( aTwipSz, MapMode( MapUnit::MapTwip ) );
    }

    for( i=m_nCurrentColumn; i<nColsReq; i++ )
    {
        HTMLTableColumn *const pCol = (*m_pColumns)[i].get();
        sal_uInt16 nTmp = bRelWidth ? nColWidth : (sal_uInt16)aTwipSz.Width();
        pCol->SetWidth( nTmp, bRelWidth );
        pCol->SetAdjust( eAdjust );
        pCol->SetVertOri( eVertOrient );
    }

    m_bColSpec = true;

    m_nCurrentColumn = nColsReq;
}

void HTMLTable::CloseTable()
{
    sal_uInt16 i;

    // The number of table rows is only dependent on the <TR> elements (i.e. nCurRow).
    // Rows that are spanned via ROWSPAN behind nCurRow need to be deleted
    // and we need to adjust the ROWSPAN in the rows above
    if( m_nRows>m_nCurrentRow )
    {
        HTMLTableRow *const pPrevRow = (*m_pRows)[m_nCurrentRow-1].get();
        HTMLTableCell *pCell;
        for( i=0; i<m_nCols; i++ )
        {
            pCell = pPrevRow->GetCell(i);
            if( pCell->GetRowSpan() > 1 )
            {
                FixRowSpan( m_nCurrentRow-1, i, pCell->GetContents() );
                ProtectRowSpan(m_nCurrentRow, i, (*m_pRows)[m_nCurrentRow]->GetCell(i)->GetRowSpan());
            }
        }
        for( i=m_nRows-1; i>=m_nCurrentRow; i-- )
            m_pRows->erase(m_pRows->begin() + i);
        m_nRows = m_nCurrentRow;
    }

    // if the table has no column, we need to add one
    if( 0==m_nCols )
    {
        m_pColumns->push_back(o3tl::make_unique<HTMLTableColumn>());
        for( i=0; i<m_nRows; i++ )
            (*m_pRows)[i]->Expand(1);
        m_nCols = 1;
        m_nFilledColumns = 1;
    }

    // if the table has no row, we need to add one
    if( 0==m_nRows )
    {
        m_pRows->push_back(o3tl::make_unique<HTMLTableRow>(m_nCols));
        m_nRows = 1;
        m_nCurrentRow = 1;
    }

    if( m_nFilledColumns < m_nCols )
    {
        m_pColumns->erase(m_pColumns->begin() + m_nFilledColumns, m_pColumns->begin() + m_nCols);
        for( i=0; i<m_nRows; i++ )
            (*m_pRows)[i]->Shrink( m_nFilledColumns );
        m_nCols = m_nFilledColumns;
    }
}

void HTMLTable::MakeTable_( SwTableBox *pBox )
{
    SwTableLines& rLines = (pBox ? pBox->GetTabLines()
                                 : const_cast<SwTable *>(m_pSwTable)->GetTabLines() );

    for( sal_uInt16 i=0; i<m_nRows; i++ )
    {
        SwTableLine *pLine = MakeTableLine( pBox, i, 0, i+1, m_nCols );
        if( pBox || i > 0 )
            rLines.push_back( pLine );
    }
}

/* How are tables aligned?

first row: without paragraph indents
second row: with paragraph indents

ALIGN=          LEFT            RIGHT           CENTER          -
-------------------------------------------------------------------------
xxx for tables with WIDTH=nn% the percentage value is important:
xxx nn = 100        text::HoriOrientation::FULL       text::HoriOrientation::FULL       text::HoriOrientation::FULL       text::HoriOrientation::FULL %
xxx                 text::HoriOrientation::NONE       text::HoriOrientation::NONE       text::HoriOrientation::NONE %     text::HoriOrientation::NONE %
xxx nn < 100        frame F        frame F        text::HoriOrientation::CENTER %   text::HoriOrientation::LEFT %
xxx                 frame F        frame F        text::HoriOrientation::CENTER %   text::HoriOrientation::NONE %

for tables with WIDTH=nn% the percentage value is important:
nn = 100        text::HoriOrientation::LEFT       text::HoriOrientation::RIGHT      text::HoriOrientation::CENTER %   text::HoriOrientation::LEFT %
                text::HoriOrientation::LEFT_AND   text::HoriOrientation::RIGHT      text::HoriOrientation::CENTER %   text::HoriOrientation::LEFT_AND %
nn < 100        frame F        frame F        text::HoriOrientation::CENTER %   text::HoriOrientation::LEFT %
                frame F        frame F        text::HoriOrientation::CENTER %   text::HoriOrientation::NONE %

otherwise the calculated width w
w = avail*      text::HoriOrientation::LEFT       text::HoriOrientation::RIGHT      text::HoriOrientation::CENTER     text::HoriOrientation::LEFT
                HORI_LEDT_AND   text::HoriOrientation::RIGHT      text::HoriOrientation::CENTER     text::HoriOrientation::LEFT_AND
w < avail       frame L        frame L        text::HoriOrientation::CENTER     text::HoriOrientation::LEFT
                frame L        frame L        text::HoriOrientation::CENTER     text::HoriOrientation::NONE

xxx *) if for the table no size was specified, always
xxx   text::HoriOrientation::FULL is taken

*/

void HTMLTable::MakeTable( SwTableBox *pBox, sal_uInt16 nAbsAvail,
                           sal_uInt16 nRelAvail, sal_uInt16 nAbsLeftSpace,
                           sal_uInt16 nAbsRightSpace, sal_uInt16 nInhAbsSpace )
{
    OSL_ENSURE( m_nRows>0 && m_nCols>0 && m_nCurrentRow==m_nRows,
            "Was CloseTable not called?" );

    OSL_ENSURE( (m_pLayoutInfo==nullptr) == (this==m_pTopTable),
            "Top-Table has no layout info or vice versa" );

    if( this==m_pTopTable )
    {
        // Calculate borders of the table and all contained tables
        SetBorders();

        // Step 1: needed layout structures are created (including tables in tables)
        CreateLayoutInfo();

        // Step 2: the minimal and maximal column width is calculated
        // (including tables in tables). Since we don't have boxes yet,
        // we'll work on the start nodes
        m_pLayoutInfo->AutoLayoutPass1();
    }

    // Step 3: the actual column widths of this table are calculated (not tables in tables)
    // We need this now to decide if we need filler cells
    // (Pass1 was needed because of this as well)
    m_pLayoutInfo->AutoLayoutPass2( nAbsAvail, nRelAvail, nAbsLeftSpace,
                                  nAbsRightSpace, nInhAbsSpace );

    if( this!=m_pTopTable )
    {
        // the right and left border of this table can be finally defined
        if( m_pLayoutInfo->GetRelRightFill() == 0 )
        {
            if( !m_bRightBorder )
            {
                // inherit left border of the outer table
                if( m_bInheritedRightBorder )
                {
                    m_bRightBorder = true;
                    m_aRightBorderLine = m_aInheritedRightBorderLine;
                }
            }
            else
            {
                // Only set border if allowed
                m_bRightBorder = m_bRightAllowed;
            }
        }

        if( m_pLayoutInfo->GetRelLeftFill() == 0 &&
            !((*m_pColumns)[0])->bLeftBorder &&
            m_bInheritedLeftBorder )
        {
            // If applicable, inherit right border of outer table
            ((*m_pColumns)[0])->bLeftBorder = true;
            m_aLeftBorderLine = m_aInheritedLeftBorderLine;
        }
    }

    // Set adjustment for the top table
    if( this==m_pTopTable )
    {
        sal_Int16 eHoriOri;
        if( m_bForceFrame )
        {
            // The table should go in a text frame and it's narrower than the
            // available space and not 100% wide. So it gets a border
            eHoriOri = m_bPrcWidth ? text::HoriOrientation::FULL : text::HoriOrientation::LEFT;
        }
        else switch( m_eTableAdjust )
        {
            // The table either fits the page but shouldn't get a text frame,
            // or it's wider than the page so it doesn't need a text frame

        case SvxAdjust::Right:
            // Don't be considerate of the right margin in right-adjusted tables
            eHoriOri = text::HoriOrientation::RIGHT;
            break;
        case SvxAdjust::Center:
            // Centred tables are not considerate of margins
            eHoriOri = text::HoriOrientation::CENTER;
            break;
        case SvxAdjust::Left:
        default:
            // left-adjusted tables are only considerate of the left margin
            eHoriOri = m_nLeftMargin ? text::HoriOrientation::LEFT_AND_WIDTH : text::HoriOrientation::LEFT;
            break;
        }

        // get the table format and adapt it
        SwFrameFormat *pFrameFormat = m_pSwTable->GetFrameFormat();
        pFrameFormat->SetFormatAttr( SwFormatHoriOrient(0,eHoriOri) );
        if( text::HoriOrientation::LEFT_AND_WIDTH==eHoriOri )
        {
            OSL_ENSURE( m_nLeftMargin || m_nRightMargin,
                    "There are still leftovers from relative margins" );

            // The right margin will be ignored anyway.
            SvxLRSpaceItem aLRItem( m_pSwTable->GetFrameFormat()->GetLRSpace() );
            aLRItem.SetLeft( m_nLeftMargin );
            aLRItem.SetRight( m_nRightMargin );
            pFrameFormat->SetFormatAttr( aLRItem );
        }

        if( m_bPrcWidth && text::HoriOrientation::FULL!=eHoriOri )
        {
            pFrameFormat->LockModify();
            SwFormatFrameSize aFrameSize( pFrameFormat->GetFrameSize() );
            aFrameSize.SetWidthPercent( (sal_uInt8)m_nWidth );
            pFrameFormat->SetFormatAttr( aFrameSize );
            pFrameFormat->UnlockModify();
        }
    }

    // get the default line and box format
    if( this==m_pTopTable )
    {
        // remember the first box and unlist it from the first row
        SwTableLine *pLine1 = (m_pSwTable->GetTabLines())[0];
        m_pBox1 = (pLine1->GetTabBoxes())[0];
        pLine1->GetTabBoxes().erase(pLine1->GetTabBoxes().begin());

        m_pLineFormat = static_cast<SwTableLineFormat*>(pLine1->GetFrameFormat());
        m_pBoxFormat = static_cast<SwTableBoxFormat*>(m_pBox1->GetFrameFormat());
    }
    else
    {
        m_pLineFormat = m_pTopTable->m_pLineFormat;
        m_pBoxFormat = m_pTopTable->m_pBoxFormat;
    }

    // If applicable, add filler cells for tables in tables
    if( this != m_pTopTable &&
        ( m_pLayoutInfo->GetRelLeftFill() > 0  ||
          m_pLayoutInfo->GetRelRightFill() > 0 ) )
    {
        OSL_ENSURE( pBox, "No TableBox for table in table" );

        SwTableLines& rLines = pBox->GetTabLines();

        // first, we need a new table line in the box
        SwTableLine *pLine =
            new SwTableLine( m_pLineFrameFormatNoHeight ? m_pLineFrameFormatNoHeight
                                                 : m_pLineFormat, 0, pBox );
        rLines.push_back( pLine );

        // Check that we have a format without height
        if( !m_pLineFrameFormatNoHeight )
        {
            // Otherwise, we need to remove the height from the attributes
            // and remember that format
            m_pLineFrameFormatNoHeight = static_cast<SwTableLineFormat*>(pLine->ClaimFrameFormat());

            ResetLineFrameFormatAttrs( m_pLineFrameFormatNoHeight );
        }

        SwTableBoxes& rBoxes = pLine->GetTabBoxes();
        SwTableBox *pNewBox;

        // If applicable, add a cell to the left
        if( m_pLayoutInfo->GetRelLeftFill() > 0 )
        {
            // pPrevStNd is the predecessor start node of the table
            // We'll add the filler node just behind
            m_pPrevStartNode = m_pParser->InsertTableSection( m_pPrevStartNode );

            pNewBox = NewTableBox( m_pPrevStartNode, pLine );
            rBoxes.push_back( pNewBox );
            FixFillerFrameFormat( pNewBox, false );
            m_pLayoutInfo->SetLeftFillerBox( pNewBox );
        }

        // modify the table now
        pNewBox = new SwTableBox( m_pBoxFormat, 0, pLine );
        rBoxes.push_back( pNewBox );

        SwFrameFormat *pFrameFormat = pNewBox->ClaimFrameFormat();
        pFrameFormat->ResetFormatAttr( RES_BOX );
        pFrameFormat->ResetFormatAttr( RES_BACKGROUND );
        pFrameFormat->ResetFormatAttr( RES_VERT_ORIENT );
        pFrameFormat->ResetFormatAttr( RES_BOXATR_FORMAT );

        MakeTable_( pNewBox );

        // and add a cell to the right if applicable
        if( m_pLayoutInfo->GetRelRightFill() > 0 )
        {
            const SwStartNode *pStNd =
                GetPrevBoxStartNode( USHRT_MAX, USHRT_MAX );
            pStNd = m_pParser->InsertTableSection( pStNd );

            pNewBox = NewTableBox( pStNd, pLine );
            rBoxes.push_back( pNewBox );

            FixFillerFrameFormat( pNewBox, true );
            m_pLayoutInfo->SetRightFillerBox( pNewBox );
        }
    }
    else
    {
        MakeTable_( pBox );
    }

    // Finally, we'll do a garbage collection for the top level table
    if( this==m_pTopTable )
    {
        if( 1==m_nRows && m_nHeight && 1==m_pSwTable->GetTabLines().size() )
        {
            // Set height of a one-row table as the minimum width of the row
            // Was originally a fixed height, but that made problems
            // and is not Netscape 4.0 compliant
            m_nHeight = SwHTMLParser::ToTwips( m_nHeight );
            if( m_nHeight < MINLAY )
                m_nHeight = MINLAY;

            (m_pSwTable->GetTabLines())[0]->ClaimFrameFormat();
            (m_pSwTable->GetTabLines())[0]->GetFrameFormat()
                ->SetFormatAttr( SwFormatFrameSize( ATT_MIN_SIZE, 0, m_nHeight ) );
        }

        if( GetBGBrush() )
            m_pSwTable->GetFrameFormat()->SetFormatAttr( *GetBGBrush() );

        const_cast<SwTable *>(m_pSwTable)->SetRowsToRepeat( static_cast< sal_uInt16 >(m_nHeadlineRepeat) );
        const_cast<SwTable *>(m_pSwTable)->GCLines();

        bool bIsInFlyFrame = m_pContext && m_pContext->GetFrameFormat();
        if( bIsInFlyFrame && !m_nWidth )
        {
            SvxAdjust eAdjust = GetTableAdjust(false);
            if (eAdjust != SvxAdjust::Left &&
                eAdjust != SvxAdjust::Right)
            {
                // If a table with a width attribute isn't flowed around left or right
                // we'll stack it with a border of 100% width, so its size will
                // be adapted. That text frame mustn't be modified
                OSL_ENSURE( HasToFly(), "Why is the table in a frame?" );
                sal_uInt32 nMin = m_pLayoutInfo->GetMin();
                if( nMin > USHRT_MAX )
                    nMin = USHRT_MAX;
                SwFormatFrameSize aFlyFrameSize( ATT_VAR_SIZE, (SwTwips)nMin, MINLAY );
                aFlyFrameSize.SetWidthPercent( 100 );
                m_pContext->GetFrameFormat()->SetFormatAttr( aFlyFrameSize );
                bIsInFlyFrame = false;
            }
            else
            {
                // left or right adjusted table without width mustn't be adjusted in width
                // as they would only shrink but never grow
                m_pLayoutInfo->SetMustNotRecalc( true );
                if( m_pContext->GetFrameFormat()->GetAnchor().GetContentAnchor()
                    ->nNode.GetNode().FindTableNode() )
                {
                    sal_uInt32 nMax = m_pLayoutInfo->GetMax();
                    if( nMax > USHRT_MAX )
                        nMax = USHRT_MAX;
                    SwFormatFrameSize aFlyFrameSize( ATT_VAR_SIZE, (SwTwips)nMax, MINLAY );
                    m_pContext->GetFrameFormat()->SetFormatAttr( aFlyFrameSize );
                    bIsInFlyFrame = false;
                }
                else
                {
                    m_pLayoutInfo->SetMustNotResize( true );
                }
            }
        }
        m_pLayoutInfo->SetMayBeInFlyFrame( bIsInFlyFrame );

        // Only tables with relative width or without width should be modified
        m_pLayoutInfo->SetMustResize( m_bPrcWidth || !m_nWidth );

        m_pLayoutInfo->SetWidths();

        const_cast<SwTable *>(m_pSwTable)->SetHTMLTableLayout( m_pLayoutInfo );

        if( m_pResizeDrawObjects )
        {
            sal_uInt16 nCount = m_pResizeDrawObjects->size();
            for( sal_uInt16 i=0; i<nCount; i++ )
            {
                SdrObject *pObj = (*m_pResizeDrawObjects)[i];
                sal_uInt16 nRow = (*m_pDrawObjectPrcWidths)[3*i];
                sal_uInt16 nCol = (*m_pDrawObjectPrcWidths)[3*i+1];
                sal_uInt8 nPrcWidth = (sal_uInt8)(*m_pDrawObjectPrcWidths)[3*i+2];

                SwHTMLTableLayoutCell *pLayoutCell =
                    m_pLayoutInfo->GetCell( nRow, nCol );
                sal_uInt16 nColSpan = pLayoutCell->GetColSpan();

                sal_uInt16 nWidth2, nDummy;
                m_pLayoutInfo->GetAvail( nCol, nColSpan, nWidth2, nDummy );
                nWidth2 = static_cast< sal_uInt16 >(((long)m_nWidth * nPrcWidth) / 100);

                SwHTMLParser::ResizeDrawObject( pObj, nWidth2 );
            }
        }
    }
}

void HTMLTable::SetTable( const SwStartNode *pStNd, HTMLTableContext *pCntxt,
                          sal_uInt16 nLeft, sal_uInt16 nRight,
                          const SwTable *pSwTab, bool bFrcFrame )
{
    m_pPrevStartNode = pStNd;
    m_pSwTable = pSwTab;
    m_pContext = pCntxt;

    m_nLeftMargin = nLeft;
    m_nRightMargin = nRight;

    m_bForceFrame = bFrcFrame;
}

void HTMLTable::RegisterDrawObject( SdrObject *pObj, sal_uInt8 nPrcWidth )
{
    if( !m_pResizeDrawObjects )
        m_pResizeDrawObjects = new SdrObjects;
    m_pResizeDrawObjects->push_back( pObj );

    if( !m_pDrawObjectPrcWidths )
        m_pDrawObjectPrcWidths = new std::vector<sal_uInt16>;
    m_pDrawObjectPrcWidths->push_back( m_nCurrentRow );
    m_pDrawObjectPrcWidths->push_back( m_nCurrentColumn );
    m_pDrawObjectPrcWidths->push_back( (sal_uInt16)nPrcWidth );
}

void HTMLTable::MakeParentContents()
{
    if( !GetContext() && !HasParentSection() )
    {
        SetParentContents(
            m_pParser->InsertTableContents( GetIsParentHeader() ) );

        SetHasParentSection( true );
    }
}

void HTMLTableContext::SavePREListingXMP( SwHTMLParser& rParser )
{
    bRestartPRE = rParser.IsReadPRE();
    bRestartXMP = rParser.IsReadXMP();
    bRestartListing = rParser.IsReadListing();
    rParser.FinishPREListingXMP();
}

void HTMLTableContext::RestorePREListingXMP( SwHTMLParser& rParser )
{
    rParser.FinishPREListingXMP();

    if( bRestartPRE )
        rParser.StartPRE();

    if( bRestartXMP )
        rParser.StartXMP();

    if( bRestartListing )
        rParser.StartListing();
}

const SwStartNode *SwHTMLParser::InsertTableSection
    ( const SwStartNode *pPrevStNd )
{
    OSL_ENSURE( pPrevStNd, "Start-Node is NULL" );

    m_pCSS1Parser->SetTDTagStyles();
    SwTextFormatColl *pColl = m_pCSS1Parser->GetTextCollFromPool( RES_POOLCOLL_TABLE );

    const SwStartNode *pStNd;
    if (m_pTable->m_bFirstCell )
    {
        SwNode *const pNd = & m_pPam->GetPoint()->nNode.GetNode();
        pNd->GetTextNode()->ChgFormatColl( pColl );
        pStNd = pNd->FindTableBoxStartNode();
        m_pTable->m_bFirstCell = false;
    }
    else
    {
        const SwNode* pNd;
        if( pPrevStNd->IsTableNode() )
            pNd = pPrevStNd;
        else
            pNd = pPrevStNd->EndOfSectionNode();
        SwNodeIndex nIdx( *pNd, 1 );
        pStNd = m_xDoc->GetNodes().MakeTextSection( nIdx, SwTableBoxStartNode,
                                                  pColl );
        m_pTable->IncBoxCount();
    }

    //Added defaults to CJK and CTL
    SwContentNode *pCNd = m_xDoc->GetNodes()[pStNd->GetIndex()+1] ->GetContentNode();
    SvxFontHeightItem aFontHeight( 40, 100, RES_CHRATR_FONTSIZE );
    pCNd->SetAttr( aFontHeight );
    SvxFontHeightItem aFontHeightCJK( 40, 100, RES_CHRATR_CJK_FONTSIZE );
    pCNd->SetAttr( aFontHeightCJK );
    SvxFontHeightItem aFontHeightCTL( 40, 100, RES_CHRATR_CTL_FONTSIZE );
    pCNd->SetAttr( aFontHeightCTL );

    return pStNd;
}

const SwStartNode *SwHTMLParser::InsertTableSection( sal_uInt16 nPoolId )
{
    switch( nPoolId )
    {
    case RES_POOLCOLL_TABLE_HDLN:
        m_pCSS1Parser->SetTHTagStyles();
        break;
    case RES_POOLCOLL_TABLE:
        m_pCSS1Parser->SetTDTagStyles();
        break;
    }

    SwTextFormatColl *pColl = m_pCSS1Parser->GetTextCollFromPool( nPoolId );

    SwNode *const pNd = & m_pPam->GetPoint()->nNode.GetNode();
    const SwStartNode *pStNd;
    if (m_pTable->m_bFirstCell)
    {
        pNd->GetTextNode()->ChgFormatColl( pColl );
        m_pTable->m_bFirstCell = false;
        pStNd = pNd->FindTableBoxStartNode();
    }
    else
    {
        SwTableNode *pTableNd = pNd->FindTableNode();
        if( pTableNd->GetTable().GetHTMLTableLayout() )
        { // if there is already a HTMTableLayout, this table is already finished
          // and we have to look for the right table in the environment
            SwTableNode *pOutTable = pTableNd;
            do {
                pTableNd = pOutTable;
                pOutTable = pOutTable->StartOfSectionNode()->FindTableNode();
            } while( pOutTable && pTableNd->GetTable().GetHTMLTableLayout() );
        }
        SwNodeIndex aIdx( *pTableNd->EndOfSectionNode() );
        pStNd = m_xDoc->GetNodes().MakeTextSection( aIdx, SwTableBoxStartNode,
                                                  pColl );

        m_pPam->GetPoint()->nNode = pStNd->GetIndex() + 1;
        SwTextNode *pTextNd = m_pPam->GetPoint()->nNode.GetNode().GetTextNode();
        m_pPam->GetPoint()->nContent.Assign( pTextNd, 0 );
        m_pTable->IncBoxCount();
    }

    return pStNd;
}

SwStartNode *SwHTMLParser::InsertTempTableCaptionSection()
{
    SwTextFormatColl *pColl = m_pCSS1Parser->GetTextCollFromPool( RES_POOLCOLL_TEXT );
    SwNodeIndex& rIdx = m_pPam->GetPoint()->nNode;
    rIdx = m_xDoc->GetNodes().GetEndOfExtras();
    SwStartNode *pStNd = m_xDoc->GetNodes().MakeTextSection( rIdx,
                                          SwNormalStartNode, pColl );

    rIdx = pStNd->GetIndex() + 1;
    m_pPam->GetPoint()->nContent.Assign( rIdx.GetNode().GetTextNode(), 0 );

    return pStNd;
}

sal_Int32 SwHTMLParser::StripTrailingLF()
{
    sal_Int32 nStripped = 0;

    const sal_Int32 nLen = m_pPam->GetPoint()->nContent.GetIndex();
    if( nLen )
    {
        SwTextNode* pTextNd = m_pPam->GetPoint()->nNode.GetNode().GetTextNode();
        // careful, when comments aren't ignored!!!
        if( pTextNd )
        {
            sal_Int32 nPos = nLen;
            sal_Int32 nLFCount = 0;
            while (nPos && ('\x0a' == pTextNd->GetText()[--nPos]))
                nLFCount++;

            if( nLFCount )
            {
                if( nLFCount > 2 )
                {
                    // On Netscape, a paragraph end matches 2 LFs
                    // (1 is just a newline, 2 creates a blank line)
                    // We already have this space with the lower paragraph gap
                    // If there's a paragraph after the <BR>, we take the maximum
                    // of the gap that results from the <BR> and <P>
                    // That's why we need to delete 2 respectively all if less than 2
                    nLFCount = 2;
                }

                nPos = nLen - nLFCount;
                SwIndex nIdx( pTextNd, nPos );
                pTextNd->EraseText( nIdx, nLFCount );
                nStripped = nLFCount;
            }
        }
    }

    return nStripped;
}

SvxBrushItem* SwHTMLParser::CreateBrushItem( const Color *pColor,
                                             const OUString& rImageURL,
                                             const OUString& rStyle,
                                             const OUString& rId,
                                             const OUString& rClass )
{
    SvxBrushItem *pBrushItem = nullptr;

    if( !rStyle.isEmpty() || !rId.isEmpty() || !rClass.isEmpty() )
    {
        SfxItemSet aItemSet( m_xDoc->GetAttrPool(), svl::Items<RES_BACKGROUND,
                                                  RES_BACKGROUND>{} );
        SvxCSS1PropertyInfo aPropInfo;

        if( !rClass.isEmpty() )
        {
            OUString aClass( rClass );
            SwCSS1Parser::GetScriptFromClass( aClass );
            const SvxCSS1MapEntry *pClass = m_pCSS1Parser->GetClass( aClass );
            if( pClass )
                aItemSet.Put( pClass->GetItemSet() );
        }

        if( !rId.isEmpty() )
        {
            const SvxCSS1MapEntry *pId = m_pCSS1Parser->GetId( rId );
            if( pId )
                aItemSet.Put( pId->GetItemSet() );
        }

        m_pCSS1Parser->ParseStyleOption( rStyle, aItemSet, aPropInfo );
        const SfxPoolItem *pItem = nullptr;
        if( SfxItemState::SET == aItemSet.GetItemState( RES_BACKGROUND, false,
                                                   &pItem ) )
        {
            pBrushItem = new SvxBrushItem( *static_cast<const SvxBrushItem *>(pItem) );
        }
    }

    if( !pBrushItem && (pColor || !rImageURL.isEmpty()) )
    {
        pBrushItem = new SvxBrushItem(RES_BACKGROUND);

        if( pColor )
            pBrushItem->SetColor(*pColor);

        if( !rImageURL.isEmpty() )
        {
            pBrushItem->SetGraphicLink( URIHelper::SmartRel2Abs( INetURLObject(m_sBaseURL), rImageURL, Link<OUString *, bool>(), false) );
            pBrushItem->SetGraphicPos( GPOS_TILED );
        }
    }

    return pBrushItem;
}

class SectionSaveStruct : public SwPendingStackData
{
    sal_uInt16 m_nBaseFontStMinSave, m_nFontStMinSave, m_nFontStHeadStartSave;
    sal_uInt16 m_nDefListDeepSave;
    size_t m_nContextStMinSave;
    size_t m_nContextStAttrMinSave;

public:

    HTMLTable *m_pTable;

    explicit SectionSaveStruct( SwHTMLParser& rParser );

#if OSL_DEBUG_LEVEL > 0
    size_t GetContextStAttrMin() const { return m_nContextStAttrMinSave; }
#endif
    void Restore( SwHTMLParser& rParser );
};

SectionSaveStruct::SectionSaveStruct( SwHTMLParser& rParser ) :
    m_nBaseFontStMinSave(0), m_nFontStMinSave(0), m_nFontStHeadStartSave(0),
    m_nDefListDeepSave(0), m_nContextStMinSave(0), m_nContextStAttrMinSave(0),
    m_pTable( nullptr )
{
    // Freeze font stacks
    m_nBaseFontStMinSave = rParser.m_nBaseFontStMin;
    rParser.m_nBaseFontStMin = rParser.m_aBaseFontStack.size();

    m_nFontStMinSave = rParser.m_nFontStMin;
    m_nFontStHeadStartSave = rParser.m_nFontStHeadStart;
    rParser.m_nFontStMin = rParser.m_aFontStack.size();

    // Freeze context stack
    m_nContextStMinSave = rParser.m_nContextStMin;
    m_nContextStAttrMinSave = rParser.m_nContextStAttrMin;
    rParser.m_nContextStMin = rParser.m_aContexts.size();
    rParser.m_nContextStAttrMin = rParser.m_nContextStMin;

    // And remember a few counters
    m_nDefListDeepSave = rParser.m_nDefListDeep;
    rParser.m_nDefListDeep = 0;
}

void SectionSaveStruct::Restore( SwHTMLParser& rParser )
{
    // Unfreeze font stacks
    sal_uInt16 nMin = rParser.m_nBaseFontStMin;
    if( rParser.m_aBaseFontStack.size() > nMin )
        rParser.m_aBaseFontStack.erase( rParser.m_aBaseFontStack.begin() + nMin,
                rParser.m_aBaseFontStack.end() );
    rParser.m_nBaseFontStMin = m_nBaseFontStMinSave;

    nMin = rParser.m_nFontStMin;
    if( rParser.m_aFontStack.size() > nMin )
        rParser.m_aFontStack.erase( rParser.m_aFontStack.begin() + nMin,
                rParser.m_aFontStack.end() );
    rParser.m_nFontStMin = m_nFontStMinSave;
    rParser.m_nFontStHeadStart = m_nFontStHeadStartSave;

    OSL_ENSURE( rParser.m_aContexts.size() == rParser.m_nContextStMin &&
            rParser.m_aContexts.size() == rParser.m_nContextStAttrMin,
            "The Context Stack was not cleaned up" );
    rParser.m_nContextStMin = m_nContextStMinSave;
    rParser.m_nContextStAttrMin = m_nContextStAttrMinSave;

    // Reconstruct a few counters
    rParser.m_nDefListDeep = m_nDefListDeepSave;

    // Reset a few flags
    rParser.m_bNoParSpace = false;
    rParser.m_nOpenParaToken = HtmlTokenId::NONE;

    if( !rParser.m_aParaAttrs.empty() )
        rParser.m_aParaAttrs.clear();
}

class CellSaveStruct : public SectionSaveStruct
{
    OUString m_aStyle, m_aId, m_aClass, m_aLang, m_aDir;
    OUString m_aBGImage;
    Color m_aBGColor;
    std::shared_ptr<SvxBoxItem> m_pBoxItem;

    HTMLTableCnts* m_pCnts;           // List of all contents
    HTMLTableCnts* m_pCurrCnts;   // current content or 0
    std::unique_ptr<SwNodeIndex> m_pNoBreakEndNodeIndex;// Paragraph index of a <NOBR>

    double m_nValue;

    sal_uInt32 m_nNumFormat;

    sal_uInt16 m_nRowSpan, m_nColSpan, m_nWidth, m_nHeight;
    sal_Int32 m_nNoBreakEndContentPos;     // Character index of a <NOBR>

    SvxAdjust m_eAdjust;
    sal_Int16 m_eVertOri;

    bool m_bHead : 1;
    bool m_bPrcWidth : 1;
    bool m_bHasNumFormat : 1;
    bool m_bHasValue : 1;
    bool m_bBGColor : 1;
    bool m_bNoWrap : 1;       // NOWRAP option
    bool m_bNoBreak : 1;      // NOBREAK tag

public:

    CellSaveStruct( SwHTMLParser& rParser, HTMLTable *pCurTable, bool bHd,
                     bool bReadOpt );

    void AddContents( HTMLTableCnts *pNewCnts );
    HTMLTableCnts *GetFirstContents() { return m_pCnts; }

    void ClearIsInSection() { m_pCurrCnts = nullptr; }
    bool IsInSection() const { return m_pCurrCnts!=nullptr; }

    void InsertCell( SwHTMLParser& rParser, HTMLTable *pCurTable );

    bool IsHeaderCell() const { return m_bHead; }

    void StartNoBreak( const SwPosition& rPos );
    void EndNoBreak( const SwPosition& rPos );
    void CheckNoBreak( const SwPosition& rPos );
};

CellSaveStruct::CellSaveStruct( SwHTMLParser& rParser, HTMLTable *pCurTable,
                                  bool bHd, bool bReadOpt ) :
    SectionSaveStruct( rParser ),
    m_pCnts( nullptr ),
    m_pCurrCnts( nullptr ),
    m_pNoBreakEndNodeIndex( nullptr ),
    m_nValue( 0.0 ),
    m_nNumFormat( 0 ),
    m_nRowSpan( 1 ),
    m_nColSpan( 1 ),
    m_nWidth( 0 ),
    m_nHeight( 0 ),
    m_nNoBreakEndContentPos( 0 ),
    m_eAdjust( pCurTable->GetInheritedAdjust() ),
    m_eVertOri( pCurTable->GetInheritedVertOri() ),
    m_bHead( bHd ),
    m_bPrcWidth( false ),
    m_bHasNumFormat( false ),
    m_bHasValue( false ),
    m_bBGColor( false ),
    m_bNoWrap( false ),
    m_bNoBreak( false )
{
    OUString aNumFormat, aValue;

    if( bReadOpt )
    {
        const HTMLOptions& rOptions = rParser.GetOptions();
        for (size_t i = rOptions.size(); i; )
        {
            const HTMLOption& rOption = rOptions[--i];
            switch( rOption.GetToken() )
            {
            case HtmlOptionId::ID:
                m_aId = rOption.GetString();
                break;
            case HtmlOptionId::COLSPAN:
                m_nColSpan = (sal_uInt16)rOption.GetNumber();
                break;
            case HtmlOptionId::ROWSPAN:
                m_nRowSpan = (sal_uInt16)rOption.GetNumber();
                break;
            case HtmlOptionId::ALIGN:
                m_eAdjust = rOption.GetEnum( aHTMLPAlignTable, m_eAdjust );
                break;
            case HtmlOptionId::VALIGN:
                m_eVertOri = rOption.GetEnum( aHTMLTableVAlignTable, m_eVertOri );
                break;
            case HtmlOptionId::WIDTH:
                m_nWidth = (sal_uInt16)rOption.GetNumber();   // Just for Netscape
                m_bPrcWidth = (rOption.GetString().indexOf('%') != -1);
                if( m_bPrcWidth && m_nWidth>100 )
                    m_nWidth = 100;
                break;
            case HtmlOptionId::HEIGHT:
                m_nHeight = (sal_uInt16)rOption.GetNumber();  // Just for Netscape
                if( rOption.GetString().indexOf('%') != -1)
                    m_nHeight = 0;    // don't consider % attributes
                break;
            case HtmlOptionId::BGCOLOR:
                // Ignore empty BGCOLOR on <TABLE>, <TR> and <TD>/<TH> like Netscape
                // *really* not on other tags
                if( !rOption.GetString().isEmpty() )
                {
                    rOption.GetColor( m_aBGColor );
                    m_bBGColor = true;
                }
                break;
            case HtmlOptionId::BACKGROUND:
                m_aBGImage = rOption.GetString();
                break;
            case HtmlOptionId::STYLE:
                m_aStyle = rOption.GetString();
                break;
            case HtmlOptionId::CLASS:
                m_aClass = rOption.GetString();
                break;
            case HtmlOptionId::LANG:
                m_aLang = rOption.GetString();
                break;
            case HtmlOptionId::DIR:
                m_aDir = rOption.GetString();
                break;
            case HtmlOptionId::SDNUM:
                aNumFormat = rOption.GetString();
                m_bHasNumFormat = true;
                break;
            case HtmlOptionId::SDVAL:
                m_bHasValue = true;
                aValue = rOption.GetString();
                break;
            case HtmlOptionId::NOWRAP:
                m_bNoWrap = true;
                break;
            default: break;
            }
        }

        if( !m_aId.isEmpty() )
            rParser.InsertBookmark( m_aId );
    }

    if( m_bHasNumFormat )
    {
        LanguageType eLang;
        m_nValue = SfxHTMLParser::GetTableDataOptionsValNum(
                            m_nNumFormat, eLang, aValue, aNumFormat,
                            *rParser.m_xDoc->GetNumberFormatter() );
    }

    // Create a new context but don't anchor the drawing::Alignment attribute there,
    // since there's no section yet
    HtmlTokenId nToken;
    sal_uInt16 nColl;
    if( m_bHead )
    {
        nToken = HtmlTokenId::TABLEHEADER_ON;
        nColl = RES_POOLCOLL_TABLE_HDLN;
    }
    else
    {
        nToken = HtmlTokenId::TABLEDATA_ON;
        nColl = RES_POOLCOLL_TABLE;
    }
    HTMLAttrContext *pCntxt = new HTMLAttrContext( nToken, nColl, aEmptyOUStr, true );
    if( SvxAdjust::End != m_eAdjust )
        rParser.InsertAttr( &rParser.m_aAttrTab.pAdjust, SvxAdjustItem(m_eAdjust, RES_PARATR_ADJUST),
                            pCntxt );

    if( SwHTMLParser::HasStyleOptions( m_aStyle, m_aId, m_aClass, &m_aLang, &m_aDir ) )
    {
        SfxItemSet aItemSet( rParser.m_xDoc->GetAttrPool(),
                             rParser.m_pCSS1Parser->GetWhichMap() );
        SvxCSS1PropertyInfo aPropInfo;

        if( rParser.ParseStyleOptions( m_aStyle, m_aId, m_aClass, aItemSet,
                                       aPropInfo, &m_aLang, &m_aDir ) )
        {
            SfxPoolItem const* pItem;
            if (SfxItemState::SET == aItemSet.GetItemState(RES_BOX, false, &pItem))
            {   // fdo#41796: steal box item to set it in FixFrameFormat later!
                m_pBoxItem.reset(dynamic_cast<SvxBoxItem *>(pItem->Clone()));
                aItemSet.ClearItem(RES_BOX);
            }
            rParser.InsertAttrs( aItemSet, aPropInfo, pCntxt );
        }
    }

    rParser.SplitPREListingXMP( pCntxt );

    rParser.PushContext( pCntxt );
}

void CellSaveStruct::AddContents( HTMLTableCnts *pNewCnts )
{
    if( m_pCnts )
        m_pCnts->Add( pNewCnts );
    else
        m_pCnts = pNewCnts;

    m_pCurrCnts = pNewCnts;
}

void CellSaveStruct::InsertCell( SwHTMLParser& rParser,
                                  HTMLTable *pCurTable )
{
#if OSL_DEBUG_LEVEL > 0
    // The attributes need to have been removed when tidying up the context stack,
    // Otherwise something's wrong. Let's check that...

    // MIB 8.1.98: When attributes were opened outside of a cell,
    // they're still in the attribut table and will only be deleted at the end
    // through the CleanContext calls in BuildTable. We don't check that there
    // so that we get no assert [violations, by translator]
    // We can see this on nContextStAttrMin: the remembered value of nContextStAttrMinSave
    // is the value that nContextStAttrMin had at the start of the table. And the
    // current value of nContextStAttrMin corresponds to the number of contexts
    // we found at the start of the cell. If the values differ, contexts
    // were created and we don't check anything.

    if( rParser.m_nContextStAttrMin == GetContextStAttrMin() )
    {
        HTMLAttr** pTable = reinterpret_cast<HTMLAttr**>(&rParser.m_aAttrTab);

        for( auto nCnt = sizeof( HTMLAttrTable ) / sizeof( HTMLAttr* );
            nCnt--; ++pTable )
        {
            OSL_ENSURE( !*pTable, "The attribute table isn't empty" );
        }
    }
#endif

    // we need to add the cell on the current position
    SvxBrushItem *pBrushItem =
        rParser.CreateBrushItem( m_bBGColor ? &m_aBGColor : nullptr, m_aBGImage,
                                 m_aStyle, m_aId, m_aClass );
    pCurTable->InsertCell( m_pCnts, m_nRowSpan, m_nColSpan, m_nWidth,
                           m_bPrcWidth, m_nHeight, m_eVertOri, pBrushItem, m_pBoxItem,
                           m_bHasNumFormat, m_nNumFormat, m_bHasValue, m_nValue,
                           m_bNoWrap );
    Restore( rParser );
}

void CellSaveStruct::StartNoBreak( const SwPosition& rPos )
{
    if( !m_pCnts ||
        (!rPos.nContent.GetIndex() && m_pCurrCnts==m_pCnts &&
         m_pCnts->GetStartNode() &&
         m_pCnts->GetStartNode()->GetIndex() + 1 ==
            rPos.nNode.GetIndex()) )
    {
        m_bNoBreak = true;
    }
}

void CellSaveStruct::EndNoBreak( const SwPosition& rPos )
{
    if( m_bNoBreak )
    {
        m_pNoBreakEndNodeIndex.reset( new SwNodeIndex( rPos.nNode ) );
        m_nNoBreakEndContentPos = rPos.nContent.GetIndex();
        m_bNoBreak = false;
    }
}

void CellSaveStruct::CheckNoBreak( const SwPosition& rPos )
{
    if( m_pCnts && m_pCurrCnts==m_pCnts )
    {
        if( m_bNoBreak )
        {
            // <NOBR> wasn't closed
            m_pCnts->SetNoBreak();
        }
        else if( m_pNoBreakEndNodeIndex &&
                 m_pNoBreakEndNodeIndex->GetIndex() == rPos.nNode.GetIndex() )
        {
            if( m_nNoBreakEndContentPos == rPos.nContent.GetIndex() )
            {
                // <NOBR> was closed immediately before the cell end
                m_pCnts->SetNoBreak();
            }
            else if( m_nNoBreakEndContentPos + 1 == rPos.nContent.GetIndex() )
            {
                SwTextNode const*const pTextNd(rPos.nNode.GetNode().GetTextNode());
                if( pTextNd )
                {
                    sal_Unicode const cLast =
                            pTextNd->GetText()[m_nNoBreakEndContentPos];
                    if( ' '==cLast || '\x0a'==cLast )
                    {
                        // There's just a blank or a newline between the <NOBR> and the cell end
                        m_pCnts->SetNoBreak();
                    }
                }
            }
        }
    }
}

HTMLTableCnts *SwHTMLParser::InsertTableContents(
                                        bool bHead )
{
    // create a new section, the PaM is gonna be there
    const SwStartNode *pStNd =
        InsertTableSection( static_cast< sal_uInt16 >(bHead ? RES_POOLCOLL_TABLE_HDLN
                                           : RES_POOLCOLL_TABLE) );

    if( GetNumInfo().GetNumRule() )
    {
        // Set the first paragraph to non-enumerated
        sal_uInt8 nLvl = GetNumInfo().GetLevel();

        SetNodeNum( nLvl );
    }

    // Reset attributation start
    const SwNodeIndex& rSttPara = m_pPam->GetPoint()->nNode;
    sal_Int32 nSttCnt = m_pPam->GetPoint()->nContent.GetIndex();

    HTMLAttr** pHTMLAttributes = reinterpret_cast<HTMLAttr**>(&m_aAttrTab);
    for (sal_uInt16 nCnt = sizeof(HTMLAttrTable) / sizeof(HTMLAttr*); nCnt--; ++pHTMLAttributes)
    {
        HTMLAttr *pAttr = *pHTMLAttributes;
        while( pAttr )
        {
            OSL_ENSURE( !pAttr->GetPrev(), "Attribute has previous list" );
            pAttr->nSttPara = rSttPara;
            pAttr->nEndPara = rSttPara;
            pAttr->nSttContent = nSttCnt;
            pAttr->nEndContent = nSttCnt;

            pAttr = pAttr->GetNext();
        }
    }

    return new HTMLTableCnts( pStNd );
}

sal_uInt16 SwHTMLParser::IncGrfsThatResizeTable()
{
    return m_pTable ? m_pTable->IncGrfsThatResize() : 0;
}

void SwHTMLParser::RegisterDrawObjectToTable( HTMLTable *pCurTable,
                                        SdrObject *pObj, sal_uInt8 nPrcWidth )
{
    pCurTable->RegisterDrawObject( pObj, nPrcWidth );
}

void SwHTMLParser::BuildTableCell( HTMLTable *pCurTable, bool bReadOptions,
                                   bool bHead )
{
    if( !IsParserWorking() && !m_pPendStack )
        return;

    CellSaveStruct* pSaveStruct;

    HtmlTokenId nToken = HtmlTokenId::NONE;
    bool bPending = false;
    if( m_pPendStack )
    {
        pSaveStruct = static_cast<CellSaveStruct*>(m_pPendStack->pData);

        SwPendingStack* pTmp = m_pPendStack->pNext;
        delete m_pPendStack;
        m_pPendStack = pTmp;
        nToken = m_pPendStack ? m_pPendStack->nToken : GetSaveToken();
        bPending = SvParserState::Error == eState && m_pPendStack != nullptr;

        SaveState( nToken );
    }
    else
    {
        // <TH> resp. <TD> were already read
        if( m_pTable->IsOverflowing() )
        {
            SaveState( HtmlTokenId::NONE );
            return;
        }

        if( !pCurTable->GetContext() )
        {
            bool bTopTable = m_pTable==pCurTable;

            // the table has no content yet, this means the actual table needs
            // to be created first

            static sal_uInt16 aWhichIds[] =
            {
                RES_PARATR_SPLIT,   RES_PARATR_SPLIT,
                RES_PAGEDESC,       RES_PAGEDESC,
                RES_BREAK,          RES_BREAK,
                RES_BACKGROUND,     RES_BACKGROUND,
                RES_KEEP,           RES_KEEP,
                RES_LAYOUT_SPLIT,   RES_LAYOUT_SPLIT,
                RES_FRAMEDIR,       RES_FRAMEDIR,
                0
            };

            SfxItemSet aItemSet( m_xDoc->GetAttrPool(), aWhichIds );
            SvxCSS1PropertyInfo aPropInfo;

            bool bStyleParsed = ParseStyleOptions( pCurTable->GetStyle(),
                                                   pCurTable->GetId(),
                                                   pCurTable->GetClass(),
                                                   aItemSet, aPropInfo,
                                                      nullptr, &pCurTable->GetDirection() );
            const SfxPoolItem *pItem = nullptr;
            if( bStyleParsed )
            {
                if( SfxItemState::SET == aItemSet.GetItemState(
                                        RES_BACKGROUND, false, &pItem ) )
                {
                    pCurTable->SetBGBrush( *static_cast<const SvxBrushItem *>(pItem) );
                    aItemSet.ClearItem( RES_BACKGROUND );
                }
                if( SfxItemState::SET == aItemSet.GetItemState(
                                        RES_PARATR_SPLIT, false, &pItem ) )
                {
                    aItemSet.Put(
                        SwFormatLayoutSplit( static_cast<const SvxFormatSplitItem *>(pItem)
                                                ->GetValue() ) );
                    aItemSet.ClearItem( RES_PARATR_SPLIT );
                }
            }

            sal_uInt16 nLeftSpace = 0;
            sal_uInt16 nRightSpace = 0;
            short nIndent;
            GetMarginsFromContextWithNumBul( nLeftSpace, nRightSpace, nIndent );

            // save the current position we'll get back to some time
            SwPosition *pSavePos = nullptr;
            bool bForceFrame = false;
            bool bAppended = false;
            bool bParentLFStripped = false;
            if( bTopTable )
            {
                SvxAdjust eTableAdjust = m_pTable->GetTableAdjust(false);

                // If the table is left or right adjusted or should be in a text frame,
                // it'll get one
                bForceFrame = eTableAdjust == SvxAdjust::Left ||
                              eTableAdjust == SvxAdjust::Right ||
                              pCurTable->HasToFly();

                // The table either shouldn't get in a text frame and isn't in one
                // (it gets simulated through cells),
                // or there's already content at that position
                OSL_ENSURE( !bForceFrame || pCurTable->HasParentSection(),
                        "table in frame has no parent!" );

                bool bAppend = false;
                if( bForceFrame )
                {
                    // If the table gets in a border, we only need to open a new
                    //paragraph if the paragraph has text frames that don't fly
                    bAppend = HasCurrentParaFlys(true);
                }
                else
                {
                    // Otherwise, we need to open a new paragraph if the paragraph
                    // is empty or contains text frames or bookmarks
                    bAppend =
                        m_pPam->GetPoint()->nContent.GetIndex() ||
                        HasCurrentParaFlys() ||
                        HasCurrentParaBookmarks();
                }
                if( bAppend )
                {
                    if( !m_pPam->GetPoint()->nContent.GetIndex() )
                    {
                        //Set default to CJK and CTL
                        m_xDoc->SetTextFormatColl( *m_pPam,
                            m_pCSS1Parser->GetTextCollFromPool(RES_POOLCOLL_STANDARD) );
                        SvxFontHeightItem aFontHeight( 40, 100, RES_CHRATR_FONTSIZE );

                        HTMLAttr* pTmp =
                            new HTMLAttr( *m_pPam->GetPoint(), aFontHeight );
                        m_aSetAttrTab.push_back( pTmp );

                        SvxFontHeightItem aFontHeightCJK( 40, 100, RES_CHRATR_CJK_FONTSIZE );
                        pTmp =
                            new HTMLAttr( *m_pPam->GetPoint(), aFontHeightCJK );
                        m_aSetAttrTab.push_back( pTmp );

                        SvxFontHeightItem aFontHeightCTL( 40, 100, RES_CHRATR_CTL_FONTSIZE );
                        pTmp =
                            new HTMLAttr( *m_pPam->GetPoint(), aFontHeightCTL );
                        m_aSetAttrTab.push_back( pTmp );

                        pTmp = new HTMLAttr( *m_pPam->GetPoint(),
                                            SvxULSpaceItem( 0, 0, RES_UL_SPACE ) );
                        m_aSetAttrTab.push_front( pTmp ); // Position 0, since
                                                          // something can be set by
                                                          // the table end before
                    }
                    AppendTextNode( AM_NOSPACE );
                    bAppended = true;
                }
                else if( !m_aParaAttrs.empty() )
                {
                    if( !bForceFrame )
                    {
                        // The paragraph will be moved right behind the table.
                        // That's why we remove all hard attributes of that paragraph

                        for(HTMLAttr* i : m_aParaAttrs)
                            i->Invalidate();
                    }

                    m_aParaAttrs.clear();
                }

                pSavePos = new SwPosition( *m_pPam->GetPoint() );
            }
            else if( pCurTable->HasParentSection() )
            {
                bParentLFStripped = StripTrailingLF() > 0;

                // Close paragraph resp. headers
                m_nOpenParaToken = HtmlTokenId::NONE;
                m_nFontStHeadStart = m_nFontStMin;

                // The hard attributes on that paragraph are never gonna be invalid anymore
                if( !m_aParaAttrs.empty() )
                    m_aParaAttrs.clear();
            }

            // create a table context
            HTMLTableContext *pTCntxt =
                        new HTMLTableContext( pSavePos, m_nContextStMin,
                                               m_nContextStAttrMin );

            // end all open attributes and open them again behind the table
            HTMLAttrs *pPostIts = nullptr;
            if( !bForceFrame && (bTopTable || pCurTable->HasParentSection()) )
            {
                SplitAttrTab( pTCntxt->aAttrTab, bTopTable );
                // If we reuse a already existing paragraph, we can't add
                // PostIts since the paragraph gets behind that table.
                // They're gonna be moved into the first paragraph of the table
                // If we have tables in tables, we also can't add PostIts to a
                // still empty paragraph, since it's not gonna be deleted that way
                if( (bTopTable && !bAppended) ||
                    (!bTopTable && !bParentLFStripped &&
                     !m_pPam->GetPoint()->nContent.GetIndex()) )
                    pPostIts = new HTMLAttrs;
                SetAttr( bTopTable, bTopTable, pPostIts );
            }
            else
            {
                SaveAttrTab( pTCntxt->aAttrTab );
                if( bTopTable && !bAppended )
                {
                    pPostIts = new HTMLAttrs;
                    SetAttr( true, true, pPostIts );
                }
            }
            m_bNoParSpace = false;

            // Save current numbering and turn it off
            pTCntxt->SetNumInfo( GetNumInfo() );
            GetNumInfo().Clear();
            pTCntxt->SavePREListingXMP( *this );

            if( bTopTable )
            {
                if( bForceFrame )
                {
                    // the table should be put in a text frame

                    SfxItemSet aFrameSet( m_xDoc->GetAttrPool(),
                                        svl::Items<RES_FRMATR_BEGIN, RES_FRMATR_END-1>{} );
                    if( !pCurTable->IsNewDoc() )
                        Reader::ResetFrameFormatAttrs( aFrameSet );

                    css::text::WrapTextMode eSurround = css::text::WrapTextMode_NONE;
                    sal_Int16 eHori;

                    switch( pCurTable->GetTableAdjust(true) )
                    {
                    case SvxAdjust::Right:
                        eHori = text::HoriOrientation::RIGHT;
                        eSurround = css::text::WrapTextMode_LEFT;
                        break;
                    case SvxAdjust::Center:
                        eHori = text::HoriOrientation::CENTER;
                        break;
                    case SvxAdjust::Left:
                        eSurround = css::text::WrapTextMode_RIGHT;
                        SAL_FALLTHROUGH;
                    default:
                        eHori = text::HoriOrientation::LEFT;
                        break;
                    }
                    SetAnchorAndAdjustment( text::VertOrientation::NONE, eHori, aFrameSet,
                                            true );
                    aFrameSet.Put( SwFormatSurround(eSurround) );

                    SwFormatFrameSize aFrameSize( ATT_VAR_SIZE, 20*MM50, MINLAY );
                    aFrameSize.SetWidthPercent( 100 );
                    aFrameSet.Put( aFrameSize );

                    sal_uInt16 nSpace = pCurTable->GetHSpace();
                    if( nSpace )
                        aFrameSet.Put( SvxLRSpaceItem(nSpace,nSpace, 0, 0, RES_LR_SPACE) );
                    nSpace = pCurTable->GetVSpace();
                    if( nSpace )
                        aFrameSet.Put( SvxULSpaceItem(nSpace,nSpace, RES_UL_SPACE) );

                    RndStdIds eAnchorId = static_cast<const SwFormatAnchor&>(aFrameSet.
                                                Get( RES_ANCHOR )).
                                                GetAnchorId();
                    SwFrameFormat *pFrameFormat =  m_xDoc->MakeFlySection(
                                eAnchorId, m_pPam->GetPoint(), &aFrameSet );

                    pTCntxt->SetFrameFormat( pFrameFormat );
                    const SwFormatContent& rFlyContent = pFrameFormat->GetContent();
                    m_pPam->GetPoint()->nNode = *rFlyContent.GetContentIdx();
                    SwContentNode *pCNd =
                        m_xDoc->GetNodes().GoNext( &(m_pPam->GetPoint()->nNode) );
                    m_pPam->GetPoint()->nContent.Assign( pCNd, 0 );

                }

                // create a SwTable with a box and set the PaM to the content of
                // the box section (the adjustment parameter is a dummy for now
                // and will be corrected later)
                OSL_ENSURE( !m_pPam->GetPoint()->nContent.GetIndex(),
                        "The paragraph after the table is not empty!" );
                const SwTable* pSwTable = m_xDoc->InsertTable(
                        SwInsertTableOptions( tabopts::HEADLINE_NO_BORDER, 1 ),
                        *m_pPam->GetPoint(), 1, 1, text::HoriOrientation::LEFT );

                if( bForceFrame )
                {
                    SwNodeIndex aDstIdx( m_pPam->GetPoint()->nNode );
                    m_pPam->Move( fnMoveBackward );
                    m_xDoc->GetNodes().Delete( aDstIdx );
                }
                else
                {
                    if( bStyleParsed )
                    {
                        m_pCSS1Parser->SetFormatBreak( aItemSet, aPropInfo );
                        pSwTable->GetFrameFormat()->SetFormatAttr( aItemSet );
                    }
                    m_pPam->Move( fnMoveBackward );
                }

                SwNode const*const pNd = & m_pPam->GetPoint()->nNode.GetNode();
                if( !bAppended && !bForceFrame )
                {
                    SwTextNode *const pOldTextNd =
                        pSavePos->nNode.GetNode().GetTextNode();
                    OSL_ENSURE( pOldTextNd, "Why we aren't in a text node?" );
                    SwFrameFormat *pFrameFormat = pSwTable->GetFrameFormat();

                    const SfxPoolItem* pItem2;
                    if( SfxItemState::SET == pOldTextNd->GetSwAttrSet()
                            .GetItemState( RES_PAGEDESC, false, &pItem2 ) &&
                        static_cast<const SwFormatPageDesc *>(pItem2)->GetPageDesc() )
                    {
                        pFrameFormat->SetFormatAttr( *pItem2 );
                        pOldTextNd->ResetAttr( RES_PAGEDESC );
                    }
                    if( SfxItemState::SET == pOldTextNd->GetSwAttrSet()
                            .GetItemState( RES_BREAK, true, &pItem2 ) )
                    {
                        switch( static_cast<const SvxFormatBreakItem *>(pItem2)->GetBreak() )
                        {
                        case SvxBreak::PageBefore:
                        case SvxBreak::PageAfter:
                        case SvxBreak::PageBoth:
                            pFrameFormat->SetFormatAttr( *pItem2 );
                            pOldTextNd->ResetAttr( RES_BREAK );
                            break;
                        default:
                            break;
                        }
                    }
                }

                if( !bAppended && pPostIts )
                {
                    // set still-existing PostIts to the first paragraph of the table
                    InsertAttrs( *pPostIts );
                    delete pPostIts;
                    pPostIts = nullptr;
                }

                pTCntxt->SetTableNode( const_cast<SwTableNode *>(pNd->FindTableNode()) );

                pCurTable->SetTable( pTCntxt->GetTableNode(), pTCntxt,
                                     nLeftSpace, nRightSpace,
                                     pSwTable, bForceFrame );

                OSL_ENSURE( !pPostIts, "unused PostIts" );
            }
            else
            {
                // still open sections need to be deleted
                if( EndSections( bParentLFStripped ) )
                    bParentLFStripped = false;

                if( pCurTable->HasParentSection() )
                {
                    // after that, we remove a possibly redundant empty paragraph,
                    // but only if it was empty before we stripped the LFs
                    if( !bParentLFStripped )
                        StripTrailingPara();

                    if( pPostIts )
                    {
                        // move still existing PostIts to the end of the current paragraph
                        InsertAttrs( *pPostIts );
                        delete pPostIts;
                        pPostIts = nullptr;
                    }
                }

                SwNode const*const pNd = & m_pPam->GetPoint()->nNode.GetNode();
                const SwStartNode *pStNd = (m_pTable->m_bFirstCell ? pNd->FindTableNode()
                                                            : pNd->FindTableBoxStartNode() );

                pCurTable->SetTable( pStNd, pTCntxt, nLeftSpace, nRightSpace );
            }

            // Freeze the context stack, since there could be attributes set
            // outside of cells. Can't happen earlier, since there may be
            // searches in the stack
            m_nContextStMin = m_aContexts.size();
            m_nContextStAttrMin = m_nContextStMin;
        }

        pSaveStruct = new CellSaveStruct( *this, pCurTable, bHead,
                                            bReadOptions );

        // If the first GetNextToken() doesn't succeed (pending input), must re-read from the beginning.
        SaveState( HtmlTokenId::NONE );
    }

    if( nToken == HtmlTokenId::NONE )
        nToken = GetNextToken();    // Token after <TABLE>

    bool bDone = false;
    while( (IsParserWorking() && !bDone) || bPending )
    {
        SaveState( nToken );

        nToken = FilterToken( nToken );

        OSL_ENSURE( m_pPendStack || !m_bCallNextToken || pSaveStruct->IsInSection(),
                "Where is the section??" );
        if( !m_pPendStack && m_bCallNextToken && pSaveStruct->IsInSection() )
        {
            // Call NextToken directly (e.g. ignore the content of floating frames or applets)
            NextToken( nToken );
        }
        else switch( nToken )
        {
        case HtmlTokenId::TABLEHEADER_ON:
        case HtmlTokenId::TABLEDATA_ON:
        case HtmlTokenId::TABLEROW_ON:
        case HtmlTokenId::TABLEROW_OFF:
        case HtmlTokenId::THEAD_ON:
        case HtmlTokenId::THEAD_OFF:
        case HtmlTokenId::TFOOT_ON:
        case HtmlTokenId::TFOOT_OFF:
        case HtmlTokenId::TBODY_ON:
        case HtmlTokenId::TBODY_OFF:
        case HtmlTokenId::TABLE_OFF:
            SkipToken();
            SAL_FALLTHROUGH;
        case HtmlTokenId::TABLEHEADER_OFF:
        case HtmlTokenId::TABLEDATA_OFF:
            bDone = true;
            break;
        case HtmlTokenId::TABLE_ON:
            {
                bool bHasToFly = false;
                SvxAdjust eTabAdjust = SvxAdjust::End;
                if( !m_pPendStack )
                {
                    // only if we create a new table, but not if we're still
                    // reading in the table after a Pending
                    pSaveStruct->m_pTable = m_pTable;

                    // HACK: create a section for a table that goes in a text frame
                    if( !pSaveStruct->IsInSection() )
                    {
                        // The loop needs to be forward, since the
                        // first option always wins
                        bool bNeedsSection = false;
                        const HTMLOptions& rHTMLOptions = GetOptions();
                        for (const auto & rOption : rHTMLOptions)
                        {
                            if( HtmlOptionId::ALIGN==rOption.GetToken() )
                            {
                                SvxAdjust eAdjust = rOption.GetEnum( aHTMLPAlignTable, SvxAdjust::End );
                                bNeedsSection = SvxAdjust::Left == eAdjust ||
                                                SvxAdjust::Right == eAdjust;
                                break;
                            }
                        }
                        if( bNeedsSection )
                        {
                            pSaveStruct->AddContents(
                                InsertTableContents(bHead  ) );
                        }
                    }
                    else
                    {
                        // If Flys are anchored in the current paragraph,
                        // the table needs to get in a text frame
                        bHasToFly = HasCurrentParaFlys(false,true);
                    }

                    // There could be a section in the cell
                    eTabAdjust = m_aAttrTab.pAdjust
                        ? static_cast<const SvxAdjustItem&>(m_aAttrTab.pAdjust->GetItem()).
                                                 GetAdjust()
                        : SvxAdjust::End;
                }

                HTMLTable *pSubTable = BuildTable( eTabAdjust,
                                                   bHead,
                                                   pSaveStruct->IsInSection(),
                                                   bHasToFly );
                if( SvParserState::Pending != GetStatus() )
                {
                    // Only if the table is really complete
                    if( pSubTable )
                    {
                        OSL_ENSURE( pSubTable->GetTableAdjust(false)!= SvxAdjust::Left &&
                                pSubTable->GetTableAdjust(false)!= SvxAdjust::Right,
                                "left or right aligned tables belong in frames" );

                        HTMLTableCnts *pParentContents =
                            pSubTable->GetParentContents();
                        if( pParentContents )
                        {
                            OSL_ENSURE( !pSaveStruct->IsInSection(),
                                    "Where is the section" );

                            // If there's no table coming, we have a section
                            pSaveStruct->AddContents( pParentContents );
                        }

                        const SwStartNode *pCapStNd =
                                pSubTable->GetCaptionStartNode();

                        if( pSubTable->GetContext() )
                        {
                            OSL_ENSURE( !pSubTable->GetContext()->GetFrameFormat(),
                                    "table in frame" );

                            if( pCapStNd && pSubTable->IsTopCaption() )
                            {
                                pSaveStruct->AddContents(
                                    new HTMLTableCnts(pCapStNd) );
                            }

                            pSaveStruct->AddContents(
                                new HTMLTableCnts(pSubTable) );

                            if( pCapStNd && !pSubTable->IsTopCaption() )
                            {
                                pSaveStruct->AddContents(
                                    new HTMLTableCnts(pCapStNd) );
                            }

                            // We don't have a section anymore
                            pSaveStruct->ClearIsInSection();
                        }
                        else if( pCapStNd )
                        {
                            // Since we can't delete this section (it might
                            // belong to the first box), we'll add it
                            pSaveStruct->AddContents(
                                new HTMLTableCnts(pCapStNd) );

                            // We don't have a section anymore
                            pSaveStruct->ClearIsInSection();
                        }
                    }

                    m_pTable = pSaveStruct->m_pTable;
                }
            }
            break;

        case HtmlTokenId::NOBR_ON:
            // HACK for MS: Is the <NOBR> at the start of the cell?
            pSaveStruct->StartNoBreak( *m_pPam->GetPoint() );
            break;

        case HtmlTokenId::NOBR_OFF:
                pSaveStruct->EndNoBreak( *m_pPam->GetPoint() );
            break;

        case HtmlTokenId::COMMENT:
            // Spaces are not gonna be deleted with comment fields,
            // and we don't want a new cell for a comment
            NextToken( nToken );
            break;

        case HtmlTokenId::MARQUEE_ON:
            if( !pSaveStruct->IsInSection() )
            {
                // create a new section, the PaM is gonna be there
                pSaveStruct->AddContents(
                    InsertTableContents( bHead ) );
            }
            m_bCallNextToken = true;
            NewMarquee( pCurTable );
            break;

        case HtmlTokenId::TEXTTOKEN:
            // Don't add a section for an empty string
            if( !pSaveStruct->IsInSection() && 1==aToken.getLength() &&
                ' '==aToken[0] )
                break;
            SAL_FALLTHROUGH;
        default:
            if( !pSaveStruct->IsInSection() )
            {
                // add a new section, the PaM's gonna be there
                pSaveStruct->AddContents(
                    InsertTableContents( bHead ) );
            }

            if( IsParserWorking() || bPending )
                NextToken( nToken );
            break;
        }

        OSL_ENSURE( !bPending || !m_pPendStack,
                "SwHTMLParser::BuildTableCell: There is a PendStack again" );
        bPending = false;
        if( IsParserWorking() )
            SaveState( HtmlTokenId::NONE );

        if( !bDone )
            nToken = GetNextToken();
    }

    if( SvParserState::Pending == GetStatus() )
    {
        m_pPendStack = new SwPendingStack( bHead ? HtmlTokenId::TABLEHEADER_ON
                                               : HtmlTokenId::TABLEDATA_ON, m_pPendStack );
        m_pPendStack->pData = pSaveStruct;

        return;
    }

    // If the content of the cell was empty, we need to create an empty content
    // We also create an empty content if the cell ended with a table and had no
    // COL tags. Otherwise, it was probably exported by us and we don't
    // want to have an additional paragraph
    if( !pSaveStruct->GetFirstContents() ||
        (!pSaveStruct->IsInSection() && !pCurTable->HasColTags()) )
    {
        OSL_ENSURE( pSaveStruct->GetFirstContents() ||
                !pSaveStruct->IsInSection(),
                "Section or not, that is the question here" );
        const SwStartNode *pStNd =
            InsertTableSection( static_cast< sal_uInt16 >(pSaveStruct->IsHeaderCell()
                                        ? RES_POOLCOLL_TABLE_HDLN
                                        : RES_POOLCOLL_TABLE ));
        const SwEndNode *pEndNd = pStNd->EndOfSectionNode();
        SwContentNode *pCNd = m_xDoc->GetNodes()[pEndNd->GetIndex()-1] ->GetContentNode();
        //Added defaults to CJK and CTL
        SvxFontHeightItem aFontHeight( 40, 100, RES_CHRATR_FONTSIZE );
        pCNd->SetAttr( aFontHeight );
        SvxFontHeightItem aFontHeightCJK( 40, 100, RES_CHRATR_CJK_FONTSIZE );
        pCNd->SetAttr( aFontHeightCJK );
        SvxFontHeightItem aFontHeightCTL( 40, 100, RES_CHRATR_CTL_FONTSIZE );
        pCNd->SetAttr( aFontHeightCTL );

        pSaveStruct->AddContents( new HTMLTableCnts(pStNd) );
        pSaveStruct->ClearIsInSection();
    }

    if( pSaveStruct->IsInSection() )
    {
        pSaveStruct->CheckNoBreak( *m_pPam->GetPoint() );

        // End all open contexts. We'll take AttrMin because nContextStMin might
        // have been modified. Since it's gonna be restored by EndContext, it's okay
        while( m_aContexts.size() > m_nContextStAttrMin+1 )
        {
            HTMLAttrContext *pCntxt = PopContext();
            EndContext( pCntxt );
            delete pCntxt;
        }

        // Remove LFs at the paragraph end
        if( StripTrailingLF()==0 && !m_pPam->GetPoint()->nContent.GetIndex() )
            StripTrailingPara();

        // If there was an adjustment set for the cell, we need to close it
        HTMLAttrContext *pCntxt = PopContext();
        EndContext( pCntxt );
        delete pCntxt;
    }
    else
    {
        // Close all still open contexts
        while( m_aContexts.size() > m_nContextStAttrMin )
        {
            HTMLAttrContext *pCntxt = PopContext();
            ClearContext( pCntxt );
            delete pCntxt;
        }
    }

    // end an enumeration
    GetNumInfo().Clear();

    SetAttr( false );

    pSaveStruct->InsertCell( *this, pCurTable );

    // we're probably before a <TH>, <TD>, <TR> or </TABLE>
    delete pSaveStruct;
}

class RowSaveStruct : public SwPendingStackData
{
public:
    SvxAdjust eAdjust;
    sal_Int16 eVertOri;
    bool bHasCells;

    RowSaveStruct() :
        eAdjust( SvxAdjust::End ), eVertOri( text::VertOrientation::TOP ), bHasCells( false )
    {}
};

void SwHTMLParser::BuildTableRow( HTMLTable *pCurTable, bool bReadOptions,
                                  SvxAdjust eGrpAdjust,
                                  sal_Int16 eGrpVertOri )
{
    // <TR> was already read

    if( !IsParserWorking() && !m_pPendStack )
        return;

    HtmlTokenId nToken = HtmlTokenId::NONE;
    RowSaveStruct* pSaveStruct;

    bool bPending = false;
    if( m_pPendStack )
    {
        pSaveStruct = static_cast<RowSaveStruct*>(m_pPendStack->pData);

        SwPendingStack* pTmp = m_pPendStack->pNext;
        delete m_pPendStack;
        m_pPendStack = pTmp;
        nToken = m_pPendStack ? m_pPendStack->nToken : GetSaveToken();
        bPending = SvParserState::Error == eState && m_pPendStack != nullptr;

        SaveState( nToken );
    }
    else
    {
        SvxAdjust eAdjust = eGrpAdjust;
        sal_Int16 eVertOri = eGrpVertOri;
        Color aBGColor;
        OUString aBGImage, aStyle, aId, aClass;
        bool bBGColor = false;
        pSaveStruct = new RowSaveStruct;

        if( bReadOptions )
        {
            const HTMLOptions& rHTMLOptions = GetOptions();
            for (size_t i = rHTMLOptions.size(); i; )
            {
                const HTMLOption& rOption = rHTMLOptions[--i];
                switch( rOption.GetToken() )
                {
                case HtmlOptionId::ID:
                    aId = rOption.GetString();
                    break;
                case HtmlOptionId::ALIGN:
                    eAdjust = rOption.GetEnum( aHTMLPAlignTable, eAdjust );
                    break;
                case HtmlOptionId::VALIGN:
                    eVertOri = rOption.GetEnum( aHTMLTableVAlignTable, eVertOri );
                    break;
                case HtmlOptionId::BGCOLOR:
                    // Ignore empty BGCOLOR on <TABLE>, <TR> and <TD>/>TH> like Netscape
                    // *really* not on other tags
                    if( !rOption.GetString().isEmpty() )
                    {
                        rOption.GetColor( aBGColor );
                        bBGColor = true;
                    }
                    break;
                case HtmlOptionId::BACKGROUND:
                    aBGImage = rOption.GetString();
                    break;
                case HtmlOptionId::STYLE:
                    aStyle = rOption.GetString();
                    break;
                case HtmlOptionId::CLASS:
                    aClass= rOption.GetString();
                    break;
                default: break;
                }
            }
        }

        if( !aId.isEmpty() )
            InsertBookmark( aId );

        SvxBrushItem *pBrushItem =
            CreateBrushItem( bBGColor ? &aBGColor : nullptr, aBGImage, aStyle,
                             aId, aClass );
        pCurTable->OpenRow( eAdjust, eVertOri, pBrushItem );
        // If the first GetNextToken() doesn't succeed (pending input), must re-read from the beginning.
        SaveState( HtmlTokenId::NONE );
    }

    if( nToken == HtmlTokenId::NONE )
        nToken = GetNextToken();

    bool bDone = false;
    while( (IsParserWorking() && !bDone) || bPending )
    {
        SaveState( nToken );

        nToken = FilterToken( nToken );

        OSL_ENSURE( m_pPendStack || !m_bCallNextToken ||
                pCurTable->GetContext() || pCurTable->HasParentSection(),
                "Where is the section??" );
        if( !m_pPendStack && m_bCallNextToken &&
            (pCurTable->GetContext() || pCurTable->HasParentSection()) )
        {
            /// Call NextToken directly (e.g. ignore the content of floating frames or applets)
            NextToken( nToken );
        }
        else switch( nToken )
        {
        case HtmlTokenId::TABLE_ON:
            if( !pCurTable->GetContext()  )
            {
                SkipToken();
                bDone = true;
            }

            break;
        case HtmlTokenId::TABLEROW_ON:
        case HtmlTokenId::THEAD_ON:
        case HtmlTokenId::THEAD_OFF:
        case HtmlTokenId::TBODY_ON:
        case HtmlTokenId::TBODY_OFF:
        case HtmlTokenId::TFOOT_ON:
        case HtmlTokenId::TFOOT_OFF:
        case HtmlTokenId::TABLE_OFF:
            SkipToken();
            SAL_FALLTHROUGH;
        case HtmlTokenId::TABLEROW_OFF:
            bDone = true;
            break;
        case HtmlTokenId::TABLEHEADER_ON:
        case HtmlTokenId::TABLEDATA_ON:
            BuildTableCell( pCurTable, true, HtmlTokenId::TABLEHEADER_ON==nToken );
            if( SvParserState::Pending != GetStatus() )
            {
                pSaveStruct->bHasCells = true;
                bDone = m_pTable->IsOverflowing();
            }
            break;
        case HtmlTokenId::CAPTION_ON:
            BuildTableCaption( pCurTable );
            bDone = m_pTable->IsOverflowing();
            break;
        case HtmlTokenId::CAPTION_OFF:
        case HtmlTokenId::TABLEHEADER_OFF:
        case HtmlTokenId::TABLEDATA_OFF:
        case HtmlTokenId::COLGROUP_ON:
        case HtmlTokenId::COLGROUP_OFF:
        case HtmlTokenId::COL_ON:
        case HtmlTokenId::COL_OFF:
            // Where no cell started, there can't be a cell ending
            // all the other tokens are bogus anyway and only break the table
            break;
        case HtmlTokenId::MULTICOL_ON:
            // we can't add columned text frames here
            break;
        case HtmlTokenId::FORM_ON:
            NewForm( false );   // don't create a new paragraph
            break;
        case HtmlTokenId::FORM_OFF:
            EndForm( false );   // don't create a new paragraph
            break;
        case HtmlTokenId::COMMENT:
            NextToken( nToken );
            break;
        case HtmlTokenId::MAP_ON:
            // an image map doesn't add anything, so we can parse it without a cell
            NextToken( nToken );
            break;
        case HtmlTokenId::TEXTTOKEN:
            if( (pCurTable->GetContext() ||
                 !pCurTable->HasParentSection()) &&
                1==aToken.getLength() && ' '==aToken[0] )
                break;
            SAL_FALLTHROUGH;
        default:
            pCurTable->MakeParentContents();
            NextToken( nToken );
            break;
        }

        OSL_ENSURE( !bPending || !m_pPendStack,
                "SwHTMLParser::BuildTableRow: There is a PendStack again" );
        bPending = false;
        if( IsParserWorking() )
            SaveState( HtmlTokenId::NONE );

        if( !bDone )
            nToken = GetNextToken();
    }

    if( SvParserState::Pending == GetStatus() )
    {
        m_pPendStack = new SwPendingStack( HtmlTokenId::TABLEROW_ON, m_pPendStack );
        m_pPendStack->pData = pSaveStruct;
    }
    else
    {
        pCurTable->CloseRow( !pSaveStruct->bHasCells );
        delete pSaveStruct;
    }

    // we're probably before <TR> or </TABLE>
}

void SwHTMLParser::BuildTableSection( HTMLTable *pCurTable,
                                      bool bReadOptions,
                                      bool bHead )
{
    // <THEAD>, <TBODY> resp. <TFOOT> were read already
    if( !IsParserWorking() && !m_pPendStack )
        return;

    HtmlTokenId nToken = HtmlTokenId::NONE;
    bool bPending = false;
    RowSaveStruct* pSaveStruct;

    if( m_pPendStack )
    {
        pSaveStruct = static_cast<RowSaveStruct*>(m_pPendStack->pData);

        SwPendingStack* pTmp = m_pPendStack->pNext;
        delete m_pPendStack;
        m_pPendStack = pTmp;
        nToken = m_pPendStack ? m_pPendStack->nToken : GetSaveToken();
        bPending = SvParserState::Error == eState && m_pPendStack != nullptr;

        SaveState( nToken );
    }
    else
    {
        pSaveStruct = new RowSaveStruct;

        if( bReadOptions )
        {
            const HTMLOptions& rHTMLOptions = GetOptions();
            for (size_t i = rHTMLOptions.size(); i; )
            {
                const HTMLOption& rOption = rHTMLOptions[--i];
                switch( rOption.GetToken() )
                {
                case HtmlOptionId::ID:
                    InsertBookmark( rOption.GetString() );
                    break;
                case HtmlOptionId::ALIGN:
                    pSaveStruct->eAdjust =
                        rOption.GetEnum( aHTMLPAlignTable, pSaveStruct->eAdjust );
                    break;
                case HtmlOptionId::VALIGN:
                    pSaveStruct->eVertOri =
                        rOption.GetEnum( aHTMLTableVAlignTable,
                                          pSaveStruct->eVertOri );
                    break;
                default: break;
                }
            }
        }

        // If the first GetNextToken() doesn't succeed (pending input), must re-read from the beginning.
        SaveState( HtmlTokenId::NONE );
    }

    if( nToken == HtmlTokenId::NONE )
        nToken = GetNextToken();

    bool bDone = false;
    while( (IsParserWorking() && !bDone) || bPending )
    {
        SaveState( nToken );

        nToken = FilterToken( nToken );

        OSL_ENSURE( m_pPendStack || !m_bCallNextToken ||
                pCurTable->GetContext() || pCurTable->HasParentSection(),
                "Where is the section?" );
        if( !m_pPendStack && m_bCallNextToken &&
            (pCurTable->GetContext() || pCurTable->HasParentSection()) )
        {
            // Call NextToken directly (e.g. ignore the content of floating frames or applets)
            NextToken( nToken );
        }
        else switch( nToken )
        {
        case HtmlTokenId::TABLE_ON:
            if( !pCurTable->GetContext()  )
            {
                SkipToken();
                bDone = true;
            }

            break;
        case HtmlTokenId::THEAD_ON:
        case HtmlTokenId::TFOOT_ON:
        case HtmlTokenId::TBODY_ON:
        case HtmlTokenId::TABLE_OFF:
            SkipToken();
            SAL_FALLTHROUGH;
        case HtmlTokenId::THEAD_OFF:
        case HtmlTokenId::TBODY_OFF:
        case HtmlTokenId::TFOOT_OFF:
            bDone = true;
            break;
        case HtmlTokenId::CAPTION_ON:
            BuildTableCaption( pCurTable );
            bDone = m_pTable->IsOverflowing();
            break;
        case HtmlTokenId::CAPTION_OFF:
            break;
        case HtmlTokenId::TABLEHEADER_ON:
        case HtmlTokenId::TABLEDATA_ON:
            SkipToken();
            BuildTableRow( pCurTable, false, pSaveStruct->eAdjust,
                           pSaveStruct->eVertOri );
            bDone = m_pTable->IsOverflowing();
            break;
        case HtmlTokenId::TABLEROW_ON:
            BuildTableRow( pCurTable, true, pSaveStruct->eAdjust,
                           pSaveStruct->eVertOri );
            bDone = m_pTable->IsOverflowing();
            break;
        case HtmlTokenId::MULTICOL_ON:
            // we can't add columned text frames here
            break;
        case HtmlTokenId::FORM_ON:
            NewForm( false );   // don't create a new paragraph
            break;
        case HtmlTokenId::FORM_OFF:
            EndForm( false );   // don't create a new paragraph
            break;
        case HtmlTokenId::TEXTTOKEN:
            // blank strings may be a series of CR+LF and no text
            if( (pCurTable->GetContext() ||
                 !pCurTable->HasParentSection()) &&
                1==aToken.getLength() && ' ' == aToken[0] )
                break;
            SAL_FALLTHROUGH;
        default:
            pCurTable->MakeParentContents();
            NextToken( nToken );
        }

        OSL_ENSURE( !bPending || !m_pPendStack,
                "SwHTMLParser::BuildTableSection: There is a PendStack again" );
        bPending = false;
        if( IsParserWorking() )
            SaveState( HtmlTokenId::NONE );

        if( !bDone )
            nToken = GetNextToken();
    }

    if( SvParserState::Pending == GetStatus() )
    {
        m_pPendStack = new SwPendingStack( bHead ? HtmlTokenId::THEAD_ON
                                               : HtmlTokenId::TBODY_ON, m_pPendStack );
        m_pPendStack->pData = pSaveStruct;
    }
    else
    {
        pCurTable->CloseSection( bHead );
        delete pSaveStruct;
    }

    // now we stand (perhaps) in front of <TBODY>,... or </TABLE>
}

struct TableColGrpSaveStruct : public SwPendingStackData
{
    sal_uInt16 nColGrpSpan;
    sal_uInt16 nColGrpWidth;
    bool bRelColGrpWidth;
    SvxAdjust eColGrpAdjust;
    sal_Int16 eColGrpVertOri;

    inline TableColGrpSaveStruct();

    inline void CloseColGroup( HTMLTable *pTable );
};

inline TableColGrpSaveStruct::TableColGrpSaveStruct() :
    nColGrpSpan( 1 ), nColGrpWidth( 0 ),
    bRelColGrpWidth( false ), eColGrpAdjust( SvxAdjust::End ),
    eColGrpVertOri( text::VertOrientation::TOP )
{}

inline void TableColGrpSaveStruct::CloseColGroup( HTMLTable *pTable )
{
    pTable->CloseColGroup( nColGrpSpan, nColGrpWidth,
                            bRelColGrpWidth, eColGrpAdjust, eColGrpVertOri );
}

void SwHTMLParser::BuildTableColGroup( HTMLTable *pCurTable,
                                       bool bReadOptions )
{
    // <COLGROUP> was read already if bReadOptions is set

    if( !IsParserWorking() && !m_pPendStack )
        return;

    HtmlTokenId nToken = HtmlTokenId::NONE;
    bool bPending = false;
    TableColGrpSaveStruct* pSaveStruct;

    if( m_pPendStack )
    {
        pSaveStruct = static_cast<TableColGrpSaveStruct*>(m_pPendStack->pData);

        SwPendingStack* pTmp = m_pPendStack->pNext;
        delete m_pPendStack;
        m_pPendStack = pTmp;
        nToken = m_pPendStack ? m_pPendStack->nToken : GetSaveToken();
        bPending = SvParserState::Error == eState && m_pPendStack != nullptr;

        SaveState( nToken );
    }
    else
    {

        pSaveStruct = new TableColGrpSaveStruct;
        if( bReadOptions )
        {
            const HTMLOptions& rColGrpOptions = GetOptions();
            for (size_t i = rColGrpOptions.size(); i; )
            {
                const HTMLOption& rOption = rColGrpOptions[--i];
                switch( rOption.GetToken() )
                {
                case HtmlOptionId::ID:
                    InsertBookmark( rOption.GetString() );
                    break;
                case HtmlOptionId::SPAN:
                    pSaveStruct->nColGrpSpan = (sal_uInt16)rOption.GetNumber();
                    break;
                case HtmlOptionId::WIDTH:
                    pSaveStruct->nColGrpWidth = (sal_uInt16)rOption.GetNumber();
                    pSaveStruct->bRelColGrpWidth =
                        (rOption.GetString().indexOf('*') != -1);
                    break;
                case HtmlOptionId::ALIGN:
                    pSaveStruct->eColGrpAdjust =
                        rOption.GetEnum( aHTMLPAlignTable, pSaveStruct->eColGrpAdjust );
                    break;
                case HtmlOptionId::VALIGN:
                    pSaveStruct->eColGrpVertOri =
                        rOption.GetEnum( aHTMLTableVAlignTable,
                                                pSaveStruct->eColGrpVertOri );
                    break;
                default: break;
                }
            }
        }
        // If the first GetNextToken() doesn't succeed (pending input), must re-read from the beginning.
        SaveState( HtmlTokenId::NONE );
    }

    if( nToken == HtmlTokenId::NONE )
        nToken = GetNextToken();    // naechstes Token

    bool bDone = false;
    while( (IsParserWorking() && !bDone) || bPending )
    {
        SaveState( nToken );

        nToken = FilterToken( nToken );

        OSL_ENSURE( m_pPendStack || !m_bCallNextToken ||
                pCurTable->GetContext() || pCurTable->HasParentSection(),
                "Where is the section?" );
        if( !m_pPendStack && m_bCallNextToken &&
            (pCurTable->GetContext() || pCurTable->HasParentSection()) )
        {
            // Call NextToken directly (e.g. ignore the content of floating frames or applets)
            NextToken( nToken );
        }
        else switch( nToken )
        {
        case HtmlTokenId::TABLE_ON:
            if( !pCurTable->GetContext()  )
            {
                SkipToken();
                bDone = true;
            }

            break;
        case HtmlTokenId::COLGROUP_ON:
        case HtmlTokenId::THEAD_ON:
        case HtmlTokenId::TFOOT_ON:
        case HtmlTokenId::TBODY_ON:
        case HtmlTokenId::TABLEROW_ON:
        case HtmlTokenId::TABLE_OFF:
            SkipToken();
            SAL_FALLTHROUGH;
        case HtmlTokenId::COLGROUP_OFF:
            bDone = true;
            break;
        case HtmlTokenId::COL_ON:
            {
                sal_uInt16 nColSpan = 1;
                sal_uInt16 nColWidth = pSaveStruct->nColGrpWidth;
                bool bRelColWidth = pSaveStruct->bRelColGrpWidth;
                SvxAdjust eColAdjust = pSaveStruct->eColGrpAdjust;
                sal_Int16 eColVertOri = pSaveStruct->eColGrpVertOri;

                const HTMLOptions& rColOptions = GetOptions();
                for (size_t i = rColOptions.size(); i; )
                {
                    const HTMLOption& rOption = rColOptions[--i];
                    switch( rOption.GetToken() )
                    {
                    case HtmlOptionId::ID:
                        InsertBookmark( rOption.GetString() );
                        break;
                    case HtmlOptionId::SPAN:
                        nColSpan = (sal_uInt16)rOption.GetNumber();
                        break;
                    case HtmlOptionId::WIDTH:
                        nColWidth = (sal_uInt16)rOption.GetNumber();
                        bRelColWidth =
                            (rOption.GetString().indexOf('*') != -1);
                        break;
                    case HtmlOptionId::ALIGN:
                        eColAdjust = rOption.GetEnum( aHTMLPAlignTable, eColAdjust );
                        break;
                    case HtmlOptionId::VALIGN:
                        eColVertOri =
                            rOption.GetEnum( aHTMLTableVAlignTable, eColVertOri );
                        break;
                    default: break;
                    }
                }
                pCurTable->InsertCol( nColSpan, nColWidth, bRelColWidth,
                                      eColAdjust, eColVertOri );

                // the attributes in <COLGRP> should be ignored, if there are <COL> elements
                pSaveStruct->nColGrpSpan = 0;
            }
            break;
        case HtmlTokenId::COL_OFF:
            break;      // Ignore
        case HtmlTokenId::MULTICOL_ON:
            // we can't add columned text frames here
            break;
        case HtmlTokenId::TEXTTOKEN:
            if( (pCurTable->GetContext() ||
                 !pCurTable->HasParentSection()) &&
                1==aToken.getLength() && ' '==aToken[0] )
                break;
            SAL_FALLTHROUGH;
        default:
            pCurTable->MakeParentContents();
            NextToken( nToken );
        }

        OSL_ENSURE( !bPending || !m_pPendStack,
                "SwHTMLParser::BuildTableColGrp: There is a PendStack again" );
        bPending = false;
        if( IsParserWorking() )
            SaveState( HtmlTokenId::NONE );

        if( !bDone )
            nToken = GetNextToken();
    }

    if( SvParserState::Pending == GetStatus() )
    {
        m_pPendStack = new SwPendingStack( HtmlTokenId::COL_ON, m_pPendStack );
        m_pPendStack->pData = pSaveStruct;
    }
    else
    {
        pSaveStruct->CloseColGroup( pCurTable );
        delete pSaveStruct;
    }
}

class CaptionSaveStruct : public SectionSaveStruct
{
    SwPosition aSavePos;
    SwHTMLNumRuleInfo aNumRuleInfo; // valid numbering

public:

    HTMLAttrTable aAttrTab;        // attributes

    CaptionSaveStruct( SwHTMLParser& rParser, const SwPosition& rPos ) :
        SectionSaveStruct( rParser ), aSavePos( rPos )
    {
        rParser.SaveAttrTab( aAttrTab );

        // The current numbering was remembered and just needs to be closed
        aNumRuleInfo.Set( rParser.GetNumInfo() );
        rParser.GetNumInfo().Clear();
    }

    const SwPosition& GetPos() const { return aSavePos; }

    void RestoreAll( SwHTMLParser& rParser )
    {
        // Recover the old stack
        Restore( rParser );

        // Recover the old attribute tables
        rParser.RestoreAttrTab( aAttrTab );

        // Re-open the old numbering
        rParser.GetNumInfo().Set( aNumRuleInfo );
    }
};

void SwHTMLParser::BuildTableCaption( HTMLTable *pCurTable )
{
    // <CAPTION> was read already

    if( !IsParserWorking() && !m_pPendStack )
        return;

    HtmlTokenId nToken = HtmlTokenId::NONE;
    CaptionSaveStruct* pSaveStruct;

    if( m_pPendStack )
    {
        pSaveStruct = static_cast<CaptionSaveStruct*>(m_pPendStack->pData);

        SwPendingStack* pTmp = m_pPendStack->pNext;
        delete m_pPendStack;
        m_pPendStack = pTmp;
        nToken = m_pPendStack ? m_pPendStack->nToken : GetSaveToken();
        OSL_ENSURE( !m_pPendStack, "Where does a PendStack coming from?" );

        SaveState( nToken );
    }
    else
    {
        if( m_pTable->IsOverflowing() )
        {
            SaveState( HtmlTokenId::NONE );
            return;
        }

        bool bTop = true;
        const HTMLOptions& rHTMLOptions = GetOptions();
        for ( size_t i = rHTMLOptions.size(); i; )
        {
            const HTMLOption& rOption = rHTMLOptions[--i];
            if( HtmlOptionId::ALIGN == rOption.GetToken() )
            {
                if (rOption.GetString().equalsIgnoreAsciiCase(
                        OOO_STRING_SVTOOLS_HTML_VA_bottom))
                {
                    bTop = false;
                }
            }
        }

        // Remember old PaM position
        pSaveStruct = new CaptionSaveStruct( *this, *m_pPam->GetPoint() );

        // Add a text section in the icon section as a container for the header
        // and set the PaM there
        const SwStartNode *pStNd;
        if( m_pTable == pCurTable )
            pStNd = InsertTempTableCaptionSection();
        else
            pStNd = InsertTableSection( RES_POOLCOLL_TEXT );

        HTMLAttrContext *pCntxt = new HTMLAttrContext( HtmlTokenId::CAPTION_ON );

        // Table headers are always centered
        NewAttr( &m_aAttrTab.pAdjust, SvxAdjustItem(SvxAdjust::Center, RES_PARATR_ADJUST) );

        HTMLAttrs &rAttrs = pCntxt->GetAttrs();
        rAttrs.push_back( m_aAttrTab.pAdjust );

        PushContext( pCntxt );

        // Remember the start node of the section at the table
        pCurTable->SetCaption( pStNd, bTop );

        // If the first GetNextToken() doesn't succeed (pending input), must re-read from the beginning.
        SaveState( HtmlTokenId::NONE );
    }

    if( nToken == HtmlTokenId::NONE )
        nToken = GetNextToken();

    // </CAPTION> is needed according to DTD
    bool bDone = false;
    while( IsParserWorking() && !bDone )
    {
        SaveState( nToken );

        nToken = FilterToken( nToken );

        switch( nToken )
        {
        case HtmlTokenId::TABLE_ON:
            if( !m_pPendStack )
            {
                pSaveStruct->m_pTable = m_pTable;
                bool bHasToFly = pSaveStruct->m_pTable!=pCurTable;
                BuildTable( pCurTable->GetTableAdjust( true ),
                            false, true, bHasToFly );
            }
            else
            {
                BuildTable( SvxAdjust::End );
            }
            if( SvParserState::Pending != GetStatus() )
            {
                m_pTable = pSaveStruct->m_pTable;
            }
            break;
        case HtmlTokenId::TABLE_OFF:
        case HtmlTokenId::COLGROUP_ON:
        case HtmlTokenId::THEAD_ON:
        case HtmlTokenId::TFOOT_ON:
        case HtmlTokenId::TBODY_ON:
        case HtmlTokenId::TABLEROW_ON:
            SkipToken();
            bDone = true;
            break;

        case HtmlTokenId::CAPTION_OFF:
            bDone = true;
            break;
        default:
            if( m_pPendStack )
            {
                SwPendingStack* pTmp = m_pPendStack->pNext;
                delete m_pPendStack;
                m_pPendStack = pTmp;

                OSL_ENSURE( !pTmp, "Further it can't go!" );
            }

            if( IsParserWorking() )
                NextToken( nToken );
            break;
        }

        if( IsParserWorking() )
            SaveState( HtmlTokenId::NONE );

        if( !bDone )
            nToken = GetNextToken();
    }

    if( SvParserState::Pending==GetStatus() )
    {
        m_pPendStack = new SwPendingStack( HtmlTokenId::CAPTION_ON, m_pPendStack );
        m_pPendStack->pData = pSaveStruct;
        return;
    }

    // end all still open contexts
    while( m_aContexts.size() > m_nContextStAttrMin+1 )
    {
        HTMLAttrContext *pCntxt = PopContext();
        EndContext( pCntxt );
        delete pCntxt;
    }

    bool bLFStripped = StripTrailingLF() > 0;

    if( m_pTable==pCurTable )
    {
        // On moving the caption later, the last paragraph isn't moved as well.
        // That means, there has to be an empty paragraph at the end of the section
        if( m_pPam->GetPoint()->nContent.GetIndex() || bLFStripped )
            AppendTextNode( AM_NOSPACE );
    }
    else
    {
        // Strip LFs at the end of the paragraph
        if( !m_pPam->GetPoint()->nContent.GetIndex() && !bLFStripped )
            StripTrailingPara();
    }

    // If there's an adjustment for the cell, we need to close it
    HTMLAttrContext *pCntxt = PopContext();
    EndContext( pCntxt );
    delete pCntxt;

    SetAttr( false );

    // Recover stack and attribute table
    pSaveStruct->RestoreAll( *this );

    // Recover PaM
    *m_pPam->GetPoint() = pSaveStruct->GetPos();

    delete pSaveStruct;
}

class TableSaveStruct : public SwPendingStackData
{
public:
    HTMLTable *m_pCurrentTable;

    explicit TableSaveStruct( HTMLTable *pCurTable ) :
        m_pCurrentTable( pCurTable )
    {}

    // Initiate creation of the table and put the table in a text frame if
    // needed. If it returns true, we need to insert a paragraph.
    void MakeTable( sal_uInt16 nWidth, SwPosition& rPos, SwDoc *pDoc );
};

void TableSaveStruct::MakeTable( sal_uInt16 nWidth, SwPosition& rPos, SwDoc *pDoc )
{
    m_pCurrentTable->MakeTable(nullptr, nWidth);

    HTMLTableContext *pTCntxt = m_pCurrentTable->GetContext();
    OSL_ENSURE( pTCntxt, "Where is the table context" );

    SwTableNode *pTableNd = pTCntxt->GetTableNode();
    OSL_ENSURE( pTableNd, "Where is the table node" );

    if( pDoc->getIDocumentLayoutAccess().GetCurrentViewShell() && pTableNd )
    {
        // If there's already a layout, the BoxFrames need to be regenerated at this table

        if( pTCntxt->GetFrameFormat() )
        {
            pTCntxt->GetFrameFormat()->DelFrames();
            pTableNd->DelFrames();
            pTCntxt->GetFrameFormat()->MakeFrames();
        }
        else
        {
            pTableNd->DelFrames();
            SwNodeIndex aIdx( *pTableNd->EndOfSectionNode(), 1 );
            OSL_ENSURE( aIdx.GetIndex() <= pTCntxt->GetPos()->nNode.GetIndex(),
                    "unexpected node for table layout" );
            pTableNd->MakeFrames( &aIdx );
        }
    }

    rPos = *pTCntxt->GetPos();
}

HTMLTableOptions::HTMLTableOptions( const HTMLOptions& rOptions,
                                    SvxAdjust eParentAdjust ) :
    nCols( 0 ),
    nWidth( 0 ), nHeight( 0 ),
    nCellPadding( USHRT_MAX ), nCellSpacing( USHRT_MAX ),
    nBorder( USHRT_MAX ),
    nHSpace( 0 ), nVSpace( 0 ),
    eAdjust( eParentAdjust ), eVertOri( text::VertOrientation::CENTER ),
    eFrame( HTMLTableFrame::Void ), eRules( HTMLTableRules::NONE ),
    bPrcWidth( false ),
    bTableAdjust( false ),
    bBGColor( false ),
    aBorderColor( COL_GRAY )
{
    bool bBorderColor = false;
    bool bHasFrame = false, bHasRules = false;

    for (size_t i = rOptions.size(); i; )
    {
        const HTMLOption& rOption = rOptions[--i];
        switch( rOption.GetToken() )
        {
        case HtmlOptionId::ID:
            aId = rOption.GetString();
            break;
        case HtmlOptionId::COLS:
            nCols = (sal_uInt16)rOption.GetNumber();
            break;
        case HtmlOptionId::WIDTH:
            nWidth = (sal_uInt16)rOption.GetNumber();
            bPrcWidth = (rOption.GetString().indexOf('%') != -1);
            if( bPrcWidth && nWidth>100 )
                nWidth = 100;
            break;
        case HtmlOptionId::HEIGHT:
            nHeight = (sal_uInt16)rOption.GetNumber();
            if( rOption.GetString().indexOf('%') != -1 )
                nHeight = 0;    // don't use % attributes
            break;
        case HtmlOptionId::CELLPADDING:
            nCellPadding = (sal_uInt16)rOption.GetNumber();
            break;
        case HtmlOptionId::CELLSPACING:
            nCellSpacing = (sal_uInt16)rOption.GetNumber();
            break;
        case HtmlOptionId::ALIGN:
            {
                if( rOption.GetEnum( eAdjust, aHTMLPAlignTable ) )
                {
                    bTableAdjust = true;
                }
            }
            break;
        case HtmlOptionId::VALIGN:
            eVertOri = rOption.GetEnum( aHTMLTableVAlignTable, eVertOri );
            break;
        case HtmlOptionId::BORDER:
            // Handle BORDER and BORDER=BORDER like BORDER=1
            if (!rOption.GetString().isEmpty() &&
                !rOption.GetString().equalsIgnoreAsciiCase(
                        OOO_STRING_SVTOOLS_HTML_O_border))
            {
                nBorder = (sal_uInt16)rOption.GetNumber();
            }
            else
                nBorder = 1;

            if( !bHasFrame )
                eFrame = ( nBorder ? HTMLTableFrame::Box : HTMLTableFrame::Void );
            if( !bHasRules )
                eRules = ( nBorder ? HTMLTableRules::All : HTMLTableRules::NONE );
            break;
        case HtmlOptionId::FRAME:
            eFrame = rOption.GetTableFrame();
            bHasFrame = true;
            break;
        case HtmlOptionId::RULES:
            eRules = rOption.GetTableRules();
            bHasRules = true;
            break;
        case HtmlOptionId::BGCOLOR:
            // Ignore empty BGCOLOR on <TABLE>, <TR> and <TD>/<TH> like Netscape
            // *really* not on other tags
            if( !rOption.GetString().isEmpty() )
            {
                rOption.GetColor( aBGColor );
                bBGColor = true;
            }
            break;
        case HtmlOptionId::BACKGROUND:
            aBGImage = rOption.GetString();
            break;
        case HtmlOptionId::BORDERCOLOR:
            rOption.GetColor( aBorderColor );
            bBorderColor = true;
            break;
        case HtmlOptionId::BORDERCOLORDARK:
            if( !bBorderColor )
                rOption.GetColor( aBorderColor );
            break;
        case HtmlOptionId::STYLE:
            aStyle = rOption.GetString();
            break;
        case HtmlOptionId::CLASS:
            aClass = rOption.GetString();
            break;
        case HtmlOptionId::DIR:
            aDir = rOption.GetString();
            break;
        case HtmlOptionId::HSPACE:
            nHSpace = (sal_uInt16)rOption.GetNumber();
            break;
        case HtmlOptionId::VSPACE:
            nVSpace = (sal_uInt16)rOption.GetNumber();
            break;
        default: break;
        }
    }

    if( nCols && !nWidth )
    {
        nWidth = 100;
        bPrcWidth = true;
    }

    // If BORDER=0 or no BORDER given, then there shouldn't be a border
    if( 0==nBorder || USHRT_MAX==nBorder )
    {
        eFrame = HTMLTableFrame::Void;
        eRules = HTMLTableRules::NONE;
    }
}

HTMLTable *SwHTMLParser::BuildTable( SvxAdjust eParentAdjust,
                                     bool bIsParentHead,
                                     bool bHasParentSection,
                                     bool bHasToFly )
{
    if( !IsParserWorking() && !m_pPendStack )
        return nullptr;

    HtmlTokenId nToken = HtmlTokenId::NONE;
    bool bPending = false;
    TableSaveStruct* pSaveStruct;

    if( m_pPendStack )
    {
        pSaveStruct = static_cast<TableSaveStruct*>(m_pPendStack->pData);

        SwPendingStack* pTmp = m_pPendStack->pNext;
        delete m_pPendStack;
        m_pPendStack = pTmp;
        nToken = m_pPendStack ? m_pPendStack->nToken : GetSaveToken();
        bPending = SvParserState::Error == eState && m_pPendStack != nullptr;

        SaveState( nToken );
    }
    else
    {
        m_pTable = nullptr;
        HTMLTableOptions *pTableOptions =
            new HTMLTableOptions( GetOptions(), eParentAdjust );

        if( !pTableOptions->aId.isEmpty() )
            InsertBookmark( pTableOptions->aId );

        HTMLTable *pCurTable = new HTMLTable( this, m_pTable,
                                              bIsParentHead,
                                              bHasParentSection,
                                              bHasToFly,
                                              pTableOptions );
        if( !m_pTable )
            m_pTable = pCurTable;

        pSaveStruct = new TableSaveStruct( pCurTable );

        delete pTableOptions;

        // Is pending on the first GetNextToken, needs to be re-read on each construction
        SaveState( HtmlTokenId::NONE );
    }

    HTMLTable *pCurTable = pSaveStruct->m_pCurrentTable;

    // </TABLE> is needed according to DTD
    if( nToken == HtmlTokenId::NONE )
        nToken = GetNextToken();

    bool bDone = false;
    while( (IsParserWorking() && !bDone) || bPending )
    {
        SaveState( nToken );

        nToken = FilterToken( nToken );

        OSL_ENSURE( m_pPendStack || !m_bCallNextToken ||
                pCurTable->GetContext() || pCurTable->HasParentSection(),
                "Where is the section?" );
        if( !m_pPendStack && m_bCallNextToken &&
            (pCurTable->GetContext() || pCurTable->HasParentSection()) )
        {
            /// Call NextToken directly (e.g. ignore the content of floating frames or applets)
            NextToken( nToken );
        }
        else switch( nToken )
        {
        case HtmlTokenId::TABLE_ON:
            if( !pCurTable->GetContext() )
            {
                // If there's no table added, read the next table'
                SkipToken();
                bDone = true;
            }

            break;
        case HtmlTokenId::TABLE_OFF:
            bDone = true;
            break;
        case HtmlTokenId::CAPTION_ON:
            BuildTableCaption( pCurTable );
            bDone = m_pTable->IsOverflowing();
            break;
        case HtmlTokenId::COL_ON:
            SkipToken();
            BuildTableColGroup( pCurTable, false );
            break;
        case HtmlTokenId::COLGROUP_ON:
            BuildTableColGroup( pCurTable, true );
            break;
        case HtmlTokenId::TABLEROW_ON:
        case HtmlTokenId::TABLEHEADER_ON:
        case HtmlTokenId::TABLEDATA_ON:
            SkipToken();
            BuildTableSection( pCurTable, false, false );
            bDone = m_pTable->IsOverflowing();
            break;
        case HtmlTokenId::THEAD_ON:
        case HtmlTokenId::TFOOT_ON:
        case HtmlTokenId::TBODY_ON:
            BuildTableSection( pCurTable, true, HtmlTokenId::THEAD_ON==nToken );
            bDone = m_pTable->IsOverflowing();
            break;
        case HtmlTokenId::MULTICOL_ON:
            // we can't add columned text frames here
            break;
        case HtmlTokenId::FORM_ON:
            NewForm( false );   // don't add a new paragraph
            break;
        case HtmlTokenId::FORM_OFF:
            EndForm( false );   // don't add a new paragraph
            break;
        case HtmlTokenId::TEXTTOKEN:
            // blank strings may be a series of CR+LF and no text
            if( (pCurTable->GetContext() ||
                 !pCurTable->HasParentSection()) &&
                1==aToken.getLength() && ' '==aToken[0] )
                break;
            SAL_FALLTHROUGH;
        default:
            pCurTable->MakeParentContents();
            NextToken( nToken );
            break;
        }

        OSL_ENSURE( !bPending || !m_pPendStack,
                "SwHTMLParser::BuildTable: There is a PendStack again" );
        bPending = false;
        if( IsParserWorking() )
            SaveState( HtmlTokenId::NONE );

        if( !bDone )
            nToken = GetNextToken();
    }

    if( SvParserState::Pending == GetStatus() )
    {
        m_pPendStack = new SwPendingStack( HtmlTokenId::TABLE_ON, m_pPendStack );
        m_pPendStack->pData = pSaveStruct;
        return nullptr;
    }

    HTMLTableContext *pTCntxt = pCurTable->GetContext();
    if( pTCntxt )
    {

        // Modify table structure
        pCurTable->CloseTable();

        // end contexts that began out of cells. Needs to exist before (!) we move the table,
        // since the current one doesn't exist anymore afterwards
        while( m_aContexts.size() > m_nContextStAttrMin )
        {
            HTMLAttrContext *pCntxt = PopContext();
            ClearContext( pCntxt );
            delete pCntxt;
        }

        m_nContextStMin = pTCntxt->GetContextStMin();
        m_nContextStAttrMin = pTCntxt->GetContextStAttrMin();

        if( m_pTable==pCurTable )
        {
            // Set table caption
            const SwStartNode *pCapStNd = m_pTable->GetCaptionStartNode();
            if( pCapStNd )
            {
                // The last paragraph of the section is never part of the copy.
                // That's why the section needs to contain at least two paragraphs

                if( pCapStNd->EndOfSectionIndex() - pCapStNd->GetIndex() > 2 )
                {
                    // Don't copy start node and the last paragraph
                    SwNodeRange aSrcRg( *pCapStNd, 1,
                                    *pCapStNd->EndOfSectionNode(), -1 );

                    bool bTop = m_pTable->IsTopCaption();
                    SwStartNode *pTableStNd = pTCntxt->GetTableNode();

                    OSL_ENSURE( pTableStNd, "Where is the table node" );
                    OSL_ENSURE( pTableStNd==m_pPam->GetNode().FindTableNode(),
                            "Are we in the wrong table?" );

                    SwNode* pNd;
                    if( bTop )
                        pNd = pTableStNd;
                    else
                        pNd = pTableStNd->EndOfSectionNode();
                    SwNodeIndex aDstIdx( *pNd, bTop ? 0 : 1 );

                    m_xDoc->getIDocumentContentOperations().MoveNodeRange( aSrcRg, aDstIdx,
                        SwMoveFlags::DEFAULT );

                    // If the caption was added before the table, a page style on that table
                    // needs to be moved to the first paragraph of the header.
                    // Additionally, all remembered indices that point to the table node
                    // need to be moved
                    if( bTop )
                    {
                        MovePageDescAttrs( pTableStNd, aSrcRg.aStart.GetIndex(),
                                           false );
                    }
                }

                // The section isn't needed anymore
                m_pPam->SetMark();
                m_pPam->DeleteMark();
                m_xDoc->getIDocumentContentOperations().DeleteSection( const_cast<SwStartNode *>(pCapStNd) );
                m_pTable->SetCaption( nullptr, false );
            }

            // Process SwTable
            sal_uInt16 nBrowseWidth = (sal_uInt16)GetCurrentBrowseWidth();
            pSaveStruct->MakeTable( nBrowseWidth, *m_pPam->GetPoint(), m_xDoc.get() );
        }

        GetNumInfo().Set( pTCntxt->GetNumInfo() );
        pTCntxt->RestorePREListingXMP( *this );
        RestoreAttrTab( pTCntxt->aAttrTab );

        if( m_pTable==pCurTable )
        {
            // Set upper paragraph spacing
            m_bUpperSpace = true;
            SetTextCollAttrs();

            m_nParaCnt = m_nParaCnt - std::min(m_nParaCnt,
                pTCntxt->GetTableNode()->GetTable().GetTabSortBoxes().size());

            // Jump to a table if needed
            if( JUMPTO_TABLE == m_eJumpTo && m_pTable->GetSwTable() &&
                m_pTable->GetSwTable()->GetFrameFormat()->GetName() == m_sJmpMark )
            {
                m_bChkJumpMark = true;
                m_eJumpTo = JUMPTO_NONE;
            }

            // If the import was canceled, don't call Show again here since
            // the SwViewShell was already deleted
            // That's not enough. Even in the ACCEPTING_STATE, a Show mustn't be called
            // because otherwise the parser's gonna be destroyed on the reschedule,
            // if there's still a DataAvailable link coming. So: only in the WORKING state
            if( !m_nParaCnt && SvParserState::Working == GetStatus() )
                Show();
        }
    }
    else if( m_pTable==pCurTable )
    {
        // There was no table read

        // We maybe need to delete a read caption
        const SwStartNode *pCapStNd = pCurTable->GetCaptionStartNode();
        if( pCapStNd )
        {
            m_pPam->SetMark();
            m_pPam->DeleteMark();
            m_xDoc->getIDocumentContentOperations().DeleteSection( const_cast<SwStartNode *>(pCapStNd) );
            pCurTable->SetCaption( nullptr, false );
        }
    }

    if( m_pTable == pCurTable  )
    {
        delete pSaveStruct->m_pCurrentTable;
        pSaveStruct->m_pCurrentTable = nullptr;
        m_pTable = nullptr;
    }

    HTMLTable* pRetTable = pSaveStruct->m_pCurrentTable;
    delete pSaveStruct;

    return pRetTable;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
