/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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

#ifndef _XMLOFF_FORMS_ATTRIBLISTMERGE_HXX_
#define _XMLOFF_FORMS_ATTRIBLISTMERGE_HXX_

#include <comphelper/stl_types.hxx>
#include <cppuhelper/implbase1.hxx>
#include <com/sun/star/xml/sax/XAttributeList.hpp>
namespace binfilter {

//.........................................................................
namespace xmloff
{
//.........................................................................

    //=====================================================================
    //= OAttribListMerger
    //=====================================================================
    typedef ::cppu::WeakImplHelper1	<	::com::sun::star::xml::sax::XAttributeList
                                    >	OAttribListMerger_Base;
    /** implements the XAttributeList list by merging different source attribute lists

        <p>Currently, the time behavious is O(n), though it would be possible to change it to O(log n).</p>
    */
    class OAttribListMerger : public OAttribListMerger_Base
    {
    protected:
        ::osl::Mutex		m_aMutex;
        DECLARE_STL_VECTOR( ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XAttributeList >, AttributeListArray );
        AttributeListArray	m_aLists;

        ~OAttribListMerger() { }

    public:
        OAttribListMerger() { }

        // attribute list handling
        // (very thinn at the moment ... only adding lists is allowed ... add more if you need it :)
        void addList(const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XAttributeList >& _rList);

        // XAttributeList
        virtual sal_Int16 SAL_CALL getLength(  ) throw(::com::sun::star::uno::RuntimeException);
        virtual ::rtl::OUString SAL_CALL getNameByIndex( sal_Int16 i ) throw(::com::sun::star::uno::RuntimeException);
        virtual ::rtl::OUString SAL_CALL getTypeByIndex( sal_Int16 i ) throw(::com::sun::star::uno::RuntimeException);
        virtual ::rtl::OUString SAL_CALL getTypeByName( const ::rtl::OUString& aName ) throw(::com::sun::star::uno::RuntimeException);
        virtual ::rtl::OUString SAL_CALL getValueByIndex( sal_Int16 i ) throw(::com::sun::star::uno::RuntimeException);
        virtual ::rtl::OUString SAL_CALL getValueByName( const ::rtl::OUString& aName ) throw(::com::sun::star::uno::RuntimeException);

    protected:
        sal_Bool seekToIndex(sal_Int16 _nGlobalIndex, ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XAttributeList >& _rSubList, sal_Int16& _rLocalIndex);
        sal_Bool seekToName(const ::rtl::OUString& _rName, ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XAttributeList >& _rSubList, sal_Int16& _rLocalIndex);
    };


//.........................................................................
}	// namespace xmloff
//.........................................................................

}//end of namespace binfilter
#endif // _XMLOFF_FORMS_ATTRIBLISTMERGE_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
