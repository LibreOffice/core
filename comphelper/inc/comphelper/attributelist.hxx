/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: attributelist.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: obo $ $Date: 2006-10-13 11:39:57 $
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

#ifndef _COMPHELPER_ATTRIBUTE_LIST_HXX
#define _COMPHELPER_ATTRIBUTE_LIST_HXX

#ifndef _COM_SUN_STAR_XML_SAX_XATTRIBUTELIST_HPP_
#include <com/sun/star/xml/sax/XAttributeList.hpp>
#endif
#ifndef _CPPUHELPER_IMPLBASE1_HXX_
#include <cppuhelper/implbase1.hxx>
#endif

#ifndef INCLUDED_COMPHELPERDLLAPI_H
#include <comphelper/comphelperdllapi.h>
#endif


namespace comphelper
{

struct AttributeList_Impl;

class COMPHELPER_DLLPUBLIC AttributeList : public ::cppu::WeakImplHelper1
<
    ::com::sun::star::xml::sax::XAttributeList
>
{
    AttributeList_Impl *m_pImpl;
public:
    AttributeList();
    virtual ~AttributeList();

    // methods that are not contained in any interface
    void AddAttribute( const ::rtl::OUString &sName , const ::rtl::OUString &sType , const ::rtl::OUString &sValue );
    void Clear();
    void RemoveAttribute( const ::rtl::OUString sName );
    void SetAttributeList( const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XAttributeList > & );
    void AppendAttributeList( const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XAttributeList > & );

    // ::com::sun::star::xml::sax::XAttributeList
    virtual sal_Int16 SAL_CALL getLength(void)
        throw( ::com::sun::star::uno::RuntimeException );
    virtual ::rtl::OUString SAL_CALL getNameByIndex(sal_Int16 i)
        throw( ::com::sun::star::uno::RuntimeException );
    virtual ::rtl::OUString SAL_CALL getTypeByIndex(sal_Int16 i)
        throw( ::com::sun::star::uno::RuntimeException );
    virtual ::rtl::OUString SAL_CALL getTypeByName(const ::rtl::OUString& aName)
        throw( ::com::sun::star::uno::RuntimeException );
    virtual ::rtl::OUString SAL_CALL getValueByIndex(sal_Int16 i)
        throw( ::com::sun::star::uno::RuntimeException );
    virtual ::rtl::OUString SAL_CALL getValueByName(const ::rtl::OUString& aName)
        throw( ::com::sun::star::uno::RuntimeException );

};

} // namespace comphelper

#endif // _COMPHELPER_ATTRIBUTE_LIST_HXX

