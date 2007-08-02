#ifndef RPT_XMLFILTER_HXX
#define RPT_XMLFILTER_HXX
/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: xmlfilter.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: hr $ $Date: 2007-08-02 14:34:48 $
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

#ifndef _COM_SUN_STAR_CONTAINER_XNAMED_HPP_
#include <com/sun/star/container/XNamed.hpp>
#endif
#ifndef _COM_SUN_STAR_DOCUMENT_XFILTER_HPP_
#include <com/sun/star/document/XFilter.hpp>
#endif
#ifndef _COM_SUN_STAR_DOCUMENT_XIMPORTER_HPP_
#include <com/sun/star/document/XImporter.hpp>
#endif
#ifndef _COM_SUN_STAR_DOCUMENT_XEXPORTER_HPP_
#include <com/sun/star/document/XExporter.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XINITIALIZATION_HPP_
#include <com/sun/star/lang/XInitialization.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XSERVICEINFO_HPP_
#include <com/sun/star/lang/XServiceInfo.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSET_HPP_
#include <com/sun/star/beans/XPropertySet.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XCOMPONENT_HPP_
#include <com/sun/star/lang/XComponent.hpp>
#endif
#ifndef _CPPUHELPER_IMPLBASE1_HXX_
#include <cppuhelper/implbase1.hxx>
#endif
#ifndef _CPPUHELPER_IMPLBASE5_HXX_
#include <cppuhelper/implbase5.hxx>
#endif
#ifndef _COM_SUN_STAR_LANG_XMULTISERVICEFACTORY_HPP_
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#endif
#ifndef _COM_SUN_STAR_IO_XACTIVEDATASOURCE_HPP_
#include <com/sun/star/io/XActiveDataSource.hpp>
#endif
#ifndef _COM_SUN_STAR_REPORT_XREPORTDEFINITION_HPP_
#include <com/sun/star/report/XReportDefinition.hpp>
#endif
#ifndef _OSL_DIAGNOSE_H_
#include <osl/diagnose.h>
#endif
#ifndef _UNOTOOLS_TEMPFILE_HXX
#include <unotools/tempfile.hxx>
#endif
#ifndef _UNOTOOLS_LOCALFILEHELPER_HXX
#include <unotools/localfilehelper.hxx>
#endif
#ifndef _UNTOOLS_UCBSTREAMHELPER_HXX
#include <unotools/ucbstreamhelper.hxx>
#endif
#ifndef _XMLOFF_XMLIMP_HXX
#include <xmloff/xmlimp.hxx>
#endif
#ifndef _COMPHELPER_STLTYPES_HXX_
#include <comphelper/stl_types.hxx>
#endif
#ifndef _COMPHELPER_SEQUENCE_HXX_
#include <comphelper/sequence.hxx>
#endif
#ifndef _COM_SUN_STAR_UNO_XCOMPONENTCONTEXT_HPP_
#include <com/sun/star/uno/XComponentContext.hpp>
#endif
#include <memory>
#include <boost/shared_ptr.hpp>

#ifndef _XMLOFF_PROPERTYHANDLERFACTORY_HXX
#include <xmloff/prhdlfac.hxx>
#endif
#ifndef _XMLOFF_PROPERTYSETMAPPER_HXX
#include <xmloff/xmlprmap.hxx>
#endif

namespace rptui
{
    class OReportModel;
}
namespace rptxml
{
using namespace ::xmloff::token;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::document;
using namespace ::com::sun::star::text;
using namespace ::com::sun::star::io;
using namespace ::com::sun::star::report;
using namespace ::com::sun::star::xml::sax;

// -------------
// - ORptFilter -
// -------------
class ORptFilter : public SvXMLImport
{
public:
    DECLARE_STL_USTRINGACCESS_MAP(Sequence<PropertyValue>,TPropertyNameMap);
    DECLARE_STL_USTRINGACCESS_MAP( Reference<XFunction> ,TGroupFunctionMap);
private:

