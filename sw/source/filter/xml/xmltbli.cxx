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

#include <hintids.hxx>

#include <com/sun/star/lang/IndexOutOfBoundsException.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/text/XTextTable.hpp>
#include <com/sun/star/table/XCellRange.hpp>
#include <o3tl/numeric.hxx>
#include <o3tl/make_unique.hxx>
#include <o3tl/safeint.hxx>
#include <sal/log.hxx>
#include <svl/itemset.hxx>
#include <svl/zformat.hxx>
#include <sax/tools/converter.hxx>
#include <unotools/configmgr.hxx>
#include <xmloff/xmlnmspe.hxx>
#include <xmloff/xmltkmap.hxx>
#include <xmloff/nmspmap.hxx>

#include <xmloff/families.hxx>
#include <xmloff/xmluconv.hxx>
#include <xmloff/i18nmap.hxx>
#include <editeng/protitem.hxx>
#include <editeng/lrspitem.hxx>
#include <poolfmt.hxx>
#include <fmtfsize.hxx>
#include <fmtornt.hxx>
#include <fmtfordr.hxx>
#include <doc.hxx>
#include <IDocumentFieldsAccess.hxx>
#include <IDocumentLayoutAccess.hxx>
#include <IDocumentStylePoolAccess.hxx>
#include <swtable.hxx>
#include <swtblfmt.hxx>
#include <pam.hxx>
#include <unotbl.hxx>
#include <unotextrange.hxx>
#include <unocrsr.hxx>
#include <cellatr.hxx>
#include <swddetbl.hxx>
#include <ddefld.hxx>
#include <sfx2/linkmgr.hxx>
#include "xmlimp.hxx"
#include "xmltbli.hxx"
#include <vcl/svapp.hxx>
#include <ndtxt.hxx>

#include <algorithm>
#include <vector>
#include <memory>

#include <limits.h>


using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::text;
using namespace ::com::sun::star::table;
using namespace ::com::sun::star::xml::sax;
using namespace ::xmloff::token;

enum SwXMLTableElemTokens
{
    XML_TOK_TABLE_HEADER_COLS,
    XML_TOK_TABLE_COLS,
    XML_TOK_TABLE_COL,
    XML_TOK_TABLE_HEADER_ROWS,
    XML_TOK_TABLE_ROWS,
    XML_TOK_TABLE_ROW,
    XML_TOK_OFFICE_DDE_SOURCE,
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
};

static const SvXMLTokenMapEntry aTableElemTokenMap[] =
{
    { XML_NAMESPACE_TABLE, XML_TABLE_HEADER_COLUMNS,
            XML_TOK_TABLE_HEADER_COLS },
    { XML_NAMESPACE_TABLE, XML_TABLE_COLUMNS,           XML_TOK_TABLE_COLS },
    { XML_NAMESPACE_TABLE, XML_TABLE_COLUMN,            XML_TOK_TABLE_COL },
    { XML_NAMESPACE_LO_EXT, XML_TABLE_COLUMN,           XML_TOK_TABLE_COL },
    { XML_NAMESPACE_TABLE, XML_TABLE_HEADER_ROWS,
            XML_TOK_TABLE_HEADER_ROWS },
    { XML_NAMESPACE_TABLE, XML_TABLE_ROWS,              XML_TOK_TABLE_ROWS },
    { XML_NAMESPACE_TABLE, XML_TABLE_ROW,               XML_TOK_TABLE_ROW },
    { XML_NAMESPACE_LO_EXT, XML_TABLE_ROW,              XML_TOK_TABLE_ROW },
    { XML_NAMESPACE_OFFICE, XML_DDE_SOURCE,
            XML_TOK_OFFICE_DDE_SOURCE },

    // There are slight differences between <table:table-columns> and
    // <table:table-columns-groups>. However, none of these are
    // supported in Writer (they are Calc-only features), so we
    // support column groups by simply using the <table:table-columns>
    // token for column groups, too.
    { XML_NAMESPACE_TABLE, XML_TABLE_COLUMN_GROUP,      XML_TOK_TABLE_COLS },

    XML_TOKEN_MAP_END
};

static const SvXMLTokenMapEntry aTableCellAttrTokenMap[] =
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
    { XML_NAMESPACE_TABLE, XML_PROTECT, XML_TOK_TABLE_PROTECTED }, // for backwards compatibility with SRC629 (and before)
    { XML_NAMESPACE_OFFICE, XML_STRING_VALUE, XML_TOK_TABLE_STRING_VALUE },
    { XML_NAMESPACE_OFFICE, XML_VALUE_TYPE, XML_TOK_TABLE_VALUE_TYPE },
    XML_TOKEN_MAP_END
};

const SvXMLTokenMap& SwXMLImport::GetTableElemTokenMap()
{
    if( !m_pTableElemTokenMap )
        m_pTableElemTokenMap.reset(new SvXMLTokenMap( aTableElemTokenMap ));

    return *m_pTableElemTokenMap;
}

const SvXMLTokenMap& SwXMLImport::GetTableCellAttrTokenMap()
{
    if( !m_pTableCellAttrTokenMap )
        m_pTableCellAttrTokenMap.reset(new SvXMLTokenMap( aTableCellAttrTokenMap ));

    return *m_pTableCellAttrTokenMap;
}

class SwXMLTableCell_Impl
{
    OUString aStyleName;

    OUString mXmlId;
    OUString m_StringValue;

    OUString sFormula;  // cell formula; valid if length > 0
    double dValue;      // formula value

    SvXMLImportContextRef   xSubTable;

    const SwStartNode *pStartNode;
    sal_uInt32 nRowSpan;
    sal_uInt32 nColSpan;

    bool bProtected : 1;
    bool bHasValue; // determines whether dValue attribute is valid
    bool mbCovered;
    bool m_bHasStringValue;

public:

    SwXMLTableCell_Impl( sal_uInt32 nRSpan=1, sal_uInt32 nCSpan=1 ) :
        dValue( 0.0 ),
        pStartNode( nullptr ),
        nRowSpan( nRSpan ),
        nColSpan( nCSpan ),
        bProtected( false ),
        bHasValue( false ),
        mbCovered( false )
        , m_bHasStringValue(false)
        {}

    inline void Set( const OUString& rStyleName,
                      sal_uInt32 nRSpan, sal_uInt32 nCSpan,
                     const SwStartNode *pStNd, SwXMLTableContext *pTable,
                     bool bProtect,
                     const OUString* pFormula,
                     bool bHasValue,
                     bool bCovered,
                     double dVal,
                     OUString const*const pStringValue,
                     OUString const& i_rXmlId);

    bool IsUsed() const { return pStartNode!=nullptr ||
                                     xSubTable.is() || bProtected;}

    sal_uInt32 GetRowSpan() const { return nRowSpan; }
    void SetRowSpan( sal_uInt32 nSet ) { nRowSpan = nSet; }
    sal_uInt32 GetColSpan() const { return nColSpan; }
    const OUString& GetStyleName() const { return aStyleName; }
    const OUString& GetFormula() const { return sFormula; }
    double GetValue() const { return dValue; }
    bool HasValue() const { return bHasValue; }
    bool IsProtected() const { return bProtected; }
    bool IsCovered() const { return mbCovered; }
    bool HasStringValue() const { return m_bHasStringValue; }
    OUString const* GetStringValue() const {
        return (m_bHasStringValue) ? &m_StringValue : nullptr;
    }

    const SwStartNode *GetStartNode() const { return pStartNode; }
    inline void SetStartNode( const SwStartNode *pSttNd );

    inline SwXMLTableContext *GetSubTable() const;

    inline void Dispose();
};

inline void SwXMLTableCell_Impl::Set( const OUString& rStyleName,
                                      sal_uInt32 nRSpan, sal_uInt32 nCSpan,
                                      const SwStartNode *pStNd,
                                      SwXMLTableContext *pTable,
                                      bool bProtect,
                                      const OUString* pFormula,
                                      bool bHasVal,
                                      bool bCov,
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
    m_bHasStringValue = (pStringValue != nullptr);
    bProtected = bProtect;

    if (!mbCovered) // ensure uniqueness
    {
        mXmlId = i_rXmlId;
    }

    // set formula, if valid
    if (pFormula != nullptr)
    {
        sFormula = *pFormula;
    }
}

inline void SwXMLTableCell_Impl::SetStartNode( const SwStartNode *pSttNd )
{
    pStartNode = pSttNd;
    xSubTable = nullptr;
}

inline SwXMLTableContext *SwXMLTableCell_Impl::GetSubTable() const
{
    return static_cast<SwXMLTableContext *>(xSubTable.get());
}

inline void SwXMLTableCell_Impl::Dispose()
{
    if( xSubTable.is() )
        xSubTable = nullptr;
}

class SwXMLTableRow_Impl
{
    OUString   aStyleName;
    OUString   aDfltCellStyleName;
    OUString   mXmlId;
    std::vector<std::unique_ptr<SwXMLTableCell_Impl>> m_Cells;
    bool       bSplitable;

public:

    SwXMLTableRow_Impl( const OUString& rStyleName, sal_uInt32 nCells,
                        const OUString *pDfltCellStyleName = nullptr,
                        const OUString& i_rXmlId = OUString() );

    inline SwXMLTableCell_Impl *GetCell( sal_uInt32 nCol );

    inline void Set( const OUString& rStyleName,
                     const OUString& rDfltCellStyleName,
                     const OUString& i_rXmlId );

    void Expand( sal_uInt32 nCells, bool bOneCell );

    void SetSplitable( bool bSet ) { bSplitable = bSet; }
    bool IsSplitable() const { return bSplitable; }

    const OUString& GetStyleName() const { return aStyleName; }
    const OUString& GetDefaultCellStyleName() const { return aDfltCellStyleName; }

    void Dispose();
};

SwXMLTableRow_Impl::SwXMLTableRow_Impl( const OUString& rStyleName,
                                        sal_uInt32 nCells,
                                        const OUString *pDfltCellStyleName,
                                        const OUString& i_rXmlId ) :
    aStyleName( rStyleName ),
    mXmlId( i_rXmlId ),
    bSplitable( false )
{
    if( pDfltCellStyleName  )
        aDfltCellStyleName = *pDfltCellStyleName;
    OSL_ENSURE( nCells <= USHRT_MAX,
            "SwXMLTableRow_Impl::SwXMLTableRow_Impl: too many cells" );
    if( nCells > USHRT_MAX )
        nCells = USHRT_MAX;

    for( sal_uInt32 i=0U; i<nCells; ++i )
    {
        m_Cells.push_back(o3tl::make_unique<SwXMLTableCell_Impl>());
    }
}

inline SwXMLTableCell_Impl *SwXMLTableRow_Impl::GetCell( sal_uInt32 nCol )
{
    OSL_ENSURE( nCol < USHRT_MAX,
            "SwXMLTableRow_Impl::GetCell: column number is to big" );
    // #i95726# - some fault tolerance
    OSL_ENSURE( nCol < m_Cells.size(),
            "SwXMLTableRow_Impl::GetCell: column number is out of bound" );
    return nCol < m_Cells.size() ? m_Cells[nCol].get() : nullptr;
}

