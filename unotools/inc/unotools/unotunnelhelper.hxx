/*************************************************************************
 *
 *  $RCSfile: unotunnelhelper.hxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 17:03:54 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef _UNOTOOLS_UNOTUNNELHLP_HXX
#define _UNOTOOLS_UNOTUNNELHLP_HXX

#ifndef _COM_SUN_STAR_UNO_REFERENCE_HXX_
#include <com/sun/star/uno/Reference.hxx>
#endif
#ifndef _COM_SUN_STAR_UNO_SEQUENCE_HXX_
#include <com/sun/star/uno/Sequence.hxx>
#endif
#ifndef _COM_SUN_STAR_LANG_XUNOTUNNEL_HPP_
#include <com/sun/star/lang/XUnoTunnel.hpp>
#endif
#ifndef _RTL_UUID_H_
#include <rtl/uuid.h>
#endif
#ifndef _RTL_MEMORY_H_
#include <rtl/memory.h>
#endif

#ifndef _CPPUHELPER_EXTRACT_HXX_
#include <cppuhelper/extract.hxx>
#endif

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
            return makeUnoSomething();
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

