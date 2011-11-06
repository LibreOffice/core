/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



#ifndef _XMLEXP_HXX
#define _XMLEXP_HXX

#include <xmloff/xmlexp.hxx>
#include "xmlitmap.hxx"
#include <xmloff/uniref.hxx>
#include <xmloff/xmltoken.hxx>

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
class SwXMLTableInfo_Impl;
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

    sal_Bool                    bExportWholeDoc : 1;// export whole document?
    sal_Bool                    bBlock : 1;         // export text block?
    sal_Bool                    bExportFirstTableOnly : 1;
    sal_Bool                    bShowProgress : 1;
    sal_Bool                    bSavedShowChanges : 1;

    void _InitItemExport();
    void _FinitItemExport();
    void ExportTableLinesAutoStyles( const SwTableLines& rLines,
                                 sal_uInt32 nAbsWidth,
                                 sal_uInt32 nBaseWidth,
                                 const ::rtl::OUString& rNamePrefix,
                                 SwXMLTableColumnsSortByWidth_Impl& rExpCols,
                                 SwXMLTableFrmFmtsSort_Impl& rExpRows,
                                 SwXMLTableFrmFmtsSort_Impl& rExpCells,
                                 SwXMLTableInfo_Impl& rTblInfo,
                                 sal_Bool bTop=sal_False );


    void ExportFmt( const SwFmt& rFmt,  enum ::xmloff::token::XMLTokenEnum eClass = ::xmloff::token::XML_TOKEN_INVALID );
    void ExportTableFmt( const SwFrmFmt& rFmt, sal_uInt32 nAbsWidth );

    void ExportTableColumnStyle( const SwXMLTableColumn_Impl& rCol );
    void ExportTableBox( const SwTableBox& rBox, sal_uInt16 nColSpan, sal_uInt16 nRowSpan,
                         SwXMLTableInfo_Impl& rTblInfo );
    void ExportTableLine( const SwTableLine& rLine,
                          const SwXMLTableLines_Impl& rLines,
                          SwXMLTableInfo_Impl& rTblInfo );
    void ExportTableLines( const SwTableLines& rLines,
                           SwXMLTableInfo_Impl& rTblInfo,
                           sal_uInt16 nHeaderRows = 0 );

    virtual void _ExportMeta();
    virtual void _ExportFontDecls();
    virtual void _ExportStyles( sal_Bool bUsed );
    virtual void _ExportAutoStyles();
    virtual void _ExportMasterStyles();
    virtual void SetBodyAttributes();
    virtual void _ExportContent();
    virtual void GetViewSettings(com::sun::star::uno::Sequence<com::sun::star::beans::PropertyValue>& aProps);
    virtual void GetConfigurationSettings(com::sun::star::uno::Sequence<com::sun::star::beans::PropertyValue>& aProps);
    virtual sal_Int32 GetDocumentSpecificSettings( ::std::list< SettingsGroup >& _out_rSettings );

#ifdef XML_CORE_API
    void SetCurPaM( SwPaM& rPaM, sal_Bool bWhole, sal_Bool bTabOnly );
#endif

    // string constants for table cell export
    const ::rtl::OUString sNumberFormat;
    const ::rtl::OUString sIsProtected;
    const ::rtl::OUString sCell;

    void setBlockMode();

protected:

    virtual XMLTextParagraphExport* CreateTextParagraphExport();
    virtual SvXMLAutoStylePoolP* CreateAutoStylePool();
    virtual XMLPageExport* CreatePageExport();
    virtual XMLShapeExport* CreateShapeExport();
    virtual XMLFontAutoStylePool* CreateFontAutoStylePool();

public:

    // #110680#
    SwXMLExport(
        const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory > xServiceFactory,
        sal_uInt16 nExportFlags = EXPORT_ALL);

#ifdef XML_CORE_API
    // #110680#
    SwXMLExport(
        const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory > xServiceFactory,
        const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XModel > & rModel,
        SwPaM& rPaM,
        const ::rtl::OUString& rFileName,
        const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XDocumentHandler > & rHandler,
        const ::com::sun::star::uno::Reference< ::com::sun::star::document::XGraphicObjectResolver > &,
        sal_Bool bExpWholeDoc,
        sal_Bool bExpFirstTableOnly,
        sal_Bool bShowProgr );
#endif
    virtual ~SwXMLExport();

    virtual sal_uInt32 exportDoc( enum ::xmloff::token::XMLTokenEnum eClass = ::xmloff::token::XML_TOKEN_INVALID );

    inline const SvXMLUnitConverter& GetTwipUnitConverter() const;

    void ExportTableAutoStyles( const SwTableNode& rTblNd );
    void ExportTable( const SwTableNode& rTblNd );

    SvXMLExportItemMapper& GetTableItemMapper() { return *pTableItemMapper; }
    const UniReference < XMLPropertySetMapper >& GetParaPropMapper()
    {
        return xParaPropMapper;
    }

    sal_Bool IsShowProgress() const { return bShowProgress; }
    void SetShowProgress( sal_Bool b ) { bShowProgress = b; }
    sal_Bool IsBlockMode() const { return bBlock; }

    // XUnoTunnel
    static const ::com::sun::star::uno::Sequence< sal_Int8 > & getUnoTunnelId() throw();
    virtual sal_Int64 SAL_CALL getSomething( const ::com::sun::star::uno::Sequence< sal_Int8 >& aIdentifier ) throw(::com::sun::star::uno::RuntimeException);

    // XServiceInfo (override parent method)
    ::rtl::OUString SAL_CALL getImplementationName()
        throw( ::com::sun::star::uno::RuntimeException );
};

inline const SvXMLUnitConverter& SwXMLExport::GetTwipUnitConverter() const
{
    return *pTwipUnitConv;
}


#endif  //  _XMLEXP_HXX

