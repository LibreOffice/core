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

#ifndef INCLUDED_SW_SOURCE_FILTER_XML_XMLIMP_HXX
#define INCLUDED_SW_SOURCE_FILTER_XML_XMLIMP_HXX

#include <sal/config.h>

#include <memory>

#include <com/sun/star/document/XDocumentProperties.hpp>

#include <sot/storage.hxx>

#include <xmloff/xmlictxt.hxx>
#include <xmloff/xmlimp.hxx>

#include "xmlitmap.hxx"
#include <o3tl/typed_flags_set.hxx>

class SwDoc;
class SvXMLUnitConverter;
class SvXMLTokenMap;
class SvXMLImportItemMapper;
class SfxItemSet;
class SwNodeIndex;
class XMLTextImportHelper;
class SvXMLGraphicHelper;
class SvXMLEmbeddedObjectHelper;
enum class SfxStyleFamily;

// define, how many steps ( = paragraphs ) the progress bar should advance
// for styles, autostyles and settings + meta
#define PROGRESS_BAR_STEP 20

namespace SwImport {
    SwDoc* GetDocFromXMLImport( SvXMLImport const & );
}

// we only need this scoped enum to be flags here, in sw
namespace o3tl
{
    template<> struct typed_flags<SfxStyleFamily> : is_typed_flags<SfxStyleFamily, 0xffff> {};
}

class SwXMLImport: public SvXMLImport
{
    std::unique_ptr<SwNodeIndex> m_pSttNdIdx;

    std::unique_ptr<SvXMLUnitConverter> m_pTwipUnitConv;
    std::unique_ptr<SvXMLImportItemMapper> m_pTableItemMapper;// paragraph item import
    std::unique_ptr<SvXMLTokenMap> m_pDocElemTokenMap;
    std::unique_ptr<SvXMLTokenMap> m_pTableElemTokenMap;
    std::unique_ptr<SvXMLTokenMap> m_pTableCellAttrTokenMap;

    rtl::Reference<SvXMLGraphicHelper> m_xGraphicStorageHandler;

    rtl::Reference<SvXMLEmbeddedObjectHelper> m_xEmbeddedResolver;

    SvXMLItemMapEntriesRef  m_xTableItemMap;
    SvXMLItemMapEntriesRef  m_xTableColItemMap;
    SvXMLItemMapEntriesRef  m_xTableRowItemMap;
    SvXMLItemMapEntriesRef  m_xTableCellItemMap;
    css::uno::Reference< css::container::XNameContainer >
                            m_xLateInitSettings;

    SfxStyleFamily      m_nStyleFamilyMask;// Mask of styles to load
    bool                m_bLoadDoc : 1;   // Load doc or styles only
    bool                m_bInsert : 1;    // Insert mode. If styles are
                                            // loaded only false means that
                                            // existing styles will be
                                            // overwritten.
    bool                m_bBlock : 1;     // Load text block
    bool                m_bOrganizerMode : 1;
    bool                m_bInititedXForms : 1;

    SwDoc*      m_pDoc; // cached for getDoc()

    void                    InitItemImport();
    void                    FinitItemImport();
    void                    UpdateTextCollConditions( SwDoc *pDoc );

    void         setTextInsertMode(
                     const css::uno::Reference< css::text::XTextRange > & rInsertPos );
    void         setStyleInsertMode( SfxStyleFamily nFamilies,
                                     bool bOverwrite );

protected:

    virtual SvXMLImportContext *CreateDocumentContext( sal_uInt16 nPrefix,
                  const OUString& rLocalName,
                  const css::uno::Reference< css::xml::sax::XAttributeList > & xAttrList ) override;

    virtual SvXMLImportContext *CreateFastContext( sal_Int32 nElement,
        const ::css::uno::Reference< ::css::xml::sax::XFastAttributeList >& xAttrList ) override;

    virtual XMLTextImportHelper* CreateTextImport() override;

    virtual XMLShapeImportHelper* CreateShapeImport() override;

public:
    SwXMLImport(
        const css::uno::Reference< css::uno::XComponentContext >& rContext,
        OUString const & implementationName, SvXMLImportFlags nImportFlags);

