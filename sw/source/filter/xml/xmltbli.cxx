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

#include <limits.h>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/text/XTextTable.hpp>
#include <com/sun/star/table/XCellRange.hpp>
#include <svl/itemset.hxx>
#include <svl/zformat.hxx>
#include <sax/tools/converter.hxx>
#include <xmloff/xmlnmspe.hxx>
#include <xmloff/xmltkmap.hxx>
#include <xmloff/nmspmap.hxx>

#include <xmloff/families.hxx>
#include <xmloff/xmluconv.hxx>
#include <xmloff/i18nmap.hxx>
#include <editeng/protitem.hxx>
#include "poolfmt.hxx"
#include "fmtfsize.hxx"
#include "fmtornt.hxx"
#include "fmtfordr.hxx"
#include "doc.hxx"
#include "swtable.hxx"
#include "swtblfmt.hxx"
#include "pam.hxx"
#include "unotbl.hxx"
#include "unotextrange.hxx"
#include "unocrsr.hxx"
#include "cellatr.hxx"
#include "swddetbl.hxx"
#include "ddefld.hxx"
#include <sfx2/linkmgr.hxx>
#include "xmlimp.hxx"
#include "xmltbli.hxx"


#include <vcl/svapp.hxx>
#include <osl/mutex.hxx>
#include "ndtxt.hxx"

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::text;
using namespace ::com::sun::star::frame;
using namespace ::com::sun::star::table;
using namespace ::com::sun::star::xml::sax;
using namespace ::xmloff::token;
using ::boost::unordered_map;

enum SwXMLTableElemTokens
{
    XML_TOK_TABLE_HEADER_COLS,
    XML_TOK_TABLE_COLS,
    XML_TOK_TABLE_COL,
    XML_TOK_TABLE_HEADER_ROWS,
    XML_TOK_TABLE_ROWS,
    XML_TOK_TABLE_ROW,
    XML_TOK_OFFICE_DDE_SOURCE,
    XML_TOK_TABLE_ELEM_END=XML_TOK_UNKNOWN
};

enum SwXMLTableCellAttrTokens
{
    XML_TOK_TABLE_XMLID,
    XML_TOK_TABLE_STYLE_NAME,
    XML_TOK_TABLE_NUM_COLS_SPANNED,
    XML_TOK_TABLE_NUM_ROWS_SPANNED,
    XML_TOK_TABLE_NUM_COLS_REPEATED,
    XML_TOK_TABLE_FORMULA,
    XML_TOK_TABLE_VALUE,
    XML_TOK_TABLE_TIME_VALUE,
    XML_TOK_TABLE_DATE_VALUE,
    XML_TOK_TABLE_BOOLEAN_VALUE,
    XML_TOK_TABLE_PROTECTED,
    XML_TOK_TABLE_STRING_VALUE,
    XML_TOK_TABLE_VALUE_TYPE,
    XML_TOK_TABLE_CELL_ATTR_END=XML_TOK_UNKNOWN
};

static SvXMLTokenMapEntry aTableElemTokenMap[] =
{
    { XML_NAMESPACE_TABLE, XML_TABLE_HEADER_COLUMNS,
            XML_TOK_TABLE_HEADER_COLS },
    { XML_NAMESPACE_TABLE, XML_TABLE_COLUMNS,           XML_TOK_TABLE_COLS },
    { XML_NAMESPACE_TABLE, XML_TABLE_COLUMN,            XML_TOK_TABLE_COL },
    { XML_NAMESPACE_TABLE, XML_TABLE_HEADER_ROWS,
            XML_TOK_TABLE_HEADER_ROWS },
    { XML_NAMESPACE_TABLE, XML_TABLE_ROWS,              XML_TOK_TABLE_ROWS },
    { XML_NAMESPACE_TABLE, XML_TABLE_ROW,               XML_TOK_TABLE_ROW },
    { XML_NAMESPACE_OFFICE, XML_DDE_SOURCE,
            XML_TOK_OFFICE_DDE_SOURCE },

    
    
    
    
    
    { XML_NAMESPACE_TABLE, XML_TABLE_COLUMN_GROUP,      XML_TOK_TABLE_COLS },

    XML_TOKEN_MAP_END
};

static SvXMLTokenMapEntry aTableCellAttrTokenMap[] =
{
    { XML_NAMESPACE_XML, XML_ID, XML_TOK_TABLE_XMLID },
    { XML_NAMESPACE_TABLE, XML_STYLE_NAME, XML_TOK_TABLE_STYLE_NAME },
    { XML_NAMESPACE_TABLE, XML_NUMBER_COLUMNS_SPANNED, XML_TOK_TABLE_NUM_COLS_SPANNED },
    { XML_NAMESPACE_TABLE, XML_NUMBER_ROWS_SPANNED, XML_TOK_TABLE_NUM_ROWS_SPANNED },
    { XML_NAMESPACE_TABLE, XML_NUMBER_COLUMNS_REPEATED, XML_TOK_TABLE_NUM_COLS_REPEATED },
    { XML_NAMESPACE_TABLE, XML_FORMULA, XML_TOK_TABLE_FORMULA },
    { XML_NAMESPACE_OFFICE, XML_VALUE, XML_TOK_TABLE_VALUE },
    { XML_NAMESPACE_OFFICE, XML_TIME_VALUE, XML_TOK_TABLE_TIME_VALUE },
    { XML_NAMESPACE_OFFICE, XML_DATE_VALUE, XML_TOK_TABLE_DATE_VALUE },
    { XML_NAMESPACE_OFFICE, XML_BOOLEAN_VALUE, XML_TOK_TABLE_BOOLEAN_VALUE },
    { XML_NAMESPACE_TABLE, XML_PROTECTED, XML_TOK_TABLE_PROTECTED },
    { XML_NAMESPACE_TABLE, XML_PROTECT, XML_TOK_TABLE_PROTECTED }, 
    { XML_NAMESPACE_OFFICE, XML_STRING_VALUE, XML_TOK_TABLE_STRING_VALUE },
    { XML_NAMESPACE_OFFICE, XML_VALUE_TYPE, XML_TOK_TABLE_VALUE_TYPE },
    XML_TOKEN_MAP_END
};

const SvXMLTokenMap& SwXMLImport::GetTableElemTokenMap()
{
    if( !pTableElemTokenMap )
        pTableElemTokenMap = new SvXMLTokenMap( aTableElemTokenMap );

    return *pTableElemTokenMap;
}

const SvXMLTokenMap& SwXMLImport::GetTableCellAttrTokenMap()
{
    if( !pTableCellAttrTokenMap )
        pTableCellAttrTokenMap = new SvXMLTokenMap( aTableCellAttrTokenMap );

    return *pTableCellAttrTokenMap;
}

class SwXMLTableCell_Impl
{
    OUString aStyleName;

    OUString mXmlId;
    OUString m_StringValue;

    OUString sFormula;  
    double dValue;      

    SvXMLImportContextRef   xSubTable;

    const SwStartNode *pStartNode;
    sal_uInt32 nRowSpan;
    sal_uInt32 nColSpan;

    sal_Bool bProtected : 1;
    sal_Bool bHasValue; 
    sal_Bool mbCovered;
    bool m_bHasStringValue;

public:

    SwXMLTableCell_Impl( sal_uInt32 nRSpan=1UL, sal_uInt32 nCSpan=1UL ) :
        dValue( 0.0 ),
        pStartNode( 0 ),
        nRowSpan( nRSpan ),
        nColSpan( nCSpan ),
        bProtected( sal_False ),
        mbCovered( sal_False )
        , m_bHasStringValue(false)
        {}

    inline void Set( const OUString& rStyleName,
                      sal_uInt32 nRSpan, sal_uInt32 nCSpan,
                     const SwStartNode *pStNd, SwXMLTableContext *pTable,
                     sal_Bool bProtect,
                     const OUString* pFormula,
                     sal_Bool bHasValue,
                     sal_Bool bCovered,
                     double dVal,
                     OUString const*const pStringValue,
                     OUString const& i_rXmlId);

    bool IsUsed() const { return pStartNode!=0 ||
                                     xSubTable.Is() || bProtected;}

    sal_uInt32 GetRowSpan() const { return nRowSpan; }
    void SetRowSpan( sal_uInt32 nSet ) { nRowSpan = nSet; }
    sal_uInt32 GetColSpan() const { return nColSpan; }
    const OUString& GetStyleName() const { return aStyleName; }
    const OUString& GetFormula() const { return sFormula; }
    double GetValue() const { return dValue; }
    sal_Bool HasValue() const { return bHasValue; }
    sal_Bool IsProtected() const { return bProtected; }
    sal_Bool IsCovered() const { return mbCovered; }
    bool HasStringValue() const { return m_bHasStringValue; }
    OUString const* GetStringValue() const {
        return (m_bHasStringValue) ? &m_StringValue : 0;
    }
    const OUString& GetXmlId() const { return mXmlId; }

    const SwStartNode *GetStartNode() const { return pStartNode; }
    inline void SetStartNode( const SwStartNode *pSttNd );

    inline SwXMLTableContext *GetSubTable() const;

    inline void Dispose();
};

inline void SwXMLTableCell_Impl::Set( const OUString& rStyleName,
                                      sal_uInt32 nRSpan, sal_uInt32 nCSpan,
                                      const SwStartNode *pStNd,
                                      SwXMLTableContext *pTable,
                                      sal_Bool bProtect,
                                      const OUString* pFormula,
                                      sal_Bool bHasVal,
                                      sal_Bool bCov,
                                      double dVal,
                                      OUString const*const pStringValue,
                                      OUString const& i_rXmlId )
{
    aStyleName = rStyleName;
    nRowSpan = nRSpan;
    nColSpan = nCSpan;
    pStartNode = pStNd;
    xSubTable = pTable;
    dValue = dVal;
    bHasValue = bHasVal;
    mbCovered = bCov;
    if (pStringValue)
    {
        m_StringValue = *pStringValue;
    }
    m_bHasStringValue = (pStringValue != 0);
    bProtected = bProtect;

    if (!mbCovered) 
    {
        mXmlId = i_rXmlId;
    }

    
    if (pFormula != NULL)
    {
        sFormula = *pFormula;
    }
}

inline void SwXMLTableCell_Impl::SetStartNode( const SwStartNode *pSttNd )
{
    pStartNode = pSttNd;
    xSubTable = 0;
}

inline SwXMLTableContext *SwXMLTableCell_Impl::GetSubTable() const
{
    return (SwXMLTableContext *)&xSubTable;
}

inline void SwXMLTableCell_Impl::Dispose()
{
    if( xSubTable.Is() )
        xSubTable = 0;
}

typedef boost::ptr_vector<SwXMLTableCell_Impl> SwXMLTableCells_Impl;

class SwXMLTableRow_Impl
{
    OUString aStyleName;
    OUString aDfltCellStyleName;
    OUString mXmlId;

    SwXMLTableCells_Impl aCells;

    sal_Bool bSplitable;

public:

    SwXMLTableRow_Impl( const OUString& rStyleName, sal_uInt32 nCells,
                         const OUString *pDfltCellStyleName = 0,
                        const OUString& i_rXmlId = OUString() );
    ~SwXMLTableRow_Impl() {}

    inline const SwXMLTableCell_Impl *GetCell( sal_uInt32 nCol ) const;
    inline SwXMLTableCell_Impl *GetCell( sal_uInt32 nCol );

    inline void Set( const OUString& rStyleName,
                     const OUString& rDfltCellStyleName,
                     const OUString& i_rXmlId );

    void Expand( sal_uInt32 nCells, sal_Bool bOneCell );

    void SetSplitable( sal_Bool bSet ) { bSplitable = bSet; }
    sal_Bool IsSplitable() const { return bSplitable; }

    const OUString& GetStyleName() const { return aStyleName; }
    const OUString& GetDefaultCellStyleName() const { return aDfltCellStyleName; }
    const OUString& GetXmlId() const { return mXmlId; }

    void Dispose();
};

SwXMLTableRow_Impl::SwXMLTableRow_Impl( const OUString& rStyleName,
                                        sal_uInt32 nCells,
                                        const OUString *pDfltCellStyleName,
                                        const OUString& i_rXmlId ) :
    aStyleName( rStyleName ),
    mXmlId( i_rXmlId ),
    bSplitable( sal_False )
{
    if( pDfltCellStyleName  )
        aDfltCellStyleName = *pDfltCellStyleName;
    OSL_ENSURE( nCells <= USHRT_MAX,
            "SwXMLTableRow_Impl::SwXMLTableRow_Impl: too many cells" );
    if( nCells > USHRT_MAX )
        nCells = USHRT_MAX;

    for( sal_uInt16 i=0U; i<nCells; i++ )
    {
        aCells.push_back( new SwXMLTableCell_Impl );
    }
}

