/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: composedprops.hxx,v $
 * $Revision: 1.5 $
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

#ifndef _COMPHELPER_COMPOSEDPROPS_HXX_
#define _COMPHELPER_COMPOSEDPROPS_HXX_

#include <cppuhelper/implbase2.hxx>
#include <comphelper/types.hxx>
#include <comphelper/stl_types.hxx>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/beans/XPropertyState.hpp>
#include <com/sun/star/beans/XPropertySetInfo.hpp>
#include <osl/mutex.hxx>
#include "comphelper/comphelperdllapi.h"

//.........................................................................
namespace comphelper
{
//.........................................................................
    //=====================================================================
    //= IPropertySetComposerCallback
    //=====================================================================
    class IPropertySetComposerCallback
    {
    public:
        /** determines whether or not a property should appear in the composed property set
            @param  _rPropertyName
                the name of the property
        */
        virtual sal_Bool isComposeable(const ::rtl::OUString& _rPropertyName) const = 0;
    };

    //=====================================================================
    //= OComposedPropertySet
    //=====================================================================
    class OComposedPropertySetInfo;
    typedef ::cppu::WeakImplHelper2 <   ::com::sun::star::beans::XPropertySet
                                    ,   ::com::sun::star::beans::XPropertyState
                                    >   OComposedPropertySet_Base;
    /** helper class for composing a property set from a sequence of other property sets.
        <p>First: This class is a fast shot, so don't sue me :) (To be honest, it's the migration of an old ugly
        implementation. It's still ugly).</p>
        <p>The property listener mechanisms are not supported (you can't add property listeners).</p>
        <p>Speaking strictly, the property defaults (getPropertyDefault) do not work correctly, as there's always
        an empty <type scope="com.sun.star.uno">Any</type> returned.</p>
    */
    class COMPHELPER_DLLPUBLIC OComposedPropertySet : public OComposedPropertySet_Base
    {
    private:
        ::com::sun::star::uno::Sequence< ::com::sun::star::beans::Property >
                                    m_aProperties;
        OComposedPropertySetInfo*   m_pInfo;

    protected:
        ::osl::Mutex                m_aMutex;
        DECLARE_STL_VECTOR(::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet>, PropertySetArray);
        PropertySetArray            m_aSingleSets;

    public:
        /** constructs a composed property set

            @param _rElements
                the single property sets to compose
                <p>The first property set in the sequence is the master set, any properties not present here
                are not present in the composed set.<br/>
                This may change in the future (as it's just missing implementation), so don't rely on this behaviour.</p>

            @param _pPropertyMetaData
                the callback for retrieving property meta data (namely composeability)<br/>
                if not specified, all properties are assumed to be composable
        */
        OComposedPropertySet(
            const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet> > & _rElements,
            const IPropertySetComposerCallback* _pPropertyMetaData = NULL
            );

        // XPropertyState
        virtual ::com::sun::star::beans::PropertyState SAL_CALL getPropertyState( const ::rtl::OUString& PropertyName ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyState > SAL_CALL getPropertyStates( const ::com::sun::star::uno::Sequence< ::rtl::OUString >& aPropertyName ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL setPropertyToDefault( const ::rtl::OUString& PropertyName ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::uno::Any SAL_CALL getPropertyDefault( const ::rtl::OUString& aPropertyName ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);

        // XPropertySet
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo(  ) throw(::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL setPropertyValue( const ::rtl::OUString& aPropertyName, const ::com::sun::star::uno::Any& aValue ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::beans::PropertyVetoException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::uno::Any SAL_CALL getPropertyValue( const ::rtl::OUString& PropertyName ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL addPropertyChangeListener( const ::rtl::OUString& aPropertyName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertyChangeListener >& xListener ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL removePropertyChangeListener( const ::rtl::OUString& aPropertyName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertyChangeListener >& aListener ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL addVetoableChangeListener( const ::rtl::OUString& PropertyName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XVetoableChangeListener >& aListener ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL removeVetoableChangeListener( const ::rtl::OUString& PropertyName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XVetoableChangeListener >& aListener ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);

    protected:
                        ~OComposedPropertySet();
        void            compose(const IPropertySetComposerCallback* _pMetaData);
    };

//.........................................................................
}   // namespace comphelper
//.........................................................................

#endif // _COMPHELPER_COMPOSEDPROPS_HXX_


