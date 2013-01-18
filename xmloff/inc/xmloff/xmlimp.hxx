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

#ifndef _XMLOFF_XMLIMP_HXX
#define _XMLOFF_XMLIMP_HXX

#include "sal/config.h"
#include "xmloff/dllapi.h"
#include "sal/types.h"
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
#include <cppuhelper/weak.hxx>
#include <xmloff/txtimp.hxx>
#include <xmloff/shapeimport.hxx>
#include <xmloff/SchXMLImportHelper.hxx>
#include <xmloff/ProgressBarHelper.hxx>
#include <cppuhelper/implbase6.hxx>
#include <xmloff/formlayerimport.hxx>

#include <com/sun/star/beans/NamedValue.hpp>

namespace com { namespace sun { namespace star {
    namespace uno { class XComponentContext; }
    namespace frame { class XModel; }
    namespace io { class XOutputStream; }
    namespace rdf { class XMetadatable; }
} } }
namespace comphelper { class UnoInterfaceToUniqueIdentifierMapper; }

class SvXMLNamespaceMap;
class SvXMLImportContext;
class SvXMLImportContexts_Impl;
class SvXMLImport_Impl;
class SvXMLUnitConverter;
class SvXMLNumFmtHelper;
class XMLFontStylesContext;
class XMLEventImportHelper;
class XMLErrors;
class StyleMap;

namespace xmloff {
    class RDFaImportHelper;
}

#define IMPORT_META         0x0001
#define IMPORT_STYLES       0x0002
#define IMPORT_MASTERSTYLES 0x0004
#define IMPORT_AUTOSTYLES   0x0008
#define IMPORT_CONTENT      0x0010
#define IMPORT_SCRIPTS      0x0020
#define IMPORT_SETTINGS     0x0040
#define IMPORT_FONTDECLS    0x0080
#define IMPORT_EMBEDDED     0x0100
#define IMPORT_OOO_NAMESPACES   0x0100
#define IMPORT_ALL          0xffff