inline const SwXMLTableCell_Impl *SwXMLTableRow_Impl::GetCell( sal_uInt32 nCol ) const
{
    OSL_ENSURE( nCol < USHRT_MAX,
            "SwXMLTableRow_Impl::GetCell: column number is to big" );
    
    OSL_ENSURE( nCol < aCells.size(),
            "SwXMLTableRow_Impl::GetCell: column number is out of bound" );
    return nCol < aCells.size() ? &aCells[(sal_uInt16)nCol] : 0;
}

inline SwXMLTableCell_Impl *SwXMLTableRow_Impl::GetCell( sal_uInt32 nCol )
{
    OSL_ENSURE( nCol < USHRT_MAX,
            "SwXMLTableRow_Impl::GetCell: column number is to big" );
    
    OSL_ENSURE( nCol < aCells.size(),
            "SwXMLTableRow_Impl::GetCell: column number is out of bound" );
    return nCol < aCells.size() ? &aCells[(sal_uInt16)nCol] : 0;
}

void SwXMLTableRow_Impl::Expand( sal_uInt32 nCells, sal_Bool bOneCell )
{
    OSL_ENSURE( nCells <= USHRT_MAX,
            "SwXMLTableRow_Impl::Expand: too many cells" );
    if( nCells > USHRT_MAX )
        nCells = USHRT_MAX;

    sal_uInt32 nColSpan = nCells - aCells.size();
    for( sal_uInt16 i=aCells.size(); i<nCells; i++ )
    {
        aCells.push_back( new SwXMLTableCell_Impl( 1UL,
                                                bOneCell ? nColSpan : 1UL ) );
        nColSpan--;
    }

    OSL_ENSURE( nCells<=aCells.size(),
            "SwXMLTableRow_Impl::Expand: wrong number of cells" );
}

inline void SwXMLTableRow_Impl::Set( const OUString& rStyleName,
                                     const OUString& rDfltCellStyleName,
                                     const OUString& i_rXmlId )
{
    aStyleName = rStyleName;
    aDfltCellStyleName = rDfltCellStyleName;
    mXmlId = i_rXmlId;
}

void SwXMLTableRow_Impl::Dispose()
{
    for( sal_uInt16 i=0; i < aCells.size(); i++ )
        aCells[i].Dispose();
}

class SwXMLTableCellContext_Impl : public SvXMLImportContext
{
    OUString aStyleName;
    OUString sFormula;
    OUString sSaveParaDefault;
    OUString mXmlId;
    OUString m_StringValue;

    SvXMLImportContextRef   xMyTable;

    double fValue;
    sal_Bool bHasValue;
    bool     m_bHasStringValue;
    bool     m_bValueTypeIsString;
    sal_Bool bProtect;

    sal_uInt32                  nRowSpan;
    sal_uInt32                  nColSpan;
    sal_uInt32                  nColRepeat;

    sal_Bool                    bHasTextContent : 1;
    sal_Bool                    bHasTableContent : 1;

    SwXMLTableContext *GetTable() { return (SwXMLTableContext *)&xMyTable; }

    sal_Bool HasContent() const { return bHasTextContent || bHasTableContent; }
    inline void _InsertContent();
    inline void InsertContent();
    inline void InsertContentIfNotThere();
    inline void InsertContent( SwXMLTableContext *pTable );

public:

    SwXMLTableCellContext_Impl(
            SwXMLImport& rImport, sal_uInt16 nPrfx, const OUString& rLName,
            const Reference< xml::sax::XAttributeList > & xAttrList,
            SwXMLTableContext *pTable );

    virtual ~SwXMLTableCellContext_Impl();

    virtual SvXMLImportContext *CreateChildContext(
            sal_uInt16 nPrefix, const OUString& rLocalName,
            const Reference< xml::sax::XAttributeList > & xAttrList );
    virtual void EndElement();

    SwXMLImport& GetSwImport() { return (SwXMLImport&)GetImport(); }
};

SwXMLTableCellContext_Impl::SwXMLTableCellContext_Impl(
        SwXMLImport& rImport, sal_uInt16 nPrfx, const OUString& rLName,
        const Reference< xml::sax::XAttributeList > & xAttrList,
        SwXMLTableContext *pTable ) :
    SvXMLImportContext( rImport, nPrfx, rLName ),
    sFormula(),
    xMyTable( pTable ),
    fValue( 0.0 ),
    bHasValue( sal_False ),
    m_bHasStringValue(false),
    m_bValueTypeIsString(false),
    bProtect( sal_False ),
    nRowSpan( 1UL ),
    nColSpan( 1UL ),
    nColRepeat( 1UL ),
    bHasTextContent( sal_False ),
    bHasTableContent( sal_False )
{
    sSaveParaDefault = GetImport().GetTextImport()->GetCellParaStyleDefault();
    sal_Int16 nAttrCount = xAttrList.is() ? xAttrList->getLength() : 0;
    for( sal_Int16 i=0; i < nAttrCount; i++ )
    {
        const OUString& rAttrName = xAttrList->getNameByIndex( i );

        OUString aLocalName;
        sal_uInt16 nPrefix =
            GetImport().GetNamespaceMap().GetKeyByAttrName( rAttrName,
                                                            &aLocalName );
        const OUString& rValue = xAttrList->getValueByIndex( i );
        const SvXMLTokenMap& rTokenMap =
            GetSwImport().GetTableCellAttrTokenMap();
        switch( rTokenMap.Get( nPrefix, aLocalName ) )
        {
        case XML_TOK_TABLE_XMLID:
            mXmlId = rValue;
            break;
        case XML_TOK_TABLE_STYLE_NAME:
            aStyleName = rValue;
            GetImport().GetTextImport()->SetCellParaStyleDefault(rValue);
            break;
        case XML_TOK_TABLE_NUM_COLS_SPANNED:
            nColSpan = (sal_uInt32)rValue.toInt32();
            if( nColSpan < 1UL )
                nColSpan = 1UL;
            break;
        case XML_TOK_TABLE_NUM_ROWS_SPANNED:
            nRowSpan = (sal_uInt32)rValue.toInt32();
            if( nRowSpan < 1UL )
                nRowSpan = 1UL;
            break;
        case XML_TOK_TABLE_NUM_COLS_REPEATED:
            nColRepeat = (sal_uInt32)rValue.toInt32();
            if( nColRepeat < 1UL )
                nColRepeat = 1UL;
            break;
        case XML_TOK_TABLE_FORMULA:
            {
                OUString sTmp;
                sal_uInt16 nPrefix2 = GetImport().GetNamespaceMap().
                        _GetKeyByAttrName( rValue, &sTmp, sal_False );
                sFormula = XML_NAMESPACE_OOOW == nPrefix2 ? sTmp : rValue;
            }
            break;
        case XML_TOK_TABLE_VALUE:
            {
                double fTmp;
                if (::sax::Converter::convertDouble(fTmp, rValue))
                {
                    fValue = fTmp;
                    bHasValue = sal_True;
                }
            }
            break;
        case XML_TOK_TABLE_TIME_VALUE:
            {
                double fTmp;
                if (::sax::Converter::convertDuration(fTmp, rValue))
                {
                    fValue = fTmp;
                    bHasValue = sal_True;
                }
            }
            break;
        case XML_TOK_TABLE_DATE_VALUE:
            {
                double fTmp;
                if (GetImport().GetMM100UnitConverter().convertDateTime(fTmp,
                                                                      rValue))
                {
                    fValue = fTmp;
                    bHasValue = sal_True;
                }
            }
            break;
        case XML_TOK_TABLE_BOOLEAN_VALUE:
            {
                bool bTmp(false);
                if (::sax::Converter::convertBool(bTmp, rValue))
                {
                    fValue = (bTmp ? 1.0 : 0.0);
                    bHasValue = sal_True;
                }
            }
            break;
        case XML_TOK_TABLE_PROTECTED:
            {
                bool bTmp(false);
                if (::sax::Converter::convertBool(bTmp, rValue))
                {
                    bProtect = bTmp;
                }
            }
            break;
        case XML_TOK_TABLE_STRING_VALUE:
            {
                m_StringValue = rValue;
                m_bHasStringValue = true;
            }
            break;
        case XML_TOK_TABLE_VALUE_TYPE:
            {
                if ("string" == rValue)
                {
                    m_bValueTypeIsString = true;
                }
                
                
                
            }
            break;
        }
    }
}

SwXMLTableCellContext_Impl::~SwXMLTableCellContext_Impl()
{
}

inline void SwXMLTableCellContext_Impl::_InsertContent()
{
    SwStartNode const*const pStartNode( GetTable()->InsertTableSection(0,
            (m_bHasStringValue && m_bValueTypeIsString &&
             !aStyleName.isEmpty()) ? & aStyleName : 0) );
    GetTable()->InsertCell( aStyleName, nRowSpan, nColSpan,
                            pStartNode,
                            mXmlId,
                            NULL, bProtect, &sFormula, bHasValue, fValue,
            (m_bHasStringValue && m_bValueTypeIsString) ? &m_StringValue : 0);
}

inline void SwXMLTableCellContext_Impl::InsertContent()
{
    OSL_ENSURE( !HasContent(), "content already there" );
    bHasTextContent = sal_True;
    _InsertContent();
}

inline void SwXMLTableCellContext_Impl::InsertContentIfNotThere()
{
    if( !HasContent() )
        InsertContent();
}

inline void SwXMLTableCellContext_Impl::InsertContent(
                                                SwXMLTableContext *pTable )
{
    GetTable()->InsertCell( aStyleName, nRowSpan, nColSpan, 0, mXmlId, pTable, bProtect );
    bHasTableContent = sal_True;
}

SvXMLImportContext *SwXMLTableCellContext_Impl::CreateChildContext(
        sal_uInt16 nPrefix,
        const OUString& rLocalName,
        const Reference< xml::sax::XAttributeList > & xAttrList )
{
    SvXMLImportContext *pContext = 0;

    OUString sXmlId;
    sal_Bool bSubTable = sal_False;
    if( XML_NAMESPACE_TABLE == nPrefix &&
        IsXMLToken( rLocalName, XML_TABLE ) )
    {
        sal_Int16 nAttrCount = xAttrList.is() ? xAttrList->getLength() : 0;
        for( sal_Int16 i=0; i < nAttrCount; i++ )
        {
            const OUString& rAttrName = xAttrList->getNameByIndex( i );

            OUString aLocalName;
            sal_uInt16 nPrefix2 =
                GetImport().GetNamespaceMap().GetKeyByAttrName( rAttrName,
                                                                &aLocalName );
            if( XML_NAMESPACE_TABLE == nPrefix2 &&
                 IsXMLToken( aLocalName, XML_IS_SUB_TABLE ) &&
                 IsXMLToken( xAttrList->getValueByIndex( i ), XML_TRUE ) )
            {
                bSubTable = sal_True;
            }
            else if ( (XML_NAMESPACE_XML == nPrefix2) &&
                     IsXMLToken( aLocalName, XML_ID ) )
            {
                sXmlId = xAttrList->getValueByIndex( i );
            }
        //FIXME: RDFa
        }
    }

    if( bSubTable )
    {
        if( !HasContent() )
        {
            SwXMLTableContext *pTblContext =
                new SwXMLTableContext( GetSwImport(), nPrefix, rLocalName,
                                       xAttrList, GetTable(), sXmlId );
            pContext = pTblContext;
            if( GetTable()->IsValid() )
                InsertContent( pTblContext );

            GetTable()->SetHasSubTables( true );
        }
    }
    else
    {
        if( GetTable()->IsValid() )
            InsertContentIfNotThere();
        
        if (!(m_bValueTypeIsString && m_bHasStringValue))
        {
            pContext = GetImport().GetTextImport()->CreateTextChildContext(
                        GetImport(), nPrefix, rLocalName, xAttrList,
                        XML_TEXT_TYPE_CELL  );
        }
    }

    if( !pContext )
        pContext = new SvXMLImportContext( GetImport(), nPrefix, rLocalName );

    return pContext;
}

