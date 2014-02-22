/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
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


static HTMLOptionEnum aHTMLTblVAlignTable[] =
{
    { OOO_STRING_SVTOOLS_HTML_VA_top,         text::VertOrientation::NONE       },
    { OOO_STRING_SVTOOLS_HTML_VA_middle,      text::VertOrientation::CENTER     },
    { OOO_STRING_SVTOOLS_HTML_VA_bottom,      text::VertOrientation::BOTTOM     },
    { 0,                    0               }
};



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

    sal_Bool bPrcWidth : 1;
    sal_Bool bTableAdjust : 1;
    sal_Bool bBGColor : 1;

    Color aBorderColor;
    Color aBGColor;

    OUString aBGImage, aStyle, aId, aClass, aDir;

    HTMLTableOptions( const HTMLOptions& rOptions, SvxAdjust eParentAdjust );
};

class _HTMLTableContext
{
    SwHTMLNumRuleInfo aNumRuleInfo; 

    SwTableNode *pTblNd;            
    SwFrmFmt *pFrmFmt;              
    SwPosition *pPos;               

    sal_uInt16 nContextStAttrMin;
    sal_uInt16 nContextStMin;

    sal_Bool    bRestartPRE : 1;
    sal_Bool    bRestartXMP : 1;
    sal_Bool    bRestartListing : 1;

public:

    _HTMLAttrTable aAttrTab;        

    _HTMLTableContext( SwPosition *pPs, sal_uInt16 nCntxtStMin,
                       sal_uInt16 nCntxtStAttrMin ) :
        pTblNd( 0 ),
        pFrmFmt( 0 ),
        pPos( pPs ),
        nContextStAttrMin( nCntxtStAttrMin ),
        nContextStMin( nCntxtStMin ),
        bRestartPRE( sal_False ),
        bRestartXMP( sal_False ),
        bRestartListing( sal_False )
    {
        memset( &aAttrTab, 0, sizeof( _HTMLAttrTable ));
    }

    ~_HTMLTableContext();

    void SetNumInfo( const SwHTMLNumRuleInfo& rInf ) { aNumRuleInfo.Set(rInf); }
    const SwHTMLNumRuleInfo& GetNumInfo() const { return aNumRuleInfo; };

    void SavePREListingXMP( SwHTMLParser& rParser );
    void RestorePREListingXMP( SwHTMLParser& rParser );

    SwPosition *GetPos() const { return pPos; }

    void SetTableNode( SwTableNode *pNd ) { pTblNd = pNd; }
    SwTableNode *GetTableNode() const { return pTblNd; }

    void SetFrmFmt( SwFrmFmt *pFmt ) { pFrmFmt = pFmt; }
    SwFrmFmt *GetFrmFmt() const { return pFrmFmt; }

    sal_uInt16 GetContextStMin() const { return nContextStMin; }
    sal_uInt16 GetContextStAttrMin() const { return nContextStAttrMin; }
};





class HTMLTableCnts
{
    HTMLTableCnts *pNext;               

    
    const SwStartNode *pStartNode;      
    HTMLTable *pTable;                  

    SwHTMLTableLayoutCnts* pLayoutInfo;

    sal_Bool bNoBreak;

    void InitCtor();

public:

    HTMLTableCnts( const SwStartNode* pStNd );
    HTMLTableCnts( HTMLTable* pTab );

    ~HTMLTableCnts();                   

    
    const SwStartNode *GetStartNode() const { return pStartNode; }
    const HTMLTable *GetTable() const { return pTable; }
    HTMLTable *GetTable() { return pTable; }

    
    void Add( HTMLTableCnts* pNewCnts );

    
    const HTMLTableCnts *Next() const { return pNext; }
    HTMLTableCnts *Next() { return pNext; }

    inline void SetTableBox( SwTableBox *pBox );

    void SetNoBreak() { bNoBreak = sal_True; }

    SwHTMLTableLayoutCnts *CreateLayoutInfo();
};



class HTMLTableCell
{
    
    
    HTMLTableCnts *pContents;       
    SvxBrushItem *pBGBrush;         
    
    ::boost::shared_ptr<SvxBoxItem> m_pBoxItem;

    sal_uInt32 nNumFmt;
    sal_uInt16 nRowSpan;                
    sal_uInt16 nColSpan;                
    sal_uInt16 nWidth;                  
    double nValue;
    sal_Int16 eVertOri;         
    sal_Bool bProtected : 1;            
    sal_Bool bRelWidth : 1;             
    sal_Bool bHasNumFmt : 1;
    sal_Bool bHasValue : 1;
    sal_Bool bNoWrap : 1;
    sal_Bool mbCovered : 1;

public:

    HTMLTableCell();                

    ~HTMLTableCell();               

    
    void Set( HTMLTableCnts *pCnts, sal_uInt16 nRSpan, sal_uInt16 nCSpan,
              sal_Int16 eVertOri, SvxBrushItem *pBGBrush,
              ::boost::shared_ptr<SvxBoxItem> const pBoxItem,
              sal_Bool bHasNumFmt, sal_uInt32 nNumFmt,
              sal_Bool bHasValue, double nValue, sal_Bool bNoWrap, sal_Bool bCovered );

    
    void SetProtected();

    
    void SetContents( HTMLTableCnts *pCnts ) { pContents = pCnts; }
    const HTMLTableCnts *GetContents() const { return pContents; }
    HTMLTableCnts *GetContents() { return pContents; }

    
    void SetRowSpan( sal_uInt16 nRSpan ) { nRowSpan = nRSpan; }
    sal_uInt16 GetRowSpan() const { return nRowSpan; }

    void SetColSpan( sal_uInt16 nCSpan ) { nColSpan = nCSpan; }
    sal_uInt16 GetColSpan() const { return nColSpan; }

    inline void SetWidth( sal_uInt16 nWidth, sal_Bool bRelWidth );

    const SvxBrushItem *GetBGBrush() const { return pBGBrush; }
    ::boost::shared_ptr<SvxBoxItem> GetBoxItem() const { return m_pBoxItem; }

    inline sal_Bool GetNumFmt( sal_uInt32& rNumFmt ) const;
    inline sal_Bool GetValue( double& rValue ) const;

    sal_Int16 GetVertOri() const { return eVertOri; }

    
    bool IsUsed() const { return pContents!=0 || bProtected; }

    SwHTMLTableLayoutCell *CreateLayoutInfo();

    sal_Bool IsCovered() const { return mbCovered; }
};



typedef boost::ptr_vector<HTMLTableCell> HTMLTableCells;

class HTMLTableRow
{
    HTMLTableCells *pCells;             

    sal_Bool bIsEndOfGroup : 1;
    sal_Bool bSplitable : 1;

    sal_uInt16 nHeight;                     
    sal_uInt16 nEmptyRows;                  

    SvxAdjust eAdjust;
    sal_Int16 eVertOri;
    SvxBrushItem *pBGBrush;             

public:

    bool bBottomBorder;                 

    HTMLTableRow( sal_uInt16 nCells=0 );    

    ~HTMLTableRow();

    inline void SetHeight( sal_uInt16 nHeight );
    sal_uInt16 GetHeight() const { return nHeight; }

    inline HTMLTableCell *GetCell( sal_uInt16 nCell ) const;
    inline const HTMLTableCells *GetCells() const { return pCells; }

    inline void SetAdjust( SvxAdjust eAdj ) { eAdjust = eAdj; }
    inline SvxAdjust GetAdjust() const { return eAdjust; }

    inline void SetVertOri( sal_Int16 eV) { eVertOri = eV; }
    inline sal_Int16 GetVertOri() const { return eVertOri; }

    void SetBGBrush( SvxBrushItem *pBrush ) { pBGBrush = pBrush; }
    const SvxBrushItem *GetBGBrush() const { return pBGBrush; }

    inline void SetEndOfGroup() { bIsEndOfGroup = sal_True; }
    inline sal_Bool IsEndOfGroup() const { return bIsEndOfGroup; }

    void IncEmptyRows() { nEmptyRows++; }
    sal_uInt16 GetEmptyRows() const { return nEmptyRows; }

    
    void Expand( sal_uInt16 nCells, sal_Bool bOneCell=sal_False );

    
    void Shrink( sal_uInt16 nCells );

    void SetSplitable( sal_Bool bSet ) { bSplitable = bSet; }
    sal_Bool IsSplitable() const { return bSplitable; }
};



class HTMLTableColumn
{
    sal_Bool bIsEndOfGroup;

    sal_uInt16 nWidth;                      
    sal_Bool bRelWidth;

    SvxAdjust eAdjust;
    sal_Int16 eVertOri;

    SwFrmFmt *aFrmFmts[6];

    inline sal_uInt16 GetFrmFmtIdx( sal_Bool bBorderLine,
                                sal_Int16 eVertOri ) const;

public:

    bool bLeftBorder;                   

    HTMLTableColumn();

    inline void SetWidth( sal_uInt16 nWidth, sal_Bool bRelWidth);

    inline void SetAdjust( SvxAdjust eAdj ) { eAdjust = eAdj; }
    inline SvxAdjust GetAdjust() const { return eAdjust; }

    inline void SetVertOri( sal_Int16 eV) { eVertOri = eV; }
    inline sal_Int16 GetVertOri() const { return eVertOri; }

    inline void SetEndOfGroup() { bIsEndOfGroup = sal_True; }
    inline sal_Bool IsEndOfGroup() const { return bIsEndOfGroup; }

    inline void SetFrmFmt( SwFrmFmt *pFmt, sal_Bool bBorderLine,
                           sal_Int16 eVertOri );
    inline SwFrmFmt *GetFrmFmt( sal_Bool bBorderLine,
                                sal_Int16 eVertOri ) const;

    SwHTMLTableLayoutColumn *CreateLayoutInfo();
};



typedef boost::ptr_vector<HTMLTableRow> HTMLTableRows;

typedef boost::ptr_vector<HTMLTableColumn> HTMLTableColumns;

typedef std::vector<SdrObject *> SdrObjects;

class HTMLTable
{
    OUString aId;
    OUString aStyle;
    OUString aClass;
    OUString aDir;

    SdrObjects *pResizeDrawObjs;
    std::vector<sal_uInt16> *pDrawObjPrcWidths;   

    HTMLTableRows *pRows;           
    HTMLTableColumns *pColumns;     

    sal_uInt16 nRows;                   
    sal_uInt16 nCols;                   
    sal_uInt16 nFilledCols;             

    sal_uInt16 nCurRow;                 
    sal_uInt16 nCurCol;                 

    sal_uInt16 nLeftMargin;             
    sal_uInt16 nRightMargin;            

    sal_uInt16 nCellPadding;            
    sal_uInt16 nCellSpacing;            
    sal_uInt16 nHSpace;
    sal_uInt16 nVSpace;

    sal_uInt16 nBoxes;                  

    const SwStartNode *pPrevStNd;   
                                    
    const SwTable *pSwTable;        
    SwTableBox *pBox1;              
                                    

    SwTableBoxFmt *pBoxFmt;         
    SwTableLineFmt *pLineFmt;       
    SwTableLineFmt *pLineFrmFmtNoHeight;
    SvxBrushItem *pBGBrush;         
    SvxBrushItem *pInhBGBrush;      
    const SwStartNode *pCaptionStartNode;   

    SvxBorderLine aTopBorderLine;   
    SvxBorderLine aBottomBorderLine;
    SvxBorderLine aLeftBorderLine;  
    SvxBorderLine aRightBorderLine; 
    SvxBorderLine aBorderLine;      
    SvxBorderLine aInhLeftBorderLine;   
    SvxBorderLine aInhRightBorderLine;  
    bool bTopBorder;                
    bool bRightBorder;              
    bool bTopAlwd;                  
    bool bRightAlwd;
    bool bFillerTopBorder;          
    bool bFillerBottomBorder;       
    bool bInhLeftBorder;
    bool bInhRightBorder;
    bool bBordersSet;               
    sal_Bool bForceFrame;
    sal_Bool bTableAdjustOfTag;         
    sal_uInt32 nHeadlineRepeat;         
    sal_Bool bIsParentHead;
    sal_Bool bHasParentSection;
    sal_Bool bMakeTopSubTable;
    sal_Bool bHasToFly;
    sal_Bool bFixedCols;
    bool bColSpec;                  
    sal_Bool bPrcWidth;                 

    SwHTMLParser *pParser;          
    HTMLTable *pTopTable;           
    HTMLTableCnts *pParentContents;

    _HTMLTableContext *pContext;    

    SwHTMLTableLayout *pLayoutInfo;


    
    sal_uInt16 nWidth;                  
    sal_uInt16 nHeight;                 
    SvxAdjust eTableAdjust;         
    sal_Int16 eVertOri;         
    sal_uInt16 nBorder;                 
    HTMLTableFrame eFrame;          
    HTMLTableRules eRules;          
    sal_Bool bTopCaption;               

    void InitCtor( const HTMLTableOptions *pOptions );

    
    
    
    void FixRowSpan( sal_uInt16 nRow, sal_uInt16 nCol, const HTMLTableCnts *pCnts );

    
    void ProtectRowSpan( sal_uInt16 nRow, sal_uInt16 nCol, sal_uInt16 nRowSpan );

    
    
    
    const SwStartNode* GetPrevBoxStartNode( sal_uInt16 nRow, sal_uInt16 nCell ) const;

    sal_uInt16 GetTopCellSpace( sal_uInt16 nRow, sal_uInt16 nRowSpan,
                            sal_Bool bSwBorders=sal_True ) const;
    sal_uInt16 GetBottomCellSpace( sal_uInt16 nRow, sal_uInt16 nRowSpan,
                               sal_Bool bSwBorders=sal_True ) const;

    
    void FixFrameFmt( SwTableBox *pBox, sal_uInt16 nRow, sal_uInt16 nCol,
                      sal_uInt16 nRowSpan, sal_uInt16 nColSpan,
                      sal_Bool bFirstPara=sal_True, sal_Bool bLastPara=sal_True ) const;
    void FixFillerFrameFmt( SwTableBox *pBox, sal_Bool bRight ) const;

    
    void _MakeTable( SwTableBox *pUpper=0 );

    
    SwTableBox *NewTableBox( const SwStartNode *pStNd,
                             SwTableLine *pUpper ) const;

    
    
    SwTableLine *MakeTableLine( SwTableBox *pUpper,
                                sal_uInt16 nTopRow, sal_uInt16 nLeftCol,
                                sal_uInt16 nBottomRow, sal_uInt16 nRightCol );

    
    SwTableBox *MakeTableBox( SwTableLine *pUpper,
                              HTMLTableCnts *pCnts,
                              sal_uInt16 nTopRow, sal_uInt16 nLeftCol,
                              sal_uInt16 nBootomRow, sal_uInt16 nRightCol );

    

    
    void InheritBorders( const HTMLTable *pParent,
                         sal_uInt16 nRow, sal_uInt16 nCol,
                         sal_uInt16 nRowSpan, sal_uInt16 nColSpan,
                         sal_Bool bFirstPara, sal_Bool bLastPara );

    
    void InheritVertBorders( const HTMLTable *pParent,
                             sal_uInt16 nCol, sal_uInt16 nColSpan );


    
    void SetBorders();

    
    bool BordersSet() const { return bBordersSet; }

    const SvxBrushItem *GetBGBrush() const { return pBGBrush; }
    const SvxBrushItem *GetInhBGBrush() const { return pInhBGBrush; }

    sal_uInt16 GetBorderWidth( const SvxBorderLine& rBLine,
                           sal_Bool bWithDistance=sal_False ) const;

public:

    sal_Bool bFirstCell;                

    HTMLTable( SwHTMLParser* pPars, HTMLTable *pTopTab,
               sal_Bool bParHead, sal_Bool bHasParentSec,
               sal_Bool bTopTbl, sal_Bool bHasToFly,
               const HTMLTableOptions *pOptions );

    ~HTMLTable();

    
    inline HTMLTableCell *GetCell( sal_uInt16 nRow, sal_uInt16 nCell ) const;

    
    inline void SetCaption( const SwStartNode *pStNd, sal_Bool bTop );
    const SwStartNode *GetCaptionStartNode() const { return pCaptionStartNode; }
    sal_Bool IsTopCaption() const { return bTopCaption; }

    SvxAdjust GetTableAdjust( sal_Bool bAny ) const
    {
        return (bTableAdjustOfTag || bAny) ? eTableAdjust : SVX_ADJUST_END;
    }
    sal_Int16 GetVertOri() const { return eVertOri; }

    sal_uInt16 GetHSpace() const { return nHSpace; }
    sal_uInt16 GetVSpace() const { return nVSpace; }

    sal_Bool HasPrcWidth() const { return bPrcWidth; }
    sal_uInt8 GetPrcWidth() const { return bPrcWidth ? (sal_uInt8)nWidth : 0; }

    sal_uInt16 GetMinWidth() const
    {
        sal_uInt32 nMin = pLayoutInfo->GetMin();
        return nMin < USHRT_MAX ? (sal_uInt16)nMin : USHRT_MAX;
    }

    
    SvxAdjust GetInheritedAdjust() const;
    sal_Int16 GetInheritedVertOri() const;

    
    void InsertCell( HTMLTableCnts *pCnts, sal_uInt16 nRowSpan, sal_uInt16 nColSpan,
                     sal_uInt16 nWidth, sal_Bool bRelWidth, sal_uInt16 nHeight,
                     sal_Int16 eVertOri, SvxBrushItem *pBGBrush,
                     boost::shared_ptr<SvxBoxItem> const pBoxItem,
                     sal_Bool bHasNumFmt, sal_uInt32 nNumFmt,
                     sal_Bool bHasValue, double nValue, sal_Bool bNoWrap );

    
    void OpenRow( SvxAdjust eAdjust, sal_Int16 eVertOri,
                  SvxBrushItem *pBGBrush );
    void CloseRow( sal_Bool bEmpty );

    
    inline void CloseSection( sal_Bool bHead );

    
    inline void CloseColGroup( sal_uInt16 nSpan, sal_uInt16 nWidth, bool bRelWidth,
                               SvxAdjust eAdjust, sal_Int16 eVertOri );

    
    void InsertCol( sal_uInt16 nSpan, sal_uInt16 nWidth, bool bRelWidth,
                    SvxAdjust eAdjust, sal_Int16 eVertOri );

    
    void CloseTable();

    
    void MakeTable( SwTableBox *pUpper, sal_uInt16 nAbsAvail,
                    sal_uInt16 nRelAvail=0, sal_uInt16 nAbsLeftSpace=0,
                    sal_uInt16 nAbsRightSpace=0, sal_uInt16 nInhAbsSpace=0 );

