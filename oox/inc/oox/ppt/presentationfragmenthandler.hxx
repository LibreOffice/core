/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: presentationfragmenthandler.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: kz $ $Date: 2008-03-05 17:56:07 $
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

#ifndef OOX_PPT_PRESENTATION_FRAGMENTHANDLER
#define OOX_PPT_PRESENTATION_FRAGMENTHANDLER

#include <com/sun/star/drawing/XDrawPage.hpp>
#include <com/sun/star/awt/Size.hpp>
#include "oox/drawingml/textliststyle.hxx"
#include "oox/ppt/slidepersist.hxx"
#include "oox/core/fragmenthandler.hxx"
#include "oox/core/relations.hxx"
#include "customshowlistcontext.hxx"

#include <stack>
#include <vector>

namespace oox { namespace ppt {

class PresentationFragmentHandler : public ::oox::core::FragmentHandler
{
public:
    PresentationFragmentHandler( ::oox::core::XmlFilterBase& rFilter, const ::rtl::OUString& rFragmentPath ) throw();
    virtual ~PresentationFragmentHandler() throw();

    virtual void SAL_CALL startDocument(  ) throw (::com::sun::star::xml::sax::SAXException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL endDocument(  ) throw (::com::sun::star::xml::sax::SAXException, ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XFastContextHandler > SAL_CALL createFastChildContext( ::sal_Int32 Element, const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XFastAttributeList >& Attribs ) throw (::com::sun::star::xml::sax::SAXException, ::com::sun::star::uno::RuntimeException);

protected:
    bool importSlide( const ::oox::core::FragmentHandlerRef& rxSlideFragmentHandler,
                        const oox::ppt::SlidePersistPtr pPersist );

private:
    std::vector< rtl::OUString > maSlideMasterVector;
    std::vector< rtl::OUString > maSlidesVector;
    std::vector< rtl::OUString > maNotesMasterVector;
    ::oox::drawingml::TextListStylePtr mpTextListStyle;

    ::com::sun::star::awt::Size maSlideSize;
    ::com::sun::star::awt::Size maNotesSize;

    std::vector< CustomShow >   maCustomShowList;
};

} }

#endif // OOX_PPT_PRESENTATION_FRAGMENTHANDLER
