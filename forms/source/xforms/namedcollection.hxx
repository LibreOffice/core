/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: namedcollection.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: hr $ $Date: 2006-06-19 13:04:29 $
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

#ifndef _NAMEDCOLLECTION_HXX
#define _NAMEDCOLLECTION_HXX

#include <collection.hxx>
#include <cppuhelper/implbase1.hxx>
#include <com/sun/star/container/XNameAccess.hpp>

#include <algorithm>

template<class T>
class NamedCollection : public cppu::ImplInheritanceHelper1<
                            Collection<T>,
                            com::sun::star::container::XNameAccess>
{
    using Collection<T>::maItems;
    using Collection<T>::getItem;
    using Collection<T>::hasItem;

public:
    NamedCollection() {}
    virtual ~NamedCollection() {}

    const T& getItem( const rtl::OUString& rName ) const
    {
        OSL_ENSURE( hasItem( rName ), "invalid name" );
        return *findItem( rName );
    }

    bool hasItem( const rtl::OUString& rName ) const
    {
        return findItem( rName ) != maItems.end();
    }

    typedef com::sun::star::uno::Sequence<rtl::OUString> Names_t;
    Names_t getNames() const
    {
        // iterate over members, and collect all those that have names
        std::vector<rtl::OUString> aNames;
        for( typename std::vector<T>::const_iterator aIter = maItems.begin();
             aIter != maItems.end();
             aIter++ )
        {
            com::sun::star::uno::Reference<com::sun::star::container::XNamed>
                xNamed( *aIter, com::sun::star::uno::UNO_QUERY );
            if( xNamed.is() )
                aNames.push_back( xNamed->getName() );
        }

        // copy names to Sequence and return
        Names_t aResult( aNames.size() );
        rtl::OUString* pStrings = aResult.getArray();
        std::copy( aNames.begin(), aNames.end(), pStrings );

        return aResult;
    }

protected:
    typename std::vector<T>::const_iterator findItem( const rtl::OUString& rName ) const
    {
        for( typename std::vector<T>::const_iterator aIter = maItems.begin();
             aIter != maItems.end();
             aIter++ )
        {
            com::sun::star::uno::Reference<com::sun::star::container::XNamed>
                xNamed( *aIter, com::sun::star::uno::UNO_QUERY );
            if( xNamed.is()  &&  xNamed->getName() == rName )
                return aIter;
        }
        return maItems.end();
    }

public:

    // XElementAccess
    virtual typename Collection<T>::Type_t SAL_CALL getElementType()
        throw( typename Collection<T>::RuntimeException_t )
    {
        return Collection<T>::getElementType();
    }

    virtual sal_Bool SAL_CALL hasElements()
        throw( typename Collection<T>::RuntimeException_t )
    {
        return Collection<T>::hasElements();
    }

    // XNameAccess : XElementAccess
    virtual typename Collection<T>::Any_t SAL_CALL getByName(
        const rtl::OUString& aName )
        throw( typename Collection<T>::NoSuchElementException_t,
               typename Collection<T>::WrappedTargetException_t,
               typename Collection<T>::RuntimeException_t )
    {
        if( hasItem( aName ) )
            return com::sun::star::uno::makeAny( getItem( aName ) );
        else
            throw typename Collection<T>::NoSuchElementException_t();

    }

    virtual Names_t SAL_CALL getElementNames()
        throw( typename Collection<T>::RuntimeException_t )
    {
        return getNames();
    }

    virtual sal_Bool SAL_CALL hasByName(
        const rtl::OUString& aName )
        throw( typename Collection<T>::RuntimeException_t )
    {
        return hasItem( aName ) ? sal_True : sal_False;
    }
};

#endif
