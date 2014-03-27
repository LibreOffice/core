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

#include <com/sun/star/document/XDocumentProperties.hpp>

#include <sot/storage.hxx>

#include <xmloff/xmlictxt.hxx>
#include <xmloff/xmlimp.hxx>

#include "xmlitmap.hxx"

class SwDoc;
class SvXMLUnitConverter;
class SvXMLTokenMap;
class SvXMLImportItemMapper;
class SfxItemSet;
class SwNodeIndex;
class XMLTextImportHelper;
class SvXMLGraphicHelper;
class SvXMLEmbeddedObjectHelper;

// define, how many steps ( = paragraphs ) the progress bar should advance
// for styles, autostyles and settings + meta
#define PROGRESS_BAR_STEP 20

namespace SwImport {
    SwDoc* GetDocFromXMLImport( SvXMLImport& );
}

class SwXMLImport: public SvXMLImport
{
    SwNodeIndex             *pSttNdIdx;

    SvXMLUnitConverter      *pTwipUnitConv;
    SvXMLImportItemMapper   *pTableItemMapper;// paragraph item import
    SvXMLTokenMap           *pDocElemTokenMap;
    SvXMLTokenMap           *pTableElemTokenMap;
    SvXMLTokenMap           *pTableCellAttrTokenMap;
    SvXMLGraphicHelper      *pGraphicResolver;
    SvXMLEmbeddedObjectHelper   *pEmbeddedResolver;

    SvXMLItemMapEntriesRef  xTableItemMap;
    SvXMLItemMapEntriesRef  xTableColItemMap;
    SvXMLItemMapEntriesRef  xTableRowItemMap;
    SvXMLItemMapEntriesRef  xTableCellItemMap;
    SvStorageRef            xPackage;
    ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameContainer >
                            xLateInitSettings;

    sal_uInt16              nStyleFamilyMask;// Mask of styles to load
    bool                bLoadDoc : 1;   // Load doc or styles only
    bool                bInsert : 1;    // Insert mode. If styles are
                                            // loaded only sal_False means that
                                            // existing styles will be
                                            // overwritten.
    bool                bBlock : 1;     // Load text block
    bool                bShowProgress : 1;
    bool                bOrganizerMode : 1;
    bool                bInititedXForms : 1;
    sal_Bool                bPreserveRedlineMode;

    SwDoc*      doc; // cached for getDoc()

    void                    _InitItemImport();
    void                    _FinitItemImport();
    void                    UpdateTxtCollConditions( SwDoc *pDoc );

    void         setTextInsertMode(
                     const ::com::sun::star::uno::Reference<
                        ::com::sun::star::text::XTextRange > & rInsertPos );
    void         setStyleInsertMode( sal_uInt16 nFamilies,
                                     sal_Bool bOverwrite );
    void         setBlockMode();
    void         setOrganizerMode();

protected:

    // This method is called after the namespace map has been updated, but
    // before a context for the current element has been pushed.
    virtual SvXMLImportContext *CreateContext( sal_uInt16 nPrefix,
                  const OUString& rLocalName,
                  const ::com::sun::star::uno::Reference<
                    ::com::sun::star::xml::sax::XAttributeList > & xAttrList ) SAL_OVERRIDE;

    virtual XMLTextImportHelper* CreateTextImport() SAL_OVERRIDE;

    virtual XMLShapeImportHelper* CreateShapeImport() SAL_OVERRIDE;

public:
    SwXMLImport(
        const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext > xContext,
        OUString const & implementationName, sal_uInt16 nImportFlags);

    ~SwXMLImport() throw();

