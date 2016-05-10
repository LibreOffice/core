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

static HTMLOptionEnum aHTMLTableVAlignTable[] =
{
    { OOO_STRING_SVTOOLS_HTML_VA_top,         text::VertOrientation::NONE       },
    { OOO_STRING_SVTOOLS_HTML_VA_middle,      text::VertOrientation::CENTER     },
    { OOO_STRING_SVTOOLS_HTML_VA_bottom,      text::VertOrientation::BOTTOM     },
    { nullptr,                    0               }
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

class _HTMLTableContext
{
    SwHTMLNumRuleInfo aNumRuleInfo; // Numbering valid before the table

    SwTableNode *pTableNd;            // table node
    SwFrameFormat *pFrameFormat;              // der Fly frame::Frame, containing the table
    SwPosition *pPos;               // position behind the table

    size_t nContextStAttrMin;
    size_t nContextStMin;

    bool    bRestartPRE : 1;
    bool    bRestartXMP : 1;
    bool    bRestartListing : 1;

public:

    _HTMLAttrTable aAttrTab;        // attributes

    _HTMLTableContext( SwPosition *pPs, size_t nCntxtStMin,
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
        memset( &aAttrTab, 0, sizeof( _HTMLAttrTable ));
    }

    ~_HTMLTableContext();

    void SetNumInfo( const SwHTMLNumRuleInfo& rInf ) { aNumRuleInfo.Set(rInf); }
    const SwHTMLNumRuleInfo& GetNumInfo() const { return aNumRuleInfo; };

    void SavePREListingXMP( SwHTMLParser& rParser );
    void RestorePREListingXMP( SwHTMLParser& rParser );

    SwPosition *GetPos() const { return pPos; }

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
    std::shared_ptr<SvxBoxItem> GetBoxItem() const { return m_pBoxItem; }

    inline bool GetNumFormat( sal_uInt32& rNumFormat ) const;
    inline bool GetValue( double& rValue ) const;

    sal_Int16 GetVertOri() const { return eVertOri; }

    // Is the cell filled or protected ?
    bool IsUsed() const { return pContents!=nullptr || bProtected; }

    SwHTMLTableLayoutCell *CreateLayoutInfo();

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

    explicit HTMLTableRow( sal_uInt16 nCells=0 );    // cells of the row are empty

    ~HTMLTableRow();

    inline void SetHeight( sal_uInt16 nHeight );
    sal_uInt16 GetHeight() const { return nHeight; }

    inline HTMLTableCell *GetCell( sal_uInt16 nCell ) const;

    inline void SetAdjust( SvxAdjust eAdj ) { eAdjust = eAdj; }
    inline SvxAdjust GetAdjust() const { return eAdjust; }

    inline void SetVertOri( sal_Int16 eV) { eVertOri = eV; }
    inline sal_Int16 GetVertOri() const { return eVertOri; }

    void SetBGBrush( SvxBrushItem *pBrush ) { pBGBrush = pBrush; }
    const SvxBrushItem *GetBGBrush() const { return pBGBrush; }

    inline void SetEndOfGroup() { bIsEndOfGroup = true; }
    inline bool IsEndOfGroup() const { return bIsEndOfGroup; }

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

    inline void SetAdjust( SvxAdjust eAdj ) { eAdjust = eAdj; }
    inline SvxAdjust GetAdjust() const { return eAdjust; }

    inline void SetVertOri( sal_Int16 eV) { eVertOri = eV; }
    inline sal_Int16 GetVertOri() const { return eVertOri; }

    inline void SetEndOfGroup() { bIsEndOfGroup = true; }
    inline bool IsEndOfGroup() const { return bIsEndOfGroup; }

    inline void SetFrameFormat( SwFrameFormat *pFormat, bool bBorderLine,
                           sal_Int16 eVertOri );
    inline SwFrameFormat *GetFrameFormat( bool bBorderLine,
                                sal_Int16 eVertOri ) const;

    SwHTMLTableLayoutColumn *CreateLayoutInfo();
};

// HTML table
typedef std::vector<std::unique_ptr<HTMLTableRow>> HTMLTableRows;

typedef std::vector<std::unique_ptr<HTMLTableColumn>> HTMLTableColumns;

typedef std::vector<SdrObject *> SdrObjects;

class HTMLTable
{
    OUString aId;
    OUString aStyle;
    OUString aClass;
    OUString aDir;

    SdrObjects *pResizeDrawObjs;// SDR objects
    std::vector<sal_uInt16> *pDrawObjPrcWidths;   // column of draw object and its rel. width

    HTMLTableRows *m_pRows;         ///< table rows
    HTMLTableColumns *m_pColumns;   ///< table columns

    sal_uInt16 nRows;                   // number of rows
    sal_uInt16 nCols;                   // number of columns
    sal_uInt16 nFilledCols;             // number of filled columns

    sal_uInt16 nCurRow;                 // current Row
    sal_uInt16 nCurCol;                 // current Column

    sal_uInt16 nLeftMargin;             // Space to the left margin (from paragraph edge)
    sal_uInt16 nRightMargin;            // Space to the right margin (from paragraph edge)

    sal_uInt16 nCellPadding;            // Space from border to Text
    sal_uInt16 nCellSpacing;            // Space between two cells
    sal_uInt16 nHSpace;
    sal_uInt16 nVSpace;

    sal_uInt16 nBoxes;                  // number of boxes in the table

    const SwStartNode *pPrevStNd;   // the Table-Node or the Start-Node of the section before
    const SwTable *pSwTable;        // SW-Table (only on Top-Level)
    SwTableBox *pBox1;              // TableBox, generated when the Top-Level-Table was build
    SwTableBoxFormat *pBoxFormat;         // frame::Frame-Format from SwTableBox
    SwTableLineFormat *pLineFormat;       // frame::Frame-Format from SwTableLine
    SwTableLineFormat *pLineFrameFormatNoHeight;
    SvxBrushItem *pBGBrush;         // background of the table
    SvxBrushItem *pInhBGBrush;      // "inherited" background of the table
    const SwStartNode *pCaptionStartNode;   // Start-Node of the table-caption
    //lines for the border
    SvxBorderLine aTopBorderLine;
    SvxBorderLine aBottomBorderLine;
    SvxBorderLine aLeftBorderLine;
    SvxBorderLine aRightBorderLine;
    SvxBorderLine aBorderLine;
    SvxBorderLine aInhLeftBorderLine;
    SvxBorderLine aInhRightBorderLine;
    bool bTopBorder;                // is there a line on the top of the table
    bool bRightBorder;              // is there a line on the top right of the table
    bool bTopAlwd;                  // is it allowed to set the border?
    bool bRightAlwd;
    bool bFillerTopBorder;          // gets the left/right filler-cell a border on the
    bool bFillerBottomBorder;       // top or in the bottom
    bool bInhLeftBorder;
    bool bInhRightBorder;
    bool bBordersSet;               // the border is setted already
    bool bForceFrame;
    bool bTableAdjustOfTag;         // comes nTableAdjust from <TABLE>?
    sal_uInt32 nHeadlineRepeat;         // repeating rows
    bool bIsParentHead;
    bool bHasParentSection;
    bool bHasToFly;
    bool bFixedCols;
    bool bColSpec;                  // where there COL(GROUP)-elements?
    bool bPrcWidth;                 // width is declared in %

    SwHTMLParser *pParser;          // the current parser
    HTMLTable *pTopTable;           // the table on the Top-Level
    HTMLTableCnts *pParentContents;

    _HTMLTableContext *pContext;    // the context of the table

    SwHTMLTableLayout *pLayoutInfo;

    // the following parameters are from the <TABLE>-Tag
    sal_uInt16 nWidth;                  // width of the table
    sal_uInt16 nHeight;                 // absolute height of the table
    SvxAdjust eTableAdjust;         // drawing::Alignment of the table
    sal_Int16 eVertOri;         // Default vertical direction of the cells
    sal_uInt16 nBorder;                 // width of the external border
    HTMLTableFrame eFrame;          // frame around the table
    HTMLTableRules eRules;          // frame in the table
    bool bTopCaption;               // Caption of the table

    void InitCtor( const HTMLTableOptions *pOptions );

    // Correction of the Row-Spans for all cells above the chosen cell and the cell itself for the indicated content. The chosen cell gets the Row-Span 1
    void FixRowSpan( sal_uInt16 nRow, sal_uInt16 nCol, const HTMLTableCnts *pCnts );

    // Protects the chosen cell and the cells among
    void ProtectRowSpan( sal_uInt16 nRow, sal_uInt16 nCol, sal_uInt16 nRowSpan );

    // Looking for the SwStartNodes of the box ahead
    // If nRow==nCell==USHRT_MAX, return the last Start-Node of the table.
    const SwStartNode* GetPrevBoxStartNode( sal_uInt16 nRow, sal_uInt16 nCell ) const;

    sal_uInt16 GetTopCellSpace( sal_uInt16 nRow, sal_uInt16 nRowSpan,
                            bool bSwBorders=true ) const;
    sal_uInt16 GetBottomCellSpace( sal_uInt16 nRow, sal_uInt16 nRowSpan,
                               bool bSwBorders=true ) const;

    // Conforming of the frame::Frame-Format of the box
    void FixFrameFormat( SwTableBox *pBox, sal_uInt16 nRow, sal_uInt16 nCol,
                      sal_uInt16 nRowSpan, sal_uInt16 nColSpan,
                      bool bFirstPara=true, bool bLastPara=true ) const;
    void FixFillerFrameFormat( SwTableBox *pBox, bool bRight ) const;

    // Create a table with the content (lines/boxes)
    void _MakeTable( SwTableBox *pUpper=nullptr );

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
                         sal_uInt16 nRowSpan, sal_uInt16 nColSpan,
                         bool bFirstPara, bool bLastPara );

    // Inherit the left and the right border of the surrounding table
    void InheritVertBorders( const HTMLTable *pParent,
                             sal_uInt16 nCol, sal_uInt16 nColSpan );

    // Set the border with the help of the information from the user
    void SetBorders();

    // is the border already setted?
    bool BordersSet() const { return bBordersSet; }

    const SvxBrushItem *GetBGBrush() const { return pBGBrush; }
    const SvxBrushItem *GetInhBGBrush() const { return pInhBGBrush; }

    sal_uInt16 GetBorderWidth( const SvxBorderLine& rBLine,
                           bool bWithDistance=false ) const;

public:

    bool bFirstCell;                // wurde schon eine Zelle angelegt?

    HTMLTable( SwHTMLParser* pPars, HTMLTable *pTopTab,
               bool bParHead, bool bHasParentSec,
               bool bHasToFly,
               const HTMLTableOptions *pOptions );

    ~HTMLTable();

    // Identifying of a cell
    inline HTMLTableCell *GetCell( sal_uInt16 nRow, sal_uInt16 nCell ) const;

    // set/determine caption
    inline void SetCaption( const SwStartNode *pStNd, bool bTop );
    const SwStartNode *GetCaptionStartNode() const { return pCaptionStartNode; }
    bool IsTopCaption() const { return bTopCaption; }

    SvxAdjust GetTableAdjust( bool bAny ) const
    {
        return (bTableAdjustOfTag || bAny) ? eTableAdjust : SVX_ADJUST_END;
    }

    sal_uInt16 GetHSpace() const { return nHSpace; }
    sal_uInt16 GetVSpace() const { return nVSpace; }

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

    // Beenden einer Tab-Definition (MUSS fuer ALLE Tabs aufgerufen werden)
    void CloseTable();

    // SwTable konstruieren (inkl. der Child-Tabellen)
    void MakeTable( SwTableBox *pUpper, sal_uInt16 nAbsAvail,
                    sal_uInt16 nRelAvail=0, sal_uInt16 nAbsLeftSpace=0,
                    sal_uInt16 nAbsRightSpace=0, sal_uInt16 nInhAbsSpace=0 );

    inline bool IsNewDoc() const { return pParser->IsNewDoc(); }

    void SetHasParentSection( bool bSet ) { bHasParentSection = bSet; }
    bool HasParentSection() const { return bHasParentSection; }

    void SetParentContents( HTMLTableCnts *pCnts ) { pParentContents = pCnts; }
    HTMLTableCnts *GetParentContents() const { return pParentContents; }

    void MakeParentContents();

    bool GetIsParentHeader() const { return bIsParentHead; }

    bool HasToFly() const { return bHasToFly; }

    void SetTable( const SwStartNode *pStNd, _HTMLTableContext *pCntxt,
                   sal_uInt16 nLeft, sal_uInt16 nRight,
                   const SwTable *pSwTab=nullptr, bool bFrcFrame=false );

    _HTMLTableContext *GetContext() const { return pContext; }

    SwHTMLTableLayout *CreateLayoutInfo();

    bool HasColTags() const { return bColSpec; }

    sal_uInt16 IncGrfsThatResize() { return pSwTable ? const_cast<SwTable *>(pSwTable)->IncGrfsThatResize() : 0; }

    void RegisterDrawObject( SdrObject *pObj, sal_uInt8 nPrcWidth );

    const SwTable *GetSwTable() const { return pSwTable; }

    void SetBGBrush( const SvxBrushItem& rBrush ) { delete pBGBrush; pBGBrush = new SvxBrushItem( rBrush ); }

    const OUString& GetId() const { return aId; }
    const OUString& GetClass() const { return aClass; }
    const OUString& GetStyle() const { return aStyle; }
    const OUString& GetDirection() const { return aDir; }

    void IncBoxCount() { nBoxes++; }
    bool IsOverflowing() const { return nBoxes > 64000; }
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
    delete m_pTable;              // die Tabellen brauchen wir nicht mehr
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
    OSL_ENSURE( m_pLayoutInfo, "Da sit noch keine Layout-Info" );
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
    // der Inhalt ist in mehrere Zellen eingetragen, darf aber nur einmal
    // geloescht werden
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
    // Die Inhalte dieser Zelle mussen nich irgenwo anders verankert
    // sein, weil sie nicht geloescht werden!!!

    // Inhalt loeschen
    pContents = nullptr;

    // Hintergrundfarbe kopieren.
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

SwHTMLTableLayoutCell *HTMLTableCell::CreateLayoutInfo()
{
    SwHTMLTableLayoutCnts *pCntInfo = pContents ? pContents->CreateLayoutInfo() : nullptr;

    return new SwHTMLTableLayoutCell( pCntInfo, nRowSpan, nColSpan, nWidth,
                                      bRelWidth, bNoWrap );
}

HTMLTableRow::HTMLTableRow(sal_uInt16 const nCells)
    : m_pCells(new HTMLTableCells),
    bIsEndOfGroup(false),
    nHeight(0),
    nEmptyRows(0),
    eAdjust(SVX_ADJUST_END),
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
        "ungueltiger Zellen-Index in HTML-Tabellenzeile" );
    return (*m_pCells)[nCell].get();
}

void HTMLTableRow::Expand( sal_uInt16 nCells, bool bOneCell )
{
    // die Zeile wird mit einer einzigen Zelle aufgefuellt, wenn
    // bOneCell gesetzt ist. Das geht, nur fuer Zeilen, in die keine
    // Zellen mehr eingefuegt werden!

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
                "RowSpan von zu loesender Zelle ist falsch" );
        OSL_ENSURE( pCell->GetColSpan() == nEnd - i,
                    "ColSpan von zu loesender Zelle ist falsch" );
        OSL_ENSURE( !pCell->GetContents(), "Zu loeschende Zelle hat Inhalt" );
    }
#endif

    m_pCells->erase( m_pCells->begin() + nCells, m_pCells->end() );
}

