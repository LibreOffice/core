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

