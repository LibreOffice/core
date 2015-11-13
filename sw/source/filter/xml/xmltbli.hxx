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

#ifndef INCLUDED_SW_SOURCE_FILTER_XML_XMLTBLI_HXX
#define INCLUDED_SW_SOURCE_FILTER_XML_XMLTBLI_HXX

#include <xmloff/XMLTextTableContext.hxx>

#include <memory>
#include <unordered_map>
#include <vector>

class SwXMLImport;
class SwTableNode;
class SwTableBox;
class SwTableLine;
class SwStartNode;
class SwTableBoxFormat;
class SwTableLineFormat;
class SwXMLTableCell_Impl;
class SwXMLTableRow_Impl;
typedef std::vector<std::unique_ptr<SwXMLTableRow_Impl>> SwXMLTableRows_Impl;
class SwXMLDDETableContext_Impl;
class TableBoxIndexHasher;
class TableBoxIndex;

namespace com { namespace sun { namespace star {
    namespace text { class XTextContent; }
    namespace text { class XTextCursor; }
} } }

class SwXMLTableContext : public XMLTextTableContext
{
    OUString     m_aStyleName;
    OUString     m_aDfltCellStyleName;

    //! Holds basic information about a column's width.
    struct ColumnWidthInfo {
        sal_uInt16 width;      //!< Column width (absolute or relative).
        bool   isRelative; //!< True for a relative width, false for absolute.
        inline ColumnWidthInfo(sal_uInt16 wdth, bool isRel) : width(wdth), isRelative(isRel) {};
    };
    std::vector<ColumnWidthInfo> m_aColumnWidths;
    std::vector<OUString> *m_pColumnDefaultCellStyleNames;

    css::uno::Reference< css::text::XTextCursor > m_xOldCursor;
    css::uno::Reference< css::text::XTextContent > m_xTextContent;

    SwXMLTableRows_Impl * m_pRows;

    SwTableNode         *m_pTableNode;
    SwTableBox          *m_pBox1;
    const SwStartNode   *m_pSttNd1;

    SwTableBoxFormat       *m_pBoxFormat;
    SwTableLineFormat      *m_pLineFormat;

    // hash map of shared format, indexed by the (XML) style name,
    // the column width, and protection flag
    typedef std::unordered_map<TableBoxIndex,SwTableBoxFormat*,
                          TableBoxIndexHasher> map_BoxFormat;
    map_BoxFormat* m_pSharedBoxFormats;

    SvXMLImportContextRef   m_xParentTable;   // if table is a sub table

    SwXMLDDETableContext_Impl   *m_pDDESource;

    bool            m_bFirstSection : 1;
    bool            m_bRelWidth : 1;
    bool            m_bHasSubTables : 1;

    sal_uInt16              m_nHeaderRows;
    sal_uInt32          m_nCurRow;
    sal_uInt32          m_nCurCol;
    sal_Int32           m_nWidth;

    SwTableBox *NewTableBox( const SwStartNode *pStNd,
                             SwTableLine *pUpper );
    SwTableBox *MakeTableBox( SwTableLine *pUpper,
                              const SwXMLTableCell_Impl *pStartNode,
                              sal_uInt32 nTopRow, sal_uInt32 nLeftCol,
                              sal_uInt32 nBottomRow, sal_uInt32 nRightCol );
    SwTableBox *MakeTableBox( SwTableLine *pUpper,
                              sal_uInt32 nTopRow, sal_uInt32 nLeftCol,
                              sal_uInt32 nBottomRow, sal_uInt32 nRightCol );
    SwTableLine *MakeTableLine( SwTableBox *pUpper,
                                sal_uInt32 nTopRow, sal_uInt32 nLeftCol,
                                sal_uInt32 nBottomRow, sal_uInt32 nRightCol );

    void _MakeTable( SwTableBox *pBox=nullptr );
    void MakeTable( SwTableBox *pBox, sal_Int32 nWidth );
    void MakeTable();

    inline SwXMLTableContext *GetParentTable() const;

    const SwStartNode *GetPrevStartNode( sal_uInt32 nRow,
                                         sal_uInt32 nCol ) const;
    inline const SwStartNode *GetLastStartNode() const;
    void FixRowSpan( sal_uInt32 nRow, sal_uInt32 nCol, sal_uInt32 nColSpan );
    void ReplaceWithEmptyCell( sal_uInt32 nRow, sal_uInt32 nCol, bool bRows );