void SwXMLTableCellContext_Impl::EndElement()
{
    if( GetTable()->IsValid() )
    {
        if( bHasTextContent )
        {
            GetImport().GetTextImport()->DeleteParagraph();
            if( nColRepeat > 1 && nColSpan == 1 )
            {
                
                
                Reference < XTextCursor > xSrcTxtCursor =
                    GetImport().GetTextImport()->GetText()->createTextCursor();
                xSrcTxtCursor->gotoEnd( sal_True );

                
                Reference<XUnoTunnel> xSrcCrsrTunnel( xSrcTxtCursor, UNO_QUERY);
                OSL_ENSURE( xSrcCrsrTunnel.is(), "missing XUnoTunnel for Cursor" );
                OTextCursorHelper *pSrcTxtCrsr = reinterpret_cast< OTextCursorHelper * >(
                        sal::static_int_cast< sal_IntPtr >( xSrcCrsrTunnel->getSomething( OTextCursorHelper::getUnoTunnelId() )));
                OSL_ENSURE( pSrcTxtCrsr, "SwXTextCursor missing" );
                SwDoc *pDoc = pSrcTxtCrsr->GetDoc();
                const SwPaM *pSrcPaM = pSrcTxtCrsr->GetPaM();

                while( nColRepeat > 1 && GetTable()->IsInsertCellPossible() )
                {
                    _InsertContent();

                    Reference<XUnoTunnel> xDstCrsrTunnel(
                        GetImport().GetTextImport()->GetCursor(), UNO_QUERY);
                    OSL_ENSURE( xDstCrsrTunnel.is(),
                            "missing XUnoTunnel for Cursor" );
                    OTextCursorHelper *pDstTxtCrsr = reinterpret_cast< OTextCursorHelper * >(
                            sal::static_int_cast< sal_IntPtr >( xDstCrsrTunnel->getSomething( OTextCursorHelper::getUnoTunnelId() )) );
                    OSL_ENSURE( pDstTxtCrsr, "SwXTextCursor missing" );
                    SwPaM aSrcPaM( *pSrcPaM->GetPoint(),
                                   *pSrcPaM->GetMark() );
                    SwPosition aDstPos( *pDstTxtCrsr->GetPaM()->GetPoint() );
                    pDoc->CopyRange( aSrcPaM, aDstPos, false );

                    nColRepeat--;
                }
            }
        }
        else if( !bHasTableContent )
        {
            InsertContent();
            if( nColRepeat > 1 && nColSpan == 1 )
            {
                while( nColRepeat > 1 && GetTable()->IsInsertCellPossible() )
                {
                    _InsertContent();
                    nColRepeat--;
                }
            }
        }
    }
    GetImport().GetTextImport()->SetCellParaStyleDefault(sSaveParaDefault);
}

class SwXMLTableColContext_Impl : public SvXMLImportContext
{
    SvXMLImportContextRef   xMyTable;

    SwXMLTableContext *GetTable() { return (SwXMLTableContext *)&xMyTable; }

public:

    SwXMLTableColContext_Impl(
            SwXMLImport& rImport, sal_uInt16 nPrfx, const OUString& rLName,
            const Reference< xml::sax::XAttributeList > & xAttrList,
            SwXMLTableContext *pTable );

    virtual ~SwXMLTableColContext_Impl();

    SwXMLImport& GetSwImport() { return (SwXMLImport&)GetImport(); }
};

SwXMLTableColContext_Impl::SwXMLTableColContext_Impl(
        SwXMLImport& rImport, sal_uInt16 nPrfx, const OUString& rLName,
        const Reference< xml::sax::XAttributeList > & xAttrList,
        SwXMLTableContext *pTable ) :
    SvXMLImportContext( rImport, nPrfx, rLName ),
    xMyTable( pTable )
{
    sal_uInt32 nColRep = 1UL;
    OUString aStyleName, aDfltCellStyleName;

    sal_Int16 nAttrCount = xAttrList.is() ? xAttrList->getLength() : 0;
    for( sal_Int16 i=0; i < nAttrCount; i++ )
    {
        const OUString& rAttrName = xAttrList->getNameByIndex( i );

        OUString aLocalName;
        sal_uInt16 nPrefix =
            GetImport().GetNamespaceMap().GetKeyByAttrName( rAttrName,
                                                            &aLocalName );
        const OUString& rValue = xAttrList->getValueByIndex( i );
        if( XML_NAMESPACE_TABLE == nPrefix )
        {
            if( IsXMLToken( aLocalName, XML_STYLE_NAME ) )
                aStyleName = rValue;
            else if( IsXMLToken( aLocalName, XML_NUMBER_COLUMNS_REPEATED ) )
                nColRep = (sal_uInt32)rValue.toInt32();
            else if( IsXMLToken( aLocalName, XML_DEFAULT_CELL_STYLE_NAME ) )
                aDfltCellStyleName = rValue;
        }
        else if ( (XML_NAMESPACE_XML == nPrefix) &&
                 IsXMLToken( aLocalName, XML_ID ) )
        {
            (void) rValue;
        //FIXME where to put this??? columns do not actually exist in writer...
        }
    }

    sal_Int32 nWidth = MINLAY;
    bool bRelWidth = true;
    if( !aStyleName.isEmpty() )
    {
        const SfxPoolItem *pItem;
        const SfxItemSet *pAutoItemSet = 0;
        if( GetSwImport().FindAutomaticStyle(
                    XML_STYLE_FAMILY_TABLE_COLUMN,
                                              aStyleName, &pAutoItemSet ) &&
            pAutoItemSet &&
            SFX_ITEM_SET == pAutoItemSet->GetItemState( RES_FRM_SIZE, false,
                                                        &pItem ) )
        {
            const SwFmtFrmSize *pSize = ((const SwFmtFrmSize *)pItem);
            nWidth = pSize->GetWidth();
            bRelWidth = ATT_VAR_SIZE == pSize->GetHeightSizeType();
        }
    }

    if( nWidth )
    {
        while( nColRep-- && GetTable()->IsInsertColPossible() )
            GetTable()->InsertColumn( nWidth, bRelWidth, &aDfltCellStyleName );
    }
}

SwXMLTableColContext_Impl::~SwXMLTableColContext_Impl()
{
}

class SwXMLTableColsContext_Impl : public SvXMLImportContext
{
    SvXMLImportContextRef   xMyTable;

    SwXMLTableContext *GetTable() { return (SwXMLTableContext *)&xMyTable; }

public:

    SwXMLTableColsContext_Impl(
            SwXMLImport& rImport, sal_uInt16 nPrfx,
            const OUString& rLName,
            const Reference< xml::sax::XAttributeList > & xAttrList,
            SwXMLTableContext *pTable );

    virtual ~SwXMLTableColsContext_Impl();

    virtual SvXMLImportContext *CreateChildContext(
            sal_uInt16 nPrefix, const OUString& rLocalName,
            const Reference< xml::sax::XAttributeList > & xAttrList );

    SwXMLImport& GetSwImport() { return (SwXMLImport&)GetImport(); }
};

SwXMLTableColsContext_Impl::SwXMLTableColsContext_Impl(
        SwXMLImport& rImport, sal_uInt16 nPrfx, const OUString& rLName,
        const Reference< xml::sax::XAttributeList > &,
        SwXMLTableContext *pTable ) :
    SvXMLImportContext( rImport, nPrfx, rLName ),
    xMyTable( pTable )
{
}

SwXMLTableColsContext_Impl::~SwXMLTableColsContext_Impl()
{
}

SvXMLImportContext *SwXMLTableColsContext_Impl::CreateChildContext(
        sal_uInt16 nPrefix,
        const OUString& rLocalName,
        const Reference< xml::sax::XAttributeList > & xAttrList )
{
    SvXMLImportContext *pContext = 0;

    if( XML_NAMESPACE_TABLE == nPrefix &&
        IsXMLToken( rLocalName, XML_TABLE_COLUMN ) &&
        GetTable()->IsInsertColPossible() )
        pContext = new SwXMLTableColContext_Impl( GetSwImport(), nPrefix,
                                                  rLocalName, xAttrList,
                                                  GetTable() );

    if( !pContext )
        pContext = new SvXMLImportContext( GetImport(), nPrefix, rLocalName );

    return pContext;
}

class SwXMLTableRowContext_Impl : public SvXMLImportContext
{
    SvXMLImportContextRef   xMyTable;

    sal_uInt32                  nRowRepeat;

    SwXMLTableContext *GetTable() { return (SwXMLTableContext *)&xMyTable; }

public:

    SwXMLTableRowContext_Impl(
            SwXMLImport& rImport, sal_uInt16 nPrfx, const OUString& rLName,
            const Reference< xml::sax::XAttributeList > & xAttrList,
            SwXMLTableContext *pTable, sal_Bool bInHead=sal_False );

    virtual ~SwXMLTableRowContext_Impl();

    virtual SvXMLImportContext *CreateChildContext( sal_uInt16 nPrefix,
            const OUString& rLocalName,
            const Reference< xml::sax::XAttributeList > & xAttrList );

    virtual void EndElement();

    SwXMLImport& GetSwImport() { return (SwXMLImport&)GetImport(); }
};

SwXMLTableRowContext_Impl::SwXMLTableRowContext_Impl( SwXMLImport& rImport,
        sal_uInt16 nPrfx,
        const OUString& rLName,
        const Reference< xml::sax::XAttributeList > & xAttrList,
        SwXMLTableContext *pTable,
        sal_Bool bInHead ) :
    SvXMLImportContext( rImport, nPrfx, rLName ),
    xMyTable( pTable ),
    nRowRepeat( 1 )
{
    OUString aStyleName, aDfltCellStyleName;
    OUString sXmlId;

    sal_Int16 nAttrCount = xAttrList.is() ? xAttrList->getLength() : 0;
    for( sal_Int16 i=0; i < nAttrCount; i++ )
    {
        const OUString& rAttrName = xAttrList->getNameByIndex( i );

        OUString aLocalName;
        sal_uInt16 nPrefix =
            GetImport().GetNamespaceMap().GetKeyByAttrName( rAttrName,
                                                            &aLocalName );
        const OUString& rValue = xAttrList->getValueByIndex( i );
        if( XML_NAMESPACE_TABLE == nPrefix )
        {
            if( IsXMLToken( aLocalName, XML_STYLE_NAME ) )
            {
                aStyleName = rValue;
            }
            else if( IsXMLToken( aLocalName, XML_NUMBER_ROWS_REPEATED ) )
            {
                nRowRepeat = (sal_uInt32)rValue.toInt32();
                if( nRowRepeat < 1UL )
                    nRowRepeat = 1UL;
            }
            else if( IsXMLToken( aLocalName, XML_DEFAULT_CELL_STYLE_NAME ) )
            {
                aDfltCellStyleName = rValue;
            }
        }
        else if ( (XML_NAMESPACE_XML == nPrefix) &&
                 IsXMLToken( aLocalName, XML_ID ) )
        {
            sXmlId = rValue;
        }
    }
    if( GetTable()->IsValid() )
        GetTable()->InsertRow( aStyleName, aDfltCellStyleName, bInHead,
                               sXmlId );
}

void SwXMLTableRowContext_Impl::EndElement()
{
    if( GetTable()->IsValid() )
    {
        GetTable()->FinishRow();

        if( nRowRepeat > 1UL )
            GetTable()->InsertRepRows( nRowRepeat );
    }
}

SwXMLTableRowContext_Impl::~SwXMLTableRowContext_Impl()
{
}

SvXMLImportContext *SwXMLTableRowContext_Impl::CreateChildContext(
        sal_uInt16 nPrefix, const OUString& rLocalName,
        const Reference< xml::sax::XAttributeList > & xAttrList )
{
    SvXMLImportContext *pContext = 0;

    if( XML_NAMESPACE_TABLE == nPrefix )
    {
        if( IsXMLToken( rLocalName, XML_TABLE_CELL ) )
        {
            if( !GetTable()->IsValid() || GetTable()->IsInsertCellPossible() )
                pContext = new SwXMLTableCellContext_Impl( GetSwImport(),
                                                           nPrefix,
                                                           rLocalName,
                                                           xAttrList,
                                                           GetTable() );
        }
        else if( IsXMLToken( rLocalName, XML_COVERED_TABLE_CELL ) )
            pContext = new SvXMLImportContext( GetImport(), nPrefix,
                                               rLocalName );
    }

    if( !pContext )
        pContext = new SvXMLImportContext( GetImport(), nPrefix, rLocalName );

    return pContext;
}

class SwXMLTableRowsContext_Impl : public SvXMLImportContext
{
    SvXMLImportContextRef   xMyTable;

    sal_Bool bHeader;

    SwXMLTableContext *GetTable() { return (SwXMLTableContext *)&xMyTable; }

public:

    SwXMLTableRowsContext_Impl( SwXMLImport& rImport, sal_uInt16 nPrfx,
            const OUString& rLName,
            const Reference< xml::sax::XAttributeList > & xAttrList,
            SwXMLTableContext *pTable,
            sal_Bool bHead );

    virtual ~SwXMLTableRowsContext_Impl();

    virtual SvXMLImportContext *CreateChildContext( sal_uInt16 nPrefix,
            const OUString& rLocalName,
            const Reference< xml::sax::XAttributeList > & xAttrList );

    SwXMLImport& GetSwImport() { return (SwXMLImport&)GetImport(); }
};

