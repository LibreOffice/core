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
#ifndef _XMLOFF_PROPERTYSETINFOKEY_HXX
#define _XMLOFF_PROPERTYSETINFOKEY_HXX

#ifndef _OSL_DIAGNOSE_H_ 
#include <osl/diagnose.h>
#endif
#ifndef _COM_SUN_STAR_UNO_SEQUENCE_HXX_
#include <com/sun/star/uno/Sequence.hxx>
#endif

namespace com { namespace sun { namespace star {
    namespace beans { class XPropertySetInfo; }
} } }
namespace binfilter {

struct PropertySetInfoKey
{
    ::com::sun::star::uno::Reference < 
        ::com::sun::star::beans::XPropertySetInfo >	xPropInfo;
    ::com::sun::star::uno::Sequence < sal_Int8 > aImplementationId;

    inline PropertySetInfoKey();
    inline PropertySetInfoKey( 
            const ::com::sun::star::uno::Reference < 
                    ::com::sun::star::beans::XPropertySetInfo >& rPropInfo,
            const ::com::sun::star::uno::Sequence < sal_Int8 >& rImplId );
};

inline PropertySetInfoKey::PropertySetInfoKey()
{
    OSL_ENSURE( aImplementationId.getLength()==16, "illegal constructor call" );
}

inline PropertySetInfoKey::PropertySetInfoKey(
        const ::com::sun::star::uno::Reference < 
            ::com::sun::star::beans::XPropertySetInfo >& rPropInfo,
        const ::com::sun::star::uno::Sequence < sal_Int8 >& rImplId ) :
    xPropInfo( rPropInfo ),
    aImplementationId( rImplId )
{
    OSL_ENSURE( rPropInfo.is(), "prop info missing" );
    OSL_ENSURE( aImplementationId.getLength()==16, "invalid implementation id" );
}

}//end of namespace binfilter
#endif
