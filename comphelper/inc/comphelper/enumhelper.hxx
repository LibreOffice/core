/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: enumhelper.hxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 02:30:12 $
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

#ifndef _COMPHELPER_ENUMHELPER_HXX_
#define _COMPHELPER_ENUMHELPER_HXX_

#ifndef _COM_SUN_STAR_CONTAINER_XNAMEACCESS_HPP_
#include <com/sun/star/container/XNameAccess.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XENUMERATION_HPP_
#include <com/sun/star/container/XEnumeration.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XINDEXACCESS_HPP_
#include <com/sun/star/container/XIndexAccess.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XEVENTLISTENER_HPP_
#include <com/sun/star/lang/XEventListener.hpp>
#endif
#ifndef _CPPUHELPER_IMPLBASE2_HXX_
#include <cppuhelper/implbase2.hxx>
#endif
#ifndef _OSL_MUTEX_HXX_
#include <osl/mutex.hxx>
#endif

#ifndef INCLUDED_COMPHELPERDLLAPI_H
#include "comphelper/comphelperdllapi.h"
#endif

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

//.........................................................................
}
//... namespace comphelper .......................................................

#endif // _COMPHELPER_ENUMHELPER_HXX_



