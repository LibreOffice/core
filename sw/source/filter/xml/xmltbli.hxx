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

#ifndef _XMLTBLI_HXX
#define _XMLTBLI_HXX

#include <xmloff/XMLTextTableContext.hxx>

// STL include
#include <boost/unordered_map.hpp>
#include <boost/ptr_container/ptr_vector.hpp>
#include <vector>

class SwXMLImport;
class SwTableNode;
class SwTableBox;
class SwTableLine;
class SwStartNode;
class SwTableBoxFmt;
class SwTableLineFmt;
class SwXMLTableCell_Impl;
class SwXMLTableRow_Impl;
typedef boost::ptr_vector<SwXMLTableRow_Impl> SwXMLTableRows_Impl;
class SwXMLDDETableContext_Impl;
class TableBoxIndexHasher;
class TableBoxIndex;

namespace com { namespace sun { namespace star {
    namespace text { class XTextContent; }
    namespace text { class XTextCursor; }
} } }



class SwXMLTableContext : public XMLTextTableContext
{
    ::rtl::OUString     aStyleName;
    ::rtl::OUString     aDfltCellStyleName;
    /// NB: this contains the xml:id only if this table is a subtable!
    ::rtl::OUString     mXmlId;

    //! Holds basic information about a column's width.
    struct ColumnWidthInfo {
        sal_uInt16 width;      //!< Column width (absolute or relative).
        bool   isRelative; //!< True for a relative width, false for absolute.
        inline ColumnWidthInfo(sal_uInt16 wdth, bool isRel) : width(wdth), isRelative(isRel) {};
    };
    std::vector<ColumnWidthInfo> aColumnWidths;
    std::vector<String> *pColumnDefaultCellStyleNames;

    ::com::sun::star::uno::Reference <
        ::com::sun::star::text::XTextCursor > xOldCursor;
    ::com::sun::star::uno::Reference <
        ::com::sun::star::text::XTextContent > xTextContent;

    SwXMLTableRows_Impl *pRows;

    SwTableNode         *pTableNode;
    SwTableBox          *pBox1;
    const SwStartNode   *pSttNd1;

    SwTableBoxFmt       *pBoxFmt;
    SwTableLineFmt      *pLineFmt;

    // hash map of shared format, indexed by the (XML) style name,
    // the column width, and protection flag
    typedef boost::unordered_map<TableBoxIndex,SwTableBoxFmt*,
                          TableBoxIndexHasher> map_BoxFmt;
    map_BoxFmt* pSharedBoxFormats;

    SvXMLImportContextRef   xParentTable;   // if table is a sub table

    SwXMLDDETableContext_Impl   *pDDESource;

    sal_Bool            bFirstSection : 1;
    sal_Bool            bRelWidth : 1;
    sal_Bool            bHasSubTables : 1;

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

    /** sets the appropriate SwTblBoxFmt at pBox. */
    SwTableBoxFmt* GetSharedBoxFormat(
        SwTableBox* pBox,   /// the table box
        const ::rtl::OUString& rStyleName, /// XML style name
        sal_Int32 nColumnWidth,     /// width of column
        sal_Bool bProtected,        /// is cell protected?
        sal_Bool bMayShare, /// may the format be shared (no value, formula...)
        sal_Bool& bNew,     /// true, if the format it not from the cache
        bool* pModifyLocked );  /// if set, call pBox->LockModify() and return old lock status

public:

    TYPEINFO();

    SwXMLTableContext( SwXMLImport& rImport, sal_uInt16 nPrfx,
                   const ::rtl::OUString& rLName,
                const ::com::sun::star::uno::Reference<
                    ::com::sun::star::xml::sax::XAttributeList > & xAttrList );
    SwXMLTableContext( SwXMLImport& rImport, sal_uInt16 nPrfx,
                   const ::rtl::OUString& rLName,
                  const ::com::sun::star::uno::Reference<
                    ::com::sun::star::xml::sax::XAttributeList > & xAttrList,
                SwXMLTableContext *pTable,
                const ::rtl::OUString& i_rXmlId );

    virtual ~SwXMLTableContext();

    virtual SvXMLImportContext *CreateChildContext( sal_uInt16 nPrefix,
                const ::rtl::OUString& rLocalName,
                const ::com::sun::star::uno::Reference<
                    ::com::sun::star::xml::sax::XAttributeList > & xAttrList );

    SwXMLImport& GetSwImport() { return (SwXMLImport&)GetImport(); }

    void InsertColumn( sal_Int32 nWidth, sal_Bool bRelWidth,
                       const ::rtl::OUString *pDfltCellStyleName = 0 );
    sal_Int32 GetColumnWidth( sal_uInt32 nCol, sal_uInt32 nColSpan=1UL ) const;
    ::rtl::OUString GetColumnDefaultCellStyleName( sal_uInt32 nCol ) const;
    inline sal_uInt32 GetColumnCount() const;
    inline sal_Bool HasColumnDefaultCellStyleNames() const;

    sal_Bool IsInsertCellPossible() const { return nCurCol < GetColumnCount(); }
    sal_Bool IsInsertColPossible() const { return nCurCol < USHRT_MAX; }
    sal_Bool IsInsertRowPossible() const { return nCurRow < USHRT_MAX; }
    sal_Bool IsValid() const { return pTableNode != 0; }

    void InsertCell( const ::rtl::OUString& rStyleName,
                     sal_uInt32 nRowSpan=1U, sal_uInt32 nColSpan=1U,
                     const SwStartNode *pStNd=0,
                     const ::rtl::OUString & i_rXmlId = ::rtl::OUString(),
                     SwXMLTableContext *pTable=0,
                     sal_Bool bIsProtected = sal_False,
                     const ::rtl::OUString *pFormula=0,
                     sal_Bool bHasValue = sal_False,
                     double fValue = 0.0,
                     ::rtl::OUString const*const pStringValue = 0);
    void InsertRow( const ::rtl::OUString& rStyleName,
                    const ::rtl::OUString& rDfltCellStyleName,
                    bool bInHead,
                    const ::rtl::OUString & i_rXmlId = ::rtl::OUString() );
    void FinishRow();
    void InsertRepRows( sal_uInt32 nCount );
    const SwXMLTableCell_Impl *GetCell( sal_uInt32 nRow, sal_uInt32 nCol ) const;
    SwXMLTableCell_Impl *GetCell( sal_uInt32 nRow, sal_uInt32 nCol );
    const SwStartNode *InsertTableSection(const SwStartNode *pPrevSttNd = 0,
                                  OUString const* pStringValueStyleName = 0);

    virtual void EndElement();

    virtual ::com::sun::star::uno::Reference <
            ::com::sun::star::text::XTextContent > GetXTextContent() const;

    void SetHasSubTables( sal_Bool bNew ) { bHasSubTables = bNew; }
};

inline SwXMLTableContext *SwXMLTableContext::GetParentTable() const
{
    return (SwXMLTableContext *)&xParentTable;
}

inline sal_uInt32 SwXMLTableContext::GetColumnCount() const
{
    return aColumnWidths.size();
}

inline const SwStartNode *SwXMLTableContext::GetLastStartNode() const
{
    return GetPrevStartNode( 0UL, GetColumnCount() );
}

inline sal_Bool SwXMLTableContext::HasColumnDefaultCellStyleNames() const
{
    return pColumnDefaultCellStyleNames != 0;
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