SwXMLTableRowsContext_Impl::SwXMLTableRowsContext_Impl( SwXMLImport& rImport,
        sal_uInt16 nPrfx,
        const OUString& rLName,
        const Reference< xml::sax::XAttributeList > &,
        SwXMLTableContext *pTable,
        sal_Bool bHead ) :
    SvXMLImportContext( rImport, nPrfx, rLName ),
    xMyTable( pTable ),
    bHeader( bHead )
{
}

SwXMLTableRowsContext_Impl::~SwXMLTableRowsContext_Impl()
{
}

SvXMLImportContext *SwXMLTableRowsContext_Impl::CreateChildContext(
        sal_uInt16 nPrefix,
        const OUString& rLocalName,
        const Reference< xml::sax::XAttributeList > & xAttrList )
{
    SvXMLImportContext *pContext = 0;

    if( XML_NAMESPACE_TABLE == nPrefix &&
        IsXMLToken( rLocalName, XML_TABLE_ROW ) &&
        GetTable()->IsInsertRowPossible() )
        pContext = new SwXMLTableRowContext_Impl( GetSwImport(), nPrefix,
                                                  rLocalName, xAttrList,
                                                  GetTable(),
                                                  bHeader );

    if( !pContext )
        pContext = new SvXMLImportContext( GetImport(), nPrefix, rLocalName );

    return pContext;
}

class SwXMLDDETableContext_Impl : public SvXMLImportContext
{
    OUString sConnectionName;
    OUString sDDEApplication;
    OUString sDDEItem;
    OUString sDDETopic;
    sal_Bool bIsAutomaticUpdate;

public:

    TYPEINFO();

    SwXMLDDETableContext_Impl(
        SwXMLImport& rImport, sal_uInt16 nPrfx, const OUString& rLName);

    ~SwXMLDDETableContext_Impl();

    virtual void StartElement(
        const Reference<xml::sax::XAttributeList> & xAttrList);

    OUString& GetConnectionName()   { return sConnectionName; }
    OUString& GetDDEApplication()   { return sDDEApplication; }
    OUString& GetDDEItem()          { return sDDEItem; }
    OUString& GetDDETopic()         { return sDDETopic; }
    sal_Bool GetIsAutomaticUpdate() { return bIsAutomaticUpdate; }
};

TYPEINIT1( SwXMLDDETableContext_Impl, SvXMLImportContext );

SwXMLDDETableContext_Impl::SwXMLDDETableContext_Impl(
    SwXMLImport& rImport, sal_uInt16 nPrfx, const OUString& rLName) :
        SvXMLImportContext(rImport, nPrfx, rLName),
        sConnectionName(),
        sDDEApplication(),
        sDDEItem(),
        sDDETopic(),
        bIsAutomaticUpdate(sal_False)
{
}

SwXMLDDETableContext_Impl::~SwXMLDDETableContext_Impl()
{
}

void SwXMLDDETableContext_Impl::StartElement(
    const Reference<xml::sax::XAttributeList> & xAttrList)
{
    sal_Int16 nAttrCount = xAttrList.is() ? xAttrList->getLength() : 0;
    for( sal_Int16 i = 0; i < nAttrCount; i++ )
    {
        const OUString& rAttrName = xAttrList->getNameByIndex( i );

        OUString aLocalName;
        sal_uInt16 nPrefix =
            GetImport().GetNamespaceMap().GetKeyByAttrName( rAttrName,
                                                            &aLocalName );
        const OUString& rValue = xAttrList->getValueByIndex( i );

        if (XML_NAMESPACE_OFFICE == nPrefix)
        {
            if ( IsXMLToken( aLocalName, XML_DDE_APPLICATION ) )
            {
                sDDEApplication = rValue;
            }
            else if ( IsXMLToken( aLocalName, XML_DDE_TOPIC ) )
            {
                sDDETopic = rValue;
            }
            else if ( IsXMLToken( aLocalName, XML_DDE_ITEM ) )
            {
                sDDEItem = rValue;
            }
            else if ( IsXMLToken( aLocalName, XML_NAME ) )
            {
                sConnectionName = rValue;
            }
            else if ( IsXMLToken( aLocalName, XML_AUTOMATIC_UPDATE ) )
            {
                bool bTmp(false);
                if (::sax::Converter::convertBool(bTmp, rValue))
                {
                    bIsAutomaticUpdate = bTmp;
                }
            }
            
        }
        
    }
}


static OUString lcl_GenerateFldTypeName(OUString sPrefix, SwTableNode* pTableNode)
{
    OUString sPrefixStr(sPrefix);

    if (sPrefixStr.isEmpty())
    {
        sPrefixStr = "_";
    }

    
    OUString sName;
    sal_Int32 nCount = 0;
    do
    {
        
        if (nCount < 0)
            return sName;

        nCount++;
        sName = sPrefixStr;
        sName += OUString::number(nCount);

    }
    while (NULL != pTableNode->GetDoc()->GetFldType(RES_DDEFLD, sName, false));

    return sName;
}


static SwDDEFieldType* lcl_GetDDEFieldType(SwXMLDDETableContext_Impl* pContext,
                                    SwTableNode* pTableNode)
{
    
    OUString sCommand(pContext->GetDDEApplication());
    sCommand += OUString(sfx2::cTokenSeparator);
    sCommand += pContext->GetDDEItem();
    sCommand += OUString(sfx2::cTokenSeparator);
    sCommand += pContext->GetDDETopic();

    sal_uInt16 nType = static_cast< sal_uInt16 >(pContext->GetIsAutomaticUpdate() ? sfx2::LINKUPDATE_ALWAYS
                                                        : sfx2::LINKUPDATE_ONCALL);

    OUString sName(pContext->GetConnectionName());

    
    SwDDEFieldType* pType = NULL;

    
    if (sName.isEmpty())
    {
        sName = lcl_GenerateFldTypeName(pContext->GetDDEApplication(),
                                        pTableNode);
    }
    else
    {
        
        SwDDEFieldType* pOldType = (SwDDEFieldType*)pTableNode->GetDoc()->GetFldType(RES_DDEFLD, sName, false);
        if (NULL != pOldType)
        {
            
            if ( (pOldType->GetCmd() == sCommand) &&
                 (pOldType->GetType() == nType) )
            {
                
                pType = pOldType;
            }
            else
            {
                
                sName = lcl_GenerateFldTypeName(pContext->GetDDEApplication(),
                                                pTableNode);
            }
        }
        
    }

    
    if (NULL == pType)
    {
        
        SwDDEFieldType aDDEFieldType(sName, sCommand, nType);
        pType = (SwDDEFieldType*)pTableNode->
            GetDoc()->InsertFldType(aDDEFieldType);
    }

    OSL_ENSURE(NULL != pType, "We really want a SwDDEFieldType here!");
    return pType;
}

class TableBoxIndex
{
public:
    OUString msName;
    sal_Int32 mnWidth;
    sal_Bool mbProtected;

    TableBoxIndex( const OUString& rName, sal_Int32 nWidth,
                   sal_Bool bProtected ) :
        msName( rName ),
        mnWidth( nWidth ),
        mbProtected( bProtected )
    { }

    bool operator== ( const TableBoxIndex& rArg ) const
    {
        return (rArg.mnWidth == mnWidth) &&
            (rArg.mbProtected == mbProtected) &&
            (rArg.msName == msName);
    }
};

class TableBoxIndexHasher
{
public:
    size_t operator() (const TableBoxIndex& rArg) const
    {
        return rArg.msName.hashCode() + rArg.mnWidth + rArg.mbProtected;
    }
};




typedef boost::ptr_vector<SwXMLTableRow_Impl> SwXMLTableRows_Impl;

const SwXMLTableCell_Impl *SwXMLTableContext::GetCell( sal_uInt32 nRow,
                                                 sal_uInt32 nCol ) const
{
    return (*pRows)[(sal_uInt16)nRow].GetCell( (sal_uInt16)nCol );
}

SwXMLTableCell_Impl *SwXMLTableContext::GetCell( sal_uInt32 nRow,
                                                 sal_uInt32 nCol )
{
    return (*pRows)[(sal_uInt16)nRow].GetCell( (sal_uInt16)nCol );
}

TYPEINIT1( SwXMLTableContext, XMLTextTableContext );

SwXMLTableContext::SwXMLTableContext( SwXMLImport& rImport,
        sal_uInt16 nPrfx,
        const OUString& rLName,
        const Reference< xml::sax::XAttributeList > & xAttrList ) :
    XMLTextTableContext( rImport, nPrfx, rLName ),
    pColumnDefaultCellStyleNames( 0 ),
    pRows( new SwXMLTableRows_Impl ),
    pTableNode( 0 ),
    pBox1( 0 ),
    pSttNd1( 0 ),
    pBoxFmt( 0 ),
    pLineFmt( 0 ),
    pSharedBoxFormats(NULL),
    pDDESource(NULL),
    bFirstSection( true ),
    bRelWidth( true ),
    bHasSubTables( false ),
    nHeaderRows( 0 ),
    nCurRow( 0UL ),
    nCurCol( 0UL ),
    nWidth( 0UL )
{
    OUString aName;
    OUString sXmlId;

    
    SolarMutexGuard aGuard;

    sal_Int16 nAttrCount = xAttrList.is() ? xAttrList->getLength() : 0;
    for( sal_Int16 i=0; i < nAttrCount; i++ )
    {
        const OUString& rAttrName = xAttrList->getNameByIndex( i );

        OUString aLocalName;
        sal_uInt16 nPrefix =
            GetImport().GetNamespaceMap().GetKeyByAttrName( rAttrName,
                                                            &aLocalName );
        const OUString& rValue = xAttrList->getValueByIndex( i );
        if( XML_NAMESPACE_TABLE == nPrefix )
        {
            if( IsXMLToken( aLocalName, XML_STYLE_NAME ) )
                aStyleName = rValue;
            else if( IsXMLToken( aLocalName, XML_NAME ) )
                aName = rValue;
            else if( IsXMLToken( aLocalName, XML_DEFAULT_CELL_STYLE_NAME ) )
                aDfltCellStyleName = rValue;
        }
        else if ( (XML_NAMESPACE_XML == nPrefix) &&
                 IsXMLToken( aLocalName, XML_ID ) )
        {
            sXmlId = rValue;
        }
    }

    SwDoc *pDoc = SwImport::GetDocFromXMLImport( GetSwImport() );

    OUString sTblName;
    if( !aName.isEmpty() )
    {
        const SwTableFmt *pTblFmt = pDoc->FindTblFmtByName( aName );
        if( !pTblFmt )
            sTblName = aName;
    }
    if( sTblName.isEmpty() )
    {
        sTblName = pDoc->GetUniqueTblName();
        GetImport().GetTextImport()
            ->GetRenameMap().Add( XML_TEXT_RENAME_TYPE_TABLE, aName, sTblName );
    }

    Reference< XTextTable > xTable;
    const SwXTextTable *pXTable = 0;
    Reference<XMultiServiceFactory> xFactory( GetImport().GetModel(),
                                              UNO_QUERY );
    OSL_ENSURE( xFactory.is(), "factory missing" );
    if( xFactory.is() )
    {
        OUString sService( "com.sun.star.text.TextTable" );
        Reference<XInterface> xIfc = xFactory->createInstance( sService );
        OSL_ENSURE( xIfc.is(), "Couldn't create a table" );

        if( xIfc.is() )
            xTable = Reference< XTextTable > ( xIfc, UNO_QUERY );
    }

    if( xTable.is() )
    {
        xTable->initialize( 1, 1 );

        try
        {
            xTextContent = xTable;
            GetImport().GetTextImport()->InsertTextContent( xTextContent );
        }
        catch( IllegalArgumentException& )
        {
            xTable = 0;
        }
    }

    if( xTable.is() )
    {
        //FIXME
        
        GetImport().SetXmlId(xTable, sXmlId);

        Reference<XUnoTunnel> xTableTunnel( xTable, UNO_QUERY);
        if( xTableTunnel.is() )
        {
            pXTable = reinterpret_cast< SwXTextTable * >(
                    sal::static_int_cast< sal_IntPtr >( xTableTunnel->getSomething( SwXTextTable::getUnoTunnelId() )));
            OSL_ENSURE( pXTable, "SwXTextTable missing" );
        }

        Reference < XCellRange > xCellRange( xTable, UNO_QUERY );
        Reference < XCell > xCell = xCellRange->getCellByPosition( 0, 0 );
        Reference < XText> xText( xCell, UNO_QUERY );
        xOldCursor = GetImport().GetTextImport()->GetCursor();
        GetImport().GetTextImport()->SetCursor( xText->createTextCursor() );

        
        GetImport().GetTextImport()->RedlineAdjustStartNodeCursor(sal_True);
    }
    if( pXTable )
    {
        SwFrmFmt *pTblFrmFmt = pXTable->GetFrmFmt();
        OSL_ENSURE( pTblFrmFmt, "table format missing" );
        SwTable *pTbl = SwTable::FindTable( pTblFrmFmt );
        OSL_ENSURE( pTbl, "table missing" );
        pTableNode = pTbl->GetTableNode();
        OSL_ENSURE( pTableNode, "table node missing" );

        pTblFrmFmt->SetName( sTblName );

        SwTableLine *pLine1 = pTableNode->GetTable().GetTabLines()[0U];
        pBox1 = pLine1->GetTabBoxes()[0U];
        pSttNd1 = pBox1->GetSttNd();
    }
}

