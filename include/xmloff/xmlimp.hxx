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

#ifndef INCLUDED_XMLOFF_XMLIMP_HXX
#define INCLUDED_XMLOFF_XMLIMP_HXX

#include <sal/config.h>

#include <set>

#include <xmloff/dllapi.h>
#include <sal/types.h>
#include <com/sun/star/embed/XStorage.hpp>
#include <com/sun/star/xml/sax/SAXParseException.hpp>
#include <com/sun/star/xml/sax/XExtendedDocumentHandler.hpp>
#include <com/sun/star/xml/sax/SAXException.hpp>
#include <com/sun/star/xml/sax/XAttributeList.hpp>
#include <com/sun/star/xml/sax/XLocator.hpp>
#include <com/sun/star/lang/XUnoTunnel.hpp>
#include <com/sun/star/util/XNumberFormatsSupplier.hpp>
#include <com/sun/star/container/XNameContainer.hpp>
#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/document/XImporter.hpp>
#include <com/sun/star/document/XFilter.hpp>
#include <com/sun/star/document/XGraphicObjectResolver.hpp>
#include <com/sun/star/document/XEmbeddedObjectResolver.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <cppuhelper/weak.hxx>
#include <xmloff/txtimp.hxx>
#include <xmloff/shapeimport.hxx>
#include <xmloff/SchXMLImportHelper.hxx>
#include <xmloff/ProgressBarHelper.hxx>
#include <cppuhelper/implbase7.hxx>
#include <xmloff/formlayerimport.hxx>

#include <com/sun/star/beans/NamedValue.hpp>

#include <com/sun/star/xml/sax/XFastDocumentHandler.hpp>
#include <com/sun/star/xml/sax/XFastContextHandler.hpp>
#include <com/sun/star/xml/sax/XFastAttributeList.hpp>
#include <o3tl/typed_flags_set.hxx>
#include <memory>

namespace com { namespace sun { namespace star {
    namespace frame { class XModel; }
    namespace io { class XOutputStream; }
    namespace rdf { class XMetadatable; }
} } }
namespace comphelper { class UnoInterfaceToUniqueIdentifierMapper; }

class SvXMLNamespaceMap;
class SvXMLImportContext;
class SvXMLImport_Impl;
class SvXMLUnitConverter;
class SvXMLNumFmtHelper;
class XMLFontStylesContext;
class XMLEventImportHelper;
class XMLErrors;
class StyleMap;
enum class SvXMLErrorFlags;

typedef std::vector<SvXMLImportContext *> SvXMLImportContexts_Impl;
typedef std::vector< ::css::uno::Reference< ::css::xml::sax::XFastContextHandler>>
            FastSvXMLImportContexts_Impl;

namespace xmloff {
    class RDFaImportHelper;
}

enum class SvXMLImportFlags {
    NONE            = 0x0000,
    META            = 0x0001,
    STYLES          = 0x0002,
    MASTERSTYLES    = 0x0004,
    AUTOSTYLES      = 0x0008,
    CONTENT         = 0x0010,
    SCRIPTS         = 0x0020,
    SETTINGS        = 0x0040,
    FONTDECLS       = 0x0080,
    EMBEDDED        = 0x0100,
    ALL             = 0xffff
};
namespace o3tl
{
    template<> struct typed_flags<SvXMLImportFlags> : is_typed_flags<SvXMLImportFlags, 0xffff> {};
}


