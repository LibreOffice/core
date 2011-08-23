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

#ifndef _XMLOFF_FORMS_EVENTEXPORT_HXX_
#define _XMLOFF_FORMS_EVENTEXPORT_HXX_

#include <com/sun/star/container/XNameReplace.hpp>
#include <com/sun/star/script/ScriptEventDescriptor.hpp>
#include <cppuhelper/implbase1.hxx>
#include <comphelper/stl_types.hxx>
namespace binfilter {

//.........................................................................
namespace xmloff
{
//.........................................................................

    //=====================================================================
    //= OEventDescriptorMapper
    //=====================================================================
    typedef ::cppu::WeakImplHelper1	<	::com::sun::star::container::XNameReplace
                                    >	OEventDescriptorMapper_Base;
    /** helper class wrapping different script event representations

        <p>In the form layer, the script events are represented by <type scope="com.sun.star.script">ScriptEventDescriptor</type>
        instances. The office applications, on the other hand, represent their a single script event as sequence
        of <type scope="com.sun.star.beans">PropertyValue</type>s, where all events of a given object are
        accessible through a <type scope="com.sun.star.container">XNameReplace</type> interface.</p>
        <p>This class maps the first representation of events of a single object to the second one.</p>
        <p>This way, we can use the helper classes here in the project.</p>
    */
    class OEventDescriptorMapper : public OEventDescriptorMapper_Base
    {
    protected:
        DECLARE_STL_USTRINGACCESS_MAP( ::com::sun::star::uno::Sequence < ::com::sun::star::beans::PropertyValue >, MapString2PropertyValueSequence );
        MapString2PropertyValueSequence	m_aMappedEvents;

    public:
        OEventDescriptorMapper(
            const ::com::sun::star::uno::Sequence< ::com::sun::star::script::ScriptEventDescriptor >& _rEvents);

        // XNameReplace
        virtual void SAL_CALL replaceByName( const ::rtl::OUString& aName, const ::com::sun::star::uno::Any& aElement ) throw(::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::container::NoSuchElementException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);

        // XNameAccess
        virtual ::com::sun::star::uno::Any SAL_CALL getByName( const ::rtl::OUString& aName ) throw(::com::sun::star::container::NoSuchElementException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getElementNames(  ) throw(::com::sun::star::uno::RuntimeException);
        virtual sal_Bool SAL_CALL hasByName( const ::rtl::OUString& aName ) throw(::com::sun::star::uno::RuntimeException);

        // XElementAccess
        virtual ::com::sun::star::uno::Type SAL_CALL getElementType(  ) throw(::com::sun::star::uno::RuntimeException);
        virtual sal_Bool SAL_CALL hasElements(  ) throw(::com::sun::star::uno::RuntimeException);
    };


//.........................................................................
}	// namespace xmloff
//.........................................................................

}//end of namespace binfilter
#endif // _XMLOFF_FORMS_EVENTEXPORT_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
