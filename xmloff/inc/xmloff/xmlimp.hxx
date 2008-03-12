/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: xmlimp.hxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: rt $ $Date: 2008-03-12 10:23:46 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#ifndef _XMLOFF_XMLIMP_HXX
#define _XMLOFF_XMLIMP_HXX

#ifndef _SAL_CONFIG_H_
#include "sal/config.h"
#endif

#ifndef INCLUDED_XMLOFF_DLLAPI_H
#include "xmloff/dllapi.h"
#endif

#ifndef _SAL_TYPES_H_
#include "sal/types.h"
#endif
#ifndef _COM_SUN_STAR_XML_SAX_SAXPARSEEXCEPTION_HPP_
#include <com/sun/star/xml/sax/SAXParseException.hpp>
#endif
#ifndef _COM_SUN_STAR_XML_SAX_XEXTENDEDDOCUMENTHANDLER_HPP_
#include <com/sun/star/xml/sax/XExtendedDocumentHandler.hpp>
#endif
#ifndef _COM_SUN_STAR_XML_SAX_SAXEXCEPTION_HPP_
#include <com/sun/star/xml/sax/SAXException.hpp>
#endif
#ifndef _COM_SUN_STAR_XML_SAX_XATTRIBUTELIST_HPP_
#include <com/sun/star/xml/sax/XAttributeList.hpp>
#endif
#ifndef _COM_SUN_STAR_XML_SAX_XLOCATOR_HPP_
#include <com/sun/star/xml/sax/XLocator.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XUNOTUNNEL_HPP_
#include <com/sun/star/lang/XUnoTunnel.hpp>
#endif
#ifndef _COM_SUN_STAR_UTIL_XNUMBERFORMATSSUPPLIER_HPP_
#include <com/sun/star/util/XNumberFormatsSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XNAMECONTAINER_HPP_
#include <com/sun/star/container/XNameContainer.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XINITIALIZATION_HPP_
#include <com/sun/star/lang/XInitialization.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XSERVICEINFO_HPP_
#include <com/sun/star/lang/XServiceInfo.hpp>
#endif
#ifndef _COM_SUN_STAR_DOCUMENT_XIMPORTER_HPP_
#include <com/sun/star/document/XImporter.hpp>
#endif
#ifndef _COM_SUN_STAR_DOCUMENT_XFILTER_HPP_
#include <com/sun/star/document/XFilter.hpp>
#endif
#ifndef _COM_SUN_STAR_DRAWING_XGRAPHICOBJECTRESOLVER_HPP_
#include <com/sun/star/document/XGraphicObjectResolver.hpp>
#endif
#ifndef _COM_SUN_STAR_DRAWING_XEMBEDDEDOBJECTRESOLVER_HPP_
#include <com/sun/star/document/XEmbeddedObjectResolver.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSET_HPP_
#include <com/sun/star/beans/XPropertySet.hpp>
#endif

#ifndef _CPPUHELPER_WEAK_HXX_ //autogen wg. OWeakObject
#include <cppuhelper/weak.hxx>
#endif

#ifndef _XMLOFF_TEXTIMP_HXX_
#include <xmloff/txtimp.hxx>
#endif

#ifndef _XMLOFF_SHAPEIMPORT_HXX_
#include <xmloff/shapeimport.hxx>
#endif

#ifndef _XMLOFF_SCH_XMLIMPORTHELPER_HXX_
#include <xmloff/SchXMLImportHelper.hxx>
#endif
#ifndef _XMLOFF_PROGRESSBARHELPER_HXX
#include <xmloff/ProgressBarHelper.hxx>
#endif

#ifndef _CPPUHELPER_IMPLBASE6_HXX_
#include <cppuhelper/implbase6.hxx>
#endif
#ifndef _XMLOFF_FORMLAYERIMPORT_HXX_
#include <xmloff/formlayerimport.hxx>
#endif

#include <com/sun/star/beans/NamedValue.hpp>

namespace rtl { class OUString; }
namespace com { namespace sun { namespace star {
    namespace frame { class XModel; }
    namespace io { class XOutputStream; }
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
class String;

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
    ::rtl::OUString             msPackageProtocol;

    SAL_DLLPRIVATE void _InitCtor();

    sal_uInt16  mnImportFlags;
    sal_uInt16  mnErrorFlags;

protected:
    // #110680#
    ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory > mxServiceFactory;

    ::com::sun::star::uno::Reference< com::sun::star::task::XStatusIndicator > mxStatusIndicator;
    sal_Bool                    mbIsFormsSupported;
    bool                        mbIsTableShapeSupported;
    bool                        mbIsGraphicLoadOnDemmandSupported;