HTMLTableColumn::HTMLTableColumn():
    bIsEndOfGroup(false),
    nWidth(0), bRelWidth(false),
    eAdjust(SVX_ADJUST_END), eVertOri(text::VertOrientation::TOP),
    bLeftBorder(false)
{
    for( sal_uInt16 i=0; i<6; i++ )
        aFrameFormats[i] = nullptr;
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

inline SwHTMLTableLayoutColumn *HTMLTableColumn::CreateLayoutInfo()
{
    return new SwHTMLTableLayoutColumn( nWidth, bRelWidth, bLeftBorder );
}

inline sal_uInt16 HTMLTableColumn::GetFrameFormatIdx( bool bBorderLine,
                                             sal_Int16 eVertOrient )
{
    OSL_ENSURE( text::VertOrientation::TOP != eVertOrient, "Top ist nicht erlaubt" );
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
    pResizeDrawObjs = nullptr;
    pDrawObjPrcWidths = nullptr;

    m_pRows = new HTMLTableRows;
    m_pColumns = new HTMLTableColumns;
    nRows = 0;
    nCurRow = 0; nCurCol = 0;

    pBox1 = nullptr;
    pBoxFormat = nullptr; pLineFormat = nullptr;
    pLineFrameFormatNoHeight = nullptr;
    pInhBGBrush = nullptr;

    pPrevStNd = nullptr;
    pSwTable = nullptr;

    bTopBorder = false; bRightBorder = false;
    bTopAlwd = true; bRightAlwd = true;
    bFillerTopBorder = false; bFillerBottomBorder = false;
    bInhLeftBorder = false; bInhRightBorder = false;
    bBordersSet = false;
    bForceFrame = false;
    nHeadlineRepeat = 0;

    nLeftMargin = 0;
    nRightMargin = 0;

    const Color& rBorderColor = pOptions->aBorderColor;

    long nBorderOpt = (long)pOptions->nBorder;
    long nPWidth = nBorderOpt==USHRT_MAX ? NETSCAPE_DFLT_BORDER
                                         : nBorderOpt;
    long nPHeight = nBorderOpt==USHRT_MAX ? 0 : nBorderOpt;
    SvxCSS1Parser::PixelToTwip( nPWidth, nPHeight );

    // nBorder gibt die Breite der Umrandung an, wie sie in die
    // Breitenberechnung in Netscape einfliesst. Wenn pOption->nBorder
    // == USHRT_MAX, wurde keine BORDER-Option angegeben. Trotzdem fliesst
    // eine 1 Pixel breite Umrandung in die Breitenberechnung mit ein.
    nBorder = (sal_uInt16)nPWidth;
    if( nBorderOpt==USHRT_MAX )
        nPWidth = 0;

    // HACK: ein Pixel-breite Linien sollen zur Haarlinie werden, wenn
    // wir mit doppelter Umrandung arbeiten
    if( pOptions->nCellSpacing!=0 && nBorderOpt==1 )
    {
        nPWidth = 1;
        nPHeight = 1;
    }

    if ( pOptions->nCellSpacing != 0 )
    {
        aTopBorderLine.SetBorderLineStyle(table::BorderLineStyle::DOUBLE);
    }
    aTopBorderLine.SetWidth( nPHeight );
    aTopBorderLine.SetColor( rBorderColor );
    aBottomBorderLine = aTopBorderLine;

    if( nPWidth == nPHeight )
    {
        aLeftBorderLine = aTopBorderLine;
    }
    else
    {
        if ( pOptions->nCellSpacing != 0 )
        {
            aLeftBorderLine.SetBorderLineStyle(table::BorderLineStyle::DOUBLE);
        }
        aLeftBorderLine.SetWidth( nPWidth );
        aLeftBorderLine.SetColor( rBorderColor );
    }
    aRightBorderLine = aLeftBorderLine;

    if( pOptions->nCellSpacing != 0 )
    {
        aBorderLine.SetBorderLineStyle(table::BorderLineStyle::DOUBLE);
        aBorderLine.SetWidth( DEF_LINE_WIDTH_0 );
    }
    else
    {
        aBorderLine.SetWidth( DEF_LINE_WIDTH_0 );
    }
    aBorderLine.SetColor( rBorderColor );

    if( nCellPadding )
    {
        if( nCellPadding==USHRT_MAX )
            nCellPadding = MIN_BORDER_DIST; // default
        else
        {
            nCellPadding = SwHTMLParser::ToTwips( nCellPadding );
            if( nCellPadding<MIN_BORDER_DIST  )
                nCellPadding = MIN_BORDER_DIST;
        }
    }
    if( nCellSpacing )
    {
        if( nCellSpacing==USHRT_MAX )
            nCellSpacing = NETSCAPE_DFLT_CELLSPACING;
        nCellSpacing = SwHTMLParser::ToTwips( nCellSpacing );
    }

    nPWidth = pOptions->nHSpace;
    nPHeight = pOptions->nVSpace;
    SvxCSS1Parser::PixelToTwip( nPWidth, nPHeight );
    nHSpace = (sal_uInt16)nPWidth;
    nVSpace = (sal_uInt16)nPHeight;

    bColSpec = false;

    pBGBrush = pParser->CreateBrushItem(
                    pOptions->bBGColor ? &(pOptions->aBGColor) : nullptr,
                    pOptions->aBGImage, aEmptyOUStr, aEmptyOUStr, aEmptyOUStr );

    pContext = nullptr;
    pParentContents = nullptr;

    aId = pOptions->aId;
    aClass = pOptions->aClass;
    aStyle = pOptions->aStyle;
    aDir = pOptions->aDir;
}

HTMLTable::HTMLTable( SwHTMLParser* pPars, HTMLTable *pTopTab,
                      bool bParHead,
                      bool bHasParentSec, bool bHasToFlw,
                      const HTMLTableOptions *pOptions ) :
    nCols( pOptions->nCols ),
    nFilledCols( 0 ),
    nCellPadding( pOptions->nCellPadding ),
    nCellSpacing( pOptions->nCellSpacing ),
    nBoxes( 1 ),
    pCaptionStartNode( nullptr ),
    bTableAdjustOfTag( !pTopTab && pOptions->bTableAdjust ),
    bIsParentHead( bParHead ),
    bHasParentSection( bHasParentSec ),
    bHasToFly( bHasToFlw ),
    bFixedCols( pOptions->nCols>0 ),
    bPrcWidth( pOptions->bPrcWidth ),
    pParser( pPars ),
    pTopTable( pTopTab ? pTopTab : this ),
    pLayoutInfo( nullptr ),
    nWidth( pOptions->nWidth ),
    nHeight( pTopTab ? 0 : pOptions->nHeight ),
    eTableAdjust( pOptions->eAdjust ),
    eVertOri( pOptions->eVertOri ),
    eFrame( pOptions->eFrame ),
    eRules( pOptions->eRules ),
    bTopCaption( false ),
    bFirstCell( !pTopTab )
{
    InitCtor( pOptions );

    for( sal_uInt16 i=0; i<nCols; i++ )
        m_pColumns->push_back(o3tl::make_unique<HTMLTableColumn>());
}

HTMLTable::~HTMLTable()
{
    delete pResizeDrawObjs;
    delete pDrawObjPrcWidths;

    delete m_pRows;
    delete m_pColumns;
    delete pBGBrush;
    delete pInhBGBrush;

    delete pContext;

    // pLayoutInfo wurde entweder bereits geloescht oder muss aber es
    // in den Besitz der SwTable uebergegangen.
}

SwHTMLTableLayout *HTMLTable::CreateLayoutInfo()
{
    sal_uInt16 nW = bPrcWidth ? nWidth : SwHTMLParser::ToTwips( nWidth );

    sal_uInt16 nBorderWidth = GetBorderWidth( aBorderLine, true );
    sal_uInt16 nLeftBorderWidth =
        (*m_pColumns)[0]->bLeftBorder ? GetBorderWidth(aLeftBorderLine, true) : 0;
    sal_uInt16 nRightBorderWidth =
        bRightBorder ? GetBorderWidth( aRightBorderLine, true ) : 0;
    sal_uInt16 nInhLeftBorderWidth = 0;
    sal_uInt16 nInhRightBorderWidth = 0;

    pLayoutInfo = new SwHTMLTableLayout(
                        pSwTable,
                        nRows, nCols, bFixedCols, bColSpec,
                        nW, bPrcWidth, nBorder, nCellPadding,
                        nCellSpacing, eTableAdjust,
                        nLeftMargin, nRightMargin,
                        nBorderWidth, nLeftBorderWidth, nRightBorderWidth,
                        nInhLeftBorderWidth, nInhRightBorderWidth );

    bool bExportable = true;
    sal_uInt16 i;
    for( i=0; i<nRows; i++ )
    {
        HTMLTableRow *const pRow = (*m_pRows)[i].get();
        for( sal_uInt16 j=0; j<nCols; j++ )
        {
            SwHTMLTableLayoutCell *pLayoutCell =
                pRow->GetCell(j)->CreateLayoutInfo();

            pLayoutInfo->SetCell( pLayoutCell, i, j );

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

    pLayoutInfo->SetExportable( bExportable );

    for( i=0; i<nCols; i++ )
        pLayoutInfo->SetColumn( ((*m_pColumns)[i])->CreateLayoutInfo(), i );

    return pLayoutInfo;
}

inline void HTMLTable::SetCaption( const SwStartNode *pStNd, bool bTop )
{
    pCaptionStartNode = pStNd;
    bTopCaption = bTop;
}

void HTMLTable::FixRowSpan( sal_uInt16 nRow, sal_uInt16 nCol,
                            const HTMLTableCnts *pCnts )
{
    sal_uInt16 nRowSpan=1;
    HTMLTableCell *pCell;
    while( ( pCell=GetCell(nRow,nCol), pCell->GetContents()==pCnts ) )
    {
        pCell->SetRowSpan( nRowSpan );
        if( pLayoutInfo )
            pLayoutInfo->GetCell(nRow,nCol)->SetRowSpan( nRowSpan );

        if( !nRow ) break;
        nRowSpan++; nRow--;
    }
}

void HTMLTable::ProtectRowSpan( sal_uInt16 nRow, sal_uInt16 nCol, sal_uInt16 nRowSpan )
{
    for( sal_uInt16 i=0; i<nRowSpan; i++ )
    {
        GetCell(nRow+i,nCol)->SetProtected();
        if( pLayoutInfo )
            pLayoutInfo->GetCell(nRow+i,nCol)->SetProtected();
    }
}

// Suchen des SwStartNodes der letzten belegten Vorgaengerbox
const SwStartNode* HTMLTable::GetPrevBoxStartNode( sal_uInt16 nRow, sal_uInt16 nCol ) const
{
    const HTMLTableCnts *pPrevCnts = nullptr;

    if( 0==nRow )
    {
        // immer die Vorgaenger-Zelle
        if( nCol>0 )
            pPrevCnts = GetCell( 0, nCol-1 )->GetContents();
        else
            return pPrevStNd;
    }
    else if( USHRT_MAX==nRow && USHRT_MAX==nCol )
        // der Contents der letzten Zelle
        pPrevCnts = GetCell( nRows-1, nCols-1 )->GetContents();
    else
    {
        sal_uInt16 i;
        HTMLTableRow *const pPrevRow = (*m_pRows)[nRow-1].get();

        // evtl. eine Zelle in der aktuellen Zeile
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

        // sonst die letzte gefuellte Zelle der Zeile davor suchen
        if( !pPrevCnts )
        {
            i = nCols;
            while( !pPrevCnts && i )
            {
                i--;
                pPrevCnts = pPrevRow->GetCell(i)->GetContents();
            }
        }
    }
    OSL_ENSURE( pPrevCnts, "keine gefuellte Vorgaenger-Zelle gefunden" );
    if( !pPrevCnts )
    {
        pPrevCnts = GetCell(0,0)->GetContents();
        if( !pPrevCnts )
            return pPrevStNd;
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

sal_uInt16 HTMLTable::GetTopCellSpace( sal_uInt16 nRow, sal_uInt16 nRowSpan,
                                   bool bSwBorders ) const
{
    sal_uInt16 nSpace = nCellPadding;

    if( nRow == 0 )
    {
        nSpace += nBorder + nCellSpacing;
        if( bSwBorders )
        {
            sal_uInt16 nTopBorderWidth =
                GetBorderWidth( aTopBorderLine, true );
            if( nSpace < nTopBorderWidth )
                nSpace = nTopBorderWidth;
        }
    }
    else if (bSwBorders && (*m_pRows)[nRow+nRowSpan-1]->bBottomBorder &&
             nSpace < MIN_BORDER_DIST )
    {
        OSL_ENSURE( !nCellPadding, "GetTopCellSpace: CELLPADDING!=0" );
        // Wenn die Gegenueberliegende Seite umrandet ist muessen
        // wir zumindest den minimalen Abstand zum Inhalt
        // beruecksichtigen. (Koennte man zusaetzlich auch an
        // nCellPadding festmachen.)
        nSpace = MIN_BORDER_DIST;
    }

    return nSpace;
}

sal_uInt16 HTMLTable::GetBottomCellSpace( sal_uInt16 nRow, sal_uInt16 nRowSpan,
                                      bool bSwBorders ) const
{
    sal_uInt16 nSpace = nCellSpacing + nCellPadding;

    if( nRow+nRowSpan == nRows )
    {
        nSpace = nSpace + nBorder;

        if( bSwBorders )
        {
            sal_uInt16 nBottomBorderWidth =
                GetBorderWidth( aBottomBorderLine, true );
            if( nSpace < nBottomBorderWidth )
                nSpace = nBottomBorderWidth;
        }
    }
    else if( bSwBorders )
    {
        if ((*m_pRows)[nRow+nRowSpan+1]->bBottomBorder)
        {
            sal_uInt16 nBorderWidth = GetBorderWidth( aBorderLine, true );
            if( nSpace < nBorderWidth )
                nSpace = nBorderWidth;
        }
        else if( nRow==0 && bTopBorder && nSpace < MIN_BORDER_DIST )
        {
            OSL_ENSURE( GetBorderWidth( aTopBorderLine, true ) > 0,
                    "GetBottomCellSpace: |aTopLine| == 0" );
            OSL_ENSURE( !nCellPadding, "GetBottomCellSpace: CELLPADDING!=0" );
            // Wenn die Gegenueberliegende Seite umrandet ist muessen
            // wir zumindest den minimalen Abstand zum Inhalt
            // beruecksichtigen. (Koennte man zusaetzlich auch an
            // nCellPadding festmachen.)
            nSpace = MIN_BORDER_DIST;
        }
    }

    return nSpace;
}

void HTMLTable::FixFrameFormat( SwTableBox *pBox,
                             sal_uInt16 nRow, sal_uInt16 nCol,
                             sal_uInt16 nRowSpan, sal_uInt16 nColSpan,
                             bool bFirstPara, bool bLastPara ) const
{
    SwFrameFormat *pFrameFormat = nullptr;      // frame::Frame-Format
    sal_Int16 eVOri = text::VertOrientation::NONE;
    const SvxBrushItem *pBGBrushItem = nullptr;   // Hintergrund
    std::shared_ptr<SvxBoxItem> pBoxItem;
    bool bTopLine = false, bBottomLine = false, bLastBottomLine = false;
    bool bReUsable = false;     // Format nochmals verwendbar?
    sal_uInt16 nEmptyRows = 0;
    bool bHasNumFormat = false;
    bool bHasValue = false;
    sal_uInt32 nNumFormat = 0;
    double nValue = 0.0;

    HTMLTableColumn *const pColumn = (*m_pColumns)[nCol].get();

    if( pBox->GetSttNd() )
    {
        // die Hintergrundfarbe/-grafik bestimmen
        const HTMLTableCell *pCell = GetCell( nRow, nCol );
        pBoxItem = pCell->GetBoxItem();
        pBGBrushItem = pCell->GetBGBrush();
        if( !pBGBrushItem )
        {
            // Wenn die Zelle ueber mehrere Zeilen geht muss ein evtl.
            // an der Zeile gesetzter Hintergrund an die Zelle uebernommen
            // werden.
            // Wenn es sich um eine Tabelle in der Tabelle handelt und
            // die Zelle ueber die gesamte Heoehe der Tabelle geht muss
            // ebenfalls der Hintergrund der Zeile uebernommen werden, weil
            // die Line von der GC (zu Recht) wegoptimiert wird.
            if( nRowSpan > 1 || (this != pTopTable && nRowSpan==nRows) )
            {
                pBGBrushItem = (*m_pRows)[nRow]->GetBGBrush();
                if( !pBGBrushItem && this != pTopTable )
                {
                    pBGBrushItem = GetBGBrush();
                    if( !pBGBrushItem )
                        pBGBrushItem = GetInhBGBrush();
                }
            }
        }

        bTopLine = 0==nRow && bTopBorder && bFirstPara;
        if ((*m_pRows)[nRow+nRowSpan-1]->bBottomBorder && bLastPara)
        {
            nEmptyRows = (*m_pRows)[nRow+nRowSpan-1]->GetEmptyRows();
            if( nRow+nRowSpan == nRows )
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

        // die Breite der Box berechnen
        SwTwips nFrameWidth = (SwTwips)pLayoutInfo->GetColumn(nCol)
                                                ->GetRelColWidth();
        for( sal_uInt16 i=1; i<nColSpan; i++ )
            nFrameWidth += (SwTwips)pLayoutInfo->GetColumn(nCol+i)
                                             ->GetRelColWidth();

        // die Umrandung nur an Edit-Boxen setzen (bei der oberen und unteren
        // Umrandung muss beruecks. werden, ob es sich um den ersten oder
        // letzen Absatz der Zelle handelt)
        if( pBox->GetSttNd() )
        {
            bool bSet = (nCellPadding > 0);

            SvxBoxItem aBoxItem( RES_BOX );
            long nInnerFrameWidth = nFrameWidth;

            if( bTopLine )
            {
                aBoxItem.SetLine( &aTopBorderLine, SvxBoxItemLine::TOP );
                bSet = true;
            }
            if( bLastBottomLine )
            {
                aBoxItem.SetLine( &aBottomBorderLine, SvxBoxItemLine::BOTTOM );
                bSet = true;
            }
            else if( bBottomLine )
            {
                if( nEmptyRows && !aBorderLine.GetInWidth() )
                {
                    // Leere Zeilen koennen zur Zeit nur dann ueber
                    // dicke Linien simuliert werden, wenn die Linie
                    // einfach ist.
                    SvxBorderLine aThickBorderLine( aBorderLine );

                    sal_uInt16 nBorderWidth = aBorderLine.GetOutWidth();
                    nBorderWidth *= (nEmptyRows + 1);
                    aThickBorderLine.SetBorderLineStyle(
                            table::BorderLineStyle::SOLID);
                    aThickBorderLine.SetWidth( nBorderWidth );
                    aBoxItem.SetLine( &aThickBorderLine, SvxBoxItemLine::BOTTOM );
                }
                else
                {
                    aBoxItem.SetLine( &aBorderLine, SvxBoxItemLine::BOTTOM );
                }
                bSet = true;
            }
            if (((*m_pColumns)[nCol])->bLeftBorder)
            {
                const SvxBorderLine& rBorderLine =
                    0==nCol ? aLeftBorderLine : aBorderLine;
                aBoxItem.SetLine( &rBorderLine, SvxBoxItemLine::LEFT );
                nInnerFrameWidth -= GetBorderWidth( rBorderLine );
                bSet = true;
            }
            if( nCol+nColSpan == nCols && bRightBorder )
            {
                aBoxItem.SetLine( &aRightBorderLine, SvxBoxItemLine::RIGHT );
                nInnerFrameWidth -= GetBorderWidth( aRightBorderLine );
                bSet = true;
            }

            if (pBoxItem)
            {
                pFrameFormat->SetFormatAttr( *pBoxItem );
            }
            else if (bSet)
            {
                // BorderDist nicht mehr Bestandteil einer Zelle mit fixer Breite
                sal_uInt16 nBDist = static_cast< sal_uInt16 >(
                    (2*nCellPadding <= nInnerFrameWidth) ? nCellPadding
                                                      : (nInnerFrameWidth / 2) );
                // wir setzen das Item nur, wenn es eine Umrandung gibt
                // oder eine sheet::Border-Distanz vorgegeben ist. Fehlt letztere,
                // dann gibt es eine Umrandung, und wir muessen die Distanz
                // setzen
                aBoxItem.SetDistance( nBDist ? nBDist : MIN_BORDER_DIST );
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

            // Format nur setzten, wenn es auch einen Value gibt oder die Box leer ist.
            if( bHasNumFormat && (bHasValue || IsBoxEmpty(pBox)) )
            {
                bool bLock = pFrameFormat->GetDoc()->GetNumberFormatter()
                                     ->IsTextFormat( nNumFormat );
                SfxItemSet aItemSet( *pFrameFormat->GetAttrSet().GetPool(),
                                     RES_BOXATR_FORMAT, RES_BOXATR_VALUE );
                SvxAdjust eAdjust = SVX_ADJUST_END;
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
                else if( pCNd && SVX_ADJUST_END != eAdjust )
                {
                    SvxAdjustItem aAdjItem( eAdjust, RES_PARATR_ADJUST );
                    pCNd->SetAttr( aAdjItem );
                }
            }
            else
                pFrameFormat->ResetFormatAttr( RES_BOXATR_FORMAT );

            OSL_ENSURE( eVOri != text::VertOrientation::TOP, "text::VertOrientation::TOP ist nicht erlaubt!" );
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
                "Box ohne Inhalt hat vertikale Ausrichtung" );
        pBox->ChgFrameFormat( static_cast<SwTableBoxFormat*>(pFrameFormat) );
    }

}

void HTMLTable::FixFillerFrameFormat( SwTableBox *pBox, bool bRight ) const
{
    SwFrameFormat *pFrameFormat = pBox->ClaimFrameFormat();

    if( bFillerTopBorder || bFillerBottomBorder ||
        (!bRight && bInhLeftBorder) || (bRight && bInhRightBorder) )
    {
        SvxBoxItem aBoxItem( RES_BOX );
        if( bFillerTopBorder )
            aBoxItem.SetLine( &aTopBorderLine, SvxBoxItemLine::TOP );
        if( bFillerBottomBorder )
            aBoxItem.SetLine( &aBottomBorderLine, SvxBoxItemLine::BOTTOM );
        if( !bRight && bInhLeftBorder )
            aBoxItem.SetLine( &aInhLeftBorderLine, SvxBoxItemLine::LEFT );
        if( bRight && bInhRightBorder )
            aBoxItem.SetLine( &aInhRightBorderLine, SvxBoxItemLine::RIGHT );
        aBoxItem.SetDistance( MIN_BORDER_DIST );
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

    if( pTopTable->pBox1 &&
        pTopTable->pBox1->GetSttNd() == pStNd )
    {
        // wenn der StartNode dem StartNode der initial angelegten Box
        // entspricht nehmen wir diese Box
        pBox = pTopTable->pBox1;
        pBox->SetUpper( pUpper );
        pTopTable->pBox1 = nullptr;
    }
    else
        pBox = new SwTableBox( pBoxFormat, *pStNd, pUpper );

    return pBox;
}

static void ResetLineFrameFormatAttrs( SwFrameFormat *pFrameFormat )
{
    pFrameFormat->ResetFormatAttr( RES_FRM_SIZE );
    pFrameFormat->ResetFormatAttr( RES_BACKGROUND );
    OSL_ENSURE( SfxItemState::SET!=pFrameFormat->GetAttrSet().GetItemState(
                                RES_VERT_ORIENT, false ),
            "Zeile hat vertikale Ausrichtung" );
}

// !!! kann noch vereinfacht werden
SwTableLine *HTMLTable::MakeTableLine( SwTableBox *pUpper,
                                       sal_uInt16 nTopRow, sal_uInt16 nLeftCol,
                                       sal_uInt16 nBottomRow, sal_uInt16 nRightCol )
{
    SwTableLine *pLine;
    if( this==pTopTable && !pUpper && 0==nTopRow )
        pLine = (pSwTable->GetTabLines())[0];
    else
        pLine = new SwTableLine( pLineFrameFormatNoHeight ? pLineFrameFormatNoHeight
                                                     : pLineFormat,
                                 0, pUpper );

    HTMLTableRow *pTopRow = (*m_pRows)[nTopRow].get();
    sal_uInt16 nRowHeight = pTopRow->GetHeight();
    const SvxBrushItem *pBGBrushItem = nullptr;
    if( this == pTopTable || nTopRow>0 || nBottomRow<nRows )
    {
        // An der Line eine Frabe zu setzen macht keinen Sinn, wenn sie
        // die auesserste und gleichzeitig einzige Zeile einer Tabelle in
        // der Tabelle ist.
        pBGBrushItem = pTopRow->GetBGBrush();

        if( !pBGBrushItem && this != pTopTable )
        {
            // Ein an einer Tabellen in der Tabelle gesetzter Hintergrund
            // wird an den Rows gesetzt. Das gilt auch fuer den Hintergrund
            // der Zelle, in dem die Tabelle vorkommt.
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
            // Tabellenhoehe einstellen. Da es sich um eine
            // Mindesthoehe handelt, kann sie genauso wie in
            // Netscape berechnet werden, also ohne Beruecksichtigung
            // der tatsaechlichen Umrandungsbreite.
            nRowHeight += GetTopCellSpace( nTopRow, 1, false ) +
                       GetBottomCellSpace( nTopRow, 1, false );

            pFrameFormat->SetFormatAttr( SwFormatFrameSize( ATT_MIN_SIZE, 0, nRowHeight ) );
        }

        if( pBGBrushItem )
        {
            pFrameFormat->SetFormatAttr( *pBGBrushItem );
        }

    }
    else if( !pLineFrameFormatNoHeight )
    {
        // sonst muessen wir die Hoehe aus dem Attribut entfernen
        // und koennen uns das Format merken
        pLineFrameFormatNoHeight = static_cast<SwTableLineFormat*>(pLine->ClaimFrameFormat());

        ResetLineFrameFormatAttrs( pLineFrameFormatNoHeight );
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
            OSL_ENSURE( nCol < nRightCol, "Zu weit gelaufen" );

            HTMLTableCell *pCell = GetCell(nTopRow,nCol);
            const bool bSplit = 1 == pCell->GetColSpan();

            OSL_ENSURE((nCol != nRightCol-1) || bSplit, "Split-Flag wrong");
            if( bSplit )
            {
                SwTableBox* pBox = nullptr;
                HTMLTableCell *pCell2 = GetCell( nTopRow, nStartCol );
                if( pCell2->GetColSpan() == (nCol+1-nStartCol) )
                {
                    // Die HTML-Tabellen-Zellen bilden genau eine Box.
                    // Dann muss hinter der Box gesplittet werden
                    nSplitCol = nCol + 1;

                    long nBoxRowSpan = pCell2->GetRowSpan();
                    if ( !pCell2->GetContents() || pCell2->IsCovered() )
                    {
                        if ( pCell2->IsCovered() )
                            nBoxRowSpan = -1 * nBoxRowSpan;

                        const SwStartNode* pPrevStartNd =
                            GetPrevBoxStartNode( nTopRow, nStartCol );
                        HTMLTableCnts *pCnts = new HTMLTableCnts(
                            pParser->InsertTableSection(pPrevStartNd) );
                        SwHTMLTableLayoutCnts *pCntsLayoutInfo =
                            pCnts->CreateLayoutInfo();

                        pCell2->SetContents( pCnts );
                        SwHTMLTableLayoutCell *pCurrCell = pLayoutInfo->GetCell( nTopRow, nStartCol );
                        pCurrCell->SetContents( pCntsLayoutInfo );
                        if( nBoxRowSpan < 0 )
                            pCurrCell->SetRowSpan( 0 );

                        // ggf. COLSPAN beachten
                        for( sal_uInt16 j=nStartCol+1; j<nSplitCol; j++ )
                        {
                            GetCell(nTopRow,j)->SetContents( pCnts );
                            pLayoutInfo->GetCell( nTopRow, j )
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
        // nur eine Inhalts-Section
        if( pCnts->GetStartNode() )
        {
            // und die ist keine Tabelle
            pBox = NewTableBox( pCnts->GetStartNode(), pUpper );
            pCnts->SetTableBox( pBox );
        }
        else
        {
            pCnts->GetTable()->InheritVertBorders( this, nLeftCol,
                                                   nRightCol-nLeftCol );
            // und die ist eine Tabelle: dann bauen wir eine neue
            // Box und fuegen die Zeilen der Tabelle in die Zeilen
            // der Box ein
            pBox = new SwTableBox( pBoxFormat, 0, pUpper );
            sal_uInt16 nAbs, nRel;
            pLayoutInfo->GetAvail( nLeftCol, nColSpan, nAbs, nRel );
            sal_uInt16 nLSpace = pLayoutInfo->GetLeftCellSpace( nLeftCol, nColSpan );
            sal_uInt16 nRSpace = pLayoutInfo->GetRightCellSpace( nLeftCol, nColSpan );
            sal_uInt16 nInhSpace = pLayoutInfo->GetInhCellSpace( nLeftCol, nColSpan );
            pCnts->GetTable()->MakeTable( pBox, nAbs, nRel, nLSpace, nRSpace,
                                          nInhSpace );
        }
    }
    else
    {
        // mehrere Inhalts Sections: dann brauchen wir eine Box mit Zeilen
        pBox = new SwTableBox( pBoxFormat, 0, pUpper );
        SwTableLines& rLines = pBox->GetTabLines();
        bool bFirstPara = true;

        while( pCnts )
        {
            if( pCnts->GetStartNode() )
            {
                // normale Absaetze werden zu einer Box in einer Zeile
                SwTableLine *pLine =
                    new SwTableLine( pLineFrameFormatNoHeight ? pLineFrameFormatNoHeight
                                                         : pLineFormat, 0, pBox );
                if( !pLineFrameFormatNoHeight )
                {
                    // Wenn es noch kein Line-Format ohne Hoehe gibt, koennen
                    // wir uns dieses her als soleches merken
                    pLineFrameFormatNoHeight = static_cast<SwTableLineFormat*>(pLine->ClaimFrameFormat());

                    ResetLineFrameFormatAttrs( pLineFrameFormatNoHeight );
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
                // Tabellen werden direkt eingetragen
                sal_uInt16 nAbs, nRel;
                pLayoutInfo->GetAvail( nLeftCol, nColSpan, nAbs, nRel );
                sal_uInt16 nLSpace = pLayoutInfo->GetLeftCellSpace( nLeftCol,
                                                                nColSpan );
                sal_uInt16 nRSpace = pLayoutInfo->GetRightCellSpace( nLeftCol,
                                                                 nColSpan );
                sal_uInt16 nInhSpace = pLayoutInfo->GetInhCellSpace( nLeftCol, nColSpan );
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
                                sal_uInt16 nRowSpan, sal_uInt16 /*nColSpan*/,
                                bool bFirstPara, bool bLastPara )
{
    OSL_ENSURE( nRows>0 && nCols>0 && nCurRow==nRows,
            "Wurde CloseTable nicht aufgerufen?" );

    // Die Child-Tabelle muss einen Rahmen bekommen, wenn die umgebende
    // Zelle einen Rand an der betreffenden Seite besitzt.
    // Der obere bzw. untere Rand wird nur gesetzt, wenn die Tabelle
    // ale erster bzw. letzter Absatz in der Zelle vorkommt. Ansonsten
    // Fuer den linken/rechten Rand kann noch nicht entschieden werden,
    // ob eine Umrandung der Tabelle noetig/moeglich ist, weil das davon
    // abhaengt, ob "Filler"-Zellen eingefuegt werden. Hier werden deshalb
    // erstmal nur Informationen gesammelt

    if( 0==nRow && pParent->bTopBorder && bFirstPara )
    {
        bTopBorder = true;
        bFillerTopBorder = true; // auch Filler bekommt eine Umrandung
        aTopBorderLine = pParent->aTopBorderLine;
    }
    if ((*pParent->m_pRows)[nRow+nRowSpan-1]->bBottomBorder && bLastPara)
    {
        (*m_pRows)[nRows-1]->bBottomBorder = true;
        bFillerBottomBorder = true; // auch Filler bekommt eine Umrandung
        aBottomBorderLine =
            nRow+nRowSpan==pParent->nRows ? pParent->aBottomBorderLine
                                          : pParent->aBorderLine;
    }

    // Die Child Tabelle darf keinen oberen oder linken Rahmen bekommen,
    // wenn der bereits durch die umgebende Tabelle gesetzt ist.
    // Sie darf jedoch immer einen oberen Rand bekommen, wenn die Tabelle
    // nicht der erste Absatz in der Zelle ist.
    bTopAlwd = ( !bFirstPara || (pParent->bTopAlwd &&
                 (0==nRow || !((*pParent->m_pRows)[nRow-1])->bBottomBorder)) );

    // die Child-Tabelle muss die Farbe der Zelle erben, in der sie
    // vorkommt, wenn sie keine eigene besitzt
    const SvxBrushItem *pInhBG = pParent->GetCell(nRow,nCol)->GetBGBrush();
    if( !pInhBG && pParent != pTopTable &&
        pParent->GetCell(nRow,nCol)->GetRowSpan() == pParent->nRows )
    {
        // die ganze umgebende Tabelle ist eine Tabelle in der Tabelle
        // und besteht nur aus einer Line, die bei der GC (zu Recht)
        // wegoptimiert wird. Deshalb muss der Hintergrund der Line in
        // diese Tabelle uebernommen werden.
        pInhBG = (*pParent->m_pRows)[nRow]->GetBGBrush();
        if( !pInhBG )
            pInhBG = pParent->GetBGBrush();
        if( !pInhBG )
            pInhBG = pParent->GetInhBGBrush();
    }
    if( pInhBG )
        pInhBGBrush = new SvxBrushItem( *pInhBG );
}

void HTMLTable::InheritVertBorders( const HTMLTable *pParent,
                                 sal_uInt16 nCol, sal_uInt16 nColSpan )
{
    sal_uInt16 nInhLeftBorderWidth = 0;
    sal_uInt16 nInhRightBorderWidth = 0;

    if( nCol+nColSpan==pParent->nCols && pParent->bRightBorder )
    {
        bInhRightBorder = true; // erstmal nur merken
        aInhRightBorderLine = pParent->aRightBorderLine;
        nInhRightBorderWidth =
            GetBorderWidth( aInhRightBorderLine, true ) + MIN_BORDER_DIST;
    }

    if (((*pParent->m_pColumns)[nCol])->bLeftBorder)
    {
        bInhLeftBorder = true;  // erstmal nur merken
        aInhLeftBorderLine = 0==nCol ? pParent->aLeftBorderLine
                                     : pParent->aBorderLine;
        nInhLeftBorderWidth =
            GetBorderWidth( aInhLeftBorderLine, true ) + MIN_BORDER_DIST;
    }

    if( !bInhLeftBorder && (bFillerTopBorder || bFillerBottomBorder) )
        nInhLeftBorderWidth = 2 * MIN_BORDER_DIST;
    if( !bInhRightBorder && (bFillerTopBorder || bFillerBottomBorder) )
        nInhRightBorderWidth = 2 * MIN_BORDER_DIST;
    pLayoutInfo->SetInhBorderWidths( nInhLeftBorderWidth,
                                     nInhRightBorderWidth );

    bRightAlwd = ( pParent->bRightAlwd &&
                  (nCol+nColSpan==pParent->nCols ||
                   !((*pParent->m_pColumns)[nCol+nColSpan])->bLeftBorder) );
}

void HTMLTable::SetBorders()
{
    sal_uInt16 i;
    for( i=1; i<nCols; i++ )
        if( HTML_TR_ALL==eRules || HTML_TR_COLS==eRules ||
            ((HTML_TR_ROWS==eRules || HTML_TR_GROUPS==eRules) &&
             ((*m_pColumns)[i-1])->IsEndOfGroup()))
        {
            ((*m_pColumns)[i])->bLeftBorder = true;
        }

    for( i=0; i<nRows-1; i++ )
        if( HTML_TR_ALL==eRules || HTML_TR_ROWS==eRules ||
            ((HTML_TR_COLS==eRules || HTML_TR_GROUPS==eRules) &&
             (*m_pRows)[i]->IsEndOfGroup()))
        {
            (*m_pRows)[i]->bBottomBorder = true;
        }

    if( bTopAlwd && (HTML_TF_ABOVE==eFrame || HTML_TF_HSIDES==eFrame ||
                     HTML_TF_BOX==eFrame) )
        bTopBorder = true;
    if( HTML_TF_BELOW==eFrame || HTML_TF_HSIDES==eFrame ||
        HTML_TF_BOX==eFrame )
    {
        (*m_pRows)[nRows-1]->bBottomBorder = true;
    }
    if( (HTML_TF_RHS==eFrame || HTML_TF_VSIDES==eFrame ||
                      HTML_TF_BOX==eFrame) )
        bRightBorder = true;
    if( HTML_TF_LHS==eFrame || HTML_TF_VSIDES==eFrame || HTML_TF_BOX==eFrame )
    {
        ((*m_pColumns)[0])->bLeftBorder = true;
    }

    for( i=0; i<nRows; i++ )
    {
        HTMLTableRow *const pRow = (*m_pRows)[i].get();
        for( sal_uInt16 j=0; j<nCols; j++ )
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
                                                pCell->GetColSpan(),
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

    bBordersSet = true;
}

sal_uInt16 HTMLTable::GetBorderWidth( const SvxBorderLine& rBLine,
                                  bool bWithDistance ) const
{
    sal_uInt16 nBorderWidth = rBLine.GetWidth();
    if( bWithDistance )
    {
        if( nCellPadding )
            nBorderWidth = nBorderWidth + nCellPadding;
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
    SvxAdjust eAdjust = (nCurCol<nCols ? ((*m_pColumns)[nCurCol])->GetAdjust()
                                       : SVX_ADJUST_END );
    if( SVX_ADJUST_END==eAdjust )
        eAdjust = (*m_pRows)[nCurRow]->GetAdjust();

    return eAdjust;
}

sal_Int16 HTMLTable::GetInheritedVertOri() const
{
    // text::VertOrientation::TOP ist der default!
    sal_Int16 eVOri = (*m_pRows)[nCurRow]->GetVertOri();
    if( text::VertOrientation::TOP==eVOri && nCurCol<nCols )
        eVOri = ((*m_pColumns)[nCurCol])->GetVertOri();
    if( text::VertOrientation::TOP==eVOri )
        eVOri = eVertOri;

    OSL_ENSURE( eVertOri != text::VertOrientation::TOP, "text::VertOrientation::TOP ist nicht erlaubt!" );
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
    if( !nRowSpan || (sal_uInt32)nCurRow + nRowSpan > USHRT_MAX )
        nRowSpan = 1;

    if( !nColSpan || (sal_uInt32)nCurCol + nColSpan > USHRT_MAX )
        nColSpan = 1;

    sal_uInt16 nColsReq = nCurCol + nColSpan;       // benoetigte Spalten
    sal_uInt16 nRowsReq = nCurRow + nRowSpan;       // benoetigte Zeilen
    sal_uInt16 i, j;

    // falls wir mehr Spalten benoetigen als wir zur Zeit haben,
    // muessen wir in allen Zeilen noch Zellen hinzufuegen
    if( nCols < nColsReq )
    {
        for( i=nCols; i<nColsReq; i++ )
            m_pColumns->push_back(o3tl::make_unique<HTMLTableColumn>());
        for( i=0; i<nRows; i++ )
            (*m_pRows)[i]->Expand( nColsReq, i<nCurRow );
        nCols = nColsReq;
        OSL_ENSURE(m_pColumns->size() == nCols,
                "wrong number of columns after expanding");
    }
    if( nColsReq > nFilledCols )
        nFilledCols = nColsReq;

    // falls wir mehr Zeilen benoetigen als wir zur Zeit haben,
    // muessen wir noch neue Zeilen hinzufuegen
    if( nRows < nRowsReq )
    {
        for( i=nRows; i<nRowsReq; i++ )
            m_pRows->push_back(o3tl::make_unique<HTMLTableRow>(nCols));
        nRows = nRowsReq;
        OSL_ENSURE(nRows == m_pRows->size(), "wrong number of rows in Insert");
    }

    // Testen, ob eine Ueberschneidung vorliegt und diese
    // gegebenfalls beseitigen
    sal_uInt16 nSpanedCols = 0;
    if( nCurRow>0 )
    {
        HTMLTableRow *const pCurRow = (*m_pRows)[nCurRow].get();
        for( i=nCurCol; i<nColsReq; i++ )
        {
            HTMLTableCell *pCell = pCurRow->GetCell(i);
            if( pCell->GetContents() )
            {
                // Der Inhalt reicht von einer weiter oben stehenden Zelle
                // hier herein. Inhalt und Farbe der Zelle sind deshalb in
                // jedem Fall noch dort verankert und koennen deshalb
                // ueberschrieben werden bzw. von ProtectRowSpan geloescht
                // (Inhalt) oder kopiert (Farbe) werden.
                nSpanedCols = i + pCell->GetColSpan();
                FixRowSpan( nCurRow-1, i, pCell->GetContents() );
                if( pCell->GetRowSpan() > nRowSpan )
                    ProtectRowSpan( nRowsReq, i,
                                    pCell->GetRowSpan()-nRowSpan );
            }
        }
        for( i=nColsReq; i<nSpanedCols; i++ )
        {
            // Auch diese Inhalte sind in jedem Fall nich in der Zeile
            // darueber verankert.
            HTMLTableCell *pCell = pCurRow->GetCell(i);
            FixRowSpan( nCurRow-1, i, pCell->GetContents() );
            ProtectRowSpan( nCurRow, i, pCell->GetRowSpan() );
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
                    ->PixelToLogic( aTwipSz, MapMode( MAP_TWIP ) );
    }

    // die Breite nur in die erste Zelle setzen!
    if( nCellWidth )
    {
        sal_uInt16 nTmp = bRelWidth ? nCellWidth : (sal_uInt16)aTwipSz.Width();
        GetCell( nCurRow, nCurCol )->SetWidth( nTmp, bRelWidth );
    }

    // Ausserdem noch die Hoehe merken
    if( nCellHeight && 1==nRowSpan )
    {
        if( nCellHeight < MINLAY )
            nCellHeight = MINLAY;
        (*m_pRows)[nCurRow]->SetHeight(static_cast<sal_uInt16>(aTwipSz.Height()));
    }

    // den Spaltenzaehler hinter die neuen Zellen setzen
    nCurCol = nColsReq;
    if( nSpanedCols > nCurCol )
        nCurCol = nSpanedCols;

    // und die naechste freie Zelle suchen
    while( nCurCol<nCols && GetCell(nCurRow,nCurCol)->IsUsed() )
        nCurCol++;
}

inline void HTMLTable::CloseSection( bool bHead )
{
    // die vorhergende Section beenden, falls es schon eine Zeile gibt
    OSL_ENSURE( nCurRow<=nRows, "ungeultige aktuelle Zeile" );
    if( nCurRow>0 && nCurRow<=nRows )
        (*m_pRows)[nCurRow-1]->SetEndOfGroup();
    if( bHead )
        nHeadlineRepeat = nCurRow;
}

void HTMLTable::OpenRow( SvxAdjust eAdjust, sal_Int16 eVertOrient,
                         SvxBrushItem *pBGBrushItem )
{
    sal_uInt16 nRowsReq = nCurRow+1;    // Anzahl benoetigter Zeilen;

    // die naechste Zeile anlegen, falls sie nicht schon da ist
    if( nRows<nRowsReq )
    {
        for( sal_uInt16 i=nRows; i<nRowsReq; i++ )
            m_pRows->push_back(o3tl::make_unique<HTMLTableRow>(nCols));
        nRows = nRowsReq;
        OSL_ENSURE( nRows == m_pRows->size(),
                "Zeilenzahl in OpenRow stimmt nicht" );
    }

    HTMLTableRow *const pCurRow = (*m_pRows)[nCurRow].get();
    pCurRow->SetAdjust( eAdjust );
    pCurRow->SetVertOri( eVertOrient );
    if( pBGBrushItem )
        (*m_pRows)[nCurRow]->SetBGBrush( pBGBrushItem );

    // den Spaltenzaehler wieder an den Anfang setzen
    nCurCol=0;

    // und die naechste freie Zelle suchen
    while( nCurCol<nCols && GetCell(nCurRow,nCurCol)->IsUsed() )
        nCurCol++;
}

void HTMLTable::CloseRow( bool bEmpty )
{
    OSL_ENSURE( nCurRow<nRows, "aktulle Zeile hinter dem Tabellenende" );

    // leere Zellen bekommen einfach einen etwas dickeren unteren Rand!
    if( bEmpty )
    {
        if( nCurRow > 0 )
            (*m_pRows)[nCurRow-1]->IncEmptyRows();
        return;
    }

    HTMLTableRow *const pRow = (*m_pRows)[nCurRow].get();

    // den COLSPAN aller leeren Zellen am Zeilenende so anpassen, dass
    // eine Zelle daraus wird. Das kann man hier machen (und auf keinen
    // Fall frueher), weill jetzt keine Zellen mehr in die Zeile eingefuegt
    // werden.
    sal_uInt16 i=nCols;
    while( i )
    {
        HTMLTableCell *pCell = pRow->GetCell(--i);
        if( !pCell->GetContents() )
        {
            sal_uInt16 nColSpan = nCols-i;
            if( nColSpan > 1 )
                pCell->SetColSpan( nColSpan );
        }
        else
            break;
    }

    nCurRow++;
}

inline void HTMLTable::CloseColGroup( sal_uInt16 nSpan, sal_uInt16 _nWidth,
                                      bool bRelWidth, SvxAdjust eAdjust,
                                      sal_Int16 eVertOrient )
{
    if( nSpan )
        InsertCol( nSpan, _nWidth, bRelWidth, eAdjust, eVertOrient );

    OSL_ENSURE( nCurCol<=nCols, "ungueltige Spalte" );
    if( nCurCol>0 && nCurCol<=nCols )
        ((*m_pColumns)[nCurCol-1])->SetEndOfGroup();
}

void HTMLTable::InsertCol( sal_uInt16 nSpan, sal_uInt16 nColWidth, bool bRelWidth,
                           SvxAdjust eAdjust, sal_Int16 eVertOrient )
{
    // #i35143# - no columns, if rows already exist.
    if ( nRows > 0 )
        return;

    sal_uInt16 i;

    if( !nSpan )
        nSpan = 1;

    sal_uInt16 nColsReq = nCurCol + nSpan;      // benoetigte Spalten

    if( nCols < nColsReq )
    {
        for( i=nCols; i<nColsReq; i++ )
            m_pColumns->push_back(o3tl::make_unique<HTMLTableColumn>());
        nCols = nColsReq;
    }

    Size aTwipSz( bRelWidth ? 0 : nColWidth, 0 );
    if( aTwipSz.Width() && Application::GetDefaultDevice() )
    {
        aTwipSz = Application::GetDefaultDevice()
                    ->PixelToLogic( aTwipSz, MapMode( MAP_TWIP ) );
    }

    for( i=nCurCol; i<nColsReq; i++ )
    {
        HTMLTableColumn *const pCol = (*m_pColumns)[i].get();
        sal_uInt16 nTmp = bRelWidth ? nColWidth : (sal_uInt16)aTwipSz.Width();
        pCol->SetWidth( nTmp, bRelWidth );
        pCol->SetAdjust( eAdjust );
        pCol->SetVertOri( eVertOrient );
    }

    bColSpec = true;

    nCurCol = nColsReq;
}

void HTMLTable::CloseTable()
{
    sal_uInt16 i;

    // Die Anzahl der Tabellenzeilen richtet sich nur nach den
    // <TR>-Elementen (d.h. nach nCurRow). Durch ROWSPAN aufgespannte
    // Zeilen hinter Zeile nCurRow muessen wir deshalb loeschen
    // und vor allem aber den ROWSPAN in den darueberliegenden Zeilen
    // anpassen.
    if( nRows>nCurRow )
    {
        HTMLTableRow *const pPrevRow = (*m_pRows)[nCurRow-1].get();
        HTMLTableCell *pCell;
        for( i=0; i<nCols; i++ )
            if( ( (pCell=(pPrevRow->GetCell(i))), (pCell->GetRowSpan()) > 1 ) )
            {
                FixRowSpan( nCurRow-1, i, pCell->GetContents() );
                ProtectRowSpan(nCurRow, i, (*m_pRows)[nCurRow]->GetCell(i)->GetRowSpan());
            }
        for( i=nRows-1; i>=nCurRow; i-- )
            m_pRows->erase(m_pRows->begin() + i);
        nRows = nCurRow;
    }

    // falls die Tabelle keine Spalte hat, muessen wir eine hinzufuegen
    if( 0==nCols )
    {
        m_pColumns->push_back(o3tl::make_unique<HTMLTableColumn>());
        for( i=0; i<nRows; i++ )
            (*m_pRows)[i]->Expand(1);
        nCols = 1;
        nFilledCols = 1;
    }

    // falls die Tabelle keine Zeile hat, muessen wir eine hinzufuegen
    if( 0==nRows )
    {
        m_pRows->push_back(o3tl::make_unique<HTMLTableRow>(nCols));
        nRows = 1;
        nCurRow = 1;
    }

    if( nFilledCols < nCols )
    {
        m_pColumns->erase(m_pColumns->begin() + nFilledCols, m_pColumns->begin() + nCols);
        for( i=0; i<nRows; i++ )
            (*m_pRows)[i]->Shrink( nFilledCols );
        nCols = nFilledCols;
    }
}

void HTMLTable::_MakeTable( SwTableBox *pBox )
{
    SwTableLines& rLines = (pBox ? pBox->GetTabLines()
                                 : const_cast<SwTable *>(pSwTable)->GetTabLines() );

    // jetzt geht's richtig los ...

    for( sal_uInt16 i=0; i<nRows; i++ )
    {
        SwTableLine *pLine = MakeTableLine( pBox, i, 0, i+1, nCols );
        if( pBox || i > 0 )
            rLines.push_back( pLine );
    }
}

/* Wie werden Tabellen ausgerichtet?

erste Zeile: ohne Absatz-Einzuege
zweite Zeile: mit Absatz-Einzuegen

ALIGN=          LEFT            RIGHT           CENTER          -
-------------------------------------------------------------------------
xxx bei Tabellen mit WIDTH=nn% ist die Prozent-Angabe von Bedeutung:
xxx nn = 100        text::HoriOrientation::FULL       text::HoriOrientation::FULL       text::HoriOrientation::FULL       text::HoriOrientation::FULL %
xxx             text::HoriOrientation::NONE       text::HoriOrientation::NONE       text::HoriOrientation::NONE %     text::HoriOrientation::NONE %
xxx nn < 100        Rahmen F        Rahmen F        text::HoriOrientation::CENTER %   text::HoriOrientation::LEFT %
xxx             Rahmen F        Rahmen F        text::HoriOrientation::CENTER %   text::HoriOrientation::NONE %

bei Tabellen mit WIDTH=nn% ist die Prozent-Angabe von Bedeutung:
nn = 100        text::HoriOrientation::LEFT       text::HoriOrientation::RIGHT      text::HoriOrientation::CENTER %   text::HoriOrientation::LEFT %
                text::HoriOrientation::LEFT_AND   text::HoriOrientation::RIGHT      text::HoriOrientation::CENTER %   text::HoriOrientation::LEFT_AND %
nn < 100        Rahmen F        Rahmen F        text::HoriOrientation::CENTER %   text::HoriOrientation::LEFT %
                Rahmen F        Rahmen F        text::HoriOrientation::CENTER %   text::HoriOrientation::NONE %

sonst die berechnete Breite w
w = avail*      text::HoriOrientation::LEFT       text::HoriOrientation::RIGHT      text::HoriOrientation::CENTER     text::HoriOrientation::LEFT
                HORI_LEDT_AND   text::HoriOrientation::RIGHT      text::HoriOrientation::CENTER     text::HoriOrientation::LEFT_AND
w < avail       Rahmen L        Rahmen L        text::HoriOrientation::CENTER     text::HoriOrientation::LEFT
                Rahmen L        Rahmen L        text::HoriOrientation::CENTER     text::HoriOrientation::NONE

xxx *) wenn fuer die Tabelle keine Groesse angegeben wurde, wird immer
xxx   text::HoriOrientation::FULL genommen

*/

void HTMLTable::MakeTable( SwTableBox *pBox, sal_uInt16 nAbsAvail,
                           sal_uInt16 nRelAvail, sal_uInt16 nAbsLeftSpace,
                           sal_uInt16 nAbsRightSpace, sal_uInt16 nInhAbsSpace )
{
    OSL_ENSURE( nRows>0 && nCols>0 && nCurRow==nRows,
            "Wurde CloseTable nicht aufgerufen?" );

    OSL_ENSURE( (pLayoutInfo==nullptr) == (this==pTopTable),
            "Top-Tabelle hat keine Layout-Info oder umgekehrt" );

    if( this==pTopTable )
    {
        // Umrandung der Tabelle und aller in ihr enthaltenen berechnen
        SetBorders();

        // Schritt 1: Die benoetigten Layout-Strukturen werden angelegt
        // (inklusive Tabellen in Tabellen).
        CreateLayoutInfo();

        // Schritt 2: Die minimalen und maximalen Spaltenbreiten werden
        // berechnet (inklusive Tabellen in Tabellen). Da wir noch keine
        // Boxen haben, arabeiten wir noch auf den Start-Nodes.
        pLayoutInfo->AutoLayoutPass1();
    }

    // Schritt 3: Die tatsaechlichen Spaltenbreiten dieser Tabelle werden
    // berechnet (nicht von Tabellen in Tabellen). Dies muss jetzt schon
    // sein, damit wir entscheiden koennen ob Filler-Zellen benoetigt werden
    // oder nicht (deshalb war auch Pass1 schon noetig).
    pLayoutInfo->AutoLayoutPass2( nAbsAvail, nRelAvail, nAbsLeftSpace,
                                  nAbsRightSpace, nInhAbsSpace );

    if( this!=pTopTable )
    {
        // die linke und rechte Umrandung der Tabelle kann jetzt entgueltig
        // festgelegt werden
        if( pLayoutInfo->GetRelRightFill() == 0 )
        {
            if( !bRightBorder )
            {
                // linke Umrandung von auesserer Tabelle uebernehmen
                if( bInhRightBorder )
                {
                    bRightBorder = true;
                    aRightBorderLine = aInhRightBorderLine;
                }
            }
            else
            {
                // Umrandung nur setzen, wenn es erlaubt ist
                bRightBorder = bRightAlwd;
            }
        }

        if( pLayoutInfo->GetRelLeftFill() == 0 &&
            !((*m_pColumns)[0])->bLeftBorder &&
            bInhLeftBorder )
        {
            // ggf. rechte Umrandung von auesserer Tabelle uebernehmen
            ((*m_pColumns)[0])->bLeftBorder = true;
            aLeftBorderLine = aInhLeftBorderLine;
        }
    }

    // Fuer die Top-Table muss die Ausrichtung gesetzt werden
    if( this==pTopTable )
    {
        sal_Int16 eHoriOri;
        if( bForceFrame )
        {
            // Die Tabelle soll in einen Rahmen und ist auch schmaler
            // als der verfuegbare Platz und nicht 100% breit.
            // Dann kommt sie in einen Rahmen
            eHoriOri = bPrcWidth ? text::HoriOrientation::FULL : text::HoriOrientation::LEFT;
        }
        else switch( eTableAdjust )
        {
            // Die Tabelle passt entweder auf die Seite, soll aber in keinen
            // Rahmen oder sie ist Breiter als die Seite und soll deshalb
            // in keinen Rahmen

        case SVX_ADJUST_RIGHT:
            // in rechtsbuendigen Tabellen kann nicht auf den rechten
            // Rand Ruecksicht genommen werden
            eHoriOri = text::HoriOrientation::RIGHT;
            break;
        case SVX_ADJUST_CENTER:
            // zentrierte Tabellen nehmen keine Ruecksicht auf Raender!
            eHoriOri = text::HoriOrientation::CENTER;
            break;
        case SVX_ADJUST_LEFT:
        default:
            // linksbuendige Tabellen nehmen nur auf den linken Rand
            // Ruecksicht
            eHoriOri = nLeftMargin ? text::HoriOrientation::LEFT_AND_WIDTH : text::HoriOrientation::LEFT;
            break;
        }

        // das Tabellenform holen und anpassen
        SwFrameFormat *pFrameFormat = pSwTable->GetFrameFormat();
        pFrameFormat->SetFormatAttr( SwFormatHoriOrient(0,eHoriOri) );
        if( text::HoriOrientation::LEFT_AND_WIDTH==eHoriOri )
        {
            OSL_ENSURE( nLeftMargin || nRightMargin,
                    "Da gibt's wohl noch Reste von relativen Breiten" );

            // The right margin will be ignored anyway.
            SvxLRSpaceItem aLRItem( pSwTable->GetFrameFormat()->GetLRSpace() );
            aLRItem.SetLeft( nLeftMargin );
            aLRItem.SetRight( nRightMargin );
            pFrameFormat->SetFormatAttr( aLRItem );
        }

        if( bPrcWidth && text::HoriOrientation::FULL!=eHoriOri )
        {
            pFrameFormat->LockModify();
            SwFormatFrameSize aFrameSize( pFrameFormat->GetFrameSize() );
            aFrameSize.SetWidthPercent( (sal_uInt8)nWidth );
            pFrameFormat->SetFormatAttr( aFrameSize );
            pFrameFormat->UnlockModify();
        }
    }

    // die Default Line- und Box-Formate holen
    if( this==pTopTable )
    {
        // die erste Box merken und aus der ersten Zeile ausketten
        SwTableLine *pLine1 = (pSwTable->GetTabLines())[0];
        pBox1 = (pLine1->GetTabBoxes())[0];
        pLine1->GetTabBoxes().erase(pLine1->GetTabBoxes().begin());

        pLineFormat = static_cast<SwTableLineFormat*>(pLine1->GetFrameFormat());
        pBoxFormat = static_cast<SwTableBoxFormat*>(pBox1->GetFrameFormat());
    }
    else
    {
        pLineFormat = pTopTable->pLineFormat;
        pBoxFormat = pTopTable->pBoxFormat;
    }

    // ggf. muessen fuer Tabellen in Tabellen "Filler"-Zellen eingefuegt
    // werden
    if( this != pTopTable &&
        ( pLayoutInfo->GetRelLeftFill() > 0  ||
          pLayoutInfo->GetRelRightFill() > 0 ) )
    {
        OSL_ENSURE( pBox, "kein TableBox fuer Tabelle in Tabelle" );

        SwTableLines& rLines = pBox->GetTabLines();

        // dazu brauchen wir erstmal ein eine neue Table-Line in der Box
        SwTableLine *pLine =
            new SwTableLine( pLineFrameFormatNoHeight ? pLineFrameFormatNoHeight
                                                 : pLineFormat, 0, pBox );
        rLines.push_back( pLine );

        // Sicherstellen, dass wie ein Format ohne Hoehe erwischt haben
        if( !pLineFrameFormatNoHeight )
        {
            // sonst muessen wir die Hoehe aus dem Attribut entfernen
            // und koennen uns das Format merken
            pLineFrameFormatNoHeight = static_cast<SwTableLineFormat*>(pLine->ClaimFrameFormat());

            ResetLineFrameFormatAttrs( pLineFrameFormatNoHeight );
        }

        SwTableBoxes& rBoxes = pLine->GetTabBoxes();
        SwTableBox *pNewBox;

        // ggf. links eine Zelle einfuegen
        if( pLayoutInfo->GetRelLeftFill() > 0 )
        {
            // pPrevStNd ist der Vorgaenger-Start-Node der Tabelle. Den
            // "Filler"-Node fuegen wir einfach dahinter ein ...
            pPrevStNd = pParser->InsertTableSection( pPrevStNd );

            pNewBox = NewTableBox( pPrevStNd, pLine );
            rBoxes.push_back( pNewBox );
            FixFillerFrameFormat( pNewBox, false );
            pLayoutInfo->SetLeftFillerBox( pNewBox );
        }

        // jetzt die Tabelle bearbeiten
        pNewBox = new SwTableBox( pBoxFormat, 0, pLine );
        rBoxes.push_back( pNewBox );

        SwFrameFormat *pFrameFormat = pNewBox->ClaimFrameFormat();
        pFrameFormat->ResetFormatAttr( RES_BOX );
        pFrameFormat->ResetFormatAttr( RES_BACKGROUND );
        pFrameFormat->ResetFormatAttr( RES_VERT_ORIENT );
        pFrameFormat->ResetFormatAttr( RES_BOXATR_FORMAT );

        _MakeTable( pNewBox );

        // und noch ggf. rechts eine Zelle einfuegen
        if( pLayoutInfo->GetRelRightFill() > 0 )
        {
            const SwStartNode *pStNd =
                GetPrevBoxStartNode( USHRT_MAX, USHRT_MAX );
            pStNd = pParser->InsertTableSection( pStNd );

            pNewBox = NewTableBox( pStNd, pLine );
            rBoxes.push_back( pNewBox );

            FixFillerFrameFormat( pNewBox, true );
            pLayoutInfo->SetRightFillerBox( pNewBox );
        }
    }
    else
    {
        _MakeTable( pBox );
    }

    // zum Schluss fuehren wir noch eine Garbage-Collection fuer die
    // Top-Level-Tabelle durch
    if( this==pTopTable )
    {
        if( 1==nRows && nHeight && 1==pSwTable->GetTabLines().size() )
        {
            // Hoehe einer einzeiligen Tabelle als Mindesthoehe der
            // Zeile setzen. (War mal fixe Hoehe, aber das gibt manchmal
            // Probleme (fix #34972#) und ist auch nicht Netscape 4.0
            // konform
            nHeight = SwHTMLParser::ToTwips( nHeight );
            if( nHeight < MINLAY )
                nHeight = MINLAY;

            (pSwTable->GetTabLines())[0]->ClaimFrameFormat();
            (pSwTable->GetTabLines())[0]->GetFrameFormat()
                ->SetFormatAttr( SwFormatFrameSize( ATT_MIN_SIZE, 0, nHeight ) );
        }

        if( GetBGBrush() )
            pSwTable->GetFrameFormat()->SetFormatAttr( *GetBGBrush() );

        const_cast<SwTable *>(pSwTable)->SetRowsToRepeat( static_cast< sal_uInt16 >(nHeadlineRepeat) );
        const_cast<SwTable *>(pSwTable)->GCLines();

        bool bIsInFlyFrame = pContext && pContext->GetFrameFormat();
        if( bIsInFlyFrame && !nWidth )
        {
            SvxAdjust eAdjust = GetTableAdjust(false);
            if (eAdjust != SVX_ADJUST_LEFT &&
                eAdjust != SVX_ADJUST_RIGHT)
            {
                // Wenn eine Tabelle ohne Breitenangabe nicht links oder
                // rechts umflossen werden soll, dann stacken wir sie
                // in einem Rahmen mit 100%-Breite, damit ihre Groesse
                // angepasst wird. Der Rahmen darf nicht angepasst werden.
                OSL_ENSURE( HasToFly(), "Warum ist die Tabelle in einem Rahmen?" );
                sal_uInt32 nMin = pLayoutInfo->GetMin();
                if( nMin > USHRT_MAX )
                    nMin = USHRT_MAX;
                SwFormatFrameSize aFlyFrameSize( ATT_VAR_SIZE, (SwTwips)nMin, MINLAY );
                aFlyFrameSize.SetWidthPercent( 100 );
                pContext->GetFrameFormat()->SetFormatAttr( aFlyFrameSize );
                bIsInFlyFrame = false;
            }
            else
            {
                // Links und rechts ausgerichtete Tabellen ohne Breite
                // duerfen leider nicht in der Breite angepasst werden, denn
                // sie wuerden nur schrumpfen aber nie wachsen.
                pLayoutInfo->SetMustNotRecalc( true );
                if( pContext->GetFrameFormat()->GetAnchor().GetContentAnchor()
                    ->nNode.GetNode().FindTableNode() )
                {
                    sal_uInt32 nMax = pLayoutInfo->GetMax();
                    if( nMax > USHRT_MAX )
                        nMax = USHRT_MAX;
                    SwFormatFrameSize aFlyFrameSize( ATT_VAR_SIZE, (SwTwips)nMax, MINLAY );
                    pContext->GetFrameFormat()->SetFormatAttr( aFlyFrameSize );
                    bIsInFlyFrame = false;
                }
                else
                {
                    pLayoutInfo->SetMustNotResize( true );
                }
            }
        }
        pLayoutInfo->SetMayBeInFlyFrame( bIsInFlyFrame );

        // Nur Tabellen mit relativer Breite oder ohne Breite muessen
        // angepasst werden.
        pLayoutInfo->SetMustResize( bPrcWidth || !nWidth );

        pLayoutInfo->SetWidths();

        const_cast<SwTable *>(pSwTable)->SetHTMLTableLayout( pLayoutInfo );

        if( pResizeDrawObjs )
        {
            sal_uInt16 nCount = pResizeDrawObjs->size();
            for( sal_uInt16 i=0; i<nCount; i++ )
            {
                SdrObject *pObj = (*pResizeDrawObjs)[i];
                sal_uInt16 nRow = (*pDrawObjPrcWidths)[3*i];
                sal_uInt16 nCol = (*pDrawObjPrcWidths)[3*i+1];
                sal_uInt8 nPrcWidth = (sal_uInt8)(*pDrawObjPrcWidths)[3*i+2];

                SwHTMLTableLayoutCell *pLayoutCell =
                    pLayoutInfo->GetCell( nRow, nCol );
                sal_uInt16 nColSpan = pLayoutCell->GetColSpan();

                sal_uInt16 nWidth2, nDummy;
                pLayoutInfo->GetAvail( nCol, nColSpan, nWidth2, nDummy );
                nWidth2 = nWidth2 - pLayoutInfo->GetLeftCellSpace( nCol, nColSpan );
                nWidth2 = nWidth2 - pLayoutInfo->GetRightCellSpace( nCol, nColSpan );
                nWidth2 = static_cast< sal_uInt16 >(((long)nWidth * nPrcWidth) / 100);

                SwHTMLParser::ResizeDrawObject( pObj, nWidth2 );
            }
        }
    }
}

void HTMLTable::SetTable( const SwStartNode *pStNd, _HTMLTableContext *pCntxt,
                          sal_uInt16 nLeft, sal_uInt16 nRight,
                          const SwTable *pSwTab, bool bFrcFrame )
{
    pPrevStNd = pStNd;
    pSwTable = pSwTab;
    pContext = pCntxt;

    nLeftMargin = nLeft;
    nRightMargin = nRight;

    bForceFrame = bFrcFrame;
}

void HTMLTable::RegisterDrawObject( SdrObject *pObj, sal_uInt8 nPrcWidth )
{
    if( !pResizeDrawObjs )
        pResizeDrawObjs = new SdrObjects;
    pResizeDrawObjs->push_back( pObj );

    if( !pDrawObjPrcWidths )
        pDrawObjPrcWidths = new std::vector<sal_uInt16>;
    pDrawObjPrcWidths->push_back( nCurRow );
    pDrawObjPrcWidths->push_back( nCurCol );
    pDrawObjPrcWidths->push_back( (sal_uInt16)nPrcWidth );
}

void HTMLTable::MakeParentContents()
{
    if( !GetContext() && !HasParentSection() )
    {
        SetParentContents(
            pParser->InsertTableContents( GetIsParentHeader() ) );

        SetHasParentSection( true );
    }
}

_HTMLTableContext::~_HTMLTableContext()
{
    delete pPos;
}

void _HTMLTableContext::SavePREListingXMP( SwHTMLParser& rParser )
{
    bRestartPRE = rParser.IsReadPRE();
    bRestartXMP = rParser.IsReadXMP();
    bRestartListing = rParser.IsReadListing();
    rParser.FinishPREListingXMP();
}

void _HTMLTableContext::RestorePREListingXMP( SwHTMLParser& rParser )
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
    OSL_ENSURE( pPrevStNd, "Start-Node ist NULL" );

    m_pCSS1Parser->SetTDTagStyles();
    SwTextFormatColl *pColl = m_pCSS1Parser->GetTextCollFromPool( RES_POOLCOLL_TABLE );

    const SwStartNode *pStNd;
    if (m_pTable->bFirstCell )
    {
        SwNode *const pNd = & m_pPam->GetPoint()->nNode.GetNode();
        pNd->GetTextNode()->ChgFormatColl( pColl );
        pStNd = pNd->FindTableBoxStartNode();
        m_pTable->bFirstCell = false;
    }
    else
    {
        const SwNode* pNd;
        if( pPrevStNd->IsTableNode() )
            pNd = pPrevStNd;
        else
            pNd = pPrevStNd->EndOfSectionNode();
        SwNodeIndex nIdx( *pNd, 1 );
        pStNd = m_pDoc->GetNodes().MakeTextSection( nIdx, SwTableBoxStartNode,
                                                  pColl );
        m_pTable->IncBoxCount();
    }

    //Added defaults to CJK and CTL
    SwContentNode *pCNd = m_pDoc->GetNodes()[pStNd->GetIndex()+1] ->GetContentNode();
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
    if (m_pTable->bFirstCell)
    {
        pNd->GetTextNode()->ChgFormatColl( pColl );
        m_pTable->bFirstCell = false;
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
        pStNd = m_pDoc->GetNodes().MakeTextSection( aIdx, SwTableBoxStartNode,
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
    rIdx = m_pDoc->GetNodes().GetEndOfExtras();
    SwStartNode *pStNd = m_pDoc->GetNodes().MakeTextSection( rIdx,
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
        // vorsicht, wenn Kommentare nicht uebrlesen werden!!!
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
                    // Bei Netscape entspricht ein Absatz-Ende zwei LFs
                    // (mit einem kommt man in die naechste Zeile, das
                    // zweite erzeugt eine Leerzeile) Diesen Abstand
                    // erreichen wie aber schon mit dem unteren
                    // Absatz-Abstand. Wenn nach den <BR> ein neuer
                    // Absatz aufgemacht wird, wird das Maximum des Abstands,
                    // der sich aus den BR und dem P ergibt genommen.
                    // Deshalb muessen wir 2 bzw. alle bei weniger
                    // als zweien loeschen
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
        SfxItemSet aItemSet( m_pDoc->GetAttrPool(), RES_BACKGROUND,
                                                  RES_BACKGROUND );
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

class _SectionSaveStruct : public SwPendingStackData
{
    sal_uInt16 m_nBaseFontStMinSave, m_nFontStMinSave, m_nFontStHeadStartSave;
    sal_uInt16 m_nDefListDeepSave;
    size_t m_nContextStMinSave;
    size_t m_nContextStAttrMinSave;

public:

    HTMLTable *m_pTable;

    explicit _SectionSaveStruct( SwHTMLParser& rParser );
    virtual ~_SectionSaveStruct();

#if OSL_DEBUG_LEVEL > 0
    size_t GetContextStAttrMin() const { return m_nContextStAttrMinSave; }
#endif
    void Restore( SwHTMLParser& rParser );
};

_SectionSaveStruct::_SectionSaveStruct( SwHTMLParser& rParser ) :
    m_nBaseFontStMinSave(0), m_nFontStMinSave(0), m_nFontStHeadStartSave(0),
    m_nDefListDeepSave(0), m_nContextStMinSave(0), m_nContextStAttrMinSave(0),
    m_pTable( nullptr )
{
    // Font-Stacks einfrieren
    m_nBaseFontStMinSave = rParser.m_nBaseFontStMin;
    rParser.m_nBaseFontStMin = rParser.m_aBaseFontStack.size();

    m_nFontStMinSave = rParser.m_nFontStMin;
    m_nFontStHeadStartSave = rParser.m_nFontStHeadStart;
    rParser.m_nFontStMin = rParser.m_aFontStack.size();

    // Kontext-Stack einfrieren
    m_nContextStMinSave = rParser.m_nContextStMin;
    m_nContextStAttrMinSave = rParser.m_nContextStAttrMin;
    rParser.m_nContextStMin = rParser.m_aContexts.size();
    rParser.m_nContextStAttrMin = rParser.m_nContextStMin;

    // und noch ein par Zaehler retten
    m_nDefListDeepSave = rParser.m_nDefListDeep;
    rParser.m_nDefListDeep = 0;
}

_SectionSaveStruct::~_SectionSaveStruct()
{}

void _SectionSaveStruct::Restore( SwHTMLParser& rParser )
{
    // Font-Stacks wieder auftauen
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

    // und noch ein par Zaehler rekonstruieren
    rParser.m_nDefListDeep = m_nDefListDeepSave;

    // und ein par Flags zuruecksetzen
    rParser.m_bNoParSpace = false;
    rParser.m_nOpenParaToken = 0;

    if( !rParser.m_aParaAttrs.empty() )
        rParser.m_aParaAttrs.clear();
}

class _CellSaveStruct : public _SectionSaveStruct
{
    OUString aStyle, aId, aClass, aLang, aDir;
    OUString aBGImage;
    Color aBGColor;
    std::shared_ptr<SvxBoxItem> m_pBoxItem;

    HTMLTableCnts* pCnts;           // Liste aller Inhalte
    HTMLTableCnts* pCurrCnts;   // der aktuelle Inhalt oder 0
    SwNodeIndex *pNoBreakEndParaIdx;// Absatz-Index eines </NOBR>

    double nValue;

    sal_uInt32 nNumFormat;

    sal_uInt16 nRowSpan, nColSpan, nWidth, nHeight;
    sal_Int32 nNoBreakEndContentPos;     // Zeichen-Index eines </NOBR>

    SvxAdjust eAdjust;
    sal_Int16 eVertOri;

    bool bHead : 1;
    bool bPrcWidth : 1;
    bool bHasNumFormat : 1;
    bool bHasValue : 1;
    bool bBGColor : 1;
    bool bNoWrap : 1;       // NOWRAP-Option
    bool bNoBreak : 1;      // NOBREAK-Tag

public:

    _CellSaveStruct( SwHTMLParser& rParser, HTMLTable *pCurTable, bool bHd,
                     bool bReadOpt );

    virtual ~_CellSaveStruct();

    void AddContents( HTMLTableCnts *pNewCnts );
    HTMLTableCnts *GetFirstContents() { return pCnts; }

    void ClearIsInSection() { pCurrCnts = nullptr; }
    bool IsInSection() const { return pCurrCnts!=nullptr; }

    void InsertCell( SwHTMLParser& rParser, HTMLTable *pCurTable );

    bool IsHeaderCell() const { return bHead; }

    void StartNoBreak( const SwPosition& rPos );
    void EndNoBreak( const SwPosition& rPos );
    void CheckNoBreak( const SwPosition& rPos, SwDoc *pDoc );
};

_CellSaveStruct::_CellSaveStruct( SwHTMLParser& rParser, HTMLTable *pCurTable,
                                  bool bHd, bool bReadOpt ) :
    _SectionSaveStruct( rParser ),
    pCnts( nullptr ),
    pCurrCnts( nullptr ),
    pNoBreakEndParaIdx( nullptr ),
    nValue( 0.0 ),
    nNumFormat( 0 ),
    nRowSpan( 1 ),
    nColSpan( 1 ),
    nWidth( 0 ),
    nHeight( 0 ),
    nNoBreakEndContentPos( 0 ),
    eAdjust( pCurTable->GetInheritedAdjust() ),
    eVertOri( pCurTable->GetInheritedVertOri() ),
    bHead( bHd ),
    bPrcWidth( false ),
    bHasNumFormat( false ),
    bHasValue( false ),
    bBGColor( false ),
    bNoWrap( false ),
    bNoBreak( false )
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
            case HTML_O_ID:
                aId = rOption.GetString();
                break;
            case HTML_O_COLSPAN:
                nColSpan = (sal_uInt16)rOption.GetNumber();
                break;
            case HTML_O_ROWSPAN:
                nRowSpan = (sal_uInt16)rOption.GetNumber();
                break;
            case HTML_O_ALIGN:
                eAdjust = (SvxAdjust)rOption.GetEnum(
                                        aHTMLPAlignTable, static_cast< sal_uInt16 >(eAdjust) );
                break;
            case HTML_O_VALIGN:
                eVertOri = rOption.GetEnum(
                                        aHTMLTableVAlignTable, eVertOri );
                break;
            case HTML_O_WIDTH:
                nWidth = (sal_uInt16)rOption.GetNumber();   // nur fuer Netscape
                bPrcWidth = (rOption.GetString().indexOf('%') != -1);
                if( bPrcWidth && nWidth>100 )
                    nWidth = 100;
                break;
            case HTML_O_HEIGHT:
                nHeight = (sal_uInt16)rOption.GetNumber();  // nur fuer Netscape
                if( rOption.GetString().indexOf('%') != -1)
                    nHeight = 0;    // keine %-Angaben beruecksichtigen
                break;
            case HTML_O_BGCOLOR:
                // Leere BGCOLOR bei <TABLE>, <TR> und <TD>/<TH> wie Netscape
                // ignorieren, bei allen anderen Tags *wirklich* nicht.
                if( !rOption.GetString().isEmpty() )
                {
                    rOption.GetColor( aBGColor );
                    bBGColor = true;
                }
                break;
            case HTML_O_BACKGROUND:
                aBGImage = rOption.GetString();
                break;
            case HTML_O_STYLE:
                aStyle = rOption.GetString();
                break;
            case HTML_O_CLASS:
                aClass = rOption.GetString();
                break;
            case HTML_O_LANG:
                aLang = rOption.GetString();
                break;
            case HTML_O_DIR:
                aDir = rOption.GetString();
                break;
            case HTML_O_SDNUM:
                aNumFormat = rOption.GetString();
                bHasNumFormat = true;
                break;
            case HTML_O_SDVAL:
                bHasValue = true;
                aValue = rOption.GetString();
                break;
            case HTML_O_NOWRAP:
                bNoWrap = true;
                break;
            }
        }

        if( !aId.isEmpty() )
            rParser.InsertBookmark( aId );
    }

    if( bHasNumFormat )
    {
        LanguageType eLang;
        nValue = SfxHTMLParser::GetTableDataOptionsValNum(
                            nNumFormat, eLang, aValue, aNumFormat,
                            *rParser.m_pDoc->GetNumberFormatter() );
    }

    // einen neuen Kontext anlegen, aber das drawing::Alignment-Attribut
    // nicht dort verankern, weil es noch ger keine Section gibt, in der
    // es gibt.
    sal_uInt16 nToken, nColl;
    if( bHead )
    {
        nToken = HTML_TABLEHEADER_ON;
        nColl = RES_POOLCOLL_TABLE_HDLN;
    }
    else
    {
        nToken = HTML_TABLEDATA_ON;
        nColl = RES_POOLCOLL_TABLE;
    }
    _HTMLAttrContext *pCntxt = new _HTMLAttrContext( nToken, nColl, aEmptyOUStr, true );
    if( SVX_ADJUST_END != eAdjust )
        rParser.InsertAttr( &rParser.m_aAttrTab.pAdjust, SvxAdjustItem(eAdjust, RES_PARATR_ADJUST),
                            pCntxt );

    if( SwHTMLParser::HasStyleOptions( aStyle, aId, aClass, &aLang, &aDir ) )
    {
        SfxItemSet aItemSet( rParser.m_pDoc->GetAttrPool(),
                             rParser.m_pCSS1Parser->GetWhichMap() );
        SvxCSS1PropertyInfo aPropInfo;

        if( rParser.ParseStyleOptions( aStyle, aId, aClass, aItemSet,
                                       aPropInfo, &aLang, &aDir ) )
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

_CellSaveStruct::~_CellSaveStruct()
{
    delete pNoBreakEndParaIdx;
}

void _CellSaveStruct::AddContents( HTMLTableCnts *pNewCnts )
{
    if( pCnts )
        pCnts->Add( pNewCnts );
    else
        pCnts = pNewCnts;

    pCurrCnts = pNewCnts;
}

void _CellSaveStruct::InsertCell( SwHTMLParser& rParser,
                                  HTMLTable *pCurTable )
{
#if OSL_DEBUG_LEVEL > 0
    // Die Attribute muessen schon beim Auefrauemen des Kontext-Stacks
    // entfernt worden sein, sonst ist etwas schiefgelaufen. Das
    // Checken wir mal eben ...
    // MIB 8.1.98: Wenn ausserhalb einer Zelle Attribute geoeffnet
    // wurden stehen diese noch in der Attribut-Tabelle und werden erst
    // ganz zum Schluss durch die CleanContext-Aufrufe in BuildTable
    // geloescht. Damit es in diesem Fall keine Asserts gibt findet dann
    // keine Ueberpruefung statt. Erkennen tut man diesen Fall an
    // nContextStAttrMin: Der gemerkte Wert nContextStAttrMinSave ist der
    // Wert, den nContextStAttrMin beim Start der Tabelle hatte. Und
    // der aktuelle Wert von nContextStAttrMin entspricht der Anzahl der
    // Kontexte, die beim Start der Zelle vorgefunden wurden. Sind beide
    // Werte unterschiedlich, wurden ausserhalb der Zelle Kontexte
    // angelegt und wir ueberpruefen nichts.

    if( rParser.m_nContextStAttrMin == GetContextStAttrMin() )
    {
        _HTMLAttr** pTable = reinterpret_cast<_HTMLAttr**>(&rParser.m_aAttrTab);

        for( auto nCnt = sizeof( _HTMLAttrTable ) / sizeof( _HTMLAttr* );
            nCnt--; ++pTable )
        {
            OSL_ENSURE( !*pTable, "Die Attribut-Tabelle ist nicht leer" );
        }
    }
#endif

    // jetzt muessen wir noch die Zelle an der aktuellen Position einfuegen
    SvxBrushItem *pBrushItem =
        rParser.CreateBrushItem( bBGColor ? &aBGColor : nullptr, aBGImage,
                                 aStyle, aId, aClass );
    pCurTable->InsertCell( pCnts, nRowSpan, nColSpan, nWidth,
                           bPrcWidth, nHeight, eVertOri, pBrushItem, m_pBoxItem,
                           bHasNumFormat, nNumFormat, bHasValue, nValue,
                           bNoWrap );
    Restore( rParser );
}

void _CellSaveStruct::StartNoBreak( const SwPosition& rPos )
{
    if( !pCnts ||
        (!rPos.nContent.GetIndex() && pCurrCnts==pCnts &&
         pCnts->GetStartNode() &&
         pCnts->GetStartNode()->GetIndex() + 1 ==
            rPos.nNode.GetIndex()) )
    {
        bNoBreak = true;
    }
}

void _CellSaveStruct::EndNoBreak( const SwPosition& rPos )
{
    if( bNoBreak )
    {
        delete pNoBreakEndParaIdx;
        pNoBreakEndParaIdx = new SwNodeIndex( rPos.nNode );
        nNoBreakEndContentPos = rPos.nContent.GetIndex();
        bNoBreak = false;
    }
}

void _CellSaveStruct::CheckNoBreak( const SwPosition& rPos, SwDoc * /*pDoc*/ )
{
    if( pCnts && pCurrCnts==pCnts )
    {
        if( bNoBreak )
        {
            // <NOBR> wurde nicht beendet
            pCnts->SetNoBreak();
        }
        else if( pNoBreakEndParaIdx &&
                 pNoBreakEndParaIdx->GetIndex() == rPos.nNode.GetIndex() )
        {
            if( nNoBreakEndContentPos == rPos.nContent.GetIndex() )
            {
                // <NOBR> wurde unmittelbar vor dem Zellen-Ende beendet
                pCnts->SetNoBreak();
            }
            else if( nNoBreakEndContentPos + 1 == rPos.nContent.GetIndex() )
            {
                SwTextNode const*const pTextNd(rPos.nNode.GetNode().GetTextNode());
                if( pTextNd )
                {
                    sal_Unicode const cLast =
                            pTextNd->GetText()[nNoBreakEndContentPos];
                    if( ' '==cLast || '\x0a'==cLast )
                    {
                        // Zwischem dem </NOBR> und dem Zellen-Ende gibt es nur
                        // ein Blank oder einen Zeilenumbruch.
                        pCnts->SetNoBreak();
                    }
                }
            }
        }
    }
}

HTMLTableCnts *SwHTMLParser::InsertTableContents(
                                        bool bHead )
{
    // eine neue Section anlegen, der PaM steht dann darin
    const SwStartNode *pStNd =
        InsertTableSection( static_cast< sal_uInt16 >(bHead ? RES_POOLCOLL_TABLE_HDLN
                                           : RES_POOLCOLL_TABLE) );

    if( GetNumInfo().GetNumRule() )
    {
        // 1. Absatz auf nicht numeriert setzen
        sal_uInt8 nLvl = GetNumInfo().GetLevel();

        SetNodeNum( nLvl, false );
    }

    // Attributierungs-Anfang neu setzen
    const SwNodeIndex& rSttPara = m_pPam->GetPoint()->nNode;
    sal_Int32 nSttCnt = m_pPam->GetPoint()->nContent.GetIndex();

    _HTMLAttr** pHTMLAttributes = reinterpret_cast<_HTMLAttr**>(&m_aAttrTab);
    for (sal_uInt16 nCnt = sizeof(_HTMLAttrTable) / sizeof(_HTMLAttr*); nCnt--; ++pHTMLAttributes)
    {
        _HTMLAttr *pAttr = *pHTMLAttributes;
        while( pAttr )
        {
            OSL_ENSURE( !pAttr->GetPrev(), "Attribut hat Previous-Liste" );
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

    _CellSaveStruct* pSaveStruct;

    int nToken = 0;
    bool bPending = false;
    if( m_pPendStack )
    {
        pSaveStruct = static_cast<_CellSaveStruct*>(m_pPendStack->pData);

        SwPendingStack* pTmp = m_pPendStack->pNext;
        delete m_pPendStack;
        m_pPendStack = pTmp;
        nToken = m_pPendStack ? m_pPendStack->nToken : GetSaveToken();
        bPending = SVPAR_ERROR == eState && m_pPendStack != nullptr;

        SaveState( nToken );
    }
    else
    {
        // <TH> bzw. <TD> wurde bereits gelesen
        if( m_pTable->IsOverflowing() )
        {
            SaveState( 0 );
            return;
        }

        if( !pCurTable->GetContext() )
        {
            bool bTopTable = m_pTable==pCurTable;

            // die Tabelle besitzt noch keinen Inhalt, d.h. die eigentliche
            // Tabelle muss erst noch angelegt werden

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

            SfxItemSet aItemSet( m_pDoc->GetAttrPool(), aWhichIds );
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

            // Den linken/rechten Absatzeinzug ermitteln
            sal_uInt16 nLeftSpace = 0;
            sal_uInt16 nRightSpace = 0;
            short nIndent;
            GetMarginsFromContextWithNumBul( nLeftSpace, nRightSpace, nIndent );

            // die aktuelle Position an die wir irgendwann zurueckkehren
            SwPosition *pSavePos = nullptr;
            bool bForceFrame = false;
            bool bAppended = false;
            bool bParentLFStripped = false;
            if( bTopTable )
            {
                SvxAdjust eTableAdjust = m_pTable->GetTableAdjust(false);

                // Wenn die Tabelle links oder rechts ausgerivchtet ist,
                // oder in einen Rahmen soll, dann kommt sie auch in einen
                // solchen.
                bForceFrame = eTableAdjust == SVX_ADJUST_LEFT ||
                              eTableAdjust == SVX_ADJUST_RIGHT ||
                              pCurTable->HasToFly();

                // Entweder kommt die Tabelle in keinen Rahmen und befindet
                // sich in keinem Rahmen (wird also durch Zellen simuliert),
                // oder es gibt bereits Inhalt an der entsprechenden Stelle.
                OSL_ENSURE( !bForceFrame || pCurTable->HasParentSection(),
                        "Tabelle im Rahmen hat keine Umgebung!" );

                bool bAppend = false;
                if( bForceFrame )
                {
                    // Wenn die Tabelle in einen Rahmen kommt, muss
                    // nur ein neuer Absatz aufgemacht werden, wenn
                    // der Absatz Rahmen ohne Umlauf enthaelt.
                    bAppend = HasCurrentParaFlys(true);
                }
                else
                {
                    // Sonst muss ein neuer Absatz aufgemacht werden,
                    // wenn der Absatz nicht leer ist, oder Rahmen
                    // oder text::Bookmarks enthaelt.
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
                        m_pDoc->SetTextFormatColl( *m_pPam,
                            m_pCSS1Parser->GetTextCollFromPool(RES_POOLCOLL_STANDARD) );
                        SvxFontHeightItem aFontHeight( 40, 100, RES_CHRATR_FONTSIZE );

                        _HTMLAttr* pTmp =
                            new _HTMLAttr( *m_pPam->GetPoint(), aFontHeight );
                        m_aSetAttrTab.push_back( pTmp );

                        SvxFontHeightItem aFontHeightCJK( 40, 100, RES_CHRATR_CJK_FONTSIZE );
                        pTmp =
                            new _HTMLAttr( *m_pPam->GetPoint(), aFontHeightCJK );
                        m_aSetAttrTab.push_back( pTmp );

                        SvxFontHeightItem aFontHeightCTL( 40, 100, RES_CHRATR_CTL_FONTSIZE );
                        pTmp =
                            new _HTMLAttr( *m_pPam->GetPoint(), aFontHeightCTL );
                        m_aSetAttrTab.push_back( pTmp );

                        pTmp = new _HTMLAttr( *m_pPam->GetPoint(),
                                            SvxULSpaceItem( 0, 0, RES_UL_SPACE ) );
                        m_aSetAttrTab.push_front( pTmp ); // ja, 0, weil schon
                                                        // vom Tabellenende vorher
                                                        // was gesetzt sein kann.
                    }
                    AppendTextNode( AM_NOSPACE );
                    bAppended = true;
                }
                else if( !m_aParaAttrs.empty() )
                {
                    if( !bForceFrame )
                    {
                        // Der Absatz wird gleich hinter die Tabelle
                        // verschoben. Deshalb entfernen wir alle harten
                        // Attribute des Absatzes

                        for( size_t i=0; i<m_aParaAttrs.size(); i++ )
                            m_aParaAttrs[i]->Invalidate();
                    }

                    m_aParaAttrs.clear();
                }

                pSavePos = new SwPosition( *m_pPam->GetPoint() );
            }
            else if( pCurTable->HasParentSection() )
            {
                bParentLFStripped = StripTrailingLF() > 0;

                // Absaetze bzw. ueberschriften beeenden
                m_nOpenParaToken = 0;
                m_nFontStHeadStart = m_nFontStMin;

                // die harten Attribute an diesem Absatz werden nie mehr ungueltig
                if( !m_aParaAttrs.empty() )
                    m_aParaAttrs.clear();
            }

            // einen Tabellen Kontext anlegen
            _HTMLTableContext *pTCntxt =
                        new _HTMLTableContext( pSavePos, m_nContextStMin,
                                               m_nContextStAttrMin );

            // alle noch offenen Attribute beenden und hinter der Tabelle
            // neu aufspannen
            _HTMLAttrs *pPostIts = nullptr;
            if( !bForceFrame && (bTopTable || pCurTable->HasParentSection()) )
            {
                SplitAttrTab( pTCntxt->aAttrTab, bTopTable );
                // Wenn wir einen schon vorhandenen Absatz verwenden, duerfen
                // in den keine PostIts eingefuegt werden, weil der Absatz
                // ja hinter die Tabelle wandert. Sie werden deshalb in den
                // ersten Absatz der Tabelle verschoben.
                // Bei Tabellen in Tabellen duerfen ebenfalls keine PostIts
                // in einen noch leeren Absatz eingefuegt werden, weil
                // der sonat nicht geloescht wird.
                if( (bTopTable && !bAppended) ||
                    (!bTopTable && !bParentLFStripped &&
                     !m_pPam->GetPoint()->nContent.GetIndex()) )
                    pPostIts = new _HTMLAttrs;
                SetAttr( bTopTable, bTopTable, pPostIts );
            }
            else
            {
                SaveAttrTab( pTCntxt->aAttrTab );
                if( bTopTable && !bAppended )
                {
                    pPostIts = new _HTMLAttrs;
                    SetAttr( true, true, pPostIts );
                }
            }
            m_bNoParSpace = false;

            // Aktuelle Numerierung retten und auschalten.
            pTCntxt->SetNumInfo( GetNumInfo() );
            GetNumInfo().Clear();
            pTCntxt->SavePREListingXMP( *this );

            if( bTopTable )
            {
                if( bForceFrame )
                {
                    // Die Tabelle soll in einen Rahmen geschaufelt werden.

                    SfxItemSet aFrameSet( m_pDoc->GetAttrPool(),
                                        RES_FRMATR_BEGIN, RES_FRMATR_END-1 );
                    if( !pCurTable->IsNewDoc() )
                        Reader::ResetFrameFormatAttrs( aFrameSet );

                    SwSurround eSurround = SURROUND_NONE;
                    sal_Int16 eHori;

                    switch( pCurTable->GetTableAdjust(true) )
                    {
                    case SVX_ADJUST_RIGHT:
                        eHori = text::HoriOrientation::RIGHT;
                        eSurround = SURROUND_LEFT;
                        break;
                    case SVX_ADJUST_CENTER:
                        eHori = text::HoriOrientation::CENTER;
                        break;
                    case SVX_ADJUST_LEFT:
                        eSurround = SURROUND_RIGHT;
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
                    SwFrameFormat *pFrameFormat =  m_pDoc->MakeFlySection(
                                eAnchorId, m_pPam->GetPoint(), &aFrameSet );

                    pTCntxt->SetFrameFormat( pFrameFormat );
                    const SwFormatContent& rFlyContent = pFrameFormat->GetContent();
                    m_pPam->GetPoint()->nNode = *rFlyContent.GetContentIdx();
                    SwContentNode *pCNd =
                        m_pDoc->GetNodes().GoNext( &(m_pPam->GetPoint()->nNode) );
                    m_pPam->GetPoint()->nContent.Assign( pCNd, 0 );

                }

                // eine SwTable mit einer Box anlegen und den PaM in den
                // Inhalt der Box-Section bewegen (der Ausrichtungs-Parameter
                // ist erstmal nur ein Dummy und wird spaeter noch richtig
                // gesetzt)
                OSL_ENSURE( !m_pPam->GetPoint()->nContent.GetIndex(),
                        "Der Absatz hinter der Tabelle ist nicht leer!" );
                const SwTable* pSwTable = m_pDoc->InsertTable(
                        SwInsertTableOptions( tabopts::HEADLINE_NO_BORDER, 1 ),
                        *m_pPam->GetPoint(), 1, 1, text::HoriOrientation::LEFT );

                if( bForceFrame )
                {
                    SwNodeIndex aDstIdx( m_pPam->GetPoint()->nNode );
                    m_pPam->Move( fnMoveBackward );
                    m_pDoc->GetNodes().Delete( aDstIdx );
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
                    OSL_ENSURE( pOldTextNd, "Wieso stehen wir in keinem Text-Node?" );
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
                        case SVX_BREAK_PAGE_BEFORE:
                        case SVX_BREAK_PAGE_AFTER:
                        case SVX_BREAK_PAGE_BOTH:
                            pFrameFormat->SetFormatAttr( *pItem2 );
                            pOldTextNd->ResetAttr( RES_BREAK );
                        default:
                            ;
                        }
                    }
                }

                if( !bAppended && pPostIts )
                {
                    // noch vorhandene PostIts in den ersten Absatz
                    // der Tabelle setzen
                    InsertAttrs( *pPostIts );
                    delete pPostIts;
                    pPostIts = nullptr;
                }

                pTCntxt->SetTableNode( const_cast<SwTableNode *>(pNd->FindTableNode()) );

                pCurTable->SetTable( pTCntxt->GetTableNode(), pTCntxt,
                                     nLeftSpace, nRightSpace,
                                     pSwTable, bForceFrame );

                OSL_ENSURE( !pPostIts, "ubenutzte PostIts" );
            }
            else
            {
                // noch offene Bereiche muessen noch entfernt werden
                if( EndSections( bParentLFStripped ) )
                    bParentLFStripped = false;

                if( pCurTable->HasParentSection() )
                {
                    // dannach entfernen wir ein ggf. zu viel vorhandenen
                    // leeren Absatz, aber nur, wenn er schon vor dem
                    // entfernen von LFs leer war
                    if( !bParentLFStripped )
                        StripTrailingPara();

                    if( pPostIts )
                    {
                        // noch vorhandene PostIts an das Ende des jetzt
                        // aktuellen Absatzes schieben
                        InsertAttrs( *pPostIts );
                        delete pPostIts;
                        pPostIts = nullptr;
                    }
                }

                SwNode const*const pNd = & m_pPam->GetPoint()->nNode.GetNode();
                const SwStartNode *pStNd = (m_pTable->bFirstCell ? pNd->FindTableNode()
                                                            : pNd->FindTableBoxStartNode() );

                pCurTable->SetTable( pStNd, pTCntxt, nLeftSpace, nRightSpace );
            }

            // Den Kontext-Stack einfrieren, denn es koennen auch mal
            // irgendwo ausserhalb von Zellen Attribute gesetzt werden.
            // Darf nicht frueher passieren, weil eventuell noch im
            // Stack gesucht wird!!!
            m_nContextStMin = m_aContexts.size();
            m_nContextStAttrMin = m_nContextStMin;
        }

        pSaveStruct = new _CellSaveStruct( *this, pCurTable, bHead,
                                            bReadOptions );

        // ist beim ersten GetNextToken schon pending, muss bei
        // wiederaufsetzen auf jedenfall neu gelesen werden!
        SaveState( 0 );
    }

    if( !nToken )
        nToken = GetNextToken();    // Token nach <TABLE>

    bool bDone = false;
    while( (IsParserWorking() && !bDone) || bPending )
    {
        SaveState( nToken );

        nToken = FilterToken( nToken );

        OSL_ENSURE( m_pPendStack || !m_bCallNextToken || pSaveStruct->IsInSection(),
                "Wo ist die Section gebieben?" );
        if( !m_pPendStack && m_bCallNextToken && pSaveStruct->IsInSection() )
        {
            // NextToken direkt aufrufen (z.B. um den Inhalt von
            // Floating-Frames oder Applets zu ignorieren)
            NextToken( nToken );
        }
        else switch( nToken )
        {
        case HTML_TABLEHEADER_ON:
        case HTML_TABLEDATA_ON:
        case HTML_TABLEROW_ON:
        case HTML_TABLEROW_OFF:
        case HTML_THEAD_ON:
        case HTML_THEAD_OFF:
        case HTML_TFOOT_ON:
        case HTML_TFOOT_OFF:
        case HTML_TBODY_ON:
        case HTML_TBODY_OFF:
        case HTML_TABLE_OFF:
            SkipToken();
            SAL_FALLTHROUGH;
        case HTML_TABLEHEADER_OFF:
        case HTML_TABLEDATA_OFF:
            bDone = true;
            break;
        case HTML_TABLE_ON:
            {
                bool bHasToFly = false;
                SvxAdjust eTabAdjust = SVX_ADJUST_END;
                if( !m_pPendStack )
                {
                    // nur wenn eine neue Tabelle aufgemacht wird, aber
                    // nicht wenn nach einem Pending in der Tabelle
                    // weitergelesen wird!
                    pSaveStruct->m_pTable = m_pTable;

                    // HACK: Eine Section fuer eine Tabelle anlegen, die
                    // in einen Rahmen kommt.
                    if( !pSaveStruct->IsInSection() )
                    {
                        // Diese Schleife muss vorwartes sein, weil die
                        // erste Option immer gewinnt.
                        bool bNeedsSection = false;
                        const HTMLOptions& rHTMLOptions = GetOptions();
                        for (size_t i = 0; i < rHTMLOptions.size(); ++i)
                        {
                            const HTMLOption& rOption = rHTMLOptions[i];
                            if( HTML_O_ALIGN==rOption.GetToken() )
                            {
                                SvxAdjust eAdjust =
                                    (SvxAdjust)rOption.GetEnum(
                                            aHTMLPAlignTable, SVX_ADJUST_END );
                                bNeedsSection = SVX_ADJUST_LEFT == eAdjust ||
                                                SVX_ADJUST_RIGHT == eAdjust;
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
                        // Wenn im aktuellen Absatz Flys verankert sind,
                        // muss die neue Tabelle in einen Rahmen.
                        bHasToFly = HasCurrentParaFlys(false,true);
                    }

                    // in der Zelle kann sich ein Bereich befinden!
                    eTabAdjust = m_aAttrTab.pAdjust
                        ? static_cast<const SvxAdjustItem&>(m_aAttrTab.pAdjust->GetItem()).
                                                 GetAdjust()
                        : SVX_ADJUST_END;
                }

                HTMLTable *pSubTable = BuildTable( eTabAdjust,
                                                   bHead,
                                                   pSaveStruct->IsInSection(),
                                                   bHasToFly );
                if( SVPAR_PENDING != GetStatus() )
                {
                    // nur wenn die Tabelle wirklich zu Ende ist!
                    if( pSubTable )
                    {
                        OSL_ENSURE( pSubTable->GetTableAdjust(false)!= SVX_ADJUST_LEFT &&
                                pSubTable->GetTableAdjust(false)!= SVX_ADJUST_RIGHT,
                                "links oder rechts ausgerichtete Tabellen gehoehren in Rahmen" );

                        HTMLTableCnts *pParentContents =
                            pSubTable->GetParentContents();
                        if( pParentContents )
                        {
                            OSL_ENSURE( !pSaveStruct->IsInSection(),
                                    "Wo ist die Section geblieben" );

                            // Wenn jetzt keine Tabelle kommt haben wir eine
                            // Section
                            pSaveStruct->AddContents( pParentContents );
                        }

                        const SwStartNode *pCapStNd =
                                pSubTable->GetCaptionStartNode();

                        if( pSubTable->GetContext() )
                        {
                            OSL_ENSURE( !pSubTable->GetContext()->GetFrameFormat(),
                                    "Tabelle steht im Rahmen" );

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

                            // Jetzt haben wir keine Section mehr
                            pSaveStruct->ClearIsInSection();
                        }
                        else if( pCapStNd )
                        {
                            // Da wir diese Section nicht mehr loeschen
                            // koennen (sie koeente zur erster Box
                            // gehoeren), fuegen wir sie ein.
                            pSaveStruct->AddContents(
                                new HTMLTableCnts(pCapStNd) );

                            // Jetzt haben wir keine Section mehr
                            pSaveStruct->ClearIsInSection();
                        }
                    }

                    m_pTable = pSaveStruct->m_pTable;
                }
            }
            break;

        case HTML_NOBR_ON:
            // HACK fuer MS: Steht das <NOBR> zu beginn der Zelle?
            pSaveStruct->StartNoBreak( *m_pPam->GetPoint() );
            break;

        case HTML_NOBR_OFF:
                pSaveStruct->EndNoBreak( *m_pPam->GetPoint() );
            break;

        case HTML_COMMENT:
            // Mit Kommentar-Feldern werden Spaces nicht mehr geloescht
            // ausserdem wollen wir fuer einen Kommentar keine neue Zelle
            // anlegen !!!
            NextToken( nToken );
            break;

        case HTML_MARQUEE_ON:
            if( !pSaveStruct->IsInSection() )
            {
                // eine neue Section anlegen, der PaM steht dann darin
                pSaveStruct->AddContents(
                    InsertTableContents( bHead ) );
            }
            m_bCallNextToken = true;
            NewMarquee( pCurTable );
            break;

        case HTML_TEXTTOKEN:
            // keine Section fuer einen leeren String anlegen
            if( !pSaveStruct->IsInSection() && 1==aToken.getLength() &&
                ' '==aToken[0] )
                break;
        default:
            if( !pSaveStruct->IsInSection() )
            {
                // eine neue Section anlegen, der PaM steht dann darin
                pSaveStruct->AddContents(
                    InsertTableContents( bHead ) );
            }

            if( IsParserWorking() || bPending )
                NextToken( nToken );
            break;
        }

        OSL_ENSURE( !bPending || !m_pPendStack,
                "SwHTMLParser::BuildTableCell: Es gibt wieder einen Pend-Stack" );
        bPending = false;
        if( IsParserWorking() )
            SaveState( 0 );

        if( !bDone )
            nToken = GetNextToken();
    }

    if( SVPAR_PENDING == GetStatus() )
    {
        m_pPendStack = new SwPendingStack( bHead ? HTML_TABLEHEADER_ON
                                               : HTML_TABLEDATA_ON, m_pPendStack );
        m_pPendStack->pData = pSaveStruct;

        return;
    }

    // Falls der Inhalt der Zelle leer war, muessen wir noch einen
    // leeren Inhalt anlegen. Ausserdem legen wir einen leeren Inhalt
    // an, wenn die Zelle mit einer Tabelle aufgehoert hat und keine
    // COL-Tags hatte (sonst wurde sie wahrscheinlich von uns exportiert,
    // und dann wollen wir natuerlich keinen zusaetzlichen Absatz haben).
    if( !pSaveStruct->GetFirstContents() ||
        (!pSaveStruct->IsInSection() && !pCurTable->HasColTags()) )
    {
        OSL_ENSURE( pSaveStruct->GetFirstContents() ||
                !pSaveStruct->IsInSection(),
                "Section oder nicht, das ist hier die Frage" );
        const SwStartNode *pStNd =
            InsertTableSection( static_cast< sal_uInt16 >(pSaveStruct->IsHeaderCell()
                                        ? RES_POOLCOLL_TABLE_HDLN
                                        : RES_POOLCOLL_TABLE ));
        const SwEndNode *pEndNd = pStNd->EndOfSectionNode();
        SwContentNode *pCNd = m_pDoc->GetNodes()[pEndNd->GetIndex()-1] ->GetContentNode();
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
        pSaveStruct->CheckNoBreak( *m_pPam->GetPoint(), m_pDoc );

        // Alle noch offenen Kontexte beenden. Wir nehmen hier
        // AttrMin, weil nContxtStMin evtl. veraendert wurde.
        // Da es durch EndContext wieder restauriert wird, geht das.
        while( m_aContexts.size() > m_nContextStAttrMin+1 )
        {
            _HTMLAttrContext *pCntxt = PopContext();
            EndContext( pCntxt );
            delete pCntxt;
        }

        // LFs am Absatz-Ende entfernen
        if( StripTrailingLF()==0 && !m_pPam->GetPoint()->nContent.GetIndex() )
            StripTrailingPara();

        // falls fuer die Zelle eine Ausrichtung gesetzt wurde, muessen
        // wir die beenden
        _HTMLAttrContext *pCntxt = PopContext();
        EndContext( pCntxt );
        delete pCntxt;
    }
    else
    {
        // Alle noch offenen Kontexte beenden
        while( m_aContexts.size() > m_nContextStAttrMin )
        {
            _HTMLAttrContext *pCntxt = PopContext();
            ClearContext( pCntxt );
            delete pCntxt;
        }
    }

    // auch eine Numerierung muss beendet werden
    GetNumInfo().Clear();

    SetAttr( false );

    pSaveStruct->InsertCell( *this, pCurTable );

    // wir stehen jetzt (wahrschenlich) vor <TH>, <TD>, <TR> oder </TABLE>
    delete pSaveStruct;
}

class _RowSaveStruct : public SwPendingStackData
{
public:
    SvxAdjust eAdjust;
    sal_Int16 eVertOri;
    bool bHasCells;

    _RowSaveStruct() :
        eAdjust( SVX_ADJUST_END ), eVertOri( text::VertOrientation::TOP ), bHasCells( false )
    {}
};

void SwHTMLParser::BuildTableRow( HTMLTable *pCurTable, bool bReadOptions,
                                  SvxAdjust eGrpAdjust,
                                  sal_Int16 eGrpVertOri )
{
    // <TR> wurde bereist gelesen

    if( !IsParserWorking() && !m_pPendStack )
        return;

    int nToken = 0;
    _RowSaveStruct* pSaveStruct;

    bool bPending = false;
    if( m_pPendStack )
    {
        pSaveStruct = static_cast<_RowSaveStruct*>(m_pPendStack->pData);

        SwPendingStack* pTmp = m_pPendStack->pNext;
        delete m_pPendStack;
        m_pPendStack = pTmp;
        nToken = m_pPendStack ? m_pPendStack->nToken : GetSaveToken();
        bPending = SVPAR_ERROR == eState && m_pPendStack != nullptr;

        SaveState( nToken );
    }
    else
    {
        SvxAdjust eAdjust = eGrpAdjust;
        sal_Int16 eVertOri = eGrpVertOri;
        Color aBGColor;
        OUString aBGImage, aStyle, aId, aClass;
        bool bBGColor = false;
        pSaveStruct = new _RowSaveStruct;

        if( bReadOptions )
        {
            const HTMLOptions& rHTMLOptions = GetOptions();
            for (size_t i = rHTMLOptions.size(); i; )
            {
                const HTMLOption& rOption = rHTMLOptions[--i];
                switch( rOption.GetToken() )
                {
                case HTML_O_ID:
                    aId = rOption.GetString();
                    break;
                case HTML_O_ALIGN:
                    eAdjust = (SvxAdjust)rOption.GetEnum(
                                    aHTMLPAlignTable, static_cast< sal_uInt16 >(eAdjust) );
                    break;
                case HTML_O_VALIGN:
                    eVertOri = rOption.GetEnum(
                                    aHTMLTableVAlignTable, eVertOri );
                    break;
                case HTML_O_BGCOLOR:
                    // Leere BGCOLOR bei <TABLE>, <TR> und <TD>/<TH> wie Netsc.
                    // ignorieren, bei allen anderen Tags *wirklich* nicht.
                    if( !rOption.GetString().isEmpty() )
                    {
                        rOption.GetColor( aBGColor );
                        bBGColor = true;
                    }
                    break;
                case HTML_O_BACKGROUND:
                    aBGImage = rOption.GetString();
                    break;
                case HTML_O_STYLE:
                    aStyle = rOption.GetString();
                    break;
                case HTML_O_CLASS:
                    aClass= rOption.GetString();
                    break;
                }
            }
        }

        if( !aId.isEmpty() )
            InsertBookmark( aId );

        SvxBrushItem *pBrushItem =
            CreateBrushItem( bBGColor ? &aBGColor : nullptr, aBGImage, aStyle,
                             aId, aClass );
        pCurTable->OpenRow( eAdjust, eVertOri, pBrushItem );
        // ist beim ersten GetNextToken schon pending, muss bei
        // wiederaufsetzen auf jedenfall neu gelesen werden!
        SaveState( 0 );
    }

    if( !nToken )
        nToken = GetNextToken();    // naechstes Token

    bool bDone = false;
    while( (IsParserWorking() && !bDone) || bPending )
    {
        SaveState( nToken );

        nToken = FilterToken( nToken );

        OSL_ENSURE( m_pPendStack || !m_bCallNextToken ||
                pCurTable->GetContext() || pCurTable->HasParentSection(),
                "Wo ist die Section gebieben?" );
        if( !m_pPendStack && m_bCallNextToken &&
            (pCurTable->GetContext() || pCurTable->HasParentSection()) )
        {
            // NextToken direkt aufrufen (z.B. um den Inhalt von
            // Floating-Frames oder Applets zu ignorieren)
            NextToken( nToken );
        }
        else switch( nToken )
        {
        case HTML_TABLE_ON:
            if( !pCurTable->GetContext()  )
            {
                SkipToken();
                bDone = true;
            }

            break;
        case HTML_TABLEROW_ON:
        case HTML_THEAD_ON:
        case HTML_THEAD_OFF:
        case HTML_TBODY_ON:
        case HTML_TBODY_OFF:
        case HTML_TFOOT_ON:
        case HTML_TFOOT_OFF:
        case HTML_TABLE_OFF:
            SkipToken();
            SAL_FALLTHROUGH;
        case HTML_TABLEROW_OFF:
            bDone = true;
            break;
        case HTML_TABLEHEADER_ON:
        case HTML_TABLEDATA_ON:
            BuildTableCell( pCurTable, true, HTML_TABLEHEADER_ON==nToken );
            if( SVPAR_PENDING != GetStatus() )
            {
                pSaveStruct->bHasCells = true;
                bDone = m_pTable->IsOverflowing();
            }
            break;
        case HTML_CAPTION_ON:
            BuildTableCaption( pCurTable );
            bDone = m_pTable->IsOverflowing();
            break;
        case HTML_CAPTION_OFF:
        case HTML_TABLEHEADER_OFF:
        case HTML_TABLEDATA_OFF:
        case HTML_COLGROUP_ON:
        case HTML_COLGROUP_OFF:
        case HTML_COL_ON:
        case HTML_COL_OFF:
            // wo keine Zelle anfing kann auch keine aufhoehren, oder?
            // und die ganzen anderen Tokens haben hier auch nicht zu
            // suchen und machen nur die Tabelle kaputt
            break;
        case HTML_MULTICOL_ON:
            // spaltige Rahmen koennen wir hier leider nicht einguegen
            break;
        case HTML_FORM_ON:
            NewForm( false );   // keinen neuen Absatz aufmachen!
            break;
        case HTML_FORM_OFF:
            EndForm( false );   // keinen neuen Absatz aufmachen!
            break;
        case HTML_COMMENT:
            NextToken( nToken );
            break;
        case HTML_MAP_ON:
            // eine Image-Map fuegt nichts ein, deshalb koennen wir sie
            // problemlos auch ohne Zelle parsen
            NextToken( nToken );
            break;
        case HTML_TEXTTOKEN:
            if( (pCurTable->GetContext() ||
                 !pCurTable->HasParentSection()) &&
                1==aToken.getLength() && ' '==aToken[0] )
                break;
        default:
            pCurTable->MakeParentContents();
            NextToken( nToken );
            break;
        }

        OSL_ENSURE( !bPending || !m_pPendStack,
                "SwHTMLParser::BuildTableRow: Es gibt wieder einen Pend-Stack" );
        bPending = false;
        if( IsParserWorking() )
            SaveState( 0 );

        if( !bDone )
            nToken = GetNextToken();
    }

    if( SVPAR_PENDING == GetStatus() )
    {
        m_pPendStack = new SwPendingStack( HTML_TABLEROW_ON, m_pPendStack );
        m_pPendStack->pData = pSaveStruct;
    }
    else
    {
        pCurTable->CloseRow( !pSaveStruct->bHasCells );
        delete pSaveStruct;
    }

    // wir stehen jetzt (wahrscheinlich) vor <TR> oder </TABLE>
}

void SwHTMLParser::BuildTableSection( HTMLTable *pCurTable,
                                      bool bReadOptions,
                                      bool bHead )
{
    // <THEAD>, <TBODY> bzw. <TFOOT> wurde bereits gelesen
    if( !IsParserWorking() && !m_pPendStack )
        return;

    int nToken = 0;
    bool bPending = false;
    _RowSaveStruct* pSaveStruct;

    if( m_pPendStack )
    {
        pSaveStruct = static_cast<_RowSaveStruct*>(m_pPendStack->pData);

        SwPendingStack* pTmp = m_pPendStack->pNext;
        delete m_pPendStack;
        m_pPendStack = pTmp;
        nToken = m_pPendStack ? m_pPendStack->nToken : GetSaveToken();
        bPending = SVPAR_ERROR == eState && m_pPendStack != nullptr;

        SaveState( nToken );
    }
    else
    {
        pSaveStruct = new _RowSaveStruct;

        if( bReadOptions )
        {
            const HTMLOptions& rHTMLOptions = GetOptions();
            for (size_t i = rHTMLOptions.size(); i; )
            {
                const HTMLOption& rOption = rHTMLOptions[--i];
                switch( rOption.GetToken() )
                {
                case HTML_O_ID:
                    InsertBookmark( rOption.GetString() );
                    break;
                case HTML_O_ALIGN:
                    pSaveStruct->eAdjust =
                        (SvxAdjust)rOption.GetEnum( aHTMLPAlignTable,
                                                     static_cast< sal_uInt16 >(pSaveStruct->eAdjust) );
                    break;
                case HTML_O_VALIGN:
                    pSaveStruct->eVertOri =
                        rOption.GetEnum( aHTMLTableVAlignTable,
                                          pSaveStruct->eVertOri );
                    break;
                }
            }
        }

        // ist beim ersten GetNextToken schon pending, muss bei
        // wiederaufsetzen auf jedenfall neu gelesen werden!
        SaveState( 0 );
    }

    if( !nToken )
        nToken = GetNextToken();    // naechstes Token

    bool bDone = false;
    while( (IsParserWorking() && !bDone) || bPending )
    {
        SaveState( nToken );

        nToken = FilterToken( nToken );

        OSL_ENSURE( m_pPendStack || !m_bCallNextToken ||
                pCurTable->GetContext() || pCurTable->HasParentSection(),
                "Wo ist die Section gebieben?" );
        if( !m_pPendStack && m_bCallNextToken &&
            (pCurTable->GetContext() || pCurTable->HasParentSection()) )
        {
            // NextToken direkt aufrufen (z.B. um den Inhalt von
            // Floating-Frames oder Applets zu ignorieren)
            NextToken( nToken );
        }
        else switch( nToken )
        {
        case HTML_TABLE_ON:
            if( !pCurTable->GetContext()  )
            {
                SkipToken();
                bDone = true;
            }

            break;
        case HTML_THEAD_ON:
        case HTML_TFOOT_ON:
        case HTML_TBODY_ON:
        case HTML_TABLE_OFF:
            SkipToken();
            SAL_FALLTHROUGH;
        case HTML_THEAD_OFF:
        case HTML_TBODY_OFF:
        case HTML_TFOOT_OFF:
            bDone = true;
            break;
        case HTML_CAPTION_ON:
            BuildTableCaption( pCurTable );
            bDone = m_pTable->IsOverflowing();
            break;
        case HTML_CAPTION_OFF:
            break;
        case HTML_TABLEHEADER_ON:
        case HTML_TABLEDATA_ON:
            SkipToken();
            BuildTableRow( pCurTable, false, pSaveStruct->eAdjust,
                           pSaveStruct->eVertOri );
            bDone = m_pTable->IsOverflowing();
            break;
        case HTML_TABLEROW_ON:
            BuildTableRow( pCurTable, true, pSaveStruct->eAdjust,
                           pSaveStruct->eVertOri );
            bDone = m_pTable->IsOverflowing();
            break;
        case HTML_MULTICOL_ON:
            // spaltige Rahmen koennen wir hier leider nicht einguegen
            break;
        case HTML_FORM_ON:
            NewForm( false );   // keinen neuen Absatz aufmachen!
            break;
        case HTML_FORM_OFF:
            EndForm( false );   // keinen neuen Absatz aufmachen!
            break;
        case HTML_TEXTTOKEN:
            // Blank-Strings sind Folge von CR+LF und kein Text
            if( (pCurTable->GetContext() ||
                 !pCurTable->HasParentSection()) &&
                1==aToken.getLength() && ' ' == aToken[0] )
                break;
        default:
            pCurTable->MakeParentContents();
            NextToken( nToken );
        }

        OSL_ENSURE( !bPending || !m_pPendStack,
                "SwHTMLParser::BuildTableSection: Es gibt wieder einen Pend-Stack" );
        bPending = false;
        if( IsParserWorking() )
            SaveState( 0 );

        if( !bDone )
            nToken = GetNextToken();
    }

    if( SVPAR_PENDING == GetStatus() )
    {
        m_pPendStack = new SwPendingStack( bHead ? HTML_THEAD_ON
                                               : HTML_TBODY_ON, m_pPendStack );
        m_pPendStack->pData = pSaveStruct;
    }
    else
    {
        pCurTable->CloseSection( bHead );
        delete pSaveStruct;
    }

    // now we stand (perhaps) in front of <TBODY>,... or </TABLE>
}

struct _TableColGrpSaveStruct : public SwPendingStackData
{
    sal_uInt16 nColGrpSpan;
    sal_uInt16 nColGrpWidth;
    bool bRelColGrpWidth;
    SvxAdjust eColGrpAdjust;
    sal_Int16 eColGrpVertOri;

    inline _TableColGrpSaveStruct();

    inline void CloseColGroup( HTMLTable *pTable );
};

inline _TableColGrpSaveStruct::_TableColGrpSaveStruct() :
    nColGrpSpan( 1 ), nColGrpWidth( 0 ),
    bRelColGrpWidth( false ), eColGrpAdjust( SVX_ADJUST_END ),
    eColGrpVertOri( text::VertOrientation::TOP )
{}

inline void _TableColGrpSaveStruct::CloseColGroup( HTMLTable *pTable )
{
    pTable->CloseColGroup( nColGrpSpan, nColGrpWidth,
                            bRelColGrpWidth, eColGrpAdjust, eColGrpVertOri );
}

void SwHTMLParser::BuildTableColGroup( HTMLTable *pCurTable,
                                       bool bReadOptions )
{
    // <COLGROUP> wurde bereits gelesen, wenn bReadOptions

    if( !IsParserWorking() && !m_pPendStack )
        return;

    int nToken = 0;
    bool bPending = false;
    _TableColGrpSaveStruct* pSaveStruct;

    if( m_pPendStack )
    {
        pSaveStruct = static_cast<_TableColGrpSaveStruct*>(m_pPendStack->pData);

        SwPendingStack* pTmp = m_pPendStack->pNext;
        delete m_pPendStack;
        m_pPendStack = pTmp;
        nToken = m_pPendStack ? m_pPendStack->nToken : GetSaveToken();
        bPending = SVPAR_ERROR == eState && m_pPendStack != nullptr;

        SaveState( nToken );
    }
    else
    {

        pSaveStruct = new _TableColGrpSaveStruct;
        if( bReadOptions )
        {
            const HTMLOptions& rColGrpOptions = GetOptions();
            for (size_t i = rColGrpOptions.size(); i; )
            {
                const HTMLOption& rOption = rColGrpOptions[--i];
                switch( rOption.GetToken() )
                {
                case HTML_O_ID:
                    InsertBookmark( rOption.GetString() );
                    break;
                case HTML_O_SPAN:
                    pSaveStruct->nColGrpSpan = (sal_uInt16)rOption.GetNumber();
                    break;
                case HTML_O_WIDTH:
                    pSaveStruct->nColGrpWidth = (sal_uInt16)rOption.GetNumber();
                    pSaveStruct->bRelColGrpWidth =
                        (rOption.GetString().indexOf('*') != -1);
                    break;
                case HTML_O_ALIGN:
                    pSaveStruct->eColGrpAdjust =
                        (SvxAdjust)rOption.GetEnum( aHTMLPAlignTable,
                                                static_cast< sal_uInt16 >(pSaveStruct->eColGrpAdjust) );
                    break;
                case HTML_O_VALIGN:
                    pSaveStruct->eColGrpVertOri =
                        rOption.GetEnum( aHTMLTableVAlignTable,
                                                pSaveStruct->eColGrpVertOri );
                    break;
                }
            }
        }
        // ist beim ersten GetNextToken schon pending, muss bei
        // wiederaufsetzen auf jedenfall neu gelesen werden!
        SaveState( 0 );
    }

    if( !nToken )
        nToken = GetNextToken();    // naechstes Token

    bool bDone = false;
    while( (IsParserWorking() && !bDone) || bPending )
    {
        SaveState( nToken );

        nToken = FilterToken( nToken );

        OSL_ENSURE( m_pPendStack || !m_bCallNextToken ||
                pCurTable->GetContext() || pCurTable->HasParentSection(),
                "Wo ist die Section gebieben?" );
        if( !m_pPendStack && m_bCallNextToken &&
            (pCurTable->GetContext() || pCurTable->HasParentSection()) )
        {
            // NextToken direkt aufrufen (z.B. um den Inhalt von
            // Floating-Frames oder Applets zu ignorieren)
            NextToken( nToken );
        }
        else switch( nToken )
        {
        case HTML_TABLE_ON:
            if( !pCurTable->GetContext()  )
            {
                SkipToken();
                bDone = true;
            }

            break;
        case HTML_COLGROUP_ON:
        case HTML_THEAD_ON:
        case HTML_TFOOT_ON:
        case HTML_TBODY_ON:
        case HTML_TABLEROW_ON:
        case HTML_TABLE_OFF:
            SkipToken();
            SAL_FALLTHROUGH;
        case HTML_COLGROUP_OFF:
            bDone = true;
            break;
        case HTML_COL_ON:
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
                    case HTML_O_ID:
                        InsertBookmark( rOption.GetString() );
                        break;
                    case HTML_O_SPAN:
                        nColSpan = (sal_uInt16)rOption.GetNumber();
                        break;
                    case HTML_O_WIDTH:
                        nColWidth = (sal_uInt16)rOption.GetNumber();
                        bRelColWidth =
                            (rOption.GetString().indexOf('*') != -1);
                        break;
                    case HTML_O_ALIGN:
                        eColAdjust =
                            (SvxAdjust)rOption.GetEnum( aHTMLPAlignTable,
                                                            static_cast< sal_uInt16 >(eColAdjust) );
                        break;
                    case HTML_O_VALIGN:
                        eColVertOri =
                            rOption.GetEnum( aHTMLTableVAlignTable,
                                                        eColVertOri );
                        break;
                    }
                }
                pCurTable->InsertCol( nColSpan, nColWidth, bRelColWidth,
                                      eColAdjust, eColVertOri );

                // die Angaben in <COLGRP> sollen ignoriert werden, wenn
                // <COL>-Elemente existieren
                pSaveStruct->nColGrpSpan = 0;
            }
            break;
        case HTML_COL_OFF:
            break;      // Ignorieren
        case HTML_MULTICOL_ON:
            // spaltige Rahmen koennen wir hier leider nicht einguegen
            break;
        case HTML_TEXTTOKEN:
            if( (pCurTable->GetContext() ||
                 !pCurTable->HasParentSection()) &&
                1==aToken.getLength() && ' '==aToken[0] )
                break;
        default:
            pCurTable->MakeParentContents();
            NextToken( nToken );
        }

        OSL_ENSURE( !bPending || !m_pPendStack,
                "SwHTMLParser::BuildTableColGrp: Es gibt wieder einen Pend-Stack" );
        bPending = false;
        if( IsParserWorking() )
            SaveState( 0 );

        if( !bDone )
            nToken = GetNextToken();
    }

    if( SVPAR_PENDING == GetStatus() )
    {
        m_pPendStack = new SwPendingStack( HTML_COL_ON, m_pPendStack );
        m_pPendStack->pData = pSaveStruct;
    }
    else
    {
        pSaveStruct->CloseColGroup( pCurTable );
        delete pSaveStruct;
    }
}

class _CaptionSaveStruct : public _SectionSaveStruct
{
    SwPosition aSavePos;
    SwHTMLNumRuleInfo aNumRuleInfo; // gueltige Numerierung

public:

    _HTMLAttrTable aAttrTab;        // und die Attribute

    _CaptionSaveStruct( SwHTMLParser& rParser, const SwPosition& rPos ) :
        _SectionSaveStruct( rParser ), aSavePos( rPos )
    {
        rParser.SaveAttrTab( aAttrTab );

        // Die aktuelle Numerierung wurde gerettet und muss nur
        // noch beendet werden.
        aNumRuleInfo.Set( rParser.GetNumInfo() );
        rParser.GetNumInfo().Clear();
    }

    const SwPosition& GetPos() const { return aSavePos; }

    void RestoreAll( SwHTMLParser& rParser )
    {
        // Die alten Stack wiederherstellen
        Restore( rParser );

        // Die alte Attribut-Tabelle wiederherstellen
        rParser.RestoreAttrTab( aAttrTab );

        // Die alte Numerierung wieder aufspannen
        rParser.GetNumInfo().Set( aNumRuleInfo );
    }

    virtual ~_CaptionSaveStruct();
};

_CaptionSaveStruct::~_CaptionSaveStruct()
{}

void SwHTMLParser::BuildTableCaption( HTMLTable *pCurTable )
{
    // <CAPTION> wurde bereits gelesen

    if( !IsParserWorking() && !m_pPendStack )
        return;

    int nToken = 0;
    _CaptionSaveStruct* pSaveStruct;

    if( m_pPendStack )
    {
        pSaveStruct = static_cast<_CaptionSaveStruct*>(m_pPendStack->pData);

        SwPendingStack* pTmp = m_pPendStack->pNext;
        delete m_pPendStack;
        m_pPendStack = pTmp;
        nToken = m_pPendStack ? m_pPendStack->nToken : GetSaveToken();
        OSL_ENSURE( !m_pPendStack, "Wo kommt hier ein Pending-Stack her?" );

        SaveState( nToken );
    }
    else
    {
        if( m_pTable->IsOverflowing() )
        {
            SaveState( 0 );
            return;
        }

        bool bTop = true;
        const HTMLOptions& rHTMLOptions = GetOptions();
        for ( size_t i = rHTMLOptions.size(); i; )
        {
            const HTMLOption& rOption = rHTMLOptions[--i];
            if( HTML_O_ALIGN == rOption.GetToken() )
            {
                if (rOption.GetString().equalsIgnoreAsciiCase(
                        OOO_STRING_SVTOOLS_HTML_VA_bottom))
                {
                    bTop = false;
                }
            }
        }

        // Alte PaM-Position retten.
        pSaveStruct = new _CaptionSaveStruct( *this, *m_pPam->GetPoint() );

        // Eine Text-Section im Icons-Bereich als Container fuer die
        // Ueberschrift anlegen und PaM dort reinstellen.
        const SwStartNode *pStNd;
        if( m_pTable == pCurTable )
            pStNd = InsertTempTableCaptionSection();
        else
            pStNd = InsertTableSection( RES_POOLCOLL_TEXT );

        _HTMLAttrContext *pCntxt = new _HTMLAttrContext( HTML_CAPTION_ON );

        // Tabellen-Ueberschriften sind immer zentriert.
        NewAttr( &m_aAttrTab.pAdjust, SvxAdjustItem(SVX_ADJUST_CENTER, RES_PARATR_ADJUST) );

        _HTMLAttrs &rAttrs = pCntxt->GetAttrs();
        rAttrs.push_back( m_aAttrTab.pAdjust );

        PushContext( pCntxt );

        // StartNode der Section an der Tabelle merken.
        pCurTable->SetCaption( pStNd, bTop );

        // ist beim ersten GetNextToken schon pending, muss bei
        // wiederaufsetzen auf jedenfall neu gelesen werden!
        SaveState( 0 );
    }

    if( !nToken )
        nToken = GetNextToken();    // naechstes Token

    // </CAPTION> wird laut DTD benoetigt
    bool bDone = false;
    while( IsParserWorking() && !bDone )
    {
        SaveState( nToken );

        nToken = FilterToken( nToken );

        switch( nToken )
        {
        case HTML_TABLE_ON:
            if( !m_pPendStack )
            {
                pSaveStruct->m_pTable = m_pTable;
                bool bHasToFly = pSaveStruct->m_pTable!=pCurTable;
                BuildTable( pCurTable->GetTableAdjust( true ),
                            false, true, bHasToFly );
            }
            else
            {
                BuildTable( SVX_ADJUST_END );
            }
            if( SVPAR_PENDING != GetStatus() )
            {
                m_pTable = pSaveStruct->m_pTable;
            }
            break;
        case HTML_TABLE_OFF:
        case HTML_COLGROUP_ON:
        case HTML_THEAD_ON:
        case HTML_TFOOT_ON:
        case HTML_TBODY_ON:
        case HTML_TABLEROW_ON:
            SkipToken();
            bDone = true;
            break;

        case HTML_CAPTION_OFF:
            bDone = true;
            break;
        default:
            if( m_pPendStack )
            {
                SwPendingStack* pTmp = m_pPendStack->pNext;
                delete m_pPendStack;
                m_pPendStack = pTmp;

                OSL_ENSURE( !pTmp, "weiter kann es nicht gehen!" );
            }

            if( IsParserWorking() )
                NextToken( nToken );
            break;
        }

        if( IsParserWorking() )
            SaveState( 0 );

        if( !bDone )
            nToken = GetNextToken();
    }

    if( SVPAR_PENDING==GetStatus() )
    {
        m_pPendStack = new SwPendingStack( HTML_CAPTION_ON, m_pPendStack );
        m_pPendStack->pData = pSaveStruct;
        return;
    }

    // Alle noch offenen Kontexte beenden
    while( m_aContexts.size() > m_nContextStAttrMin+1 )
    {
        _HTMLAttrContext *pCntxt = PopContext();
        EndContext( pCntxt );
        delete pCntxt;
    }

    // LF am Absatz-Ende entfernen
    bool bLFStripped = StripTrailingLF() > 0;

    if( m_pTable==pCurTable )
    {
        // Beim spaeteren verschieben der Beschriftung vor oder hinter
        // die Tabelle wird der letzte Absatz nicht mitverschoben.
        // Deshalb muss sich am Ende der Section immer ein leerer
        // Absatz befinden.
        if( m_pPam->GetPoint()->nContent.GetIndex() || bLFStripped )
            AppendTextNode( AM_NOSPACE );
    }
    else
    {
        // LFs am Absatz-Ende entfernen
        if( !m_pPam->GetPoint()->nContent.GetIndex() && !bLFStripped )
            StripTrailingPara();
    }

    // falls fuer die Zelle eine Ausrichtung gesetzt wurde, muessen
    // wir die beenden
    _HTMLAttrContext *pCntxt = PopContext();
    EndContext( pCntxt );
    delete pCntxt;

    SetAttr( false );

    // Stacks und Attribut-Tabelle wiederherstellen
    pSaveStruct->RestoreAll( *this );

    // PaM wiederherstellen.
    *m_pPam->GetPoint() = pSaveStruct->GetPos();

    delete pSaveStruct;
}

class _TableSaveStruct : public SwPendingStackData
{
public:
    HTMLTable *m_pCurrentTable;

    explicit _TableSaveStruct( HTMLTable *pCurTable ) :
        m_pCurrentTable( pCurTable )
    {}

    virtual ~_TableSaveStruct();

    // Aufbau der Tabelle anstossen und die Tabelle ggf. in einen
    // Rahmen packen. Wenn true zurueckgegeben wird muss noch ein
    // Absatz eingefuegt werden!
    void MakeTable( sal_uInt16 nWidth, SwPosition& rPos, SwDoc *pDoc );
};

_TableSaveStruct::~_TableSaveStruct()
{}

void _TableSaveStruct::MakeTable( sal_uInt16 nWidth, SwPosition& rPos, SwDoc *pDoc )
{
    m_pCurrentTable->MakeTable(nullptr, nWidth);

    _HTMLTableContext *pTCntxt = m_pCurrentTable->GetContext();
    OSL_ENSURE( pTCntxt, "Wo ist der Tabellen-Kontext" );

    SwTableNode *pTableNd = pTCntxt->GetTableNode();
    OSL_ENSURE( pTableNd, "Wo ist der Tabellen-Node" );

    if( pDoc->getIDocumentLayoutAccess().GetCurrentViewShell() && pTableNd )
    {
        // Existiert schon ein Layout, dann muss an dieser Tabelle die
        // BoxFrames neu erzeugt werden.

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
                    "unerwarteter Node fuer das Tabellen-Layout" );
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
    eFrame( HTML_TF_VOID ), eRules( HTML_TR_NONE ),
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
        case HTML_O_ID:
            aId = rOption.GetString();
            break;
        case HTML_O_COLS:
            nCols = (sal_uInt16)rOption.GetNumber();
            break;
        case HTML_O_WIDTH:
            nWidth = (sal_uInt16)rOption.GetNumber();
            bPrcWidth = (rOption.GetString().indexOf('%') != -1);
            if( bPrcWidth && nWidth>100 )
                nWidth = 100;
            break;
        case HTML_O_HEIGHT:
            nHeight = (sal_uInt16)rOption.GetNumber();
            if( rOption.GetString().indexOf('%') != -1 )
                nHeight = 0;    // keine %-Anagben benutzen!!!
            break;
        case HTML_O_CELLPADDING:
            nCellPadding = (sal_uInt16)rOption.GetNumber();
            break;
        case HTML_O_CELLSPACING:
            nCellSpacing = (sal_uInt16)rOption.GetNumber();
            break;
        case HTML_O_ALIGN:
            {
                sal_uInt16 nAdjust = static_cast< sal_uInt16 >(eAdjust);
                if( rOption.GetEnum( nAdjust, aHTMLPAlignTable ) )
                {
                    eAdjust = (SvxAdjust)nAdjust;
                    bTableAdjust = true;
                }
            }
            break;
        case HTML_O_VALIGN:
            eVertOri = rOption.GetEnum( aHTMLTableVAlignTable, eVertOri );
            break;
        case HTML_O_BORDER:
            // BORDER und BORDER=BORDER wie BORDER=1 behandeln
            if (!rOption.GetString().isEmpty() &&
                !rOption.GetString().equalsIgnoreAsciiCase(
                        OOO_STRING_SVTOOLS_HTML_O_border))
            {
                nBorder = (sal_uInt16)rOption.GetNumber();
            }
            else
                nBorder = 1;

            if( !bHasFrame )
                eFrame = ( nBorder ? HTML_TF_BOX : HTML_TF_VOID );
            if( !bHasRules )
                eRules = ( nBorder ? HTML_TR_ALL : HTML_TR_NONE );
            break;
        case HTML_O_FRAME:
            eFrame = rOption.GetTableFrame();
            bHasFrame = true;
            break;
        case HTML_O_RULES:
            eRules = rOption.GetTableRules();
            bHasRules = true;
            break;
        case HTML_O_BGCOLOR:
            // Leere BGCOLOR bei <TABLE>, <TR> und <TD>/<TH> wie Netscape
            // ignorieren, bei allen anderen Tags *wirklich* nicht.
            if( !rOption.GetString().isEmpty() )
            {
                rOption.GetColor( aBGColor );
                bBGColor = true;
            }
            break;
        case HTML_O_BACKGROUND:
            aBGImage = rOption.GetString();
            break;
        case HTML_O_BORDERCOLOR:
            rOption.GetColor( aBorderColor );
            bBorderColor = true;
            break;
        case HTML_O_BORDERCOLORDARK:
            if( !bBorderColor )
                rOption.GetColor( aBorderColor );
            break;
        case HTML_O_STYLE:
            aStyle = rOption.GetString();
            break;
        case HTML_O_CLASS:
            aClass = rOption.GetString();
            break;
        case HTML_O_DIR:
            aDir = rOption.GetString();
            break;
        case HTML_O_HSPACE:
            nHSpace = (sal_uInt16)rOption.GetNumber();
            break;
        case HTML_O_VSPACE:
            nVSpace = (sal_uInt16)rOption.GetNumber();
            break;
        }
    }

    if( nCols && !nWidth )
    {
        nWidth = 100;
        bPrcWidth = true;
    }

    // Wenn BORDER=0 oder kein BORDER gegeben ist, daan darf es auch
    // keine Umrandung geben
    if( 0==nBorder || USHRT_MAX==nBorder )
    {
        eFrame = HTML_TF_VOID;
        eRules = HTML_TR_NONE;
    }
}

HTMLTable *SwHTMLParser::BuildTable( SvxAdjust eParentAdjust,
                                     bool bIsParentHead,
                                     bool bHasParentSection,
                                     bool bHasToFly )
{
    if( !IsParserWorking() && !m_pPendStack )
        return nullptr;

    int nToken = 0;
    bool bPending = false;
    _TableSaveStruct* pSaveStruct;

    if( m_pPendStack )
    {
        pSaveStruct = static_cast<_TableSaveStruct*>(m_pPendStack->pData);

        SwPendingStack* pTmp = m_pPendStack->pNext;
        delete m_pPendStack;
        m_pPendStack = pTmp;
        nToken = m_pPendStack ? m_pPendStack->nToken : GetSaveToken();
        bPending = SVPAR_ERROR == eState && m_pPendStack != nullptr;

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

        pSaveStruct = new _TableSaveStruct( pCurTable );

        delete pTableOptions;

        // ist beim ersten GetNextToken schon pending, muss bei
        // wiederaufsetzen auf jedenfall neu gelesen werden!
        SaveState( 0 );
    }

    HTMLTable *pCurTable = pSaveStruct->m_pCurrentTable;

    // </TABLE> wird laut DTD benoetigt
    if( !nToken )
        nToken = GetNextToken();    // naechstes Token

    bool bDone = false;
    while( (IsParserWorking() && !bDone) || bPending )
    {
        SaveState( nToken );

        nToken = FilterToken( nToken );

        OSL_ENSURE( m_pPendStack || !m_bCallNextToken ||
                pCurTable->GetContext() || pCurTable->HasParentSection(),
                "Wo ist die Section gebieben?" );
        if( !m_pPendStack && m_bCallNextToken &&
            (pCurTable->GetContext() || pCurTable->HasParentSection()) )
        {
            // NextToken direkt aufrufen (z.B. um den Inhalt von
            // Floating-Frames oder Applets zu ignorieren)
            NextToken( nToken );
        }
        else switch( nToken )
        {
        case HTML_TABLE_ON:
            if( !pCurTable->GetContext() )
            {
                // Wenn noch keine Tabelle eingefuegt wurde,
                // die naechste Tabelle lesen
                SkipToken();
                bDone = true;
            }

            break;
        case HTML_TABLE_OFF:
            bDone = true;
            break;
        case HTML_CAPTION_ON:
            BuildTableCaption( pCurTable );
            bDone = m_pTable->IsOverflowing();
            break;
        case HTML_COL_ON:
            SkipToken();
            BuildTableColGroup( pCurTable, false );
            break;
        case HTML_COLGROUP_ON:
            BuildTableColGroup( pCurTable, true );
            break;
        case HTML_TABLEROW_ON:
        case HTML_TABLEHEADER_ON:
        case HTML_TABLEDATA_ON:
            SkipToken();
            BuildTableSection( pCurTable, false, false );
            bDone = m_pTable->IsOverflowing();
            break;
        case HTML_THEAD_ON:
        case HTML_TFOOT_ON:
        case HTML_TBODY_ON:
            BuildTableSection( pCurTable, true, HTML_THEAD_ON==nToken );
            bDone = m_pTable->IsOverflowing();
            break;
        case HTML_MULTICOL_ON:
            // spaltige Rahmen koennen wir hier leider nicht einguegen
            break;
        case HTML_FORM_ON:
            NewForm( false );   // keinen neuen Absatz aufmachen!
            break;
        case HTML_FORM_OFF:
            EndForm( false );   // keinen neuen Absatz aufmachen!
            break;
        case HTML_TEXTTOKEN:
            // Blank-Strings sind u. U. eine Folge von CR+LF und kein Text
            if( (pCurTable->GetContext() ||
                 !pCurTable->HasParentSection()) &&
                1==aToken.getLength() && ' '==aToken[0] )
                break;
        default:
            pCurTable->MakeParentContents();
            NextToken( nToken );
            break;
        }

        OSL_ENSURE( !bPending || !m_pPendStack,
                "SwHTMLParser::BuildTable: Es gibt wieder einen Pend-Stack" );
        bPending = false;
        if( IsParserWorking() )
            SaveState( 0 );

        if( !bDone )
            nToken = GetNextToken();
    }

    if( SVPAR_PENDING == GetStatus() )
    {
        m_pPendStack = new SwPendingStack( HTML_TABLE_ON, m_pPendStack );
        m_pPendStack->pData = pSaveStruct;
        return nullptr;
    }

    _HTMLTableContext *pTCntxt = pCurTable->GetContext();
    if( pTCntxt )
    {
        // Die Tabelle wurde auch angelegt

        // Tabellen-Struktur anpassen
        pCurTable->CloseTable();

        // ausserhalb von Zellen begonnene Kontexte beenden
        // muss vor(!) dem Umsetzten der Attribut Tabelle existieren,
        // weil die aktuelle danach nicht mehr existiert
        while( m_aContexts.size() > m_nContextStAttrMin )
        {
            _HTMLAttrContext *pCntxt = PopContext();
            ClearContext( pCntxt );
            delete pCntxt;
        }

        m_nContextStMin = pTCntxt->GetContextStMin();
        m_nContextStAttrMin = pTCntxt->GetContextStAttrMin();

        if( m_pTable==pCurTable )
        {
            // Tabellen-Beschriftung setzen
            const SwStartNode *pCapStNd = m_pTable->GetCaptionStartNode();
            if( pCapStNd )
            {
                // Der letzte Absatz der Section wird nie mitkopiert. Deshalb
                // muss die Section mindestens zwei Absaetze enthalten.

                if( pCapStNd->EndOfSectionIndex() - pCapStNd->GetIndex() > 2 )
                {
                    // Start-Node und letzten Absatz nicht mitkopieren.
                    SwNodeRange aSrcRg( *pCapStNd, 1,
                                    *pCapStNd->EndOfSectionNode(), -1 );

                    bool bTop = m_pTable->IsTopCaption();
                    SwStartNode *pTableStNd = pTCntxt->GetTableNode();

                    OSL_ENSURE( pTableStNd, "Wo ist der Tabellen-Node" );
                    OSL_ENSURE( pTableStNd==m_pPam->GetNode().FindTableNode(),
                            "Stehen wir in der falschen Tabelle?" );

                    SwNode* pNd;
                    if( bTop )
                        pNd = pTableStNd;
                    else
                        pNd = pTableStNd->EndOfSectionNode();
                    SwNodeIndex aDstIdx( *pNd, bTop ? 0 : 1 );

                    m_pDoc->getIDocumentContentOperations().MoveNodeRange( aSrcRg, aDstIdx,
                        SwMoveFlags::DEFAULT );

                    // Wenn die Caption vor der Tabelle eingefuegt wurde muss
                    // eine an der Tabelle gestzte Seitenvorlage noch in den
                    // ersten Absatz der Ueberschrift verschoben werden.
                    // Ausserdem muessen alle gemerkten Indizes, die auf den
                    // Tabellen-Node zeigen noch verschoben werden.
                    if( bTop )
                    {
                        MovePageDescAttrs( pTableStNd, aSrcRg.aStart.GetIndex(),
                                           false );
                    }
                }

                // Die Section wird jetzt nicht mehr gebraucht.
                m_pPam->SetMark();
                m_pPam->DeleteMark();
                m_pDoc->getIDocumentContentOperations().DeleteSection( const_cast<SwStartNode *>(pCapStNd) );
                m_pTable->SetCaption( nullptr, false );
            }

            // SwTable aufbereiten
            sal_uInt16 nBrowseWidth = (sal_uInt16)GetCurrentBrowseWidth();
            pSaveStruct->MakeTable( nBrowseWidth, *m_pPam->GetPoint(), m_pDoc );
        }

        GetNumInfo().Set( pTCntxt->GetNumInfo() );
        pTCntxt->RestorePREListingXMP( *this );
        RestoreAttrTab( pTCntxt->aAttrTab );

        if( m_pTable==pCurTable )
        {
            // oberen Absatz-Abstand einstellen
            m_bUpperSpace = true;
            SetTextCollAttrs();

            m_nParaCnt = m_nParaCnt - std::min(m_nParaCnt,
                pTCntxt->GetTableNode()->GetTable().GetTabSortBoxes().size());

            // ggfs. eine Tabelle anspringen
            if( JUMPTO_TABLE == m_eJumpTo && m_pTable->GetSwTable() &&
                m_pTable->GetSwTable()->GetFrameFormat()->GetName() == m_sJmpMark )
            {
                m_bChkJumpMark = true;
                m_eJumpTo = JUMPTO_NONE;
            }

            // Wenn Import abgebrochen wurde kein erneutes Show
            // aufrufen, weil die SwViewShell schon geloescht wurde!
            // Genuegt nicht. Auch im ACCEPTING_STATE darf
            // kein Show aufgerufen werden, weil sonst waehrend des
            // Reschedules der Parser zerstoert wird, wenn noch ein
            // DataAvailable-Link kommt. Deshalb: Nur im WORKING-State.
            if( !m_nParaCnt && SVPAR_WORKING == GetStatus() )
                Show();
        }
    }
    else if( m_pTable==pCurTable )
    {
        // Es wurde gar keine Tabelle gelesen.

        // Dann muss eine evtl gelesene Beschriftung noch geloescht werden.
        const SwStartNode *pCapStNd = pCurTable->GetCaptionStartNode();
        if( pCapStNd )
        {
            m_pPam->SetMark();
            m_pPam->DeleteMark();
            m_pDoc->getIDocumentContentOperations().DeleteSection( const_cast<SwStartNode *>(pCapStNd) );
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
