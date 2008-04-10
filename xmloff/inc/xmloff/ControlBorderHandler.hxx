/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: ControlBorderHandler.hxx,v $
 * $Revision: 1.3 $
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

#ifndef _XMLOFF_CONTROLBORDERHDL_HXX_
#define _XMLOFF_CONTROLBORDERHDL_HXX_

#include <xmloff/prhdlfac.hxx>
#include <vos/ref.hxx>
#include <rtl/ustring.hxx>
#include <xmloff/xmlexppr.hxx>

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
