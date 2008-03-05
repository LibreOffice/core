/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: slidemastertextstylescontext.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: kz $ $Date: 2008-03-05 17:56:40 $
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

#ifndef OOX_DRAWINGML_SLIDEMASTERTEXTSTYLESCONTEXT_HXX
#define OOX_DRAWINGML_SLIDEMASTERTEXTSTYLESCONTEXT_HXX

#include "oox/drawingml/theme.hxx"
#include "oox/core/contexthandler.hxx"
#include "oox/core/fragmenthandler.hxx"
#include "oox/ppt/slidepersist.hxx"

namespace oox { namespace ppt {

class SlideMasterTextStylesContext : public oox::core::ContextHandler
{
public:
    SlideMasterTextStylesContext( ::oox::core::ContextHandler& rParent, SlidePersistPtr pSlidePersistPtr );
    ~SlideMasterTextStylesContext();
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XFastContextHandler > SAL_CALL createFastChildContext( ::sal_Int32 Element, const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XFastAttributeList >& Attribs ) throw (::com::sun::star::xml::sax::SAXException, ::com::sun::star::uno::RuntimeException);

protected:
    SlidePersistPtr     mpSlidePersistPtr;
};

} }

#endif  //  OOX_DRAWINGML_SLIDEMASTERTEXTSTYLESCONTEXT_HXX
