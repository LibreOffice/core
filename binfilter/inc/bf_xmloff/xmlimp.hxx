/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 * 
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#ifndef _XMLOFF_XMLIMP_HXX
#define _XMLOFF_XMLIMP_HXX

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

#include <bf_xmloff/txtimp.hxx>

#include <bf_xmloff/shapeimport.hxx>

#include <bf_xmloff/SchXMLImportHelper.hxx>
#include <bf_xmloff/ProgressBarHelper.hxx>

#include <cppuhelper/implbase6.hxx>
#include <bf_xmloff/formlayerimport.hxx>

namespace rtl { class OUString; }
namespace com { namespace sun { namespace star {
    namespace frame { class XModel; }
    namespace io { class XOutputStream; }
} } }

namespace binfilter {

class SvXMLNamespaceMap;
class SvXMLImportContext;
class SvXMLImportContexts_Impl;
class SvXMLImport_Impl;
class SvXMLUnitConverter;
class SvXMLNumFmtHelper;
class XMLFontStylesContext;
class XMLEventImportHelper;
class XMLErrors;
class SvXMLImportEventListener;

#define IMPORT_META			0x0001
#define IMPORT_STYLES		0x0002
#define IMPORT_MASTERSTYLES 0x0004
#define IMPORT_AUTOSTYLES	0x0008
#define IMPORT_CONTENT		0x0010
#define IMPORT_SCRIPTS		0x0020
#define IMPORT_SETTINGS		0x0040
#define IMPORT_FONTDECLS	0x0080
#define IMPORT_EMBEDDED		0x0100
#define IMPORT_ALL			0xffff

class SvXMLImport : public ::cppu::WeakImplHelper6<
             ::com::sun::star::xml::sax::XExtendedDocumentHandler,
             ::com::sun::star::lang::XServiceInfo,
             ::com::sun::star::lang::XInitialization,
             ::com::sun::star::document::XImporter,
             ::com::sun::star::document::XFilter,
             ::com::sun::star::lang::XUnoTunnel>
{
    friend class SvXMLImportContext;

    ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XLocator > xLocator;
    ::com::sun::star::uno::Reference< ::com::sun::star::frame::XModel > xModel;
    ::com::sun::star::uno::Reference< ::com::sun::star::util::XNumberFormatsSupplier > xNumberFormatsSupplier;
    ::com::sun::star::uno::Reference< ::com::sun::star::document::XGraphicObjectResolver > xGraphicResolver;
    ::com::sun::star::uno::Reference< ::com::sun::star::document::XEmbeddedObjectResolver > xEmbeddedResolver;
    ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet > xImportInfo;

    UniReference< XMLTextImportHelper >				mxTextImport;
    UniReference< XMLShapeImportHelper >			mxShapeImport;
#ifndef SVX_LIGHT
    UniReference< SchXMLImportHelper >				mxChartImport;
    UniReference< ::binfilter::xmloff::OFormLayerXMLImport >	mxFormImport;
#endif

    SvXMLImportContextRef xFontDecls;
    SvXMLImportContextRef xStyles;
    SvXMLImportContextRef xAutoStyles;
    SvXMLImportContextRef xMasterStyles;

    ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameContainer > xGradientHelper;
    ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameContainer > xHatchHelper;
    ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameContainer > xBitmapHelper;
    ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameContainer > xTransGradientHelper;
    ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameContainer > xMarkerHelper;
    ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameContainer > xDashHelper;
    ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameContainer > xNumberStyles;

    SvXMLImport_Impl			*pImpl;			// dummy

    // #110680#
    ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory > mxServiceFactory;

    SvXMLNamespaceMap 			*pNamespaceMap;
    SvXMLUnitConverter			*pUnitConv;
    SvXMLImportContexts_Impl	*pContexts;
    SvXMLNumFmtHelper			*pNumImport;
    ProgressBarHelper			*pProgressBarHelper;
    XMLEventImportHelper   		*pEventImportHelper;
    XMLErrors					*pXMLErrors;
    SvXMLImportEventListener	*pEventListener;
    ::rtl::OUString				sPackageProtocol;

    void _InitCtor();

    sal_uInt16	mnImportFlags;
    sal_uInt16	mnErrorFlags;

protected:
    ::com::sun::star::uno::Reference< ::com::sun::star::task::XStatusIndicator > mxStatusIndicator;
    sal_Bool					mbIsFormsSupported;

    // This method is called after the namespace map has been updated, but
    // before a context for the current element has been pushed.
    virtual SvXMLImportContext *CreateContext( sal_uInt16 nPrefix,
                                      const ::rtl::OUString& rLocalName,
                                      const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XAttributeList >& xAttrList );

    virtual XMLTextImportHelper* CreateTextImport();
    virtual XMLShapeImportHelper* CreateShapeImport();
    inline sal_Bool HasShapeImport() const { return mxShapeImport.is(); }
    inline void ClearShapeImport() { mxShapeImport = 0; }

#ifndef SVX_LIGHT
    virtual SchXMLImportHelper* CreateChartImport();
    virtual ::binfilter::xmloff::OFormLayerXMLImport* CreateFormImport();
#endif

    void SetFontDecls( XMLFontStylesContext *pFontDecls );
    void SetStyles( SvXMLStylesContext *pStyles );
    void SetAutoStyles( SvXMLStylesContext *pAutoStyles );
    void SetMasterStyles( SvXMLStylesContext *pMasterStyles );

    const ::com::sun::star::uno::Reference< ::com::sun::star::document::XEmbeddedObjectResolver >& GetEmbeddedResolver() const { return xEmbeddedResolver; }
    inline void SetEmbeddedResolver( ::com::sun::star::uno::Reference< ::com::sun::star::document::XEmbeddedObjectResolver >& _xEmbeddedResolver );

    const ::com::sun::star::uno::Reference< ::com::sun::star::document::XGraphicObjectResolver >& GetGraphicResolver() const { return xGraphicResolver; }
    void SetGraphicResolver( ::com::sun::star::uno::Reference< ::com::sun::star::document::XGraphicObjectResolver >& _xGraphicResolver );

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
    static SvXMLImport* getImplementation( ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > ) throw();

    // ::com::sun::star::xml::sax::XDocumentHandler
    virtual void SAL_CALL startDocument(void)
        throw( ::com::sun::star::xml::sax::SAXException, ::com::sun::star::uno::RuntimeException );
    virtual void SAL_CALL endDocument(void)
        throw( ::com::sun::star::xml::sax::SAXException, ::com::sun::star::uno::RuntimeException );
    virtual void SAL_CALL startElement(const ::rtl::OUString& aName,
                              const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XAttributeList > & xAttribs)
        throw( ::com::sun::star::xml::sax::SAXException, ::com::sun::star::uno::RuntimeException );
    virtual void SAL_CALL endElement(const ::rtl::OUString& aName)
        throw( ::com::sun::star::xml::sax::SAXException, ::com::sun::star::uno::RuntimeException );
    virtual void SAL_CALL characters(const ::rtl::OUString& aChars)
        throw( ::com::sun::star::xml::sax::SAXException, ::com::sun::star::uno::RuntimeException );
    virtual void SAL_CALL ignorableWhitespace(const ::rtl::OUString& aWhitespaces)
        throw( ::com::sun::star::xml::sax::SAXException, ::com::sun::star::uno::RuntimeException );
    virtual void SAL_CALL processingInstruction(const ::rtl::OUString& aTarget,
                                       const ::rtl::OUString& aData)
        throw( ::com::sun::star::xml::sax::SAXException, ::com::sun::star::uno::RuntimeException );
    virtual void SAL_CALL setDocumentLocator(const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XLocator > & xLocator)
        throw( ::com::sun::star::xml::sax::SAXException, ::com::sun::star::uno::RuntimeException );

    // ::com::sun::star::xml::sax::XExtendedDocumentHandler
    virtual void SAL_CALL startCDATA(void) throw( ::com::sun::star::xml::sax::SAXException, ::com::sun::star::uno::RuntimeException );
    virtual void SAL_CALL endCDATA(void) throw( ::com::sun::star::uno::RuntimeException );
    virtual void SAL_CALL comment(const ::rtl::OUString& sComment)
        throw( ::com::sun::star::xml::sax::SAXException, ::com::sun::star::uno::RuntimeException );
    virtual void SAL_CALL allowLineBreak(void)
        throw( ::com::sun::star::xml::sax::SAXException, ::com::sun::star::uno::RuntimeException );
    virtual void SAL_CALL unknown(const ::rtl::OUString& sString)
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
    virtual ::rtl::OUString SAL_CALL getImplementationName(  ) throw(::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL supportsService( const ::rtl::OUString& ServiceName ) throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames(  ) throw(::com::sun::star::uno::RuntimeException);

    // is called if the meta contains a meta:document-statistic element and gives the attributes of this element
    virtual void SetStatisticAttributes(const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XAttributeList > & xAttribs);

    // get import helper for text
    inline UniReference< XMLTextImportHelper > GetTextImport();
    sal_Bool HasTextImport() const { return mxTextImport.is(); }
    inline SvXMLNumFmtHelper* GetDataStylesImport();

    // get import helper for shapes
    inline UniReference< XMLShapeImportHelper > GetShapeImport();

#ifndef SVX_LIGHT
    // get import helper for charts
    inline UniReference< SchXMLImportHelper > GetChartImport();

    // get import helper for form layer
    inline UniReference< ::binfilter::xmloff::OFormLayerXMLImport > GetFormImport();
#endif // #ifndef SVX_LIGHT

    // get XPropertySet with import information
    inline ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet > getImportInfo() const { return xImportInfo; }

    // get import helper for events
    XMLEventImportHelper& GetEventImport();

    SvXMLNamespaceMap& GetNamespaceMap() { return *pNamespaceMap; }
    const SvXMLNamespaceMap& GetNamespaceMap() const { return *pNamespaceMap; }
    const SvXMLUnitConverter& GetMM100UnitConverter() const { return *pUnitConv; }
        SvXMLUnitConverter& GetMM100UnitConverter() { return *pUnitConv; }
    const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XLocator > & GetLocator() const { return xLocator; }
    const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XModel > &
               GetModel() const { return xModel; }

    const ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameContainer > & GetGradientHelper();
    const ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameContainer > & GetHatchHelper();
    const ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameContainer > & GetBitmapHelper();
    const ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameContainer > & GetTransGradientHelper();
    const ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameContainer > & GetMarkerHelper();
    const ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameContainer > & GetDashHelper();
    inline ::com::sun::star::uno::Reference< ::com::sun::star::util::XNumberFormatsSupplier > & GetNumberFormatsSupplier();

    // Convert a local packe URL into either a graphic manager or a
    // internal package URL. The later one happens only if bLoadURL is true
    ::rtl::OUString ResolveGraphicObjectURL( const ::rtl::OUString& rURL, sal_Bool bLoadOnDemand );
    ::com::sun::star::uno::Reference< ::com::sun::star::io::XOutputStream >
        GetStreamForGraphicObjectURLFromBase64();
    ::rtl::OUString ResolveGraphicObjectURLFromBase64(
        const ::com::sun::star::uno::Reference< ::com::sun::star::io::XOutputStream >& rOut );

    ::rtl::OUString ResolveEmbeddedObjectURL( const ::rtl::OUString& rURL,
                                              const ::rtl::OUString& rClassId );
    ::com::sun::star::uno::Reference< ::com::sun::star::io::XOutputStream >
        ResolveEmbeddedObjectURLFromBase64( const ::rtl::OUString& rURL );

    ProgressBarHelper*	GetProgressBarHelper();

    void AddNumberStyle(sal_Int32 nKey, const ::rtl::OUString& sName);

    virtual void SetViewSettings(const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue>& aViewProps);
    virtual void SetConfigurationSettings(const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue>& aConfigProps);

    XMLFontStylesContext *GetFontDecls();
    SvXMLStylesContext *GetStyles();
    SvXMLStylesContext *GetAutoStyles();
    const XMLFontStylesContext *GetFontDecls() const;
    const SvXMLStylesContext *GetStyles() const;
    const SvXMLStylesContext *GetAutoStyles() const;

    sal_uInt16	getImportFlags() const { return mnImportFlags; }
#ifndef SVX_LIGHT
    sal_Bool	IsFormsSupported() const { return mbIsFormsSupported; }
#endif
    ::rtl::OUString GetAbsoluteReference(const ::rtl::OUString& rValue);

#ifdef CONV_STAR_FONTS
    sal_Unicode ConvStarBatsCharToStarSymbol( sal_Unicode c );
    sal_Unicode ConvStarMathCharToStarSymbol( sal_Unicode c );
#endif

    /**
     * Record an error condition that occured during import. The
     * behavior of SetError can be modified using the error flag
     * constants.
     */
    void SetError(
        /// error ID, may contain an error flag
        sal_Int32 nId,
        /// string parameters for the error message
        const ::com::sun::star::uno::Sequence< ::rtl::OUString> & rMsgParams,
        /// original exception message (if applicable)
        const ::rtl::OUString& rExceptionMessage,
        /// error location (if applicable)
        const ::com::sun::star::uno::Reference< 
            ::com::sun::star::xml::sax::XLocator> & rLocator );

    void SetError(
        sal_Int32 nId,
        const ::com::sun::star::uno::Sequence< ::rtl::OUString> & rMsgParams);

    /** return current error flags */
    sal_uInt16 GetErrorFlags()  { return mnErrorFlags; }

    virtual void DisposingModel();

    // #110680#
    ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory > getServiceFactory();
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

#ifndef SVX_LIGHT
inline UniReference< SchXMLImportHelper > SvXMLImport::GetChartImport()
{
    if( !mxChartImport.is() )
        mxChartImport = CreateChartImport();

    return mxChartImport;
}
#endif

#ifndef SVX_LIGHT
inline UniReference< ::binfilter::xmloff::OFormLayerXMLImport > SvXMLImport::GetFormImport()
{
    if( !mxFormImport.is() )
        mxFormImport = CreateFormImport();

    return mxFormImport;
}
#endif // #ifndef SVX_LIGHT

inline void SvXMLImport::SetEmbeddedResolver(
    ::com::sun::star::uno::Reference< ::com::sun::star::document::XEmbeddedObjectResolver >& _xEmbeddedResolver )
{
    xEmbeddedResolver = _xEmbeddedResolver;
}

inline void SvXMLImport::SetGraphicResolver(
    ::com::sun::star::uno::Reference< ::com::sun::star::document::XGraphicObjectResolver >& _xGraphicResolver )
{
    xGraphicResolver = _xGraphicResolver;
}

inline ::com::sun::star::uno::Reference< ::com::sun::star::util::XNumberFormatsSupplier > & SvXMLImport::GetNumberFormatsSupplier()
{
    if ( ! xNumberFormatsSupplier.is() && xModel.is() )
        _CreateNumberFormatsSupplier();

    return xNumberFormatsSupplier;
}

inline SvXMLNumFmtHelper* SvXMLImport::GetDataStylesImport()
{
    if ( pNumImport == NULL)
        _CreateDataStylesImport();

    return pNumImport;
}

}//end of namespace binfilter
#endif	//  _XMLOFF_XMLIMP_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
