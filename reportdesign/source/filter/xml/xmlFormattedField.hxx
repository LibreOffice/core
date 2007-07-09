#ifndef RPT_XMLFORMATTEDFIELD_HXX
#define RPT_XMLFORMATTEDFIELD_HXX
/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: xmlFormattedField.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2007-07-09 11:56:17 $
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
#ifndef _COM_SUN_STAR_REPORT_XFORMATTEDFIELD_HPP_
#include <com/sun/star/report/XFormattedField.hpp>
#endif

namespace rptxml
{
    class ORptFilter;
    class OXMLFormattedField : public OXMLReportElementBase
    {
        OXMLFormattedField(const OXMLFormattedField&);
        void operator =(const OXMLFormattedField&);
    public:

        OXMLFormattedField( ORptFilter& rImport
                    ,sal_uInt16 nPrfx
                    ,const ::rtl::OUString& rLName
                    ,const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XAttributeList > & xAttrList
                    ,const ::com::sun::star::uno::Reference< ::com::sun::star::report::XFormattedField >& _xComponent
                    ,OXMLTable* _pContainer
                    ,bool _bPageCount);
        virtual ~OXMLFormattedField();
    };
// -----------------------------------------------------------------------------
} // namespace rptxml
// -----------------------------------------------------------------------------

#endif // RPT_XMLFORMATTEDFIELD_HXX