    // This method is called after the namespace map has been updated, but
    // before a context for the current element has been pushed.
    virtual SvXMLImportContext *CreateContext( sal_uInt16 nPrefix,
                                      const ::rtl::OUString& rLocalName,
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

    // #110680#
    //SvXMLImport( const ::com::sun::star::uno::Reference<
    //      ::com::sun::star::frame::XModel > & ) throw();
    SvXMLImport(
        const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& xServiceFactory,
        const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XModel > & ) throw();

    // #110680#
    //SvXMLImport( const ::com::sun::star::uno::Reference<
    //      ::com::sun::star::frame::XModel > &,
    //      const ::com::sun::star::uno::Reference< ::com::sun::star::document::XGraphicObjectResolver > & ) throw();
    SvXMLImport(
        const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& xServiceFactory,
        const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XModel > &,
        const ::com::sun::star::uno::Reference< ::com::sun::star::document::XGraphicObjectResolver > & ) throw();

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
    ::rtl::OUString ResolveGraphicObjectURL( const ::rtl::OUString& rURL, sal_Bool bLoadOnDemand );
    ::com::sun::star::uno::Reference< ::com::sun::star::io::XOutputStream >
        GetStreamForGraphicObjectURLFromBase64();
    ::rtl::OUString ResolveGraphicObjectURLFromBase64(
        const ::com::sun::star::uno::Reference< ::com::sun::star::io::XOutputStream >& rOut );

    sal_Bool IsPackageURL( const ::rtl::OUString& rURL ) const;
    ::rtl::OUString ResolveEmbeddedObjectURL( const ::rtl::OUString& rURL,
                                              const ::rtl::OUString& rClassId );
    ::com::sun::star::uno::Reference< ::com::sun::star::io::XOutputStream >
        GetStreamForEmbeddedObjectURLFromBase64();
    ::rtl::OUString ResolveEmbeddedObjectURLFromBase64();

    void AddStyleDisplayName( sal_uInt16 nFamily,
                              const ::rtl::OUString& rName,
                              const ::rtl::OUString& rDisplayName );
    ::rtl::OUString GetStyleDisplayName( sal_uInt16 nFamily,
                                      const ::rtl::OUString& rName );

    ProgressBarHelper*  GetProgressBarHelper();

    void AddNumberStyle(sal_Int32 nKey, const rtl::OUString& sName);

    virtual void SetViewSettings(const com::sun::star::uno::Sequence<com::sun::star::beans::PropertyValue>& aViewProps);
    virtual void SetConfigurationSettings(const com::sun::star::uno::Sequence<com::sun::star::beans::PropertyValue>& aConfigProps);

    XMLFontStylesContext *GetFontDecls();
    SvXMLStylesContext *GetStyles();
    SvXMLStylesContext *GetAutoStyles();
    SvXMLStylesContext *GetMasterStyles();
    const XMLFontStylesContext *GetFontDecls() const;
    const SvXMLStylesContext *GetStyles() const;
    const SvXMLStylesContext *GetAutoStyles() const;
    const SvXMLStylesContext *GetMasterStyles() const;

    sal_uInt16  getImportFlags() const { return mnImportFlags; }
    sal_Bool    IsFormsSupported() const { return mbIsFormsSupported; }
    rtl::OUString GetAbsoluteReference(const rtl::OUString& rValue);

    sal_Unicode ConvStarBatsCharToStarSymbol( sal_Unicode c );
    sal_Unicode ConvStarMathCharToStarSymbol( sal_Unicode c );

    bool IsTableShapeSupported() const { return mbIsTableShapeSupported; }

    ::rtl::OUString GetODFVersion() const;

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

    void SetError( sal_Int32 nId );
    void SetError( sal_Int32 nId, const rtl::OUString& rMsg1 );
    void SetError( sal_Int32 nId, const rtl::OUString& rMsg1,
                                  const rtl::OUString& rMsg2 );
    void SetError( sal_Int32 nId, const rtl::OUString& rMsg1,
                                  const rtl::OUString& rMsg2,
                                  const rtl::OUString& rMsg3 );
    void SetError( sal_Int32 nId, const rtl::OUString& rMsg1,
                                  const rtl::OUString& rMsg2,
                                  const rtl::OUString& rMsg3,
                                  const rtl::OUString& rMsg4 );

    /** return list of errors */
    XMLErrors* GetErrors();

    /** return current error flags */
    sal_uInt16 GetErrorFlags()  { return mnErrorFlags; }

    virtual void DisposingModel();

    ::comphelper::UnoInterfaceToUniqueIdentifierMapper& getInterfaceToIdentifierMapper();

    // #110680#
    ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory > getServiceFactory();

    // --> OD 2004-08-10 #i28749#
    sal_Bool IsShapePositionInHoriL2R() const;
    // <--

    // --> OD 2007-12-19 #152540#
    sal_Bool IsTextDocInOOoFileFormat() const;
    // <--

    String GetBaseURL() const;
    String GetDocumentBase() const;

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
