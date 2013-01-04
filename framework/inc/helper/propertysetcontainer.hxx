/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#ifndef __FRAMEWORK_HELPER_PROPERTYSETCONTAINER_HXX_
#define __FRAMEWORK_HELPER_PROPERTYSETCONTAINER_HXX_

/** Attention: stl headers must(!) be included at first. Otherwhise it can make trouble
               with solaris headers ...
*/
#include <vector>
#include <cppuhelper/weak.hxx>
#include <com/sun/star/container/XIndexContainer.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <threadhelp/threadhelpbase.hxx>
#include <framework/fwedllapi.h>

namespace framework
{

class FWE_DLLPUBLIC PropertySetContainer : public com::sun::star::container::XIndexContainer    ,
                             public ThreadHelpBase                              ,   // Struct for right initalization of mutex member! Must be first of baseclasses.
                             public ::cppu::OWeakObject
{
    public:
        PropertySetContainer();
        virtual ~PropertySetContainer();

        // XInterface
        virtual ::com::sun::star::uno::Any SAL_CALL queryInterface( const ::com::sun::star::uno::Type& aType )
            throw (::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL acquire() throw ();
        virtual void SAL_CALL release() throw ();

        // XIndexContainer
        virtual void SAL_CALL insertByIndex( sal_Int32 Index, const ::com::sun::star::uno::Any& Element )
            throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);

        virtual void SAL_CALL removeByIndex( sal_Int32 Index )
            throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);

        // XIndexReplace
        virtual void SAL_CALL replaceByIndex( sal_Int32 Index, const ::com::sun::star::uno::Any& Element )
            throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);

        // XIndexAccess
        virtual sal_Int32 SAL_CALL getCount()
            throw (::com::sun::star::uno::RuntimeException);

        virtual ::com::sun::star::uno::Any SAL_CALL getByIndex( sal_Int32 Index )
            throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);

        // XElementAccess
        virtual ::com::sun::star::uno::Type SAL_CALL getElementType()
            throw (::com::sun::star::uno::RuntimeException)
        {
            return ::getCppuType((com::sun::star::uno::Reference< com::sun::star::beans::XPropertySet >*)0);
        }

        virtual sal_Bool SAL_CALL hasElements()
            throw (::com::sun::star::uno::RuntimeException);

    private:
        typedef std::vector< com::sun::star::uno::Reference< com::sun::star::beans::XPropertySet > > PropertySetVector;
        PropertySetVector                                                               m_aPropertySetVector;

};

}

#endif // __FRAMEWORK_CLASSES_PROPERTYSETCONTAINER_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
