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



#ifndef OOX_PPT_TIMENODELISTCONTEXT
#define OOX_PPT_TIMENODELISTCONTEXT

#include "oox/core/contexthandler.hxx"
#include "oox/ppt/timenode.hxx"

#include <com/sun/star/animations/XTimeContainer.hpp>

namespace oox { namespace ppt {


    class TimeNodeContext : public ::oox::core::ContextHandler
    {
    public:
        virtual ~TimeNodeContext() throw();

        static TimeNodeContext * SAL_CALL makeContext( ::oox::core::ContextHandler& rParent, sal_Int32  aElement, const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XFastAttributeList >& xAttribs, const TimeNodePtr & pNode );

    protected:
        TimeNodeContext( ::oox::core::ContextHandler& rParent, sal_Int32  aElement, const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XFastAttributeList >& xAttribs, const TimeNodePtr & pNode ) throw();

        sal_Int32 mnElement;
        TimeNodePtr mpNode;
    };



/** FastParser context for XML_tnLst, XML_subTnLst and XML_childTnLst */
class TimeNodeListContext : public ::oox::core::ContextHandler
{
public:
    TimeNodeListContext( ::oox::core::ContextHandler& rParent, TimeNodePtrList & aList ) throw();

    virtual ~TimeNodeListContext() throw();

    virtual ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XFastContextHandler > SAL_CALL
    createFastChildContext( ::sal_Int32 Element,
                                                    const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XFastAttributeList >& Attribs )
        throw (::com::sun::star::xml::sax::SAXException, ::com::sun::star::uno::RuntimeException);

private:
    TimeNodePtrList & maList;
};

} }

#endif // OOX_PPT_SLIDEFRAGMENTHANDLER