SwXMLTableContext::SwXMLTableContext( SwXMLImport& rImport,
        sal_uInt16 nPrfx,
        const OUString& rLName,
        const Reference< xml::sax::XAttributeList > &,
        SwXMLTableContext *pTable,
        OUString const & i_rXmlId ) :
    XMLTextTableContext( rImport, nPrfx, rLName ),
    mXmlId( i_rXmlId ),
    pColumnDefaultCellStyleNames( 0 ),
    pRows( new SwXMLTableRows_Impl ),
    pTableNode( pTable->pTableNode ),
    pBox1( 0 ),
    pSttNd1( 0 ),
    pBoxFmt( 0 ),
    pLineFmt( 0 ),
    pSharedBoxFormats(NULL),
    xParentTable( pTable ),
    pDDESource(NULL),
    bFirstSection( false ),
    bRelWidth( true ),
    bHasSubTables( false ),
    nHeaderRows( 0 ),
    nCurRow( 0UL ),
    nCurCol( 0UL ),
    nWidth( 0UL )
{
}

SwXMLTableContext::~SwXMLTableContext()
{
    delete pColumnDefaultCellStyleNames;
    delete pSharedBoxFormats;
    delete pRows;

    
    GetImport().GetTextImport()->RedlineAdjustStartNodeCursor(sal_False);
}

SvXMLImportContext *SwXMLTableContext::CreateChildContext( sal_uInt16 nPrefix,
        const OUString& rLocalName,
        const Reference< xml::sax::XAttributeList > & xAttrList )
{
    SvXMLImportContext *pContext = 0;

    const SvXMLTokenMap& rTokenMap = GetSwImport().GetTableElemTokenMap();
    sal_Bool bHeader = sal_False;
    switch( rTokenMap.Get( nPrefix, rLocalName ) )
    {
    case XML_TOK_TABLE_HEADER_COLS:
    case XML_TOK_TABLE_COLS:
        if( IsValid() )
            pContext = new SwXMLTableColsContext_Impl( GetSwImport(), nPrefix,
                                                       rLocalName, xAttrList,
                                                       this );
        break;
    case XML_TOK_TABLE_COL:
        if( IsValid() && IsInsertColPossible() )
            pContext = new SwXMLTableColContext_Impl( GetSwImport(), nPrefix,
                                                      rLocalName, xAttrList,
                                                      this );
        break;
    case XML_TOK_TABLE_HEADER_ROWS:
        bHeader = sal_True;
    case XML_TOK_TABLE_ROWS:
        pContext = new SwXMLTableRowsContext_Impl( GetSwImport(), nPrefix,
                                                   rLocalName, xAttrList,
                                                   this, bHeader );
        break;
    case XML_TOK_TABLE_ROW:
        if( IsInsertRowPossible() )
            pContext = new SwXMLTableRowContext_Impl( GetSwImport(), nPrefix,
                                                      rLocalName, xAttrList,
                                                      this );
        break;
    case XML_TOK_OFFICE_DDE_SOURCE:
        
        if( IsValid() )
        {
            if (pDDESource != NULL)
            {
                pDDESource->ReleaseRef();
            }
            pDDESource = new SwXMLDDETableContext_Impl( GetSwImport(), nPrefix,
                                                        rLocalName );
            pDDESource->AddRef();
            pContext = pDDESource;
        }
        break;
    }

    if( !pContext )
        pContext = new SvXMLImportContext( GetImport(), nPrefix, rLocalName );

    return pContext;
}

void SwXMLTableContext::InsertColumn( sal_Int32 nWidth2, bool bRelWidth2,
                                         const OUString *pDfltCellStyleName )
{
    OSL_ENSURE( nCurCol < USHRT_MAX,
            "SwXMLTableContext::InsertColumn: no space left" );
    if( nCurCol >= USHRT_MAX )
        return;

    if( nWidth2 < MINLAY )
        nWidth2 = MINLAY;
    else if( nWidth2 > USHRT_MAX )
        nWidth2 = USHRT_MAX;
    aColumnWidths.push_back( ColumnWidthInfo(nWidth2, bRelWidth2) );
    if( (pDfltCellStyleName && !pDfltCellStyleName->isEmpty()) ||
        pColumnDefaultCellStyleNames )
    {
        if( !pColumnDefaultCellStyleNames )
        {
            pColumnDefaultCellStyleNames = new std::vector<OUString>;
            sal_uLong nCount = aColumnWidths.size() - 1;
            while( nCount-- )
                pColumnDefaultCellStyleNames->push_back(OUString());
        }

        if(pDfltCellStyleName)
            pColumnDefaultCellStyleNames->push_back(*pDfltCellStyleName);
        else
            pColumnDefaultCellStyleNames->push_back(OUString());
    }
}

sal_Int32 SwXMLTableContext::GetColumnWidth( sal_uInt32 nCol,
                                             sal_uInt32 nColSpan ) const
{
    sal_uInt32 nLast = nCol+nColSpan;
    if( nLast > aColumnWidths.size() )
        nLast = aColumnWidths.size();

    sal_Int32 nWidth2 = 0L;
    for( sal_uInt32 i=nCol; i < nLast; ++i )
        nWidth2 += aColumnWidths[i].width;

    return nWidth2;
}

OUString SwXMLTableContext::GetColumnDefaultCellStyleName( sal_uInt32 nCol ) const
{
    if( pColumnDefaultCellStyleNames && nCol < pColumnDefaultCellStyleNames->size())
        return (*pColumnDefaultCellStyleNames)[static_cast<size_t>(nCol)];

    return OUString();
}

void SwXMLTableContext::InsertCell( const OUString& rStyleName,
                                    sal_uInt32 nRowSpan, sal_uInt32 nColSpan,
                                    const SwStartNode *pStartNode,
                                    const OUString & i_rXmlId,
                                    SwXMLTableContext *pTable,
                                    sal_Bool bProtect,
                                    const OUString* pFormula,
                                    sal_Bool bHasValue,
                                    double fValue,
                                    OUString const*const pStringValue )
{
    OSL_ENSURE( nCurCol < GetColumnCount(),
            "SwXMLTableContext::InsertCell: row is full" );
    OSL_ENSURE( nCurRow < USHRT_MAX,
            "SwXMLTableContext::InsertCell: table is full" );
    if( nCurCol >= USHRT_MAX || nCurRow > USHRT_MAX )
        return;

    OSL_ENSURE( nRowSpan >=1UL, "SwXMLTableContext::InsertCell: row span is 0" );
    if( 0UL == nRowSpan )
        nRowSpan = 1UL;
    OSL_ENSURE( nColSpan >=1UL, "SwXMLTableContext::InsertCell: col span is 0" );
    if( 0UL == nColSpan )
        nColSpan = 1UL;

    sal_uInt32 i, j;

    
    sal_uInt32 nColsReq = nCurCol + nColSpan;
    if( nColsReq > GetColumnCount() )
    {
        nColSpan = GetColumnCount() - nCurCol;
        nColsReq = GetColumnCount();
    }

    
    
    if( nCurRow > 0UL && nColSpan > 1UL )
    {
        SwXMLTableRow_Impl *pCurRow = &(*pRows)[(sal_uInt16)nCurRow];
        sal_uInt32 nLastCol = GetColumnCount() < nColsReq ? GetColumnCount()
                                                     : nColsReq;
        for( i=nCurCol+1UL; i<nLastCol; i++ )
        {
            if( pCurRow->GetCell(i)->IsUsed() )
            {
                
                nColSpan = i - nCurCol;
                nColsReq = i;
                break;
            }
        }
    }

    sal_uInt32 nRowsReq = nCurRow + nRowSpan;
    if( nRowsReq > USHRT_MAX )
    {
        nRowSpan = USHRT_MAX - nCurRow;
        nRowsReq = USHRT_MAX;
    }

    
    
    if ( nColsReq > GetColumnCount() )
    {
        for( i=GetColumnCount(); i<nColsReq; i++ )
        {
            aColumnWidths.push_back( ColumnWidthInfo(MINLAY, true) );
        }
        
        for( i=0; i<pRows->size(); i++ )
            (*pRows)[(sal_uInt16)i].Expand( nColsReq, i<nCurRow );
    }

    
    if( pRows->size() < nRowsReq )
    {
        OUString aStyleName2;
        for( i = pRows->size(); i < nRowsReq; ++i )
            pRows->push_back( new SwXMLTableRow_Impl(aStyleName2, GetColumnCount()) );
    }

    OUString sStyleName( rStyleName );
    if( sStyleName.isEmpty() )
    {
        sStyleName = (*pRows)[(sal_uInt16)nCurRow].GetDefaultCellStyleName();
        if( sStyleName.isEmpty() && HasColumnDefaultCellStyleNames() )
        {
            sStyleName = GetColumnDefaultCellStyleName( nCurCol );
            if( sStyleName.isEmpty() )
                sStyleName = aDfltCellStyleName;
        }
    }

    
    for( i=nColSpan; i>0UL; i-- )
    {
        for( j=nRowSpan; j>0UL; j-- )
        {
            const bool bCovered = i != nColSpan || j != nRowSpan;
            GetCell( nRowsReq-j, nColsReq-i )
                ->Set( sStyleName, j, i, pStartNode,
                       pTable, bProtect, pFormula, bHasValue, bCovered, fValue,
                       pStringValue, i_rXmlId );
        }
    }

    
    nCurCol = nColsReq;
    while( nCurCol<GetColumnCount() && GetCell(nCurRow,nCurCol)->IsUsed() )
        nCurCol++;
}

void SwXMLTableContext::InsertRow( const OUString& rStyleName,
                                   const OUString& rDfltCellStyleName,
                                   bool bInHead,
                                   const OUString & i_rXmlId )
{
    OSL_ENSURE( nCurRow < USHRT_MAX,
            "SwXMLTableContext::InsertRow: no space left" );
    if( nCurRow >= USHRT_MAX )
        return;

    
    if( 0==nCurRow && 0UL == GetColumnCount()  )
        InsertColumn( USHRT_MAX, true );

    if( nCurRow < pRows->size() )
    {
        
        
        (*pRows)[(sal_uInt16)nCurRow].Set(
            rStyleName, rDfltCellStyleName, i_rXmlId );
    }
    else
    {
        
        pRows->push_back( new SwXMLTableRow_Impl( rStyleName, GetColumnCount(),
                                               &rDfltCellStyleName, i_rXmlId ) );
    }

    
    nCurCol=0UL;

    
    while( nCurCol<GetColumnCount() && GetCell(nCurRow,nCurCol)->IsUsed() )
        nCurCol++;

    if( bInHead  &&  nHeaderRows == nCurRow )
        nHeaderRows++;
}

void SwXMLTableContext::InsertRepRows( sal_uInt32 nCount )
{
    const SwXMLTableRow_Impl *pSrcRow = &(*pRows)[(sal_uInt16)nCurRow-1];
    while( nCount > 1 && IsInsertRowPossible() )
    {
        InsertRow( pSrcRow->GetStyleName(), pSrcRow->GetDefaultCellStyleName(),
                   false );
        while( nCurCol < GetColumnCount() )
        {
            if( !GetCell(nCurRow,nCurCol)->IsUsed() )
            {
                const SwXMLTableCell_Impl *pSrcCell =
                    GetCell( nCurRow-1, nCurCol );
                InsertCell( pSrcCell->GetStyleName(), 1U,
                            pSrcCell->GetColSpan(),
                            InsertTableSection(),
                            OUString(),
                            0, pSrcCell->IsProtected(),
                            &pSrcCell->GetFormula(),
                            pSrcCell->HasValue(), pSrcCell->GetValue(),
                            pSrcCell->GetStringValue() );
            }
        }
        FinishRow();
        nCount--;
    }
}

void SwXMLTableContext::FinishRow()
{
    
    if( nCurCol < GetColumnCount() )
    {
        OUString aStyleName2;
        InsertCell( aStyleName2, 1U, GetColumnCount() - nCurCol,
                    InsertTableSection() );
    }

    
    nCurRow++;
}

