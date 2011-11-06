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



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_framework.hxx"
#include <uiconfiguration/graphicnameaccess.hxx>

//_________________________________________________________________________________________________________________
//  interface includes
//_________________________________________________________________________________________________________________

//_________________________________________________________________________________________________________________
//  other includes
//_________________________________________________________________________________________________________________

#include <comphelper/sequence.hxx>

using namespace ::com::sun::star;

namespace framework
{

GraphicNameAccess::GraphicNameAccess()
{
}

GraphicNameAccess::~GraphicNameAccess()
{
}

void GraphicNameAccess::addElement( const rtl::OUString& rName, const uno::Reference< graphic::XGraphic >& rElement )
{
    m_aNameToElementMap.insert( NameGraphicHashMap::value_type( rName, rElement ));
}

// XNameAccess
uno::Any SAL_CALL GraphicNameAccess::getByName( const ::rtl::OUString& aName )
throw( container::NoSuchElementException,
       lang::WrappedTargetException,
       uno::RuntimeException)
{
    NameGraphicHashMap::const_iterator pIter = m_aNameToElementMap.find( aName );
    if ( pIter != m_aNameToElementMap.end() )
        return uno::makeAny( pIter->second );
    else
        throw container::NoSuchElementException();
}

uno::Sequence< ::rtl::OUString > SAL_CALL GraphicNameAccess::getElementNames()
throw(::com::sun::star::uno::RuntimeException)
{
    if ( m_aSeq.getLength() == 0 )
    {
        uno::Sequence< rtl::OUString > aSeq( m_aNameToElementMap.size() );
        NameGraphicHashMap::const_iterator pIter = m_aNameToElementMap.begin();
        sal_Int32 i( 0);
        while ( pIter != m_aNameToElementMap.end())
        {
            aSeq[i++] = pIter->first;
            ++pIter;
        }
        m_aSeq = aSeq;
    }

    return m_aSeq;
}

sal_Bool SAL_CALL GraphicNameAccess::hasByName( const ::rtl::OUString& aName )
throw(::com::sun::star::uno::RuntimeException)
{
    NameGraphicHashMap::const_iterator pIter = m_aNameToElementMap.find( aName );
    return ( pIter != m_aNameToElementMap.end() );
}

// XElementAccess
sal_Bool SAL_CALL GraphicNameAccess::hasElements()
throw( uno::RuntimeException )
{
    return ( !m_aNameToElementMap.empty() );
}

uno::Type SAL_CALL GraphicNameAccess::getElementType()
throw( uno::RuntimeException )
{
    return ::getCppuType( (const uno::Reference< graphic::XGraphic > *)NULL );
}

} // namespace framework
