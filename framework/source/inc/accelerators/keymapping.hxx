/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: keymapping.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: hr $ $Date: 2006-06-19 11:21:43 $
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

#ifndef __FRAMEWORK_ACCELERATORS_KEYMAPPING_HXX_
#define __FRAMEWORK_ACCELERATORS_KEYMAPPING_HXX_

//__________________________________________
// own includes

#ifndef __FRAMEWORK_GENERAL_H_
#include <general.h>
#endif

#ifndef __FRAMEWORK_STDTYPES_H_
#include <stdtypes.h>
#endif

//__________________________________________
// interface includes

#ifndef _COM_SUN_STAR_LANG_ILLEGALARGUMENTEXCEPTION_HPP_
#include <com/sun/star/lang/IllegalArgumentException.hpp>
#endif

//__________________________________________
// other includes

//__________________________________________
// definition

namespace framework
{

//__________________________________________
/**
    can be used to map key identifier to the
    corresponding key codes ...
 */
class KeyMapping
{
    //______________________________________
    // const, types

    private:

        //---------------------------------------
        /** @short  is used to map a key code
                    to the right key identifier, which is
                    used to make the xml file "human readable"
         */
        struct KeyIdentifierInfo
        {
            sal_Int16       Code      ;
            const char*     Identifier;
        };

        //---------------------------------------
        /** @short  hash structure to map identifier to key codes. */
        typedef BaseHash< sal_Int16 > Identifier2CodeHash;

        //---------------------------------------
        /** @short  hash structure to map key codes to identifier. */
        typedef ::std::hash_map< sal_Int16                    ,
                                 ::rtl::OUString              ,
                                 ShortHashCode                ,
                                 ::std::equal_to< sal_Int16 > > Code2IdentifierHash;

    //______________________________________
    // member

    private:

        static KeyIdentifierInfo KeyIdentifierMap[];

        //---------------------------------------
        /** @short  hash to map identifier to key codes. */
        Identifier2CodeHash m_lIdentifierHash;

        //---------------------------------------
        /** @short  hash to map key codes to identifier. */
        Code2IdentifierHash m_lCodeHash;

    //______________________________________
    // interface

    public:

                 KeyMapping();
        virtual ~KeyMapping();

        //----------------------------------
        /** @short  return a suitable key code
                    for the specified key identifier.

            @param  sIdentifier
                    string value, which describe the key.

            @return [css::awt::KeyEvent]
                    the corresponding key code as
                    short value.

            @throw  [css::lang::IllegalArgumentException]
                    if the given identifier does not describe
                    a well known key code.
         */
        virtual sal_uInt16 mapIdentifierToCode(const ::rtl::OUString& sIdentifier)
            throw(css::lang::IllegalArgumentException);

        //----------------------------------
        /** @short  return a suitable key identifier
                    for the specified key code.

            @param  nCode
                    short value, which describe the key.

            @return The corresponding string identifier.
         */
        virtual ::rtl::OUString mapCodeToIdentifier(sal_uInt16 nCode);

    //______________________________________
    // helper

    private:

        //----------------------------------
        /** @short  check if the given string describe a numeric
                    value ... and convert it.

            @param  sIdentifier
                    the string value, which should be converted.


            @param  rCode
                    contains the converted code, but is defined only
                    if this method returns TRUE!

            @return [boolean]
                    TRUE if convertion was successfully.
          */
        sal_Bool impl_st_interpretIdentifierAsPureKeyCode(const ::rtl::OUString& sIdentifier,
                                                                sal_uInt16&      rCode      );
};

} // namespace framework

#endif // __FRAMEWORK_ACCELERATORS_KEYMAPPING_HXX_