    inline sal_Bool IsNewDoc() const { return pParser->IsNewDoc(); }

    void SetHasParentSection( sal_Bool bSet ) { bHasParentSection = bSet; }
    sal_Bool HasParentSection() const { return bHasParentSection; }

    void SetParentContents( HTMLTableCnts *pCnts ) { pParentContents = pCnts; }
    HTMLTableCnts *GetParentContents() const { return pParentContents; }

    void MakeParentContents();

    sal_Bool GetIsParentHeader() const { return bIsParentHead; }

    sal_Bool IsMakeTopSubTable() const { return bMakeTopSubTable; }
    void SetHasToFly() { bHasToFly=sal_True; }
    sal_Bool HasToFly() const { return bHasToFly; }

    void SetTable( const SwStartNode *pStNd, _HTMLTableContext *pCntxt,
                   sal_uInt16 nLeft, sal_uInt16 nRight,
                   const SwTable *pSwTab=0, sal_Bool bFrcFrame=sal_False );

    _HTMLTableContext *GetContext() const { return pContext; }

    SwHTMLTableLayout *CreateLayoutInfo();

    bool HasColTags() const { return bColSpec; }

    sal_uInt16 IncGrfsThatResize() { return pSwTable ? ((SwTable *)pSwTable)->IncGrfsThatResize() : 0; }

    void RegisterDrawObject( SdrObject *pObj, sal_uInt8 nPrcWidth );

    const SwTable *GetSwTable() const { return pSwTable; }

    void SetBGBrush( const SvxBrushItem& rBrush ) { delete pBGBrush; pBGBrush = new SvxBrushItem( rBrush ); }

    const OUString& GetId() const { return aId; }
    const OUString& GetClass() const { return aClass; }
    const OUString& GetStyle() const { return aStyle; }
    const OUString& GetDirection() const { return aDir; }

    void IncBoxCount() { nBoxes++; }
    sal_Bool IsOverflowing() const { return nBoxes > 64000; }
};



void HTMLTableCnts::InitCtor()
{
    pNext = 0;
    pLayoutInfo = 0;

    bNoBreak = sal_False;
}

HTMLTableCnts::HTMLTableCnts( const SwStartNode* pStNd ):
    pStartNode(pStNd), pTable(0)
{
    InitCtor();
}

HTMLTableCnts::HTMLTableCnts( HTMLTable* pTab ):
    pStartNode(0), pTable(pTab)
{
    InitCtor();
}

HTMLTableCnts::~HTMLTableCnts()
{
    delete pTable;              
    delete pNext;
}

void HTMLTableCnts::Add( HTMLTableCnts* pNewCnts )
{
    HTMLTableCnts *pCnts = this;

    while( pCnts->pNext )
        pCnts = pCnts->pNext;

    pCnts->pNext = pNewCnts;
}

inline void HTMLTableCnts::SetTableBox( SwTableBox *pBox )
{
    OSL_ENSURE( pLayoutInfo, "Da sit noch keine Layout-Info" );
    if( pLayoutInfo )
        pLayoutInfo->SetTableBox( pBox );
}

SwHTMLTableLayoutCnts *HTMLTableCnts::CreateLayoutInfo()
{
    if( !pLayoutInfo )
    {
        SwHTMLTableLayoutCnts *pNextInfo = pNext ? pNext->CreateLayoutInfo() : 0;
        SwHTMLTableLayout *pTableInfo = pTable ? pTable->CreateLayoutInfo() : 0;

        pLayoutInfo = new SwHTMLTableLayoutCnts( pStartNode, pTableInfo,
                                                 bNoBreak, pNextInfo );
    }

    return pLayoutInfo;
}


HTMLTableCell::HTMLTableCell():
    pContents(0),
    pBGBrush(0),
    nNumFmt(0),
    nRowSpan(1),
    nColSpan(1),
    nWidth( 0 ),
    nValue(0),
    eVertOri( text::VertOrientation::NONE ),
    bProtected(sal_False),
    bRelWidth( sal_False ),
    bHasNumFmt(sal_False),
    bHasValue(sal_False),
    bNoWrap(sal_False),
    mbCovered(sal_False)
{}

HTMLTableCell::~HTMLTableCell()
{
    
    
    if( 1==nRowSpan && 1==nColSpan )
    {
        delete pContents;
        delete pBGBrush;
    }
}

void HTMLTableCell::Set( HTMLTableCnts *pCnts, sal_uInt16 nRSpan, sal_uInt16 nCSpan,
                         sal_Int16 eVert, SvxBrushItem *pBrush,
                         ::boost::shared_ptr<SvxBoxItem> const pBoxItem,
                         sal_Bool bHasNF, sal_uInt32 nNF, sal_Bool bHasV, double nVal,
                         sal_Bool bNWrap, sal_Bool bCovered )
{
    pContents = pCnts;
    nRowSpan = nRSpan;
    nColSpan = nCSpan;
    bProtected = sal_False;
    eVertOri = eVert;
    pBGBrush = pBrush;
    m_pBoxItem = pBoxItem;

    bHasNumFmt = bHasNF;
    bHasValue = bHasV;
    nNumFmt = nNF;
    nValue = nVal;

    bNoWrap = bNWrap;
    mbCovered = bCovered;
}

inline void HTMLTableCell::SetWidth( sal_uInt16 nWdth, sal_Bool bRelWdth )
{
    nWidth = nWdth;
    bRelWidth = bRelWdth;
}

void HTMLTableCell::SetProtected()
{
    
    

    
    pContents = 0;

    
    if( pBGBrush )
        pBGBrush = new SvxBrushItem( *pBGBrush );

    nRowSpan = 1;
    nColSpan = 1;
    bProtected = sal_True;
}

inline sal_Bool HTMLTableCell::GetNumFmt( sal_uInt32& rNumFmt ) const
{
    rNumFmt = nNumFmt;
    return bHasNumFmt;
}

inline sal_Bool HTMLTableCell::GetValue( double& rValue ) const
{
    rValue = nValue;
    return bHasValue;
}

SwHTMLTableLayoutCell *HTMLTableCell::CreateLayoutInfo()
{
    SwHTMLTableLayoutCnts *pCntInfo = pContents ? pContents->CreateLayoutInfo() : 0;

    return new SwHTMLTableLayoutCell( pCntInfo, nRowSpan, nColSpan, nWidth,
                                      bRelWidth, bNoWrap );
}


HTMLTableRow::HTMLTableRow( sal_uInt16 nCells ):
    pCells(new HTMLTableCells),
    bIsEndOfGroup(sal_False),
    bSplitable( sal_False ),
    nHeight(0),
    nEmptyRows(0),
    eAdjust(SVX_ADJUST_END),
    eVertOri(text::VertOrientation::TOP),
    pBGBrush(0),
    bBottomBorder(false)
{
    for( sal_uInt16 i=0; i<nCells; i++ )
    {
        pCells->push_back( new HTMLTableCell );
    }

    OSL_ENSURE(nCells == pCells->size(),
            "wrong Cell count in new HTML table row");
}

HTMLTableRow::~HTMLTableRow()
{
    delete pCells;
    delete pBGBrush;
}

inline void HTMLTableRow::SetHeight( sal_uInt16 nHght )
{
    if( nHght > nHeight  )
        nHeight = nHght;
}

inline HTMLTableCell *HTMLTableRow::GetCell( sal_uInt16 nCell ) const
{
    OSL_ENSURE( nCell<pCells->size(),
        "ungueltiger Zellen-Index in HTML-Tabellenzeile" );
    return &(*pCells)[nCell];
}

void HTMLTableRow::Expand( sal_uInt16 nCells, sal_Bool bOneCell )
{
    
    
    

    sal_uInt16 nColSpan = nCells-pCells->size();
    for( sal_uInt16 i=pCells->size(); i<nCells; i++ )
    {
        HTMLTableCell *pCell = new HTMLTableCell;
        if( bOneCell )
            pCell->SetColSpan( nColSpan );

        pCells->push_back( pCell );
        nColSpan--;
    }

    OSL_ENSURE(nCells == pCells->size(),
            "wrong Cell count in expanded HTML table row");
}

void HTMLTableRow::Shrink( sal_uInt16 nCells )
{
    OSL_ENSURE(nCells < pCells->size(), "number of cells too large");

#if OSL_DEBUG_LEVEL > 0
     sal_uInt16 nEnd = pCells->size();
#endif
    
    
    sal_uInt16 i=nCells;
    while( i )
    {
        HTMLTableCell *pCell = &(*pCells)[--i];
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
        HTMLTableCell *pCell = &(*pCells)[i];
        OSL_ENSURE( pCell->GetRowSpan() == 1,
                "RowSpan von zu loesender Zelle ist falsch" );
        OSL_ENSURE( pCell->GetColSpan() == nEnd - i,
                    "ColSpan von zu loesender Zelle ist falsch" );
        OSL_ENSURE( !pCell->GetContents(), "Zu loeschende Zelle hat Inhalt" );
    }
#endif

    pCells->erase( pCells->begin() + nCells, pCells->end() );
}


HTMLTableColumn::HTMLTableColumn():
    bIsEndOfGroup(sal_False),
    nWidth(0), bRelWidth(sal_False),
    eAdjust(SVX_ADJUST_END), eVertOri(text::VertOrientation::TOP),
    bLeftBorder(false)
{
    for( sal_uInt16 i=0; i<6; i++ )
        aFrmFmts[i] = 0;
}

inline void HTMLTableColumn::SetWidth( sal_uInt16 nWdth, sal_Bool bRelWdth )
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

inline sal_uInt16 HTMLTableColumn::GetFrmFmtIdx( sal_Bool bBorderLine,
                                             sal_Int16 eVertOrient ) const
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

inline void HTMLTableColumn::SetFrmFmt( SwFrmFmt *pFmt, sal_Bool bBorderLine,
                                        sal_Int16 eVertOrient )
{
    aFrmFmts[GetFrmFmtIdx(bBorderLine,eVertOrient)] = pFmt;
}

inline SwFrmFmt *HTMLTableColumn::GetFrmFmt( sal_Bool bBorderLine,
                                             sal_Int16 eVertOrient ) const
{
    return aFrmFmts[GetFrmFmtIdx(bBorderLine,eVertOrient)];
}



void HTMLTable::InitCtor( const HTMLTableOptions *pOptions )
{
    pResizeDrawObjs = 0;
    pDrawObjPrcWidths = 0;

    pRows = new HTMLTableRows;
    pColumns = new HTMLTableColumns;
    nRows = 0;
    nCurRow = 0; nCurCol = 0;

    pBox1 = 0;
    pBoxFmt = 0; pLineFmt = 0;
    pLineFrmFmtNoHeight = 0;
    pInhBGBrush = 0;

    pPrevStNd = 0;
    pSwTable = 0;

    bTopBorder = false; bRightBorder = false;
    bTopAlwd = true; bRightAlwd = true;
    bFillerTopBorder = false; bFillerBottomBorder = false;
    bInhLeftBorder = false; bInhRightBorder = false;
    bBordersSet = false;
    bForceFrame = sal_False;
    nHeadlineRepeat = 0;

    nLeftMargin = 0;
    nRightMargin = 0;

    const Color& rBorderColor = pOptions->aBorderColor;

    long nBorderOpt = (long)pOptions->nBorder;
    long nPWidth = nBorderOpt==USHRT_MAX ? NETSCAPE_DFLT_BORDER
                                         : nBorderOpt;
    long nPHeight = nBorderOpt==USHRT_MAX ? 0 : nBorderOpt;
    SvxCSS1Parser::PixelToTwip( nPWidth, nPHeight );

    
    
    
    
    nBorder = (sal_uInt16)nPWidth;
    if( nBorderOpt==USHRT_MAX )
        nPWidth = 0;

    
    
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
            nCellPadding = MIN_BORDER_DIST; 
        else
        {
            nCellPadding = pParser->ToTwips( nCellPadding );
            if( nCellPadding<MIN_BORDER_DIST  )
                nCellPadding = MIN_BORDER_DIST;
        }
    }
    if( nCellSpacing )
    {
        if( nCellSpacing==USHRT_MAX )
            nCellSpacing = NETSCAPE_DFLT_CELLSPACING;
        nCellSpacing = pParser->ToTwips( nCellSpacing );
    }

    nPWidth = pOptions->nHSpace;
    nPHeight = pOptions->nVSpace;
    SvxCSS1Parser::PixelToTwip( nPWidth, nPHeight );
    nHSpace = (sal_uInt16)nPWidth;
    nVSpace = (sal_uInt16)nPHeight;

    bColSpec = false;

    pBGBrush = pParser->CreateBrushItem(
                    pOptions->bBGColor ? &(pOptions->aBGColor) : 0,
                    pOptions->aBGImage, aEmptyOUStr, aEmptyOUStr, aEmptyOUStr );

    pContext = 0;
    pParentContents = 0;

    aId = pOptions->aId;
    aClass = pOptions->aClass;
    aStyle = pOptions->aStyle;
    aDir = pOptions->aDir;
}

HTMLTable::HTMLTable( SwHTMLParser* pPars, HTMLTable *pTopTab,
                      sal_Bool bParHead,
                      sal_Bool bHasParentSec, sal_Bool bTopTbl, sal_Bool bHasToFlw,
                      const HTMLTableOptions *pOptions ) :
    nCols( pOptions->nCols ),
    nFilledCols( 0 ),
    nCellPadding( pOptions->nCellPadding ),
    nCellSpacing( pOptions->nCellSpacing ),
    nBoxes( 1 ),
    pCaptionStartNode( 0 ),
    bTableAdjustOfTag( pTopTab ? sal_False : pOptions->bTableAdjust ),
    bIsParentHead( bParHead ),
    bHasParentSection( bHasParentSec ),
    bMakeTopSubTable( bTopTbl ),
    bHasToFly( bHasToFlw ),
    bFixedCols( pOptions->nCols>0 ),
    bPrcWidth( pOptions->bPrcWidth ),
    pParser( pPars ),
    pTopTable( pTopTab ? pTopTab : this ),
    pLayoutInfo( 0 ),
    nWidth( pOptions->nWidth ),
    nHeight( pTopTab ? 0 : pOptions->nHeight ),
    eTableAdjust( pOptions->eAdjust ),
    eVertOri( pOptions->eVertOri ),
    eFrame( pOptions->eFrame ),
    eRules( pOptions->eRules ),
    bTopCaption( sal_False ),
    bFirstCell( !pTopTab )
{
    InitCtor( pOptions );

    for( sal_uInt16 i=0; i<nCols; i++ )
        pColumns->push_back( new HTMLTableColumn );
}


HTMLTable::~HTMLTable()
{
    delete pResizeDrawObjs;
    delete pDrawObjPrcWidths;

    delete pRows;
    delete pColumns;
    delete pBGBrush;
    delete pInhBGBrush;

    delete pContext;

    
    
}

