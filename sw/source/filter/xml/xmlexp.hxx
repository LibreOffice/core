/*************************************************************************
 *
 *  $RCSfile: xmlexp.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: mib $ $Date: 2000-10-12 17:30:28 $
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

#ifndef _XMLEXP_HXX
#define _XMLEXP_HXX

#ifndef _XMLOFF_XMLEXP_HXX
#include <xmloff/xmlexp.hxx>
#endif

#ifndef _XMLOFF_XMLITMAP_HXX
#include <xmloff/xmlitmap.hxx>
#endif
#ifndef _UNIVERSALL_REFERENCE_HXX
#include <xmloff/uniref.hxx>
#endif

class SwDoc;
class SwPaM;
class SwTxtNode;
class SwSectionNode;
class SwFmt;
class SwFrmFmt;
class SvXMLUnitConverter;
class SvXMLExportItemMapper;
class SwXMLAutoStylePool;
class SvXMLAutoStylePool;
class SvXMLAutoStylePoolP;
class SwNodeNum;
class SfxPoolItem;
class SfxItemSet;
class SwXMLNumRuleInfo;
class OUStrings_Impl;
class OUStringsSort_Impl;
class SwTableLine;
class SwTableLines;
class SwTableBox;
class SwXMLTableColumn_Impl;
class SwXMLTableLines_Impl;
class SwXMLTableLinesCache_Impl;
class SwXMLTableColumnsSortByWidth_Impl;
class SwXMLTableFrmFmtsSort_Impl;
class SwTableNode;
#ifdef XML_CORE_API
class SvXMLNumFmtExport;
class XMLTextFieldExport;
#endif
class XMLPropertySetMapper;
class XMLTextMasterPageExport;
class SwXMLTextParagraphExport;

class SwXMLExport : public SvXMLExport
{
    friend class SwXMLExpContext;

    SwDoc                       *pDoc;          // the current doc
#ifdef XML_CORE_API
    SwPaM                       *pCurPaM;       // the current PaM
    SwPaM                       *pOrigPaM;      // the original PaM
#endif

    SvXMLUnitConverter          *pTwipUnitConv;

#ifdef XML_CORE_API
    SvXMLExportItemMapper       *pParaItemMapper;
#endif
    SvXMLExportItemMapper       *pTableItemMapper;
#ifdef XML_CORE_API
    SwXMLAutoStylePool          *pItemSetAutoStylePool;
    OUStrings_Impl              *pListElements;
    OUStringsSort_Impl          *pExportedLists;
#endif
    SwXMLTableLinesCache_Impl   *pTableLines;
#ifdef XML_CORE_API
    SvXMLNumFmtExport           *pNumberFormatExport;
    XMLTextFieldExport          *pTextFieldExport;
#endif

    SvXMLItemMapEntriesRef      xTableItemMap;
    SvXMLItemMapEntriesRef      xTableRowItemMap;
    SvXMLItemMapEntriesRef      xTableCellItemMap;
    UniReference < XMLPropertySetMapper > xParaPropMapper;
    UniReference < XMLTextMasterPageExport> xMasterPageExport;

    sal_Bool                    bExportWholeDoc : 1;// export whole document?
    sal_Bool                    bExportFirstTableOnly : 1;
    sal_Bool                    bShowProgress : 1;

    void _InitItemExport();
    void _FinitItemExport();
#ifdef XML_CORE_API
    void _FinitNumRuleExport();

    void AddTextAutoStyle( const SfxPoolItem& rItem );
    void AddParaAutoStyle( const ::rtl::OUString& rParent,
                           const SfxItemSet& rItemSet );
    ::rtl::OUString FindTextAutoStyle( const SfxPoolItem& rItem );
    ::rtl::OUString FindParaAutoStyle( const ::rtl::OUString& rParent,
                                       const SfxItemSet& rItemSet );

    void ExportCurPaMAutoStyles( sal_Bool bExportWholePaM=sal_True );
    void ExportTxtNodeAutoStyles( const SwTxtNode& rTxtNd, xub_StrLen nStart,
                                  xub_StrLen nEnd, sal_Bool bExportWholeNode );
    void ExportSectionAutoStyles( const SwSectionNode& rSectNd );
#endif
    void ExportTableLinesAutoStyles( const SwTableLines& rLines,
                                 sal_uInt32 nAbsWidth,
                                 sal_uInt32 nBaseWidth,
                                 const ::rtl::OUString& rNamePrefix,
                                 SwXMLTableColumnsSortByWidth_Impl& rExpCols,
                                 SwXMLTableFrmFmtsSort_Impl& rExpRows,
                                 SwXMLTableFrmFmtsSort_Impl& rExpCells,
                                 sal_Bool bTop=sal_False );

#ifdef XML_CORE_API
    void ExportCurPaM( sal_Bool bExportWholePaM=sal_True );
    void ExportTxtNode( const SwTxtNode& rTxtNd, xub_StrLen nStart,
                        xub_StrLen nEnd, sal_Bool bExportWholeNode );
    void ExportSection( const SwSectionNode& rSectNd );
#endif

    void ExportFmt( const SwFmt& rFmt, const sal_Char *pFamily = 0 );
    void ExportTableFmt( const SwFrmFmt& rFmt, sal_uInt32 nAbsWidth );
#ifdef XML_CORE_API
    void ExportNumRules( sal_Bool bAuto, sal_Bool bUsed );
    void ExportNodeNum( const SwNodeNum& rNdNum );
    void ExportListChange( const SwXMLNumRuleInfo& rPrvInfo,
                           const SwXMLNumRuleInfo& rNextInfo );
#endif

    void ExportTableColumnStyle( const SwXMLTableColumn_Impl& rCol );
    void ExportTableBox( const SwTableBox& rBox, sal_uInt16 nColSpan );
    void ExportTableLine( const SwTableLine& rLine,
                          const SwXMLTableLines_Impl& rLines );
    void ExportTableLines( const SwTableLines& rLines,
                           sal_Bool bHeadline=sal_False );

//  virtual void _ExportMeta();
    virtual void _ExportStyles( sal_Bool bUsed );
    virtual void _ExportAutoStyles();
    virtual void _ExportMasterStyles();
    virtual void _ExportContent();

#ifdef XML_CORE_API
    void SetCurPaM( SwPaM& rPaM, sal_Bool bWhole, sal_Bool bTabOnly );
#endif

protected:

#ifndef XML_CORE_API
    virtual XMLTextParagraphExport* CreateTextParagraphExport();
    virtual SvXMLAutoStylePoolP* CreateAutoStylePool();
#endif

public:

    SwXMLExport( const ::com::sun::star::uno::Reference<
                     ::com::sun::star::frame::XModel > & rModel,
                 SwPaM& rPaM, const ::rtl::OUString& rFileName,
                 const ::com::sun::star::uno::Reference<
                     ::com::sun::star::xml::sax::XDocumentHandler > & rHandler,
                 sal_Bool bExpWholeDoc, sal_Bool bExpFirstTableOnly,
                 sal_Bool bShowProgr );
    virtual ~SwXMLExport();

    inline const SvXMLUnitConverter& GetTwipUnitConverter() const;

    void ExportTableAutoStyles( const SwTableNode& rTblNd );
    void ExportTable( const SwTableNode& rTblNd );

#ifdef XML_CORE_API
    SvXMLExportItemMapper& GetParaItemMapper() { return *pParaItemMapper; }
#endif
    SvXMLExportItemMapper& GetTableItemMapper() { return *pTableItemMapper; }
    const UniReference < XMLPropertySetMapper >& GetParaPropMapper()
    {
        return xParaPropMapper;
    }
#ifdef XML_CORE_API
    SwXMLAutoStylePool& GetItemSetAutoStylePool() { return *pItemSetAutoStylePool; }
#endif

#ifdef XML_CORE_API
    SvXMLNumFmtExport& GetNumberFormatExport() { return *pNumberFormatExport; }
    XMLTextFieldExport& GetTextFieldExport() { return *pTextFieldExport; }
#endif

    SwDoc& GetDoc() { return *pDoc; }
};

inline const SvXMLUnitConverter& SwXMLExport::GetTwipUnitConverter() const
{
    return *pTwipUnitConv;
}


#endif  //  _XMLEXP_HXX

