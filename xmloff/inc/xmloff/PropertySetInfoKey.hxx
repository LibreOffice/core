/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: PropertySetInfoKey.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2007-04-11 13:21:06 $
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


struct PropertySetInfoKey
{
    ::com::sun::star::uno::Reference <
        ::com::sun::star::beans::XPropertySetInfo > xPropInfo;
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

#endif
