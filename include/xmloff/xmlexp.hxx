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

#ifndef INCLUDED_XMLOFF_XMLEXP_HXX
#define INCLUDED_XMLOFF_XMLEXP_HXX

#include <sal/config.h>
#include <xmloff/dllapi.h>
#include <sal/types.h>

#include <rtl/ustring.hxx>
#include <xmloff/txtparae.hxx>
#include <xmloff/formlayerexport.hxx>
#include <xmloff/xmlnumfe.hxx>
#include <xmloff/xmlaustp.hxx>
#include <xmloff/shapeexport.hxx>
#include <xmloff/xmltoken.hxx>
#include <xmloff/SchXMLExportHelper.hxx>
#include <xmloff/XMLFontAutoStylePool.hxx>
#include <xmloff/xmluconv.hxx>
#include <com/sun/star/document/XFilter.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/document/XExporter.hpp>
#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/container/XNamed.hpp>

#include <unotools/saveopt.hxx>
#include <unotools/securityoptions.hxx>

#include <xmloff/XMLPageExport.hxx>
#include <comphelper/attributelist.hxx>
#include <cppuhelper/implbase.hxx>
#include <tools/fldunit.hxx>
#include <comphelper/errcode.hxx>

#include <vector>
#include <memory>
#include <o3tl/typed_flags_set.hxx>

namespace com::sun::star::beans { class XPropertySet; }
namespace com::sun::star::document { class XEmbeddedObjectResolver; }
namespace com::sun::star::document { class XGraphicStorageHandler; }
namespace com::sun::star::embed { class XStorage; }
namespace com::sun::star::graphic { class XGraphic; }
namespace com::sun::star::lang { class XEventListener; }
namespace com::sun::star::task { class XStatusIndicator; }
namespace com::sun::star::uno { class XComponentContext; }
namespace com::sun::star::util { class XNumberFormatsSupplier; }
namespace com::sun::star::xml::sax { class XAttributeList; }
namespace com::sun::star::xml::sax { class XDocumentHandler; }
namespace com::sun::star::xml::sax { class XExtendedDocumentHandler; }
namespace com::sun::star::xml::sax { class XLocator; }

class SvXMLNamespaceMap;
class SvtSecurityMapPersonalInfo;
class SvXMLExport_Impl;
class ProgressBarHelper;
class XMLEventExport;
class XMLImageMapExport;
class XMLErrors;
class LanguageTag;
enum class SvXMLErrorFlags;

// Shapes in Writer cannot be named via context menu (#i51726#)
#include <unotools/moduleoptions.hxx>

namespace com::sun::star {
    namespace frame { class XModel; }
    namespace lang { struct Locale; }
}
namespace comphelper { class UnoInterfaceToUniqueIdentifierMapper; }

namespace model { class Theme; }

enum class SvXMLExportFlags {
    NONE                     = 0,
    META                     = 0x0001,
    STYLES                   = 0x0002,
    MASTERSTYLES             = 0x0004,
    AUTOSTYLES               = 0x0008,
    CONTENT                  = 0x0010,
    SCRIPTS                  = 0x0020,
    SETTINGS                 = 0x0040,
    FONTDECLS                = 0x0080,
    EMBEDDED                 = 0x0100,
    PRETTY                   = 0x0400,
    OASIS                    = 0x8000,
    ALL                      = 0x05ff
};
namespace o3tl
{
    template<> struct typed_flags<SvXMLExportFlags> : is_typed_flags<SvXMLExportFlags, 0x85ff> {};
}