    /** sets the appropriate SwTableBoxFormat at pBox. */
    SwTableBoxFormat* GetSharedBoxFormat(
        SwTableBox* pBox,   /// the table box
        const OUString& rStyleName, /// XML style name
        sal_Int32 nColumnWidth,     /// width of column
        bool bProtected,        /// is cell protected?
        bool bMayShare, /// may the format be shared (no value, formula...)
        bool& bNew,     /// true, if the format it not from the cache
        bool* pModifyLocked );  /// if set, call pBox->LockModify() and return old lock status

public:


    SwXMLTableContext( SwXMLImport& rImport, sal_uInt16 nPrfx,
                       const OUString& rLName,
                       const css::uno::Reference< css::xml::sax::XAttributeList > & xAttrList );
    SwXMLTableContext( SwXMLImport& rImport, sal_uInt16 nPrfx,
                       const OUString& rLName,
                       const css::uno::Reference< css::xml::sax::XAttributeList > & xAttrList,
                       SwXMLTableContext *pTable );

    virtual ~SwXMLTableContext();

    virtual SvXMLImportContext *CreateChildContext( sal_uInt16 nPrefix,
                const OUString& rLocalName,
                const css::uno::Reference< css::xml::sax::XAttributeList > & xAttrList ) override;

    SwXMLImport& GetSwImport() { return static_cast<SwXMLImport&>(GetImport()); }

    void InsertColumn( sal_Int32 nWidth, bool bRelWidth,
                       const OUString *pDfltCellStyleName = nullptr );
    sal_Int32 GetColumnWidth( sal_uInt32 nCol, sal_uInt32 nColSpan=1UL ) const;
    OUString GetColumnDefaultCellStyleName( sal_uInt32 nCol ) const;
    inline sal_uInt32 GetColumnCount() const;
    inline bool HasColumnDefaultCellStyleNames() const;

    bool IsInsertCellPossible() const { return m_nCurCol < GetColumnCount(); }
    bool IsInsertColPossible() const { return m_nCurCol < USHRT_MAX; }
    bool IsInsertRowPossible() const { return m_nCurRow < USHRT_MAX; }
    bool IsValid() const { return m_pTableNode != nullptr; }

    void InsertCell( const OUString& rStyleName,
                     sal_uInt32 nRowSpan=1U, sal_uInt32 nColSpan=1U,
                     const SwStartNode *pStNd=nullptr,
                     const OUString & i_rXmlId = OUString(),
                     SwXMLTableContext *pTable=nullptr,
                     bool bIsProtected = false,
                     const OUString *pFormula=nullptr,
                     bool bHasValue = false,
                     double fValue = 0.0,
                     OUString const*const pStringValue = nullptr);
    void InsertRow( const OUString& rStyleName,
                    const OUString& rDfltCellStyleName,
                    bool bInHead,
                    const OUString & i_rXmlId = OUString() );
    void FinishRow();
    void InsertRepRows( sal_uInt32 nCount );
    const SwXMLTableCell_Impl *GetCell( sal_uInt32 nRow, sal_uInt32 nCol ) const;
    SwXMLTableCell_Impl *GetCell( sal_uInt32 nRow, sal_uInt32 nCol );
    const SwStartNode *InsertTableSection(const SwStartNode *pPrevSttNd = nullptr,
                                  OUString const* pStringValueStyleName = nullptr);

    virtual void EndElement() override;

    void SetHasSubTables( bool bNew ) { m_bHasSubTables = bNew; }
};

inline SwXMLTableContext *SwXMLTableContext::GetParentTable() const
{
    return static_cast<SwXMLTableContext *>(&m_xParentTable);
}

inline sal_uInt32 SwXMLTableContext::GetColumnCount() const
{
    return m_aColumnWidths.size();
}

inline const SwStartNode *SwXMLTableContext::GetLastStartNode() const
{
    return GetPrevStartNode( 0UL, GetColumnCount() );
}

inline bool SwXMLTableContext::HasColumnDefaultCellStyleNames() const
{
    return m_pColumnDefaultCellStyleNames != nullptr;
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
