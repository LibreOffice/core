/*************************************************************************
 *
 *  $RCSfile: xmltbli.hxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 17:15:00 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef _XMLTBLI_HXX
#define _XMLTBLI_HXX

#ifndef _XMLOFF_XMLICTXT_HXX
#include <xmloff/xmlictxt.hxx>
#endif

#if !defined(_SVSTDARR_USHORTS_DECL) || !defined(_SVSTDARR_BOOLS_DECL)
#define _SVSTDARR_USHORTS
#define _SVSTDARR_BOOLS
#include <svtools/svstdarr.hxx>
#endif

class SwXMLImport;
class SwTableNode;
class SwTableBox;
class SwTableLine;
class SwStartNode;
class SwTableBoxFmt;
class SwTableLineFmt;
class SwXMLTableCell_Impl;
class SwXMLTableRows_Impl;


#ifdef XML_CORE_API
enum SwXMLStyleSubFamily
{
    SW_STYLE_SUBFAMILY_TABLE = 0x0001,
    SW_STYLE_SUBFAMILY_TABLE_LINE = 0x0002,
    SW_STYLE_SUBFAMILY_TABLE_BOX = 0x0004,
    SW_STYLE_SUBFAMILY_TABLE_COL = 0x0008
};
#endif


class SwXMLTableContext : public SvXMLImportContext
{
    ::rtl::OUString     aStyleName;

    SvUShorts           aColumnWidths;
    SvBools             aColumnRelWidths;

#ifndef XML_CORE_API
    ::com::sun::star::uno::Reference <
        ::com::sun::star::text::XTextCursor > xOldCursor;
#endif
    SwXMLTableRows_Impl *pRows;

    SwTableNode         *pTableNode;
    SwTableBox          *pBox1;
    const SwStartNode   *pSttNd1;

    SwTableBoxFmt       *pBoxFmt;
    SwTableLineFmt      *pLineFmt;

    SvXMLImportContextRef   xParentTable;   // if table ia a sub table

    sal_Bool            bFirstSection : 1;
    sal_Bool            bRelWidth : 1;
    sal_Bool            bHasHeading : 1;

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
    void ReplaceWithEmptyCell( sal_uInt32 nRow, sal_uInt32 nCol );

public:

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

    void InsertColumn( sal_Int32 nWidth, sal_Bool bRelWidth );
    sal_Int32 GetColumnWidth( sal_uInt32 nCol, sal_uInt32 nColSpan=1UL ) const;
    inline sal_uInt32 GetColumnCount() const;

    sal_Bool IsInsertCellPossible() const { return nCurCol < GetColumnCount(); }
    sal_Bool IsInsertColPossible() const { return nCurCol < USHRT_MAX; }
    sal_Bool IsInsertRowPossible() const { return nCurRow < USHRT_MAX; }


    void InsertCell( const ::rtl::OUString& rStyleName,
                     sal_uInt32 nRowSpan=1U, sal_uInt32 nColSpan=1U,
                     const SwStartNode *pStNd=0,
                     SwXMLTableContext *pTable=0 );
    void InsertRow( const ::rtl::OUString& rStyleName,
                    sal_Bool bInHead );
    void FinishRow();
    SwXMLTableCell_Impl *GetCell( sal_uInt32 nRow, sal_uInt32 nCol ) const;
    const SwStartNode *InsertTableSection( const SwStartNode *pPrevSttNd=0 );

    virtual void EndElement();
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


#endif