class XMLOFF_DLLPUBLIC SvXMLExport : public cppu::WeakImplHelper<
             css::document::XFilter,
             css::lang::XServiceInfo,
             css::document::XExporter,
             css::lang::XInitialization,
             css::container::XNamed>
{
    std::unique_ptr<SvXMLExport_Impl>            mpImpl;            // dummy

    css::uno::Reference< css::uno::XComponentContext > m_xContext;
    OUString m_implementationName;

    css::uno::Reference< css::frame::XModel > mxModel;
    css::uno::Reference< css::xml::sax::XDocumentHandler >            mxHandler;      // the handlers
    css::uno::Reference< css::xml::sax::XExtendedDocumentHandler >    mxExtHandler;
    css::uno::Reference< css::util::XNumberFormatsSupplier > mxNumberFormatsSupplier;
    css::uno::Reference< css::document::XGraphicStorageHandler > mxGraphicStorageHandler;
    css::uno::Reference< css::document::XEmbeddedObjectResolver > mxEmbeddedResolver;
    css::uno::Reference< css::task::XStatusIndicator > mxStatusIndicator;
    css::uno::Reference< css::beans::XPropertySet > mxExportInfo;
    css::uno::Reference< css::lang::XEventListener > mxEventListener;

    rtl::Reference<comphelper::AttributeList> mxAttrList;        // a common attribute list

    OUString     msOrigFileName; // the original URL
    OUString     msFilterName;
    OUString     msImgFilterName;
    std::unique_ptr<SvXMLNamespaceMap> mpNamespaceMap;    // the namespace map
    std::unique_ptr<SvtSecurityMapPersonalInfo> mpAuthorIDs; // map authors to remove personal info
    SvXMLUnitConverter          maUnitConv;        // the unit converter
    std::unique_ptr<SvXMLNumFmtExport> mpNumExport;
    std::unique_ptr<ProgressBarHelper> mpProgressBarHelper;

    rtl::Reference< XMLTextParagraphExport > mxTextParagraphExport;
    rtl::Reference< XMLShapeExport > mxShapeExport;
    rtl::Reference< SvXMLAutoStylePoolP > mxAutoStylePool;
    rtl::Reference< SchXMLExportHelper > mxChartExport;
    rtl::Reference< XMLPageExport > mxPageExport;
    rtl::Reference< XMLFontAutoStylePool > mxFontAutoStylePool;
    rtl::Reference< xmloff::OFormLayerXMLExport > mxFormExport;
    std::unique_ptr<XMLEventExport> mpEventExport;
    std::unique_ptr<XMLImageMapExport> mpImageMapExport;
    std::unique_ptr<XMLErrors>  mpXMLErrors;

    const enum ::xmloff::token::XMLTokenEnum meClass;
    SAL_DLLPRIVATE void InitCtor_();

    SvXMLExportFlags  mnExportFlags;
    SvXMLErrorFlags   mnErrorFlags;

    const OUString               msWS;           // " "

    // Shapes in Writer cannot be named via context menu (#i51726#)
    SvtModuleOptions::EFactory meModelType;
    SAL_DLLPRIVATE void DetermineModelType_();

    SAL_DLLPRIVATE void ImplExportMeta(); // <office:meta>
    SAL_DLLPRIVATE void ImplExportSettings(); // <office:settings>
    SAL_DLLPRIVATE void ImplExportStyles(); // <office:styles>
    SAL_DLLPRIVATE void ImplExportAutoStyles();
        // <office:automatic-styles>
    SAL_DLLPRIVATE void ImplExportMasterStyles();
        // <office:master-styles>
    SAL_DLLPRIVATE void ImplExportContent(); // <office:body>
    virtual void SetBodyAttributes();
    void GetViewSettingsAndViews(css::uno::Sequence<css::beans::PropertyValue>& rProps);

protected:
    void setExportFlags( SvXMLExportFlags nExportFlags ) { mnExportFlags = nExportFlags; }

    // Get (modifiable) namespace map
    SvXMLNamespaceMap& GetNamespaceMap_() { return *mpNamespaceMap; }

    // get a new namespace map (used in starmath to have a default namespace)
    void ResetNamespaceMap();

    /// Override this method to export the content of <office:meta>.
    /// There is a default implementation.
    virtual void ExportMeta_();

    /// Override this method to export the content of <office:scripts>.
    /// There is a default implementation.
    virtual void ExportScripts_();

    /// Override this method to export the font declarations
    /// The default implementation will export the contents of the
    /// XMLFontAutoStylePool if it has been created.
    virtual void ExportFontDecls_();

    /// Override this method to export the content of <style:styles>.
    /// If bUsed is set, used styles should be exported only.
    /// Overriding Methods must call this method !
    virtual void ExportStyles_( bool bUsed );

    /// Override this method to export the contents of <style:auto-styles>.
    virtual void ExportAutoStyles_() = 0;

    /// Override this method to export the contents of <style:master-styles>.
    virtual void ExportMasterStyles_() = 0;

    /// Override this method to export the content of <office:body>.
    virtual void ExportContent_() = 0;

    OUString const & GetSourceShellID() const;
    OUString const & GetDestinationShellID() const;

    // save linked sections? (may be false in global documents)
    bool mbSaveLinkedSections;

    virtual XMLTextParagraphExport* CreateTextParagraphExport();
    virtual XMLShapeExport* CreateShapeExport();
    virtual SvXMLAutoStylePoolP* CreateAutoStylePool();
    SchXMLExportHelper* CreateChartExport();
    virtual XMLPageExport* CreatePageExport();
    virtual XMLFontAutoStylePool* CreateFontAutoStylePool();
    xmloff::OFormLayerXMLExport* CreateFormExport();
    virtual void GetViewSettings(css::uno::Sequence<css::beans::PropertyValue>& aProps);
    virtual void GetConfigurationSettings(css::uno::Sequence<css::beans::PropertyValue>& aProps);

    struct SettingsGroup
    {
        ::xmloff::token::XMLTokenEnum                     eGroupName;
        css::uno::Sequence< css::beans::PropertyValue >   aSettings;

        SettingsGroup(
                const ::xmloff::token::XMLTokenEnum _eGroupName,
                const css::uno::Sequence< css::beans::PropertyValue >& _rSettings )
            :eGroupName( _eGroupName )
            ,aSettings( _rSettings )
        {
        }
    };
    /** returns the current document settings

        The default implementation will obtain the view settings by calling GetViewSettingsAndViews, and the
        configuration settings by calling GetConfigurationSettings, and return them together with the proper XML token.

        @return
            the accumulated count of all settings in all groups
    */
    virtual sal_Int32 GetDocumentSpecificSettings( ::std::vector< SettingsGroup >& _out_rSettings );

    const css::uno::Reference< css::document::XEmbeddedObjectResolver >& GetEmbeddedResolver() const { return mxEmbeddedResolver; }
    inline void SetEmbeddedResolver( css::uno::Reference< css::document::XEmbeddedObjectResolver > const & _xEmbeddedResolver );

    const css::uno::Reference<css::document::XGraphicStorageHandler> & GetGraphicStorageHandler() const
    {
        return mxGraphicStorageHandler;
    }
    void SetGraphicStorageHandler(css::uno::Reference<css::document::XGraphicStorageHandler> const & rxGraphicStorageHandler);

    void SetDocHandler( const css::uno::Reference< css::xml::sax::XDocumentHandler > &rHandler );

    void ExportThemeElement(std::shared_ptr<model::Theme> const& pTheme);

    bool mbAutoStylesCollected;
public:

    SvXMLExport(
        const css::uno::Reference< css::uno::XComponentContext >& xContext,
        OUString implementationName,
        sal_Int16 const eDefaultMeasureUnit /*css::util::MeasureUnit*/,
        const enum ::xmloff::token::XMLTokenEnum eClass,
        SvXMLExportFlags nExportFlag );

    SvXMLExport(
        const css::uno::Reference< css::uno::XComponentContext >& xContext,
        OUString implementationName,
        OUString fileName,
        sal_Int16 const eDefaultMeasureUnit /*css::util::MeasureUnit*/,
        const css::uno::Reference< css::xml::sax::XDocumentHandler > & rHandler);

    SvXMLExport(
        const css::uno::Reference< css::uno::XComponentContext >& xContext,
        OUString implementationName,
        OUString fileName,
        const css::uno::Reference< css::xml::sax::XDocumentHandler > & rHandler,
        const css::uno::Reference< css::frame::XModel > &,
        FieldUnit const eDefaultFieldUnit,
        SvXMLExportFlags nExportFlag );

    virtual ~SvXMLExport() override;

    virtual void collectAutoStyles();

    // XExporter
    virtual void SAL_CALL setSourceDocument( const css::uno::Reference< css::lang::XComponent >& xDoc ) override;

    // XFilter
    virtual sal_Bool SAL_CALL filter( const css::uno::Sequence< css::beans::PropertyValue >& aDescriptor ) override;
    virtual void SAL_CALL cancel() override;

    // XInitialization
    virtual void SAL_CALL initialize( const css::uno::Sequence< css::uno::Any >& aArguments ) override;

    // XNamed
    virtual OUString SAL_CALL getName(  ) override final;
    virtual void SAL_CALL setName( const OUString& aName ) override final;

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName(  ) final override;
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) final override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames(  ) final override;

    /** ensures that the given namespace is in scope at the next started
        element.

        <p>If the namespace is not yet declared, the necessary attribute will
        be added, as well.</p>

        @param i_rNamespace         the namespace to be declared

        @returns the actual prefix that the namespace is associated with
      */
    OUString EnsureNamespace(OUString const & i_rNamespace );

    // Check if common attribute list is empty.
