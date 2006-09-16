/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: graphicnameaccess.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: obo $ $Date: 2006-09-16 14:15:18 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_framework.hxx"

#ifndef __FRAMEWORK_UICONFIGURATION_GRAPHICNAMEACCESS_HXX_
#include <uiconfiguration/graphicnameaccess.hxx>
#endif

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

sal_uInt32 GraphicNameAccess::size() const
{
    return m_aNameToElementMap.size();
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
    return ( m_aNameToElementMap.size() > 0 );
}

uno::Type SAL_CALL GraphicNameAccess::getElementType()
throw( uno::RuntimeException )
{
    return ::getCppuType( (const uno::Reference< graphic::XGraphic > *)NULL );
}

} // namespace framework
