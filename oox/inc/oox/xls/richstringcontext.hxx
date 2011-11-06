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



#ifndef OOX_XLS_RICHSTRINGCONTEXT_HXX
#define OOX_XLS_RICHSTRINGCONTEXT_HXX

#include "oox/xls/excelhandlers.hxx"
#include "oox/xls/richstring.hxx"

namespace oox {
namespace xls {

// ============================================================================

class RichStringContext : public WorkbookContextBase
{
public:
    template< typename ParentType >
    explicit            RichStringContext( ParentType& rParent, RichStringRef xString );

protected:
    virtual ::oox::core::ContextHandlerRef onCreateContext( sal_Int32 nElement, const AttributeList& rAttribs );
    virtual void        onCharacters( const ::rtl::OUString& rChars );

private:
    RichStringRef       mxString;       /// Processed string.
    RichStringPortionRef mxPortion;     /// Processed portion in the string.
    RichStringPhoneticRef mxPhonetic;   /// Processed phonetic text portion.
    FontRef             mxFont;         /// Processed font of the portion.
};

// ----------------------------------------------------------------------------

template< typename ParentType >
RichStringContext::RichStringContext( ParentType& rParent, RichStringRef xString ) :
    WorkbookContextBase( rParent ),
    mxString( xString )
{
    OSL_ENSURE( mxString.get(), "RichStringContext::RichStringContext - missing string object" );
}

// ============================================================================

} // namespace xls
} // namespace oox

#endif
