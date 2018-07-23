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
#include <xmloff/xmltoken.hxx>
#include <vector>

class SwDoc;
class SwFormat;
class SwFrameFormat;
class SvXMLUnitConverter;
class SvXMLExportItemMapper;
class SvXMLAutoStylePoolP;
class SwTableLine;
class SwTableLines;
class SwTableBox;
class SwXMLTableColumn_Impl;
class SwXMLTableLines_Impl;
class SwXMLTableColumnsSortByWidth_Impl;
class SwXMLTableFrameFormatsSort_Impl;
class SwXMLTableInfo_Impl;
class SwTableNode;
class XMLPropertySetMapper;
class SwXMLTableLines_Impl;

typedef std::vector< SwXMLTableLines_Impl* > SwXMLTableLinesCache_Impl;

class SwXMLExport : public SvXMLExport
{
    std::unique_ptr<SvXMLUnitConverter>    m_pTwipUnitConverter;
    std::unique_ptr<SvXMLExportItemMapper> m_pTableItemMapper;
    std::unique_ptr<SwXMLTableLinesCache_Impl> m_pTableLines;

    SvXMLItemMapEntriesRef      m_xTableItemMap;
    SvXMLItemMapEntriesRef      m_xTableRowItemMap;
    SvXMLItemMapEntriesRef      m_xTableCellItemMap;

    bool                    m_bBlock : 1;         // export text block?
    bool                    m_bShowProgress : 1;
    bool                    m_bSavedShowChanges : 1;

    SwDoc*                      m_pDoc; // cached for getDoc()

    void InitItemExport();
    void FinitItemExport();
    void ExportTableLinesAutoStyles( const SwTableLines& rLines,
                                 sal_uInt32 nAbsWidth,
                                 sal_uInt32 nBaseWidth,
                                 const OUString& rNamePrefix,
                                 SwXMLTableColumnsSortByWidth_Impl& rExpCols,
                                 SwXMLTableFrameFormatsSort_Impl& rExpRows,
                                 SwXMLTableFrameFormatsSort_Impl& rExpCells,
                                 SwXMLTableInfo_Impl& rTableInfo,
                                 bool bTop=false );

    void ExportFormat( const SwFormat& rFormat,  enum ::xmloff::token::XMLTokenEnum eClass );
    void ExportTableFormat( const SwFrameFormat& rFormat, sal_uInt32 nAbsWidth );

    void ExportTableColumnStyle( const SwXMLTableColumn_Impl& rCol );
    void ExportTableBox( const SwTableBox& rBox, sal_uInt32 nColSpan, sal_uInt32 nRowSpan,
                         SwXMLTableInfo_Impl& rTableInfo );
    void ExportTableLine( const SwTableLine& rLine,
                          const SwXMLTableLines_Impl& rLines,
                          SwXMLTableInfo_Impl& rTableInfo );
    void ExportTableLines( const SwTableLines& rLines,
                           SwXMLTableInfo_Impl& rTableInfo,
                           sal_uInt32 nHeaderRows = 0 );

    virtual void ExportMeta_() override;
    virtual void ExportFontDecls_() override;
    virtual void ExportStyles_( bool bUsed ) override;
    virtual void ExportAutoStyles_() override;
    virtual void ExportMasterStyles_() override;
    virtual void SetBodyAttributes() override;
    virtual void ExportContent_() override;
    virtual void GetViewSettings(css::uno::Sequence<css::beans::PropertyValue>& aProps) override;
    virtual void GetConfigurationSettings(css::uno::Sequence<css::beans::PropertyValue>& aProps) override;
    virtual sal_Int32 GetDocumentSpecificSettings( std::vector< SettingsGroup >& _out_rSettings ) override;

private:
    void DeleteTableLines();
protected:

    virtual XMLTextParagraphExport* CreateTextParagraphExport() override;
    virtual SvXMLAutoStylePoolP* CreateAutoStylePool() override;
    virtual XMLPageExport* CreatePageExport() override;
    virtual XMLShapeExport* CreateShapeExport() override;
    virtual XMLFontAutoStylePool* CreateFontAutoStylePool() override;

public:
    SwXMLExport(
        const css::uno::Reference< css::uno::XComponentContext >& rContext,
        OUString const & implementationName, SvXMLExportFlags nExportFlags);

    virtual ~SwXMLExport() override;

    void collectAutoStyles() override;

    virtual ErrCode exportDoc( enum ::xmloff::token::XMLTokenEnum eClass = ::xmloff::token::XML_TOKEN_INVALID ) override;

    inline const SvXMLUnitConverter& GetTwipUnitConverter() const;

    void ExportTableAutoStyles( const SwTableNode& rTableNd );
    void ExportTable( const SwTableNode& rTableNd );

    bool IsShowProgress() const { return m_bShowProgress; }
    void SetShowProgress( bool b ) { m_bShowProgress = b; }

    // XUnoTunnel
    static const css::uno::Sequence< sal_Int8 > & getUnoTunnelId() throw();
    virtual sal_Int64 SAL_CALL getSomething( const css::uno::Sequence< sal_Int8 >& aIdentifier ) override;

    const SwDoc* getDoc() const;
    SwDoc* getDoc();
};

inline const SvXMLUnitConverter& SwXMLExport::GetTwipUnitConverter() const
{
    return *m_pTwipUnitConverter;
}

#endif // INCLUDED_SW_SOURCE_FILTER_XML_XMLEXP_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
