/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_COMPHELPER_UNIQUE_DISPOSING_PTR_HXX
#define INCLUDED_COMPHELPER_UNIQUE_DISPOSING_PTR_HXX

#include <memory>
#include <cppuhelper/implbase.hxx>

#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/frame/XDesktop.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>

#include <o3tl/deleter.hxx>
#include <vcl/svapp.hxx>

namespace comphelper
{
//Similar to std::unique_ptr, except additionally releases the ptr on XComponent::disposing and/or XTerminateListener::notifyTermination if supported
template<class T> class unique_disposing_ptr
{
private:
    std::unique_ptr<T, o3tl::default_delete<T>> m_xItem;
    css::uno::Reference< css::frame::XTerminateListener> m_xTerminateListener;

    unique_disposing_ptr(const unique_disposing_ptr&) = delete;
    unique_disposing_ptr& operator=(const unique_disposing_ptr&) = delete;
public:
    unique_disposing_ptr( const css::uno::Reference< css::lang::XComponent > &rComponent, T * p = nullptr, bool bComponent = false)
        : m_xItem(p)
    {
        m_xTerminateListener = new TerminateListener(rComponent, *this, bComponent);
    }

    virtual void reset(T * p = nullptr)
    {
        m_xItem.reset(p);
    }

    T & operator*() const
    {
        return *m_xItem;
    }

    T * get() const
    {
        return m_xItem.get();
    }

    T * operator->() const
    {
        return m_xItem.get();
    }

    operator bool () const
    {
        return static_cast< bool >(m_xItem);
    }

    virtual ~unique_disposing_ptr() COVERITY_NOEXCEPT_FALSE
    {
        reset();
    }
private:
    class TerminateListener : public ::cppu::WeakImplHelper< css::frame::XTerminateListener,
                                            css::lang::XServiceInfo>
    {
    private:
        css::uno::Reference< css::lang::XComponent > m_xComponent;
        unique_disposing_ptr<T>& m_rItem;
        bool const mbComponentDLL;
    public:
        TerminateListener(const css::uno::Reference< css::lang::XComponent > &rComponent,
            unique_disposing_ptr<T>& rItem, bool bComponentDLL) :
                    m_xComponent(rComponent),
                    m_rItem(rItem),
                    mbComponentDLL(bComponentDLL)
        {
            if (m_xComponent.is())
            {
                css::uno::Reference< css::frame::XDesktop> xDesktop(m_xComponent, css::uno::UNO_QUERY);
                if (xDesktop.is())
                    xDesktop->addTerminateListener(this);
                else
                    m_xComponent->addEventListener(this);
            }
        }

        virtual ~TerminateListener() override
        {
            if ( m_xComponent.is() )
            {
                css::uno::Reference< css::frame::XDesktop> xDesktop(m_xComponent, css::uno::UNO_QUERY);
                if (xDesktop.is())
                    xDesktop->removeTerminateListener(this);
                else
                    m_xComponent->removeEventListener(this);
            }
        }

        // XEventListener
        virtual void SAL_CALL disposing( const css::lang::EventObject& rEvt ) override
        {
            bool shutDown = (rEvt.Source == m_xComponent);

            if (shutDown && m_xComponent.is())
            {
                css::uno::Reference< css::frame::XDesktop> xDesktop(m_xComponent, css::uno::UNO_QUERY);
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
        virtual void SAL_CALL queryTermination( const css::lang::EventObject& ) override
        {
        }

        virtual void SAL_CALL notifyTermination( const css::lang::EventObject& rEvt ) override
        {
            disposing(rEvt);
        }

        virtual OUString SAL_CALL getImplementationName() override
        {
            if (mbComponentDLL)
                return OUString("com.sun.star.comp.ComponentDLLListener");
            else
                return OUString("com.sun.star.comp.DisposingTerminateListener");
        }

        virtual sal_Bool SAL_CALL supportsService(const OUString& /*rName*/) override
        {
            return false;
        }

        virtual css::uno::Sequence<OUString> SAL_CALL getSupportedServiceNames() override
        {
            return css::uno::Sequence<OUString>();
        }
   };
};

//Something like an OutputDevice requires the SolarMutex to be taken before use
//for threadsafety. The user can ensure this, except in the case of its dtor
//being called from reset due to a terminate on the XComponent being called
//from an arbitrary thread
template<class T> class unique_disposing_solar_mutex_reset_ptr
    : public unique_disposing_ptr<T>
{
public:
    unique_disposing_solar_mutex_reset_ptr( const css::uno::Reference< css::lang::XComponent > &rComponent, T * p = nullptr, bool bComponent = false)
        : unique_disposing_ptr<T>(rComponent, p, bComponent)
    {
    }

    virtual void reset(T * p = nullptr) override
    {
        SolarMutexGuard aGuard;
        unique_disposing_ptr<T>::reset(p);
    }

    virtual ~unique_disposing_solar_mutex_reset_ptr() override
    {
        if (unique_disposing_ptr<T>::get() && comphelper::SolarMutex::get())
            reset();
    }
};

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
