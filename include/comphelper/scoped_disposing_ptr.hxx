/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef _SCOPED_DISPOSING_PTR
#define _SCOPED_DISPOSING_PTR

#include <cppuhelper/implbase1.hxx>
#include <boost/utility.hpp>
#include <boost/scoped_ptr.hpp>

#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/frame/XDesktop.hpp>

// for locking SolarMutex: svapp + mutex
#include <vcl/svapp.hxx>
#include <osl/mutex.hxx>

namespace comphelper
{
//Similar to boost::scoped_ptr, except additionally releases the ptr on XComponent::disposing and/or XTerminateListener::notifyTermination if supported
template<class T> class scoped_disposing_ptr : private boost::noncopyable
{
private:
    boost::scoped_ptr<T> m_aItem;
    ::com::sun::star::uno::Reference< ::com::sun::star::frame::XTerminateListener> m_xTerminateListener;
public:
    scoped_disposing_ptr( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XComponent > &rComponent, T * p = 0 )
        : m_aItem(p)
    {
        m_xTerminateListener = new TerminateListener(rComponent, *this);
    }

    virtual void reset(T * p = 0)
    {
        m_aItem.reset(p);
    }

    T & operator*() const
    {
        return *m_aItem;
    }

    T * get() const
    {
        return m_aItem.get();
    }

    T * operator->() const
    {
        return m_aItem.get();
    }

    operator bool () const
    {
        return static_cast< bool >(m_aItem);
    }

    virtual ~scoped_disposing_ptr()
    {
        reset();
    }
private:
    class TerminateListener : public ::cppu::WeakImplHelper1< ::com::sun::star::frame::XTerminateListener >
    {
    private:
        ::com::sun::star::uno::Reference< ::com::sun::star::lang::XComponent > m_xComponent;
        scoped_disposing_ptr<T>& m_rItem;
    public:
        TerminateListener(const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XComponent > &rComponent,
            scoped_disposing_ptr<T>& rItem) : m_xComponent(rComponent), m_rItem(rItem)
        {
            if (m_xComponent.is())
            {
                ::com::sun::star::uno::Reference< ::com::sun::star::frame::XDesktop> xDesktop(m_xComponent, ::com::sun::star::uno::UNO_QUERY);
                if (xDesktop.is())
                    xDesktop->addTerminateListener(this);
                else
                    m_xComponent->addEventListener(this);
            }
        }

        ~TerminateListener()
        {
            if ( m_xComponent.is() )
            {
                ::com::sun::star::uno::Reference< ::com::sun::star::frame::XDesktop> xDesktop(m_xComponent, ::com::sun::star::uno::UNO_QUERY);
                if (xDesktop.is())
                    xDesktop->removeTerminateListener(this);
                else
                    m_xComponent->removeEventListener(this);
            }
        }

    private:
        // XEventListener
        virtual void SAL_CALL disposing( const ::com::sun::star::lang::EventObject& rEvt )
            throw (::com::sun::star::uno::RuntimeException, std::exception)
        {
            bool shutDown = (rEvt.Source == m_xComponent);

            if (shutDown && m_xComponent.is())
            {
                ::com::sun::star::uno::Reference< ::com::sun::star::frame::XDesktop> xDesktop(m_xComponent, ::com::sun::star::uno::UNO_QUERY);
                if (xDesktop.is())
                    xDesktop->removeTerminateListener(this);
                else
                    m_xComponent->removeEventListener(this);
                m_xComponent.clear();
            }

            if (shutDown)
               m_rItem.reset();
        }

        // XTerminateListener
        virtual void SAL_CALL queryTermination( const ::com::sun::star::lang::EventObject& )
            throw(::com::sun::star::frame::TerminationVetoException,
                  ::com::sun::star::uno::RuntimeException, std::exception)
        {
        }

        virtual void SAL_CALL notifyTermination( const ::com::sun::star::lang::EventObject& rEvt )
            throw (::com::sun::star::uno::RuntimeException, std::exception)
        {
            disposing(rEvt);
        }
   };
};

//Something like an OutputDevice requires the SolarMutex to be taken before use
//for threadsafety. The user can ensure this, except in the case of its dtor
//being called from reset due to a terminate on the XComponent being called
//from an aribitrary thread
template<class T> class scoped_disposing_solar_mutex_reset_ptr
    : public scoped_disposing_ptr<T>
{
public:
    scoped_disposing_solar_mutex_reset_ptr( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XComponent > &rComponent, T * p = 0 )
        : scoped_disposing_ptr<T>(rComponent, p)
    {
    }

    virtual void reset(T * p = 0)
    {
        SolarMutexGuard aGuard;
        scoped_disposing_ptr<T>::reset(p);
    }
};

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
