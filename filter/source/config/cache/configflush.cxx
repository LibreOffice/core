/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: configflush.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: obo $ $Date: 2006-09-17 07:36:33 $
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
#include "precompiled_filter.hxx"

#include "configflush.hxx"
#include "constant.hxx"

//_______________________________________________
// includes

#ifndef _OSL_DIAGNOSE_H_
#include <osl/diagnose.h>
#endif

//_______________________________________________
// namespace

namespace css = ::com::sun::star;

namespace filter{
    namespace config{

//_______________________________________________
// definitions

//-----------------------------------------------
ConfigFlush::ConfigFlush(const css::uno::Reference< css::lang::XMultiServiceFactory >& xSMGR)
    : BaseLock   (       )
    , m_xSMGR    (xSMGR  )
    , m_lListener(m_aLock)
{
}

//-----------------------------------------------
ConfigFlush::~ConfigFlush()
{
}

//-----------------------------------------------
::rtl::OUString SAL_CALL ConfigFlush::getImplementationName()
    throw (css::uno::RuntimeException)
{
    return impl_getImplementationName();
    // <- SAFE
}

//-----------------------------------------------
sal_Bool SAL_CALL ConfigFlush::supportsService(const ::rtl::OUString& sServiceName)
    throw (css::uno::RuntimeException)
{
          css::uno::Sequence< ::rtl::OUString > lServiceNames = impl_getSupportedServiceNames();
          sal_Int32                             c             = lServiceNames.getLength();
    const ::rtl::OUString*                      pNames        = lServiceNames.getConstArray();
    for (sal_Int32 i=0; i<c; ++i)
    {
        if (pNames[i].equals(sServiceName))
            return sal_True;
    }
    return sal_False;
}

//-----------------------------------------------
css::uno::Sequence< ::rtl::OUString > SAL_CALL ConfigFlush::getSupportedServiceNames()
    throw (css::uno::RuntimeException)
{
    return impl_getSupportedServiceNames();
}

//-----------------------------------------------
void SAL_CALL ConfigFlush::refresh()
    throw(css::uno::RuntimeException)
{
    // notify listener outside the lock!
    // The used listener helper lives if we live
    // and is threadsafe by itself.
    // Further its not a good idea to hold the own lock
    // if an outside object is called :-)
    css::lang::EventObject             aSource    (static_cast< css::util::XRefreshable* >(this));
    ::cppu::OInterfaceContainerHelper* pContainer = m_lListener.getContainer(::getCppuType(static_cast< css::uno::Reference< css::util::XRefreshListener >* >(NULL)));
    if (pContainer)
    {
        ::cppu::OInterfaceIteratorHelper pIterator(*pContainer);
        while (pIterator.hasMoreElements())
        {
            try
            {
                // ... this pointer can be interesting to find out, where will be called as listener
                // Dont optimize it to a direct iterator cast :-)
                css::util::XRefreshListener* pListener = (css::util::XRefreshListener*)pIterator.next();
                pListener->refreshed(aSource);
            }
            catch(const css::uno::Exception&)
            {
                // ignore any "damaged" flush listener!
                // May its remote reference is broken ...
                pIterator.remove();
            }
        }
    }
}

//-----------------------------------------------
void SAL_CALL ConfigFlush::addRefreshListener(const css::uno::Reference< css::util::XRefreshListener >& xListener)
    throw(css::uno::RuntimeException)
{
    // no locks neccessary
    // used helper lives if we live and is threadsafe by itself ...
    m_lListener.addInterface(::getCppuType(static_cast< css::uno::Reference< css::util::XRefreshListener >* >(NULL)),
                             xListener);
}

//-----------------------------------------------
void SAL_CALL ConfigFlush::removeRefreshListener(const css::uno::Reference< css::util::XRefreshListener >& xListener)
    throw(css::uno::RuntimeException)
{
    // no locks neccessary
    // used helper lives if we live and is threadsafe by itself ...
    m_lListener.removeInterface(::getCppuType(static_cast< css::uno::Reference< css::util::XRefreshListener >* >(NULL)),
                                xListener);
}

//-----------------------------------------------
::rtl::OUString ConfigFlush::impl_getImplementationName()
{
    static ::rtl::OUString IMPLNAME = ::rtl::OUString::createFromAscii("com.sun.star.comp.filter.config.ConfigFlush");
    return IMPLNAME;
}

//-----------------------------------------------
css::uno::Sequence< ::rtl::OUString > ConfigFlush::impl_getSupportedServiceNames()
{
    css::uno::Sequence< ::rtl::OUString > lServiceNames(1);
    lServiceNames[0] = SERVICE_FILTERCONFIGREFRESH;
    return lServiceNames;
}

//-----------------------------------------------
css::uno::Reference< css::uno::XInterface > ConfigFlush::impl_createInstance(const css::uno::Reference< css::lang::XMultiServiceFactory >& xSMGR)
{
    ConfigFlush* pNew = new ConfigFlush(xSMGR);
    return css::uno::Reference< css::uno::XInterface >(static_cast< css::util::XRefreshable* >(pNew), css::uno::UNO_QUERY);
}

    } // namespace config
} // namespace filter