    TGroupFunctionMap                               m_aFunctions;
    com::sun::star::uno::Any                        m_aViewSettings;
    Reference< XComponent >                         m_xSrcDoc;
    mutable ::std::auto_ptr<SvXMLTokenMap>          m_pDocElemTokenMap;
    mutable ::std::auto_ptr<SvXMLTokenMap>          m_pReportElemTokenMap;
    mutable ::std::auto_ptr<SvXMLTokenMap>          m_pGroupsElemTokenMap;
    mutable ::std::auto_ptr<SvXMLTokenMap>          m_pGroupElemTokenMap;
    mutable ::std::auto_ptr<SvXMLTokenMap>          m_pSectionElemTokenMap;
    mutable ::std::auto_ptr<SvXMLTokenMap>          m_pComponentElemTokenMap;
    mutable ::std::auto_ptr<SvXMLTokenMap>          m_pElemTokenMap;
    mutable ::std::auto_ptr<SvXMLTokenMap>          m_pControlElemTokenMap;
    mutable ::std::auto_ptr<SvXMLTokenMap>          m_pFunctionElemTokenMap;
    mutable ::std::auto_ptr<SvXMLTokenMap>          m_pSubDocumentElemTokenMap;
    mutable ::std::auto_ptr<SvXMLTokenMap>          m_pFormatElemTokenMap;
    mutable ::std::auto_ptr<SvXMLTokenMap>          m_pColumnTokenMap;
    mutable ::std::auto_ptr<SvXMLTokenMap>          m_pCellElemTokenMap;

    UniReference < XMLPropertyHandlerFactory >      m_xPropHdlFactory;
    UniReference < XMLPropertySetMapper >           m_xCellStylesPropertySetMapper;
    UniReference < XMLPropertySetMapper >           m_xColumnStylesPropertySetMapper;
    UniReference < XMLPropertySetMapper >           m_xRowStylesPropertySetMapper;
    UniReference < XMLPropertySetMapper >           m_xTableStylesPropertySetMapper;

    Reference<XReportDefinition>                    m_xReportDefinition;
    ::boost::shared_ptr<rptui::OReportModel>        m_pReportModel;
    sal_Int32                                       m_nPreviewMode;

    sal_Bool                            implImport( const Sequence< PropertyValue >& rDescriptor ) throw (RuntimeException);

    SvXMLImportContext* CreateStylesContext(const ::rtl::OUString& rLocalName,
                                     const Reference< XAttributeList>& xAttrList, sal_Bool bIsAutoStyle );
    SvXMLImportContext *CreateFontDeclsContext(const ::rtl::OUString& rLocalName,
            const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XAttributeList > & xAttrList );
protected:
    // SvXMLImport
    virtual SvXMLImportContext *CreateContext( sal_uInt16 nPrefix,
                                      const ::rtl::OUString& rLocalName,
                                      const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XAttributeList >& xAttrList );

    virtual XMLShapeImportHelper* CreateShapeImport();

    virtual ~ORptFilter()  throw();
public:

    ORptFilter( const Reference< XMultiServiceFactory >& _rxMSF,sal_uInt16 nImportFlags = IMPORT_ALL );

    // XFilter
    virtual sal_Bool SAL_CALL filter( const Sequence< PropertyValue >& rDescriptor ) throw(RuntimeException);

    // ::com::sun::star::lang::XServiceInfo
    virtual ::sal_Bool SAL_CALL supportsService( const ::rtl::OUString& ServiceName ) throw (::com::sun::star::uno::RuntimeException);
    virtual ::rtl::OUString SAL_CALL getImplementationName(  ) throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames(  ) throw(::com::sun::star::uno::RuntimeException);

    static ::com::sun::star::uno::Sequence< ::rtl::OUString > getSupportedServiceNames_Static(void) throw( ::com::sun::star::uno::RuntimeException );
    static ::rtl::OUString getImplementationName_Static(void) throw( ::com::sun::star::uno::RuntimeException );
    static ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > SAL_CALL
        create(::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext > const & xContext);

    inline Reference< XMultiServiceFactory > getORB() { return SvXMLImport::getServiceFactory(); }
    inline Reference<XReportDefinition> getReportDefinition() const { return m_xReportDefinition; }
    /** return the SdrModel of the real model
    *
    * \return
    */
    ::boost::shared_ptr<rptui::OReportModel> getSdrModel() const { return m_pReportModel; }
    void FinishStyles();

