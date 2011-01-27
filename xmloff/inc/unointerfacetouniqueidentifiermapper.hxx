/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
#ifndef __COMPHELPER_UNOINTERFACETOUNIQUEIDENTIFIERMAPPER__
#define __COMPHELPER_UNOINTERFACETOUNIQUEIDENTIFIERMAPPER__

#include <map>
#include <rtl/ustring.hxx>
#include <com/sun/star/uno/XInterface.hpp>

namespace comphelper
{

typedef ::std::map< rtl::OUString, const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > > IdMap_t;

class UnoInterfaceToUniqueIdentifierMapper
{
public:
    UnoInterfaceToUniqueIdentifierMapper();

    /** returns a unique identifier for the given uno object. IF a uno object is
        registered more than once, the returned identifier is always the same.
    */
    const rtl::OUString& registerReference( const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >& rInterface );

    /** registers the given uno object with the given identifier.

        @returns
            false, if the given identifier already exists and is not associated with the given interface
    */
    bool registerReference( const rtl::OUString& rIdentifier, const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >& rInterface );

    /** @returns
            the identifier for the given uno object. If this uno object is not already
            registered, an empty string is returned
    */
    const rtl::OUString& getIdentifier( const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >& rInterface ) const;

    /** @returns
        the uno object that is registered with the given identifier. If no uno object
        is registered with the given identifier, an empty reference is returned.
    */
    const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >& getReference( const rtl::OUString& rIdentifier ) const;

private:
    bool findReference( const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >& rInterface, IdMap_t::const_iterator& rIter ) const;
    bool findIdentifier( const rtl::OUString& rIdentifier, IdMap_t::const_iterator& rIter ) const;

    IdMap_t maEntries;
    sal_Int32 mnNextId;
};

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
