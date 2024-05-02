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

#pragma once

#include <rtl/ustring.hxx>
#include <unordered_map>

// definition

namespace framework
{

/**
    can be used to map key identifier to the
    corresponding key codes ...
 */
class KeyMapping
{

    // const, types

    private:

        /** @short  is used to map a key code
                    to the right key identifier, which is
                    used to make the xml file "human readable"
         */
        struct KeyIdentifierInfo
        {
            sal_Int16       Code;
            OUString        Identifier;
        };

        /** @short  hash structure to map identifier to key codes. */
        typedef std::unordered_map<OUString, sal_Int16> Identifier2CodeHash;

        /** @short  hash structure to map key codes to identifier. */
        typedef std::unordered_map<sal_Int16, OUString> Code2IdentifierHash;

    // member

    private:

        static KeyIdentifierInfo const KeyIdentifierMap[];

        /** @short  hash to map identifier to key codes. */
        Identifier2CodeHash m_lIdentifierHash;

        /** @short  hash to map key codes to identifier. */
        Code2IdentifierHash m_lCodeHash;

    // interface

    public:

                 KeyMapping();

        static KeyMapping & get();

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
        sal_uInt16 mapIdentifierToCode(const OUString& sIdentifier);

        /** @short  return a suitable key identifier
                    for the specified key code.

            @param  nCode
                    short value, which describe the key.

            @return The corresponding string identifier.
         */
        OUString mapCodeToIdentifier(sal_uInt16 nCode);

    // helper

    private:

        /** @short  check if the given string describe a numeric
                    value ... and convert it.

            @param  sIdentifier
                    the string value, which should be converted.

            @param  rCode
                    contains the converted code, but is defined only
                    if this method returns sal_True!

            @return [boolean]
                    sal_True if conversion was successful.
          */
        static bool impl_st_interpretIdentifierAsPureKeyCode(std::u16string_view sIdentifier,
                                                                sal_uInt16&      rCode      );
};

} // namespace framework

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
