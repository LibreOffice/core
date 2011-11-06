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



#ifndef ACCESSIBILITY_HELPER_CHARACTERATTRIBUTESHELPER_HXX
#define ACCESSIBILITY_HELPER_CHARACTERATTRIBUTESHELPER_HXX

#ifndef _COM_SUN_STAR_UNO_SEQUENCE_HXX
#include <com/sun/star/uno/Sequence.hxx>
#endif
#include <com/sun/star/beans/PropertyValue.hpp>
#include <vcl/font.hxx>

#include <map>

// -----------------------------------------------------------------------------
// class CharacterAttributesHelper
// -----------------------------------------------------------------------------

class CharacterAttributesHelper
{
private:

    typedef ::std::map< ::rtl::OUString, ::com::sun::star::uno::Any, ::std::less< ::rtl::OUString > > AttributeMap;

    AttributeMap    m_aAttributeMap;

public:

    CharacterAttributesHelper( const Font& rFont, sal_Int32 nBackColor, sal_Int32 nColor );
    ~CharacterAttributesHelper();

    ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue > GetCharacterAttributes();
    ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue > GetCharacterAttributes( const ::com::sun::star::uno::Sequence< ::rtl::OUString >& aRequestedAttributes );
};

#endif  // ACCESSIBILITY_HELPER_CHARACTERATTRIBUTESHELPER_HXX