void SwXMLTableRow_Impl::Expand( sal_uInt32 nCells, bool bOneCell )
{
    OSL_ENSURE( nCells <= USHRT_MAX,
            "SwXMLTableRow_Impl::Expand: too many cells" );
    if( nCells > USHRT_MAX )
        nCells = USHRT_MAX;

    sal_uInt32 nColSpan = nCells - m_Cells.size();
    for (size_t i = m_Cells.size(); i < nCells; ++i)
    {
        m_Cells.push_back(o3tl::make_unique<SwXMLTableCell_Impl>(
                1UL, bOneCell ? nColSpan : 1UL));
        nColSpan--;
    }

    OSL_ENSURE( nCells <= m_Cells.size(),
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
    for (auto & pCell : m_Cells)
    {
        pCell->Dispose();
    }
}

class SwXMLTableCellContext_Impl : public SvXMLImportContext
{
    OUString m_aStyleName;
    OUString m_sFormula;
    OUString m_sSaveParaDefault;
    OUString mXmlId;
    OUString m_StringValue;

    SvXMLImportContextRef   m_xMyTable;

    double m_fValue;
    bool m_bHasValue;
    bool     m_bHasStringValue;
    bool     m_bValueTypeIsString;
    bool m_bProtect;

    sal_uInt32                  m_nRowSpan;
    sal_uInt32                  m_nColSpan;
    sal_uInt32                  m_nColRepeat;

    bool                    m_bHasTextContent : 1;
    bool                    m_bHasTableContent : 1;

    SwXMLTableContext *GetTable() { return static_cast<SwXMLTableContext *>(m_xMyTable.get()); }

    bool HasContent() const { return m_bHasTextContent || m_bHasTableContent; }
    inline void InsertContent_();
    inline void InsertContent();
    inline void InsertContent( SwXMLTableContext *pTable );

public:

    SwXMLTableCellContext_Impl(
            SwXMLImport& rImport, sal_uInt16 nPrfx, const OUString& rLName,
            const Reference< xml::sax::XAttributeList > & xAttrList,
            SwXMLTableContext *pTable );

    virtual SvXMLImportContextRef CreateChildContext(
            sal_uInt16 nPrefix, const OUString& rLocalName,
            const Reference< xml::sax::XAttributeList > & xAttrList ) override;
    virtual void EndElement() override;

    SwXMLImport& GetSwImport() { return static_cast<SwXMLImport&>(GetImport()); }
};

SwXMLTableCellContext_Impl::SwXMLTableCellContext_Impl(
        SwXMLImport& rImport, sal_uInt16 nPrfx, const OUString& rLName,
        const Reference< xml::sax::XAttributeList > & xAttrList,
        SwXMLTableContext *pTable ) :
    SvXMLImportContext( rImport, nPrfx, rLName ),
    m_sFormula(),
    m_xMyTable( pTable ),
    m_fValue( 0.0 ),
    m_bHasValue( false ),
    m_bHasStringValue(false),
    m_bValueTypeIsString(false),
    m_bProtect( false ),
    m_nRowSpan( 1 ),
    m_nColSpan( 1 ),
    m_nColRepeat( 1 ),
    m_bHasTextContent( false ),
    m_bHasTableContent( false )
{
    m_sSaveParaDefault = GetImport().GetTextImport()->GetCellParaStyleDefault();
    sal_Int16 nAttrCount = xAttrList.is() ? xAttrList->getLength() : 0;
    for( sal_Int16 i=0; i < nAttrCount; i++ )
    {
        const OUString& rAttrName = xAttrList->getNameByIndex( i );

        OUString aLocalName;
        const sal_uInt16 nPrefix =
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
            m_aStyleName = rValue;
            GetImport().GetTextImport()->SetCellParaStyleDefault(rValue);
            break;
        case XML_TOK_TABLE_NUM_COLS_SPANNED:
            m_nColSpan = static_cast<sal_uInt32>(std::max<sal_Int32>(1, rValue.toInt32()));
            if (m_nColSpan > 256)
            {
                SAL_INFO("sw.xml", "ignoring huge table:number-columns-spanned " << m_nColSpan);
                m_nColSpan = 1;
            }
            break;
        case XML_TOK_TABLE_NUM_ROWS_SPANNED:
            m_nRowSpan = static_cast<sal_uInt32>(std::max<sal_Int32>(1, rValue.toInt32()));
            if (m_nRowSpan > 8192 || (m_nRowSpan > 256 && utl::ConfigManager::IsFuzzing()))
            {
                SAL_INFO("sw.xml", "ignoring huge table:number-rows-spanned " << m_nRowSpan);
                m_nRowSpan = 1;
            }
            break;
        case XML_TOK_TABLE_NUM_COLS_REPEATED:
            m_nColRepeat = static_cast<sal_uInt32>(std::max<sal_Int32>(1, rValue.toInt32()));
            if (m_nColRepeat > 256)
            {
                SAL_INFO("sw.xml", "ignoring huge table:number-columns-repeated " << m_nColRepeat);
                m_nColRepeat = 1;
            }
            break;
        case XML_TOK_TABLE_FORMULA:
            {
                OUString sTmp;
                const sal_uInt16 nPrefix2 = GetImport().GetNamespaceMap().
                        GetKeyByAttrName_( rValue, &sTmp );
                m_sFormula = XML_NAMESPACE_OOOW == nPrefix2 ? sTmp : rValue;
            }
            break;
        case XML_TOK_TABLE_VALUE:
            {
                double fTmp;
                if (::sax::Converter::convertDouble(fTmp, rValue))
                {
                    m_fValue = fTmp;
                    m_bHasValue = true;
                }
            }
            break;
        case XML_TOK_TABLE_TIME_VALUE:
            {
                double fTmp;
                if (::sax::Converter::convertDuration(fTmp, rValue))
                {
                    m_fValue = fTmp;
                    m_bHasValue = true;
                }
            }
            break;
        case XML_TOK_TABLE_DATE_VALUE:
            {
                double fTmp;
                if (GetImport().GetMM100UnitConverter().convertDateTime(fTmp,
                                                                      rValue))
                {
                    m_fValue = fTmp;
                    m_bHasValue = true;
                }
            }
            break;
        case XML_TOK_TABLE_BOOLEAN_VALUE:
            {
                bool bTmp(false);
                if (::sax::Converter::convertBool(bTmp, rValue))
                {
                    m_fValue = (bTmp ? 1.0 : 0.0);
                    m_bHasValue = true;
                }
            }
            break;
        case XML_TOK_TABLE_PROTECTED:
            {
                bool bTmp(false);
                if (::sax::Converter::convertBool(bTmp, rValue))
                {
                    m_bProtect = bTmp;
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
                // ignore other types - it would be correct to require
                // matching value-type and $type-value attributes,
                // but we've been reading those without checking forever.
            }
            break;
        }
    }
}

inline void SwXMLTableCellContext_Impl::InsertContent_()
{
    SwStartNode const*const pStartNode( GetTable()->InsertTableSection(nullptr,
            (m_bHasStringValue && m_bValueTypeIsString &&
             !m_aStyleName.isEmpty()) ? & m_aStyleName : nullptr) );
    GetTable()->InsertCell( m_aStyleName, m_nRowSpan, m_nColSpan,
                            pStartNode,
                            mXmlId,
                            nullptr, m_bProtect, &m_sFormula, m_bHasValue, m_fValue,
            (m_bHasStringValue && m_bValueTypeIsString) ? &m_StringValue : nullptr);
}

inline void SwXMLTableCellContext_Impl::InsertContent()
{
    OSL_ENSURE( !HasContent(), "content already there" );
    m_bHasTextContent = true;
    InsertContent_();
}

inline void SwXMLTableCellContext_Impl::InsertContent(
                                                SwXMLTableContext *pTable )
{
    GetTable()->InsertCell( m_aStyleName, m_nRowSpan, m_nColSpan, nullptr, mXmlId, pTable, m_bProtect );
    m_bHasTableContent = true;
}

SvXMLImportContextRef SwXMLTableCellContext_Impl::CreateChildContext(
        sal_uInt16 nPrefix,
        const OUString& rLocalName,
        const Reference< xml::sax::XAttributeList > & xAttrList )
{
    SvXMLImportContext *pContext = nullptr;

    bool bSubTable = false;
    if( XML_NAMESPACE_TABLE == nPrefix &&
        IsXMLToken( rLocalName, XML_TABLE ) )
    {
        sal_Int16 nAttrCount = xAttrList.is() ? xAttrList->getLength() : 0;
        for( sal_Int16 i=0; i < nAttrCount; i++ )
        {
            const OUString& rAttrName = xAttrList->getNameByIndex( i );

            OUString aLocalName;
            const sal_uInt16 nPrefix2 =
                GetImport().GetNamespaceMap().GetKeyByAttrName( rAttrName,
                                                                &aLocalName );
            if( XML_NAMESPACE_TABLE == nPrefix2 &&
                 IsXMLToken( aLocalName, XML_IS_SUB_TABLE ) &&
                 IsXMLToken( xAttrList->getValueByIndex( i ), XML_TRUE ) )
            {
                bSubTable = true;
            }
        //FIXME: RDFa
        }
    }

    if( bSubTable )
    {
        if( !HasContent() )
        {
            SwXMLTableContext *pTableContext =
                new SwXMLTableContext( GetSwImport(), nPrefix, rLocalName,
                                       GetTable() );
            pContext = pTableContext;
            if( GetTable()->IsValid() )
                InsertContent( pTableContext );

            GetTable()->SetHasSubTables( true );
        }
    }
    else
    {
        if( GetTable()->IsValid() && !HasContent() )
            InsertContent();
        // fdo#60842: "office:string-value" overrides text content -> no import
        if (!(m_bValueTypeIsString && m_bHasStringValue))
        {
            pContext = GetImport().GetTextImport()->CreateTextChildContext(
                        GetImport(), nPrefix, rLocalName, xAttrList,
                        XMLTextType::Cell  );
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
        if( m_bHasTextContent )
        {
            GetImport().GetTextImport()->DeleteParagraph();
            if( m_nColRepeat > 1 && m_nColSpan == 1 )
            {
                // The original text is invalid after deleting the last
                // paragraph
                Reference < XTextCursor > xSrcTextCursor =
                    GetImport().GetTextImport()->GetText()->createTextCursor();
                xSrcTextCursor->gotoEnd( true );

                // Until we have an API for copying we have to use the core.
                Reference<XUnoTunnel> xSrcCursorTunnel( xSrcTextCursor, UNO_QUERY);
                assert(xSrcCursorTunnel.is() && "missing XUnoTunnel for Cursor");
                OTextCursorHelper *pSrcTextCursor = reinterpret_cast< OTextCursorHelper * >(
                        sal::static_int_cast< sal_IntPtr >( xSrcCursorTunnel->getSomething( OTextCursorHelper::getUnoTunnelId() )));
                assert(pSrcTextCursor && "SwXTextCursor missing");
                SwDoc *pDoc = pSrcTextCursor->GetDoc();
                const SwPaM *pSrcPaM = pSrcTextCursor->GetPaM();

                while( m_nColRepeat > 1 && GetTable()->IsInsertCellPossible() )
                {
                    InsertContent_();

                    Reference<XUnoTunnel> xDstCursorTunnel(
                        GetImport().GetTextImport()->GetCursor(), UNO_QUERY);
                    assert(xDstCursorTunnel.is() && "missing XUnoTunnel for Cursor");
                    OTextCursorHelper *pDstTextCursor = reinterpret_cast< OTextCursorHelper * >(
                            sal::static_int_cast< sal_IntPtr >( xDstCursorTunnel->getSomething( OTextCursorHelper::getUnoTunnelId() )) );
                    assert(pDstTextCursor && "SwXTextCursor missing");
                    SwPaM aSrcPaM(*pSrcPaM->GetMark(), *pSrcPaM->GetPoint());
                    SwPosition aDstPos( *pDstTextCursor->GetPaM()->GetPoint() );
                    pDoc->getIDocumentContentOperations().CopyRange( aSrcPaM, aDstPos, /*bCopyAll=*/false, /*bCheckPos=*/true );

                    m_nColRepeat--;
                }
            }
        }
        else if( !m_bHasTableContent )
        {
            InsertContent();
            if( m_nColRepeat > 1 && m_nColSpan == 1 )
            {
                while( m_nColRepeat > 1 && GetTable()->IsInsertCellPossible() )
                {
                    InsertContent_();
                    m_nColRepeat--;
                }
            }
        }
    }
    GetImport().GetTextImport()->SetCellParaStyleDefault(m_sSaveParaDefault);
}

class SwXMLTableColContext_Impl : public SvXMLImportContext
{
    SvXMLImportContextRef   xMyTable;

    SwXMLTableContext *GetTable() { return static_cast<SwXMLTableContext *>(xMyTable.get()); }

public:

    SwXMLTableColContext_Impl(
            SwXMLImport& rImport, sal_uInt16 nPrfx, const OUString& rLName,
            const Reference< xml::sax::XAttributeList > & xAttrList,
            SwXMLTableContext *pTable );

    SwXMLImport& GetSwImport() { return static_cast<SwXMLImport&>(GetImport()); }
};

SwXMLTableColContext_Impl::SwXMLTableColContext_Impl(
        SwXMLImport& rImport, sal_uInt16 nPrfx, const OUString& rLName,
        const Reference< xml::sax::XAttributeList > & xAttrList,
        SwXMLTableContext *pTable ) :
    SvXMLImportContext( rImport, nPrfx, rLName ),
    xMyTable( pTable )
{
    sal_uInt32 nColRep = 1;
    OUString aStyleName, aDfltCellStyleName;

    sal_Int16 nAttrCount = xAttrList.is() ? xAttrList->getLength() : 0;
    for( sal_Int16 i=0; i < nAttrCount; i++ )
    {
        const OUString& rAttrName = xAttrList->getNameByIndex( i );

        OUString aLocalName;
        const sal_uInt16 nPrefix =
            GetImport().GetNamespaceMap().GetKeyByAttrName( rAttrName,
                                                            &aLocalName );
        const OUString& rValue = xAttrList->getValueByIndex( i );
        if( XML_NAMESPACE_TABLE == nPrefix )
        {
            if( IsXMLToken( aLocalName, XML_STYLE_NAME ) )
                aStyleName = rValue;
            else if( IsXMLToken( aLocalName, XML_NUMBER_COLUMNS_REPEATED ) )
            {
                nColRep = static_cast<sal_uInt32>(std::max<sal_Int32>(1, rValue.toInt32()));
                if (nColRep > 256)
                {
                    SAL_INFO("sw.xml", "ignoring huge table:number-columns-repeated " << nColRep);
                    nColRep = 1;
                }
            }
            else if( IsXMLToken( aLocalName, XML_DEFAULT_CELL_STYLE_NAME ) )
                aDfltCellStyleName = rValue;
        }
        else if ( (XML_NAMESPACE_XML == nPrefix) &&
                 IsXMLToken( aLocalName, XML_ID ) )
        {
        //FIXME where to put this??? columns do not actually exist in writer...
        }
    }

    sal_Int32 nWidth = MINLAY;
    bool bRelWidth = true;
    if( !aStyleName.isEmpty() )
    {
        const SfxPoolItem *pItem;
        const SfxItemSet *pAutoItemSet = nullptr;
        if( GetSwImport().FindAutomaticStyle(
                    XML_STYLE_FAMILY_TABLE_COLUMN,
                                              aStyleName, &pAutoItemSet ) &&
            pAutoItemSet &&
            SfxItemState::SET == pAutoItemSet->GetItemState( RES_FRM_SIZE, false,
                                                        &pItem ) )
        {
            const SwFormatFrameSize *pSize = static_cast<const SwFormatFrameSize *>(pItem);
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

class SwXMLTableColsContext_Impl : public SvXMLImportContext
{
    SvXMLImportContextRef   xMyTable;

    SwXMLTableContext *GetTable() { return static_cast<SwXMLTableContext *>(xMyTable.get()); }

public:

    SwXMLTableColsContext_Impl(
            SwXMLImport& rImport, sal_uInt16 nPrfx,
            const OUString& rLName,
            SwXMLTableContext *pTable );

    virtual SvXMLImportContextRef CreateChildContext(
            sal_uInt16 nPrefix, const OUString& rLocalName,
            const Reference< xml::sax::XAttributeList > & xAttrList ) override;

    SwXMLImport& GetSwImport() { return static_cast<SwXMLImport&>(GetImport()); }
};

SwXMLTableColsContext_Impl::SwXMLTableColsContext_Impl(
        SwXMLImport& rImport, sal_uInt16 nPrfx, const OUString& rLName,
        SwXMLTableContext *pTable ) :
    SvXMLImportContext( rImport, nPrfx, rLName ),
    xMyTable( pTable )
{
}

SvXMLImportContextRef SwXMLTableColsContext_Impl::CreateChildContext(
        sal_uInt16 nPrefix,
        const OUString& rLocalName,
        const Reference< xml::sax::XAttributeList > & xAttrList )
{
    SvXMLImportContext *pContext = nullptr;

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

    SwXMLTableContext *GetTable() { return static_cast<SwXMLTableContext *>(xMyTable.get()); }

public:

    SwXMLTableRowContext_Impl(
            SwXMLImport& rImport, sal_uInt16 nPrfx, const OUString& rLName,
            const Reference< xml::sax::XAttributeList > & xAttrList,
            SwXMLTableContext *pTable, bool bInHead=false );

    virtual SvXMLImportContextRef CreateChildContext( sal_uInt16 nPrefix,
            const OUString& rLocalName,
            const Reference< xml::sax::XAttributeList > & xAttrList ) override;

    virtual void EndElement() override;

    SwXMLImport& GetSwImport() { return static_cast<SwXMLImport&>(GetImport()); }
};

SwXMLTableRowContext_Impl::SwXMLTableRowContext_Impl( SwXMLImport& rImport,
        sal_uInt16 nPrfx,
        const OUString& rLName,
        const Reference< xml::sax::XAttributeList > & xAttrList,
        SwXMLTableContext *pTable,
        bool bInHead ) :
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
        const sal_uInt16 nPrefix =
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
                nRowRepeat = static_cast<sal_uInt32>(std::max<sal_Int32>(1, rValue.toInt32()));
                if (nRowRepeat > 8192 || (nRowRepeat > 256 && utl::ConfigManager::IsFuzzing()))
                {
                    SAL_INFO("sw.xml", "ignoring huge table:number-rows-repeated " << nRowRepeat);
                    nRowRepeat = 1;
                }
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

        if( nRowRepeat > 1 )
            GetTable()->InsertRepRows( nRowRepeat );
    }
}

SvXMLImportContextRef SwXMLTableRowContext_Impl::CreateChildContext(
        sal_uInt16 nPrefix, const OUString& rLocalName,
        const Reference< xml::sax::XAttributeList > & xAttrList )
{
    SvXMLImportContext *pContext = nullptr;

    if( XML_NAMESPACE_TABLE == nPrefix || XML_NAMESPACE_LO_EXT == nPrefix )
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

    bool bHeader;

    SwXMLTableContext *GetTable() { return static_cast<SwXMLTableContext *>(xMyTable.get()); }

public:

    SwXMLTableRowsContext_Impl( SwXMLImport& rImport, sal_uInt16 nPrfx,
            const OUString& rLName,
            SwXMLTableContext *pTable,
            bool bHead );

    virtual SvXMLImportContextRef CreateChildContext( sal_uInt16 nPrefix,
            const OUString& rLocalName,
            const Reference< xml::sax::XAttributeList > & xAttrList ) override;

    SwXMLImport& GetSwImport() { return static_cast<SwXMLImport&>(GetImport()); }
};

SwXMLTableRowsContext_Impl::SwXMLTableRowsContext_Impl( SwXMLImport& rImport,
        sal_uInt16 nPrfx,
        const OUString& rLName,
        SwXMLTableContext *pTable,
        bool bHead ) :
    SvXMLImportContext( rImport, nPrfx, rLName ),
    xMyTable( pTable ),
    bHeader( bHead )
{
}

SvXMLImportContextRef SwXMLTableRowsContext_Impl::CreateChildContext(
        sal_uInt16 nPrefix,
        const OUString& rLocalName,
        const Reference< xml::sax::XAttributeList > & xAttrList )
{
    SvXMLImportContext *pContext = nullptr;

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
    bool bIsAutomaticUpdate;

public:


    SwXMLDDETableContext_Impl(
        SwXMLImport& rImport, sal_uInt16 nPrfx, const OUString& rLName);

    virtual void StartElement(
        const Reference<xml::sax::XAttributeList> & xAttrList) override;

    OUString& GetConnectionName()   { return sConnectionName; }
    OUString& GetDDEApplication()   { return sDDEApplication; }
    OUString& GetDDEItem()          { return sDDEItem; }
    OUString& GetDDETopic()         { return sDDETopic; }
    bool GetIsAutomaticUpdate() { return bIsAutomaticUpdate; }
};


SwXMLDDETableContext_Impl::SwXMLDDETableContext_Impl(
    SwXMLImport& rImport, sal_uInt16 nPrfx, const OUString& rLName) :
        SvXMLImportContext(rImport, nPrfx, rLName),
        sConnectionName(),
        sDDEApplication(),
        sDDEItem(),
        sDDETopic(),
        bIsAutomaticUpdate(false)
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
        const sal_uInt16 nPrefix =
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
            // else: unknown attribute
        }
        // else: unknown attribute namespace
    }
}

