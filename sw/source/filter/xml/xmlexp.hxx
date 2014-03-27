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

#ifndef INCLUDED_SW_SOURCE_FILTER_XML_XMLEXP_HXX
#define INCLUDED_SW_SOURCE_FILTER_XML_XMLEXP_HXX

#include <xmloff/xmlexp.hxx>
#include "xmlitmap.hxx"
#include <xmloff/uniref.hxx>
#include <xmloff/xmltoken.hxx>
#include <vector>

class SwDoc;
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
class SwXMLTableColumnsSortByWidth_Impl;
class SwXMLTableFrmFmtsSort_Impl;
class SwXMLTableInfo_Impl;
class SwTableNode;
class XMLPropertySetMapper;
class SwXMLTableLines_Impl;

typedef ::std::vector< SwXMLTableLines_Impl* > SwXMLTableLinesCache_Impl;

class SwXMLExport : public SvXMLExport
{
    SvXMLUnitConverter*         pTwipUnitConv;
    SvXMLExportItemMapper*      pTableItemMapper;
    SwXMLTableLinesCache_Impl*  pTableLines;

    SvXMLItemMapEntriesRef      xTableItemMap;
    SvXMLItemMapEntriesRef      xTableRowItemMap;
    SvXMLItemMapEntriesRef      xTableCellItemMap;
    UniReference < XMLPropertySetMapper > xParaPropMapper;

    sal_Bool                    bBlock : 1;         // export text block?
    sal_Bool                    bShowProgress : 1;
    sal_Bool                    bSavedShowChanges : 1;

    SwDoc*                      doc; // cached for getDoc()

    void _InitItemExport();
    void _FinitItemExport();
    void ExportTableLinesAutoStyles( const SwTableLines& rLines,
                                 sal_uInt32 nAbsWidth,
                                 sal_uInt32 nBaseWidth,
                                 const OUString& rNamePrefix,
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

    virtual void _ExportMeta() SAL_OVERRIDE;
    virtual void _ExportFontDecls() SAL_OVERRIDE;
    virtual void _ExportStyles( bool bUsed ) SAL_OVERRIDE;
    virtual void _ExportAutoStyles() SAL_OVERRIDE;
    virtual void _ExportMasterStyles() SAL_OVERRIDE;
    virtual void SetBodyAttributes() SAL_OVERRIDE;
    virtual void _ExportContent() SAL_OVERRIDE;
    virtual void GetViewSettings(com::sun::star::uno::Sequence<com::sun::star::beans::PropertyValue>& aProps) SAL_OVERRIDE;
    virtual void GetConfigurationSettings(com::sun::star::uno::Sequence<com::sun::star::beans::PropertyValue>& aProps) SAL_OVERRIDE;
    virtual sal_Int32 GetDocumentSpecificSettings( ::std::list< SettingsGroup >& _out_rSettings ) SAL_OVERRIDE;

    // string constants for table cell export
    const OUString sNumberFormat;
    const OUString sIsProtected;
    const OUString sCell;

    void setBlockMode();
private:
    void DeleteTableLines();
protected:

    virtual XMLTextParagraphExport* CreateTextParagraphExport() SAL_OVERRIDE;
    virtual SvXMLAutoStylePoolP* CreateAutoStylePool() SAL_OVERRIDE;
    virtual XMLPageExport* CreatePageExport() SAL_OVERRIDE;
    virtual XMLShapeExport* CreateShapeExport() SAL_OVERRIDE;
    virtual XMLFontAutoStylePool* CreateFontAutoStylePool() SAL_OVERRIDE;

public:
    SwXMLExport(
        const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext > xContext,
        OUString const & implementationName, sal_uInt16 nExportFlags);

    virtual ~SwXMLExport();

    virtual sal_uInt32 exportDoc( enum ::xmloff::token::XMLTokenEnum eClass = ::xmloff::token::XML_TOKEN_INVALID ) SAL_OVERRIDE;

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
    virtual sal_Int64 SAL_CALL getSomething( const ::com::sun::star::uno::Sequence< sal_Int8 >& aIdentifier ) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    const SwDoc* getDoc() const;
    SwDoc* getDoc();
};

inline const SvXMLUnitConverter& SwXMLExport::GetTwipUnitConverter() const
{
    return *pTwipUnitConv;
}

#endif // INCLUDED_SW_SOURCE_FILTER_XML_XMLEXP_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
