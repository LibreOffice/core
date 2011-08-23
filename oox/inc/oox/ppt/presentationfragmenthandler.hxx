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

    std::vector< CustomShow >	maCustomShowList;
};

} }

#endif // OOX_PPT_PRESENTATION_FRAGMENTHANDLER