// generate a new name for DDE field type (called by lcl_GetDDEFieldType below)
static OUString lcl_GenerateFieldTypeName(const OUString& sPrefix, SwTableNode* pTableNode)
{
    const OUString sPrefixStr(sPrefix.isEmpty() ? OUString("_") : sPrefix);

    // increase count until we find a name that is not yet taken
    OUString sName;
    sal_Int32 nCount = 0;
    do
    {
        // this is crazy, but just in case all names are taken: exit gracefully
        if (nCount == SAL_MAX_INT32)
            return sName;

        ++nCount;
        sName = sPrefixStr + OUString::number(nCount);
    }
    while (nullptr != pTableNode->GetDoc()->getIDocumentFieldsAccess().GetFieldType(SwFieldIds::Dde, sName, false));

    return sName;
}

// set table properties
static SwDDEFieldType* lcl_GetDDEFieldType(SwXMLDDETableContext_Impl* pContext,
                                    SwTableNode* pTableNode)
{
    // make command string
    const OUString sCommand(pContext->GetDDEApplication()
        + OUStringLiteral1(sfx2::cTokenSeparator)
        + pContext->GetDDEItem()
        + OUStringLiteral1(sfx2::cTokenSeparator)
        + pContext->GetDDETopic());

    const SfxLinkUpdateMode nType = pContext->GetIsAutomaticUpdate()
        ? SfxLinkUpdateMode::ALWAYS
        : SfxLinkUpdateMode::ONCALL;

    OUString sName(pContext->GetConnectionName());

    // field type to be returned
    SwDDEFieldType* pType = nullptr;

    // valid name?
    if (sName.isEmpty())
    {
        sName = lcl_GenerateFieldTypeName(pContext->GetDDEApplication(),
                                        pTableNode);
    }
    else
    {
        // check for existing DDE field type with the same name
        SwDDEFieldType* pOldType = static_cast<SwDDEFieldType*>(pTableNode->GetDoc()->getIDocumentFieldsAccess().GetFieldType(SwFieldIds::Dde, sName, false));
        if (nullptr != pOldType)
        {
            // same values -> return old type
            if ( (pOldType->GetCmd() == sCommand) &&
                 (pOldType->GetType() == nType) )
            {
                // same name, same values -> return old type!
                pType = pOldType;
            }
            else
            {
                // same name, different values -> think of new name
                sName = lcl_GenerateFieldTypeName(pContext->GetDDEApplication(),
                                                pTableNode);
            }
        }
        // no old type -> create new one
    }

    // create new field type (unless we already have one)
    if (nullptr == pType)
    {
        // create new field type and return
        SwDDEFieldType aDDEFieldType(sName, sCommand, nType);
        pType = static_cast<SwDDEFieldType*>(pTableNode->
            GetDoc()->getIDocumentFieldsAccess().InsertFieldType(aDDEFieldType));
    }

    OSL_ENSURE(nullptr != pType, "We really want a SwDDEFieldType here!");
    return pType;
}

class TableBoxIndex
{
public:
    OUString msName;
    sal_Int32 mnWidth;
    bool mbProtected;

