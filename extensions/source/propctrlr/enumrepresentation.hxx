/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: enumrepresentation.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2006-07-26 07:55:08 $
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

#ifndef EXTENSIONS_SOURCE_PROPCTRLR_ENUMREPRESENTATION_HXX
#define EXTENSIONS_SOURCE_PROPCTRLR_ENUMREPRESENTATION_HXX

/** === begin UNO includes === **/
#ifndef _COM_SUN_STAR_UNO_ANY_HXX_
#include <com/sun/star/uno/Any.hxx>
#endif
/** === end UNO includes === **/

#ifndef _RTL_REF_HXX_
#include <rtl/ref.hxx>
#endif
#ifndef _RTL_USTRING_HXX_
#include <rtl/ustring.hxx>
#endif

#include <vector>

//........................................................................
namespace pcr
{
//........................................................................

    //====================================================================
    //= IPropertyEnumRepresentation
    //====================================================================
    class SAL_NO_VTABLE IPropertyEnumRepresentation : public ::rtl::IReference
    {
    public:
        /** retrieves all descriptions of all possible values of the enumeration property
        */
        virtual ::std::vector< ::rtl::OUString > SAL_CALL getDescriptions(
            ) const = 0;

        /** converts a given description into a property value
        */
        virtual void SAL_CALL getValueFromDescription(
                const ::rtl::OUString& _rDescription,
                ::com::sun::star::uno::Any& _out_rValue
            ) const = 0;

        /** converts a given property value into a description
        */
        virtual ::rtl::OUString SAL_CALL getDescriptionForValue(
                const ::com::sun::star::uno::Any& _rEnumValue
            ) const = 0;

        virtual ~IPropertyEnumRepresentation() { };
    };

//........................................................................
} // namespace pcr
//........................................................................

#endif // EXTENSIONS_SOURCE_PROPCTRLR_ENUMREPRESENTATION_HXX