    virtual ~SwXMLImport() throw() override;

    // css::xml::sax::XDocumentHandler
    virtual void SAL_CALL startDocument() override;
    virtual void SAL_CALL endDocument() override;

    // XUnoTunnel
    static const css::uno::Sequence< sal_Int8 > & getUnoTunnelId() throw();
    virtual sal_Int64 SAL_CALL getSomething( const css::uno::Sequence< sal_Int8 >& aIdentifier ) override;

    // XInitialization
    virtual void SAL_CALL initialize( const css::uno::Sequence< css::uno::Any >& aArguments ) override;

    void                    InsertStyles( bool bAuto );
    void                    FinishStyles();

    // namespace office

    // NB: in contrast to other CreateFooContexts, this particular one handles
    //     the root element (i.e. office:document-meta)
    SvXMLImportContext *CreateMetaContext( const sal_Int32 nElement );
    SvXMLImportContext *CreateScriptContext( const OUString& rLocalName );
    SvXMLImportContext *CreateStylesContext(
                const OUString& rLocalName,
                const css::uno::Reference< css::xml::sax::XAttributeList > & xAttrList,
                bool bAuto );
    SvXMLImportContext *CreateMasterStylesContext(
                const OUString& rLocalName,
                const css::uno::Reference< css::xml::sax::XAttributeList > & xAttrList );
    SvXMLImportContext *CreateFontDeclsContext(
            const OUString& rLocalName,
            const css::uno::Reference< css::xml::sax::XAttributeList > & xAttrList );
    SvXMLImportContext *CreateBodyContentContext( const OUString& rLocalName );
    SfxStyleFamily GetStyleFamilyMask() const { return m_nStyleFamilyMask; }
    bool IsInsertMode() const { return m_bInsert; }
    bool IsStylesOnlyMode() const { return !m_bLoadDoc; }
    bool IsBlockMode() const { return m_bBlock; }

    inline const SvXMLImportItemMapper& GetTableItemMapper() const;
    inline       SvXMLImportItemMapper& GetTableItemMapper();
    SvXMLImportContext *CreateTableItemImportContext( sal_uInt16 nPrefix,
                const OUString& rLocalName,
                const css::uno::Reference< css::xml::sax::XAttributeList > & xAttrList,
                sal_uInt16 nSubFamily, SfxItemSet& rItemSet );

    const SvXMLTokenMap& GetDocElemTokenMap();
    const SvXMLTokenMap& GetTableElemTokenMap();
    const SvXMLTokenMap& GetTableCellAttrTokenMap();

    bool FindAutomaticStyle( sal_uInt16 nFamily,
                             const OUString& rName,
                             const SfxItemSet **ppItemSet ) const;
    void MergeListsAtDocumentInsertPosition(SwDoc *pDoc);

    virtual void SetStatistics(
        const css::uno::Sequence< css::beans::NamedValue> & i_rStats) override;
    virtual void SetViewSettings(const css::uno::Sequence<css::beans::PropertyValue>& aViewProps) override;
    virtual void SetConfigurationSettings(const css::uno::Sequence<css::beans::PropertyValue>& aConfigProps) override;
    virtual void SetDocumentSpecificSettings(const OUString& _rSettingsGroupName,
                    const css::uno::Sequence<css::beans::PropertyValue>& _rSettings) override;

    // initialize XForms
    virtual void initXForms() override;

    // get the document properties, but only if they actually need importing
    css::uno::Reference<css::document::XDocumentProperties>
            GetDocumentProperties() const;

    virtual void NotifyEmbeddedFontRead() override;

    const SwDoc* getDoc() const;
    SwDoc* getDoc();
};

inline const SvXMLImportItemMapper& SwXMLImport::GetTableItemMapper() const
{
    return *m_pTableItemMapper;
}

inline       SvXMLImportItemMapper& SwXMLImport::GetTableItemMapper()
{
    return *m_pTableItemMapper;
}

#endif  //  _XMLIMP_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
