/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



#ifndef __FRAMEWORK_ACCELERATORS_KEYMAPPING_HXX_
#define __FRAMEWORK_ACCELERATORS_KEYMAPPING_HXX_

//__________________________________________
// own includes

#include <general.h>
#include <stdtypes.h>

//__________________________________________
// interface includes
#include <com/sun/star/lang/IllegalArgumentException.hpp>

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
                    if this method returns sal_True!

            @return [boolean]
                    sal_True if conversion was successfully.
          */
        sal_Bool impl_st_interpretIdentifierAsPureKeyCode(const ::rtl::OUString& sIdentifier,
                                                                sal_uInt16&      rCode      );
};

} // namespace framework

#endif // __FRAMEWORK_ACCELERATORS_KEYMAPPING_HXX_
