#ifndef RPT_XMLCELL_HXX
#define RPT_XMLCELL_HXX
/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: xmlCell.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: hr $ $Date: 2007-08-03 09:56:15 $
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

namespace rptxml
{
    class ORptFilter;
    class OXMLTable;
    class OXMLCell : public SvXMLImportContext
    {
        ::com::sun::star::uno::Reference< ::com::sun::star::report::XReportComponent >  m_xComponent;
        OXMLTable*      m_pContainer;
        OXMLCell*       m_pCell;
          ::rtl::OUString m_sStyleName;
        ::rtl::OUString m_sText;
        sal_Int32       m_nCurrentCount;
        bool            m_bContainsShape;

        ORptFilter& GetOwnImport();
        OXMLCell(const OXMLCell&);
        void operator =(const OXMLCell&);
    public:

        OXMLCell( ORptFilter& rImport
                    ,sal_uInt16 nPrfx
                    ,const ::rtl::OUString& rLName
                    ,const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XAttributeList > & xAttrList
                    ,OXMLTable* _pContainer
                    ,OXMLCell* _pCell = NULL);
        virtual ~OXMLCell();

        virtual SvXMLImportContext *CreateChildContext( sal_uInt16 nPrefix,
                    const ::rtl::OUString& rLocalName,
                    const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XAttributeList > & xAttrList );

        virtual void Characters( const ::rtl::OUString& rChars );
        virtual void EndElement();

        void setComponent(const ::com::sun::star::uno::Reference< ::com::sun::star::report::XReportComponent >& _xComponent);
    };
// -----------------------------------------------------------------------------
} // namespace rptxml
// -----------------------------------------------------------------------------

#endif // RPT_XMLCELL_HXX