SwHTMLTableLayout *HTMLTable::CreateLayoutInfo()
{
    sal_uInt16 nW = bPrcWidth ? nWidth : pParser->ToTwips( nWidth );

    sal_uInt16 nBorderWidth = GetBorderWidth( aBorderLine, sal_True );
    sal_uInt16 nLeftBorderWidth =
        ((*pColumns)[0]).bLeftBorder ? GetBorderWidth( aLeftBorderLine, sal_True ) : 0;
    sal_uInt16 nRightBorderWidth =
        bRightBorder ? GetBorderWidth( aRightBorderLine, sal_True ) : 0;
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

    sal_Bool bExportable = sal_True;
    sal_uInt16 i;
    for( i=0; i<nRows; i++ )
    {
        HTMLTableRow *pRow = &(*pRows)[i];
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
        pLayoutInfo->SetColumn( ((*pColumns)[i]).CreateLayoutInfo(), i );

    return pLayoutInfo;
}

inline void HTMLTable::SetCaption( const SwStartNode *pStNd, sal_Bool bTop )
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



const SwStartNode* HTMLTable::GetPrevBoxStartNode( sal_uInt16 nRow, sal_uInt16 nCol ) const
{
    const HTMLTableCnts *pPrevCnts = 0;

    if( 0==nRow )
    {
        
        if( nCol>0 )
            pPrevCnts = GetCell( 0, nCol-1 )->GetContents();
        else
            return pPrevStNd;
    }
    else if( USHRT_MAX==nRow && USHRT_MAX==nCol )
        
        pPrevCnts = GetCell( nRows-1, nCols-1 )->GetContents();
    else
    {
        sal_uInt16 i;
        HTMLTableRow *pPrevRow = &(*pRows)[nRow-1];

        
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


static sal_Bool IsBoxEmpty( const SwTableBox *pBox )
{
    const SwStartNode *pSttNd = pBox->GetSttNd();
    if( pSttNd &&
        pSttNd->GetIndex() + 2 == pSttNd->EndOfSectionIndex() )
    {
        const SwCntntNode *pCNd =
            pSttNd->GetNodes()[pSttNd->GetIndex()+1]->GetCntntNode();
        if( pCNd && !pCNd->Len() )
            return sal_True;
    }

    return sal_False;
}

sal_uInt16 HTMLTable::GetTopCellSpace( sal_uInt16 nRow, sal_uInt16 nRowSpan,
                                   sal_Bool bSwBorders ) const
{
    sal_uInt16 nSpace = nCellPadding;

    if( nRow == 0 )
    {
        nSpace += nBorder + nCellSpacing;
        if( bSwBorders )
        {
            sal_uInt16 nTopBorderWidth =
                GetBorderWidth( aTopBorderLine, sal_True );
            if( nSpace < nTopBorderWidth )
                nSpace = nTopBorderWidth;
        }
    }
    else if( bSwBorders && (*pRows)[nRow+nRowSpan-1].bBottomBorder &&
             nSpace < MIN_BORDER_DIST )
    {
        OSL_ENSURE( !nCellPadding, "GetTopCellSpace: CELLPADDING!=0" );
        
        
        
        
        nSpace = MIN_BORDER_DIST;
    }

    return nSpace;
}

sal_uInt16 HTMLTable::GetBottomCellSpace( sal_uInt16 nRow, sal_uInt16 nRowSpan,
                                      sal_Bool bSwBorders ) const
{
    sal_uInt16 nSpace = nCellSpacing + nCellPadding;

    if( nRow+nRowSpan == nRows )
    {
        nSpace = nSpace + nBorder;

        if( bSwBorders )
        {
            sal_uInt16 nBottomBorderWidth =
                GetBorderWidth( aBottomBorderLine, sal_True );
            if( nSpace < nBottomBorderWidth )
                nSpace = nBottomBorderWidth;
        }
    }
    else if( bSwBorders )
    {
        if( (*pRows)[nRow+nRowSpan+1].bBottomBorder )
        {
            sal_uInt16 nBorderWidth = GetBorderWidth( aBorderLine, sal_True );
            if( nSpace < nBorderWidth )
                nSpace = nBorderWidth;
        }
        else if( nRow==0 && bTopBorder && nSpace < MIN_BORDER_DIST )
        {
            OSL_ENSURE( GetBorderWidth( aTopBorderLine, sal_True ) > 0,
                    "GetBottomCellSpace: |aTopLine| == 0" );
            OSL_ENSURE( !nCellPadding, "GetBottomCellSpace: CELLPADDING!=0" );
            
            
            
            
            nSpace = MIN_BORDER_DIST;
        }
    }

    return nSpace;
}

void HTMLTable::FixFrameFmt( SwTableBox *pBox,
                             sal_uInt16 nRow, sal_uInt16 nCol,
                             sal_uInt16 nRowSpan, sal_uInt16 nColSpan,
                             sal_Bool bFirstPara, sal_Bool bLastPara ) const
{
    SwFrmFmt *pFrmFmt = 0;      
    sal_Int16 eVOri = text::VertOrientation::NONE;
    const SvxBrushItem *pBGBrushItem = 0;   
    boost::shared_ptr<SvxBoxItem> pBoxItem;
    sal_Bool bTopLine = sal_False, bBottomLine = sal_False, bLastBottomLine = sal_False;
    sal_Bool bReUsable = sal_False;     
    sal_uInt16 nEmptyRows = 0;
    sal_Bool bHasNumFmt = sal_False;
    sal_Bool bHasValue = sal_False;
    sal_uInt32 nNumFmt = 0;
    double nValue = 0.0;

    HTMLTableColumn *pColumn = &(*pColumns)[nCol];

    if( pBox->GetSttNd() )
    {
        
        const HTMLTableCell *pCell = GetCell( nRow, nCol );
        pBoxItem = pCell->GetBoxItem();
        pBGBrushItem = pCell->GetBGBrush();
        if( !pBGBrushItem )
        {
            
            
            
            
            
            
            
            if( nRowSpan > 1 || (this != pTopTable && nRowSpan==nRows) )
            {
                pBGBrushItem = (*pRows)[nRow].GetBGBrush();
                if( !pBGBrushItem && this != pTopTable )
                {
                    pBGBrushItem = GetBGBrush();
                    if( !pBGBrushItem )
                        pBGBrushItem = GetInhBGBrush();
                }
            }
        }

        bTopLine = 0==nRow && bTopBorder && bFirstPara;
        if( (*pRows)[nRow+nRowSpan-1].bBottomBorder && bLastPara )
        {
            nEmptyRows = (*pRows)[nRow+nRowSpan-1].GetEmptyRows();
            if( nRow+nRowSpan == nRows )
                bLastBottomLine = sal_True;
            else
                bBottomLine = sal_True;
        }

        eVOri = pCell->GetVertOri();
        bHasNumFmt = pCell->GetNumFmt( nNumFmt );
        if( bHasNumFmt )
            bHasValue = pCell->GetValue( nValue );

        if( nColSpan==1 && !bTopLine && !bLastBottomLine && !nEmptyRows &&
            !pBGBrushItem && !bHasNumFmt && !pBoxItem)
        {
            pFrmFmt = pColumn->GetFrmFmt( bBottomLine, eVOri );
            bReUsable = !pFrmFmt;
        }
    }

    if( !pFrmFmt )
    {
        pFrmFmt = pBox->ClaimFrmFmt();

        
        SwTwips nFrmWidth = (SwTwips)pLayoutInfo->GetColumn(nCol)
                                                ->GetRelColWidth();
        for( sal_uInt16 i=1; i<nColSpan; i++ )
            nFrmWidth += (SwTwips)pLayoutInfo->GetColumn(nCol+i)
                                             ->GetRelColWidth();

        
        
        
        if( pBox->GetSttNd() )
        {
            sal_Bool bSet = (nCellPadding > 0);

            SvxBoxItem aBoxItem( RES_BOX );
            long nInnerFrmWidth = nFrmWidth;

            if( bTopLine )
            {
                aBoxItem.SetLine( &aTopBorderLine, BOX_LINE_TOP );
                bSet = sal_True;
            }
            if( bLastBottomLine )
            {
                aBoxItem.SetLine( &aBottomBorderLine, BOX_LINE_BOTTOM );
                bSet = sal_True;
            }
            else if( bBottomLine )
            {
                if( nEmptyRows && !aBorderLine.GetInWidth() )
                {
                    
                    
                    
                    SvxBorderLine aThickBorderLine( aBorderLine );

                    sal_uInt16 nBorderWidth = aBorderLine.GetOutWidth();
                    nBorderWidth *= (nEmptyRows + 1);
                    aThickBorderLine.SetBorderLineStyle(
                            table::BorderLineStyle::SOLID);
                    aThickBorderLine.SetWidth( nBorderWidth );
                    aBoxItem.SetLine( &aThickBorderLine, BOX_LINE_BOTTOM );
                }
                else
                {
                    aBoxItem.SetLine( &aBorderLine, BOX_LINE_BOTTOM );
                }
                bSet = sal_True;
            }
            if( ((*pColumns)[nCol]).bLeftBorder )
            {
                const SvxBorderLine& rBorderLine =
                    0==nCol ? aLeftBorderLine : aBorderLine;
                aBoxItem.SetLine( &rBorderLine, BOX_LINE_LEFT );
                nInnerFrmWidth -= GetBorderWidth( rBorderLine );
                bSet = sal_True;
            }
            if( nCol+nColSpan == nCols && bRightBorder )
            {
                aBoxItem.SetLine( &aRightBorderLine, BOX_LINE_RIGHT );
                nInnerFrmWidth -= GetBorderWidth( aRightBorderLine );
                bSet = sal_True;
            }

            if (pBoxItem)
            {
                pFrmFmt->SetFmtAttr( *pBoxItem );
            }
            else if (bSet)
            {
                
                sal_uInt16 nBDist = static_cast< sal_uInt16 >(
                    (2*nCellPadding <= nInnerFrmWidth) ? nCellPadding
                                                      : (nInnerFrmWidth / 2) );
                
                
                
                
                aBoxItem.SetDistance( nBDist ? nBDist : MIN_BORDER_DIST );
                pFrmFmt->SetFmtAttr( aBoxItem );
            }
            else
                pFrmFmt->ResetFmtAttr( RES_BOX );

            if( pBGBrushItem )
            {
                pFrmFmt->SetFmtAttr( *pBGBrushItem );
            }
            else
                pFrmFmt->ResetFmtAttr( RES_BACKGROUND );

            
            if( bHasNumFmt && (bHasValue || IsBoxEmpty(pBox)) )
            {
                sal_Bool bLock = pFrmFmt->GetDoc()->GetNumberFormatter()
                                     ->IsTextFormat( nNumFmt );
                SfxItemSet aItemSet( *pFrmFmt->GetAttrSet().GetPool(),
                                     RES_BOXATR_FORMAT, RES_BOXATR_VALUE );
                SvxAdjust eAdjust = SVX_ADJUST_END;
                SwCntntNode *pCNd = 0;
                if( !bLock )
                {
                    const SwStartNode *pSttNd = pBox->GetSttNd();
                    pCNd = pSttNd->GetNodes()[pSttNd->GetIndex()+1]
                                 ->GetCntntNode();
                    const SfxPoolItem *pItem;
                    if( pCNd && pCNd->HasSwAttrSet() &&
                        SFX_ITEM_SET==pCNd->GetpSwAttrSet()->GetItemState(
                            RES_PARATR_ADJUST, false, &pItem ) )
                    {
                        eAdjust = ((const SvxAdjustItem *)pItem)
                            ->GetAdjust();
                    }
                }
                aItemSet.Put( SwTblBoxNumFormat(nNumFmt) );
                if( bHasValue )
                    aItemSet.Put( SwTblBoxValue(nValue) );

                if( bLock )
                    pFrmFmt->LockModify();
                pFrmFmt->SetFmtAttr( aItemSet );
                if( bLock )
                    pFrmFmt->UnlockModify();
                else if( pCNd && SVX_ADJUST_END != eAdjust )
                {
                    SvxAdjustItem aAdjItem( eAdjust, RES_PARATR_ADJUST );
                    pCNd->SetAttr( aAdjItem );
                }
            }
            else
                pFrmFmt->ResetFmtAttr( RES_BOXATR_FORMAT );

            OSL_ENSURE( eVOri != text::VertOrientation::TOP, "text::VertOrientation::TOP ist nicht erlaubt!" );
            if( text::VertOrientation::NONE != eVOri )
            {
                pFrmFmt->SetFmtAttr( SwFmtVertOrient( 0, eVOri ) );
            }
            else
                pFrmFmt->ResetFmtAttr( RES_VERT_ORIENT );

            if( bReUsable )
                pColumn->SetFrmFmt( pFrmFmt, bBottomLine, eVOri );
        }
        else
        {
            pFrmFmt->ResetFmtAttr( RES_BOX );
            pFrmFmt->ResetFmtAttr( RES_BACKGROUND );
            pFrmFmt->ResetFmtAttr( RES_VERT_ORIENT );
            pFrmFmt->ResetFmtAttr( RES_BOXATR_FORMAT );
        }
    }
    else
    {
        OSL_ENSURE( pBox->GetSttNd() ||
                SFX_ITEM_SET!=pFrmFmt->GetAttrSet().GetItemState(
                                    RES_VERT_ORIENT, false ),
                "Box ohne Inhalt hat vertikale Ausrichtung" );
        pBox->ChgFrmFmt( (SwTableBoxFmt*)pFrmFmt );
    }

}

void HTMLTable::FixFillerFrameFmt( SwTableBox *pBox, sal_Bool bRight ) const
{
    SwFrmFmt *pFrmFmt = pBox->ClaimFrmFmt();

    if( bFillerTopBorder || bFillerBottomBorder ||
        (!bRight && bInhLeftBorder) || (bRight && bInhRightBorder) )
    {
        SvxBoxItem aBoxItem( RES_BOX );
        if( bFillerTopBorder )
            aBoxItem.SetLine( &aTopBorderLine, BOX_LINE_TOP );
        if( bFillerBottomBorder )
            aBoxItem.SetLine( &aBottomBorderLine, BOX_LINE_BOTTOM );
        if( !bRight && bInhLeftBorder )
            aBoxItem.SetLine( &aInhLeftBorderLine, BOX_LINE_LEFT );
        if( bRight && bInhRightBorder )
            aBoxItem.SetLine( &aInhRightBorderLine, BOX_LINE_RIGHT );
        aBoxItem.SetDistance( MIN_BORDER_DIST );
        pFrmFmt->SetFmtAttr( aBoxItem );
    }
    else
    {
        pFrmFmt->ResetFmtAttr( RES_BOX );
    }

    if( GetInhBGBrush() )
        pFrmFmt->SetFmtAttr( *GetInhBGBrush() );
    else
        pFrmFmt->ResetFmtAttr( RES_BACKGROUND );

    pFrmFmt->ResetFmtAttr( RES_VERT_ORIENT );
    pFrmFmt->ResetFmtAttr( RES_BOXATR_FORMAT );
}

SwTableBox *HTMLTable::NewTableBox( const SwStartNode *pStNd,
                                    SwTableLine *pUpper ) const
{
    SwTableBox *pBox;

    if( pTopTable->pBox1 &&
        pTopTable->pBox1->GetSttNd() == pStNd )
    {
        
        
        pBox = pTopTable->pBox1;
        pBox->SetUpper( pUpper );
        pTopTable->pBox1 = 0;
    }
    else
        pBox = new SwTableBox( pBoxFmt, *pStNd, pUpper );

    return pBox;
}


static void ResetLineFrmFmtAttrs( SwFrmFmt *pFrmFmt )
{
    pFrmFmt->ResetFmtAttr( RES_FRM_SIZE );
    pFrmFmt->ResetFmtAttr( RES_BACKGROUND );
    OSL_ENSURE( SFX_ITEM_SET!=pFrmFmt->GetAttrSet().GetItemState(
                                RES_VERT_ORIENT, false ),
            "Zeile hat vertikale Ausrichtung" );
}


SwTableLine *HTMLTable::MakeTableLine( SwTableBox *pUpper,
                                       sal_uInt16 nTopRow, sal_uInt16 nLeftCol,
                                       sal_uInt16 nBottomRow, sal_uInt16 nRightCol )
{
    SwTableLine *pLine;
    if( this==pTopTable && !pUpper && 0==nTopRow )
        pLine = (pSwTable->GetTabLines())[0];
    else
        pLine = new SwTableLine( pLineFrmFmtNoHeight ? pLineFrmFmtNoHeight
                                                     : pLineFmt,
                                 0, pUpper );

    HTMLTableRow *pTopRow = &(*pRows)[nTopRow];
    sal_uInt16 nRowHeight = pTopRow->GetHeight();
    const SvxBrushItem *pBGBrushItem = 0;
    if( this == pTopTable || nTopRow>0 || nBottomRow<nRows )
    {
        
        
        
        pBGBrushItem = pTopRow->GetBGBrush();

        if( !pBGBrushItem && this != pTopTable )
        {
            
            
            
            pBGBrushItem = GetBGBrush();
            if( !pBGBrushItem )
                pBGBrushItem = GetInhBGBrush();
        }
    }
    if( nTopRow==nBottomRow-1 && (nRowHeight || pBGBrushItem) )
    {
        SwTableLineFmt *pFrmFmt = (SwTableLineFmt*)pLine->ClaimFrmFmt();
        ResetLineFrmFmtAttrs( pFrmFmt );

        if( nRowHeight )
        {
            
            
            
            
            nRowHeight += GetTopCellSpace( nTopRow, 1, sal_False ) +
                       GetBottomCellSpace( nTopRow, 1, sal_False );

            pFrmFmt->SetFmtAttr( SwFmtFrmSize( ATT_MIN_SIZE, 0, nRowHeight ) );
        }

        if( pBGBrushItem )
        {
            pFrmFmt->SetFmtAttr( *pBGBrushItem );
        }

    }
    else if( !pLineFrmFmtNoHeight )
    {
        
        
        pLineFrmFmtNoHeight = (SwTableLineFmt*)pLine->ClaimFrmFmt();

        ResetLineFrmFmtAttrs( pLineFrmFmtNoHeight );
    }


    SwTableBoxes& rBoxes = pLine->GetTabBoxes();

    sal_uInt16 nStartCol = nLeftCol;
    while( nStartCol<nRightCol )
    {
        sal_uInt16 nCol = nStartCol;
        sal_uInt16 nSplitCol = nRightCol;
        sal_Bool bSplitted = sal_False;
        while( !bSplitted )
        {
            OSL_ENSURE( nCol < nRightCol, "Zu weit gelaufen" );

            HTMLTableCell *pCell = GetCell(nTopRow,nCol);
            const sal_Bool bSplit = 1 == pCell->GetColSpan();

            OSL_ENSURE((nCol != nRightCol-1) || bSplit, "Split-Flag wrong");
            if( bSplit )
            {
                SwTableBox* pBox = 0;
                HTMLTableCell *pCell2 = GetCell( nTopRow, nStartCol );
                if( pCell2->GetColSpan() == (nCol+1-nStartCol) )
                {
                    
                    
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

                    bSplitted = sal_True;
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
        
        if( pCnts->GetStartNode() )
        {
            
            pBox = NewTableBox( pCnts->GetStartNode(), pUpper );
            pCnts->SetTableBox( pBox );
        }
        else
        {
            pCnts->GetTable()->InheritVertBorders( this, nLeftCol,
                                                   nRightCol-nLeftCol );
            
            
            
            pBox = new SwTableBox( pBoxFmt, 0, pUpper );
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
        
        pBox = new SwTableBox( pBoxFmt, 0, pUpper );
        SwTableLines& rLines = pBox->GetTabLines();
        sal_Bool bFirstPara = sal_True;

        while( pCnts )
        {
            if( pCnts->GetStartNode() )
            {
                
                SwTableLine *pLine =
                    new SwTableLine( pLineFrmFmtNoHeight ? pLineFrmFmtNoHeight
                                                         : pLineFmt, 0, pBox );
                if( !pLineFrmFmtNoHeight )
                {
                    
                    
                    pLineFrmFmtNoHeight = (SwTableLineFmt*)pLine->ClaimFrmFmt();

                    ResetLineFrmFmtAttrs( pLineFrmFmtNoHeight );
                }

                SwTableBox* pCntBox = NewTableBox( pCnts->GetStartNode(),
                                                   pLine );
                pCnts->SetTableBox( pCntBox );
                FixFrameFmt( pCntBox, nTopRow, nLeftCol, nRowSpan, nColSpan,
                             bFirstPara, 0==pCnts->Next() );
                pLine->GetTabBoxes().push_back( pCntBox );

                rLines.push_back( pLine );
            }
            else
            {
                pCnts->GetTable()->InheritVertBorders( this, nLeftCol,
                                                       nRightCol-nLeftCol );
                
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
            bFirstPara = sal_False;
        }
    }

    FixFrameFmt( pBox, nTopRow, nLeftCol, nRowSpan, nColSpan );

    return pBox;
}

void HTMLTable::InheritBorders( const HTMLTable *pParent,
                                sal_uInt16 nRow, sal_uInt16 nCol,
                                sal_uInt16 nRowSpan, sal_uInt16 /*nColSpan*/,
                                sal_Bool bFirstPara, sal_Bool bLastPara )
{
    OSL_ENSURE( nRows>0 && nCols>0 && nCurRow==nRows,
            "Wurde CloseTable nicht aufgerufen?" );

    
    
    
    
    
    
    
    
    //
    if( 0==nRow && pParent->bTopBorder && bFirstPara )
    {
        bTopBorder = true;
        bFillerTopBorder = true; 
        aTopBorderLine = pParent->aTopBorderLine;
    }
    if( (*pParent->pRows)[nRow+nRowSpan-1].bBottomBorder && bLastPara )
    {
        (*pRows)[nRows-1].bBottomBorder = true;
        bFillerBottomBorder = true; 
        aBottomBorderLine =
            nRow+nRowSpan==pParent->nRows ? pParent->aBottomBorderLine
                                          : pParent->aBorderLine;
    }


    
    
    
    
    bTopAlwd = ( !bFirstPara || (pParent->bTopAlwd &&
                 (0==nRow || !((*pParent->pRows)[nRow-1]).bBottomBorder)) );

    
    
    const SvxBrushItem *pInhBG = pParent->GetCell(nRow,nCol)->GetBGBrush();
    if( !pInhBG && pParent != pTopTable &&
        pParent->GetCell(nRow,nCol)->GetRowSpan() == pParent->nRows )
    {
        
        
        
        
        pInhBG = (*pParent->pRows)[nRow].GetBGBrush();
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
        bInhRightBorder = true; 
        aInhRightBorderLine = pParent->aRightBorderLine;
        nInhRightBorderWidth =
            GetBorderWidth( aInhRightBorderLine, sal_True ) + MIN_BORDER_DIST;
    }

    if( ((*pParent->pColumns)[nCol]).bLeftBorder )
    {
        bInhLeftBorder = true;  
        aInhLeftBorderLine = 0==nCol ? pParent->aLeftBorderLine
                                     : pParent->aBorderLine;
        nInhLeftBorderWidth =
            GetBorderWidth( aInhLeftBorderLine, sal_True ) + MIN_BORDER_DIST;
    }

    if( !bInhLeftBorder && (bFillerTopBorder || bFillerBottomBorder) )
        nInhLeftBorderWidth = 2 * MIN_BORDER_DIST;
    if( !bInhRightBorder && (bFillerTopBorder || bFillerBottomBorder) )
        nInhRightBorderWidth = 2 * MIN_BORDER_DIST;
    pLayoutInfo->SetInhBorderWidths( nInhLeftBorderWidth,
                                     nInhRightBorderWidth );

    bRightAlwd = ( pParent->bRightAlwd &&
                  (nCol+nColSpan==pParent->nCols ||
                   !((*pParent->pColumns)[nCol+nColSpan]).bLeftBorder) );
}

void HTMLTable::SetBorders()
{
    sal_uInt16 i;
    for( i=1; i<nCols; i++ )
        if( HTML_TR_ALL==eRules || HTML_TR_COLS==eRules ||
            ((HTML_TR_ROWS==eRules || HTML_TR_GROUPS==eRules) &&
             ((*pColumns)[i-1]).IsEndOfGroup()) )
            ((*pColumns)[i]).bLeftBorder = true;

    for( i=0; i<nRows-1; i++ )
        if( HTML_TR_ALL==eRules || HTML_TR_ROWS==eRules ||
            ((HTML_TR_COLS==eRules || HTML_TR_GROUPS==eRules) &&
             (*pRows)[i].IsEndOfGroup()) )
            (*pRows)[i].bBottomBorder = true;

    if( bTopAlwd && (HTML_TF_ABOVE==eFrame || HTML_TF_HSIDES==eFrame ||
                     HTML_TF_BOX==eFrame) )
        bTopBorder = true;
    if( HTML_TF_BELOW==eFrame || HTML_TF_HSIDES==eFrame ||
        HTML_TF_BOX==eFrame )
        (*pRows)[nRows-1].bBottomBorder = true;
    if( (HTML_TF_RHS==eFrame || HTML_TF_VSIDES==eFrame ||
                      HTML_TF_BOX==eFrame) )
        bRightBorder = true;
    if( HTML_TF_LHS==eFrame || HTML_TF_VSIDES==eFrame || HTML_TF_BOX==eFrame )
        ((*pColumns)[0]).bLeftBorder = true;

    for( i=0; i<nRows; i++ )
    {
        HTMLTableRow *pRow = &(*pRows)[i];
        for( sal_uInt16 j=0; j<nCols; j++ )
        {
            HTMLTableCell *pCell = pRow->GetCell(j);
            if( pCell->GetContents()  )
            {
                HTMLTableCnts *pCnts = pCell->GetContents();
                sal_Bool bFirstPara = sal_True;
                while( pCnts )
                {
                    HTMLTable *pTable = pCnts->GetTable();
                    if( pTable && !pTable->BordersSet() )
                    {
                        pTable->InheritBorders( this, i, j,
                                                pCell->GetRowSpan(),
                                                pCell->GetColSpan(),
                                                bFirstPara,
                                                0==pCnts->Next() );
                        pTable->SetBorders();
                    }
                    bFirstPara = sal_False;
                    pCnts = pCnts->Next();
                }
            }
        }
    }

    bBordersSet = true;
}

sal_uInt16 HTMLTable::GetBorderWidth( const SvxBorderLine& rBLine,
                                  sal_Bool bWithDistance ) const
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
    OSL_ENSURE(nRow < pRows->size(), "invalid row index in HTML table");
    return (*pRows)[nRow].GetCell( nCell );
}


SvxAdjust HTMLTable::GetInheritedAdjust() const
{
    SvxAdjust eAdjust = (nCurCol<nCols ? ((*pColumns)[nCurCol]).GetAdjust()
                                       : SVX_ADJUST_END );
    if( SVX_ADJUST_END==eAdjust )
        eAdjust = (*pRows)[nCurRow].GetAdjust();

    return eAdjust;
}

sal_Int16 HTMLTable::GetInheritedVertOri() const
{
    
    sal_Int16 eVOri = (*pRows)[nCurRow].GetVertOri();
    if( text::VertOrientation::TOP==eVOri && nCurCol<nCols )
        eVOri = ((*pColumns)[nCurCol]).GetVertOri();
    if( text::VertOrientation::TOP==eVOri )
        eVOri = eVertOri;

    OSL_ENSURE( eVertOri != text::VertOrientation::TOP, "text::VertOrientation::TOP ist nicht erlaubt!" );
    return eVOri;
}

void HTMLTable::InsertCell( HTMLTableCnts *pCnts,
                            sal_uInt16 nRowSpan, sal_uInt16 nColSpan,
                            sal_uInt16 nCellWidth, sal_Bool bRelWidth, sal_uInt16 nCellHeight,
                            sal_Int16 eVertOrient, SvxBrushItem *pBGBrushItem,
                            boost::shared_ptr<SvxBoxItem> const pBoxItem,
                            sal_Bool bHasNumFmt, sal_uInt32 nNumFmt,
                            sal_Bool bHasValue, double nValue, sal_Bool bNoWrap )
{
    if( !nRowSpan || (sal_uInt32)nCurRow + nRowSpan > USHRT_MAX )
        nRowSpan = 1;

    if( !nColSpan || (sal_uInt32)nCurCol + nColSpan > USHRT_MAX )
        nColSpan = 1;

    sal_uInt16 nColsReq = nCurCol + nColSpan;       
    sal_uInt16 nRowsReq = nCurRow + nRowSpan;       
    sal_uInt16 i, j;

    
    
    if( nCols < nColsReq )
    {
        for( i=nCols; i<nColsReq; i++ )
            pColumns->push_back( new HTMLTableColumn );
        for( i=0; i<nRows; i++ )
            (*pRows)[i].Expand( nColsReq, i<nCurRow );
        nCols = nColsReq;
        OSL_ENSURE(pColumns->size() == nCols,
                "wrong number of columns after expanding");
    }
    if( nColsReq > nFilledCols )
        nFilledCols = nColsReq;

    
    
    if( nRows < nRowsReq )
    {
        for( i=nRows; i<nRowsReq; i++ )
            pRows->push_back( new HTMLTableRow(nCols) );
        nRows = nRowsReq;
        OSL_ENSURE(nRows == pRows->size(), "wrong number of rows in Insert");
    }

    
    
    sal_uInt16 nSpanedCols = 0;
    if( nCurRow>0 )
    {
        HTMLTableRow *pCurRow = &(*pRows)[nCurRow];
        for( i=nCurCol; i<nColsReq; i++ )
        {
            HTMLTableCell *pCell = pCurRow->GetCell(i);
            if( pCell->GetContents() )
            {
                
                
                
                
                
                nSpanedCols = i + pCell->GetColSpan();
                FixRowSpan( nCurRow-1, i, pCell->GetContents() );
                if( pCell->GetRowSpan() > nRowSpan )
                    ProtectRowSpan( nRowsReq, i,
                                    pCell->GetRowSpan()-nRowSpan );
            }
        }
        for( i=nColsReq; i<nSpanedCols; i++ )
        {
            
            
            HTMLTableCell *pCell = pCurRow->GetCell(i);
            FixRowSpan( nCurRow-1, i, pCell->GetContents() );
            ProtectRowSpan( nCurRow, i, pCell->GetRowSpan() );
        }
    }

    
    for( i=nColSpan; i>0; i-- )
    {
        for( j=nRowSpan; j>0; j-- )
        {
            const bool bCovered = i != nColSpan || j != nRowSpan;
            GetCell( nRowsReq-j, nColsReq-i )
                ->Set( pCnts, j, i, eVertOrient, pBGBrushItem, pBoxItem,
                       bHasNumFmt, nNumFmt, bHasValue, nValue, bNoWrap, bCovered );
        }
    }

    Size aTwipSz( bRelWidth ? 0 : nCellWidth, nCellHeight );
    if( (aTwipSz.Width() || aTwipSz.Height()) && Application::GetDefaultDevice() )
    {
        aTwipSz = Application::GetDefaultDevice()
                    ->PixelToLogic( aTwipSz, MapMode( MAP_TWIP ) );
    }

    
    if( nCellWidth )
    {
        sal_uInt16 nTmp = bRelWidth ? nCellWidth : (sal_uInt16)aTwipSz.Width();
        GetCell( nCurRow, nCurCol )->SetWidth( nTmp, bRelWidth );
    }

    
    if( nCellHeight && 1==nRowSpan )
    {
        if( nCellHeight < MINLAY )
            nCellHeight = MINLAY;
        (*pRows)[nCurRow].SetHeight( (sal_uInt16)aTwipSz.Height() );
    }

    
    nCurCol = nColsReq;
    if( nSpanedCols > nCurCol )
        nCurCol = nSpanedCols;

    
    while( nCurCol<nCols && GetCell(nCurRow,nCurCol)->IsUsed() )
        nCurCol++;
}

inline void HTMLTable::CloseSection( sal_Bool bHead )
{
    
    OSL_ENSURE( nCurRow<=nRows, "ungeultige aktuelle Zeile" );
    if( nCurRow>0 && nCurRow<=nRows )
        (*pRows)[nCurRow-1].SetEndOfGroup();
    if( bHead )
        nHeadlineRepeat = nCurRow;
}

void HTMLTable::OpenRow( SvxAdjust eAdjust, sal_Int16 eVertOrient,
                         SvxBrushItem *pBGBrushItem )
{
    sal_uInt16 nRowsReq = nCurRow+1;    

    
    if( nRows<nRowsReq )
    {
        for( sal_uInt16 i=nRows; i<nRowsReq; i++ )
            pRows->push_back( new HTMLTableRow(nCols) );
        nRows = nRowsReq;
        OSL_ENSURE( nRows==pRows->size(),
                "Zeilenzahl in OpenRow stimmt nicht" );
    }

    HTMLTableRow *pCurRow = &((*pRows)[nCurRow]);
    pCurRow->SetAdjust( eAdjust );
    pCurRow->SetVertOri( eVertOrient );
    if( pBGBrushItem )
        (*pRows)[nCurRow].SetBGBrush( pBGBrushItem );

    
    nCurCol=0;

    
    while( nCurCol<nCols && GetCell(nCurRow,nCurCol)->IsUsed() )
        nCurCol++;
}

void HTMLTable::CloseRow( sal_Bool bEmpty )
{
    OSL_ENSURE( nCurRow<nRows, "aktulle Zeile hinter dem Tabellenende" );

    
    if( bEmpty )
    {
        if( nCurRow > 0 )
            (*pRows)[nCurRow-1].IncEmptyRows();
        return;
    }

    HTMLTableRow *pRow = &(*pRows)[nCurRow];

    
    
    
    
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
        ((*pColumns)[nCurCol-1]).SetEndOfGroup();
}

void HTMLTable::InsertCol( sal_uInt16 nSpan, sal_uInt16 nColWidth, bool bRelWidth,
                           SvxAdjust eAdjust, sal_Int16 eVertOrient )
{
    
    if ( nRows > 0 )
        return;

    sal_uInt16 i;

    if( !nSpan )
        nSpan = 1;

    sal_uInt16 nColsReq = nCurCol + nSpan;      

    if( nCols < nColsReq )
    {
        for( i=nCols; i<nColsReq; i++ )
            pColumns->push_back( new HTMLTableColumn );
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
        HTMLTableColumn *pCol = &(*pColumns)[i];
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

    
    
    
    
    
    if( nRows>nCurRow )
    {
        HTMLTableRow *pPrevRow = &(*pRows)[nCurRow-1];
        HTMLTableCell *pCell;
        for( i=0; i<nCols; i++ )
            if( ( (pCell=(pPrevRow->GetCell(i))), (pCell->GetRowSpan()) > 1 ) )
            {
                FixRowSpan( nCurRow-1, i, pCell->GetContents() );
                ProtectRowSpan( nCurRow, i, (*pRows)[nCurRow].GetCell(i)->GetRowSpan() );
            }
        for( i=nRows-1; i>=nCurRow; i-- )
            pRows->erase(pRows->begin() + i);
        nRows = nCurRow;
    }

    
    if( 0==nCols )
    {
        pColumns->push_back( new HTMLTableColumn );
        for( i=0; i<nRows; i++ )
            (*pRows)[i].Expand(1);
        nCols = 1;
        nFilledCols = 1;
    }

    
    if( 0==nRows )
    {
        pRows->push_back( new HTMLTableRow(nCols) );
        nRows = 1;
        nCurRow = 1;
    }

    if( nFilledCols < nCols )
    {
        pColumns->erase( pColumns->begin() + nFilledCols, pColumns->begin() + nCols );
        for( i=0; i<nRows; i++ )
            (*pRows)[i].Shrink( nFilledCols );
        nCols = nFilledCols;
    }
}

void HTMLTable::_MakeTable( SwTableBox *pBox )
{
    SwTableLines& rLines = (pBox ? pBox->GetTabLines()
                                 : ((SwTable *)pSwTable)->GetTabLines() );

    

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

    OSL_ENSURE( (pLayoutInfo==0) == (this==pTopTable),
            "Top-Tabelle hat keine Layout-Info oder umgekehrt" );

    if( this==pTopTable )
    {
        
        SetBorders();

        
        
        CreateLayoutInfo();

        
        
        
        pLayoutInfo->AutoLayoutPass1();
    }

    
    
    
    
    pLayoutInfo->AutoLayoutPass2( nAbsAvail, nRelAvail, nAbsLeftSpace,
                                  nAbsRightSpace, nInhAbsSpace );

    if( this!=pTopTable )
    {
        
        
        if( pLayoutInfo->GetRelRightFill() == 0 )
        {
            if( !bRightBorder )
            {
                
                if( bInhRightBorder )
                {
                    bRightBorder = true;
                    aRightBorderLine = aInhRightBorderLine;
                }
            }
            else
            {
                
                bRightBorder = bRightAlwd;
            }
        }

        if( pLayoutInfo->GetRelLeftFill() == 0 &&
            !((*pColumns)[0]).bLeftBorder &&
            bInhLeftBorder )
        {
            
            ((*pColumns)[0]).bLeftBorder = true;
            aLeftBorderLine = aInhLeftBorderLine;
        }
    }

    
    if( this==pTopTable )
    {
        sal_Int16 eHoriOri;
        if( bForceFrame )
        {
            
            
            
            eHoriOri = bPrcWidth ? text::HoriOrientation::FULL : text::HoriOrientation::LEFT;
        }
        else switch( eTableAdjust )
        {
            
            
            

        case SVX_ADJUST_RIGHT:
            
            
            eHoriOri = text::HoriOrientation::RIGHT;
            break;
        case SVX_ADJUST_CENTER:
            
            eHoriOri = text::HoriOrientation::CENTER;
            break;
        case SVX_ADJUST_LEFT:
        default:
            
            
            eHoriOri = nLeftMargin ? text::HoriOrientation::LEFT_AND_WIDTH : text::HoriOrientation::LEFT;
            break;
        }

        
        SwFrmFmt *pFrmFmt = pSwTable->GetFrmFmt();
        pFrmFmt->SetFmtAttr( SwFmtHoriOrient(0,eHoriOri) );
        if( text::HoriOrientation::LEFT_AND_WIDTH==eHoriOri )
        {
            OSL_ENSURE( nLeftMargin || nRightMargin,
                    "Da gibt's wohl noch Reste von relativen Breiten" );

            
            SvxLRSpaceItem aLRItem( pSwTable->GetFrmFmt()->GetLRSpace() );
            aLRItem.SetLeft( nLeftMargin );
            aLRItem.SetRight( nRightMargin );
            pFrmFmt->SetFmtAttr( aLRItem );
        }

        if( bPrcWidth && text::HoriOrientation::FULL!=eHoriOri )
        {
            pFrmFmt->LockModify();
            SwFmtFrmSize aFrmSize( pFrmFmt->GetFrmSize() );
            aFrmSize.SetWidthPercent( (sal_uInt8)nWidth );
            pFrmFmt->SetFmtAttr( aFrmSize );
            pFrmFmt->UnlockModify();
        }
    }

    
    if( this==pTopTable )
    {
        
        SwTableLine *pLine1 = (pSwTable->GetTabLines())[0];
        pBox1 = (pLine1->GetTabBoxes())[0];
        pLine1->GetTabBoxes().erase(pLine1->GetTabBoxes().begin());

        pLineFmt = (SwTableLineFmt*)pLine1->GetFrmFmt();
        pBoxFmt = (SwTableBoxFmt*)pBox1->GetFrmFmt();
    }
    else
    {
        pLineFmt = (SwTableLineFmt*)pTopTable->pLineFmt;
        pBoxFmt = (SwTableBoxFmt*)pTopTable->pBoxFmt;
    }

    
    
    if( this != pTopTable &&
        ( pLayoutInfo->GetRelLeftFill() > 0  ||
          pLayoutInfo->GetRelRightFill() > 0 ) )
    {
        OSL_ENSURE( pBox, "kein TableBox fuer Tabelle in Tabelle" );

        SwTableLines& rLines = pBox->GetTabLines();

        
        SwTableLine *pLine =
            new SwTableLine( pLineFrmFmtNoHeight ? pLineFrmFmtNoHeight
                                                 : pLineFmt, 0, pBox );
        rLines.push_back( pLine );

        
        if( !pLineFrmFmtNoHeight )
        {
            
            
            pLineFrmFmtNoHeight = (SwTableLineFmt*)pLine->ClaimFrmFmt();

            ResetLineFrmFmtAttrs( pLineFrmFmtNoHeight );
        }

        SwTableBoxes& rBoxes = pLine->GetTabBoxes();
        SwTableBox *pNewBox;

        
        if( pLayoutInfo->GetRelLeftFill() > 0 )
        {
            
            
            pPrevStNd = pParser->InsertTableSection( pPrevStNd );

            pNewBox = NewTableBox( pPrevStNd, pLine );
            rBoxes.push_back( pNewBox );
            FixFillerFrameFmt( pNewBox, sal_False );
            pLayoutInfo->SetLeftFillerBox( pNewBox );
        }

        
        pNewBox = new SwTableBox( pBoxFmt, 0, pLine );
        rBoxes.push_back( pNewBox );

        SwFrmFmt *pFrmFmt = pNewBox->ClaimFrmFmt();
        pFrmFmt->ResetFmtAttr( RES_BOX );
        pFrmFmt->ResetFmtAttr( RES_BACKGROUND );
        pFrmFmt->ResetFmtAttr( RES_VERT_ORIENT );
        pFrmFmt->ResetFmtAttr( RES_BOXATR_FORMAT );


        _MakeTable( pNewBox );

        
        if( pLayoutInfo->GetRelRightFill() > 0 )
        {
            const SwStartNode *pStNd =
                GetPrevBoxStartNode( USHRT_MAX, USHRT_MAX );
            pStNd = pParser->InsertTableSection( pStNd );

            pNewBox = NewTableBox( pStNd, pLine );
            rBoxes.push_back( pNewBox );

            FixFillerFrameFmt( pNewBox, sal_True );
            pLayoutInfo->SetRightFillerBox( pNewBox );
        }
    }
    else
    {
        _MakeTable( pBox );
    }

    
    
    if( this==pTopTable )
    {
        if( 1==nRows && nHeight && 1==pSwTable->GetTabLines().size() )
        {
            
            
            
            
            nHeight = pParser->ToTwips( nHeight );
            if( nHeight < MINLAY )
                nHeight = MINLAY;

            (pSwTable->GetTabLines())[0]->ClaimFrmFmt();
            (pSwTable->GetTabLines())[0]->GetFrmFmt()
                ->SetFmtAttr( SwFmtFrmSize( ATT_MIN_SIZE, 0, nHeight ) );
        }

        if( GetBGBrush() )
            pSwTable->GetFrmFmt()->SetFmtAttr( *GetBGBrush() );

        ((SwTable *)pSwTable)->SetRowsToRepeat( static_cast< sal_uInt16 >(nHeadlineRepeat) );
        ((SwTable *)pSwTable)->GCLines();

        sal_Bool bIsInFlyFrame = pContext && pContext->GetFrmFmt();
        if( bIsInFlyFrame && !nWidth )
        {
            SvxAdjust eTblAdjust = GetTableAdjust(sal_False);
            if( eTblAdjust != SVX_ADJUST_LEFT &&
                eTblAdjust != SVX_ADJUST_RIGHT )
            {
                
                
                
                
                OSL_ENSURE( HasToFly(), "Warum ist die Tabelle in einem Rahmen?" );
                sal_uInt32 nMin = pLayoutInfo->GetMin();
                if( nMin > USHRT_MAX )
                    nMin = USHRT_MAX;
                SwFmtFrmSize aFlyFrmSize( ATT_VAR_SIZE, (SwTwips)nMin, MINLAY );
                aFlyFrmSize.SetWidthPercent( 100 );
                pContext->GetFrmFmt()->SetFmtAttr( aFlyFrmSize );
                bIsInFlyFrame = sal_False;
            }
            else
            {
                
                
                
                pLayoutInfo->SetMustNotRecalc( sal_True );
                if( pContext->GetFrmFmt()->GetAnchor().GetCntntAnchor()
                    ->nNode.GetNode().FindTableNode() )
                {
                    sal_uInt32 nMax = pLayoutInfo->GetMax();
                    if( nMax > USHRT_MAX )
                        nMax = USHRT_MAX;
                    SwFmtFrmSize aFlyFrmSize( ATT_VAR_SIZE, (SwTwips)nMax, MINLAY );
                    pContext->GetFrmFmt()->SetFmtAttr( aFlyFrmSize );
                    bIsInFlyFrame = sal_False;
                }
                else
                {
                    pLayoutInfo->SetMustNotResize( sal_True );
                }
            }
        }
        pLayoutInfo->SetMayBeInFlyFrame( bIsInFlyFrame );

        
        
        pLayoutInfo->SetMustResize( bPrcWidth || !nWidth );

        pLayoutInfo->SetWidths();

        ((SwTable *)pSwTable)->SetHTMLTableLayout( pLayoutInfo );

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

                pParser->ResizeDrawObject( pObj, nWidth2 );
            }
        }
    }
}

void HTMLTable::SetTable( const SwStartNode *pStNd, _HTMLTableContext *pCntxt,
                          sal_uInt16 nLeft, sal_uInt16 nRight,
                          const SwTable *pSwTab, sal_Bool bFrcFrame )
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

        SetHasParentSection( sal_True );
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

    pCSS1Parser->SetTDTagStyles();
    SwTxtFmtColl *pColl = pCSS1Parser->GetTxtCollFromPool( RES_POOLCOLL_TABLE );

    const SwStartNode *pStNd;
    if( pTable && pTable->bFirstCell )
    {
        SwNode *const pNd = & pPam->GetPoint()->nNode.GetNode();
        pNd->GetTxtNode()->ChgFmtColl( pColl );
        pStNd = pNd->FindTableBoxStartNode();
        pTable->bFirstCell = sal_False;
    }
    else
    {
        const SwNode* pNd;
        if( pPrevStNd->IsTableNode() )
            pNd = pPrevStNd;
        else
            pNd = pPrevStNd->EndOfSectionNode();
        SwNodeIndex nIdx( *pNd, 1 );
        pStNd = pDoc->GetNodes().MakeTextSection( nIdx, SwTableBoxStartNode,
                                                  pColl );
        pTable->IncBoxCount();
    }

    
    SwCntntNode *pCNd = pDoc->GetNodes()[pStNd->GetIndex()+1] ->GetCntntNode();
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
        pCSS1Parser->SetTHTagStyles();
        break;
    case RES_POOLCOLL_TABLE:
        pCSS1Parser->SetTDTagStyles();
        break;
    }

    SwTxtFmtColl *pColl = pCSS1Parser->GetTxtCollFromPool( nPoolId );

    SwNode *const pNd = & pPam->GetPoint()->nNode.GetNode();
    const SwStartNode *pStNd;
    if( pTable && pTable->bFirstCell )
    {
        pNd->GetTxtNode()->ChgFmtColl( pColl );
        pTable->bFirstCell = sal_False;
        pStNd = pNd->FindTableBoxStartNode();
    }
    else
    {
        SwTableNode *pTblNd = pNd->FindTableNode();
        if( pTblNd->GetTable().GetHTMLTableLayout() )
        { 
          
            SwTableNode *pOutTbl = pTblNd;
            do {
                pTblNd = pOutTbl;
                pOutTbl = pOutTbl->StartOfSectionNode()->FindTableNode();
            } while( pOutTbl && pTblNd->GetTable().GetHTMLTableLayout() );
        }
        SwNodeIndex aIdx( *pTblNd->EndOfSectionNode() );
        pStNd = pDoc->GetNodes().MakeTextSection( aIdx, SwTableBoxStartNode,
                                                  pColl );

        pPam->GetPoint()->nNode = pStNd->GetIndex() + 1;
        SwTxtNode *pTxtNd = pPam->GetPoint()->nNode.GetNode().GetTxtNode();
        pPam->GetPoint()->nContent.Assign( pTxtNd, 0 );
        pTable->IncBoxCount();
    }

    return pStNd;
}

SwStartNode *SwHTMLParser::InsertTempTableCaptionSection()
{
    SwTxtFmtColl *pColl = pCSS1Parser->GetTxtCollFromPool( RES_POOLCOLL_TEXT );
    SwNodeIndex& rIdx = pPam->GetPoint()->nNode;
    rIdx = pDoc->GetNodes().GetEndOfExtras();
    SwStartNode *pStNd = pDoc->GetNodes().MakeTextSection( rIdx,
                                          SwNormalStartNode, pColl );

    rIdx = pStNd->GetIndex() + 1;
    pPam->GetPoint()->nContent.Assign( rIdx.GetNode().GetTxtNode(), 0 );

    return pStNd;
}


sal_Int32 SwHTMLParser::StripTrailingLF()
{
    sal_Int32 nStripped = 0;

    const sal_Int32 nLen = pPam->GetPoint()->nContent.GetIndex();
    if( nLen )
    {
        SwTxtNode* pTxtNd = pPam->GetPoint()->nNode.GetNode().GetTxtNode();
        
        if( pTxtNd )
        {
            sal_Int32 nPos = nLen;
            sal_Int32 nLFCount = 0;
            while (nPos && ('\x0a' == pTxtNd->GetTxt()[--nPos]))
                nLFCount++;

            if( nLFCount )
            {
                if( nLFCount > 2 )
                {
                    
                    
                    
                    
                    
                    
                    
                    
                    
                    nLFCount = 2;
                }

                nPos = nLen - nLFCount;
                SwIndex nIdx( pTxtNd, nPos );
                pTxtNd->EraseText( nIdx, nLFCount );
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
    SvxBrushItem *pBrushItem = 0;

    if( !rStyle.isEmpty() || !rId.isEmpty() || !rClass.isEmpty() )
    {
        SfxItemSet aItemSet( pDoc->GetAttrPool(), RES_BACKGROUND,
                                                  RES_BACKGROUND );
        SvxCSS1PropertyInfo aPropInfo;

        if( !rClass.isEmpty() )
        {
            OUString aClass( rClass );
            SwCSS1Parser::GetScriptFromClass( aClass );
            const SvxCSS1MapEntry *pClass = pCSS1Parser->GetClass( aClass );
            if( pClass )
                aItemSet.Put( pClass->GetItemSet() );
        }

        if( !rId.isEmpty() )
        {
            const SvxCSS1MapEntry *pId = pCSS1Parser->GetId( rId );
            if( pId )
                aItemSet.Put( pId->GetItemSet() );
        }

        pCSS1Parser->ParseStyleOption( rStyle, aItemSet, aPropInfo );
        const SfxPoolItem *pItem = 0;
        if( SFX_ITEM_SET == aItemSet.GetItemState( RES_BACKGROUND, false,
                                                   &pItem ) )
        {
            pBrushItem = new SvxBrushItem( *((const SvxBrushItem *)pItem) );
        }
    }

    if( !pBrushItem && (pColor || !rImageURL.isEmpty()) )
    {
        pBrushItem = new SvxBrushItem(RES_BACKGROUND);

        if( pColor )
            pBrushItem->SetColor(*pColor);

        if( !rImageURL.isEmpty() )
        {
            pBrushItem->SetGraphicLink( URIHelper::SmartRel2Abs( INetURLObject(sBaseURL), rImageURL, Link(), false) );
            pBrushItem->SetGraphicPos( GPOS_TILED );
        }
    }

    return pBrushItem;
}


class _SectionSaveStruct : public SwPendingStackData
{
    sal_uInt16 nBaseFontStMinSave, nFontStMinSave, nFontStHeadStartSave;
    sal_uInt16 nDefListDeepSave, nContextStMinSave, nContextStAttrMinSave;

public:

    HTMLTable *pTable;

    _SectionSaveStruct( SwHTMLParser& rParser );
    virtual ~_SectionSaveStruct();

    sal_uInt16 GetContextStAttrMin() const { return nContextStAttrMinSave; }

    void Restore( SwHTMLParser& rParser );
};

_SectionSaveStruct::_SectionSaveStruct( SwHTMLParser& rParser ) :
    nBaseFontStMinSave(0), nFontStMinSave(0), nFontStHeadStartSave(0),
    nDefListDeepSave(0), nContextStMinSave(0), nContextStAttrMinSave(0),
    pTable( 0 )
{
    
    nBaseFontStMinSave = rParser.nBaseFontStMin;
    rParser.nBaseFontStMin = rParser.aBaseFontStack.size();

    nFontStMinSave = rParser.nFontStMin;
    nFontStHeadStartSave = rParser.nFontStHeadStart;
    rParser.nFontStMin = rParser.aFontStack.size();

    
    nContextStMinSave = rParser.nContextStMin;
    nContextStAttrMinSave = rParser.nContextStAttrMin;
    rParser.nContextStMin = rParser.aContexts.size();
    rParser.nContextStAttrMin = rParser.nContextStMin;

    
    nDefListDeepSave = rParser.nDefListDeep;
    rParser.nDefListDeep = 0;
}

_SectionSaveStruct::~_SectionSaveStruct()
{}

void _SectionSaveStruct::Restore( SwHTMLParser& rParser )
{
    
    sal_uInt16 nMin = rParser.nBaseFontStMin;
    if( rParser.aBaseFontStack.size() > nMin )
        rParser.aBaseFontStack.erase( rParser.aBaseFontStack.begin() + nMin,
                rParser.aBaseFontStack.end() );
    rParser.nBaseFontStMin = nBaseFontStMinSave;


    nMin = rParser.nFontStMin;
    if( rParser.aFontStack.size() > nMin )
        rParser.aFontStack.erase( rParser.aFontStack.begin() + nMin,
                rParser.aFontStack.end() );
    rParser.nFontStMin = nFontStMinSave;
    rParser.nFontStHeadStart = nFontStHeadStartSave;

    OSL_ENSURE( rParser.aContexts.size() == rParser.nContextStMin &&
            rParser.aContexts.size() == rParser.nContextStAttrMin,
            "The Context Stack was not cleaned up" );
    rParser.nContextStMin = nContextStMinSave;
    rParser.nContextStAttrMin = nContextStAttrMinSave;

    
    rParser.nDefListDeep = nDefListDeepSave;

    
    rParser.bNoParSpace = false;
    rParser.nOpenParaToken = 0;

    if( !rParser.aParaAttrs.empty() )
        rParser.aParaAttrs.clear();
}


class _CellSaveStruct : public _SectionSaveStruct
{
    OUString aStyle, aId, aClass, aLang, aDir;
    OUString aBGImage;
    Color aBGColor;
    boost::shared_ptr<SvxBoxItem> m_pBoxItem;

    HTMLTableCnts* pCnts;           
    HTMLTableCnts* pCurrCnts;   
    SwNodeIndex *pNoBreakEndParaIdx;

    double nValue;

    sal_uInt32 nNumFmt;

    sal_uInt16 nRowSpan, nColSpan, nWidth, nHeight;
    sal_Int32 nNoBreakEndCntntPos;     

    SvxAdjust eAdjust;
    sal_Int16 eVertOri;

    sal_Bool bHead : 1;
    sal_Bool bPrcWidth : 1;
    sal_Bool bHasNumFmt : 1;
    sal_Bool bHasValue : 1;
    sal_Bool bBGColor : 1;
    sal_Bool bNoWrap : 1;       
    sal_Bool bNoBreak : 1;      

public:

    _CellSaveStruct( SwHTMLParser& rParser, HTMLTable *pCurTable, sal_Bool bHd,
                     sal_Bool bReadOpt );

    virtual ~_CellSaveStruct();

    void AddContents( HTMLTableCnts *pNewCnts );
    HTMLTableCnts *GetFirstContents() { return pCnts; }

    void ClearIsInSection() { pCurrCnts = 0; }
    sal_Bool IsInSection() const { return pCurrCnts!=0; }
    HTMLTableCnts *GetCurrContents() const { return pCurrCnts; }

    void InsertCell( SwHTMLParser& rParser, HTMLTable *pCurTable );

    sal_Bool IsHeaderCell() const { return bHead; }

    void StartNoBreak( const SwPosition& rPos );
    void EndNoBreak( const SwPosition& rPos );
    void CheckNoBreak( const SwPosition& rPos, SwDoc *pDoc );
};


_CellSaveStruct::_CellSaveStruct( SwHTMLParser& rParser, HTMLTable *pCurTable,
                                  sal_Bool bHd, sal_Bool bReadOpt ) :
    _SectionSaveStruct( rParser ),
    pCnts( 0 ),
    pCurrCnts( 0 ),
    pNoBreakEndParaIdx( 0 ),
    nValue( 0.0 ),
    nNumFmt( 0 ),
    nRowSpan( 1 ),
    nColSpan( 1 ),
    nWidth( 0 ),
    nHeight( 0 ),
    nNoBreakEndCntntPos( 0 ),
    eAdjust( pCurTable->GetInheritedAdjust() ),
    eVertOri( pCurTable->GetInheritedVertOri() ),
    bHead( bHd ),
    bPrcWidth( sal_False ),
    bHasNumFmt( sal_False ),
    bHasValue( sal_False ),
    bBGColor( sal_False ),
    bNoWrap( sal_False ),
    bNoBreak( sal_False )
{
    OUString aNumFmt, aValue;

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
                                        aHTMLTblVAlignTable, eVertOri );
                break;
            case HTML_O_WIDTH:
                nWidth = (sal_uInt16)rOption.GetNumber();   
                bPrcWidth = (rOption.GetString().indexOf('%') != -1);
                if( bPrcWidth && nWidth>100 )
                    nWidth = 100;
                break;
            case HTML_O_HEIGHT:
                nHeight = (sal_uInt16)rOption.GetNumber();  
                if( rOption.GetString().indexOf('%') != -1)
                    nHeight = 0;    
                break;
            case HTML_O_BGCOLOR:
                
                
                if( !rOption.GetString().isEmpty() )
                {
                    rOption.GetColor( aBGColor );
                    bBGColor = sal_True;
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
                aNumFmt = rOption.GetString();
                bHasNumFmt = sal_True;
                break;
            case HTML_O_SDVAL:
                bHasValue = sal_True;
                aValue = rOption.GetString();
                break;
            case HTML_O_NOWRAP:
                bNoWrap = sal_True;
                break;
            }
        }

        if( !aId.isEmpty() )
            rParser.InsertBookmark( aId );
    }

    if( bHasNumFmt )
    {
        LanguageType eLang;
        nValue = rParser.GetTableDataOptionsValNum(
                            nNumFmt, eLang, aValue, aNumFmt,
                            *rParser.pDoc->GetNumberFormatter() );
    }

    
    
    
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
    _HTMLAttrContext *pCntxt = new _HTMLAttrContext( nToken, nColl, aEmptyOUStr, sal_True );
    if( SVX_ADJUST_END != eAdjust )
        rParser.InsertAttr( &rParser.aAttrTab.pAdjust, SvxAdjustItem(eAdjust, RES_PARATR_ADJUST),
                            pCntxt );

    if( rParser.HasStyleOptions( aStyle, aId, aClass, &aLang, &aDir ) )
    {
        SfxItemSet aItemSet( rParser.pDoc->GetAttrPool(),
                             rParser.pCSS1Parser->GetWhichMap() );
        SvxCSS1PropertyInfo aPropInfo;

        if( rParser.ParseStyleOptions( aStyle, aId, aClass, aItemSet,
                                       aPropInfo, &aLang, &aDir ) )
        {
            SfxPoolItem const* pItem;
            if (SFX_ITEM_SET == aItemSet.GetItemState(RES_BOX, false, &pItem))
            {   
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
    
    
    
    
    
    
    
    
    
    
    
    
    
    

    if( rParser.nContextStAttrMin == GetContextStAttrMin() )
    {
        _HTMLAttr** pTbl = (_HTMLAttr**)&rParser.aAttrTab;

        for( sal_uInt16 nCnt = sizeof( _HTMLAttrTable ) / sizeof( _HTMLAttr* );
            nCnt--; ++pTbl )
        {
            OSL_ENSURE( !*pTbl, "Die Attribut-Tabelle ist nicht leer" );
        }
    }
#endif

    
    SvxBrushItem *pBrushItem =
        rParser.CreateBrushItem( bBGColor ? &aBGColor : 0, aBGImage,
                                 aStyle, aId, aClass );
    pCurTable->InsertCell( pCnts, nRowSpan, nColSpan, nWidth,
                           bPrcWidth, nHeight, eVertOri, pBrushItem, m_pBoxItem,
                           bHasNumFmt, nNumFmt, bHasValue, nValue,
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
        bNoBreak = sal_True;
    }
}

void _CellSaveStruct::EndNoBreak( const SwPosition& rPos )
{
    if( bNoBreak )
    {
        delete pNoBreakEndParaIdx;
        pNoBreakEndParaIdx = new SwNodeIndex( rPos.nNode );
        nNoBreakEndCntntPos = rPos.nContent.GetIndex();
        bNoBreak = sal_False;
    }
}

void _CellSaveStruct::CheckNoBreak( const SwPosition& rPos, SwDoc * /*pDoc*/ )
{
    if( pCnts && pCurrCnts==pCnts )
    {
        if( bNoBreak )
        {
            
            pCnts->SetNoBreak();
        }
        else if( pNoBreakEndParaIdx &&
                 pNoBreakEndParaIdx->GetIndex() == rPos.nNode.GetIndex() )
        {
            if( nNoBreakEndCntntPos == rPos.nContent.GetIndex() )
            {
                
                pCnts->SetNoBreak();
            }
            else if( nNoBreakEndCntntPos + 1 == rPos.nContent.GetIndex() )
            {
                SwTxtNode const*const pTxtNd(rPos.nNode.GetNode().GetTxtNode());
                if( pTxtNd )
                {
                    sal_Unicode const cLast =
                            pTxtNd->GetTxt()[nNoBreakEndCntntPos];
                    if( ' '==cLast || '\x0a'==cLast )
                    {
                        
                        
                        pCnts->SetNoBreak();
                    }
                }
            }
        }
    }
}



HTMLTableCnts *SwHTMLParser::InsertTableContents(
                                        sal_Bool bHead )
{
    
    const SwStartNode *pStNd =
        InsertTableSection( static_cast< sal_uInt16 >(bHead ? RES_POOLCOLL_TABLE_HDLN
                                           : RES_POOLCOLL_TABLE) );

    if( GetNumInfo().GetNumRule() )
    {
        
        sal_uInt8 nLvl = GetNumInfo().GetLevel();

        SetNodeNum( nLvl, false );
    }

    
    const SwNodeIndex& rSttPara = pPam->GetPoint()->nNode;
    sal_Int32 nSttCnt = pPam->GetPoint()->nContent.GetIndex();

    _HTMLAttr** pTbl = (_HTMLAttr**)&aAttrTab;
    for( sal_uInt16 nCnt = sizeof( _HTMLAttrTable ) / sizeof( _HTMLAttr* );
        nCnt--; ++pTbl )
    {

        _HTMLAttr *pAttr = *pTbl;
        while( pAttr )
        {
            OSL_ENSURE( !pAttr->GetPrev(), "Attribut hat Previous-Liste" );
            pAttr->nSttPara = rSttPara;
            pAttr->nEndPara = rSttPara;
            pAttr->nSttCntnt = nSttCnt;
            pAttr->nEndCntnt = nSttCnt;

            pAttr = pAttr->GetNext();
        }
    }

    return new HTMLTableCnts( pStNd );
}

sal_uInt16 SwHTMLParser::IncGrfsThatResizeTable()
{
    return pTable ? pTable->IncGrfsThatResize() : 0;
}

void SwHTMLParser::RegisterDrawObjectToTable( HTMLTable *pCurTable,
                                        SdrObject *pObj, sal_uInt8 nPrcWidth )
{
    pCurTable->RegisterDrawObject( pObj, nPrcWidth );
}

void SwHTMLParser::BuildTableCell( HTMLTable *pCurTable, sal_Bool bReadOptions,
                                   sal_Bool bHead )
{
    if( !IsParserWorking() && !pPendStack )
        return;

    _CellSaveStruct* pSaveStruct;

    int nToken = 0;
    sal_Bool bPending = sal_False;
    if( pPendStack )
    {
        pSaveStruct = (_CellSaveStruct*)pPendStack->pData;

        SwPendingStack* pTmp = pPendStack->pNext;
        delete pPendStack;
        pPendStack = pTmp;
        nToken = pPendStack ? pPendStack->nToken : GetSaveToken();
        bPending = SVPAR_ERROR == eState && pPendStack != 0;

        SaveState( nToken );
    }
    else
    {
        
        if( pTable->IsOverflowing() )
        {
            SaveState( 0 );
            return;
        }

        if( !pCurTable->GetContext() )
        {
            sal_Bool bTopTable = pTable==pCurTable;

            
            

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

            SfxItemSet aItemSet( pDoc->GetAttrPool(), aWhichIds );
            SvxCSS1PropertyInfo aPropInfo;

            sal_Bool bStyleParsed = ParseStyleOptions( pCurTable->GetStyle(),
                                                   pCurTable->GetId(),
                                                   pCurTable->GetClass(),
                                                   aItemSet, aPropInfo,
                                                      0, &pCurTable->GetDirection() );
            const SfxPoolItem *pItem = 0;
            if( bStyleParsed )
            {
                if( SFX_ITEM_SET == aItemSet.GetItemState(
                                        RES_BACKGROUND, false, &pItem ) )
                {
                    pCurTable->SetBGBrush( *(const SvxBrushItem *)pItem );
                    aItemSet.ClearItem( RES_BACKGROUND );
                }
                if( SFX_ITEM_SET == aItemSet.GetItemState(
                                        RES_PARATR_SPLIT, false, &pItem ) )
                {
                    aItemSet.Put(
                        SwFmtLayoutSplit( ((const SvxFmtSplitItem *)pItem)
                                                ->GetValue() ) );
                    aItemSet.ClearItem( RES_PARATR_SPLIT );
                }
            }

            
            sal_uInt16 nLeftSpace = 0;
            sal_uInt16 nRightSpace = 0;
            short nIndent;
            GetMarginsFromContextWithNumBul( nLeftSpace, nRightSpace, nIndent );

            
            SwPosition *pSavePos = 0;
            sal_Bool bForceFrame = sal_False;
            sal_Bool bAppended = sal_False;
            sal_Bool bParentLFStripped = sal_False;
            if( bTopTable )
            {
                SvxAdjust eTblAdjust = pTable->GetTableAdjust(sal_False);

                
                
                
                bForceFrame = eTblAdjust == SVX_ADJUST_LEFT ||
                              eTblAdjust == SVX_ADJUST_RIGHT ||
                              pCurTable->HasToFly();

                
                
                
                OSL_ENSURE( !bForceFrame || pCurTable->HasParentSection(),
                        "Tabelle im Rahmen hat keine Umgebung!" );

                sal_Bool bAppend = sal_False;
                if( bForceFrame )
                {
                    
                    
                    
                    bAppend = HasCurrentParaFlys(sal_True);
                }
                else
                {
                    
                    
                    
                    bAppend =
                        pPam->GetPoint()->nContent.GetIndex() ||
                        HasCurrentParaFlys() ||
                        HasCurrentParaBookmarks();
                }
                if( bAppend )
                {
                    if( !pPam->GetPoint()->nContent.GetIndex() )
                    {
                        
                        pDoc->SetTxtFmtColl( *pPam,
                            pCSS1Parser->GetTxtCollFromPool(RES_POOLCOLL_STANDARD) );
                        SvxFontHeightItem aFontHeight( 40, 100, RES_CHRATR_FONTSIZE );

                        _HTMLAttr* pTmp =
                            new _HTMLAttr( *pPam->GetPoint(), aFontHeight );
                        aSetAttrTab.push_back( pTmp );

                        SvxFontHeightItem aFontHeightCJK( 40, 100, RES_CHRATR_CJK_FONTSIZE );
                        pTmp =
                            new _HTMLAttr( *pPam->GetPoint(), aFontHeightCJK );
                        aSetAttrTab.push_back( pTmp );

                        SvxFontHeightItem aFontHeightCTL( 40, 100, RES_CHRATR_CTL_FONTSIZE );
                        pTmp =
                            new _HTMLAttr( *pPam->GetPoint(), aFontHeightCTL );
                        aSetAttrTab.push_back( pTmp );

                        pTmp = new _HTMLAttr( *pPam->GetPoint(),
                                            SvxULSpaceItem( 0, 0, RES_UL_SPACE ) );
                        aSetAttrTab.push_front( pTmp ); 
                                                        
                                                        
                    }
                    AppendTxtNode( AM_NOSPACE );
                    bAppended = sal_True;
                }
                else if( !aParaAttrs.empty() )
                {
                    if( !bForceFrame )
                    {
                        
                        
                        

                        for( sal_uInt16 i=0; i<aParaAttrs.size(); i++ )
                            aParaAttrs[i]->Invalidate();
                    }

                    aParaAttrs.clear();
                }

                pSavePos = new SwPosition( *pPam->GetPoint() );
            }
            else if( pCurTable->HasParentSection() )
            {
                bParentLFStripped = StripTrailingLF() > 0;

                
                nOpenParaToken = 0;
                nFontStHeadStart = nFontStMin;

                
                if( !aParaAttrs.empty() )
                    aParaAttrs.clear();
            }

            
            _HTMLTableContext *pTCntxt =
                        new _HTMLTableContext( pSavePos, nContextStMin,
                                               nContextStAttrMin );

            
            
            _HTMLAttrs *pPostIts = 0;
            if( !bForceFrame && (bTopTable || pCurTable->HasParentSection()) )
            {
                SplitAttrTab( pTCntxt->aAttrTab, bTopTable );
                
                
                
                
                
                
                
                if( (bTopTable && !bAppended) ||
                    (!bTopTable && !bParentLFStripped &&
                     !pPam->GetPoint()->nContent.GetIndex()) )
                    pPostIts = new _HTMLAttrs;
                SetAttr( bTopTable, bTopTable, pPostIts );
            }
            else
            {
                SaveAttrTab( pTCntxt->aAttrTab );
                if( bTopTable && !bAppended )
                {
                    pPostIts = new _HTMLAttrs;
                    SetAttr( sal_True, sal_True, pPostIts );
                }
            }
            bNoParSpace = false;

            
            pTCntxt->SetNumInfo( GetNumInfo() );
            GetNumInfo().Clear();
            pTCntxt->SavePREListingXMP( *this );

            if( bTopTable )
            {
                if( bForceFrame )
                {
                    

                    SfxItemSet aFrmSet( pDoc->GetAttrPool(),
                                        RES_FRMATR_BEGIN, RES_FRMATR_END-1 );
                    if( !pCurTable->IsNewDoc() )
                        Reader::ResetFrmFmtAttrs( aFrmSet );

                    SwSurround eSurround = SURROUND_NONE;
                    sal_Int16 eHori;

                    switch( pCurTable->GetTableAdjust(sal_True) )
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
                    default:
                        eHori = text::HoriOrientation::LEFT;
                        break;
                    }
                    SetAnchorAndAdjustment( text::VertOrientation::NONE, eHori, aFrmSet,
                                            sal_True );
                    aFrmSet.Put( SwFmtSurround(eSurround) );

                    SwFmtFrmSize aFrmSize( ATT_VAR_SIZE, 20*MM50, MINLAY );
                    aFrmSize.SetWidthPercent( 100 );
                    aFrmSet.Put( aFrmSize );

                    sal_uInt16 nSpace = pCurTable->GetHSpace();
                    if( nSpace )
                        aFrmSet.Put( SvxLRSpaceItem(nSpace,nSpace, 0, 0, RES_LR_SPACE) );
                    nSpace = pCurTable->GetVSpace();
                    if( nSpace )
                        aFrmSet.Put( SvxULSpaceItem(nSpace,nSpace, RES_UL_SPACE) );

                    RndStdIds eAnchorId = ((const SwFmtAnchor&)aFrmSet.
                                                Get( RES_ANCHOR )).
                                                GetAnchorId();
                    SwFrmFmt *pFrmFmt =  pDoc->MakeFlySection(
                                eAnchorId, pPam->GetPoint(), &aFrmSet );

                    pTCntxt->SetFrmFmt( pFrmFmt );
                    const SwFmtCntnt& rFlyCntnt = pFrmFmt->GetCntnt();
                    pPam->GetPoint()->nNode = *rFlyCntnt.GetCntntIdx();
                    SwCntntNode *pCNd =
                        pDoc->GetNodes().GoNext( &(pPam->GetPoint()->nNode) );
                    pPam->GetPoint()->nContent.Assign( pCNd, 0 );

                }

                
                
                
                
                OSL_ENSURE( !pPam->GetPoint()->nContent.GetIndex(),
                        "Der Absatz hinter der Tabelle ist nicht leer!" );
                const SwTable* pSwTable = pDoc->InsertTable(
                        SwInsertTableOptions( tabopts::HEADLINE_NO_BORDER, 1 ),
                        *pPam->GetPoint(), 1, 1, text::HoriOrientation::LEFT );

                if( bForceFrame )
                {
                    SwNodeIndex aDstIdx( pPam->GetPoint()->nNode );
                    pPam->Move( fnMoveBackward );
                    pDoc->GetNodes().Delete( aDstIdx );
                }
                else
                {
                    if( bStyleParsed )
                    {
                        pCSS1Parser->SetFmtBreak( aItemSet, aPropInfo );
                        pSwTable->GetFrmFmt()->SetFmtAttr( aItemSet );
                    }
                    pPam->Move( fnMoveBackward );
                }

                SwNode const*const pNd = & pPam->GetPoint()->nNode.GetNode();
                if( !bAppended && !bForceFrame )
                {
                    SwTxtNode *const pOldTxtNd =
                        pSavePos->nNode.GetNode().GetTxtNode();
                    OSL_ENSURE( pOldTxtNd, "Wieso stehen wir in keinem Txt-Node?" );
                    SwFrmFmt *pFrmFmt = pSwTable->GetFrmFmt();

                    const SfxPoolItem* pItem2;
                    if( SFX_ITEM_SET == pOldTxtNd->GetSwAttrSet()
                            .GetItemState( RES_PAGEDESC, false, &pItem2 ) &&
                        ((SwFmtPageDesc *)pItem2)->GetPageDesc() )
                    {
                        pFrmFmt->SetFmtAttr( *pItem2 );
                        pOldTxtNd->ResetAttr( RES_PAGEDESC );
                    }
                    if( SFX_ITEM_SET == pOldTxtNd->GetSwAttrSet()
                            .GetItemState( RES_BREAK, true, &pItem2 ) )
                    {
                        switch( ((SvxFmtBreakItem *)pItem2)->GetBreak() )
                        {
                        case SVX_BREAK_PAGE_BEFORE:
                        case SVX_BREAK_PAGE_AFTER:
                        case SVX_BREAK_PAGE_BOTH:
                            pFrmFmt->SetFmtAttr( *pItem2 );
                            pOldTxtNd->ResetAttr( RES_BREAK );
                        default:
                            ;
                        }
                    }
                }

                if( !bAppended && pPostIts )
                {
                    
                    
                    InsertAttrs( *pPostIts );
                    delete pPostIts;
                    pPostIts = 0;
                }

                pTCntxt->SetTableNode( (SwTableNode *)pNd->FindTableNode() );

                pCurTable->SetTable( pTCntxt->GetTableNode(), pTCntxt,
                                     nLeftSpace, nRightSpace,
                                     pSwTable, bForceFrame );

                OSL_ENSURE( !pPostIts, "ubenutzte PostIts" );
            }
            else
            {
                
                if( EndSections( bParentLFStripped ) )
                    bParentLFStripped = sal_False;

                if( pCurTable->HasParentSection() )
                {
                    
                    
                    
                    if( !bParentLFStripped )
                        StripTrailingPara();

                    if( pPostIts )
                    {
                        
                        
                        InsertAttrs( *pPostIts );
                        delete pPostIts;
                        pPostIts = 0;
                    }
                }

                SwNode const*const pNd = & pPam->GetPoint()->nNode.GetNode();
                const SwStartNode *pStNd = (pTable->bFirstCell ? pNd->FindTableNode()
                                                            : pNd->FindTableBoxStartNode() );

                pCurTable->SetTable( pStNd, pTCntxt, nLeftSpace, nRightSpace );
            }

            
            
            
            
            nContextStMin = aContexts.size();
            nContextStAttrMin = nContextStMin;
        }

        pSaveStruct = new _CellSaveStruct( *this, pCurTable, bHead,
                                            bReadOptions );

        
        
        SaveState( 0 );
    }

    if( !nToken )
        nToken = GetNextToken();    

    sal_Bool bDone = sal_False;
    while( (IsParserWorking() && !bDone) || bPending )
    {
        SaveState( nToken );

        nToken = FilterToken( nToken );

        OSL_ENSURE( pPendStack || !bCallNextToken || pSaveStruct->IsInSection(),
                "Wo ist die Section gebieben?" );
        if( !pPendStack && bCallNextToken && pSaveStruct->IsInSection() )
        {
            
            
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
            SkipToken(-1);
        case HTML_TABLEHEADER_OFF:
        case HTML_TABLEDATA_OFF:
            bDone = sal_True;
            break;
        case HTML_TABLE_ON:
            {
                sal_Bool bTopTable = sal_False;
                sal_Bool bHasToFly = sal_False;
                SvxAdjust eTabAdjust = SVX_ADJUST_END;
                if( !pPendStack )
                {
                    
                    
                    
                    pSaveStruct->pTable = pTable;

                    
                    
                    if( !pSaveStruct->IsInSection() )
                    {
                        
                        
                        sal_Bool bNeedsSection = sal_False;
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
                        
                        
                        
                        
                        bTopTable = (0 ==
                            pPam->GetPoint()->nNode.GetNode().FindTableNode());

                        
                        
                        bHasToFly = HasCurrentParaFlys(sal_False,sal_True);
                    }

                    
                    eTabAdjust = aAttrTab.pAdjust
                        ? ((const SvxAdjustItem&)aAttrTab.pAdjust->GetItem()).
                                                 GetAdjust()
                        : SVX_ADJUST_END;
                }

                HTMLTable *pSubTable = BuildTable( eTabAdjust,
                                                   bHead,
                                                   pSaveStruct->IsInSection(),
                                                   bTopTable, bHasToFly );
                if( SVPAR_PENDING != GetStatus() )
                {
                    
                    if( pSubTable )
                    {
                        OSL_ENSURE( pSubTable->GetTableAdjust(sal_False)!= SVX_ADJUST_LEFT &&
                                pSubTable->GetTableAdjust(sal_False)!= SVX_ADJUST_RIGHT,
                                "links oder rechts ausgerichtete Tabellen gehoehren in Rahmen" );


                        HTMLTableCnts *pParentContents =
                            pSubTable->GetParentContents();
                        if( pParentContents )
                        {
                            OSL_ENSURE( !pSaveStruct->IsInSection(),
                                    "Wo ist die Section geblieben" );

                            
                            
                            pSaveStruct->AddContents( pParentContents );
                        }

                        const SwStartNode *pCapStNd =
                                pSubTable->GetCaptionStartNode();

                        if( pSubTable->GetContext() )
                        {
                            OSL_ENSURE( !pSubTable->GetContext()->GetFrmFmt(),
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

                            
                            pSaveStruct->ClearIsInSection();
                        }
                        else if( pCapStNd )
                        {
                            
                            
                            
                            pSaveStruct->AddContents(
                                new HTMLTableCnts(pCapStNd) );

                            
                            pSaveStruct->ClearIsInSection();
                        }
                    }

                    pTable = pSaveStruct->pTable;
                }
            }
            break;

        case HTML_NOBR_ON:
            
            pSaveStruct->StartNoBreak( *pPam->GetPoint() );
            break;

        case HTML_NOBR_OFF:
                pSaveStruct->EndNoBreak( *pPam->GetPoint() );
            break;

        case HTML_COMMENT:
            
            
            
            NextToken( nToken );
            break;

        case HTML_MARQUEE_ON:
            if( !pSaveStruct->IsInSection() )
            {
                
                pSaveStruct->AddContents(
                    InsertTableContents( bHead ) );
            }
            bCallNextToken = true;
            NewMarquee( pCurTable );
            break;

        case HTML_TEXTTOKEN:
            
            if( !pSaveStruct->IsInSection() && 1==aToken.getLength() &&
                ' '==aToken[0] )
                break;
        default:
            if( !pSaveStruct->IsInSection() )
            {
                
                pSaveStruct->AddContents(
                    InsertTableContents( bHead ) );
            }

            if( IsParserWorking() || bPending )
                NextToken( nToken );
            break;
        }

        OSL_ENSURE( !bPending || !pPendStack,
                "SwHTMLParser::BuildTableCell: Es gibt wieder einen Pend-Stack" );
        bPending = sal_False;
        if( IsParserWorking() )
            SaveState( 0 );

        if( !bDone )
            nToken = GetNextToken();
    }

    if( SVPAR_PENDING == GetStatus() )
    {
        pPendStack = new SwPendingStack( bHead ? HTML_TABLEHEADER_ON
                                               : HTML_TABLEDATA_ON, pPendStack );
        pPendStack->pData = pSaveStruct;

        return;
    }

    
    
    
    
    
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
        SwCntntNode *pCNd = pDoc->GetNodes()[pEndNd->GetIndex()-1] ->GetCntntNode();
        
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
        pSaveStruct->CheckNoBreak( *pPam->GetPoint(), pDoc );

        
        
        
        while( (sal_uInt16)aContexts.size() > nContextStAttrMin+1 )
        {
            _HTMLAttrContext *pCntxt = PopContext();
            EndContext( pCntxt );
            delete pCntxt;
        }

        
        if( StripTrailingLF()==0 && !pPam->GetPoint()->nContent.GetIndex() )
            StripTrailingPara();

        
        
        _HTMLAttrContext *pCntxt = PopContext();
        EndContext( pCntxt );
        delete pCntxt;
    }
    else
    {
        
        while( aContexts.size() > nContextStAttrMin )
        {
            _HTMLAttrContext *pCntxt = PopContext();
            ClearContext( pCntxt );
            delete pCntxt;
        }
    }

    
    GetNumInfo().Clear();

    SetAttr( sal_False );

    pSaveStruct->InsertCell( *this, pCurTable );

    
    delete pSaveStruct;
}


class _RowSaveStruct : public SwPendingStackData
{
public:
    SvxAdjust eAdjust;
    sal_Int16 eVertOri;
    sal_Bool bHasCells;

    _RowSaveStruct() :
        eAdjust( SVX_ADJUST_END ), eVertOri( text::VertOrientation::TOP ), bHasCells( sal_False )
    {}
};


void SwHTMLParser::BuildTableRow( HTMLTable *pCurTable, sal_Bool bReadOptions,
                                  SvxAdjust eGrpAdjust,
                                  sal_Int16 eGrpVertOri )
{
    

    if( !IsParserWorking() && !pPendStack )
        return;

    int nToken = 0;
    _RowSaveStruct* pSaveStruct;

    sal_Bool bPending = sal_False;
    if( pPendStack )
    {
        pSaveStruct = (_RowSaveStruct*)pPendStack->pData;

        SwPendingStack* pTmp = pPendStack->pNext;
        delete pPendStack;
        pPendStack = pTmp;
        nToken = pPendStack ? pPendStack->nToken : GetSaveToken();
        bPending = SVPAR_ERROR == eState && pPendStack != 0;

        SaveState( nToken );
    }
    else
    {
        SvxAdjust eAdjust = eGrpAdjust;
        sal_Int16 eVertOri = eGrpVertOri;
        Color aBGColor;
        OUString aBGImage, aStyle, aId, aClass;
        sal_Bool bBGColor = sal_False;
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
                                    aHTMLTblVAlignTable, eVertOri );
                    break;
                case HTML_O_BGCOLOR:
                    
                    
                    if( !rOption.GetString().isEmpty() )
                    {
                        rOption.GetColor( aBGColor );
                        bBGColor = sal_True;
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
            CreateBrushItem( bBGColor ? &aBGColor : 0, aBGImage, aStyle,
                             aId, aClass );
        pCurTable->OpenRow( eAdjust, eVertOri, pBrushItem );
        
        
        SaveState( 0 );
    }

    if( !nToken )
        nToken = GetNextToken();    

    sal_Bool bDone = sal_False;
    while( (IsParserWorking() && !bDone) || bPending )
    {
        SaveState( nToken );

        nToken = FilterToken( nToken );

        OSL_ENSURE( pPendStack || !bCallNextToken ||
                pCurTable->GetContext() || pCurTable->HasParentSection(),
                "Wo ist die Section gebieben?" );
        if( !pPendStack && bCallNextToken &&
            (pCurTable->GetContext() || pCurTable->HasParentSection()) )
        {
            
            
            NextToken( nToken );
        }
        else switch( nToken )
        {
        case HTML_TABLE_ON:
            if( !pCurTable->GetContext()  )
            {
                SkipToken( -1 );
                bDone = sal_True;
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
            SkipToken( -1 );
        case HTML_TABLEROW_OFF:
            bDone = sal_True;
            break;
        case HTML_TABLEHEADER_ON:
        case HTML_TABLEDATA_ON:
            BuildTableCell( pCurTable, sal_True, HTML_TABLEHEADER_ON==nToken );
            if( SVPAR_PENDING != GetStatus() )
            {
                pSaveStruct->bHasCells = sal_True;
                bDone = pTable->IsOverflowing();
            }
            break;
        case HTML_CAPTION_ON:
            BuildTableCaption( pCurTable );
            bDone = pTable->IsOverflowing();
            break;
        case HTML_CAPTION_OFF:
        case HTML_TABLEHEADER_OFF:
        case HTML_TABLEDATA_OFF:
        case HTML_COLGROUP_ON:
        case HTML_COLGROUP_OFF:
        case HTML_COL_ON:
        case HTML_COL_OFF:
            
            
            
            break;
        case HTML_MULTICOL_ON:
            
            break;
        case HTML_FORM_ON:
            NewForm( sal_False );   
            break;
        case HTML_FORM_OFF:
            EndForm( sal_False );   
            break;
        case HTML_COMMENT:
            NextToken( nToken );
            break;
        case HTML_MAP_ON:
            
            
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

        OSL_ENSURE( !bPending || !pPendStack,
                "SwHTMLParser::BuildTableRow: Es gibt wieder einen Pend-Stack" );
        bPending = sal_False;
        if( IsParserWorking() )
            SaveState( 0 );

        if( !bDone )
            nToken = GetNextToken();
    }

    if( SVPAR_PENDING == GetStatus() )
    {
        pPendStack = new SwPendingStack( HTML_TABLEROW_ON, pPendStack );
        pPendStack->pData = pSaveStruct;
    }
    else
    {
        pCurTable->CloseRow( !pSaveStruct->bHasCells );
        delete pSaveStruct;
    }

    
}

void SwHTMLParser::BuildTableSection( HTMLTable *pCurTable,
                                      sal_Bool bReadOptions,
                                      sal_Bool bHead )
{
    
    if( !IsParserWorking() && !pPendStack )
        return;

    int nToken = 0;
    sal_Bool bPending = sal_False;
    _RowSaveStruct* pSaveStruct;

    if( pPendStack )
    {
        pSaveStruct = (_RowSaveStruct*)pPendStack->pData;

        SwPendingStack* pTmp = pPendStack->pNext;
        delete pPendStack;
        pPendStack = pTmp;
        nToken = pPendStack ? pPendStack->nToken : GetSaveToken();
        bPending = SVPAR_ERROR == eState && pPendStack != 0;

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
                        rOption.GetEnum( aHTMLTblVAlignTable,
                                          pSaveStruct->eVertOri );
                    break;
                }
            }
        }

        
        
        SaveState( 0 );
    }

    if( !nToken )
        nToken = GetNextToken();    

    sal_Bool bDone = sal_False;
    while( (IsParserWorking() && !bDone) || bPending )
    {
        SaveState( nToken );

        nToken = FilterToken( nToken );

        OSL_ENSURE( pPendStack || !bCallNextToken ||
                pCurTable->GetContext() || pCurTable->HasParentSection(),
                "Wo ist die Section gebieben?" );
        if( !pPendStack && bCallNextToken &&
            (pCurTable->GetContext() || pCurTable->HasParentSection()) )
        {
            
            
            NextToken( nToken );
        }
        else switch( nToken )
        {
        case HTML_TABLE_ON:
            if( !pCurTable->GetContext()  )
            {
                SkipToken( -1 );
                bDone = sal_True;
            }

            break;
        case HTML_THEAD_ON:
        case HTML_TFOOT_ON:
        case HTML_TBODY_ON:
        case HTML_TABLE_OFF:
            SkipToken( -1 );
        case HTML_THEAD_OFF:
        case HTML_TBODY_OFF:
        case HTML_TFOOT_OFF:
            bDone = sal_True;
            break;
        case HTML_CAPTION_ON:
            BuildTableCaption( pCurTable );
            bDone = pTable->IsOverflowing();
            break;
        case HTML_CAPTION_OFF:
            break;
        case HTML_TABLEHEADER_ON:
        case HTML_TABLEDATA_ON:
            SkipToken( -1 );
            BuildTableRow( pCurTable, sal_False, pSaveStruct->eAdjust,
                           pSaveStruct->eVertOri );
            bDone = pTable->IsOverflowing();
            break;
        case HTML_TABLEROW_ON:
            BuildTableRow( pCurTable, sal_True, pSaveStruct->eAdjust,
                           pSaveStruct->eVertOri );
            bDone = pTable->IsOverflowing();
            break;
        case HTML_MULTICOL_ON:
            
            break;
        case HTML_FORM_ON:
            NewForm( sal_False );   
            break;
        case HTML_FORM_OFF:
            EndForm( sal_False );   
            break;
        case HTML_TEXTTOKEN:
            
            if( (pCurTable->GetContext() ||
                 !pCurTable->HasParentSection()) &&
                1==aToken.getLength() && ' ' == aToken[0] )
                break;
        default:
            pCurTable->MakeParentContents();
            NextToken( nToken );
        }

        OSL_ENSURE( !bPending || !pPendStack,
                "SwHTMLParser::BuildTableSection: Es gibt wieder einen Pend-Stack" );
        bPending = sal_False;
        if( IsParserWorking() )
            SaveState( 0 );

        if( !bDone )
            nToken = GetNextToken();
    }

    if( SVPAR_PENDING == GetStatus() )
    {
        pPendStack = new SwPendingStack( bHead ? HTML_THEAD_ON
                                               : HTML_TBODY_ON, pPendStack );
        pPendStack->pData = pSaveStruct;
    }
    else
    {
        pCurTable->CloseSection( bHead );
        delete pSaveStruct;
    }

    
}

struct _TblColGrpSaveStruct : public SwPendingStackData
{
    sal_uInt16 nColGrpSpan;
    sal_uInt16 nColGrpWidth;
    sal_Bool bRelColGrpWidth;
    SvxAdjust eColGrpAdjust;
    sal_Int16 eColGrpVertOri;

    inline _TblColGrpSaveStruct();


    inline void CloseColGroup( HTMLTable *pTable );
};

inline _TblColGrpSaveStruct::_TblColGrpSaveStruct() :
    nColGrpSpan( 1 ), nColGrpWidth( 0 ),
    bRelColGrpWidth( sal_False ), eColGrpAdjust( SVX_ADJUST_END ),
    eColGrpVertOri( text::VertOrientation::TOP )
{}


inline void _TblColGrpSaveStruct::CloseColGroup( HTMLTable *pTable )
{
    pTable->CloseColGroup( nColGrpSpan, nColGrpWidth,
                            bRelColGrpWidth, eColGrpAdjust, eColGrpVertOri );
}

void SwHTMLParser::BuildTableColGroup( HTMLTable *pCurTable,
                                       sal_Bool bReadOptions )
{
    

    if( !IsParserWorking() && !pPendStack )
        return;

    int nToken = 0;
    sal_Bool bPending = sal_False;
    _TblColGrpSaveStruct* pSaveStruct;

    if( pPendStack )
    {
        pSaveStruct = (_TblColGrpSaveStruct*)pPendStack->pData;

        SwPendingStack* pTmp = pPendStack->pNext;
        delete pPendStack;
        pPendStack = pTmp;
        nToken = pPendStack ? pPendStack->nToken : GetSaveToken();
        bPending = SVPAR_ERROR == eState && pPendStack != 0;

        SaveState( nToken );
    }
    else
    {

        pSaveStruct = new _TblColGrpSaveStruct;
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
                        rOption.GetEnum( aHTMLTblVAlignTable,
                                                pSaveStruct->eColGrpVertOri );
                    break;
                }
            }
        }
        
        
        SaveState( 0 );
    }

    if( !nToken )
        nToken = GetNextToken();    

    sal_Bool bDone = sal_False;
    while( (IsParserWorking() && !bDone) || bPending )
    {
        SaveState( nToken );

        nToken = FilterToken( nToken );

        OSL_ENSURE( pPendStack || !bCallNextToken ||
                pCurTable->GetContext() || pCurTable->HasParentSection(),
                "Wo ist die Section gebieben?" );
        if( !pPendStack && bCallNextToken &&
            (pCurTable->GetContext() || pCurTable->HasParentSection()) )
        {
            
            
            NextToken( nToken );
        }
        else switch( nToken )
        {
        case HTML_TABLE_ON:
            if( !pCurTable->GetContext()  )
            {
                SkipToken( -1 );
                bDone = sal_True;
            }

            break;
        case HTML_COLGROUP_ON:
        case HTML_THEAD_ON:
        case HTML_TFOOT_ON:
        case HTML_TBODY_ON:
        case HTML_TABLEROW_ON:
        case HTML_TABLE_OFF:
            SkipToken( -1 );
        case HTML_COLGROUP_OFF:
            bDone = sal_True;
            break;
        case HTML_COL_ON:
            {
                sal_uInt16 nColSpan = 1;
                sal_uInt16 nColWidth = pSaveStruct->nColGrpWidth;
                sal_Bool bRelColWidth = pSaveStruct->bRelColGrpWidth;
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
                            rOption.GetEnum( aHTMLTblVAlignTable,
                                                        eColVertOri );
                        break;
                    }
                }
                pCurTable->InsertCol( nColSpan, nColWidth, bRelColWidth,
                                      eColAdjust, eColVertOri );

                
                
                pSaveStruct->nColGrpSpan = 0;
            }
            break;
        case HTML_COL_OFF:
            break;      
        case HTML_MULTICOL_ON:
            
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

        OSL_ENSURE( !bPending || !pPendStack,
                "SwHTMLParser::BuildTableColGrp: Es gibt wieder einen Pend-Stack" );
        bPending = sal_False;
        if( IsParserWorking() )
            SaveState( 0 );

        if( !bDone )
            nToken = GetNextToken();
    }

    if( SVPAR_PENDING == GetStatus() )
    {
        pPendStack = new SwPendingStack( HTML_COL_ON, pPendStack );
        pPendStack->pData = pSaveStruct;
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
    SwHTMLNumRuleInfo aNumRuleInfo; 

public:

    _HTMLAttrTable aAttrTab;        

    _CaptionSaveStruct( SwHTMLParser& rParser, const SwPosition& rPos ) :
        _SectionSaveStruct( rParser ), aSavePos( rPos )
    {
        rParser.SaveAttrTab( aAttrTab );

        
        
        aNumRuleInfo.Set( rParser.GetNumInfo() );
        rParser.GetNumInfo().Clear();
    }

    const SwPosition& GetPos() const { return aSavePos; }

    void RestoreAll( SwHTMLParser& rParser )
    {
        
        Restore( rParser );

        
        rParser.RestoreAttrTab( aAttrTab );

        
        rParser.GetNumInfo().Set( aNumRuleInfo );
    }

    virtual ~_CaptionSaveStruct();
};

_CaptionSaveStruct::~_CaptionSaveStruct()
{}

void SwHTMLParser::BuildTableCaption( HTMLTable *pCurTable )
{
    

    if( !IsParserWorking() && !pPendStack )
        return;

    int nToken = 0;
    _CaptionSaveStruct* pSaveStruct;

    if( pPendStack )
    {
        pSaveStruct = (_CaptionSaveStruct*)pPendStack->pData;

        SwPendingStack* pTmp = pPendStack->pNext;
        delete pPendStack;
        pPendStack = pTmp;
        nToken = pPendStack ? pPendStack->nToken : GetSaveToken();
        OSL_ENSURE( !pPendStack, "Wo kommt hier ein Pending-Stack her?" );

        SaveState( nToken );
    }
    else
    {
        if( pTable->IsOverflowing() )
        {
            SaveState( 0 );
            return;
        }

        sal_Bool bTop = sal_True;
        const HTMLOptions& rHTMLOptions = GetOptions();
        for ( size_t i = rHTMLOptions.size(); i; )
        {
            const HTMLOption& rOption = rHTMLOptions[--i];
            if( HTML_O_ALIGN == rOption.GetToken() )
            {
                if (rOption.GetString().equalsIgnoreAsciiCase(
                        OOO_STRING_SVTOOLS_HTML_VA_bottom))
                {
                    bTop = sal_False;
                }
            }
        }

        
        pSaveStruct = new _CaptionSaveStruct( *this, *pPam->GetPoint() );

        
        
        const SwStartNode *pStNd;
        if( pTable == pCurTable )
            pStNd = InsertTempTableCaptionSection();
        else
            pStNd = InsertTableSection( RES_POOLCOLL_TEXT );

        _HTMLAttrContext *pCntxt = new _HTMLAttrContext( HTML_CAPTION_ON );

        
        NewAttr( &aAttrTab.pAdjust, SvxAdjustItem(SVX_ADJUST_CENTER, RES_PARATR_ADJUST) );

        _HTMLAttrs &rAttrs = pCntxt->GetAttrs();
        rAttrs.push_back( aAttrTab.pAdjust );

        PushContext( pCntxt );

        
        pCurTable->SetCaption( pStNd, bTop );

        
        
        SaveState( 0 );
    }

    if( !nToken )
        nToken = GetNextToken();    

    
    sal_Bool bDone = sal_False;
    while( IsParserWorking() && !bDone )
    {
        SaveState( nToken );

        nToken = FilterToken( nToken );

        switch( nToken )
        {
        case HTML_TABLE_ON:
            if( !pPendStack )
            {
                pSaveStruct->pTable = pTable;
                sal_Bool bHasToFly = pSaveStruct->pTable!=pCurTable;
                BuildTable( pCurTable->GetTableAdjust( sal_True ),
                            sal_False, sal_True, sal_True, bHasToFly );
            }
            else
            {
                BuildTable( SVX_ADJUST_END );
            }
            if( SVPAR_PENDING != GetStatus() )
            {
                pTable = pSaveStruct->pTable;
            }
            break;
        case HTML_TABLE_OFF:
        case HTML_COLGROUP_ON:
        case HTML_THEAD_ON:
        case HTML_TFOOT_ON:
        case HTML_TBODY_ON:
        case HTML_TABLEROW_ON:
            SkipToken( -1 );
            bDone = sal_True;
            break;

        case HTML_CAPTION_OFF:
            bDone = sal_True;
            break;
        default:
            if( pPendStack )
            {
                SwPendingStack* pTmp = pPendStack->pNext;
                delete pPendStack;
                pPendStack = pTmp;

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
        pPendStack = new SwPendingStack( HTML_CAPTION_ON, pPendStack );
        pPendStack->pData = pSaveStruct;
        return;
    }

    
    while( (sal_uInt16)aContexts.size() > nContextStAttrMin+1 )
    {
        _HTMLAttrContext *pCntxt = PopContext();
        EndContext( pCntxt );
        delete pCntxt;
    }

    
    sal_Bool bLFStripped = StripTrailingLF() > 0;

    if( pTable==pCurTable )
    {
        
        
        
        
        if( pPam->GetPoint()->nContent.GetIndex() || bLFStripped )
            AppendTxtNode( AM_NOSPACE );
    }
    else
    {
        
        if( !pPam->GetPoint()->nContent.GetIndex() && !bLFStripped )
            StripTrailingPara();
    }

    
    
    _HTMLAttrContext *pCntxt = PopContext();
    EndContext( pCntxt );
    delete pCntxt;

    SetAttr( sal_False );

    
    pSaveStruct->RestoreAll( *this );

    
    *pPam->GetPoint() = pSaveStruct->GetPos();

    delete pSaveStruct;
}

class _TblSaveStruct : public SwPendingStackData
{
public:
    HTMLTable *pCurTable;

    _TblSaveStruct( HTMLTable *pCurTbl ) :
        pCurTable( pCurTbl )
    {}

    virtual ~_TblSaveStruct();

    
    
    
    void MakeTable( sal_uInt16 nWidth, SwPosition& rPos, SwDoc *pDoc );
};

_TblSaveStruct::~_TblSaveStruct()
{}


void _TblSaveStruct::MakeTable( sal_uInt16 nWidth, SwPosition& rPos, SwDoc *pDoc )
{
    pCurTable->MakeTable( 0, nWidth );

    _HTMLTableContext *pTCntxt = pCurTable->GetContext();
    OSL_ENSURE( pTCntxt, "Wo ist der Tabellen-Kontext" );

    SwTableNode *pTblNd = pTCntxt->GetTableNode();
    OSL_ENSURE( pTblNd, "Wo ist der Tabellen-Node" );

    if( pDoc->GetCurrentViewShell() && pTblNd )
    {
        
        

        if( pTCntxt->GetFrmFmt() )
        {
            pTCntxt->GetFrmFmt()->DelFrms();
            pTblNd->DelFrms();
            pTCntxt->GetFrmFmt()->MakeFrms();
        }
        else
        {
            pTblNd->DelFrms();
            SwNodeIndex aIdx( *pTblNd->EndOfSectionNode(), 1 );
            OSL_ENSURE( aIdx.GetIndex() <= pTCntxt->GetPos()->nNode.GetIndex(),
                    "unerwarteter Node fuer das Tabellen-Layout" );
            pTblNd->MakeFrms( &aIdx );
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
    bPrcWidth( sal_False ),
    bTableAdjust( sal_False ),
    bBGColor( sal_False ),
    aBorderColor( COL_GRAY )
{
    sal_Bool bBorderColor = sal_False;
    sal_Bool bHasFrame = sal_False, bHasRules = sal_False;

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
                nHeight = 0;    
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
                    bTableAdjust = sal_True;
                }
            }
            break;
        case HTML_O_VALIGN:
            eVertOri = rOption.GetEnum( aHTMLTblVAlignTable, eVertOri );
            break;
        case HTML_O_BORDER:
            
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
            bHasFrame = sal_True;
            break;
        case HTML_O_RULES:
            eRules = rOption.GetTableRules();
            bHasRules = sal_True;
            break;
        case HTML_O_BGCOLOR:
            
            
            if( !rOption.GetString().isEmpty() )
            {
                rOption.GetColor( aBGColor );
                bBGColor = sal_True;
            }
            break;
        case HTML_O_BACKGROUND:
            aBGImage = rOption.GetString();
            break;
        case HTML_O_BORDERCOLOR:
            rOption.GetColor( aBorderColor );
            bBorderColor = sal_True;
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
        bPrcWidth = sal_True;
    }

    
    
    if( 0==nBorder || USHRT_MAX==nBorder )
    {
        eFrame = HTML_TF_VOID;
        eRules = HTML_TR_NONE;
    }
}


HTMLTable *SwHTMLParser::BuildTable( SvxAdjust eParentAdjust,
                                     sal_Bool bIsParentHead,
                                     sal_Bool bHasParentSection,
                                     sal_Bool bMakeTopSubTable,
                                     sal_Bool bHasToFly )
{
    if( !IsParserWorking() && !pPendStack )
        return 0;

    int nToken = 0;
    sal_Bool bPending = sal_False;
    _TblSaveStruct* pSaveStruct;

    if( pPendStack )
    {
        pSaveStruct = (_TblSaveStruct*)pPendStack->pData;

        SwPendingStack* pTmp = pPendStack->pNext;
        delete pPendStack;
        pPendStack = pTmp;
        nToken = pPendStack ? pPendStack->nToken : GetSaveToken();
        bPending = SVPAR_ERROR == eState && pPendStack != 0;

        SaveState( nToken );
    }
    else
    {
        pTable = 0;
        HTMLTableOptions *pTblOptions =
            new HTMLTableOptions( GetOptions(), eParentAdjust );

        if( !pTblOptions->aId.isEmpty() )
            InsertBookmark( pTblOptions->aId );

        HTMLTable *pCurTable = new HTMLTable( this, pTable,
                                              bIsParentHead,
                                              bHasParentSection,
                                              bMakeTopSubTable,
                                              bHasToFly,
                                              pTblOptions );
        if( !pTable )
            pTable = pCurTable;

        pSaveStruct = new _TblSaveStruct( pCurTable );

        delete pTblOptions;

        
        
        SaveState( 0 );
    }

    HTMLTable *pCurTable = pSaveStruct->pCurTable;

    
    if( !nToken )
        nToken = GetNextToken();    

    sal_Bool bDone = sal_False;
    while( (IsParserWorking() && !bDone) || bPending )
    {
        SaveState( nToken );

        nToken = FilterToken( nToken );

        OSL_ENSURE( pPendStack || !bCallNextToken ||
                pCurTable->GetContext() || pCurTable->HasParentSection(),
                "Wo ist die Section gebieben?" );
        if( !pPendStack && bCallNextToken &&
            (pCurTable->GetContext() || pCurTable->HasParentSection()) )
        {
            
            
            NextToken( nToken );
        }
        else switch( nToken )
        {
        case HTML_TABLE_ON:
            if( !pCurTable->GetContext() )
            {
                
                
                SkipToken( -1 );
                bDone = sal_True;
            }

            break;
        case HTML_TABLE_OFF:
            bDone = sal_True;
            break;
        case HTML_CAPTION_ON:
            BuildTableCaption( pCurTable );
            bDone = pTable->IsOverflowing();
            break;
        case HTML_COL_ON:
            SkipToken( -1 );
            BuildTableColGroup( pCurTable, sal_False );
            break;
        case HTML_COLGROUP_ON:
            BuildTableColGroup( pCurTable, sal_True );
            break;
        case HTML_TABLEROW_ON:
        case HTML_TABLEHEADER_ON:
        case HTML_TABLEDATA_ON:
            SkipToken( -1 );
            BuildTableSection( pCurTable, sal_False, sal_False );
            bDone = pTable->IsOverflowing();
            break;
        case HTML_THEAD_ON:
        case HTML_TFOOT_ON:
        case HTML_TBODY_ON:
            BuildTableSection( pCurTable, sal_True, HTML_THEAD_ON==nToken );
            bDone = pTable->IsOverflowing();
            break;
        case HTML_MULTICOL_ON:
            
            break;
        case HTML_FORM_ON:
            NewForm( sal_False );   
            break;
        case HTML_FORM_OFF:
            EndForm( sal_False );   
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

        OSL_ENSURE( !bPending || !pPendStack,
                "SwHTMLParser::BuildTable: Es gibt wieder einen Pend-Stack" );
        bPending = sal_False;
        if( IsParserWorking() )
            SaveState( 0 );

        if( !bDone )
            nToken = GetNextToken();
    }

    if( SVPAR_PENDING == GetStatus() )
    {
        pPendStack = new SwPendingStack( HTML_TABLE_ON, pPendStack );
        pPendStack->pData = pSaveStruct;
        return 0;
    }

    _HTMLTableContext *pTCntxt = pCurTable->GetContext();
    if( pTCntxt )
    {
        

        
        pCurTable->CloseTable();

        
        
        
        while( aContexts.size() > nContextStAttrMin )
        {
            _HTMLAttrContext *pCntxt = PopContext();
            ClearContext( pCntxt );
            delete pCntxt;
        }

        nContextStMin = pTCntxt->GetContextStMin();
        nContextStAttrMin = pTCntxt->GetContextStAttrMin();

        if( pTable==pCurTable )
        {
            
            const SwStartNode *pCapStNd = pTable->GetCaptionStartNode();
            if( pCapStNd )
            {
                
                

                if( pCapStNd->EndOfSectionIndex() - pCapStNd->GetIndex() > 2 )
                {
                    
                    SwNodeRange aSrcRg( *pCapStNd, 1,
                                    *pCapStNd->EndOfSectionNode(), -1 );

                    sal_Bool bTop = pTable->IsTopCaption();
                    SwStartNode *pTblStNd = pTCntxt->GetTableNode();

                    OSL_ENSURE( pTblStNd, "Wo ist der Tabellen-Node" );
                    OSL_ENSURE( pTblStNd==pPam->GetNode()->FindTableNode(),
                            "Stehen wir in der falschen Tabelle?" );

                    SwNode* pNd;
                    if( bTop )
                        pNd = pTblStNd;
                    else
                        pNd = pTblStNd->EndOfSectionNode();
                    SwNodeIndex aDstIdx( *pNd, bTop ? 0 : 1 );

                    pDoc->MoveNodeRange( aSrcRg, aDstIdx,
                        IDocumentContentOperations::DOC_MOVEDEFAULT );

                    
                    
                    
                    
                    
                    if( bTop )
                    {
                        MovePageDescAttrs( pTblStNd, aSrcRg.aStart.GetIndex(),
                                           sal_False );
                    }
                }

                
                pPam->SetMark();
                pPam->DeleteMark();
                pDoc->DeleteSection( (SwStartNode *)pCapStNd );
                pTable->SetCaption( 0, sal_False );
            }

            
            sal_uInt16 nBrowseWidth = (sal_uInt16)GetCurrentBrowseWidth();
            pSaveStruct->MakeTable( nBrowseWidth, *pPam->GetPoint(), pDoc );
        }

        GetNumInfo().Set( pTCntxt->GetNumInfo() );
        pTCntxt->RestorePREListingXMP( *this );
        RestoreAttrTab( pTCntxt->aAttrTab );

        if( pTable==pCurTable )
        {
            
            bUpperSpace = true;
            SetTxtCollAttrs();

            nParaCnt = nParaCnt - std::min(nParaCnt,
                pTCntxt->GetTableNode()->GetTable().GetTabSortBoxes().size());

            
            if( JUMPTO_TABLE == eJumpTo && pTable->GetSwTable() &&
                pTable->GetSwTable()->GetFrmFmt()->GetName() == sJmpMark )
            {
                bChkJumpMark = true;
                eJumpTo = JUMPTO_NONE;
            }

            
            
            
            
            
            
            if( !nParaCnt && SVPAR_WORKING == GetStatus() )
                Show();
        }
    }
    else if( pTable==pCurTable )
    {
        

        
        const SwStartNode *pCapStNd = pCurTable->GetCaptionStartNode();
        if( pCapStNd )
        {
            pPam->SetMark();
            pPam->DeleteMark();
            pDoc->DeleteSection( (SwStartNode *)pCapStNd );
            pCurTable->SetCaption( 0, sal_False );
        }
    }

    if( pTable == pCurTable  )
    {
        delete pSaveStruct->pCurTable;
        pSaveStruct->pCurTable = 0;
        pTable = 0;
    }

    HTMLTable* pRetTbl = pSaveStruct->pCurTable;
    delete pSaveStruct;

    return pRetTbl;
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