const SwStartNode *SwXMLTableContext::GetPrevStartNode( sal_uInt32 nRow,
                                                        sal_uInt32 nCol ) const
{
    const SwXMLTableCell_Impl *pPrevCell = 0;
    if( GetColumnCount() == nCol )
    {
        
        pPrevCell = GetCell( pRows->size()-1U, GetColumnCount()-1UL );
    }
    else if( nCol > 0UL )
    {
        
        pPrevCell = GetCell( nRow, nCol-1UL );
    }
    else if( nRow > 0UL )
    {
        
        pPrevCell = GetCell( nRow-1UL, GetColumnCount()-1UL );
    }

    const SwStartNode *pSttNd = 0;
    if( pPrevCell )
    {
        if( pPrevCell->GetStartNode() )
            pSttNd = pPrevCell->GetStartNode();
        

        else if ( pPrevCell->GetSubTable() )
            pSttNd = pPrevCell->GetSubTable()->GetLastStartNode();

        OSL_ENSURE( pSttNd != 0,
                "table corrupt" );
    }

    return pSttNd;
}

void SwXMLTableContext::FixRowSpan( sal_uInt32 nRow, sal_uInt32 nCol,
                                    sal_uInt32 nColSpan )
{
    sal_uInt32 nLastCol = nCol + nColSpan;
    for( sal_uInt16 i = (sal_uInt16)nCol; i < nLastCol; i++ )
    {
        sal_uInt32 j = nRow;
        sal_uInt32 nRowSpan = 1UL;
        SwXMLTableCell_Impl *pCell = GetCell( j, i );
        while( pCell && pCell->GetRowSpan() > 1UL )
        {
            pCell->SetRowSpan( nRowSpan++ );
            pCell = j > 0UL ? GetCell( --j, i ) : 0;
        }
    }
}

void SwXMLTableContext::ReplaceWithEmptyCell( sal_uInt32 nRow, sal_uInt32 nCol, bool bRows )
{
    const SwStartNode *pPrevSttNd = GetPrevStartNode( nRow, nCol );
    const SwStartNode *pSttNd = InsertTableSection( pPrevSttNd );

    const SwXMLTableCell_Impl *pCell = GetCell( nRow, nCol );
    sal_uInt32 nLastRow = bRows ? nRow + pCell->GetRowSpan() : nRow + 1;
    sal_uInt32 nLastCol = nCol + pCell->GetColSpan();

    for( sal_uInt32 i=nRow; i<nLastRow; i++ )
    {
        SwXMLTableRow_Impl *pRow = &(*pRows)[(sal_uInt16)i];
        for( sal_uInt32 j=nCol; j<nLastCol; j++ )
            pRow->GetCell( j )->SetStartNode( pSttNd );
    }

}

SwTableBox *SwXMLTableContext::NewTableBox( const SwStartNode *pStNd,
                                             SwTableLine *pUpper )
{
    
    
    if( xParentTable.Is() )
        return ((SwXMLTableContext *)&xParentTable)->NewTableBox( pStNd,
                                                                  pUpper );

    SwTableBox *pBox;

    if( pBox1 &&
        pBox1->GetSttNd() == pStNd )
    {
        
        
        pBox = pBox1;
        pBox->SetUpper( pUpper );
        pBox1 = 0;
    }
    else
        pBox = new SwTableBox( pBoxFmt, *pStNd, pUpper );

    return pBox;
}

SwTableBoxFmt* SwXMLTableContext::GetSharedBoxFormat(
    SwTableBox* pBox,
    const OUString& rStyleName,
    sal_Int32 nColumnWidth,
    sal_Bool bProtected,
    sal_Bool bMayShare,
    sal_Bool& bNew,
    bool* pModifyLocked )
{
    if ( pSharedBoxFormats == NULL )
        pSharedBoxFormats = new map_BoxFmt();

    SwTableBoxFmt* pBoxFmt2;

    TableBoxIndex aKey( rStyleName, nColumnWidth, bProtected );
    map_BoxFmt::iterator aIter = pSharedBoxFormats->find( aKey );
    if ( aIter == pSharedBoxFormats->end() )
    {
        

        
        
        pBoxFmt2 = (SwTableBoxFmt*)pBox->ClaimFrmFmt();
        SwFmtFillOrder aFillOrder( pBoxFmt2->GetFillOrder() );
        pBoxFmt2->ResetAllFmtAttr(); 
        pBoxFmt2->SetFmtAttr( aFillOrder );
        bNew = sal_True;    

        
        if ( bMayShare )
            (*pSharedBoxFormats)[ aKey ] = pBoxFmt2;
    }
    else
    {
        
        pBoxFmt2 = aIter->second;
        pBox->ChgFrmFmt( pBoxFmt2 );
        bNew = sal_False;   

        
        if ( !bMayShare )
            pBoxFmt2 = (SwTableBoxFmt*)pBox->ClaimFrmFmt();
    }

    
    if ( pModifyLocked != NULL )
    {
        (*pModifyLocked) = pBoxFmt2->IsModifyLocked();
        pBoxFmt2->LockModify();
    }

    return pBoxFmt2;
}

SwTableBox *SwXMLTableContext::MakeTableBox( SwTableLine *pUpper,
                                             sal_uInt32 nTopRow,
                                             sal_uInt32 nLeftCol,
                                             sal_uInt32 nBottomRow,
                                             sal_uInt32 nRightCol )
{
    //FIXME: here would be a great place to handle XmlId for cell
    SwTableBox *pBox = new SwTableBox( pBoxFmt, 0, pUpper );

    sal_uInt32 nColSpan = nRightCol - nLeftCol;
    sal_Int32 nColWidth = GetColumnWidth( nLeftCol, nColSpan );

    
    SwFrmFmt *pFrmFmt = pBox->ClaimFrmFmt();
    SwFmtFillOrder aFillOrder( pFrmFmt->GetFillOrder() );
    pFrmFmt->ResetAllFmtAttr(); 
    pFrmFmt->SetFmtAttr( aFillOrder );

    pFrmFmt->SetFmtAttr( SwFmtFrmSize( ATT_VAR_SIZE, nColWidth ) );

    SwTableLines& rLines = pBox->GetTabLines();
    bool bSplitted = false;

    while( !bSplitted )
    {
        sal_uInt32 nStartRow = nTopRow;
        sal_uInt32 i;

        for( i = nTopRow; i < nBottomRow; i++ )
        {
            
            sal_Bool bSplit = sal_True;
            SwXMLTableRow_Impl *pRow = &(*pRows)[(sal_uInt16)i];
            for( sal_uInt32 j=nLeftCol; j<nRightCol; j++ )
            {
                bSplit = ( 1UL == pRow->GetCell(j)->GetRowSpan() );
                if( !bSplit )
                    break;
            }
            if( bSplit && (nStartRow>nTopRow || i+1UL<nBottomRow) )
            {
                SwTableLine *pLine =
                    MakeTableLine( pBox, nStartRow, nLeftCol, i+1UL,
                                   nRightCol );

                rLines.push_back( pLine );

                nStartRow = i+1UL;
                bSplitted = true;
            }
        }
        if( !bSplitted )
        {
            
            

            nStartRow = nTopRow;
            while( nStartRow < nBottomRow )
            {
                sal_uInt32 nMaxRowSpan = 0UL;
                SwXMLTableRow_Impl *pStartRow = &(*pRows)[(sal_uInt16)nStartRow];
                const SwXMLTableCell_Impl *pCell;
                for( i=nLeftCol; i<nRightCol; i++ )
                    if( ( pCell=pStartRow->GetCell(i),
                          pCell->GetRowSpan() > nMaxRowSpan ) )
                        nMaxRowSpan = pCell->GetRowSpan();

                nStartRow += nMaxRowSpan;
                if( nStartRow<nBottomRow )
                {
                    SwXMLTableRow_Impl *pPrevRow =
                                        &(*pRows)[(sal_uInt16)nStartRow-1U];
                    i = nLeftCol;
                    while( i < nRightCol )
                    {
                        if( pPrevRow->GetCell(i)->GetRowSpan() > 1UL )
                        {
                            const SwXMLTableCell_Impl *pCell2 =
                                GetCell( nStartRow, i );
                            const sal_uInt32 nColSpan2 = pCell2->GetColSpan();
                            FixRowSpan( nStartRow-1UL, i, nColSpan2 );
                            ReplaceWithEmptyCell( nStartRow, i, true );
                            i += nColSpan2;
                        }
                        else
                        {
                            i++;
                        }
                    }
                }
            }
            
        }
    }

    return pBox;
}

SwTableBox *SwXMLTableContext::MakeTableBox(
        SwTableLine *pUpper, const SwXMLTableCell_Impl *pCell,
        sal_uInt32 /*nTopRow*/, sal_uInt32 nLeftCol, sal_uInt32 /*nBottomRow*/,
        sal_uInt32 nRightCol )
{
    //FIXME: here would be a great place to handle XmlId for cell
    SwTableBox *pBox;
    sal_uInt32 nColSpan = nRightCol - nLeftCol;
    sal_Int32 nColWidth = GetColumnWidth( nLeftCol, nColSpan );

    if( pCell->GetStartNode() )
    {
        pBox = NewTableBox( pCell->GetStartNode(), pUpper );
    }
    else
    {
        
        
        pBox = new SwTableBox( pBoxFmt, 0, pUpper );
        pCell->GetSubTable()->MakeTable( pBox, nColWidth );
    }

    
    OUString sStyleName = pCell->GetStyleName();
    bool bModifyLocked;
    sal_Bool bNew;
    SwTableBoxFmt *pBoxFmt2 = GetSharedBoxFormat(
        pBox, sStyleName, nColWidth, pCell->IsProtected(),
        pCell->GetStartNode() && pCell->GetFormula().isEmpty() &&
            ! pCell->HasValue(),
        bNew, &bModifyLocked  );

    
    if ( bNew )
    {
        
        const SfxItemSet *pAutoItemSet = 0;
        if( pCell->GetStartNode() && !sStyleName.isEmpty() &&
            GetSwImport().FindAutomaticStyle(
                XML_STYLE_FAMILY_TABLE_CELL, sStyleName, &pAutoItemSet ) )
        {
            if( pAutoItemSet )
                pBoxFmt2->SetFmtAttr( *pAutoItemSet );
        }
    }

    if( pCell->GetStartNode() )
    {
        if (pCell->HasStringValue())
        {
            SwNodeIndex const aNodeIndex(*(pCell->GetStartNode()), 1);
            SwTxtNode *const pTxtNode(aNodeIndex.GetNode().GetTxtNode());
            SAL_WARN_IF(!pTxtNode, "sw", "Should have a text node in cell?");
            if (pTxtNode)
            {
                SAL_WARN_IF(!pTxtNode->GetTxt().isEmpty(), "sw",
                        "why text here?");
                pTxtNode->InsertText(*pCell->GetStringValue(),
                        SwIndex(pTxtNode, 0));
            }
        }

        
        
        
        
        
        
        
        bool bSuppressNumericContent = false;
        if( pCell->HasValue() && (pCell->GetValue() == 0.0) &&
            pCell->GetFormula().isEmpty() &&
            !sStyleName.isEmpty() )
        {
            
            const SfxPoolItem* pItem = NULL;
            if( pBoxFmt2->GetItemState( RES_BOXATR_FORMAT, sal_False, &pItem )
                            == SFX_ITEM_SET )
            {
                const SwTblBoxNumFormat* pNumFormat =
                    static_cast<const SwTblBoxNumFormat*>( pItem );
                if( ( pNumFormat != NULL ) && ( pNumFormat->GetValue() == 0 ) )
                {
                    
                    SwNodeIndex aNodeIndex( *(pCell->GetStartNode()), 1 );
                    if( ( aNodeIndex.GetNode().EndOfSectionIndex() -
                          aNodeIndex.GetNode().StartOfSectionIndex() ) == 2 )
                    {
                        SwTxtNode* pTxtNode= aNodeIndex.GetNode().GetTxtNode();
                        if( pTxtNode != NULL )
                        {
                            
                            const OUString& rText = pTxtNode->GetTxt();
                            if( ( rText.getLength() > 10 ) ||
                                ( rText.indexOf( '0' ) == -1 ) )
                            {
                                bSuppressNumericContent = true;
                            }
                        }
                    }
                    else
                        bSuppressNumericContent = true; 
                }
            }
        }

        if( bSuppressNumericContent )
        {
            
            pBoxFmt2->ResetFmtAttr( RES_BOXATR_FORMULA );
            pBoxFmt2->ResetFmtAttr( RES_BOXATR_FORMAT );
            pBoxFmt2->ResetFmtAttr( RES_BOXATR_VALUE );
        }
        else
        {
            

            const OUString& rFormula = pCell->GetFormula();
            if (!rFormula.isEmpty())
            {
                
                SwTblBoxFormula aFormulaItem( rFormula );
                pBoxFmt2->SetFmtAttr( aFormulaItem );
            }
            else if (!pCell->HasValue() && pCell->HasStringValue())
            {
                
                
                
                
                const SfxPoolItem* pItem = NULL;
                if( pBoxFmt->GetItemState( RES_BOXATR_FORMAT, sal_False, &pItem )
                    == SFX_ITEM_SET )
                {
                    const SwDoc* pDoc = pBoxFmt->GetDoc();
                    const SvNumberFormatter* pNumberFormatter = pDoc ?
                        pDoc->GetNumberFormatter() : 0;
                    const SwTblBoxNumFormat* pNumFormat =
                        static_cast<const SwTblBoxNumFormat*>( pItem );
                    if( pNumFormat != NULL && pNumberFormatter &&
                        !pNumberFormatter->GetEntry( pNumFormat->GetValue() )->IsTextFormat() )
                        pBoxFmt->ResetFmtAttr( RES_BOXATR_FORMAT );
                }
            }
            
            if( pCell->HasValue() )
            {
                SwTblBoxValue aValueItem( pCell->GetValue() );
                pBoxFmt2->SetFmtAttr( aValueItem );
            }
        }

        
        pBox->ActualiseValueBox();
    }

    
    if( pCell->IsProtected() )
    {
        SvxProtectItem aProtectItem( RES_PROTECT );
        aProtectItem.SetCntntProtect( sal_True );
        pBoxFmt2->SetFmtAttr( aProtectItem );
    }

    
    if (! bModifyLocked)
        pBoxFmt2->UnlockModify();

    pBoxFmt2->SetFmtAttr( SwFmtFrmSize( ATT_VAR_SIZE, nColWidth ) );

    return pBox;
}