#ifndef DBG_UTIL
    void CheckAttrList() { (void) this; /* avoid loplugin:staticmethods */ }
#else
    void CheckAttrList();
#endif

    // Clear common attribute list.
    void ClearAttrList();

    // Add an attribute to the common attribute list.
    void AddAttribute( sal_uInt16 nPrefix, const OUString& rName,
                       const OUString& rValue );
    void AddAttribute( sal_uInt16 nPrefix,
                       enum ::xmloff::token::XMLTokenEnum eName,
                       const OUString& rValue );
    void AddAttribute( sal_uInt16 nPrefix,
                       enum ::xmloff::token::XMLTokenEnum eName,
                       enum ::xmloff::token::XMLTokenEnum eValue );
    void AddAttribute( const OUString& rQName,
                       const OUString& rValue );
    void AddAttribute( const OUString& rQName,
                       enum ::xmloff::token::XMLTokenEnum eValue );

    /** Add language tag attributes, deciding which are necessary.

        @param  nPrefix
                Namespace prefix for *:language, *:script and *:country

        @param  nPrefixRfc
                Namespace prefix for *:rfc-language-tag

        @param  bWriteEmpty
                Whether to write empty *:language and *:country attribute
                values in case of an empty locale (denoting system).
     */
    void AddLanguageTagAttributes( sal_uInt16 nPrefix, sal_uInt16 nPrefixRfc,
            const css::lang::Locale& rLocale, bool bWriteEmpty);

    /** Same as AddLanguageTagAttributes() but with LanguageTag parameter
        instead of Locale.
     */
    void AddLanguageTagAttributes( sal_uInt16 nPrefix, sal_uInt16 nPrefixRfc,
            const LanguageTag& rLanguageTag, bool bWriteEmpty );

    // add several attributes to the common attribute list
    void AddAttributeList( const css::uno::Reference<
                                  css::xml::sax::XAttributeList >& xAttrList );

    // Get common attribute list as implementation or interface.
    comphelper::AttributeList &GetAttrList() { return *mxAttrList; }
    css::uno::Reference< css::xml::sax::XAttributeList > GetXAttrList() const { return mxAttrList; }

    // Get document handler. This methods are not const, because the
    // reference allows modifications through the handler.
    const css::uno::Reference< css::xml::sax::XDocumentHandler > & GetDocHandler() const { return mxHandler; }

    // Get original URL.
    const OUString& GetOrigFileName() const { return msOrigFileName; }

    // Get (const) namespace map.
    const SvXMLNamespaceMap& GetNamespaceMap() const { return *mpNamespaceMap; }

    // Get author id to remove personal info
    size_t GetInfoID( const OUString sPersonalInfo ) const { return mpAuthorIDs->GetInfoID(sPersonalInfo); }

    // Get unit converter
    const SvXMLUnitConverter& GetMM100UnitConverter() const { return maUnitConv; }

    SvXMLUnitConverter& GetMM100UnitConverter() { return maUnitConv; }

    void addChaffWhenEncryptedStorage();

    // Export the document.
    virtual ErrCode exportDoc( enum ::xmloff::token::XMLTokenEnum eClass = ::xmloff::token::XML_TOKEN_INVALID );

    void collectDataStyles(bool bFromUsedStyles);
    virtual void addDataStyle(const sal_Int32 nNumberFormat, bool bTimeFormat = false );
    virtual void exportDataStyles();
    virtual void exportAutoDataStyles();
    virtual OUString getDataStyleName(const sal_Int32 nNumberFormat, bool bTimeFormat = false ) const;
    sal_Int32 dataStyleForceSystemLanguage(sal_Int32 nFormat) const;

    virtual void exportAnnotationMeta( const css::uno::Reference < css::drawing::XShape >& xShape);

    // Get XModel
    const css::uno::Reference< css::frame::XModel > &
               GetModel() const { return mxModel; }
    // Get XNumberFormatsSupplier
    css::uno::Reference< css::util::XNumberFormatsSupplier > & GetNumberFormatsSupplier() { return mxNumberFormatsSupplier; }
    void SetNumberFormatsSupplier(const css::uno::Reference< css::util::XNumberFormatsSupplier >& _xNumberFormatSupplier)
    {
        mxNumberFormatsSupplier = _xNumberFormatSupplier;
        if ( mxNumberFormatsSupplier.is() && mxHandler.is() )
            mpNumExport.reset( new SvXMLNumFmtExport(*this, mxNumberFormatsSupplier) );
    }

    // get export helper for text
    inline rtl::Reference< XMLTextParagraphExport > const & GetTextParagraphExport();

    // get export helper for shapes
    inline rtl::Reference< XMLShapeExport > const & GetShapeExport();

    // get auto style pool
    inline rtl::Reference< SvXMLAutoStylePoolP > const & GetAutoStylePool();

    // get Page Export
    inline rtl::Reference< XMLPageExport > const & GetPageExport();

    // get chart export helper
    inline rtl::Reference< SchXMLExportHelper > const & GetChartExport();

    // get font auto style pool
    inline rtl::Reference< XMLFontAutoStylePool > const & GetFontAutoStylePool();

    ProgressBarHelper*  GetProgressBarHelper();

    // get Formlayer Export
    inline rtl::Reference< xmloff::OFormLayerXMLExport > const & GetFormExport();
    inline bool HasFormExport() const;

    // get XPropertySet with export information
    const css::uno::Reference< css::beans::XPropertySet >& getExportInfo() const { return mxExportInfo; }

    const css::uno::Reference< css::task::XStatusIndicator >& GetStatusIndicator() const { return mxStatusIndicator; }

    /// get Event export, with handlers for script types "None" and
    /// "StarBasic" already registered; other handlers may be registered, too.
    XMLEventExport& GetEventExport();

    /// get the export for image maps
    XMLImageMapExport& GetImageMapExport();

    OUString AddEmbeddedXGraphic(css::uno::Reference<css::graphic::XGraphic> const & rxGraphic, OUString & rOutMimeType, OUString const & rRequestedName = OUString());
    bool AddEmbeddedXGraphicAsBase64(css::uno::Reference<css::graphic::XGraphic> const & rxGraphic);
    bool GetGraphicMimeTypeFromStream(css::uno::Reference<css::graphic::XGraphic> const & rxGraphic, OUString & rOutMimeType);

    OUString AddEmbeddedObject(
                            const OUString& rEmbeddedObjectURL );
    bool AddEmbeddedObjectAsBase64(
                            const OUString& rEmbeddedObjectURL );

    OUString EncodeStyleName( const OUString& rName,
                                     bool *pEncoded=nullptr ) const;

    // save linked sections?
    bool IsSaveLinkedSections() const { return mbSaveLinkedSections; }

    // get export flags
    SvXMLExportFlags getExportFlags() const { return mnExportFlags; }

    void ExportEmbeddedOwnObject(
        css::uno::Reference<css::lang::XComponent > const & rComp );

    OUString GetRelativeReference(const OUString& rValue);

    // methods for accessing the document handler and handling SAX errors
    void StartElement(sal_uInt16 nPrefix,
                        enum ::xmloff::token::XMLTokenEnum eName,
                        bool bIgnWSOutside );
    void StartElement(const OUString& rName,
                        bool bIgnWSOutside );
    void Characters(const OUString& rChars);
    void EndElement(sal_uInt16 nPrefix,
                        enum ::xmloff::token::XMLTokenEnum eName,
                        bool bIgnWSInside );
    void EndElement(const OUString& rName,
                        bool bIgnWSInside );
    void IgnorableWhitespace();

    /**
     * Record an error condition that occurred during export. The
     * behavior of SetError can be modified using the error flag
     * constants.
     */
    void SetError(
        /// error ID, may contain an error flag
        sal_Int32 nId,
        /// string parameters for the error message
        const css::uno::Sequence< OUString> & rMsgParams,
        /// original exception message (if applicable)
        const OUString& rExceptionMessage,
        /// error location (if applicable)
        const css::uno::Reference<css::xml::sax::XLocator> & rLocator );

    void SetError(
        sal_Int32 nId,
        const css::uno::Sequence< OUString> & rMsgParams);

    virtual void DisposingModel();

    ::comphelper::UnoInterfaceToUniqueIdentifierMapper& getInterfaceToIdentifierMapper();

    const css::uno::Reference< css::uno::XComponentContext >& getComponentContext() const { return m_xContext;}

    // Shapes in Writer cannot be named via context menu (#i51726#)
    SvtModuleOptions::EFactory GetModelType() const
    {
        return meModelType;
    }

    // Written OpenDocument file format doesn't fit to the created text document (#i69627#)
    bool writeOutlineStyleAsNormalListStyle() const;

    css::uno::Reference< css::embed::XStorage > const & GetTargetStorage() const;

    /// returns value of ODF version attribute
    char const* GetODFVersionAttributeValue() const;

    /// returns the deterministic version for odf export
    SvtSaveOptions::ODFSaneDefaultVersion getSaneDefaultVersion() const;

    // FIXME: this is only for legacy stuff that has not yet been adapted
    //        to implement XMetadatable; this can write duplicate IDs!
    /// add xml:id and legacy namespace id
    void SAL_DLLPRIVATE AddAttributeIdLegacy(
            sal_uInt16 const nLegacyPrefix, OUString const& rValue);

    /// add xml:id attribute (for RDF metadata)
    void AddAttributeXmlId(css::uno::Reference<css::uno::XInterface> const & i_xIfc);

    /// add RDFa attributes for a metadatable text content
    void AddAttributesRDFa( css::uno::Reference<css::text::XTextContent> const & i_xTextContent);

    bool exportTextNumberElement() const;

    /// set null date from model to unit converter, if not already done
    bool SetNullDateOnUnitConverter();

    /// Get clamped mimetype for image export (empty if none)
    OUString const & GetImageFilterName() const;
};

