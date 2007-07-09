#ifndef RPT_XMLCONDPRTEXPR_HXX
#define RPT_XMLCONDPRTEXPR_HXX
/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: xmlCondPrtExpr.hxx,v $
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

#ifndef _XMLOFF_XMLICTXT_HXX
#include <xmloff/xmlictxt.hxx>
#endif
#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSET_HPP_
#include <com/sun/star/beans/XPropertySet.hpp>
#endif

namespace rptxml
{
    class ORptFilter;
    class OXMLCondPrtExpr : public SvXMLImportContext
    {
        ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >   m_xComponent;
        OXMLCondPrtExpr(const OXMLCondPrtExpr&);
        void operator =(const OXMLCondPrtExpr&);
    public:

        OXMLCondPrtExpr( ORptFilter& _rImport
                    ,sal_uInt16 nPrfx
                    ,const ::rtl::OUString& rLName
                    ,const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XAttributeList > & _xAttrList
                    ,const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& _xComponent);
        virtual ~OXMLCondPrtExpr();

        // This method is called for all characters that are contained in the
        // current element. The default is to ignore them.
        virtual void Characters( const ::rtl::OUString& rChars );
    };
// -----------------------------------------------------------------------------
} // namespace rptxml
// -----------------------------------------------------------------------------

#endif // RPT_XMLCONDPRTEXPR_HXX
