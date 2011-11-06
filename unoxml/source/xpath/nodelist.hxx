/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



#ifndef XPATH_NODELIST_HXX
#define XPATH_NODELIST_HXX

#include <sal/types.h>
#include <rtl/ref.hxx>

#include <cppuhelper/implbase1.hxx>

#include <com/sun/star/uno/Reference.h>
#include <com/sun/star/xml/dom/XNode.hpp>
#include <com/sun/star/xml/dom/XNodeList.hpp>
#include <com/sun/star/xml/xpath/XXPathObject.hpp>

#include "libxml/tree.h"
#include "libxml/xpath.h"

#include <boost/shared_ptr.hpp>


using ::rtl::OUString;
using namespace com::sun::star::uno;
using namespace com::sun::star::xml::dom;
using namespace com::sun::star::xml::xpath;

namespace DOM {
    class CDocument;
}

namespace XPath
{

    class CNodeList : public cppu::WeakImplHelper1< XNodeList >
    {
    private:
        /// #i115995# keep document alive
        ::rtl::Reference< DOM::CDocument > const m_pDocument;
        ::osl::Mutex & m_rMutex;
        /// retain the result set in case the CXPathObject is released
        boost::shared_ptr<xmlXPathObject> m_pXPathObj;
        xmlNodeSetPtr m_pNodeSet;

    public:
        CNodeList(
                ::rtl::Reference<DOM::CDocument> const& pDocument,
                ::osl::Mutex & rMutex,
                boost::shared_ptr<xmlXPathObject> const& rxpathObj);
        /**
        The number of nodes in the list.
        */
        virtual sal_Int32 SAL_CALL getLength() throw (RuntimeException);
        /**
        Returns the indexth item in the collection.
        */
        virtual Reference< XNode > SAL_CALL item(sal_Int32 index)
            throw (RuntimeException);
    };
}

#endif