inline rtl::Reference< XMLTextParagraphExport > const & SvXMLExport::GetTextParagraphExport()
{
    if( !mxTextParagraphExport.is() )
        mxTextParagraphExport = CreateTextParagraphExport();

    return mxTextParagraphExport;
}

inline rtl::Reference< XMLShapeExport > const & SvXMLExport::GetShapeExport()
{
    if( !mxShapeExport.is() )
        mxShapeExport = CreateShapeExport();

    return mxShapeExport;
}

inline rtl::Reference< SvXMLAutoStylePoolP > const & SvXMLExport::GetAutoStylePool()
{
    if( !mxAutoStylePool.is() )
        mxAutoStylePool = CreateAutoStylePool();

    return mxAutoStylePool;
}

inline rtl::Reference< SchXMLExportHelper > const & SvXMLExport::GetChartExport()
{
    if( !mxChartExport.is() )
        mxChartExport = CreateChartExport();

    return mxChartExport;
}

inline rtl::Reference< XMLPageExport > const & SvXMLExport::GetPageExport()
{
    if( !mxPageExport.is() )
        mxPageExport = CreatePageExport();

    return mxPageExport;
}

inline rtl::Reference< XMLFontAutoStylePool > const & SvXMLExport::GetFontAutoStylePool()
{
    if( !mxFontAutoStylePool.is() )
        mxFontAutoStylePool = CreateFontAutoStylePool();

    return mxFontAutoStylePool;
}

