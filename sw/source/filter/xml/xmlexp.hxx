/*************************************************************************
 *
 *  $RCSfile: xmlexp.hxx,v $
 *
 *  $Revision: 1.17 $
 *
 *  last change: $Author: mtg $ $Date: 2001-03-19 13:46:11 $
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

class SwPaM;
class SwFmt;
class SwFrmFmt;
class SvXMLUnitConverter;
class SvXMLExportItemMapper;
class SvXMLAutoStylePoolP;
class SwTableLine;
class SwTableLines;
class SwTableBox;
class SwXMLTableColumn_Impl;
class SwXMLTableLines_Impl;
class SwXMLTableLinesCache_Impl;
class SwXMLTableColumnsSortByWidth_Impl;
class SwXMLTableFrmFmtsSort_Impl;
class SwTableNode;
class XMLPropertySetMapper;


class SwXMLExport : public SvXMLExport
{
    friend class SwXMLExpContext;

#ifdef XML_CORE_API
    SwPaM                       *pCurPaM;       // the current PaM
    SwPaM                       *pOrigPaM;      // the original PaM
#endif

    SvXMLUnitConverter          *pTwipUnitConv;

    SvXMLExportItemMapper       *pTableItemMapper;
    SwXMLTableLinesCache_Impl   *pTableLines;

    SvXMLItemMapEntriesRef      xTableItemMap;
    SvXMLItemMapEntriesRef      xTableRowItemMap;
    SvXMLItemMapEntriesRef      xTableCellItemMap;
    UniReference < XMLPropertySetMapper > xParaPropMapper;

    sal_Int32                   nContentProgressStart;
    sal_Bool                    bExportWholeDoc : 1;// export whole document?
    sal_Bool                    bBlock : 1;         // export text block?
    sal_Bool                    bExportFirstTableOnly : 1;
    sal_Bool                    bShowProgress : 1;

    void _InitItemExport();
    void _FinitItemExport();
    void ExportTableLinesAutoStyles( const SwTableLines& rLines,
                                 sal_uInt32 nAbsWidth,
                                 sal_uInt32 nBaseWidth,
                                 const ::rtl::OUString& rNamePrefix,
                                 SwXMLTableColumnsSortByWidth_Impl& rExpCols,
                                 SwXMLTableFrmFmtsSort_Impl& rExpRows,
                                 SwXMLTableFrmFmtsSort_Impl& rExpCells,
                                 sal_Bool bTop=sal_False );


    void ExportFmt( const SwFmt& rFmt, const sal_Char *pFamily = 0 );
    void ExportTableFmt( const SwFrmFmt& rFmt, sal_uInt32 nAbsWidth );

    void ExportTableColumnStyle( const SwXMLTableColumn_Impl& rCol );
    void ExportTableBox( const SwTableBox& rBox, sal_uInt16 nColSpan );
    void ExportTableLine( const SwTableLine& rLine,
                          const SwXMLTableLines_Impl& rLines );
    void ExportTableLines( const SwTableLines& rLines,
                           sal_Bool bHeadline=sal_False );

    virtual void _ExportMeta();
    virtual void _ExportFontDecls();
    virtual void _ExportStyles( sal_Bool bUsed );
    virtual void _ExportAutoStyles();
    virtual void _ExportMasterStyles();
    virtual void _ExportContent();
    virtual void GetViewSettings(com::sun::star::uno::Sequence<com::sun::star::beans::PropertyValue>& aProps);
    virtual void GetConfigurationSettings(com::sun::star::uno::Sequence<com::sun::star::beans::PropertyValue>& aProps);

#ifdef XML_CORE_API
    void SetCurPaM( SwPaM& rPaM, sal_Bool bWhole, sal_Bool bTabOnly );
#endif

    const ::rtl::OUString sNumberFormat;
    const ::rtl::OUString sCell;

protected:

    virtual XMLTextParagraphExport* CreateTextParagraphExport();
    virtual SvXMLAutoStylePoolP* CreateAutoStylePool();
    virtual XMLPageExport* CreatePageExport();
    virtual XMLShapeExport* CreateShapeExport();
    virtual XMLFontAutoStylePool* CreateFontAutoStylePool();

public:

    SwXMLExport(sal_uInt16 nExportFlags = EXPORT_ALL);
#ifdef XML_CORE_API
    SwXMLExport( const ::com::sun::star::uno::Reference<
                     ::com::sun::star::frame::XModel > & rModel,
                 SwPaM& rPaM, const ::rtl::OUString& rFileName,
                 const ::com::sun::star::uno::Reference<
                     ::com::sun::star::xml::sax::XDocumentHandler > & rHandler,
                 const ::com::sun::star::uno::Reference<
                    ::com::sun::star::document::XGraphicObjectResolver > &,
                 sal_Bool bExpWholeDoc, sal_Bool bExpFirstTableOnly,
                 sal_Bool bShowProgr );
#endif
    virtual ~SwXMLExport();

    void setBlockMode();

    virtual sal_uInt32 exportDoc( const sal_Char *pClass=0 );

    inline const SvXMLUnitConverter& GetTwipUnitConverter() const;

    void ExportTableAutoStyles( const SwTableNode& rTblNd );
    void ExportTable( const SwTableNode& rTblNd );

    SvXMLExportItemMapper& GetTableItemMapper() { return *pTableItemMapper; }
    const UniReference < XMLPropertySetMapper >& GetParaPropMapper()
    {
        return xParaPropMapper;
    }

    sal_Bool IsShowProgress() const { return bShowProgress; }
    sal_Bool IsBlockMode() const { return bBlock; }

    // XUnoTunnel
    static const ::com::sun::star::uno::Sequence< sal_Int8 > & getUnoTunnelId() throw();
    virtual sal_Int64 SAL_CALL getSomething( const ::com::sun::star::uno::Sequence< sal_Int8 >& aIdentifier ) throw(::com::sun::star::uno::RuntimeException);
};

inline const SvXMLUnitConverter& SwXMLExport::GetTwipUnitConverter() const
{
    return *pTwipUnitConv;
}


#endif  //  _XMLEXP_HXX

