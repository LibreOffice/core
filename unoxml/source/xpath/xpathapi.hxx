/*************************************************************************
 *
 *  $RCSfile: xpathapi.hxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: lo $ $Date: 2004-01-28 16:32:02 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2004 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef _XPATHAPI_HXX
#define _XPATHAPI_HXX

#include <map>

#include <sal/types.h>
#include <cppuhelper/implbase2.hxx>
#include <com/sun/star/uno/Reference.h>
#include <com/sun/star/uno/Sequence.h>

#include <com/sun/star/uno/XInterface.hpp>
#include <com/sun/star/uno/Exception.hpp>
#include <com/sun/star/xml/xpath/XXPathAPI.hpp>
#include <com/sun/star/xml/dom/XNode.hpp>
#include <com/sun/star/xml/dom/XNodeList.hpp>
#include <com/sun/star/xml/xpath/XXPathAPI.hpp>
#include <com/sun/star/xml/xpath/XXPathObject.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/lang/XSingleServiceFactory.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/lang/XUnoTunnel.hpp>

#include "libxml/tree.h"

using namespace rtl;
using namespace com::sun::star::uno;
using namespace com::sun::star::lang;
using namespace com::sun::star::xml::dom;
using namespace com::sun::star::xml::xpath;

namespace XPath
{
    typedef std::map<OUString, OUString> nsmap_t;
    
    class  CXPathAPI
        : public ::cppu::WeakImplHelper2< XXPathAPI, XServiceInfo >
    {

    private:
        nsmap_t m_nsmap;
        
    public:

        // call for factory
        static Reference< XInterface > getInstance(const Reference < XMultiServiceFactory >& xFactory);

        // static helpers for service info and component management
        static const char* aImplementationName;
        static const char* aSupportedServiceNames[];
        static OUString _getImplementationName();
        static Sequence< OUString > _getSupportedServiceNames();
        static Reference< XInterface > _getInstance(const Reference< XMultiServiceFactory >& rSMgr);

        // XServiceInfo
        virtual OUString SAL_CALL getImplementationName()
            throw (RuntimeException);
        virtual sal_Bool SAL_CALL supportsService(const OUString& ServiceName)
            throw (RuntimeException);
        virtual Sequence< OUString > SAL_CALL getSupportedServiceNames ()
            throw (RuntimeException);


        // --- XXPathAPI ---

        virtual void SAL_CALL CXPathAPI::registerNS(const OUString& aPrefix, const OUString& aURI)
            throw (RuntimeException);

        virtual void SAL_CALL CXPathAPI::unregisterNS(const OUString& aPrefix, const OUString& aURI)
            throw (RuntimeException);

        /**
        Use an XPath string to select a nodelist.
        */
        virtual Reference< XNodeList > SAL_CALL selectNodeList(const Reference< XNode >& contextNode, const OUString& str)
            throw (RuntimeException);

        /**
        Use an XPath string to select a nodelist.
        */
        virtual Reference< XNodeList > SAL_CALL selectNodeListNS(const Reference< XNode >& contextNode, const OUString& str, const Reference< XNode >&  namespaceNode)
            throw (RuntimeException);

        /**
        Use an XPath string to select a single node.
        */
        virtual Reference< XNode > SAL_CALL selectSingleNode(const Reference< XNode >& contextNode, const OUString& str)
            throw (RuntimeException);

        /**
        Use an XPath string to select a single node.
        */
        virtual Reference< XNode > SAL_CALL selectSingleNodeNS(const Reference< XNode >& contextNode, const OUString& str, const Reference< XNode >&  namespaceNode)
            throw (RuntimeException);

        virtual Reference< XXPathObject > SAL_CALL eval(const Reference< XNode >& contextNode, const OUString& str)
            throw (RuntimeException);

        virtual Reference< XXPathObject > SAL_CALL evalNS(const Reference< XNode >& contextNode, const OUString& str, const Reference< XNode >&  namespaceNode)
            throw (RuntimeException);

     

    };
}

#endif
