/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: elementlist.hxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: ihi $ $Date: 2007-11-19 16:43:48 $
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

using namespace rtl;
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