inline rtl::Reference< xmloff::OFormLayerXMLExport > const & SvXMLExport::GetFormExport()
{
    if( !mxFormExport.is() )
        mxFormExport = CreateFormExport();

    return mxFormExport;
}

inline bool SvXMLExport::HasFormExport() const
{
    return mxFormExport.is();
}

inline void SvXMLExport::SetEmbeddedResolver(
    css::uno::Reference< css::document::XEmbeddedObjectResolver > const & _xEmbeddedResolver )
{
    mxEmbeddedResolver = _xEmbeddedResolver;
}

inline void SvXMLExport::SetGraphicStorageHandler(
    css::uno::Reference<css::document::XGraphicStorageHandler> const & rxGraphicStorageHandler)
{
    mxGraphicStorageHandler = rxGraphicStorageHandler;
}

// Helper class to export an element.
class XMLOFF_DLLPUBLIC SvXMLElementExport
{
    SvXMLExport& mrExport;
    OUString maElementName;
    const bool mbIgnoreWhitespaceInside :1;
    const bool mbDoSomething :1;

    SAL_DLLPRIVATE
    void StartElement(
        const sal_uInt16 nPrefix,
        const OUString& rName,
        const bool bIgnoreWhitespaceOutside );

public:

    // The constructor prints a start tag that has the common attributes
    // of the XMLExport instance attached.
    SvXMLElementExport( SvXMLExport& rExp, sal_uInt16 nPrefix,
                        const OUString& rName,
                        bool bIgnWSOutside, bool bIgnWSInside );
    SvXMLElementExport( SvXMLExport& rExp, sal_uInt16 nPrefix,
                        enum ::xmloff::token::XMLTokenEnum eName,
                        bool bIgnWSOutside, bool bIgnWSInside );
    SvXMLElementExport( SvXMLExport& rExp, const OUString& rQName,
                        bool bIgnWSOutside, bool bIgnWSInside );

    // These constructors do nothing if bDoSomething is not set
    SvXMLElementExport( SvXMLExport& rExp, bool bDoSomething,
                        sal_uInt16 nPrefix,
                        enum ::xmloff::token::XMLTokenEnum eName,
                        bool bIgnWSOutside, bool bIgnWSInside );

    // The destructor prints an end tag.
    ~SvXMLElementExport();
};

#endif  //  _XMLOFF_SVXMLEXP_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
