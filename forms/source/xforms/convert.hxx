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



#ifndef _CONVERT_HXX
#define _CONVERT_HXX

#include <com/sun/star/uno/Sequence.hxx>
#include <map>

namespace com { namespace sun { namespace star { namespace uno
{
    class Any;
    class Type;
} } } }
namespace rtl { class OUString; }
class ConvertImpl;

namespace xforms
{

struct TypeLess
{
    bool operator()( const com::sun::star::uno::Type& rType1,
                     const com::sun::star::uno::Type& rType2 ) const
    { return rType1.getTypeName() < rType2.getTypeName(); }
};

class Convert
{
    typedef com::sun::star::uno::Type Type_t;
    typedef com::sun::star::uno::Sequence<com::sun::star::uno::Type> Types_t;
    typedef com::sun::star::uno::Any Any_t;

    // hold conversion objects
    typedef rtl::OUString (*fn_toXSD)( const Any_t& );
    typedef Any_t (*fn_toAny)( const rtl::OUString& );
    typedef std::pair<fn_toXSD,fn_toAny> Convert_t;
    typedef std::map<Type_t,Convert_t,TypeLess> Map_t;
    Map_t maMap;

    Convert();

    void init();

public:
    /** get/create Singleton class */
    static Convert& get();

    /// can we convert this type?
    bool hasType( const Type_t& );

    /// get list of convertable types
    Types_t getTypes();

    /// convert any to XML representation
    rtl::OUString toXSD( const Any_t& rAny );

    /// convert XML representation to Any of given type
    Any_t toAny( const rtl::OUString&, const Type_t& );

    /** translates the whitespaces in a given string, according
        to a given <type scope="com::sun::star::xsd">WhiteSpaceTreatment</type>.

        @param _rString
            the string to convert
        @param _nWhitespaceTreatment
            a constant from the <type scope="com::sun::star::xsd">WhiteSpaceTreatment</type> group, specifying
            how to handle whitespaces
        @return
            the converted string
    */
    static ::rtl::OUString convertWhitespace(
            const ::rtl::OUString& _rString,
            sal_Int16   _nWhitespaceTreatment
    );

    /** replace all occurrences 0x08, 0x0A, 0x0D with 0x20
    */
    static ::rtl::OUString replaceWhitespace( const ::rtl::OUString& _rString );

    /** replace all sequences of 0x08, 0x0A, 0x0D, 0x20 with a single 0x20.
        also strip leading/trailing whitespace.
    */
    static ::rtl::OUString collapseWhitespace( const ::rtl::OUString& _rString );
};

} // namespace xforms

#endif
