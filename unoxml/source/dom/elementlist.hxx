/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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

#ifndef DOM_ELEMENTLIST_HXX
#define DOM_ELEMENTLIST_HXX

#include <vector>

#include <boost/scoped_array.hpp>

#include <libxml/tree.h>

#include <sal/types.h>
#include <rtl/ref.hxx>

#include <com/sun/star/uno/Reference.h>
#include <com/sun/star/uno/Exception.hpp>
#include <com/sun/star/xml/dom/XNode.hpp>
#include <com/sun/star/xml/dom/XNodeList.hpp>
#include <com/sun/star/xml/dom/events/XEvent.hpp>
#include <com/sun/star/xml/dom/events/XEventListener.hpp>

#include <cppuhelper/implbase2.hxx>


using ::rtl::OUString;
using namespace com::sun::star::uno;
using namespace com::sun::star::xml::dom;
using namespace com::sun::star::xml::dom::events;

namespace DOM
{
    class CElement;

    typedef std::vector< xmlNodePtr > nodevector_t;

    class CElementList
        : public cppu::WeakImplHelper2< XNodeList,
                com::sun::star::xml::dom::events::XEventListener >
    {
    private:
        ::rtl::Reference<CElement> const m_pElement;
        ::osl::Mutex & m_rMutex;
        ::boost::scoped_array<xmlChar> const m_pName;
        ::boost::scoped_array<xmlChar> const m_pURI;
        bool m_bRebuild;
        nodevector_t m_nodevector;

        void buildlist(xmlNodePtr pNode, sal_Bool start=sal_True);
        void registerListener(CElement & rElement);

    public:
        CElementList(::rtl::Reference<CElement> const& pElement,
                ::osl::Mutex & rMutex,
                OUString const& rName, OUString const*const pURI = 0);

        /**
        The number of nodes in the list.
        */
        virtual sal_Int32 SAL_CALL getLength() throw (RuntimeException);
        /**
        Returns the indexth item in the collection.
        */
        virtual Reference< XNode > SAL_CALL item(sal_Int32 index)
            throw (RuntimeException);

        // XEventListener
        virtual void SAL_CALL handleEvent(const Reference< XEvent >& evt)
            throw (RuntimeException);
    };
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
