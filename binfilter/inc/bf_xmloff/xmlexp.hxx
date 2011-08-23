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

#ifndef _XMLOFF_XMLEXP_HXX
#define _XMLOFF_XMLEXP_HXX

#ifndef _COM_SUN_STAR_XML_SAX_SAXPARSEEXCEPTION_HPP_
#include <com/sun/star/xml/sax/SAXParseException.hpp>
#endif
#ifndef _COM_SUN_STAR_XML_SAX_XEXTENDEDDOCUMENTHANDLER_HPP_
#include <com/sun/star/xml/sax/XExtendedDocumentHandler.hpp>
#endif
#ifndef _COM_SUN_STAR_XML_SAX_SAXEXCEPTION_HPP_
#include <com/sun/star/xml/sax/SAXException.hpp>
#endif
#ifndef _COM_SUN_STAR_XML_SAX_XDOCUMENTHANDLER_HPP_
#include <com/sun/star/xml/sax/XDocumentHandler.hpp>
#endif
#ifndef _COM_SUN_STAR_XML_SAX_XATTRIBUTELIST_HPP_
#include <com/sun/star/xml/sax/XAttributeList.hpp>
#endif
#ifndef _COM_SUN_STAR_XML_SAX_XLOCATOR_HPP_
#include <com/sun/star/xml/sax/XLocator.hpp>
#endif
#ifndef _COM_SUN_STAR_UTIL_XNUMBERFORMATSSUPPLIER_HPP_
#include <com/sun/star/util/XNumberFormatsSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XUNOTUNNEL_HPP_
#include <com/sun/star/lang/XUnoTunnel.hpp>
#endif

#ifndef _RTL_USTRING_HXX_
#include <rtl/ustring.hxx>
#endif
#ifndef _VCL_MAPUNIT_HXX
#include <vcl/mapunit.hxx>
#endif

#ifndef _XMLOFF_TEXTPARAE_HXX_
#include <bf_xmloff/txtparae.hxx>
#endif
#ifndef _XMLOFF_FORMLAYEREXPORT_HXX_
#include <bf_xmloff/formlayerexport.hxx>
#endif
#ifndef _XMLOFF_XMLNUMFE_HXX
#include <bf_xmloff/xmlnumfe.hxx>
#endif

#ifndef _XMLOFF_XMLASTPLP_HXX
#include <bf_xmloff/xmlaustp.hxx>
#endif

#ifndef _XMLOFF_SHAPEEXPORT_HXX_
#include <bf_xmloff/shapeexport.hxx>
#endif

#ifndef _XMLOFF_XMLTOKEN_HXX
#include <bf_xmloff/xmltoken.hxx>
#endif

#ifndef _XMLOFF_SCH_XMLEXPORTHELPER_HXX_
#include <bf_xmloff/SchXMLExportHelper.hxx>
#endif
#ifndef _XMLOFF_XMLFONTAUTOSTYLEPOOL_HXX
#include <bf_xmloff/XMLFontAutoStylePool.hxx>
#endif
#ifndef _COM_SUN_STAR_DOCUMENT_XFILTER_HPP_
#include <com/sun/star/document/XFilter.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XSERVICEINFO_HPP_
#include <com/sun/star/lang/XServiceInfo.hpp>
#endif
#ifndef _COM_SUN_STAR_DOCUMENT_XEXPORTER_HPP_
#include <com/sun/star/document/XExporter.hpp>
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
#ifndef _COM_SUN_STAR_LANG_XINITIALIZATION_HPP_
#include <com/sun/star/lang/XInitialization.hpp>
#endif
#ifndef _COM_SUN_STAR_DOCUMENT_XEVENTLISTENER_HPP_
#include <com/sun/star/lang/XEventListener.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XNAMED_HPP_
#include <com/sun/star/container/XNamed.hpp>
#endif

#ifndef _XMLOFF_XMLPAGEEXPORT_HXX
#include <bf_xmloff/XMLPageExport.hxx>
#endif
#ifndef _XMLOFF_PROGRESSBARHELPER_HXX
#include <bf_xmloff/ProgressBarHelper.hxx>
#endif

