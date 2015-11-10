/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#ifndef INCLUDED_UNOXML_SOURCE_DOM_ELEMENTLIST_HXX
#define INCLUDED_UNOXML_SOURCE_DOM_ELEMENTLIST_HXX

#include <vector>

#include <memory>

#include <libxml/tree.h>

#include <sal/types.h>
#include <rtl/ref.hxx>

#include <com/sun/star/uno/Reference.h>
#include <com/sun/star/uno/Exception.hpp>
#include <com/sun/star/xml/dom/XNode.hpp>
#include <com/sun/star/xml/dom/XNodeList.hpp>
#include <com/sun/star/xml/dom/events/XEvent.hpp>
#include <com/sun/star/xml/dom/events/XEventListener.hpp>

#include <cppuhelper/implbase.hxx>

namespace DOM
{
    class CElement;

    typedef std::vector< xmlNodePtr > nodevector_t;

    class CElementListImpl
        : public cppu::WeakImplHelper< css::xml::dom::XNodeList,
                css::xml::dom::events::XEventListener >
    {
    private:
        /** @short  proxy weak binding to forward Events to ourself without
                    an ownership cycle
          */
        css::uno::Reference< css::xml::dom::events::XEventListener > m_xEventListener;

        ::rtl::Reference<CElement> const m_pElement;
        ::osl::Mutex & m_rMutex;
        ::std::unique_ptr<xmlChar[]> const m_pName;
        ::std::unique_ptr<xmlChar[]> const m_pURI;
        bool m_bRebuild;
        nodevector_t m_nodevector;

        void buildlist(xmlNodePtr pNode, bool start=true);

    public:
        CElementListImpl(::rtl::Reference<CElement> const& pElement,
                ::osl::Mutex & rMutex,
                OUString const& rName, OUString const*const pURI = nullptr);

        void registerListener(CElement & rElement);

        virtual ~CElementListImpl();

        /**
        The number of nodes in the list.
        */
        virtual sal_Int32 SAL_CALL getLength() throw (css::uno::RuntimeException, std::exception) override;
        /**
        Returns the indexth item in the collection.
        */
        virtual css::uno::Reference< css::xml::dom::XNode > SAL_CALL item(sal_Int32 index)
            throw (css::uno::RuntimeException, std::exception) override;

        // XEventListener
        virtual void SAL_CALL handleEvent(const css::uno::Reference< css::xml::dom::events::XEvent >& evt)
            throw (css::uno::RuntimeException, std::exception) override;
    };

    class CElementList
        : public cppu::WeakImplHelper< css::xml::dom::XNodeList,
                css::xml::dom::events::XEventListener >
    {
    private:
        rtl::Reference<CElementListImpl> m_xImpl;
    public:
        CElementList(::rtl::Reference<CElement> const& pElement,
                ::osl::Mutex & rMutex,
                OUString const& rName, OUString const*const pURI = nullptr);

        /**
        The number of nodes in the list.
        */
        virtual sal_Int32 SAL_CALL getLength() throw (css::uno::RuntimeException, std::exception) override
        {
            return m_xImpl->getLength();
        }
        /**
        Returns the indexth item in the collection.
        */
        virtual css::uno::Reference< css::xml::dom::XNode > SAL_CALL item(sal_Int32 index)
            throw (css::uno::RuntimeException, std::exception) override
        {
            return m_xImpl->item(index);
        }

        // XEventListener
        virtual void SAL_CALL handleEvent(const css::uno::Reference< css::xml::dom::events::XEvent >& evt)
            throw (css::uno::RuntimeException, std::exception) override
        {
            m_xImpl->handleEvent(evt);
        }
    };
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