class XMLOFF_DLLPUBLIC SvXMLImport : public ::cppu::WeakImplHelper7<
             css::xml::sax::XExtendedDocumentHandler,
             css::xml::sax::XFastDocumentHandler,
             css::lang::XServiceInfo,
             css::lang::XInitialization,
             css::document::XImporter,
             css::document::XFilter,
             css::lang::XUnoTunnel>
{
    friend class SvXMLImportContext;

    css::uno::Reference< css::xml::sax::XLocator > mxLocator;
    css::uno::Reference< css::frame::XModel > mxModel;
    css::uno::Reference< css::util::XNumberFormatsSupplier > mxNumberFormatsSupplier;
    css::uno::Reference< css::document::XGraphicObjectResolver > mxGraphicResolver;
    css::uno::Reference< css::document::XEmbeddedObjectResolver > mxEmbeddedResolver;
    css::uno::Reference< css::beans::XPropertySet > mxImportInfo;

    rtl::Reference< XMLTextImportHelper >             mxTextImport;
    rtl::Reference< XMLShapeImportHelper >            mxShapeImport;
    rtl::Reference< SchXMLImportHelper >              mxChartImport;
    rtl::Reference< ::xmloff::OFormLayerXMLImport >   mxFormImport;

    SvXMLImportContextRef mxFontDecls;
    SvXMLImportContextRef mxStyles;
    SvXMLImportContextRef mxAutoStyles;
    SvXMLImportContextRef mxMasterStyles;

    css::uno::Reference< css::container::XNameContainer > mxGradientHelper;
    css::uno::Reference< css::container::XNameContainer > mxHatchHelper;
    css::uno::Reference< css::container::XNameContainer > mxBitmapHelper;
    css::uno::Reference< css::container::XNameContainer > mxTransGradientHelper;
    css::uno::Reference< css::container::XNameContainer > mxMarkerHelper;
    css::uno::Reference< css::container::XNameContainer > mxDashHelper;
    css::uno::Reference< css::container::XNameContainer > mxNumberStyles;
    css::uno::Reference< css::lang::XEventListener > mxEventListener;

    std::unique_ptr<SvXMLImport_Impl>  mpImpl;            // dummy

    SvXMLNamespaceMap           *mpNamespaceMap;
    SvXMLUnitConverter          *mpUnitConv;
    SvXMLImportContexts_Impl    *mpContexts;
    FastSvXMLImportContexts_Impl    *mpFastContexts;
    SvXMLNumFmtHelper           *mpNumImport;
    ProgressBarHelper           *mpProgressBarHelper;
    XMLEventImportHelper        *mpEventImportHelper;
    XMLErrors                   *mpXMLErrors;
    StyleMap                    *mpStyleMap;
    OUString                    msPackageProtocol;

    SAL_DLLPRIVATE void InitCtor_();

    SvXMLImportFlags  mnImportFlags;
    SvXMLErrorFlags  mnErrorFlags;
    std::set< OUString > embeddedFontUrlsKnown;

protected:

    css::uno::Reference< css::task::XStatusIndicator > mxStatusIndicator;
    bool                        mbIsFormsSupported;
    bool                        mbIsTableShapeSupported;
    bool                        mbIsGraphicLoadOnDemandSupported;

    // This method is called after the namespace map has been updated, but
    // before a context for the current element has been pushed.
    virtual SvXMLImportContext *CreateContext( sal_uInt16 nPrefix,
                                               const OUString& rLocalName,
                                               const css::uno::Reference< css::xml::sax::XAttributeList >& xAttrList );
    virtual SvXMLImportContext *CreateFastContext( sal_Int32 Element,
        const ::css::uno::Reference< ::css::xml::sax::XFastAttributeList >& xAttrList );

    virtual XMLTextImportHelper* CreateTextImport();
    inline void ClearTextImport() { mxTextImport = nullptr; }
    virtual XMLShapeImportHelper* CreateShapeImport();
    inline bool HasShapeImport() const { return mxShapeImport.is(); }
    inline void ClearShapeImport() { mxShapeImport = nullptr; }

    static SchXMLImportHelper* CreateChartImport();
    ::xmloff::OFormLayerXMLImport* CreateFormImport();

    void SetFontDecls( XMLFontStylesContext *pFontDecls );
    void SetStyles( SvXMLStylesContext *pStyles );
    void SetAutoStyles( SvXMLStylesContext *pAutoStyles );
    void SetMasterStyles( SvXMLStylesContext *pMasterStyles );

    bool IsODFVersionConsistent( const OUString& aODFVersion );

    const css::uno::Reference< css::document::XEmbeddedObjectResolver >& GetEmbeddedResolver() const { return mxEmbeddedResolver; }
    inline void SetEmbeddedResolver( css::uno::Reference< css::document::XEmbeddedObjectResolver >& _xEmbeddedResolver );

    const css::uno::Reference< css::document::XGraphicObjectResolver >& GetGraphicResolver() const { return mxGraphicResolver; }
    void SetGraphicResolver( css::uno::Reference< css::document::XGraphicObjectResolver >& _xGraphicResolver );


    void CreateNumberFormatsSupplier_();
    void CreateDataStylesImport_();

public:
    // SvXMLImport( sal_uInt16 nImportFlags = IMPORT_ALL ) throw();
    SvXMLImport(
        const css::uno::Reference< css::uno::XComponentContext >& xContext,
        OUString const & implementationName,
        SvXMLImportFlags nImportFlags = SvXMLImportFlags::ALL ) throw();

    virtual ~SvXMLImport() throw();

    static const css::uno::Sequence< sal_Int8 > & getUnoTunnelId() throw();

    // css::xml::sax::XDocumentHandler
    virtual void SAL_CALL startDocument()
        throw (css::xml::sax::SAXException,
               css::uno::RuntimeException,
               std::exception) override;
    virtual void SAL_CALL endDocument()
        throw(css::xml::sax::SAXException,
              css::uno::RuntimeException,
              std::exception) override;
    virtual void SAL_CALL startElement(const OUString& aName,
        const css::uno::Reference< css::xml::sax::XAttributeList > & xAttribs)
        throw( css::xml::sax::SAXException, css::uno::RuntimeException, std::exception ) override;
    virtual void SAL_CALL endElement(const OUString& aName)
        throw( css::xml::sax::SAXException, css::uno::RuntimeException, std::exception ) override;
    virtual void SAL_CALL characters(const OUString& aChars)
        throw( css::xml::sax::SAXException, css::uno::RuntimeException, std::exception ) override;
    virtual void SAL_CALL ignorableWhitespace(const OUString& aWhitespaces)
        throw( css::xml::sax::SAXException, css::uno::RuntimeException, std::exception ) override;
    virtual void SAL_CALL processingInstruction(const OUString& aTarget,
                                                const OUString& aData)
        throw( css::xml::sax::SAXException, css::uno::RuntimeException, std::exception ) override;
    virtual void SAL_CALL setDocumentLocator(const css::uno::Reference< css::xml::sax::XLocator > & xLocator)
        throw( css::xml::sax::SAXException, css::uno::RuntimeException, std::exception ) override;

    // ::css::xml::sax::XFastContextHandler
    virtual void SAL_CALL startFastElement(sal_Int32 Element,
        const css::uno::Reference< css::xml::sax::XFastAttributeList > & Attribs)
        throw (css::uno::RuntimeException, css::xml::sax::SAXException, std::exception) override;
    virtual void SAL_CALL startUnknownElement(const OUString & Namespace,
        const OUString & Name,
        const css::uno::Reference< css::xml::sax::XFastAttributeList > & Attribs)
        throw (css::uno::RuntimeException, css::xml::sax::SAXException, std::exception) override;
    virtual void SAL_CALL endFastElement(sal_Int32 Element)
        throw (css::uno::RuntimeException, css::xml::sax::SAXException, std::exception) override;
    virtual void SAL_CALL endUnknownElement(const OUString & Namespace,
        const OUString & Name)
        throw (css::uno::RuntimeException, css::xml::sax::SAXException, std::exception) override;
    virtual css::uno::Reference< css::xml::sax::XFastContextHandler > SAL_CALL
    createFastChildContext(sal_Int32 Element,
        const css::uno::Reference< css::xml::sax::XFastAttributeList > & Attribs)
        throw (css::uno::RuntimeException, css::xml::sax::SAXException, std::exception) override;
    virtual css::uno::Reference< css::xml::sax::XFastContextHandler > SAL_CALL
    createUnknownChildContext(const OUString & Namespace, const OUString & Name,
        const css::uno::Reference< css::xml::sax::XFastAttributeList > & Attribs)
        throw (css::uno::RuntimeException, css::xml::sax::SAXException, std::exception) override;

    // css::xml::sax::XExtendedDocumentHandler
    virtual void SAL_CALL startCDATA() throw( css::xml::sax::SAXException, css::uno::RuntimeException, std::exception ) override;
    virtual void SAL_CALL endCDATA() throw( css::uno::RuntimeException, std::exception ) override;
    virtual void SAL_CALL comment(const OUString& sComment)
        throw( css::xml::sax::SAXException, css::uno::RuntimeException, std::exception ) override;
    virtual void SAL_CALL allowLineBreak()
        throw( css::xml::sax::SAXException, css::uno::RuntimeException, std::exception ) override;
    virtual void SAL_CALL unknown(const OUString& sString)
        throw( css::xml::sax::SAXException, css::uno::RuntimeException, std::exception ) override;

    // XImporter
    virtual void SAL_CALL setTargetDocument( const css::uno::Reference< css::lang::XComponent >& xDoc ) throw(css::lang::IllegalArgumentException, css::uno::RuntimeException, std::exception) override;

    // XFilter
    virtual sal_Bool SAL_CALL filter( const css::uno::Sequence< css::beans::PropertyValue >& aDescriptor ) throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL cancel(  ) throw (css::uno::RuntimeException, std::exception) override;

    // XInitialization
    virtual void SAL_CALL initialize( const css::uno::Sequence< css::uno::Any >& aArguments ) throw(css::uno::Exception, css::uno::RuntimeException, std::exception) override;

    // XUnoTunnel
    virtual sal_Int64 SAL_CALL getSomething( const css::uno::Sequence< sal_Int8 >& aIdentifier ) throw(css::uno::RuntimeException, std::exception) override;

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName(  )
        throw(css::uno::RuntimeException, std::exception) final override;
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName )
        throw(css::uno::RuntimeException, std::exception) final override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames(  )
        throw(css::uno::RuntimeException, std::exception) final override;

    // may be called by certain subclasses that handle document meta-data
    // override to provide customized handling of document statistics
    // the base class implementation initializes the progress bar and should
    // be called by overriding methods
    virtual void SetStatistics(const css::uno::Sequence< css::beans::NamedValue > & i_rStats);

    // get import helper for text
    inline rtl::Reference< XMLTextImportHelper > GetTextImport();
    bool HasTextImport() const { return mxTextImport.is(); }
    inline SvXMLNumFmtHelper* GetDataStylesImport();

    // get import helper for shapes
    inline rtl::Reference< XMLShapeImportHelper > GetShapeImport();

    // get import helper for charts
    inline rtl::Reference< SchXMLImportHelper > GetChartImport();

    // get import helper for form layer
    inline rtl::Reference< ::xmloff::OFormLayerXMLImport > GetFormImport();

    // get XPropertySet with import information
    const css::uno::Reference< css::beans::XPropertySet >& getImportInfo() const { return mxImportInfo; }

    // get import helper for events
    XMLEventImportHelper& GetEventImport();

    SvXMLNamespaceMap& GetNamespaceMap() { return *mpNamespaceMap; }
    const SvXMLNamespaceMap& GetNamespaceMap() const { return *mpNamespaceMap; }
    const SvXMLUnitConverter& GetMM100UnitConverter() const { return *mpUnitConv; }
        SvXMLUnitConverter& GetMM100UnitConverter() { return *mpUnitConv; }
    const css::uno::Reference< css::xml::sax::XLocator > & GetLocator() const { return mxLocator; }
    const css::uno::Reference< css::frame::XModel > &
        GetModel() const { return mxModel; }

    const css::uno::Reference< css::container::XNameContainer > & GetGradientHelper();
    const css::uno::Reference< css::container::XNameContainer > & GetHatchHelper();
    const css::uno::Reference< css::container::XNameContainer > & GetBitmapHelper();
    const css::uno::Reference< css::container::XNameContainer > & GetTransGradientHelper();
    const css::uno::Reference< css::container::XNameContainer > & GetMarkerHelper();
    const css::uno::Reference< css::container::XNameContainer > & GetDashHelper();
    inline css::uno::Reference< css::util::XNumberFormatsSupplier > & GetNumberFormatsSupplier();
    inline void SetNumberFormatsSupplier(const css::uno::Reference< css::util::XNumberFormatsSupplier >& _xNumberFormatSupplier)
    {
        mxNumberFormatsSupplier = _xNumberFormatSupplier;
    }

    // Convert a local package URL into either a graphic manager or a
    // internal package URL. The later one happens only if bLoadURL is true
    OUString ResolveGraphicObjectURL( const OUString& rURL, bool bLoadOnDemand );
    css::uno::Reference< css::io::XOutputStream >
          GetStreamForGraphicObjectURLFromBase64();
    OUString ResolveGraphicObjectURLFromBase64(
        const css::uno::Reference< css::io::XOutputStream >& rOut );

    bool IsPackageURL( const OUString& rURL ) const;
    OUString ResolveEmbeddedObjectURL( const OUString& rURL,
                                       const OUString& rClassId );
    css::uno::Reference< css::io::XOutputStream >
        GetStreamForEmbeddedObjectURLFromBase64();
    OUString ResolveEmbeddedObjectURLFromBase64();

    // get source storage we're importing from (if available)
    css::uno::Reference< css::embed::XStorage >
          GetSourceStorage();

    void AddStyleDisplayName( sal_uInt16 nFamily,
                              const OUString& rName,
                              const OUString& rDisplayName );
    OUString GetStyleDisplayName( sal_uInt16 nFamily,
                                  const OUString& rName ) const;

    ProgressBarHelper*  GetProgressBarHelper();

    void AddNumberStyle(sal_Int32 nKey, const OUString& sName);

    virtual void SetViewSettings(const css::uno::Sequence<css::beans::PropertyValue>& aViewProps);
    virtual void SetConfigurationSettings(const css::uno::Sequence<css::beans::PropertyValue>& aConfigProps);
    virtual void SetDocumentSpecificSettings(const OUString& _rSettingsGroupName,
                    const css::uno::Sequence<css::beans::PropertyValue>& _rSettings);

    XMLFontStylesContext *GetFontDecls();
    SvXMLStylesContext *GetStyles();
    SvXMLStylesContext *GetAutoStyles();
    const XMLFontStylesContext *GetFontDecls() const;
    const SvXMLStylesContext *GetStyles() const;
    const SvXMLStylesContext *GetAutoStyles() const;

    SvXMLImportFlags  getImportFlags() const { return mnImportFlags; }
    bool    IsFormsSupported() const { return mbIsFormsSupported; }
    OUString GetAbsoluteReference(const OUString& rValue) const;

    sal_Unicode ConvStarBatsCharToStarSymbol( sal_Unicode c );
    sal_Unicode ConvStarMathCharToStarSymbol( sal_Unicode c );

    bool IsTableShapeSupported() const { return mbIsTableShapeSupported; }

    OUString GetODFVersion() const;
    bool IsOOoXML() const; // legacy non-ODF format?

    /**
     * Record an error condition that occurred during import. The
     * behavior of SetError can be modified using the error flag
     * constants.
     */
    void SetError(
        /// error ID, may contain an error flag
        sal_Int32 nId,
        /// string parameters for the error message
        const css::uno::Sequence< OUString > & rMsgParams,
        /// original exception message (if applicable)
        const OUString& rExceptionMessage,
        /// error location (if applicable)
        const css::uno::Reference< css::xml::sax::XLocator> & rLocator );

    void SetError(
        sal_Int32 nId,
        const css::uno::Sequence< OUString> & rMsgParams);

    void SetError( sal_Int32 nId );
    void SetError( sal_Int32 nId, const OUString& rMsg1 );
    void SetError( sal_Int32 nId, const OUString& rMsg1,
                                  const OUString& rMsg2 );

    virtual void DisposingModel();

    ::comphelper::UnoInterfaceToUniqueIdentifierMapper& getInterfaceToIdentifierMapper();

    css::uno::Reference< css::uno::XComponentContext >
    GetComponentContext() const;

    // Convert drawing object positions from OOo file format to OASIS file format and vice versa (#i28749#)
    bool IsShapePositionInHoriL2R() const;

    bool IsTextDocInOOoFileFormat() const;

    OUString GetBaseURL() const;
    OUString GetDocumentBase() const;

    /// name of stream in package, e.g., "content.xml"
    OUString GetStreamName() const;

    /// set the XmlId attribute of given UNO object (for RDF metadata)
    void SetXmlId(css::uno::Reference<
                  css::uno::XInterface> const & i_xIfc,
                  OUString const & i_rXmlId);

    /// Add a RDFa statement; parameters are XML attribute values
    void AddRDFa( const css::uno::Reference< css::rdf::XMetadatable>& i_xObject,
                  OUString const & i_rAbout,
                  OUString const & i_rProperty,
                  OUString const & i_rContent,
                  OUString const & i_rDatatype);

    /// do not dllexport this; only for advanced cases (bookmark-start)
    SAL_DLLPRIVATE ::xmloff::RDFaImportHelper & GetRDFaImportHelper();

    // #i31958# XForms helper method
    // (to be implemented by applications supporting XForms)
    virtual void initXForms();

    /** returns the upd and build id (f.e. "680m124$Build-8964" gives rMaster = 680 and rBuild = 8964)
        from the metafile.
        this only works if the meta.xml was already imported and the
        import propertyset contains the string property "BuildId".
        If false is returned the build ids are not available (yet).
    **/
    bool getBuildIds( sal_Int32& rUPD, sal_Int32& rBuild ) const;

    static const sal_uInt16 OOo_1x = 10;
    static const sal_uInt16 OOo_2x = 20;
    static const sal_uInt16 OOo_30x = 30;
    static const sal_uInt16 OOo_31x = 31;
    static const sal_uInt16 OOo_32x = 32;
    static const sal_uInt16 OOo_33x = 33;
    static const sal_uInt16 OOo_34x = 34;
    // for AOO, no release overlaps with OOo, so continue OOo version numbers
    static const sal_uInt16 AOO_40x = 40;
    // @ATTENTION: it's not usually ok to use the "4x" "wildcard" in an "=="
    // comparison, since that will match unreleased versions too; it is also
    // risky to use it in "<" comparison, because it requires checking and
    // possibly adapting all such uses when a new value for a more specific
    // version is added.
    static const sal_uInt16 AOO_4x = 41;
    static const sal_uInt16 LO_flag = 0x100;
    static const sal_uInt16 LO_3x = 30 | LO_flag;
    static const sal_uInt16 LO_41x = 41 | LO_flag;
    static const sal_uInt16 LO_42x = 42 | LO_flag;
    static const sal_uInt16 LO_43x = 43 | LO_flag;
    static const sal_uInt16 LO_44x = 44 | LO_flag;
    /// @ATTENTION: when adding a new value more specific than "5x", grep for
    /// all current uses and adapt them!!!
    static const sal_uInt16 LO_5x = 50 | LO_flag;
    static const sal_uInt16 ProductVersionUnknown = SAL_MAX_UINT16;

    /** depending on whether the generator version indicates LO, compare
        against either the given LO or given OOo version */
    bool isGeneratorVersionOlderThan(
            sal_uInt16 const nOOoVersion, sal_uInt16 const nLOVersion);

    /** this checks the build ID and returns

        * OOo_1x for files created with OpenOffice.org 1.x or StarOffice 7 (this also includes binary import over binfilter)
        * OOo_2x for files created with OpenOffice.org 2.x or StarOffice 8
        * OOo_30x for files created with OpenOffice.org 3.0/3.0.1 or StarOffice 9/9 PU01
        * OOo_31x for files created with OpenOffice.org 3.1/3.1.1 or StarOffice 9 PU02/9 PU03
        * OOo_32x for files created with OpenOffice.org 3.2/3.2.1 or StarOffice 9 PU04 or Oracle Open Office 3.2.1
        * OOo_33x for files created with OpenOffice.org 3.3 (and minors) or Oracle Open Office 3.3 (and minors)
        * OOo_34x for files created with OpenOffice.org 3.4 Beta or Oracle Open Office 3.4 Beta
        * ProductVersionUnknown for files not created with OpenOffice.org, StarOffice or Oracle Open Office
    */
    sal_uInt16 getGeneratorVersion() const;

    /** If true, the URL for graphic shapes may be stored as a package URL and
        loaded later (on demand) by the application. Otherwise graphics are
        loaded immediately and the graphic shape gets the graphic manager URL.

        @see <member>mbIsGraphicLoadOnDemandSupported</member>
     */
    bool isGraphicLoadOnDemandSupported() const { return mbIsGraphicLoadOnDemandSupported;}

    /**
        Returns true if the embedded font document URL has already been processed.
        Otherwise returns false and consequent calls with the same URL will return true.
    */
    bool embeddedFontAlreadyProcessed( const OUString& url );

    virtual void NotifyEmbeddedFontRead() {};

    bool needFixPositionAfterZ() const;
};

