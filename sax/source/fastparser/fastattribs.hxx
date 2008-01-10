/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: fastattribs.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: obo $ $Date: 2008-01-10 12:52:20 $
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

#ifndef _SAX_FASTATTRIBS_HXX_
#define _SAX_FASTATTRIBS_HXX_

#include <com/sun/star/xml/sax/XFastAttributeList.hpp>
#include <com/sun/star/xml/sax/XFastTokenHandler.hpp>
#include <com/sun/star/xml/Attribute.hpp>

#include <cppuhelper/implbase1.hxx>

#include <map>
#include <vector>

namespace sax_fastparser
{

struct UnknownAttribute;

typedef std::map< sal_Int32, ::rtl::OString > FastAttributeMap;
typedef std::vector< UnknownAttribute > UnknownAttributeList;

class FastAttributeList : public ::cppu::WeakImplHelper1< ::com::sun::star::xml::sax::XFastAttributeList >
{
public:
    FastAttributeList( const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XFastTokenHandler >& xTokenHandler );
    virtual ~FastAttributeList();

    void clear();
    void add( sal_Int32 nToken, const ::rtl::OString& rValue );
    void addUnknown( const ::rtl::OUString& rNamespaceURL, const ::rtl::OString& rName, const ::rtl::OString& rValue );
    void addUnknown( const ::rtl::OString& rName, const ::rtl::OString& rValue );

    // XFastAttributeList
    virtual ::sal_Bool SAL_CALL hasAttribute( ::sal_Int32 Token ) throw (::com::sun::star::uno::RuntimeException);
    virtual ::sal_Int32 SAL_CALL getValueToken( ::sal_Int32 Token ) throw (::com::sun::star::xml::sax::SAXException, ::com::sun::star::uno::RuntimeException);
    virtual ::sal_Int32 SAL_CALL getOptionalValueToken( ::sal_Int32 Token, ::sal_Int32 Default ) throw (::com::sun::star::uno::RuntimeException);
    virtual ::rtl::OUString SAL_CALL getValue( ::sal_Int32 Token ) throw (::com::sun::star::xml::sax::SAXException, ::com::sun::star::uno::RuntimeException);
    virtual ::rtl::OUString SAL_CALL getOptionalValue( ::sal_Int32 Token ) throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::xml::Attribute > SAL_CALL getUnknownAttributes(  ) throw (::com::sun::star::uno::RuntimeException);

private:
    FastAttributeMap maAttributes;
    UnknownAttributeList maUnknownAttributes;
    FastAttributeMap::iterator maLastIter;
    ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XFastTokenHandler > mxTokenHandler;

};

}

#endif