SwTableLine *SwXMLTableContext::MakeTableLine( SwTableBox *pUpper,
                                               sal_uInt32 nTopRow,
                                               sal_uInt32 nLeftCol,
                                               sal_uInt32 nBottomRow,
                                               sal_uInt32 nRightCol )
{
    //FIXME: here would be a great place to handle XmlId for row
    SwTableLine *pLine;
    if( !pUpper && 0UL==nTopRow )
    {
        pLine = pTableNode->GetTable().GetTabLines()[0U];
    }
    else
    {
        pLine = new SwTableLine( pLineFmt, 0, pUpper );
    }

    
    SwFrmFmt *pFrmFmt = pLine->ClaimFrmFmt();
    SwFmtFillOrder aFillOrder( pFrmFmt->GetFillOrder() );
    pFrmFmt->ResetAllFmtAttr(); 
    pFrmFmt->SetFmtAttr( aFillOrder );

    const SfxItemSet *pAutoItemSet = 0;
    const OUString& rStyleName = (*pRows)[(sal_uInt16)nTopRow].GetStyleName();
    if( 1UL == (nBottomRow - nTopRow) &&
        !rStyleName.isEmpty() &&
        GetSwImport().FindAutomaticStyle(
            XML_STYLE_FAMILY_TABLE_ROW, rStyleName, &pAutoItemSet ) )
    {
        if( pAutoItemSet )
            pFrmFmt->SetFmtAttr( *pAutoItemSet );
    }

    SwTableBoxes& rBoxes = pLine->GetTabBoxes();

    sal_uInt32 nStartCol = nLeftCol;
    while( nStartCol < nRightCol )
    {
        for( sal_uInt32 nRow=nTopRow; nRow<nBottomRow; nRow++ )
            (*pRows)[(sal_uInt16)nRow].SetSplitable( sal_True );

        sal_uInt32 nCol = nStartCol;
        sal_uInt32 nSplitCol = nRightCol;
        bool bSplitted = false;
        while( !bSplitted )
        {
            OSL_ENSURE( nCol < nRightCol, "Zu weit gelaufen" );

            
            
            
            bool bSplit = true;
            bool bHoriSplitMayContinue = false;
            bool bHoriSplitPossible = false;

            if ( bHasSubTables )
            {
                
                for( sal_uInt32 nRow=nTopRow; nRow<nBottomRow; nRow++ )
                {
                    SwXMLTableCell_Impl *pCell = GetCell(nRow,nCol);
                    
                    
                    bool bHoriSplit = (*pRows)[(sal_uInt16)nRow].IsSplitable() &&
                                      nRow+1UL < nBottomRow &&
                                      1UL == pCell->GetRowSpan();
                    (*pRows)[(sal_uInt16)nRow].SetSplitable( bHoriSplit );

                    
                    
                    bSplit &= ( 1UL == pCell->GetColSpan() );
                    if( bSplit )
                    {
                        bHoriSplitPossible |= bHoriSplit;

                        
                        
                        bHoriSplit &= (nCol+1UL < nRightCol &&
                                       1UL == GetCell(nRow,nCol+1UL)->GetRowSpan());
                        bHoriSplitMayContinue |= bHoriSplit;
                    }
                }
            }
            else
            {
                
                SwXMLTableCell_Impl *pCell = GetCell(nTopRow,nCol);

                
                if ( pCell == 0 )
                {
                    OSL_FAIL( "table seems to be corrupt." );
                    break;
                }

                
                
                bSplit = 1UL == pCell->GetColSpan();
            }

#if OSL_DEBUG_LEVEL > 0
            if( nCol == nRightCol-1UL )
            {
                OSL_ENSURE( bSplit, "Split-Flag falsch" );
                if ( bHasSubTables )
                {
                    OSL_ENSURE( !bHoriSplitMayContinue,
                            "HoriSplitMayContinue-Flag falsch" );
                    SwXMLTableCell_Impl *pTmpCell = GetCell( nTopRow, nStartCol );
                    OSL_ENSURE( pTmpCell->GetRowSpan() != (nBottomRow-nTopRow) ||
                            !bHoriSplitPossible, "HoriSplitPossible-Flag falsch" );
                }
            }
#endif

            OSL_ENSURE( !bHasSubTables || !bHoriSplitMayContinue || bHoriSplitPossible,
                    "bHoriSplitMayContinue, aber nicht bHoriSplitPossible" );

            if( bSplit )
            {
                SwTableBox* pBox = 0;
                SwXMLTableCell_Impl *pCell = GetCell( nTopRow, nStartCol );
                
                if( ( !bHasSubTables || ( pCell->GetRowSpan() == (nBottomRow-nTopRow) ) ) &&
                    pCell->GetColSpan() == (nCol+1UL-nStartCol) &&
                    ( pCell->GetStartNode() || pCell->GetSubTable() ) )
                {
                    
                    long nBoxRowSpan = 1;
                    if ( !bHasSubTables )
                    {
                        nBoxRowSpan = pCell->GetRowSpan();
                        if ( pCell->IsCovered() )
                        {
                            nBoxRowSpan = -1 * nBoxRowSpan;
                            ReplaceWithEmptyCell( nTopRow, nStartCol, false );
                        }
                    }

                    
                    
                    nSplitCol = nCol + 1UL;

                    pBox = MakeTableBox( pLine, pCell,
                                         nTopRow, nStartCol,
                                         nBottomRow, nSplitCol );

                    if ( 1 != nBoxRowSpan )
                        pBox->setRowSpan( nBoxRowSpan );

                    bSplitted = true;
                }
                else if( bHasSubTables && bHoriSplitPossible && bHoriSplitMayContinue )
                {
                    
                    
                    
                    
                    
                    
                    nSplitCol = nCol + 1UL;
                }
                else if ( bHasSubTables )
                {
                    
                    
                    
                    
                    
                    
                    
                    if( bHoriSplitPossible || nSplitCol > nCol+1 )
                    {
                        OSL_ENSURE( !bHoriSplitMayContinue,
                                "bHoriSplitMayContinue==sal_True" );
                        OSL_ENSURE( bHoriSplitPossible || nSplitCol == nRightCol,
                                "bHoriSplitPossible-Flag sollte gesetzt sein" );

                        nSplitCol = nCol + 1UL;
                    }

                    pBox = MakeTableBox( pLine, nTopRow, nStartCol,
                                         nBottomRow, nSplitCol );
                    bSplitted = true;
                }

                OSL_ENSURE( bHasSubTables || pBox, "Colspan trouble" );

                if( pBox )
                    rBoxes.push_back( pBox );
            }
            nCol++;
        }
        nStartCol = nSplitCol;
    }

    return pLine;
}

void SwXMLTableContext::_MakeTable( SwTableBox *pBox )
{
    
    std::vector<ColumnWidthInfo>::iterator colIter;
    sal_uInt32 nCols = GetColumnCount();

    
    

    if( pRows->size() > nCurRow )
    {
        SwXMLTableRow_Impl *pPrevRow = &(*pRows)[(sal_uInt16)nCurRow-1U];
        const SwXMLTableCell_Impl *pCell;
        for( sal_uLong i = 0; i < aColumnWidths.size(); ++i )
        {
            if( ( pCell=pPrevRow->GetCell(i), pCell->GetRowSpan() > 1UL ) )
            {
                FixRowSpan( nCurRow-1UL, i, 1UL );
            }
        }
        for( sal_uLong i = pRows->size()-1UL; i>=nCurRow; --i )
            pRows->pop_back();
    }

    if( pRows->empty() )
    {
        OUString aStyleName2;
        InsertCell( aStyleName2, 1U, nCols, InsertTableSection() );
    }

    
    
    sal_Int32 nAbsWidth = 0L;
    sal_Int32 nMinAbsColWidth = 0L;
    sal_Int32 nRelWidth = 0L;
    sal_Int32 nMinRelColWidth = 0L;
    sal_uInt32 nRelCols = 0UL;
    for( colIter = aColumnWidths.begin(); colIter < aColumnWidths.end(); ++colIter)
    {
        if( colIter->isRelative )
        {
            nRelWidth += colIter->width;
            if( 0L == nMinRelColWidth || colIter->width < nMinRelColWidth )
                nMinRelColWidth = colIter->width;
            nRelCols++;
        }
        else
        {
            nAbsWidth += colIter->width;
            if( 0L == nMinAbsColWidth || colIter->width < nMinAbsColWidth )
                nMinAbsColWidth = colIter->width;
        }
    }
    sal_uInt32 nAbsCols = nCols - nRelCols;

    if( bRelWidth )
    {
        
        
        if( nAbsCols > 0UL )
        {
            
            
            
            
            
            if( 0L == nMinRelColWidth )
                nMinRelColWidth = nMinAbsColWidth;

            for( colIter = aColumnWidths.begin(); nAbsCols > 0UL && colIter < aColumnWidths.end(); ++colIter)
            {
                if( !colIter->isRelative )
                {
                    sal_Int32 nRelCol = ( colIter->width * nMinRelColWidth) / nMinAbsColWidth;
                    colIter->width = nRelCol;
                    colIter->isRelative = true;
                    nRelWidth += nRelCol;
                    nAbsCols--;
                }
            }
        }

        if( !nWidth )
        {
            
            
            
            
            nWidth = nRelWidth > USHRT_MAX ? USHRT_MAX : nRelWidth;
        }
        if( nRelWidth != nWidth && nRelWidth && nCols )
        {
            double n = (double)nWidth / (double)nRelWidth;
            nRelWidth = 0L;
            for( colIter = aColumnWidths.begin(); colIter < aColumnWidths.end() - 1; ++colIter)
            {
                sal_Int32 nW = (sal_Int32)( colIter->width * n);
                colIter->width = (sal_uInt16)nW;
                nRelWidth += nW;
            }
            aColumnWidths.back().width = (nWidth-nRelWidth);
        }
    }
    else
    {
        
        
        if( nRelCols > 0UL )
        {
            
            
            sal_Int32 nAbsForRelWidth =
                    nWidth > nAbsWidth ? nWidth - nAbsWidth : (sal_Int32)0L;

            
            
            sal_Int32 nExtraRel = nRelWidth - (nRelCols * nMinRelColWidth);

            
            
            sal_Int32 nMinAbs = nRelCols * MINLAY;
            sal_Int32 nExtraAbs =
                    nAbsForRelWidth > nMinAbs ? nAbsForRelWidth - nMinAbs : (sal_Int32)0L;

            bool bMin = false;      
            bool bMinExtra = false; 
                                    

            if( nAbsForRelWidth <= nMinAbs )
            {
                
                
                nAbsForRelWidth = nMinAbs;
                bMin = true;
            }
            else if( nAbsForRelWidth <= (nRelWidth * MINLAY) /
                                        nMinRelColWidth )
            {
                
                
                
                
                
                bMinExtra = true;
            }
            
            

            for( colIter = aColumnWidths.begin(); nRelCols > 0UL && colIter < aColumnWidths.end(); ++colIter )
            {
                if( colIter->isRelative )
                {
                    sal_Int32 nAbsCol;
                    if( 1UL == nRelCols )
                    {
                        
                        
                        nAbsCol = nAbsForRelWidth;
                    }
                    else
                    {
                        if( bMin )
                        {
                            nAbsCol = MINLAY;
                        }
                        else if( bMinExtra )
                        {
                            sal_Int32 nExtraRelCol = colIter->width - nMinRelColWidth;
                            nAbsCol = MINLAY + (nExtraRelCol * nExtraAbs) /
                                                 nExtraRel;
                        }
                        else
                        {
                            nAbsCol = ( colIter->width * nAbsForRelWidth) / nRelWidth;
                        }
                    }
                    colIter->width = nAbsCol;
                    colIter->isRelative = false;
                    nAbsForRelWidth -= nAbsCol;
                    nAbsWidth += nAbsCol;
                    nRelCols--;
                }
            }
        }

        if( nCols && nAbsWidth )
        {
            if( nAbsWidth < nWidth )
            {
                
                
                sal_Int32 nExtraAbs = nWidth - nAbsWidth;
                sal_Int32 nAbsLastCol = aColumnWidths.back().width + nExtraAbs;
                for( colIter = aColumnWidths.begin(); colIter < aColumnWidths.end()-1UL; ++colIter )
                {
                    sal_Int32 nAbsCol = colIter->width;
                    sal_Int32 nExtraAbsCol = (nAbsCol * nExtraAbs) /
                                             nAbsWidth;
                    nAbsCol += nExtraAbsCol;
                    colIter->width = nAbsCol;
                    nAbsLastCol -= nExtraAbsCol;
                }
                aColumnWidths.back().width = nAbsLastCol;
            }
            else if( nAbsWidth > nWidth )
            {
                
                
                
                sal_Int32 nExtraAbs = nWidth - (nCols * MINLAY);
                sal_Int32 nAbsLastCol = MINLAY + nExtraAbs;
                for( colIter = aColumnWidths.begin(); colIter < aColumnWidths.end()-1UL; ++colIter )
                {
                    sal_Int32 nAbsCol = colIter->width;
                    sal_Int32 nExtraAbsCol = (nAbsCol * nExtraAbs) /
                                             nAbsWidth;
                    nAbsCol = MINLAY + nExtraAbsCol;
                    colIter->width = nAbsCol;
                    nAbsLastCol -= nExtraAbsCol;
                }
                aColumnWidths.back().width = nAbsLastCol;
            }
        }
    }

    SwTableLines& rLines =
        pBox ? pBox->GetTabLines()
             : pTableNode->GetTable().GetTabLines();

    sal_uInt32 nStartRow = 0UL;
    sal_uInt32 nRows = pRows->size();
    for(sal_uInt32 i=0UL; i<nRows; ++i )
    {
        
        bool bSplit = true;
        if ( bHasSubTables )
        {
            SwXMLTableRow_Impl *pRow = &(*pRows)[(sal_uInt16)i];
            for( sal_uInt32 j=0UL; j<nCols; j++ )
            {
                bSplit = ( 1UL == pRow->GetCell(j)->GetRowSpan() );
                if( !bSplit )
                    break;
            }
        }

        if( bSplit )
        {
            SwTableLine *pLine =
                MakeTableLine( pBox, nStartRow, 0UL, i+1UL, nCols );
            if( pBox || nStartRow>0UL )
                rLines.push_back( pLine );
            nStartRow = i+1UL;
        }
    }
}