    virtual void SAL_CALL startDocument(void)
        throw( ::com::sun::star::xml::sax::SAXException, ::com::sun::star::uno::RuntimeException );
    virtual void SAL_CALL endDocument(void)
        throw( ::com::sun::star::xml::sax::SAXException, ::com::sun::star::uno::RuntimeException );

    const SvXMLTokenMap& GetDocElemTokenMap() const;
    const SvXMLTokenMap& GetReportElemTokenMap() const;
    const SvXMLTokenMap& GetGroupElemTokenMap() const;
    const SvXMLTokenMap& GetSectionElemTokenMap() const;
    const SvXMLTokenMap& GetComponentElemTokenMap() const;
    const SvXMLTokenMap& GetReportElementElemTokenMap() const;
    const SvXMLTokenMap& GetControlElemTokenMap() const;
    const SvXMLTokenMap& GetControlPropertyElemTokenMap() const;
    const SvXMLTokenMap& GetFunctionElemTokenMap() const;
    const SvXMLTokenMap& GetFormatElemTokenMap() const;
    const SvXMLTokenMap& GetSubDocumentElemTokenMap() const;
    const SvXMLTokenMap& GetColumnTokenMap() const;
    const SvXMLTokenMap& GetCellElemTokenMap() const;

    inline UniReference < XMLPropertySetMapper > GetCellStylesPropertySetMapper()      const   { return m_xCellStylesPropertySetMapper;    }
    inline UniReference < XMLPropertySetMapper > GetColumnStylesPropertySetMapper()    const   { return m_xColumnStylesPropertySetMapper;  }
    inline UniReference < XMLPropertySetMapper > GetRowStylesPropertySetMapper()       const   { return m_xRowStylesPropertySetMapper;     }
    inline UniReference < XMLPropertySetMapper > GetTableStylesPropertySetMapper()     const   { return m_xTableStylesPropertySetMapper;   }
    static ::rtl::OUString convertFormula(const ::rtl::OUString& _sFormula);
    /** inserts a new function
    *
    * \param _xFunction
    */
    void insertFunction(const ::com::sun::star::uno::Reference< ::com::sun::star::report::XFunction > & _xFunction);
    void removeFunction(const ::rtl::OUString& _sFunctionName);
    inline const TGroupFunctionMap& getFunctions() const { return m_aFunctions; }

    virtual SvXMLImport&                getGlobalContext();

    virtual void                        enterEventContext();
    virtual void                        leaveEventContext();
};

/** Imports only settings
 * \ingroup reportdesign_source_filter_xml
 *
 */
class ORptImportHelper
{
public:
    static ::rtl::OUString getImplementationName_Static(  ) throw (::com::sun::star::uno::RuntimeException);
    static Sequence< ::rtl::OUString > getSupportedServiceNames_Static(  ) throw(::com::sun::star::uno::RuntimeException);
    static ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > SAL_CALL
        create(::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext > const & xContext);
};

/** Imports only content
 * \ingroup reportdesign_source_filter_xml
 *
 */
class ORptContentImportHelper
{
public:
    static ::rtl::OUString getImplementationName_Static(  ) throw (::com::sun::star::uno::RuntimeException);
    static Sequence< ::rtl::OUString > getSupportedServiceNames_Static(  ) throw(::com::sun::star::uno::RuntimeException);
    static ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > SAL_CALL
        create(::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext > const & xContext);
};

/** Imports only styles
 * \ingroup reportdesign_source_filter_xml
 *
 */
class ORptStylesImportHelper
{
public:
    static ::rtl::OUString getImplementationName_Static(  ) throw (::com::sun::star::uno::RuntimeException);
    static Sequence< ::rtl::OUString > getSupportedServiceNames_Static(  ) throw(::com::sun::star::uno::RuntimeException);
    static ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > SAL_CALL
        create(::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext > const & xContext);
};

// -----------------------------------------------------------------------------
} // rptxml
// -----------------------------------------------------------------------------
#endif // RPT_XMLFILTER_HXX
