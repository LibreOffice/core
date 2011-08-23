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

#ifndef _ELEMENTLIST_HXX
#define _ELEMENTLIST_HXX

#include <vector>
#include <sal/types.h>
#include <cppuhelper/implbase1.hxx>
#include <cppuhelper/implbase2.hxx>
#include <com/sun/star/uno/Reference.h>
#include <com/sun/star/uno/Exception.hpp>
#include <com/sun/star/xml/dom/XNode.hpp>
#include <com/sun/star/xml/dom/XNodeList.hpp>
#include <com/sun/star/xml/dom/events/XEvent.hpp>
#include <com/sun/star/xml/dom/events/XEventListener.hpp>
#include "element.hxx"
#include "document.hxx"
#include "libxml/tree.h"

using ::rtl::OUString;
using namespace com::sun::star::uno;
using namespace com::sun::star::xml::dom;
using namespace com::sun::star::xml::dom::events;

namespace DOM
{
    typedef std::vector< xmlNodePtr > nodevector;

    class CElementList : public cppu::WeakImplHelper2< XNodeList, com::sun::star::xml::dom::events::XEventListener >
    {
    private:
        const CElement* m_pElement;
        const OUString m_aName;
        const OUString m_aURI;
        xmlChar *xName;
        xmlChar *xURI;
        sal_Bool m_bRebuild;
        nodevector m_nodevector;
        
        
        void buildlist(xmlNodePtr pNode, sal_Bool start=sal_True);
        void registerListener(const CElement* pElement);

    public:
        CElementList(const CElement* aDoc, const OUString& aName);
        CElementList(const CElement* aDoc, const OUString& aName, const OUString& aURI);
        /**
        The number of nodes in the list.
        */
        virtual sal_Int32 SAL_CALL getLength() throw (RuntimeException);
        /**
        Returns the indexth item in the collection.
        */
        virtual Reference< XNode > SAL_CALL item(sal_Int32 index) throw (RuntimeException);

        // XEventListener
        virtual void SAL_CALL handleEvent(const Reference< XEvent >& evt) throw (RuntimeException);
    };
}

#endif
