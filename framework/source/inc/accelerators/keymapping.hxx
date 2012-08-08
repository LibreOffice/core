/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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

#ifndef __FRAMEWORK_ACCELERATORS_KEYMAPPING_HXX_
#define __FRAMEWORK_ACCELERATORS_KEYMAPPING_HXX_

#include <general.h>
#include <stdtypes.h>

#include <com/sun/star/lang/IllegalArgumentException.hpp>

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
        typedef ::boost::unordered_map< sal_Int16                    ,
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
                    if this method returns sal_True!

            @return [boolean]
                    sal_True if convertion was successfully.
          */
        sal_Bool impl_st_interpretIdentifierAsPureKeyCode(const ::rtl::OUString& sIdentifier,
                                                                sal_uInt16&      rCode      );
};

} // namespace framework

#endif // __FRAMEWORK_ACCELERATORS_KEYMAPPING_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
