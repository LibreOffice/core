/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#ifndef _UNOTOOLS_UNOTUNNELHLP_HXX
#define _UNOTOOLS_UNOTUNNELHLP_HXX

#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/uno/Sequence.hxx>
#include <com/sun/star/lang/XUnoTunnel.hpp>
#include <rtl/uuid.h>
#include <rtl/memory.h>
#include <comphelper/extract.hxx>

namespace utl
{
namespace staruno = ::com::sun::star::uno;
namespace starlang = ::com::sun::star::lang;

//-----------------------------------------------------------------------------------------------------------
// to use the following, define
// sal_Bool getTunneledImplementation(Classname*& pObjImpl, staruno::Reference<starlang::XUnoTunnel> const& xObj);

template <class Classname>
sal_Bool getImplementation(Classname*& pImpl, staruno::Reference<starlang::XUnoTunnel> const& xTunnel)
    throw(staruno::RuntimeException)
{
    if (xTunnel.is())
        return getTunneledImplementation(pImpl, xTunnel);

    pImpl = 0;
    return sal_False;
}

template <class Classname>
sal_Bool getImplementation(Classname*& pImpl, staruno::Reference<staruno::XInterface> const& xObj)
    throw(staruno::RuntimeException)
{
    staruno::Reference<starlang::XUnoTunnel> xTunnel(xObj,staruno::UNO_QUERY);
    if (xTunnel.is())
        return getTunneledImplementation(pImpl, xTunnel);

    pImpl = 0;
    return sal_False;
}

template <class Classname>
sal_Bool getImplementation(Classname*& pImpl, staruno::Any const& aObj)
    throw(staruno::RuntimeException)
{
    staruno::Reference<starlang::XUnoTunnel> xTunnel;
    if (cppu::extractInterface(xTunnel, aObj))
        getTunneledImplementation(pImpl, xTunnel);

    pImpl = 0;
    return sal_False;
}

template <class Classname>
sal_Bool getImplementation(Classname*& pImpl, starlang::XUnoTunnel* pObj)
    throw(staruno::RuntimeException)
{
    if (pObj)
    {
        staruno::Reference<starlang::XUnoTunnel> xTunnel(pObj);
        return getTunneledImplementation(pImpl, xTunnel);
    }

    pImpl = 0;
    return sal_False;
}

//-----------------------------------------------------------------------------------------------------------

class UnoTunnelId
{
    sal_Int8 tunnelId[16];
public:
    UnoTunnelId(sal_Bool bUseMAC = sal_True) throw()
    {
        rtl_createUuid(reinterpret_cast<sal_uInt8*>(tunnelId),0,bUseMAC);
    }

    staruno::Sequence<sal_Int8> getId() const throw(staruno::RuntimeException)
    {
        return staruno::Sequence<sal_Int8>(tunnelId, sizeof(tunnelId));
    }

    sal_Bool equalTo(staruno::Sequence<sal_Int8> const& rIdentifier) throw()
    {
        return  rIdentifier.getLength() == sizeof(tunnelId) &&
                rtl_compareMemory(tunnelId, rIdentifier.getConstArray(), sizeof(tunnelId)) == 0;
    }

    sal_Int8 const (&getIdBytes() const)[16] { return tunnelId; }
};

//-----------------------------------------------------------------------------------------------------------
template<class Classname>
class UnoTunnelImplBase
{
protected:
    Classname* ThisImplementation() throw() { return static_cast<Classname*>(this); }

    sal_Int64 makeUnoSomething() throw()
    {
        return reinterpret_cast<sal_Int64>(static_cast<void*>(ThisImplementation()));
    }

    static Classname* extractUnoSomething(sal_Int64 nSomething) throw()
    {
        if (nSomething != sal_Int64())
            return static_cast<Classname*>(reinterpret_cast<void*>(nSomething));

        return NULL;
    }
#ifdef LINUX
public:
#endif
    static Classname*
        extractUnoSomething(
            staruno::Reference<starlang::XUnoTunnel> const& xObj,
            staruno::Sequence<sal_Int8> const& rMyTunnelId
        )
        throw(staruno::RuntimeException)
    {
        return xObj.is() ? extractUnoSomething(xObj->getSomething(rMyTunnelId)) : NULL;
    }
};
//-----------------------------------------------------------------------------------------------------------


template<class Classname>
class UnoTunnelHelper : public UnoTunnelImplBase<Classname>
{
protected:
    static UnoTunnelId s_aTunnelId;

    sal_Int64 getSomething(staruno::Sequence<sal_Int8> const& rTunnelId) throw()
    {
        if (s_aTunnelId.equalTo(rTunnelId))
            return this->makeUnoSomething();
        else
            return sal_Int64();
    }
public:
    static staruno::Sequence<sal_Int8> getImplementationTunnelId()
        throw(staruno::RuntimeException)
    {
        return s_aTunnelId.getId();
    }
#ifndef LINUX
    friend sal_Bool getTunneledImplementation(Classname*& pImpl, staruno::Reference<starlang::XUnoTunnel> const& xObj)
        throw(staruno::RuntimeException)
    {
        pImpl = UnoTunnelHelper<Classname>::UnoTunnelHelper<Classname>::extractUnoSomething( xObj, UnoTunnelHelper<Classname>::getImplementationTunnelId() );

        return pImpl != 0;
    }
#endif
};
template<class Classname>
UnoTunnelId UnoTunnelHelper<Classname>::s_aTunnelId;


//-----------------------------------------------------------------------------------------------------------
} // namespace utl

#endif // _UNOTOOLS_UNOTUNNELHLP_HXX