class XMLOFF_DLLPUBLIC SvXMLImport : public ::cppu::WeakImplHelper6<
             ::com::sun::star::xml::sax::XExtendedDocumentHandler,
             ::com::sun::star::lang::XServiceInfo,
             ::com::sun::star::lang::XInitialization,
             ::com::sun::star::document::XImporter,
             ::com::sun::star::document::XFilter,
             ::com::sun::star::lang::XUnoTunnel>
{
    friend class SvXMLImportContext;

    ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XLocator > mxLocator;
    ::com::sun::star::uno::Reference< ::com::sun::star::frame::XModel > mxModel;
    ::com::sun::star::uno::Reference< ::com::sun::star::util::XNumberFormatsSupplier > mxNumberFormatsSupplier;
    ::com::sun::star::uno::Reference< ::com::sun::star::document::XGraphicObjectResolver > mxGraphicResolver;
    ::com::sun::star::uno::Reference< ::com::sun::star::document::XEmbeddedObjectResolver > mxEmbeddedResolver;
    ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet > mxImportInfo;

    UniReference< XMLTextImportHelper >             mxTextImport;
    UniReference< XMLShapeImportHelper >            mxShapeImport;
    UniReference< SchXMLImportHelper >              mxChartImport;
    UniReference< ::xmloff::OFormLayerXMLImport >   mxFormImport;

    SvXMLImportContextRef mxFontDecls;
    SvXMLImportContextRef mxStyles;
    SvXMLImportContextRef mxAutoStyles;
    SvXMLImportContextRef mxMasterStyles;

    ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameContainer > mxGradientHelper;
    ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameContainer > mxHatchHelper;
    ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameContainer > mxBitmapHelper;
    ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameContainer > mxTransGradientHelper;
    ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameContainer > mxMarkerHelper;
    ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameContainer > mxDashHelper;
    ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameContainer > mxNumberStyles;
    ::com::sun::star::uno::Reference< ::com::sun::star::lang::XEventListener > mxEventListener;

    SvXMLImport_Impl            *mpImpl;            // dummy

    SvXMLNamespaceMap           *mpNamespaceMap;
    SvXMLUnitConverter          *mpUnitConv;
    SvXMLImportContexts_Impl    *mpContexts;
    SvXMLNumFmtHelper           *mpNumImport;
    ProgressBarHelper           *mpProgressBarHelper;
    XMLEventImportHelper        *mpEventImportHelper;
    XMLErrors                   *mpXMLErrors;
    StyleMap                    *mpStyleMap;
    OUString                    msPackageProtocol;

    SAL_DLLPRIVATE void _InitCtor();

    sal_uInt16  mnImportFlags;
    sal_uInt16  mnErrorFlags;

protected:
    // #110680#
    ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory > mxServiceFactory;

    ::com::sun::star::uno::Reference< com::sun::star::task::XStatusIndicator > mxStatusIndicator;
    sal_Bool                    mbIsFormsSupported;
    bool                        mbIsTableShapeSupported;
    bool                        mbIsGraphicLoadOnDemandSupported;

    // This method is called after the namespace map has been updated, but
    // before a context for the current element has been pushed.
    virtual SvXMLImportContext *CreateContext( sal_uInt16 nPrefix,
                                               const OUString& rLocalName,
                                               const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XAttributeList >& xAttrList );

    virtual XMLTextImportHelper* CreateTextImport();
    virtual XMLShapeImportHelper* CreateShapeImport();
    inline sal_Bool HasShapeImport() const { return mxShapeImport.is(); }
    inline void ClearShapeImport() { mxShapeImport = 0; }

    virtual SchXMLImportHelper* CreateChartImport();
    virtual ::xmloff::OFormLayerXMLImport* CreateFormImport();

    void SetFontDecls( XMLFontStylesContext *pFontDecls );
    void SetStyles( SvXMLStylesContext *pStyles );
    void SetAutoStyles( SvXMLStylesContext *pAutoStyles );
    void SetMasterStyles( SvXMLStylesContext *pMasterStyles );

    sal_Bool IsODFVersionConsistent( const OUString& aODFVersion );

    const ::com::sun::star::uno::Reference< ::com::sun::star::document::XEmbeddedObjectResolver >& GetEmbeddedResolver() const { return mxEmbeddedResolver; }
    inline void SetEmbeddedResolver( com::sun::star::uno::Reference< com::sun::star::document::XEmbeddedObjectResolver >& _xEmbeddedResolver );

    const ::com::sun::star::uno::Reference< ::com::sun::star::document::XGraphicObjectResolver >& GetGraphicResolver() const { return mxGraphicResolver; }
    void SetGraphicResolver( com::sun::star::uno::Reference< com::sun::star::document::XGraphicObjectResolver >& _xGraphicResolver );


    void _CreateNumberFormatsSupplier();
    void _CreateDataStylesImport();

public:
    // #110680#
    // SvXMLImport( sal_uInt16 nImportFlags = IMPORT_ALL ) throw();
    SvXMLImport(
        const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& xServiceFactory,
        sal_uInt16 nImportFlags = IMPORT_ALL ) throw();

    virtual ~SvXMLImport() throw();

    static const ::com::sun::star::uno::Sequence< sal_Int8 > & getUnoTunnelId() throw();

    // ::com::sun::star::xml::sax::XDocumentHandler
    virtual void SAL_CALL startDocument(void)
        throw( ::com::sun::star::xml::sax::SAXException, ::com::sun::star::uno::RuntimeException );
    virtual void SAL_CALL endDocument(void)
        throw( ::com::sun::star::xml::sax::SAXException, ::com::sun::star::uno::RuntimeException );
    virtual void SAL_CALL startElement(const OUString& aName,
                                       const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XAttributeList > & xAttribs)
        throw( ::com::sun::star::xml::sax::SAXException, ::com::sun::star::uno::RuntimeException );
    virtual void SAL_CALL endElement(const OUString& aName)
        throw( ::com::sun::star::xml::sax::SAXException, ::com::sun::star::uno::RuntimeException );
    virtual void SAL_CALL characters(const OUString& aChars)
        throw( ::com::sun::star::xml::sax::SAXException, ::com::sun::star::uno::RuntimeException );
    virtual void SAL_CALL ignorableWhitespace(const OUString& aWhitespaces)
        throw( ::com::sun::star::xml::sax::SAXException, ::com::sun::star::uno::RuntimeException );
    virtual void SAL_CALL processingInstruction(const OUString& aTarget,
                                                const OUString& aData)
        throw( ::com::sun::star::xml::sax::SAXException, ::com::sun::star::uno::RuntimeException );
    virtual void SAL_CALL setDocumentLocator(const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XLocator > & xLocator)
        throw( ::com::sun::star::xml::sax::SAXException, ::com::sun::star::uno::RuntimeException );

    // ::com::sun::star::xml::sax::XExtendedDocumentHandler
    virtual void SAL_CALL startCDATA(void) throw( ::com::sun::star::xml::sax::SAXException, ::com::sun::star::uno::RuntimeException );
    virtual void SAL_CALL endCDATA(void) throw( ::com::sun::star::uno::RuntimeException );
    virtual void SAL_CALL comment(const OUString& sComment)
        throw( ::com::sun::star::xml::sax::SAXException, ::com::sun::star::uno::RuntimeException );
    virtual void SAL_CALL allowLineBreak(void)
        throw( ::com::sun::star::xml::sax::SAXException, ::com::sun::star::uno::RuntimeException );
    virtual void SAL_CALL unknown(const OUString& sString)
        throw( ::com::sun::star::xml::sax::SAXException, ::com::sun::star::uno::RuntimeException );

    // XImporter
    virtual void SAL_CALL setTargetDocument( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XComponent >& xDoc ) throw(::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException);

    // XFilter
    virtual sal_Bool SAL_CALL filter( const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& aDescriptor ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL cancel(  ) throw (::com::sun::star::uno::RuntimeException);

    // XInitialization
    virtual void SAL_CALL initialize( const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any >& aArguments ) throw(::com::sun::star::uno::Exception, ::com::sun::star::uno::RuntimeException);

    // XUnoTunnel
    virtual sal_Int64 SAL_CALL getSomething( const ::com::sun::star::uno::Sequence< sal_Int8 >& aIdentifier ) throw(::com::sun::star::uno::RuntimeException);

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName(  )
        throw(::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName )
        throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames(  )
        throw(::com::sun::star::uno::RuntimeException);

    // may be called by certain subclasses that handle document meta-data
    // override to provide customized handling of document statistics
    // the base class implementation initializes the progress bar and should
    // be called by overriding methods
    virtual void SetStatistics(const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::NamedValue > & i_rStats);

    // get import helper for text
    inline UniReference< XMLTextImportHelper > GetTextImport();
    sal_Bool HasTextImport() const { return mxTextImport.is(); }
    inline SvXMLNumFmtHelper* GetDataStylesImport();

    // get import helper for shapes
    inline UniReference< XMLShapeImportHelper > GetShapeImport();

    // get import helper for charts
    inline UniReference< SchXMLImportHelper > GetChartImport();

    // get import helper for form layer
    inline UniReference< ::xmloff::OFormLayerXMLImport > GetFormImport();

    // get XPropertySet with import information
    inline ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet > getImportInfo() const { return mxImportInfo; }

    // get import helper for events
    XMLEventImportHelper& GetEventImport();

    SvXMLNamespaceMap& GetNamespaceMap() { return *mpNamespaceMap; }
    const SvXMLNamespaceMap& GetNamespaceMap() const { return *mpNamespaceMap; }
    const SvXMLUnitConverter& GetMM100UnitConverter() const { return *mpUnitConv; }
        SvXMLUnitConverter& GetMM100UnitConverter() { return *mpUnitConv; }
    const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XLocator > & GetLocator() const { return mxLocator; }
    const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XModel > &
        GetModel() const { return mxModel; }

    const ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameContainer > & GetGradientHelper();
    const ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameContainer > & GetHatchHelper();
    const ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameContainer > & GetBitmapHelper();
    const ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameContainer > & GetTransGradientHelper();
    const ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameContainer > & GetMarkerHelper();
    const ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameContainer > & GetDashHelper();
    inline ::com::sun::star::uno::Reference< ::com::sun::star::util::XNumberFormatsSupplier > & GetNumberFormatsSupplier();
    inline void SetNumberFormatsSupplier(const ::com::sun::star::uno::Reference< ::com::sun::star::util::XNumberFormatsSupplier >& _xNumberFormatSupplier)
    {
        mxNumberFormatsSupplier = _xNumberFormatSupplier;
    }

    // Convert a local packe URL into either a graphic manager or a
    // internal package URL. The later one happens only if bLoadURL is true
    OUString ResolveGraphicObjectURL( const OUString& rURL, sal_Bool bLoadOnDemand );
    ::com::sun::star::uno::Reference< ::com::sun::star::io::XOutputStream >
          GetStreamForGraphicObjectURLFromBase64();
    OUString ResolveGraphicObjectURLFromBase64(
        const ::com::sun::star::uno::Reference< ::com::sun::star::io::XOutputStream >& rOut );

    sal_Bool IsPackageURL( const OUString& rURL ) const;
    OUString ResolveEmbeddedObjectURL( const OUString& rURL,
                                       const OUString& rClassId );
    ::com::sun::star::uno::Reference< ::com::sun::star::io::XOutputStream >
        GetStreamForEmbeddedObjectURLFromBase64();
    OUString ResolveEmbeddedObjectURLFromBase64();

    // get source storage we're importing from (if available)
    ::com::sun::star::uno::Reference< ::com::sun::star::embed::XStorage >
          GetSourceStorage();

    void AddStyleDisplayName( sal_uInt16 nFamily,
                              const OUString& rName,
                              const OUString& rDisplayName );
    OUString GetStyleDisplayName( sal_uInt16 nFamily,
                                  const OUString& rName ) const;

    ProgressBarHelper*  GetProgressBarHelper();

    void AddNumberStyle(sal_Int32 nKey, const OUString& sName);

    virtual void SetViewSettings(const com::sun::star::uno::Sequence<com::sun::star::beans::PropertyValue>& aViewProps);
    virtual void SetConfigurationSettings(const com::sun::star::uno::Sequence<com::sun::star::beans::PropertyValue>& aConfigProps);
    virtual void SetDocumentSpecificSettings(const OUString& _rSettingsGroupName,
                    const com::sun::star::uno::Sequence<com::sun::star::beans::PropertyValue>& _rSettings);

    XMLFontStylesContext *GetFontDecls();
    SvXMLStylesContext *GetStyles();
    SvXMLStylesContext *GetAutoStyles();
    const XMLFontStylesContext *GetFontDecls() const;
    const SvXMLStylesContext *GetStyles() const;
    const SvXMLStylesContext *GetAutoStyles() const;

    sal_uInt16  getImportFlags() const { return mnImportFlags; }
    sal_Bool    IsFormsSupported() const { return mbIsFormsSupported; }
    OUString GetAbsoluteReference(const OUString& rValue) const;

    sal_Unicode ConvStarBatsCharToStarSymbol( sal_Unicode c );
    sal_Unicode ConvStarMathCharToStarSymbol( sal_Unicode c );

    bool IsTableShapeSupported() const { return mbIsTableShapeSupported; }

    OUString GetODFVersion() const;

    /**
     * Record an error condition that occurred during import. The
     * behavior of SetError can be modified using the error flag
     * constants.
     */
    void SetError(
        /// error ID, may contain an error flag
        sal_Int32 nId,
        /// string parameters for the error message
        const ::com::sun::star::uno::Sequence< OUString > & rMsgParams,
        /// original exception message (if applicable)
        const OUString& rExceptionMessage,
        /// error location (if applicable)
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::xml::sax::XLocator> & rLocator );

    void SetError(
        sal_Int32 nId,
        const ::com::sun::star::uno::Sequence< OUString> & rMsgParams);

    void SetError( sal_Int32 nId );
    void SetError( sal_Int32 nId, const OUString& rMsg1 );
    void SetError( sal_Int32 nId, const OUString& rMsg1,
                                  const OUString& rMsg2 );

    /** return current error flags */
    sal_uInt16 GetErrorFlags()  { return mnErrorFlags; }

    virtual void DisposingModel();

    ::comphelper::UnoInterfaceToUniqueIdentifierMapper& getInterfaceToIdentifierMapper();

    // #110680#
    ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory > getServiceFactory();

    ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >
    GetComponentContext() const;

    // Convert drawing object positions from OOo file format to OASIS file format and vice versa (#i28749#)
    sal_Bool IsShapePositionInHoriL2R() const;

    sal_Bool IsTextDocInOOoFileFormat() const;

    OUString GetBaseURL() const;
    OUString GetDocumentBase() const;

    /// name of stream in package, e.g., "content.xml"
    OUString GetStreamName() const;

    /// set the XmlId attribute of given UNO object (for RDF metadata)
    void SetXmlId(::com::sun::star::uno::Reference<
                  ::com::sun::star::uno::XInterface> const & i_xIfc,
                  OUString const & i_rXmlId);

    /// Add a RDFa statement; parameters are XML attribute values
    void AddRDFa( ::com::sun::star::uno::Reference< ::com::sun::star::rdf::XMetadatable> i_xObject,
                  OUString const & i_rAbout,
                  OUString const & i_rProperty,
                  OUString const & i_rContent,
                  OUString const & i_rDatatype);

    /// do not dllexport this; only for advanced cases (bookmark-start)
    SAL_DLLPRIVATE ::xmloff::RDFaImportHelper & GetRDFaImportHelper();

    // #i31958# XForms helper method
    // (to be implemented by applications suporting XForms)
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
    static const sal_uInt16 LO_flag = 0x100;
    static const sal_uInt16 LO_3x = 30 | LO_flag;
    static const sal_uInt16 LO_4x = 40 | LO_flag;
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
    bool isGraphicLoadOnDemandSupported() const;
};

inline UniReference< XMLTextImportHelper > SvXMLImport::GetTextImport()
{
    if( !mxTextImport.is() )
        mxTextImport = CreateTextImport();

    return mxTextImport;
}

inline UniReference< XMLShapeImportHelper > SvXMLImport::GetShapeImport()
{
    if( !mxShapeImport.is() )
        mxShapeImport = CreateShapeImport();

    return mxShapeImport;
}

inline UniReference< SchXMLImportHelper > SvXMLImport::GetChartImport()
{
    if( !mxChartImport.is() )
        mxChartImport = CreateChartImport();

    return mxChartImport;
}

inline UniReference< ::xmloff::OFormLayerXMLImport > SvXMLImport::GetFormImport()
{
    if( !mxFormImport.is() )
        mxFormImport = CreateFormImport();

    return mxFormImport;
}

inline void SvXMLImport::SetEmbeddedResolver(
    com::sun::star::uno::Reference< com::sun::star::document::XEmbeddedObjectResolver >& _xEmbeddedResolver )
{
    mxEmbeddedResolver = _xEmbeddedResolver;
}

inline void SvXMLImport::SetGraphicResolver(
    com::sun::star::uno::Reference< com::sun::star::document::XGraphicObjectResolver >& _xGraphicResolver )
{
    mxGraphicResolver = _xGraphicResolver;
}

inline ::com::sun::star::uno::Reference< ::com::sun::star::util::XNumberFormatsSupplier > & SvXMLImport::GetNumberFormatsSupplier()
{
    if ( ! mxNumberFormatsSupplier.is() && mxModel.is() )
        _CreateNumberFormatsSupplier();

    return mxNumberFormatsSupplier;
}

inline SvXMLNumFmtHelper* SvXMLImport::GetDataStylesImport()
{
    if ( mpNumImport == NULL)
        _CreateDataStylesImport();

    return mpNumImport;
}


#endif  //  _XMLOFF_XMLIMP_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
