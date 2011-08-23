/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 * 
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#ifndef _XMLTBLI_HXX
#define _XMLTBLI_HXX

#ifndef _XMLOFF_XMLTEXTTABLECONTEXT_HXX
#include <bf_xmloff/XMLTextTableContext.hxx>
#endif

#include <hash_map>

#if !defined(_SVSTDARR_USHORTS_DECL) || !defined(_SVSTDARR_BOOLS_DECL) || !defined(_SVSTDARR_STRINGSDTOR_DECL)
#define _SVSTDARR_USHORTS
#define _SVSTDARR_BOOLS
#define _SVSTDARR_STRINGSDTOR
#include <bf_svtools/svstdarr.hxx>
#endif
namespace binfilter {

class SwXMLImport;
class SwTableNode;
class SwTableBox;
class SwTableLine;
class SwStartNode;
class SwTableBoxFmt;
class SwTableLineFmt;
class SwXMLTableCell_Impl;
class SwXMLTableRows_Impl;
class SwXMLDDETableContext_Impl;
class TableBoxIndexHasher;
class TableBoxIndex;

namespace com { namespace sun { namespace star {
    namespace text { class XTextContent; }
    namespace text { class XTextCursor; }
} } }



class SwXMLTableContext : public XMLTextTableContext
{
    ::rtl::OUString		aStyleName;
    ::rtl::OUString		aDfltCellStyleName;

    SvUShorts			aColumnWidths;
    SvBools				aColumnRelWidths;
    SvStringsDtor		*pColumnDefaultCellStyleNames;

    ::com::sun::star::uno::Reference <
        ::com::sun::star::text::XTextCursor > xOldCursor;
    ::com::sun::star::uno::Reference <
        ::com::sun::star::text::XTextContent > xTextContent;

    SwXMLTableRows_Impl *pRows;

    SwTableNode			*pTableNode;
    SwTableBox 			*pBox1;
    const SwStartNode	*pSttNd1;

    SwTableBoxFmt		*pBoxFmt;
    SwTableLineFmt 		*pLineFmt;

    // hash map of shared format, indexed by the (XML) style name,
    // the column width, and protection flag
    typedef std::hash_map<TableBoxIndex,SwTableBoxFmt*,
                          TableBoxIndexHasher> map_BoxFmt;
    map_BoxFmt* pSharedBoxFormats;

    SvXMLImportContextRef	xParentTable;	// if table is a sub table

    SwXMLDDETableContext_Impl	*pDDESource;

    sal_Bool			bFirstSection : 1;
    sal_Bool			bRelWidth : 1;
    sal_Bool			bHasHeading : 1;

    sal_uInt32 			nCurRow;
    sal_uInt32			nCurCol;
    sal_Int32			nWidth;

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
    void ReplaceWithEmptyCell( sal_uInt32 nRow, sal_uInt32 nCol );

    /** sets the appropriate SwTblBoxFmt at pBox. */
    SwTableBoxFmt* GetSharedBoxFormat( 
        SwTableBox* pBox,   /// the table box
        const ::rtl::OUString& rStyleName, /// XML style name
        sal_Int32 nColumnWidth,     /// width of column
        sal_Bool bProtected,        /// is cell protected?
        sal_Bool bMayShare, /// may the format be shared (no value, formula...)
        sal_Bool& bNew,     /// true, if the format it not from the cache
        sal_Bool* pModifyLocked );  /// if set, call pBox->LockModify() and return old lock status

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
                SwXMLTableContext *pTable );

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
                     SwXMLTableContext *pTable=0,
                     sal_Bool bIsProtected = sal_False,
                     const ::rtl::OUString *pFormula=0,
                     sal_Bool bHasValue = sal_False,
                     double fValue = 0.0 );
    void InsertRow( const ::rtl::OUString& rStyleName,
                    const ::rtl::OUString& rDfltCellStyleName,
                    sal_Bool bInHead );
    void FinishRow();
    void InsertRepRows( sal_uInt32 nCount );
    SwXMLTableCell_Impl *GetCell( sal_uInt32 nRow, sal_uInt32 nCol ) const;
    const SwStartNode *InsertTableSection( const SwStartNode *pPrevSttNd=0 );

    virtual void EndElement();

    virtual ::com::sun::star::uno::Reference <
            ::com::sun::star::text::XTextContent > GetXTextContent() const;
};

inline SwXMLTableContext *SwXMLTableContext::GetParentTable() const
{
    return (SwXMLTableContext *)&xParentTable;
}

inline sal_uInt32 SwXMLTableContext::GetColumnCount() const
{
    return aColumnWidths.Count();
}

inline const SwStartNode *SwXMLTableContext::GetLastStartNode() const
{
    return GetPrevStartNode( 0UL, GetColumnCount() );
}

inline sal_Bool SwXMLTableContext::HasColumnDefaultCellStyleNames() const
{
    return pColumnDefaultCellStyleNames != 0;
}

} //namespace binfilter
#endif