#ifndef _CPPUHELPER_IMPLBASE6_HXX_
#include <cppuhelper/implbase6.hxx>
#endif
namespace rtl { class OUString; }
namespace com { namespace sun { namespace star {
    namespace frame { class XModel; }
    namespace container { class XIndexContainer; }
} } }
namespace binfilter {

class SvXMLNamespaceMap;
class SvXMLAttributeList;
class SvXMLExport_Impl;
class SvXMLUnitConverter;
class ProgressBarHelper;
class XMLEventExport;
class XMLSettingsExportHelper;
class XMLImageMapExport;
class XMLErrors;
class SvXMLExportEventListener;


#define EXPORT_META			0x0001
#define EXPORT_STYLES		0x0002
#define EXPORT_MASTERSTYLES 0x0004
#define EXPORT_AUTOSTYLES	0x0008
#define EXPORT_CONTENT		0x0010
#define EXPORT_SCRIPTS		0x0020
#define EXPORT_SETTINGS		0x0040
#define EXPORT_FONTDECLS	0x0080
#define EXPORT_EMBEDDED		0x0100
#define EXPORT_NODOCTYPE	0x0200
#define EXPORT_PRETTY		0x0400
#define EXPORT_ALL			0xffff

class SvXMLExport : public ::cppu::WeakImplHelper6<
             ::com::sun::star::document::XFilter,
             ::com::sun::star::lang::XServiceInfo,
             ::com::sun::star::document::XExporter,
              ::com::sun::star::lang::XInitialization,
             ::com::sun::star::container::XNamed,
             ::com::sun::star::lang::XUnoTunnel>
{
    SvXMLExport_Impl			*pImpl;			// dummy

    // #110680#
    ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory > mxServiceFactory;

    ::com::sun::star::uno::Reference< ::com::sun::star::frame::XModel > xModel;
    ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XDocumentHandler > 			xHandler;		// the handlers
    ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XExtendedDocumentHandler > 	xExtHandler;
    ::com::sun::star::uno::Reference< ::com::sun::star::util::XNumberFormatsSupplier > xNumberFormatsSupplier;
    ::com::sun::star::uno::Reference< ::com::sun::star::document::XGraphicObjectResolver > xGraphicResolver;
    ::com::sun::star::uno::Reference< ::com::sun::star::document::XEmbeddedObjectResolver > xEmbeddedResolver;
    ::com::sun::star::uno::Reference< ::com::sun::star::task::XStatusIndicator > xStatusIndicator;
    ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet > xExportInfo;

    SvXMLAttributeList			*pAttrList;		// a common attribute list
    ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XAttributeList > 			xAttrList;		// and an interface of it

    ::rtl::OUString		sOrigFileName;	// the original URL
    ::rtl::OUString		sPicturesPath;
    ::rtl::OUString		sGraphicObjectProtocol;
    ::rtl::OUString		sEmbeddedObjectProtocol;
    ::rtl::OUString		sObjectsPath;
    ::rtl::OUString     sFilterName;
    SvXMLNamespaceMap			*pNamespaceMap;	// the namepspace map
    SvXMLUnitConverter			*pUnitConv;		// the unit converter
    SvXMLNumFmtExport			*pNumExport;
    ProgressBarHelper			*pProgressBarHelper;

    UniReference< XMLTextParagraphExport > mxTextParagraphExport;
    UniReference< XMLShapeExport > mxShapeExport;
    UniReference< SvXMLAutoStylePoolP > mxAutoStylePool;
    UniReference< SchXMLExportHelper > mxChartExport;
    UniReference< XMLPageExport > mxPageExport;
    UniReference< XMLFontAutoStylePool > mxFontAutoStylePool;
    UniReference< xmloff::OFormLayerXMLExport > mxFormExport;
    XMLEventExport* pEventExport;
    XMLImageMapExport* pImageMapExport;
    XMLErrors*	pXMLErrors;
    SvXMLExportEventListener* pEventListener;

    sal_Bool						bExtended;		// Does document contain extens.

    const enum ::binfilter::xmloff::token::XMLTokenEnum meClass;
    void _InitCtor();

    sal_uInt16	mnExportFlags;
    sal_uInt16	mnErrorFlags;

public:

    const ::rtl::OUString				sWS;			// " "

private:

    void ImplExportMeta();							// <office:meta>
    void ImplExportSettings();						// <office:settings>
    void ImplExportStyles( sal_Bool bUsed );		// <office:styles>
    void ImplExportAutoStyles( sal_Bool bUsed );	// <office:automatic-styles>
    void ImplExportMasterStyles( sal_Bool bUsed );	// <office:master-styles>
    void ImplExportContent();						// <office:body>
    virtual void SetBodyAttributes();

protected:
    void setExportFlags( sal_uInt16 nExportFlags ) { mnExportFlags = nExportFlags; }

    // Get (modifyable) namespace map
    SvXMLNamespaceMap& _GetNamespaceMap() { return *pNamespaceMap; }

    // This method can be overloaded to export the content of <office:meta>.
    // There is a default implementation.
    virtual void _ExportMeta();

    // This method can be overloaded to export the content of <office:view-settings>.
    // There is no default implementation.
    virtual void _ExportViewSettings(const XMLSettingsExportHelper& rSettingsExportHelper);

    // This method can be overloaded to export the content of <office:configuration-settings>.
    // There is no default implementation.
    virtual void _ExportConfigurationSettings(const XMLSettingsExportHelper& rSettingsExportHelper);

    // This method can be overloaded to export the content of <office:script>.
    // There is a default implementation.
    virtual void _ExportScripts();

    // This method can be overloaded to export the font declarationsE
    // The default implementation will export the contents of the
    // XMLFontAutoStylePool if it has been created.
    virtual void _ExportFontDecls();

    // This method should be overloaded to export the content of <style:styles>.
    // If bUsed is set, used styles should be exported only.
    // Overloaded Methods must call this method !
    virtual void _ExportStyles( sal_Bool bUsed ) ;

    // This method must be overloaded to export the contents of
    // <style:auto-styles>
    virtual void _ExportAutoStyles() = 0;

    // This method must be overloaded to export the contents of
    // <style:master-styles>
    virtual void _ExportMasterStyles() = 0;

    // This method must be overloaded to export the content of <office:body>.
    virtual void _ExportContent() = 0;

    void SetExtended( sal_Bool bSet=sal_True ) { bExtended = bSet; }

    // save linked sections? (may be false in global documents)
    sal_Bool bSaveLinkedSections;

    virtual XMLTextParagraphExport* CreateTextParagraphExport();
    virtual XMLShapeExport* CreateShapeExport();
    virtual SvXMLAutoStylePoolP* CreateAutoStylePool();
    virtual SchXMLExportHelper* CreateChartExport();
    virtual XMLPageExport* CreatePageExport();
    virtual XMLFontAutoStylePool* CreateFontAutoStylePool();
    virtual xmloff::OFormLayerXMLExport* CreateFormExport();
    virtual void GetViewSettings( ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue>& aProps);
    virtual void GetConfigurationSettings( ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue>& aProps);

    const ::com::sun::star::uno::Reference< ::com::sun::star::document::XEmbeddedObjectResolver >& GetEmbeddedResolver() const { return xEmbeddedResolver; }
    inline void SetEmbeddedResolver( ::com::sun::star::uno::Reference< ::com::sun::star::document::XEmbeddedObjectResolver >& _xEmbeddedResolver );

    const ::com::sun::star::uno::Reference< ::com::sun::star::document::XGraphicObjectResolver >& GetGraphicResolver() const { return xGraphicResolver; }
    void SetGraphicResolver( ::com::sun::star::uno::Reference< ::com::sun::star::document::XGraphicObjectResolver >& _xGraphicResolver );
public:

    // #110680#
    //SvXMLExport( MapUnit eDfltUnit,
    //             const enum ::binfilter::xmloff::token::XMLTokenEnum eClass = xmloff::token::XML_TOKEN_INVALID,
    //             sal_uInt16 nExportFlag = EXPORT_ALL );
    SvXMLExport(
        const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& xServiceFactory,
        MapUnit eDfltUnit,
        const enum ::binfilter::xmloff::token::XMLTokenEnum eClass = xmloff::token::XML_TOKEN_INVALID,
        sal_uInt16 nExportFlag = EXPORT_ALL );

    virtual ~SvXMLExport();

    static const ::com::sun::star::uno::Sequence< sal_Int8 > & getUnoTunnelId() throw();
    static SvXMLExport* getImplementation( ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > ) throw();

    // XExporter
    virtual void SAL_CALL setSourceDocument( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XComponent >& xDoc ) throw(::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException);

    // XFilter
    virtual sal_Bool SAL_CALL filter( const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& aDescriptor ) throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL cancel() throw(::com::sun::star::uno::RuntimeException);

    // XInitialization
    virtual void SAL_CALL initialize( const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any >& aArguments ) throw(::com::sun::star::uno::Exception, ::com::sun::star::uno::RuntimeException);

    // XNamed
    virtual ::rtl::OUString SAL_CALL getName(  ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setName( const ::rtl::OUString& aName ) throw (::com::sun::star::uno::RuntimeException);

    // XServiceInfo
    virtual ::rtl::OUString SAL_CALL getImplementationName(  ) throw(::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL supportsService( const ::rtl::OUString& ServiceName ) throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames(  ) throw(::com::sun::star::uno::RuntimeException);

    // XUnoTunnel
    virtual sal_Int64 SAL_CALL getSomething( const ::com::sun::star::uno::Sequence< sal_Int8 >& aIdentifier ) throw(::com::sun::star::uno::RuntimeException);

    // Check if common attribute list is empty.
#ifndef DBG_UTIL
    void CheckAttrList() {}
#else
    void CheckAttrList();
#endif

    // Clear common attribute list.
    void ClearAttrList();

    // Add an attribute to the common attribute list.
    void AddAttributeASCII( sal_uInt16 nPrefix, const sal_Char *pName,
                            const sal_Char *pValue );
    void AddAttribute( sal_uInt16 nPrefix, const sal_Char *pName,
                       const ::rtl::OUString& rValue );
    void AddAttribute( sal_uInt16 nPrefix, const ::rtl::OUString& rName,
                       const ::rtl::OUString& rValue );
    void AddAttribute( sal_uInt16 nPrefix,
                       enum ::binfilter::xmloff::token::XMLTokenEnum eName,
                       const ::rtl::OUString& rValue );
    void AddAttribute( sal_uInt16 nPrefix,
                       enum ::binfilter::xmloff::token::XMLTokenEnum eName,
                       enum ::binfilter::xmloff::token::XMLTokenEnum eValue );
    void AddAttribute( const ::rtl::OUString& rQName,
                       const ::rtl::OUString& rValue );
    void AddAttribute( const ::rtl::OUString& rQName,
                       enum ::binfilter::xmloff::token::XMLTokenEnum eValue );

    // Get common attribute list as implementation or interface.
    SvXMLAttributeList &GetAttrList() { return *pAttrList; }
    const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XAttributeList > & GetXAttrList() { return xAttrList; }

    // Get document handler. This methods are not const, because the
    // reference allowes modifications through the handler.
    const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XDocumentHandler > & GetDocHandler() { return xHandler; }
    const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XExtendedDocumentHandler > & GetExtDocHandler()
    {
        return xExtHandler;
    }

    // Get original ::com::sun::star::util::URL.
    const ::rtl::OUString& GetOrigFileName() const { return sOrigFileName; }

    // Get the relative path for embedded pictures
    const ::rtl::OUString& GetPicturesPath() const { return sPicturesPath; }

    // Get the relative path for embedded objects
    const ::rtl::OUString& GetObjectsPath() const { return sObjectsPath; }

    // Get (const) namespace map.
    const SvXMLNamespaceMap& GetNamespaceMap() const { return *pNamespaceMap; }

    // Get unit converter
    const SvXMLUnitConverter& GetMM100UnitConverter() const { return *pUnitConv; }

    SvXMLUnitConverter& GetMM100UnitConverter() { return *pUnitConv; }

    // Export the document.
    virtual sal_uInt32 exportDoc( enum ::binfilter::xmloff::token::XMLTokenEnum eClass = xmloff::token::XML_TOKEN_INVALID );

    virtual void addDataStyle(const sal_Int32 nNumberFormat, sal_Bool bTimeFormat = sal_False );
    virtual void exportDataStyles();
    virtual void exportAutoDataStyles();
    virtual ::rtl::OUString getDataStyleName(const sal_Int32 nNumberFormat, sal_Bool bTimeFormat = sal_False ) const;
    sal_Int32 dataStyleForceSystemLanguage(sal_Int32 nFormat) const;

    // Get XModel
    const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XModel > &
               GetModel() const { return xModel; }
    // Get XNumberFormatsSupplier
    ::com::sun::star::uno::Reference< ::com::sun::star::util::XNumberFormatsSupplier > & GetNumberFormatsSupplier() { return xNumberFormatsSupplier; }

    // get export helper for text
    inline UniReference< XMLTextParagraphExport > GetTextParagraphExport();

    // get export helper for shapes
    inline UniReference< XMLShapeExport > GetShapeExport();

    // get auto style pool
    inline UniReference< SvXMLAutoStylePoolP > GetAutoStylePool();

    // get Page Export
    inline UniReference< XMLPageExport > GetPageExport();

    // get chart export helper
    inline UniReference< SchXMLExportHelper > GetChartExport();

    // get font auto style pool
    inline UniReference< XMLFontAutoStylePool > GetFontAutoStylePool();

    ProgressBarHelper*	GetProgressBarHelper();

    // get Formlayer Export
    inline UniReference< xmloff::OFormLayerXMLExport > GetFormExport();
    inline bool HasFormExport();

    // get XPropertySet with export information
    inline ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet > getExportInfo() const { return xExportInfo; }

    ::com::sun::star::uno::Reference< ::com::sun::star::task::XStatusIndicator > GetStatusIndicator() { return xStatusIndicator; }

    /// get Event export, with handlers for script types "None" and
    /// "StarBasic" already registered; other handlers may be registered, too.
    XMLEventExport& GetEventExport();

    /// get the export for image maps
    XMLImageMapExport& GetImageMapExport();

    ::rtl::OUString AddEmbeddedGraphicObject(
                            const ::rtl::OUString& rGraphicObjectURL );
    sal_Bool AddEmbeddedGraphicObjectAsBase64(
                            const ::rtl::OUString& rGraphicObjectURL );

    ::rtl::OUString AddEmbeddedObject(
                            const ::rtl::OUString& rEmbeddedObjectURL );
    sal_Bool AddEmbeddedObjectAsBase64(
                            const ::rtl::OUString& rEmbeddedObjectURL );

    // save linked sections?
    inline sal_Bool IsSaveLinkedSections() { return bSaveLinkedSections; }

    // get export flags
    sal_uInt16 getExportFlags() const { return mnExportFlags; }

    sal_Bool ExportEmbeddedOwnObject(
        ::com::sun::star::uno::Reference<
            ::com::sun::star::lang::XComponent >& rComp );

    ::rtl::OUString GetRelativeReference(const ::rtl::OUString& rValue);

    // methods for accessing the document handler and handling SAX errors
    void StartElement(sal_uInt16 nPrefix,
                        enum ::binfilter::xmloff::token::XMLTokenEnum eName,
                        sal_Bool bIgnWSOutside );
    void StartElement(const ::rtl::OUString& rName,
                        sal_Bool bIgnWSOutside );
    void Characters(const ::rtl::OUString& rChars);
    void EndElement(sal_uInt16 nPrefix,
                        enum ::binfilter::xmloff::token::XMLTokenEnum eName,
                        sal_Bool bIgnWSInside );
    void EndElement(const ::rtl::OUString& rName,
                        sal_Bool bIgnWSInside );
    void IgnorableWhitespace();

    /**
     * Record an error condition that occured during export. The
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

    /** return current error flags (logical 'or' of all error flags so far) */
    sal_uInt16 GetErrorFlags()  { return mnErrorFlags; }

    virtual void DisposingModel();

    // #110680#
    ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory > getServiceFactory();
};

inline UniReference< XMLTextParagraphExport > SvXMLExport::GetTextParagraphExport()
{
    if( !mxTextParagraphExport.is() )
        mxTextParagraphExport = CreateTextParagraphExport();

    return mxTextParagraphExport;
}

inline UniReference< XMLShapeExport > SvXMLExport::GetShapeExport()
{
    if( !mxShapeExport.is() )
        mxShapeExport = CreateShapeExport();

    return mxShapeExport;
}

inline UniReference< SvXMLAutoStylePoolP > SvXMLExport::GetAutoStylePool()
{
    if( !mxAutoStylePool.is() )
        mxAutoStylePool = CreateAutoStylePool();

    return mxAutoStylePool;
}

inline UniReference< SchXMLExportHelper > SvXMLExport::GetChartExport()
{
    if( !mxChartExport.is() )
        mxChartExport = CreateChartExport();

    return mxChartExport;
}

inline UniReference< XMLPageExport > SvXMLExport::GetPageExport()
{
    if( !mxPageExport.is() )
        mxPageExport = CreatePageExport();

    return mxPageExport;
}

inline UniReference< XMLFontAutoStylePool > SvXMLExport::GetFontAutoStylePool()
{
    if( !mxFontAutoStylePool.is() )
        mxFontAutoStylePool = CreateFontAutoStylePool();

    return mxFontAutoStylePool;
}

inline UniReference< xmloff::OFormLayerXMLExport > SvXMLExport::GetFormExport()
{
    if( !mxFormExport.is() )
        mxFormExport = CreateFormExport();

    return mxFormExport;
}

inline bool SvXMLExport::HasFormExport()
{
    return mxFormExport.is();
}

inline void SvXMLExport::SetEmbeddedResolver(
    ::com::sun::star::uno::Reference< ::com::sun::star::document::XEmbeddedObjectResolver >& _xEmbeddedResolver )
{
    xEmbeddedResolver = _xEmbeddedResolver;
}

inline void SvXMLExport::SetGraphicResolver(
    ::com::sun::star::uno::Reference< ::com::sun::star::document::XGraphicObjectResolver >& _xGraphicResolver )
{
    xGraphicResolver = _xGraphicResolver;
}

// Helper class to export an element.
class SvXMLElementExport
{
    SvXMLExport& rExport;
    ::rtl::OUString aName;
    sal_Bool bIgnWS : 1;
    sal_Bool bDoSomething : 1;

    void StartElement( SvXMLExport& rExp, sal_uInt16 nPrefix,
                       const ::rtl::OUString& rName,
                       sal_Bool bIgnWSOutside );

public:

    // The constructor prints a start tag that has the common attributes
    // of the XMLExport instance attached.
    SvXMLElementExport( SvXMLExport& rExp, sal_uInt16 nPrefix,
                        const sal_Char *pName,
                        sal_Bool bIgnWSOutside, sal_Bool bIgnWSInside );
    SvXMLElementExport( SvXMLExport& rExp, sal_uInt16 nPrefix,
                        const ::rtl::OUString& rName,
                        sal_Bool bIgnWSOutside, sal_Bool bIgnWSInside );
    SvXMLElementExport( SvXMLExport& rExp, sal_uInt16 nPrefix,
                        enum ::binfilter::xmloff::token::XMLTokenEnum eName,
                        sal_Bool bIgnWSOutside, sal_Bool bIgnWSInside );
    SvXMLElementExport( SvXMLExport& rExp, const ::rtl::OUString& rQName,
                        sal_Bool bIgnWSOutside, sal_Bool bIgnWSInside );

    // Thes constructors do nothing if bDoSomething is not set
    SvXMLElementExport( SvXMLExport& rExp, sal_Bool bDoSomething,
                        sal_uInt16 nPrefix,
                        enum ::binfilter::xmloff::token::XMLTokenEnum eName,
                        sal_Bool bIgnWSOutside, sal_Bool bIgnWSInside );

    // The destructor prints an end tag.
    ~SvXMLElementExport();
};

}//end of namespace binfilter
#endif	//  _XMLOFF_SVXMLEXP_HXX

