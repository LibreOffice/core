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
    OUString     aStyleName;
    OUString     aDfltCellStyleName;

    //! Holds basic information about a column's width.
    struct ColumnWidthInfo {
        sal_uInt16 width;      //!< Column width (absolute or relative).
        bool   isRelative; //!< True for a relative width, false for absolute.
        inline ColumnWidthInfo(sal_uInt16 wdth, bool isRel) : width(wdth), isRelative(isRel) {};
    };
    std::vector<ColumnWidthInfo> aColumnWidths;
    std::vector<OUString> *pColumnDefaultCellStyleNames;

    ::com::sun::star::uno::Reference <
        ::com::sun::star::text::XTextCursor > xOldCursor;
    ::com::sun::star::uno::Reference <
        ::com::sun::star::text::XTextContent > xTextContent;

    SwXMLTableRows_Impl * m_pRows;

    SwTableNode         *pTableNode;
    SwTableBox          *pBox1;
    const SwStartNode   *pSttNd1;

    SwTableBoxFormat       *pBoxFormat;
    SwTableLineFormat      *pLineFormat;

    // hash map of shared format, indexed by the (XML) style name,
    // the column width, and protection flag
    typedef std::unordered_map<TableBoxIndex,SwTableBoxFormat*,
                          TableBoxIndexHasher> map_BoxFormat;
    map_BoxFormat* pSharedBoxFormats;

    SvXMLImportContextRef   xParentTable;   // if table is a sub table

    SwXMLDDETableContext_Impl   *pDDESource;

    bool            bFirstSection : 1;
    bool            bRelWidth : 1;
    bool            bHasSubTables : 1;

    sal_uInt16              nHeaderRows;
    sal_uInt32          nCurRow;
    sal_uInt32          nCurCol;
    sal_Int32           nWidth;

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

    void _MakeTable( SwTableBox *pBox=0 );
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
                const ::com::sun::star::uno::Reference<
                    ::com::sun::star::xml::sax::XAttributeList > & xAttrList );
    SwXMLTableContext( SwXMLImport& rImport, sal_uInt16 nPrfx,
                   const OUString& rLName,
                  const ::com::sun::star::uno::Reference<
                    ::com::sun::star::xml::sax::XAttributeList > & xAttrList,
                SwXMLTableContext *pTable );

    virtual ~SwXMLTableContext();

    virtual SvXMLImportContext *CreateChildContext( sal_uInt16 nPrefix,
                const OUString& rLocalName,
                const ::com::sun::star::uno::Reference<
                    ::com::sun::star::xml::sax::XAttributeList > & xAttrList ) override;

    SwXMLImport& GetSwImport() { return static_cast<SwXMLImport&>(GetImport()); }

    void InsertColumn( sal_Int32 nWidth, bool bRelWidth,
                       const OUString *pDfltCellStyleName = 0 );
    sal_Int32 GetColumnWidth( sal_uInt32 nCol, sal_uInt32 nColSpan=1UL ) const;
    OUString GetColumnDefaultCellStyleName( sal_uInt32 nCol ) const;
    inline sal_uInt32 GetColumnCount() const;
    inline bool HasColumnDefaultCellStyleNames() const;

    bool IsInsertCellPossible() const { return nCurCol < GetColumnCount(); }
    bool IsInsertColPossible() const { return nCurCol < USHRT_MAX; }
    bool IsInsertRowPossible() const { return nCurRow < USHRT_MAX; }
    bool IsValid() const { return pTableNode != 0; }

    void InsertCell( const OUString& rStyleName,
                     sal_uInt32 nRowSpan=1U, sal_uInt32 nColSpan=1U,
                     const SwStartNode *pStNd=0,
                     const OUString & i_rXmlId = OUString(),
                     SwXMLTableContext *pTable=0,
                     bool bIsProtected = false,
                     const OUString *pFormula=0,
                     bool bHasValue = false,
                     double fValue = 0.0,
                     OUString const*const pStringValue = 0);
    void InsertRow( const OUString& rStyleName,
                    const OUString& rDfltCellStyleName,
                    bool bInHead,
                    const OUString & i_rXmlId = OUString() );
    void FinishRow();
    void InsertRepRows( sal_uInt32 nCount );
    const SwXMLTableCell_Impl *GetCell( sal_uInt32 nRow, sal_uInt32 nCol ) const;
    SwXMLTableCell_Impl *GetCell( sal_uInt32 nRow, sal_uInt32 nCol );
    const SwStartNode *InsertTableSection(const SwStartNode *pPrevSttNd = 0,
                                  OUString const* pStringValueStyleName = 0);

    virtual void EndElement() override;

    void SetHasSubTables( bool bNew ) { bHasSubTables = bNew; }
};

inline SwXMLTableContext *SwXMLTableContext::GetParentTable() const
{
    return static_cast<SwXMLTableContext *>(&xParentTable);
}

inline sal_uInt32 SwXMLTableContext::GetColumnCount() const
{
    return aColumnWidths.size();
}

inline const SwStartNode *SwXMLTableContext::GetLastStartNode() const
{
    return GetPrevStartNode( 0UL, GetColumnCount() );
}

inline bool SwXMLTableContext::HasColumnDefaultCellStyleNames() const
{
    return pColumnDefaultCellStyleNames != 0;
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
