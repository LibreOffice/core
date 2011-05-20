/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * Version: MPL 1.1 / GPLv3+ / LGPLv3+
 *
 * The contents of this file are subject to the Mozilla Public License Version
 * 1.1 (the "License"); you may not use this file except in compliance with
 * the License. You may obtain a copy of the License at
 * http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * The Initial Developer of the Original Code is
 *        Caolán McNamara <caolanm@redhat.com> (Red Hat, Inc.)
 * Portions created by the Initial Developer are Copyright (C) 2011 the
 * Initial Developer. All Rights Reserved.
 *
 * Contributor(s): Caolán McNamara <caolanm@redhat.com>
 *
 * Alternatively, the contents of this file may be used under the terms of
 * either the GNU General Public License Version 3 or later (the "GPLv3+"), or
 * the GNU Lesser General Public License Version 3 or later (the "LGPLv3+"),
 * in which case the provisions of the GPLv3+ or the LGPLv3+ are applicable
 * instead of those above.
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
        return m_aItem;
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
            throw (::com::sun::star::uno::RuntimeException)
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
                  ::com::sun::star::uno::RuntimeException)
        {
        }

        virtual void SAL_CALL notifyTermination( const ::com::sun::star::lang::EventObject& rEvt )
            throw (::com::sun::star::uno::RuntimeException)
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
