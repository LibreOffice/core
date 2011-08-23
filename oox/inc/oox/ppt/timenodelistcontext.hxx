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
