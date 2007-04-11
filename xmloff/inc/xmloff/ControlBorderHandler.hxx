/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: ControlBorderHandler.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2007-04-11 13:19:15 $
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

#ifndef _XMLOFF_CONTROLBORDERHDL_HXX_
#define _XMLOFF_CONTROLBORDERHDL_HXX_

#ifndef _XMLOFF_PROPERTYHANDLERFACTORY_HXX
#include <xmloff/prhdlfac.hxx>
#endif
#ifndef _VOS_REF_HXX_
#include <vos/ref.hxx>
#endif
#ifndef _RTL_USTRING_HXX_
#include <rtl/ustring.hxx>
#endif
#ifndef _XMLOFF_XMLEXPPR_HXX
#include <xmloff/xmlexppr.hxx>
#endif

//.........................................................................
namespace xmloff
{
//.........................................................................

    //=====================================================================
    //= OControlBorderHandlerFactory
    //=====================================================================
    class OControlBorderHandlerFactory
    {
    private:
        OControlBorderHandlerFactory(); // never implemented

    public:
        static const XMLPropertyHandler* createBorderHandler();
        static const XMLPropertyHandler* createBorderColorHandler();
    };

    //=====================================================================
    //= OControlTextEmphasisHandler
    //=====================================================================
    class OControlTextEmphasisHandler : public XMLPropertyHandler
    {
    public:
        OControlTextEmphasisHandler();

        virtual sal_Bool importXML( const ::rtl::OUString& _rStrImpValue, ::com::sun::star::uno::Any& _rValue, const SvXMLUnitConverter& _rUnitConverter ) const;
        virtual sal_Bool exportXML( ::rtl::OUString& _rStrExpValue, const ::com::sun::star::uno::Any& _rValue, const SvXMLUnitConverter& _rUnitConverter ) const;
    };

//.........................................................................
}   // namespace xmloff
//.........................................................................

#endif // _XMLOFF_CONTROLBORDERHDL_HXX_
