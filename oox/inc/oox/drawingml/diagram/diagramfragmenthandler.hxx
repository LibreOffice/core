/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: diagramfragmenthandler.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: kz $ $Date: 2008-03-05 17:52:35 $
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


#ifndef OOX_DRAWINGML_DIAGRAMFRAGMENTHANDLER
#define OOX_DRAWINGML_DIAGRAMFRAGMENTHANDLER

#include "oox/core/fragmenthandler.hxx"
#include "oox/drawingml/diagram/diagram.hxx"

namespace oox { namespace drawingml {


class DiagramDataFragmentHandler : public ::oox::core::FragmentHandler
{
public:
    DiagramDataFragmentHandler( oox::core::XmlFilterBase& rFilter, const ::rtl::OUString& rFragmentPath, const DiagramDataPtr pDataPtr ) throw();
    virtual ~DiagramDataFragmentHandler() throw();

    virtual void SAL_CALL endDocument() throw (::com::sun::star::xml::sax::SAXException, ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XFastContextHandler > SAL_CALL createFastChildContext( ::sal_Int32 Element, const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XFastAttributeList >& Attribs ) throw (::com::sun::star::xml::sax::SAXException, ::com::sun::star::uno::RuntimeException);

private:

    DiagramDataPtr  mpDataPtr;
};



class DiagramLayoutFragmentHandler : public ::oox::core::FragmentHandler
{
public:
    DiagramLayoutFragmentHandler( oox::core::XmlFilterBase& rFilter, const ::rtl::OUString& rFragmentPath, const DiagramLayoutPtr pDataPtr ) throw();
    virtual ~DiagramLayoutFragmentHandler() throw();

    virtual void SAL_CALL endDocument() throw (::com::sun::star::xml::sax::SAXException, ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XFastContextHandler > SAL_CALL createFastChildContext( ::sal_Int32 Element, const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XFastAttributeList >& Attribs ) throw (::com::sun::star::xml::sax::SAXException, ::com::sun::star::uno::RuntimeException);

private:

    DiagramLayoutPtr    mpDataPtr;
};

class DiagramQStylesFragmentHandler : public ::oox::core::FragmentHandler
{
public:
    DiagramQStylesFragmentHandler( oox::core::XmlFilterBase& rFilter, const ::rtl::OUString& rFragmentPath, const DiagramQStylesPtr pDataPtr ) throw();
    virtual ~DiagramQStylesFragmentHandler() throw();

    virtual void SAL_CALL endDocument() throw (::com::sun::star::xml::sax::SAXException, ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XFastContextHandler > SAL_CALL createFastChildContext( ::sal_Int32 Element, const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XFastAttributeList >& Attribs ) throw (::com::sun::star::xml::sax::SAXException, ::com::sun::star::uno::RuntimeException);

private:

    DiagramQStylesPtr   mpDataPtr;
};


class DiagramColorsFragmentHandler : public ::oox::core::FragmentHandler
{
public:
    DiagramColorsFragmentHandler( ::oox::core::XmlFilterBase& rFilter, const ::rtl::OUString& rFragmentPath, const DiagramColorsPtr pDataPtr ) throw();
    virtual ~DiagramColorsFragmentHandler() throw();

    virtual void SAL_CALL endDocument() throw (::com::sun::star::xml::sax::SAXException, ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XFastContextHandler > SAL_CALL createFastChildContext( ::sal_Int32 Element, const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XFastAttributeList >& Attribs ) throw (::com::sun::star::xml::sax::SAXException, ::com::sun::star::uno::RuntimeException);

private:

    DiagramColorsPtr    mpDataPtr;
};

} }


#endif
