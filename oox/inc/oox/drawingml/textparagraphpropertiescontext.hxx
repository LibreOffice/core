/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: textparagraphpropertiescontext.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: kz $ $Date: 2008-03-05 17:46:03 $
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

#ifndef OOX_DRAWINGML_TEXTPARAGRAPHPROPERTIESCONTEXT_HXX
#define OOX_DRAWINGML_TEXTPARAGRAPHPROPERTIESCONTEXT_HXX

#include <list>

#include <com/sun/star/style/TabStop.hpp>
#include "oox/drawingml/textparagraphproperties.hxx"
#include "oox/drawingml/textspacing.hxx"
#include "oox/core/contexthandler.hxx"

namespace oox { namespace drawingml {

class TextParagraphPropertiesContext : public ::oox::core::ContextHandler
{
public:
    TextParagraphPropertiesContext( ::oox::core::ContextHandler& rParent,
        const com::sun::star::uno::Reference< com::sun::star::xml::sax::XFastAttributeList >& rXAttributes,
            TextParagraphProperties& rTextParagraphProperties );
    ~TextParagraphPropertiesContext();

    virtual void SAL_CALL endFastElement( ::sal_Int32 Element ) throw (::com::sun::star::xml::sax::SAXException, ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XFastContextHandler > SAL_CALL createFastChildContext( ::sal_Int32 Element, const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XFastAttributeList >& Attribs ) throw (::com::sun::star::xml::sax::SAXException, ::com::sun::star::uno::RuntimeException);

protected:
    TextParagraphProperties& mrTextParagraphProperties;
    TextSpacing     maLineSpacing;
    TextSpacing&    mrSpaceBefore;
    TextSpacing&    mrSpaceAfter;
    BulletList&     mrBulletList;
    ::std::list< ::com::sun::star::style::TabStop >  maTabList;
};

} }

#endif  //  OOX_DRAWINGML_TEXTPARAGRAPHPROPERTIESCONTEXT_HXX