inline rtl::Reference< XMLTextImportHelper > SvXMLImport::GetTextImport()
{
    if( !mxTextImport.is() )
        mxTextImport = CreateTextImport();

    return mxTextImport;
}

inline rtl::Reference< XMLShapeImportHelper > SvXMLImport::GetShapeImport()
{
    if( !mxShapeImport.is() )
        mxShapeImport = CreateShapeImport();

    return mxShapeImport;
}

inline rtl::Reference< SchXMLImportHelper > SvXMLImport::GetChartImport()
{
    if( !mxChartImport.is() )
        mxChartImport = CreateChartImport();

    return mxChartImport;
}

inline rtl::Reference< ::xmloff::OFormLayerXMLImport > SvXMLImport::GetFormImport()
{
    if( !mxFormImport.is() )
        mxFormImport = CreateFormImport();

    return mxFormImport;
}

inline void SvXMLImport::SetEmbeddedResolver(
    css::uno::Reference< css::document::XEmbeddedObjectResolver >& _xEmbeddedResolver )
{
    mxEmbeddedResolver = _xEmbeddedResolver;
}

inline void SvXMLImport::SetGraphicResolver(
    css::uno::Reference< css::document::XGraphicObjectResolver >& _xGraphicResolver )
{
    mxGraphicResolver = _xGraphicResolver;
}

inline css::uno::Reference< css::util::XNumberFormatsSupplier > & SvXMLImport::GetNumberFormatsSupplier()
{
    if ( ! mxNumberFormatsSupplier.is() && mxModel.is() )
        CreateNumberFormatsSupplier_();

    return mxNumberFormatsSupplier;
}

inline SvXMLNumFmtHelper* SvXMLImport::GetDataStylesImport()
{
    if ( mpNumImport == nullptr)
        CreateDataStylesImport_();

    return mpNumImport;
}


#endif // INCLUDED_XMLOFF_XMLIMP_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