    TableBoxIndex( const OUString& rName, sal_Int32 nWidth,
                   bool bProtected ) :
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
        return rArg.msName.hashCode() + rArg.mnWidth + (rArg.mbProtected ? 1 : 0);
    }
};

#if __cplusplus <= 201402 || (defined __GNUC__ && __GNUC__ <= 6 && !defined __clang__)
constexpr sal_Int32 SwXMLTableContext::MAX_WIDTH;
#endif

const SwXMLTableCell_Impl *SwXMLTableContext::GetCell( sal_uInt32 nRow,
                                                 sal_uInt32 nCol ) const
{
    return (*m_pRows)[nRow]->GetCell( nCol );
}

SwXMLTableCell_Impl *SwXMLTableContext::GetCell( sal_uInt32 nRow,
                                                 sal_uInt32 nCol )
{
    return (*m_pRows)[nRow]->GetCell( nCol );
}


SwXMLTableContext::SwXMLTableContext( SwXMLImport& rImport,
        sal_uInt16 nPrfx,
        const OUString& rLName,
        const Reference< xml::sax::XAttributeList > & xAttrList ) :
    XMLTextTableContext( rImport, nPrfx, rLName ),
    m_pColumnDefaultCellStyleNames( nullptr ),
    m_pRows( new SwXMLTableRows_Impl ),
    m_pTableNode( nullptr ),
    m_pBox1( nullptr ),
    m_bOwnsBox1( false ),
    m_pSttNd1( nullptr ),
    m_pBoxFormat( nullptr ),
    m_pLineFormat( nullptr ),
    m_pSharedBoxFormats(nullptr),
    m_bFirstSection( true ),
    m_bRelWidth( true ),
    m_bHasSubTables( false ),
    m_nHeaderRows( 0 ),
    m_nCurRow( 0 ),
    m_nCurCol( 0 ),
    m_nWidth( 0 )
{
    OUString aName;
    OUString sXmlId;

    // this method will modify the document directly -> lock SolarMutex
    SolarMutexGuard aGuard;

    sal_Int16 nAttrCount = xAttrList.is() ? xAttrList->getLength() : 0;
    for( sal_Int16 i=0; i < nAttrCount; i++ )
    {
        const OUString& rAttrName = xAttrList->getNameByIndex( i );

        OUString aLocalName;
        const sal_uInt16 nPrefix =
            GetImport().GetNamespaceMap().GetKeyByAttrName( rAttrName,
                                                            &aLocalName );
        const OUString& rValue = xAttrList->getValueByIndex( i );
        if( XML_NAMESPACE_TABLE == nPrefix )
        {
            if( IsXMLToken( aLocalName, XML_STYLE_NAME ) )
                m_aStyleName = rValue;
            else if( IsXMLToken( aLocalName, XML_NAME ) )
                aName = rValue;
            else if( IsXMLToken( aLocalName, XML_DEFAULT_CELL_STYLE_NAME ) )
                m_aDfltCellStyleName = rValue;
            else if( IsXMLToken( aLocalName, XML_TEMPLATE_NAME ) )
                m_aTemplateName = rValue;
        }
        else if ( (XML_NAMESPACE_XML == nPrefix) &&
                 IsXMLToken( aLocalName, XML_ID ) )
        {
            sXmlId = rValue;
        }
    }

    SwDoc *pDoc = SwImport::GetDocFromXMLImport( GetSwImport() );

    OUString sTableName;
    if( !aName.isEmpty() )
    {
        const SwTableFormat *pTableFormat = pDoc->FindTableFormatByName( aName );
        if( !pTableFormat )
            sTableName = aName;
    }
    if( sTableName.isEmpty() )
    {
        sTableName = pDoc->GetUniqueTableName();
        GetImport().GetTextImport()
            ->GetRenameMap().Add( XML_TEXT_RENAME_TYPE_TABLE, aName, sTableName );
    }

    Reference< XTextTable > xTable;
    SwXTextTable *pXTable = nullptr;
    Reference<XMultiServiceFactory> xFactory( GetImport().GetModel(),
                                              UNO_QUERY );
    OSL_ENSURE( xFactory.is(), "factory missing" );
    if( xFactory.is() )
    {
        Reference<XInterface> xIfc = xFactory->createInstance( "com.sun.star.text.TextTable" );
        OSL_ENSURE( xIfc.is(), "Couldn't create a table" );

        if( xIfc.is() )
            xTable.set( xIfc, UNO_QUERY );
    }

    if( xTable.is() )
    {
        xTable->initialize( 1, 1 );

        try
        {
            m_xTextContent = xTable;
            GetImport().GetTextImport()->InsertTextContent( m_xTextContent );
        }
        catch( IllegalArgumentException& )
        {
            xTable = nullptr;
        }
    }

    if( xTable.is() )
    {
        //FIXME
        // xml:id for RDF metadata
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
        m_xOldCursor = GetImport().GetTextImport()->GetCursor();
        GetImport().GetTextImport()->SetCursor( xText->createTextCursor() );

        // take care of open redlines for tables
        GetImport().GetTextImport()->RedlineAdjustStartNodeCursor(true);
    }
    if( pXTable )
    {
        SwFrameFormat *const pTableFrameFormat = pXTable->GetFrameFormat();
        OSL_ENSURE( pTableFrameFormat, "table format missing" );
        SwTable *pTable = SwTable::FindTable( pTableFrameFormat );
        OSL_ENSURE( pTable, "table missing" );
        m_pTableNode = pTable->GetTableNode();
        OSL_ENSURE( m_pTableNode, "table node missing" );

        pTableFrameFormat->SetName( sTableName );

        SwTableLine *pLine1 = m_pTableNode->GetTable().GetTabLines()[0U];
        m_pBox1 = pLine1->GetTabBoxes()[0U];
        m_pSttNd1 = m_pBox1->GetSttNd();
    }
}

SwXMLTableContext::SwXMLTableContext( SwXMLImport& rImport,
        sal_uInt16 nPrfx,
        const OUString& rLName,
        SwXMLTableContext *pTable ) :
    XMLTextTableContext( rImport, nPrfx, rLName ),
    m_pColumnDefaultCellStyleNames( nullptr ),
    m_pRows( new SwXMLTableRows_Impl ),
    m_pTableNode( pTable->m_pTableNode ),
    m_pBox1( nullptr ),
    m_bOwnsBox1( false ),
    m_pSttNd1( nullptr ),
    m_pBoxFormat( nullptr ),
    m_pLineFormat( nullptr ),
    m_pSharedBoxFormats(nullptr),
    m_xParentTable( pTable ),
    m_bFirstSection( false ),
    m_bRelWidth( true ),
    m_bHasSubTables( false ),
    m_nHeaderRows( 0 ),
    m_nCurRow( 0 ),
    m_nCurCol( 0 ),
    m_nWidth( 0 )
{
}

SwXMLTableContext::~SwXMLTableContext()
{
    if (m_bOwnsBox1)
        delete m_pBox1;
    m_pColumnDefaultCellStyleNames.reset();
    m_pSharedBoxFormats.reset();
    m_pRows.reset();

    // close redlines on table end nodes
    GetImport().GetTextImport()->RedlineAdjustStartNodeCursor(false);
}

