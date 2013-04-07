/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

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
                                 OUString              ,
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
        virtual sal_uInt16 mapIdentifierToCode(const OUString& sIdentifier)
            throw(css::lang::IllegalArgumentException);

        //----------------------------------
        /** @short  return a suitable key identifier
                    for the specified key code.

            @param  nCode
                    short value, which describe the key.

            @return The corresponding string identifier.
         */
        virtual OUString mapCodeToIdentifier(sal_uInt16 nCode);

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
        sal_Bool impl_st_interpretIdentifierAsPureKeyCode(const OUString& sIdentifier,
                                                                sal_uInt16&      rCode      );
};

} // namespace framework

#endif // __FRAMEWORK_ACCELERATORS_KEYMAPPING_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
