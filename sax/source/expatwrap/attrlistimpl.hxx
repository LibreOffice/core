/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: attrlistimpl.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: obo $ $Date: 2008-02-26 14:40:37 $
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

#ifndef _SAX_ATTRLISTIMPL_HXX
#define _SAX_ATTRLISTIMPL_HXX

#include "sal/config.h"
//#include "sax/saxdllapi.h"

#include <cppuhelper/implbase2.hxx>

#include <com/sun/star/uno/RuntimeException.hpp>
#include <com/sun/star/util/XCloneable.hpp>
#include <com/sun/star/xml/sax/XAttributeList.hpp>

namespace sax_expatwrap
{

struct AttributeList_impl;

//FIXME
class /*SAX_DLLPUBLIC*/ AttributeList :
    public ::cppu::WeakImplHelper2<
                ::com::sun::star::xml::sax::XAttributeList,
                ::com::sun::star::util::XCloneable >
{
public:
    AttributeList();
    AttributeList( const AttributeList & );
    virtual ~AttributeList();

    void addAttribute( const ::rtl::OUString &sName ,
        const ::rtl::OUString &sType , const ::rtl::OUString &sValue );
    void clear();
    void removeAttribute( const ::rtl::OUString &sName );
    void setAttributeList( const ::com::sun::star::uno::Reference<
            ::com::sun::star::xml::sax::XAttributeList > & );

public:
    // XAttributeList
    virtual sal_Int16 SAL_CALL getLength(void)
        throw(::com::sun::star::uno::RuntimeException);
    virtual ::rtl::OUString SAL_CALL getNameByIndex(sal_Int16 i)
        throw(::com::sun::star::uno::RuntimeException);
    virtual ::rtl::OUString SAL_CALL getTypeByIndex(sal_Int16 i)
        throw(::com::sun::star::uno::RuntimeException);
    virtual ::rtl::OUString SAL_CALL getTypeByName(const ::rtl::OUString& aName)
        throw(::com::sun::star::uno::RuntimeException);
    virtual ::rtl::OUString SAL_CALL getValueByIndex(sal_Int16 i)
        throw(::com::sun::star::uno::RuntimeException);
    virtual ::rtl::OUString SAL_CALL getValueByName(const ::rtl::OUString& aName)
        throw( ::com::sun::star::uno::RuntimeException);

    // XCloneable
    virtual ::com::sun::star::uno::Reference< XCloneable > SAL_CALL
        createClone()   throw(::com::sun::star::uno::RuntimeException);

private:
    struct AttributeList_impl *m_pImpl;
};

}

#endif

