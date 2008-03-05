/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: timenodelistcontext.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: kz $ $Date: 2008-03-05 17:57:57 $
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
