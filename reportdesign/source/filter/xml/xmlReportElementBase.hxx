#ifndef RPT_XMLREPORTELEMENTBASE_HXX
#define RPT_XMLREPORTELEMENTBASE_HXX
/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: xmlReportElementBase.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: kz $ $Date: 2008-03-05 18:05:31 $
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

#ifndef _XMLOFF_XMLICTXT_HXX
#include <xmloff/xmlictxt.hxx>
#endif
#ifndef _COM_SUN_STAR_REPORT_XREPORTCOMPONENT_HPP_
#include <com/sun/star/report/XReportComponent.hpp>
#endif
#include <vector>

namespace rptxml
{
    class ORptFilter;
    class OXMLTable;

    class SAL_NO_VTABLE IMasterDetailFieds
    {
    public:
        virtual void addMasterDetailPair(const ::std::pair< ::rtl::OUString,::rtl::OUString >& _aPair) = 0;
    };

    class OXMLReportElementBase : public SvXMLImportContext
    {
        OXMLReportElementBase(const OXMLReportElementBase&);
        void operator =(const OXMLReportElementBase&);
    protected:
        ORptFilter&   m_rImport;
        OXMLTable*    m_pContainer;
        ::com::sun::star::uno::Reference< ::com::sun::star::report::XReportComponent >      m_xComponent;

        virtual SvXMLImportContext* _CreateChildContext( sal_uInt16 nPrefix,
                    const ::rtl::OUString& rLocalName,
                    const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XAttributeList > & xAttrList );
    public:

        OXMLReportElementBase( ORptFilter& rImport
                    ,sal_uInt16 nPrfx
                    ,const ::rtl::OUString& rLName
                    ,const ::com::sun::star::uno::Reference< ::com::sun::star::report::XReportComponent >& _xComponent
                    ,OXMLTable* _pContainer);
        virtual ~OXMLReportElementBase();

        virtual SvXMLImportContext *CreateChildContext( sal_uInt16 nPrefix,
                    const ::rtl::OUString& rLocalName,
                    const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XAttributeList > & xAttrList );

        virtual void EndElement();
    };
// -----------------------------------------------------------------------------
} // namespace rptxml
// -----------------------------------------------------------------------------

#endif // RPT_XMLREPORTELEMENTBASE_HXX
