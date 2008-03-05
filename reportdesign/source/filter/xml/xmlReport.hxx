#ifndef RPT_XMLREPORT_HXX
#define RPT_XMLREPORT_HXX
/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: xmlReport.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: kz $ $Date: 2008-03-05 18:05:00 $
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

#ifndef RPT_XMLREPORTELEMENTBASE_HXX
#include "xmlReportElementBase.hxx"
#endif
#include <vector>

namespace rptxml
{
    class ORptFilter;
    class OXMLReport : public OXMLReportElementBase, public IMasterDetailFieds
    {
        ::com::sun::star::uno::Reference< ::com::sun::star::report::XReportDefinition > m_xComponent;
        ::std::vector< ::rtl::OUString> m_aMasterFields;
        ::std::vector< ::rtl::OUString> m_aDetailFields;
        OXMLReport(const OXMLReport&);
        void operator =(const OXMLReport&);
    public:

        OXMLReport( ORptFilter& rImport, sal_uInt16 nPrfx,
                    const ::rtl::OUString& rLName,
                    const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XAttributeList > & xAttrList
                    ,const ::com::sun::star::uno::Reference< ::com::sun::star::report::XReportDefinition >& _xComponent
                    ,OXMLTable* _pContainer);
        virtual ~OXMLReport();

        virtual SvXMLImportContext *CreateChildContext( sal_uInt16 nPrefix,
                    const ::rtl::OUString& rLocalName,
                    const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XAttributeList > & xAttrList );

        virtual void EndElement();

        virtual void addMasterDetailPair(const ::std::pair< ::rtl::OUString,::rtl::OUString >& _aPair);

    private:
        /** initializes our object's properties whose runtime (API) default is different from the file
            format default.
        */
        void    impl_initRuntimeDefaults() const;
    };
// -----------------------------------------------------------------------------
} // namespace rptxml
// -----------------------------------------------------------------------------

#endif // RPT_XMLREPORT_HXX