    // ::com::sun::star::xml::sax::XDocumentHandler
    virtual void SAL_CALL startDocument()
        throw (::com::sun::star::xml::sax::SAXException,
               ::com::sun::star::uno::RuntimeException,
               std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL endDocument(void)
        throw( ::com::sun::star::xml::sax::SAXException, ::com::sun::star::uno::RuntimeException, std::exception ) SAL_OVERRIDE;

    // XUnoTunnel
    static const ::com::sun::star::uno::Sequence< sal_Int8 > & getUnoTunnelId() throw();
    virtual sal_Int64 SAL_CALL getSomething( const ::com::sun::star::uno::Sequence< sal_Int8 >& aIdentifier ) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    // XInitialization
    virtual void SAL_CALL initialize( const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any >& aArguments ) throw(::com::sun::star::uno::Exception, ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    void                    InsertStyles( sal_Bool bAuto );
    void                    FinishStyles();

    // namespace office

    // NB: in contrast to other CreateFooContexts, this particular one handles
    //     the root element (i.e. office:document-meta)
    SvXMLImportContext *CreateMetaContext( const OUString& rLocalName );
    SvXMLImportContext *CreateScriptContext( const OUString& rLocalName );
    SvXMLImportContext *CreateStylesContext(
                const OUString& rLocalName,
                const ::com::sun::star::uno::Reference<
                    ::com::sun::star::xml::sax::XAttributeList > & xAttrList,
                sal_Bool bAuto );
    SvXMLImportContext *CreateMasterStylesContext(
                const OUString& rLocalName,
                const ::com::sun::star::uno::Reference<
                    ::com::sun::star::xml::sax::XAttributeList > & xAttrList );
    SvXMLImportContext *CreateFontDeclsContext(
            const OUString& rLocalName,
            const ::com::sun::star::uno::Reference<
                    ::com::sun::star::xml::sax::XAttributeList > & xAttrList );
    SvXMLImportContext *CreateBodyContentContext( const OUString& rLocalName );
    sal_uInt16 GetStyleFamilyMask() const { return nStyleFamilyMask; }
    bool IsInsertMode() const { return bInsert; }
    bool IsStylesOnlyMode() const { return !bLoadDoc; }
    bool IsBlockMode() const { return bBlock; }
    bool IsOrganizerMode() const { return bOrganizerMode; }

    inline const SvXMLUnitConverter& GetTwipUnitConverter() const;
    inline const SvXMLImportItemMapper& GetTableItemMapper() const;
    inline       SvXMLImportItemMapper& GetTableItemMapper();
    SvXMLImportContext *CreateTableItemImportContext( sal_uInt16 nPrefix,
                const OUString& rLocalName,
                const ::com::sun::star::uno::Reference<
                    ::com::sun::star::xml::sax::XAttributeList > & xAttrList,
                sal_uInt16 nSubFamily, SfxItemSet& rItemSet );

    const SvXMLTokenMap& GetDocElemTokenMap();
    const SvXMLTokenMap& GetTableElemTokenMap();
    const SvXMLTokenMap& GetTableCellAttrTokenMap();

    bool FindAutomaticStyle( sal_uInt16 nFamily,
                             const OUString& rName,
                             const SfxItemSet **ppItemSet=0,
                             OUString *pParent=0 ) const;

    virtual void SetStatistics(
        const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::NamedValue> & i_rStats) SAL_OVERRIDE;
    virtual void SetViewSettings(const com::sun::star::uno::Sequence<com::sun::star::beans::PropertyValue>& aViewProps) SAL_OVERRIDE;
    virtual void SetConfigurationSettings(const com::sun::star::uno::Sequence<com::sun::star::beans::PropertyValue>& aConfigProps) SAL_OVERRIDE;
    virtual void SetDocumentSpecificSettings(const OUString& _rSettingsGroupName,
                    const com::sun::star::uno::Sequence<com::sun::star::beans::PropertyValue>& _rSettings) SAL_OVERRIDE;

    SvStorage *GetPackage() { return &xPackage; }

    void SetProgressValue( sal_Int32 nValue );

    // initialize XForms
    virtual void initXForms() SAL_OVERRIDE;

    // get the document properties, but only if they actually need importing
    ::com::sun::star::uno::Reference<
        ::com::sun::star::document::XDocumentProperties>
            GetDocumentProperties() const;

    virtual void NotifyEmbeddedFontRead() SAL_OVERRIDE;

    const SwDoc* getDoc() const;
    SwDoc* getDoc();
};

inline const SvXMLUnitConverter& SwXMLImport::GetTwipUnitConverter() const
{
    return *pTwipUnitConv;
}

inline const SvXMLImportItemMapper& SwXMLImport::GetTableItemMapper() const
{
    return *pTableItemMapper;
}

inline       SvXMLImportItemMapper& SwXMLImport::GetTableItemMapper()
{
    return *pTableItemMapper;
}

inline void SwXMLImport::SetProgressValue( sal_Int32 nValue )
{
    if ( bShowProgress )
        GetProgressBarHelper()->SetValue(nValue);
}

#endif  //  _XMLIMP_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
