/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: OOXMLFastTokenHandler.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: obo $ $Date: 2008-01-10 11:58:34 $
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

#ifndef INCLUDED_OOXML_FAST_TOKEN_HANDLER_HXX
#define INCLUDED_OOXML_FAST_TOKEN_HANDLER_HXX

#include "sal/config.h"
#include "com/sun/star/uno/XComponentContext.hpp"
#include "cppuhelper/implbase1.hxx"
#include "com/sun/star/xml/sax/XFastTokenHandler.hpp"

namespace css = ::com::sun::star;

namespace writerfilter {
namespace ooxml
{

class OOXMLFastTokenHandler:
    public ::cppu::WeakImplHelper1<
        css::xml::sax::XFastTokenHandler>
{
public:
    explicit OOXMLFastTokenHandler(css::uno::Reference< css::uno::XComponentContext > const & context);

    // ::com::sun::star::xml::sax::XFastTokenHandler:
    virtual ::sal_Int32 SAL_CALL getToken(const ::rtl::OUString & Identifier) throw (css::uno::RuntimeException);
    virtual ::rtl::OUString SAL_CALL getIdentifier(::sal_Int32 Token) throw (css::uno::RuntimeException);
    virtual ::sal_Int32 SAL_CALL getTokenFromUTF8(const css::uno::Sequence< ::sal_Int8 > & Identifier) throw (css::uno::RuntimeException);

private:
    OOXMLFastTokenHandler(OOXMLFastTokenHandler &); // not defined
    void operator =(OOXMLFastTokenHandler &); // not defined

    virtual ~OOXMLFastTokenHandler() {}

    css::uno::Reference< css::uno::XComponentContext > m_xContext;
};

}}
#endif // INCLUDED_OOXML_FAST_TOKEN_HANDLER_HXX