void SwXMLTableContext::MakeTable()
{
    
    
    
    SolarMutexGuard aGuard;

    
    if (!pRows || pRows->empty() || !GetColumnCount())
    {
        OSL_FAIL("invalid table: no cells; deleting...");
        pTableNode->GetDoc()->DeleteSection( pTableNode );
        pTableNode = 0;
        pBox1 = 0;
        pSttNd1 = 0;
        return;
    }

    SwXMLImport& rSwImport = GetSwImport();

    SwFrmFmt *pFrmFmt = pTableNode->GetTable().GetFrmFmt();

    sal_Int16 eHoriOrient = text::HoriOrientation::FULL;
    bool bSetHoriOrient = false;

    sal_uInt16 nPrcWidth = 0U;

    pTableNode->GetTable().SetRowsToRepeat( nHeaderRows );
    pTableNode->GetTable().SetTableModel( !bHasSubTables );

    const SfxItemSet *pAutoItemSet = 0;
    if( !aStyleName.isEmpty() &&
        rSwImport.FindAutomaticStyle(
            XML_STYLE_FAMILY_TABLE_TABLE, aStyleName, &pAutoItemSet ) &&
         pAutoItemSet )
    {
        const SfxPoolItem *pItem;
        const SvxLRSpaceItem *pLRSpace = 0;
        if( SFX_ITEM_SET == pAutoItemSet->GetItemState( RES_LR_SPACE, false,
                                                        &pItem ) )
            pLRSpace = (const SvxLRSpaceItem *)pItem;

        if( SFX_ITEM_SET == pAutoItemSet->GetItemState( RES_HORI_ORIENT, false,
                                                        &pItem ) )
        {
            eHoriOrient = ((const SwFmtHoriOrient *)pItem)->GetHoriOrient();
            switch( eHoriOrient )
            {
            case text::HoriOrientation::FULL:
                if( pLRSpace )
                {
                    eHoriOrient = text::HoriOrientation::NONE;
                    bSetHoriOrient = true;
                }
                break;
            case text::HoriOrientation::LEFT:
                if( pLRSpace )
                {
                    eHoriOrient = text::HoriOrientation::LEFT_AND_WIDTH;
                    bSetHoriOrient = true;
                }
                break;
            default:
                ;
            }
        }
        else
        {
            bSetHoriOrient = true;
        }

        const SwFmtFrmSize *pSize = 0;
        if( SFX_ITEM_SET == pAutoItemSet->GetItemState( RES_FRM_SIZE, false,
                                                        &pItem ) )
            pSize = (const SwFmtFrmSize *)pItem;

        switch( eHoriOrient )
        {
        case text::HoriOrientation::FULL:
        case text::HoriOrientation::NONE:
            
            
            
            
            
            
            nWidth = USHRT_MAX;
            break;
        default:
            if( pSize )
            {
                if( pSize->GetWidthPercent() )
                {
                    
                    nPrcWidth = pSize->GetWidthPercent();
                }
                else
                {
                    nWidth = pSize->GetWidth();
                    if( nWidth < (sal_Int32)GetColumnCount() * MINLAY )
                    {
                        nWidth = GetColumnCount() * MINLAY;
                    }
                    else if( nWidth > USHRT_MAX )
                    {
                        nWidth = USHRT_MAX;
                    }
                    bRelWidth = false;
                }
            }
            else
            {
                eHoriOrient = text::HoriOrientation::LEFT_AND_WIDTH == eHoriOrient
                                    ? text::HoriOrientation::NONE : text::HoriOrientation::FULL;
                bSetHoriOrient = true;
                nWidth = USHRT_MAX;
            }
            break;
        }

        pFrmFmt->SetFmtAttr( *pAutoItemSet );
    }
    else
    {
        bSetHoriOrient = true;
        nWidth = USHRT_MAX;
    }

    SwTableLine *pLine1 = pTableNode->GetTable().GetTabLines()[0U];
    OSL_ENSURE( pBox1 == pLine1->GetTabBoxes()[0U],
                "Why is box 1 change?" );
    pBox1->pSttNd = pSttNd1;
    pLine1->GetTabBoxes().erase( pLine1->GetTabBoxes().begin() );

    pLineFmt = (SwTableLineFmt*)pLine1->GetFrmFmt();
    pBoxFmt = (SwTableBoxFmt*)pBox1->GetFrmFmt();

    _MakeTable( 0 );

    if( bSetHoriOrient )
        pFrmFmt->SetFmtAttr( SwFmtHoriOrient( 0, eHoriOrient ) );

    
    
    pFrmFmt->LockModify();
    SwFmtFrmSize aSize( ATT_VAR_SIZE, nWidth );
    aSize.SetWidthPercent( (sal_Int8)nPrcWidth );
    pFrmFmt->SetFmtAttr( aSize );
    pFrmFmt->UnlockModify();


    for( sal_uInt16 i=0; i<pRows->size(); i++ )
        (*pRows)[i].Dispose();

    
    if (NULL != pDDESource)
    {
        
        
        SwDDEFieldType* pFldType = lcl_GetDDEFieldType( pDDESource,
                                                        pTableNode );

        
        pDDESource->ReleaseRef();

        
        SwDDETable* pDDETable = new SwDDETable( pTableNode->GetTable(),
                                                pFldType, sal_False );

        
        pTableNode->SetNewTable(pDDETable, sal_False);
    }

    
    if( pTableNode->GetDoc()->GetCurrentViewShell() )
    {
        pTableNode->DelFrms();
        SwNodeIndex aIdx( *pTableNode->EndOfSectionNode(), 1 );
        pTableNode->MakeFrms( &aIdx );
    }
}

void SwXMLTableContext::MakeTable( SwTableBox *pBox, sal_Int32 nW )
{
    //FIXME: here would be a great place to handle XmlId for subtable
    pLineFmt = GetParentTable()->pLineFmt;
    pBoxFmt = GetParentTable()->pBoxFmt;
    nWidth = nW;
    bRelWidth = GetParentTable()->bRelWidth;

    _MakeTable( pBox );

    for (size_t i=0; i < pRows->size(); ++i) 
        (*pRows)[i].Dispose();
}

const SwStartNode *SwXMLTableContext::InsertTableSection(
        const SwStartNode *const pPrevSttNd,
        OUString const*const pStringValueStyleName)
{
    
    
    if( xParentTable.Is() )
        return static_cast<SwXMLTableContext *>(&xParentTable)
                    ->InsertTableSection(pPrevSttNd, pStringValueStyleName);

    const SwStartNode *pStNd;
    Reference<XUnoTunnel> xCrsrTunnel( GetImport().GetTextImport()->GetCursor(),
                                       UNO_QUERY);
    OSL_ENSURE( xCrsrTunnel.is(), "missing XUnoTunnel for Cursor" );
    OTextCursorHelper *pTxtCrsr = reinterpret_cast< OTextCursorHelper * >(
            sal::static_int_cast< sal_IntPtr >( xCrsrTunnel->getSomething( OTextCursorHelper::getUnoTunnelId() )));
    OSL_ENSURE( pTxtCrsr, "SwXTextCursor missing" );

    if( bFirstSection )
    {
        
        pStNd = pTxtCrsr->GetPaM()->GetNode()->FindTableBoxStartNode();
        bFirstSection = false;
        OUString sStyleName("Standard");
        GetImport().GetTextImport()->SetStyleAndAttrs( GetImport(),
            GetImport().GetTextImport()->GetCursor(), sStyleName, sal_True );
    }
    else
    {
        SwDoc* pDoc = SwImport::GetDocFromXMLImport( GetSwImport() );
        const SwEndNode *pEndNd = pPrevSttNd ? pPrevSttNd->EndOfSectionNode()
                                             : pTableNode->EndOfSectionNode();
        
        OSL_ENSURE( pDoc, "<SwXMLTableContext::InsertTableSection(..)> - no <pDoc> at <SwXTextCursor> instance - <SwXTextCurosr> doesn't seem to be registered at a <SwUnoCrsr> instance." );
        if ( !pDoc )
        {
            pDoc = const_cast<SwDoc*>(pEndNd->GetDoc());
        }
        sal_uInt32 nOffset = pPrevSttNd ? 1UL : 0UL;
        SwNodeIndex aIdx( *pEndNd, nOffset );
        SwTxtFmtColl *pColl =
            pDoc->GetTxtCollFromPool( RES_POOLCOLL_STANDARD, false );
        pStNd = pDoc->GetNodes().MakeTextSection( aIdx, SwTableBoxStartNode,
                                                 pColl );
        
        if( !pPrevSttNd && pBox1 != NULL )

        {
            pBox1->pSttNd = pStNd;
            SwCntntNode *pCNd = pDoc->GetNodes()[ pStNd->GetIndex() + 1 ]
                                                            ->GetCntntNode();
            SwPosition aPos( *pCNd );
            aPos.nContent.Assign( pCNd, 0U );

            const uno::Reference< text::XTextRange > xTextRange =
                SwXTextRange::CreateXTextRange( *pDoc, aPos, 0 );
            Reference < XText > xText = xTextRange->getText();
            Reference < XTextCursor > xTextCursor =
                xText->createTextCursorByRange( xTextRange );
            GetImport().GetTextImport()->SetCursor( xTextCursor );
        }
    }

    if (pStringValueStyleName)
    {   
        GetImport().GetTextImport()->SetStyleAndAttrs( GetImport(),
            GetImport().GetTextImport()->GetCursor(), *pStringValueStyleName,
            true, false, -1, false); 
    }

    return pStNd;
}

void SwXMLTableContext::EndElement()
{
    if( IsValid() && !xParentTable.Is() )
    {
        MakeTable();
        GetImport().GetTextImport()->SetCursor( xOldCursor );
    }
}

Reference < XTextContent > SwXMLTableContext::GetXTextContent() const
{
    return xTextContent;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
