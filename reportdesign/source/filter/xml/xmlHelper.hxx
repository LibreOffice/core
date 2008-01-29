#ifndef RPT_XMLHELPER_HXX
#define RPT_XMLHELPER_HXX
/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: xmlHelper.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2008-01-29 13:46:02 $
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

#ifndef _XMLOFF_PROPERTYSETMAPPER_HXX
#include <xmloff/xmlprmap.hxx>
#endif
#ifndef _XMLOFF_CONTEXTID_HXX_
#include <xmloff/contextid.hxx>
#endif
#ifndef _XMLOFF_FORMS_CONTROLPROPERTYHDL_HXX_
#include <xmloff/controlpropertyhdl.hxx>
#endif
#ifndef _COM_SUN_STAR_FRAME_XMODEL_HPP_
#include <com/sun/star/frame/XModel.hpp>
#endif
#ifndef _COM_SUN_STAR_REPORT_XREPORTDEFINITION_HPP_
#include <com/sun/star/report/XReportDefinition.hpp>
#endif
#ifndef _COM_SUN_STAR_UTIL_XNUMBERFORMATSSUPPLIER_HPP_
#include <com/sun/star/util/XNumberFormatsSupplier.hpp>
#endif

#include <memory>

#define CTF_RPT_NUMBERFORMAT                            (XML_DB_CTF_START + 1)
#define CTF_RPT_PARAVERTALIGNMENT                       (XML_DB_CTF_START + 2)

#define XML_STYLE_FAMILY_REPORT_ID              700
#define XML_STYLE_FAMILY_REPORT_NAME            "report-element"
#define XML_STYLE_FAMILY_REPORT_PREFIX          "rptelem"


class SvXMLImport;
class SvXMLExport;
class SvXMLStylesContext;
namespace rptxml
{
    class OPropertyHandlerFactory : public ::xmloff::OControlPropertyHandlerFactory
    {
        OPropertyHandlerFactory(const OPropertyHandlerFactory&);
        void operator =(const OPropertyHandlerFactory&);
    protected:
        mutable ::std::auto_ptr<XMLConstantsPropertyHandler>    m_pDisplayHandler;
        mutable ::std::auto_ptr<XMLPropertyHandler>             m_pTextAlignHandler;
    public:
        OPropertyHandlerFactory();
        virtual ~OPropertyHandlerFactory();

        virtual const XMLPropertyHandler* GetPropertyHandler(sal_Int32 _nType) const;
    };

    class OXMLHelper
    {
    public:
        static UniReference < XMLPropertySetMapper > GetCellStylePropertyMap(bool _bOldFormat = false);

        static const SvXMLEnumMapEntry* GetReportPrintOptions();
        static const SvXMLEnumMapEntry* GetForceNewPageOptions();
        static const SvXMLEnumMapEntry* GetKeepTogetherOptions();
        static const SvXMLEnumMapEntry* GetImagePositionOptions();
        static const SvXMLEnumMapEntry* GetImageAlignOptions();
        static const SvXMLEnumMapEntry* GetCommandTypeOptions();

        static const XMLPropertyMapEntry* GetTableStyleProps();
        static const XMLPropertyMapEntry* GetColumnStyleProps();

        static const XMLPropertyMapEntry* GetRowStyleProps();

        static com::sun::star::uno::Reference< com::sun::star::util::XNumberFormatsSupplier > GetNumberFormatsSupplier(const com::sun::star::uno::Reference< com::sun::star::report::XReportDefinition>& _xReportDefinition);

        static void copyStyleElements(const ::rtl::OUString& _sStyleName,const SvXMLStylesContext* _pAutoStyles,const com::sun::star::uno::Reference< com::sun::star::beans::XPropertySet>& _xProp);
        static com::sun::star::uno::Reference< com::sun::star::beans::XPropertySet> createBorderPropertySet();
    };
// -----------------------------------------------------------------------------
} // rptxml
// -----------------------------------------------------------------------------
#endif // RPT_XMLHELPER_HXX