SvXMLImportContextRef SwXMLTableContext::CreateChildContext( sal_uInt16 nPrefix,
        const OUString& rLocalName,
        const Reference< xml::sax::XAttributeList > & xAttrList )
{
    SvXMLImportContext *pContext = nullptr;

    const SvXMLTokenMap& rTokenMap = GetSwImport().GetTableElemTokenMap();
    bool bHeader = false;
    switch( rTokenMap.Get( nPrefix, rLocalName ) )
    {
    case XML_TOK_TABLE_HEADER_COLS:
    case XML_TOK_TABLE_COLS:
        if( IsValid() )
            pContext = new SwXMLTableColsContext_Impl( GetSwImport(), nPrefix,
                                                       rLocalName,
                                                       this );
        break;
    case XML_TOK_TABLE_COL:
        if( IsValid() && IsInsertColPossible() )
            pContext = new SwXMLTableColContext_Impl( GetSwImport(), nPrefix,
                                                      rLocalName, xAttrList,
                                                      this );
        break;
    case XML_TOK_TABLE_HEADER_ROWS:
        bHeader = true;
        SAL_FALLTHROUGH;
    case XML_TOK_TABLE_ROWS:
        pContext = new SwXMLTableRowsContext_Impl( GetSwImport(), nPrefix,
                                                   rLocalName,
                                                   this, bHeader );
        break;
    case XML_TOK_TABLE_ROW:
        if( IsInsertRowPossible() )
            pContext = new SwXMLTableRowContext_Impl( GetSwImport(), nPrefix,
                                                      rLocalName, xAttrList,
                                                      this );
        break;
    case XML_TOK_OFFICE_DDE_SOURCE:
        // save context for later processing (discard old context, if approp.)
        if( IsValid() )
        {
            m_xDDESource.set(new SwXMLDDETableContext_Impl( GetSwImport(), nPrefix,
                                                        rLocalName ));
            pContext = m_xDDESource.get();
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
    OSL_ENSURE( m_nCurCol < USHRT_MAX,
            "SwXMLTableContext::InsertColumn: no space left" );
    if( m_nCurCol >= USHRT_MAX )
        return;

    if( nWidth2 < MINLAY )
        nWidth2 = MINLAY;
    else if( nWidth2 > MAX_WIDTH )
        nWidth2 = MAX_WIDTH;
    m_aColumnWidths.emplace_back(nWidth2, bRelWidth2 );
    if( (pDfltCellStyleName && !pDfltCellStyleName->isEmpty()) ||
        m_pColumnDefaultCellStyleNames )
    {
        if( !m_pColumnDefaultCellStyleNames )
        {
            m_pColumnDefaultCellStyleNames.reset(new std::vector<OUString>);
            sal_uLong nCount = m_aColumnWidths.size() - 1;
            while( nCount-- )
                m_pColumnDefaultCellStyleNames->push_back(OUString());
        }

        if(pDfltCellStyleName)
            m_pColumnDefaultCellStyleNames->push_back(*pDfltCellStyleName);
        else
            m_pColumnDefaultCellStyleNames->push_back(OUString());
    }
}

sal_Int32 SwXMLTableContext::GetColumnWidth( sal_uInt32 nCol,
                                             sal_uInt32 nColSpan ) const
{
    sal_uInt32 nLast = nCol+nColSpan;
    if( nLast > m_aColumnWidths.size() )
        nLast = m_aColumnWidths.size();

    sal_Int32 nWidth2 = 0;
    for( sal_uInt32 i=nCol; i < nLast; ++i )
        nWidth2 += m_aColumnWidths[i].width;

    return nWidth2;
}

OUString SwXMLTableContext::GetColumnDefaultCellStyleName( sal_uInt32 nCol ) const
{
    if( m_pColumnDefaultCellStyleNames && nCol < m_pColumnDefaultCellStyleNames->size())
        return (*m_pColumnDefaultCellStyleNames)[static_cast<size_t>(nCol)];

    return OUString();
}

void SwXMLTableContext::InsertCell( const OUString& rStyleName,
                                    sal_uInt32 nRowSpan, sal_uInt32 nColSpan,
                                    const SwStartNode *pStartNode,
                                    const OUString & i_rXmlId,
                                    SwXMLTableContext *pTable,
                                    bool bProtect,
                                    const OUString* pFormula,
                                    bool bHasValue,
                                    double fValue,
                                    OUString const*const pStringValue )
{
    OSL_ENSURE( m_nCurCol < GetColumnCount(),
            "SwXMLTableContext::InsertCell: row is full" );
    OSL_ENSURE( m_nCurRow < USHRT_MAX,
            "SwXMLTableContext::InsertCell: table is full" );
    if( m_nCurCol >= USHRT_MAX || m_nCurRow > USHRT_MAX )
        return;

    OSL_ENSURE( nRowSpan >=1, "SwXMLTableContext::InsertCell: row span is 0" );
    if( 0 == nRowSpan )
        nRowSpan = 1;
    OSL_ENSURE( nColSpan >=1, "SwXMLTableContext::InsertCell: col span is 0" );
    if( 0 == nColSpan )
        nColSpan = 1;

    // Until it is possible to add columns here, fix the column span.
    sal_uInt32 nColsReq = m_nCurCol + nColSpan;
    if( nColsReq > GetColumnCount() )
    {
        nColSpan = GetColumnCount() - m_nCurCol;
        nColsReq = GetColumnCount();
    }

    // Check whether there are cells from a previous line already that reach
    // into the current row.
    if( m_nCurRow > 0 && nColSpan > 1 )
    {
        SwXMLTableRow_Impl *pCurRow = (*m_pRows)[m_nCurRow].get();
        sal_uInt32 nLastCol = GetColumnCount() < nColsReq ? GetColumnCount()
                                                     : nColsReq;
        for( sal_uInt32 i=m_nCurCol+1; i<nLastCol; ++i )
        {
            if( pCurRow->GetCell(i)->IsUsed() )
            {
                // If this cell is used, the column span is truncated
                nColSpan = i - m_nCurCol;
                nColsReq = i;
                break;
            }
        }
    }

    sal_uInt32 nRowsReq = m_nCurRow + nRowSpan;
    if( nRowsReq > USHRT_MAX )
    {
        nRowSpan = USHRT_MAX - m_nCurRow;
        nRowsReq = USHRT_MAX;
    }

    // Add columns (if # required columns greater than # columns):
    // This should never happen, since we require column definitions!
    if ( nColsReq > GetColumnCount() )
    {
        for( sal_uInt32 i=GetColumnCount(); i<nColsReq; ++i )
        {
            m_aColumnWidths.emplace_back(MINLAY, true );
        }
        // adjust columns in *all* rows, if columns must be inserted
        for (size_t i = 0; i < m_pRows->size(); ++i)
            (*m_pRows)[i]->Expand( nColsReq, i<m_nCurRow );
    }

    // Add rows
    if (m_pRows->size() < nRowsReq)
    {
        for (size_t i = m_pRows->size(); i < nRowsReq; ++i)
            m_pRows->push_back(o3tl::make_unique<SwXMLTableRow_Impl>(
                        "", GetColumnCount()));
    }

    OUString sStyleName( rStyleName );
    if( sStyleName.isEmpty() )
    {
        sStyleName = (*m_pRows)[m_nCurRow]->GetDefaultCellStyleName();
        if( sStyleName.isEmpty() && m_pColumnDefaultCellStyleNames )
        {
            sStyleName = GetColumnDefaultCellStyleName( m_nCurCol );
            if( sStyleName.isEmpty() )
                sStyleName = m_aDfltCellStyleName;
        }
    }

    // Fill the cells
    for( sal_uInt32 i=nColSpan; i>0; --i )
    {
        for( sal_uInt32 j=nRowSpan; j>0; --j )
        {
            const bool bCovered = i != nColSpan || j != nRowSpan;
            SwXMLTableCell_Impl *pCell = GetCell( nRowsReq-j, nColsReq-i );
            if (!pCell)
                throw css::lang::IndexOutOfBoundsException();
            pCell->Set( sStyleName, j, i, pStartNode,
                       pTable, bProtect, pFormula, bHasValue, bCovered, fValue,
                       pStringValue, i_rXmlId );
        }
    }

    // Set current col to the next (free) column
    m_nCurCol = nColsReq;
    while( m_nCurCol<GetColumnCount() && GetCell(m_nCurRow,m_nCurCol)->IsUsed() )
        m_nCurCol++;
}

void SwXMLTableContext::InsertRow( const OUString& rStyleName,
                                   const OUString& rDfltCellStyleName,
                                   bool bInHead,
                                   const OUString & i_rXmlId )
{
    OSL_ENSURE( m_nCurRow < USHRT_MAX,
            "SwXMLTableContext::InsertRow: no space left" );
    if( m_nCurRow >= USHRT_MAX )
        return;

    // Make sure there is at least one column.
    if( 0==m_nCurRow && 0 == GetColumnCount()  )
        InsertColumn( USHRT_MAX, true );

    if (m_nCurRow < m_pRows->size())
    {
        // The current row has already been inserted because of a row span
        // of a previous row.
        (*m_pRows)[m_nCurRow]->Set(
            rStyleName, rDfltCellStyleName, i_rXmlId );
    }
    else
    {
        // add a new row
        m_pRows->push_back(o3tl::make_unique<SwXMLTableRow_Impl>(
                rStyleName, GetColumnCount(),
                                       &rDfltCellStyleName, i_rXmlId));
    }

    // We start at the first column ...
    m_nCurCol=0;

    // ... but this cell may be occupied already.
    while( m_nCurCol<GetColumnCount() && GetCell(m_nCurRow,m_nCurCol)->IsUsed() )
        m_nCurCol++;

    if( bInHead  &&  m_nHeaderRows == m_nCurRow )
        m_nHeaderRows++;
}

void SwXMLTableContext::InsertRepRows( sal_uInt32 nCount )
{
    const SwXMLTableRow_Impl *pSrcRow = (*m_pRows)[m_nCurRow-1].get();
    while( nCount > 1 && IsInsertRowPossible() )
    {
        InsertRow( pSrcRow->GetStyleName(), pSrcRow->GetDefaultCellStyleName(),
                   false );
        while( m_nCurCol < GetColumnCount() )
        {
            if( !GetCell(m_nCurRow,m_nCurCol)->IsUsed() )
            {
                const SwXMLTableCell_Impl *pSrcCell =
                    GetCell( m_nCurRow-1, m_nCurCol );
                InsertCell( pSrcCell->GetStyleName(), 1U,
                            pSrcCell->GetColSpan(),
                            InsertTableSection(),
                            OUString(),
                            nullptr, pSrcCell->IsProtected(),
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
    // Insert an empty cell at the end of the line if the row is not complete
    if( m_nCurCol < GetColumnCount() )
    {
        InsertCell( "", 1U, GetColumnCount() - m_nCurCol,
                    InsertTableSection() );
    }

    // Move to the next row.
    m_nCurRow++;
}

const SwStartNode *SwXMLTableContext::GetPrevStartNode( sal_uInt32 nRow,
                                                        sal_uInt32 nCol ) const
{
    const SwXMLTableCell_Impl *pPrevCell = nullptr;
    if( GetColumnCount() == nCol )
    {
        // The last cell is the right one here.
        pPrevCell = GetCell( m_pRows->size() - 1U, GetColumnCount() - 1 );
    }
    else if( nCol > 0 )
    {
        // The previous cell in this row.
        pPrevCell = GetCell( nRow, nCol-1 );
    }
    else if( nRow > 0 )
    {
        // The last cell from the previous row.
        pPrevCell = GetCell( nRow-1, GetColumnCount()-1 );
    }

    const SwStartNode *pSttNd = nullptr;
    if( pPrevCell )
    {
        if( pPrevCell->GetStartNode() )
            pSttNd = pPrevCell->GetStartNode();
        // #i95726# - Some fault tolerance
//        else
        else if ( pPrevCell->GetSubTable() )
            pSttNd = pPrevCell->GetSubTable()->GetLastStartNode();

        OSL_ENSURE( pSttNd != nullptr,
                "table corrupt" );
    }

    return pSttNd;
}

void SwXMLTableContext::FixRowSpan( sal_uInt32 nRow, sal_uInt32 nCol,
                                    sal_uInt32 nColSpan )
{
    sal_uInt32 nLastCol = nCol + nColSpan;
    for( sal_uInt32 i = nCol; i < nLastCol; i++ )
    {
        sal_uInt32 j = nRow;
        sal_uInt32 nRowSpan = 1;
        SwXMLTableCell_Impl *pCell = GetCell( j, i );
        while( pCell && pCell->GetRowSpan() > 1 )
        {
            pCell->SetRowSpan( nRowSpan++ );
            pCell = j > 0 ? GetCell( --j, i ) : nullptr;
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
        SwXMLTableRow_Impl *pRow = (*m_pRows)[i].get();
        for( sal_uInt32 j=nCol; j<nLastCol; j++ )
            pRow->GetCell( j )->SetStartNode( pSttNd );
    }

}

SwTableBox *SwXMLTableContext::NewTableBox( const SwStartNode *pStNd,
                                             SwTableLine *pUpper )
{
    // The topmost table is the only table that maintains the two members
    // pBox1 and bFirstSection.
    if( m_xParentTable.is() )
        return static_cast<SwXMLTableContext *>(m_xParentTable.get())->NewTableBox( pStNd,
                                                                  pUpper );

    SwTableBox *pBox;

    if( m_pBox1 &&
        m_pBox1->GetSttNd() == pStNd )
    {
        // if the StartNode is equal to the StartNode of the initially
        // created box, we use this box
        pBox = m_pBox1;
        pBox->SetUpper( pUpper );
        m_pBox1 = nullptr;
        m_bOwnsBox1 = false;
    }
    else
        pBox = new SwTableBox( m_pBoxFormat, *pStNd, pUpper );

    return pBox;
}

SwTableBoxFormat* SwXMLTableContext::GetSharedBoxFormat(
    SwTableBox* pBox,
    const OUString& rStyleName,
    sal_Int32 nColumnWidth,
    bool bProtected,
    bool bMayShare,
    bool& bNew,
    bool* pModifyLocked )
{
    if ( m_pSharedBoxFormats == nullptr )
        m_pSharedBoxFormats.reset(new map_BoxFormat);

    SwTableBoxFormat* pBoxFormat2;

    TableBoxIndex aKey( rStyleName, nColumnWidth, bProtected );
    map_BoxFormat::iterator aIter = m_pSharedBoxFormats->find( aKey );
    if ( aIter == m_pSharedBoxFormats->end() )
    {
        // unknown format so far -> construct a new one

        // get the old format, and reset all attributes
        // (but preserve FillOrder)
        pBoxFormat2 = static_cast<SwTableBoxFormat*>(pBox->ClaimFrameFormat());
        SwFormatFillOrder aFillOrder( pBoxFormat2->GetFillOrder() );
        pBoxFormat2->ResetAllFormatAttr(); // #i73790# - method renamed
        pBoxFormat2->SetFormatAttr( aFillOrder );
        bNew = true;    // it's a new format now

        // share this format, if allowed
        if ( bMayShare )
            (*m_pSharedBoxFormats)[ aKey ] = pBoxFormat2;
    }
    else
    {
        // set the shared format
        pBoxFormat2 = aIter->second;
        pBox->ChgFrameFormat( pBoxFormat2 );
        bNew = false;   // copied from an existing format

        // claim it, if we are not allowed to share
        if ( !bMayShare )
            pBoxFormat2 = static_cast<SwTableBoxFormat*>(pBox->ClaimFrameFormat());
    }

    // lock format (if so desired)
    if ( pModifyLocked != nullptr )
    {
        (*pModifyLocked) = pBoxFormat2->IsModifyLocked();
        pBoxFormat2->LockModify();
    }

    return pBoxFormat2;
}

SwTableBox *SwXMLTableContext::MakeTableBox( SwTableLine *pUpper,
                                             sal_uInt32 nTopRow,
                                             sal_uInt32 nLeftCol,
                                             sal_uInt32 nBottomRow,
                                             sal_uInt32 nRightCol )
{
    //FIXME: here would be a great place to handle XmlId for cell
    SwTableBox *pBox = new SwTableBox( m_pBoxFormat, 0, pUpper );

    sal_uInt32 nColSpan = nRightCol - nLeftCol;
    sal_Int32 nColWidth = GetColumnWidth( nLeftCol, nColSpan );

    // TODO: Share formats!
    SwFrameFormat *pFrameFormat = pBox->ClaimFrameFormat();
    SwFormatFillOrder aFillOrder( pFrameFormat->GetFillOrder() );
    pFrameFormat->ResetAllFormatAttr(); // #i73790# - method renamed
    pFrameFormat->SetFormatAttr( aFillOrder );

    pFrameFormat->SetFormatAttr( SwFormatFrameSize( ATT_VAR_SIZE, nColWidth ) );

    SwTableLines& rLines = pBox->GetTabLines();
    bool bSplitted = false;

    while( !bSplitted )
    {
        sal_uInt32 nStartRow = nTopRow;
        sal_uInt32 i;

        for( i = nTopRow; i < nBottomRow; i++ )
        {
            // Could the table be splitted behind the current row?
            bool bSplit = true;
            SwXMLTableRow_Impl *pRow = (*m_pRows)[i].get();
            for( sal_uInt32 j=nLeftCol; j<nRightCol; j++ )
            {
                bSplit = ( 1 == pRow->GetCell(j)->GetRowSpan() );
                if( !bSplit )
                    break;
            }
            if( bSplit && (nStartRow>nTopRow || i+1<nBottomRow) )
            {
                SwTableLine *pLine =
                    MakeTableLine( pBox, nStartRow, nLeftCol, i+1,
                                   nRightCol );

                rLines.push_back( pLine );

                nStartRow = i+1;
                bSplitted = true;
            }
        }
        if( !bSplitted )
        {
            // No splitting was possible. That for, we have to force it.
            // Ruthless!

            nStartRow = nTopRow;
            while( nStartRow < nBottomRow )
            {
                sal_uInt32 nMaxRowSpan = 0;
                SwXMLTableRow_Impl *pStartRow = (*m_pRows)[nStartRow].get();
                const SwXMLTableCell_Impl *pCell;
                for( i=nLeftCol; i<nRightCol; i++ )
                {
                    pCell = pStartRow->GetCell(i);
                    if( pCell->GetRowSpan() > nMaxRowSpan )
                        nMaxRowSpan = pCell->GetRowSpan();
                }

                nStartRow += nMaxRowSpan;
                if( nStartRow<nBottomRow )
                {
                    SwXMLTableRow_Impl *pPrevRow = (*m_pRows)[nStartRow - 1U].get();
                    i = nLeftCol;
                    while( i < nRightCol )
                    {
                        if( pPrevRow->GetCell(i)->GetRowSpan() > 1 )
                        {
                            const SwXMLTableCell_Impl *pCell2 =
                                GetCell( nStartRow, i );
                            const sal_uInt32 nColSpan2 = pCell2->GetColSpan();
                            FixRowSpan( nStartRow-1, i, nColSpan2 );
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
            // and now start over again...
        }
    }

    return pBox;
}

SwTableBox *SwXMLTableContext::MakeTableBox(
        SwTableLine *pUpper, const SwXMLTableCell_Impl *pCell,
        sal_uInt32 nLeftCol, sal_uInt32 nRightCol )
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
        // and it is a table: therefore we build a new box and
        // put the rows of the table into the rows of the box
        pBox = new SwTableBox( m_pBoxFormat, 0, pUpper );
        pCell->GetSubTable()->MakeTable( pBox, nColWidth );
    }

    // Share formats!
    const OUString sStyleName = pCell->GetStyleName();
    bool bModifyLocked;
    bool bNew;
    SwTableBoxFormat *pBoxFormat2 = GetSharedBoxFormat(
        pBox, sStyleName, nColWidth, pCell->IsProtected(),
        pCell->GetStartNode() && pCell->GetFormula().isEmpty() &&
            ! pCell->HasValue(),
        bNew, &bModifyLocked  );

    // if a new format was created, then we need to set the style
    if ( bNew )
    {
        // set style
        const SfxItemSet *pAutoItemSet = nullptr;
        if( pCell->GetStartNode() && !sStyleName.isEmpty() &&
            GetSwImport().FindAutomaticStyle(
                XML_STYLE_FAMILY_TABLE_CELL, sStyleName, &pAutoItemSet ) )
        {
            if( pAutoItemSet )
                pBoxFormat2->SetFormatAttr( *pAutoItemSet );
        }
    }

    if( pCell->GetStartNode() )
    {
        if (pCell->HasStringValue())
        {
            SwNodeIndex const aNodeIndex(*(pCell->GetStartNode()), 1);
            SwTextNode *const pTextNode(aNodeIndex.GetNode().GetTextNode());
            SAL_WARN_IF(!pTextNode, "sw", "Should have a text node in cell?");
            if (pTextNode)
            {
                SAL_WARN_IF(!pTextNode->GetText().isEmpty(), "sw",
                        "why text here?");
                pTextNode->InsertText(*pCell->GetStringValue(),
                        SwIndex(pTextNode, 0));
            }
        }

        // try to rescue broken documents with a certain pattern
        // if: 1) the cell has a default number format (number 0)
        //     2) the call has no formula
        //     3) the value is 0.0
        //     4) the text doesn't look anything like 0.0
        //        [read: length > 10, or length smaller 10 and no 0 in it]
        // then make it a text cell!
        bool bSuppressNumericContent = false;
        if( pCell->HasValue() && (pCell->GetValue() == 0.0) &&
            pCell->GetFormula().isEmpty() &&
            !sStyleName.isEmpty() )
        {
            // default num format?
            const SfxPoolItem* pItem = nullptr;
            if( pBoxFormat2->GetItemState( RES_BOXATR_FORMAT, false, &pItem )
                            == SfxItemState::SET )
            {
                const SwTableBoxNumFormat* pNumFormat =
                    static_cast<const SwTableBoxNumFormat*>( pItem );
                if( ( pNumFormat != nullptr ) && ( pNumFormat->GetValue() == 0 ) )
                {
                    // only one text node?
                    SwNodeIndex aNodeIndex( *(pCell->GetStartNode()), 1 );
                    if( ( aNodeIndex.GetNode().EndOfSectionIndex() -
                          aNodeIndex.GetNode().StartOfSectionIndex() ) == 2 )
                    {
                        SwTextNode* pTextNode= aNodeIndex.GetNode().GetTextNode();
                        if( pTextNode != nullptr )
                        {
                            // check text: does it look like some form of 0.0?
                            const OUString& rText = pTextNode->GetText();
                            if( ( rText.getLength() > 10 ) ||
                                ( rText.indexOf( '0' ) == -1 ) )
                            {
                                bSuppressNumericContent = true;
                            }
                        }
                    }
                    else
                        bSuppressNumericContent = true; // several nodes
                }
            }
        }

        if( bSuppressNumericContent )
        {
            // suppress numeric content? Then reset number format!
            pBoxFormat2->ResetFormatAttr( RES_BOXATR_FORMULA );
            pBoxFormat2->ResetFormatAttr( RES_BOXATR_FORMAT );
            pBoxFormat2->ResetFormatAttr( RES_BOXATR_VALUE );
        }
        else
        {
            // the normal case: set formula and value (if available)

            const OUString& rFormula = pCell->GetFormula();
            if (!rFormula.isEmpty())
            {
                // formula cell: insert formula if valid
                SwTableBoxFormula aFormulaItem( rFormula );
                pBoxFormat2->SetFormatAttr( aFormulaItem );
            }
            else if (!pCell->HasValue() && pCell->HasStringValue())
            {
                // Check for another inconsistency:
                // No value but a non-textual format, i.e. a number format
                // Solution: the number format will be removed,
                // the cell gets the default text format.
                const SfxPoolItem* pItem = nullptr;
                if( m_pBoxFormat->GetItemState( RES_BOXATR_FORMAT, false, &pItem )
                    == SfxItemState::SET )
                {
                    const SwDoc* pDoc = m_pBoxFormat->GetDoc();
                    const SvNumberFormatter* pNumberFormatter = pDoc ?
                        pDoc->GetNumberFormatter() : nullptr;
                    const SwTableBoxNumFormat* pNumFormat =
                        static_cast<const SwTableBoxNumFormat*>( pItem );
                    if( pNumFormat != nullptr && pNumberFormatter &&
                        !pNumberFormatter->GetEntry( pNumFormat->GetValue() )->IsTextFormat() )
                        m_pBoxFormat->ResetFormatAttr( RES_BOXATR_FORMAT );
                }
            }
            // always insert value, even if default
            if( pCell->HasValue() )
            {
                SwTableBoxValue aValueItem( pCell->GetValue() );
                pBoxFormat2->SetFormatAttr( aValueItem );
            }
        }

        // update cell content depend on the default language
        pBox->ActualiseValueBox();
    }

    // table cell protection
    if( pCell->IsProtected() )
    {
        SvxProtectItem aProtectItem( RES_PROTECT );
        aProtectItem.SetContentProtect( true );
        pBoxFormat2->SetFormatAttr( aProtectItem );
    }

    // restore old modify-lock state
    if (! bModifyLocked)
        pBoxFormat2->UnlockModify();

    pBoxFormat2->SetFormatAttr( SwFormatFrameSize( ATT_VAR_SIZE, nColWidth ) );

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
        pLine = m_pTableNode->GetTable().GetTabLines()[0U];
    }
    else
    {
        pLine = new SwTableLine( m_pLineFormat, 0, pUpper );
    }

    // TODO: Share formats!
    SwFrameFormat *pFrameFormat = pLine->ClaimFrameFormat();
    SwFormatFillOrder aFillOrder( pFrameFormat->GetFillOrder() );
    pFrameFormat->ResetAllFormatAttr(); // #i73790# - method renamed
    pFrameFormat->SetFormatAttr( aFillOrder );

    const SfxItemSet *pAutoItemSet = nullptr;
    const OUString& rStyleName = (*m_pRows)[nTopRow]->GetStyleName();
    if( 1 == (nBottomRow - nTopRow) &&
        !rStyleName.isEmpty() &&
        GetSwImport().FindAutomaticStyle(
            XML_STYLE_FAMILY_TABLE_ROW, rStyleName, &pAutoItemSet ) )
    {
        if( pAutoItemSet )
            pFrameFormat->SetFormatAttr( *pAutoItemSet );
    }

    SwTableBoxes& rBoxes = pLine->GetTabBoxes();

    sal_uInt32 nStartCol = nLeftCol;
    while( nStartCol < nRightCol )
    {
        for( sal_uInt32 nRow=nTopRow; nRow<nBottomRow; nRow++ )
            (*m_pRows)[nRow]->SetSplitable( true );

        sal_uInt32 nCol = nStartCol;
        sal_uInt32 nSplitCol = nRightCol;
        bool bSplitted = false;
        while( !bSplitted )
        {
            OSL_ENSURE( nCol < nRightCol, "Ran too far" );

            // Can be split after current HTML table column?
            // If yes, can the created region still be split to
            // rows if the next column is added to it?
            bool bSplit = true;
            bool bHoriSplitMayContinue = false;
            bool bHoriSplitPossible = false;

            if ( m_bHasSubTables )
            {
                // Convert row spans if the table has subtables:
                for( sal_uInt32 nRow=nTopRow; nRow<nBottomRow; nRow++ )
                {
                    SwXMLTableCell_Impl *pCell = GetCell(nRow,nCol);
                    // Could the table fragment be splitted horizontally behind
                    // the current line?
                    bool bHoriSplit = (*m_pRows)[nRow]->IsSplitable() &&
                                      nRow+1 < nBottomRow &&
                                      1 == pCell->GetRowSpan();
                    (*m_pRows)[nRow]->SetSplitable( bHoriSplit );

                    // Could the table fragment be splitted vertically behind the
                    // current column (uptp the current line?
                    bSplit &= ( 1 == pCell->GetColSpan() );
                    if( bSplit )
                    {
                        bHoriSplitPossible |= bHoriSplit;

                        // Could the current table fragment be splitted
                        // horizontally behind the next column, too?
                        bHoriSplit &= (nCol+1 < nRightCol &&
                                       1 == GetCell(nRow,nCol+1)->GetRowSpan());
                        bHoriSplitMayContinue |= bHoriSplit;
                    }
                }
            }
            else
            {
                // No subtables: we use the new table model.
                SwXMLTableCell_Impl *pCell = GetCell(nTopRow,nCol);

                // #i95726# - some fault tolerance
                if ( pCell == nullptr )
                {
                    OSL_FAIL( "table seems to be corrupt." );
                    break;
                }

                // Could the table fragment be splitted vertically behind the
                // current column (uptp the current line?
                bSplit = 1 == pCell->GetColSpan();
            }

#if OSL_DEBUG_LEVEL > 0
            if( nCol == nRightCol-1 )
            {
                OSL_ENSURE( bSplit, "Split-Flag wrong" );
                if ( m_bHasSubTables )
                {
                    OSL_ENSURE( !bHoriSplitMayContinue,
                            "HoriSplitMayContinue-Flag wrong" );
                    SwXMLTableCell_Impl *pTmpCell = GetCell( nTopRow, nStartCol );
                    OSL_ENSURE( pTmpCell->GetRowSpan() != (nBottomRow-nTopRow) ||
                            !bHoriSplitPossible, "HoriSplitPossible-Flag wrong" );
                }
            }
#endif

            OSL_ENSURE( !m_bHasSubTables || !bHoriSplitMayContinue || bHoriSplitPossible,
                    "bHoriSplitMayContinue, but not bHoriSplitPossible" );

            if( bSplit )
            {
                SwTableBox* pBox = nullptr;
                SwXMLTableCell_Impl *pCell = GetCell( nTopRow, nStartCol );
                // #i95726# - some fault tolerance
                if( ( !m_bHasSubTables || ( pCell->GetRowSpan() == (nBottomRow-nTopRow) ) ) &&
                    pCell->GetColSpan() == (nCol+1-nStartCol) &&
                    ( pCell->GetStartNode() || pCell->GetSubTable() ) )
                {
                    // insert new empty cell for covered cells:
                    long nBoxRowSpan = 1;
                    if ( !m_bHasSubTables )
                    {
                        nBoxRowSpan = pCell->GetRowSpan();
                        if ( pCell->IsCovered() )
                        {
                            nBoxRowSpan = -1 * nBoxRowSpan;
                            ReplaceWithEmptyCell( nTopRow, nStartCol, false );
                        }
                    }

                    // The remaining box neither contains lines nor rows (i.e.
                    // is a content box
                    nSplitCol = nCol + 1;

                    pBox = MakeTableBox( pLine, pCell, nStartCol, nSplitCol );

                    if ( 1 != nBoxRowSpan )
                        pBox->setRowSpan( nBoxRowSpan );

                    bSplitted = true;
                }
                else if( m_bHasSubTables && bHoriSplitPossible && bHoriSplitMayContinue )
                {
                    // The table fragment could be splitted behind the current
                    // column, and the remaining fragment could be divided
                    // into lines. Anyway, it could be that this applies to
                    // the next column, too. That for, we check the next
                    // column but remember the current one as a good place to
                    // split.
                    nSplitCol = nCol + 1;
                }
                else if ( m_bHasSubTables )
                {
                    // If the table resulting table fragment could be divided
                    // into lines if splitting behind the current column, but
                    // this doesn't apply for thr next column, we split begind
                    // the current column. This applies for the last column,
                    // too.
                    // If the resulting box cannot be splitted into rows,
                    // the split at the last split position we remembered.
                    if( bHoriSplitPossible || nSplitCol > nCol+1 )
                    {
                        OSL_ENSURE( !bHoriSplitMayContinue,
                                "bHoriSplitMayContinue==true" );
                        OSL_ENSURE( bHoriSplitPossible || nSplitCol == nRightCol,
                                "bHoriSplitPossible flag should be set" );

                        nSplitCol = nCol + 1;
                    }

                    pBox = MakeTableBox( pLine, nTopRow, nStartCol,
                                         nBottomRow, nSplitCol );
                    bSplitted = true;
                }

                OSL_ENSURE( m_bHasSubTables || pBox, "Colspan trouble" );

                if( pBox )
                    rBoxes.push_back( pBox );
            }
            nCol++;
        }
        nStartCol = nSplitCol;
    }

    return pLine;
}

void SwXMLTableContext::MakeTable_( SwTableBox *pBox )
{
    // fix column widths
    std::vector<ColumnWidthInfo>::iterator colIter;
    sal_uInt32 nCols = GetColumnCount();

    // If there are empty rows (because of some row span of previous rows)
    // the have to be deleted. The previous rows have to be truncated.

    if (m_pRows->size() > m_nCurRow)
    {
        SwXMLTableRow_Impl *pPrevRow = (*m_pRows)[m_nCurRow - 1U].get();
        const SwXMLTableCell_Impl *pCell;
        for( sal_uLong i = 0; i < m_aColumnWidths.size(); ++i )
        {
            pCell = pPrevRow->GetCell(i);
            if( pCell->GetRowSpan() > 1 )
            {
                FixRowSpan( m_nCurRow-1, i, 1UL );
            }
        }
        for (sal_uLong i = m_pRows->size() - 1; i >= m_nCurRow; --i)
            m_pRows->pop_back();
    }

    if (m_pRows->empty())
    {
        InsertCell( "", 1U, nCols, InsertTableSection() );
    }

    // TODO: Do we have to keep both values, the relative and the absolute
    // width?
    sal_Int32 nAbsWidth = 0;
    sal_Int32 nMinAbsColWidth = 0;
    sal_Int32 nRelWidth = 0;
    sal_Int32 nMinRelColWidth = 0;
    sal_uInt32 nRelCols = 0;
    for( colIter = m_aColumnWidths.begin(); colIter < m_aColumnWidths.end(); ++colIter)
    {
        if( colIter->isRelative )
        {
            nRelWidth += colIter->width;
            if( 0 == nMinRelColWidth || colIter->width < nMinRelColWidth )
                nMinRelColWidth = colIter->width;
            nRelCols++;
        }
        else
        {
            nAbsWidth += colIter->width;
            if( 0 == nMinAbsColWidth || colIter->width < nMinAbsColWidth )
                nMinAbsColWidth = colIter->width;
        }
    }
    sal_uInt32 nAbsCols = nCols - nRelCols;

    if( m_bRelWidth )
    {
        // If there a columns that have an absolute width, we have to
        // calculate a relative one for them.
        if( nAbsCols > 0 )
        {
            // All column that have absolute widths get relative widths;
            // these widths relate to each over like the original absolute
            // widths. The smallest column gets a width that has the same
            // value as the smallest column that has an relative width
            // already.
            if( 0 == nMinRelColWidth )
                nMinRelColWidth = nMinAbsColWidth;

            for( colIter = m_aColumnWidths.begin(); nAbsCols > 0 && colIter < m_aColumnWidths.end(); ++colIter)
            {
                if( !colIter->isRelative )
                {
                    if (nMinAbsColWidth == 0)
                        throw o3tl::divide_by_zero();
                    sal_Int32 nVal;
                    if (o3tl::checked_multiply<sal_Int32>(colIter->width, nMinRelColWidth, nVal))
                        throw std::overflow_error("overflow in multiply");
                    sal_Int32 nRelCol = nVal / nMinAbsColWidth;
                    colIter->width = nRelCol;
                    colIter->isRelative = true;
                    nRelWidth += nRelCol;
                    nAbsCols--;
                }
            }
        }

        if( !m_nWidth )
        {
            // This happens only for percentage values for the table itself.
            // In this case, the columns get the correct width even if
            // the sum of the relative widths is smaller than the available
            // width in TWIP. Therefore, we can use the relative width.
            m_nWidth = std::min(nRelWidth, MAX_WIDTH);
        }
        if( nRelWidth != m_nWidth && nRelWidth && nCols )
        {
            double n = static_cast<double>(m_nWidth) / static_cast<double>(nRelWidth);
            nRelWidth = 0;
            for( colIter = m_aColumnWidths.begin(); colIter < m_aColumnWidths.end() - 1; ++colIter)
            {
                sal_Int32 nW = static_cast<sal_Int32>( colIter->width * n);
                colIter->width = static_cast<sal_uInt16>(nW);
                nRelWidth += nW;
            }
            m_aColumnWidths.back().width = (m_nWidth-nRelWidth);
        }
    }
    else
    {
        // If there are columns that have relative widths, we have to
        // calculate a absolute widths for them.
        if( nRelCols > 0 )
        {
            // The absolute space that is available for all columns with a
            // relative width.
            sal_Int32 nAbsForRelWidth =
                    m_nWidth > nAbsWidth ? m_nWidth - nAbsWidth : sal_Int32(0L);

            // The relative width that has to be distributed in addition to
            // equally widthed columns.
            sal_Int32 nExtraRel = nRelWidth - (nRelCols * nMinRelColWidth);

            // The absolute space that may be distributed in addition to
            // minimum widthed columns.
            sal_Int32 nMinAbs = nRelCols * MINLAY;
            sal_Int32 nExtraAbs =
                    nAbsForRelWidth > nMinAbs ? nAbsForRelWidth - nMinAbs : sal_Int32(0L);

            bool bMin = false;      // Do all columns get the minimum width?
            bool bMinExtra = false; // Do all columns get the minimum width plus
                                    // some extra space?

            if( nAbsForRelWidth <= nMinAbs )
            {
                // If there is not enough space left for all columns to
                // get the minimum width, they get the minimum width, anyway.
                nAbsForRelWidth = nMinAbs;
                bMin = true;
            }
            else if( nAbsForRelWidth <= (nRelWidth * MINLAY) /
                                        nMinRelColWidth )
            {
                // If there is enough space for all columns to get the
                // minimum width, but not to get a width that takes the
                // relative width into account, each column gets the minimum
                // width plus some extra space that is based on the additional
                // space that is available.
                bMinExtra = true;
            }
            // Otherwise, if there is enough space for every column, every
            // column gets this space.

            for( colIter = m_aColumnWidths.begin(); nRelCols > 0 && colIter < m_aColumnWidths.end(); ++colIter )
            {
                if( colIter->isRelative )
                {
                    sal_Int32 nAbsCol;
                    if( 1 == nRelCols )
                    {
                        // The last column that has a relative width gets
                        // all absolute space that is left.
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
            if( nAbsWidth < m_nWidth )
            {
                // If the table's width is larger than the sum of the absolute
                // column widths, every column get some extra width.
                sal_Int32 nExtraAbs = m_nWidth - nAbsWidth;
                sal_Int32 nAbsLastCol = m_aColumnWidths.back().width + nExtraAbs;
                for( colIter = m_aColumnWidths.begin(); colIter < m_aColumnWidths.end()-1; ++colIter )
                {
                    sal_Int32 nAbsCol = colIter->width;
                    sal_Int32 nExtraAbsCol = (nAbsCol * nExtraAbs) /
                                             nAbsWidth;
                    nAbsCol += nExtraAbsCol;
                    colIter->width = nAbsCol;
                    nAbsLastCol -= nExtraAbsCol;
                }
                m_aColumnWidths.back().width = nAbsLastCol;
            }
            else if( nAbsWidth > m_nWidth )
            {
                // If the table's width is smaller than the sum of the absolute
                // column widths, every column needs to shrink.
                // Every column gets the minimum width plus some extra width.
                sal_Int32 nExtraAbs = m_nWidth - (nCols * MINLAY);
                sal_Int32 nAbsLastCol = MINLAY + nExtraAbs;
                for( colIter = m_aColumnWidths.begin(); colIter < m_aColumnWidths.end()-1; ++colIter )
                {
                    sal_Int32 nAbsCol = colIter->width;
                    sal_Int32 nExtraAbsCol = (nAbsCol * nExtraAbs) /
                                             nAbsWidth;
                    nAbsCol = MINLAY + nExtraAbsCol;
                    colIter->width = nAbsCol;
                    nAbsLastCol -= nExtraAbsCol;
                }
                m_aColumnWidths.back().width = nAbsLastCol;
            }
        }
    }

    SwTableLines& rLines =
        pBox ? pBox->GetTabLines()
             : m_pTableNode->GetTable().GetTabLines();

    sal_uInt32 nStartRow = 0;
    sal_uInt32 nRows = m_pRows->size();
    for(sal_uInt32 i=0; i<nRows; ++i )
    {
        // Could we split the table behind the current line?
        bool bSplit = true;
        if ( m_bHasSubTables )
        {
            SwXMLTableRow_Impl *pRow = (*m_pRows)[i].get();
            for( sal_uInt32 j=0; j<nCols; j++ )
            {
                bSplit = ( 1 == pRow->GetCell(j)->GetRowSpan() );
                if( !bSplit )
                    break;
            }
        }

        if( bSplit )
        {
            SwTableLine *pLine =
                MakeTableLine( pBox, nStartRow, 0UL, i+1, nCols );
            if( pBox || nStartRow>0 )
                rLines.push_back( pLine );
            nStartRow = i+1;
        }
    }
}

void SwXMLTableContext::MakeTable()
{
    // this method will modify the document directly -> lock SolarMutex
    // This will call all other MakeTable*(..) methods, so
    // those don't need to be locked separately.
    SolarMutexGuard aGuard;

    // #i97274# handle invalid tables
    if (!m_pRows || m_pRows->empty() || !GetColumnCount())
    {
        OSL_FAIL("invalid table: no cells; deleting...");
        m_pTableNode->GetDoc()->getIDocumentContentOperations().DeleteSection( m_pTableNode );
        m_pTableNode = nullptr;
        m_pBox1 = nullptr;
        m_bOwnsBox1 = false;
        m_pSttNd1 = nullptr;
        return;
    }

    SwXMLImport& rSwImport = GetSwImport();

    SwFrameFormat *pFrameFormat = m_pTableNode->GetTable().GetFrameFormat();

    sal_Int16 eHoriOrient = text::HoriOrientation::FULL;
    bool bSetHoriOrient = false;

    sal_uInt8 nPrcWidth = 0U;

    m_pTableNode->GetTable().SetRowsToRepeat( m_nHeaderRows );
    m_pTableNode->GetTable().SetTableModel( !m_bHasSubTables );
    m_pTableNode->GetTable().SetTableStyleName( m_aTemplateName );

    const SfxItemSet *pAutoItemSet = nullptr;
    if( !m_aStyleName.isEmpty() &&
        rSwImport.FindAutomaticStyle(
            XML_STYLE_FAMILY_TABLE_TABLE, m_aStyleName, &pAutoItemSet ) &&
         pAutoItemSet )
    {
        const SfxPoolItem *pItem;
        const SvxLRSpaceItem *pLRSpace = nullptr;
        if( SfxItemState::SET == pAutoItemSet->GetItemState( RES_LR_SPACE, false,
                                                        &pItem ) )
            pLRSpace = static_cast<const SvxLRSpaceItem *>(pItem);

        if( SfxItemState::SET == pAutoItemSet->GetItemState( RES_HORI_ORIENT, false,
                                                        &pItem ) )
        {
            eHoriOrient = static_cast<const SwFormatHoriOrient *>(pItem)->GetHoriOrient();
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

        const SwFormatFrameSize *pSize = nullptr;
        if( SfxItemState::SET == pAutoItemSet->GetItemState( RES_FRM_SIZE, false,
                                                        &pItem ) )
            pSize = static_cast<const SwFormatFrameSize *>(pItem);

        switch( eHoriOrient )
        {
        case text::HoriOrientation::FULL:
        case text::HoriOrientation::NONE:
            // For text::HoriOrientation::NONE we would prefer to use the sum
            // of the relative column widths as reference width.
            // Unfortunately this works only if this sum interpreted as
            // twip value is larger than the space that is available.
            // We don't know that space, so we have to use MAX_WIDTH, too.
            // Even if a size is specified, it will be ignored!
            m_nWidth = MAX_WIDTH;
            break;
        default:
            if( pSize )
            {
                if( pSize->GetWidthPercent() )
                {
                    // The width will be set in MakeTable_
                    nPrcWidth = pSize->GetWidthPercent();
                }
                else
                {
                    m_nWidth = pSize->GetWidth();
                    sal_Int32 const min = static_cast<sal_Int32>(
                        std::min<sal_uInt32>(GetColumnCount() * MINLAY, MAX_WIDTH));
                    if( m_nWidth < min )
                    {
                        m_nWidth = min;
                    }
                    else if( m_nWidth > MAX_WIDTH )
                    {
                        m_nWidth = MAX_WIDTH;
                    }
                    m_bRelWidth = false;
                }
            }
            else
            {
                eHoriOrient = text::HoriOrientation::LEFT_AND_WIDTH == eHoriOrient
                                    ? text::HoriOrientation::NONE : text::HoriOrientation::FULL;
                bSetHoriOrient = true;
                m_nWidth = MAX_WIDTH;
            }
            break;
        }

        pFrameFormat->SetFormatAttr( *pAutoItemSet );
    }
    else
    {
        bSetHoriOrient = true;
        m_nWidth = MAX_WIDTH;
    }

    SwTableLine *pLine1 = m_pTableNode->GetTable().GetTabLines()[0U];
    assert(m_pBox1 == pLine1->GetTabBoxes()[0] && !m_bOwnsBox1 && "Why is box 1 change?");
    m_pBox1->m_pStartNode = m_pSttNd1;
    pLine1->GetTabBoxes().erase( pLine1->GetTabBoxes().begin() );
    m_bOwnsBox1 = true;

    m_pLineFormat = static_cast<SwTableLineFormat*>(pLine1->GetFrameFormat());
    m_pBoxFormat = static_cast<SwTableBoxFormat*>(m_pBox1->GetFrameFormat());

    MakeTable_();

    if( bSetHoriOrient )
        pFrameFormat->SetFormatAttr( SwFormatHoriOrient( 0, eHoriOrient ) );

    // This must be after the call to MakeTable_, because nWidth might be
    // changed there.
    pFrameFormat->LockModify();
    SwFormatFrameSize aSize( ATT_VAR_SIZE, m_nWidth );
    aSize.SetWidthPercent( nPrcWidth );
    pFrameFormat->SetFormatAttr( aSize );
    pFrameFormat->UnlockModify();

    for (std::unique_ptr<SwXMLTableRow_Impl> & rRow : *m_pRows)
        rRow->Dispose();

    // now that table is complete, change into DDE table (if appropriate)
    if (m_xDDESource.is())
    {
        // change existing table into DDE table:
        // 1) Get DDE field type (get data from dde-source context),
        SwDDEFieldType* pFieldType = lcl_GetDDEFieldType( m_xDDESource.get(),
                                                        m_pTableNode );

        // 2) release the DDE source context,
        m_xDDESource.set(nullptr);

        // 3) create new DDE table, and
        std::unique_ptr<SwDDETable> pDDETable( new SwDDETable( m_pTableNode->GetTable(),
                                                pFieldType, false ) );

        // 4) set new (DDE)table at node.
        m_pTableNode->SetNewTable(std::move(pDDETable), false);
    }

    // ??? this is always false: root frame is only created in SwViewShell::Init
    if( m_pTableNode->GetDoc()->getIDocumentLayoutAccess().GetCurrentViewShell() )
    {
        m_pTableNode->DelFrames();
        SwNodeIndex aIdx( *m_pTableNode->EndOfSectionNode(), 1 );
        m_pTableNode->MakeOwnFrames(&aIdx);
    }
}

void SwXMLTableContext::MakeTable( SwTableBox *pBox, sal_Int32 nW )
{
    //FIXME: here would be a great place to handle XmlId for subtable
    m_pLineFormat = GetParentTable()->m_pLineFormat;
    m_pBoxFormat = GetParentTable()->m_pBoxFormat;
    m_nWidth = nW;
    m_bRelWidth = GetParentTable()->m_bRelWidth;

    MakeTable_( pBox );

    for (std::unique_ptr<SwXMLTableRow_Impl> & rpRow : *m_pRows)
    {
        // i#113600, to break the cyclic reference to SwXMLTableContext object
        rpRow->Dispose();
    }
}

const SwStartNode *SwXMLTableContext::InsertTableSection(
        const SwStartNode *const pPrevSttNd,
        OUString const*const pStringValueStyleName)
{
    // The topmost table is the only table that maintains the two members
    // pBox1 and bFirstSection.
    if( m_xParentTable.is() )
        return static_cast<SwXMLTableContext *>(m_xParentTable.get())
                    ->InsertTableSection(pPrevSttNd, pStringValueStyleName);

    const SwStartNode *pStNd;
    Reference<XUnoTunnel> xCursorTunnel( GetImport().GetTextImport()->GetCursor(),
                                       UNO_QUERY);
    OSL_ENSURE( xCursorTunnel.is(), "missing XUnoTunnel for Cursor" );
    OTextCursorHelper *pTextCursor = reinterpret_cast< OTextCursorHelper * >(
            sal::static_int_cast< sal_IntPtr >( xCursorTunnel->getSomething( OTextCursorHelper::getUnoTunnelId() )));
    OSL_ENSURE( pTextCursor, "SwXTextCursor missing" );

    if( m_bFirstSection )
    {
        // The Cursor already is in the first section
        pStNd = pTextCursor->GetPaM()->GetNode().FindTableBoxStartNode();
        m_bFirstSection = false;
        GetImport().GetTextImport()->SetStyleAndAttrs( GetImport(),
            GetImport().GetTextImport()->GetCursor(), "Standard", true );
    }
    else
    {
        SwDoc* pDoc = SwImport::GetDocFromXMLImport( GetSwImport() );
        const SwEndNode *pEndNd = pPrevSttNd ? pPrevSttNd->EndOfSectionNode()
                                             : m_pTableNode->EndOfSectionNode();
        // #i78921# - make code robust
        OSL_ENSURE( pDoc, "<SwXMLTableContext::InsertTableSection(..)> - no <pDoc> at <SwXTextCursor> instance - <SwXTextCurosr> doesn't seem to be registered at a <SwUnoCursor> instance." );
        if ( !pDoc )
        {
            pDoc = const_cast<SwDoc*>(pEndNd->GetDoc());
        }
        sal_uInt32 nOffset = pPrevSttNd ? 1UL : 0UL;
        SwNodeIndex aIdx( *pEndNd, nOffset );
        SwTextFormatColl *pColl =
            pDoc->getIDocumentStylePoolAccess().GetTextCollFromPool( RES_POOLCOLL_STANDARD, false );
        pStNd = pDoc->GetNodes().MakeTextSection( aIdx, SwTableBoxStartNode,
                                                 pColl );
        // Consider the case that a table is defined without a row.
        if( !pPrevSttNd && m_pBox1 != nullptr )

        {
            m_pBox1->m_pStartNode = pStNd;
            SwContentNode *pCNd = pDoc->GetNodes()[ pStNd->GetIndex() + 1 ]
                                                            ->GetContentNode();
            SwPosition aPos( *pCNd );
            aPos.nContent.Assign( pCNd, 0U );

            const uno::Reference< text::XTextRange > xTextRange =
                SwXTextRange::CreateXTextRange( *pDoc, aPos, nullptr );
            Reference < XText > xText = xTextRange->getText();
            Reference < XTextCursor > xTextCursor =
                xText->createTextCursorByRange( xTextRange );
            GetImport().GetTextImport()->SetCursor( xTextCursor );
        }
    }

    if (pStringValueStyleName)
    {   // fdo#62147: apply style to paragraph on string-value cell
        GetImport().GetTextImport()->SetStyleAndAttrs( GetImport(),
            GetImport().GetTextImport()->GetCursor(), *pStringValueStyleName,
            true, false, -1, false); // parameters same as sCellParaStyleName
    }

    return pStNd;
}

void SwXMLTableContext::EndElement()
{
    if( IsValid() && !m_xParentTable.is() )
    {
        MakeTable();
        GetImport().GetTextImport()->SetCursor( m_xOldCursor );
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
