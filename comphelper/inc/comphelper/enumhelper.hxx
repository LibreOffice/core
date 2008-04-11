/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: enumhelper.hxx,v $
 * $Revision: 1.8 $
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

#ifndef _COMPHELPER_ENUMHELPER_HXX_
#define _COMPHELPER_ENUMHELPER_HXX_

#include <vector>
#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/container/XEnumeration.hpp>
#include <com/sun/star/container/XIndexAccess.hpp>
#include <com/sun/star/lang/XEventListener.hpp>
#include <cppuhelper/implbase1.hxx>
#include <cppuhelper/implbase2.hxx>
#include <osl/mutex.hxx>
#include "comphelper/comphelperdllapi.h"

//.........................................................................
namespace comphelper
{
//.........................................................................

    namespace starcontainer     = ::com::sun::star::container;
    namespace staruno           = ::com::sun::star::uno;
    namespace starlang          = ::com::sun::star::lang;

//==================================================================
//= OEnumerationLock
//==================================================================
struct OEnumerationLock
{
    public:
        ::osl::Mutex m_aLock;
};

//==================================================================
//= OEnumerationByName
//==================================================================
/** provides an <type scope="com.sun.star.container">XEnumeration</type> access based
    on an object implementing the <type scope="com.sun.star.container">XNameAccess</type> interface
*/
class COMPHELPER_DLLPUBLIC OEnumerationByName : private OEnumerationLock
                         , public ::cppu::WeakImplHelper2< starcontainer::XEnumeration ,
                                                           starlang::XEventListener    >
{
    staruno::Sequence< ::rtl::OUString >                m_aNames;
    sal_Int32                                           m_nPos;
    staruno::Reference< starcontainer::XNameAccess >    m_xAccess;
    sal_Bool                                            m_bListening;

public:
    OEnumerationByName(const staruno::Reference< starcontainer::XNameAccess >& _rxAccess);
    OEnumerationByName(const staruno::Reference< starcontainer::XNameAccess >& _rxAccess,
                       const staruno::Sequence< ::rtl::OUString >&             _aNames  );
    virtual ~OEnumerationByName();

    virtual sal_Bool SAL_CALL hasMoreElements(  ) throw(staruno::RuntimeException);
    virtual staruno::Any SAL_CALL nextElement(  )
        throw(starcontainer::NoSuchElementException, starlang::WrappedTargetException, staruno::RuntimeException);

    virtual void SAL_CALL disposing(const starlang::EventObject& aEvent) throw(staruno::RuntimeException);

private:
    COMPHELPER_DLLPRIVATE void impl_startDisposeListening();
    COMPHELPER_DLLPRIVATE void impl_stopDisposeListening();
};

//==================================================================
//= OEnumerationByIndex
//==================================================================
/** provides an <type scope="com.sun.star.container">XEnumeration</type> access based
    on an object implementing the <type scope="com.sun.star.container">XNameAccess</type> interface
*/
class COMPHELPER_DLLPUBLIC OEnumerationByIndex : private OEnumerationLock
                          , public ::cppu::WeakImplHelper2< starcontainer::XEnumeration ,
                                                            starlang::XEventListener    >
{
    sal_Int32                                         m_nPos;
    staruno::Reference< starcontainer::XIndexAccess > m_xAccess;
    sal_Bool                                          m_bListening;

public:
    OEnumerationByIndex(const staruno::Reference< starcontainer::XIndexAccess >& _rxAccess);
    virtual ~OEnumerationByIndex();

    virtual sal_Bool SAL_CALL hasMoreElements(  ) throw(staruno::RuntimeException);
    virtual staruno::Any SAL_CALL nextElement(  )
        throw(starcontainer::NoSuchElementException, starlang::WrappedTargetException, staruno::RuntimeException);

    virtual void SAL_CALL disposing(const starlang::EventObject& aEvent) throw(staruno::RuntimeException);

private:
    COMPHELPER_DLLPRIVATE void impl_startDisposeListening();
    COMPHELPER_DLLPRIVATE void impl_stopDisposeListening();
};

//==================================================================
//= OAnyEnumeration
//==================================================================
/** provides an <type scope="com.sun.star.container">XEnumeration</type>
    for an outside set vector of Any's.

*/
class COMPHELPER_DLLPUBLIC OAnyEnumeration : private OEnumerationLock
                                           , public  ::cppu::WeakImplHelper1< starcontainer::XEnumeration >
{
    sal_Int32                         m_nPos;
    staruno::Sequence< staruno::Any > m_lItems;

public:
    OAnyEnumeration(const staruno::Sequence< staruno::Any >& lItems);
    virtual ~OAnyEnumeration();

    virtual sal_Bool SAL_CALL hasMoreElements(  ) throw(staruno::RuntimeException);
    virtual staruno::Any SAL_CALL nextElement(  )
        throw(starcontainer::NoSuchElementException, starlang::WrappedTargetException, staruno::RuntimeException);

};

//.........................................................................
}
//... namespace comphelper .......................................................

#endif // _COMPHELPER_ENUMHELPER_HXX_



