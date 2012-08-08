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

#ifndef __FRAMEWORK_UICONFIGURATION_GRAPHICNAMEACCESS_HXX_
#define __FRAMEWORK_UICONFIGURATION_GRAPHICNAMEACCESS_HXX_

#include <stdtypes.h>

#include <com/sun/star/container/XNameContainer.hpp>
#include <com/sun/star/graphic/XGraphic.hpp>
#include <cppuhelper/implbase1.hxx>

namespace framework
{
    class GraphicNameAccess : public ::cppu::WeakImplHelper1< ::com::sun::star::container::XNameAccess >
    {
        public:
            GraphicNameAccess();
            virtual ~GraphicNameAccess();

            void addElement( const rtl::OUString& rName, const ::com::sun::star::uno::Reference< ::com::sun::star::graphic::XGraphic >& rElement );

            // XNameAccess
            virtual ::com::sun::star::uno::Any SAL_CALL getByName( const ::rtl::OUString& aName )
                throw(::com::sun::star::container::NoSuchElementException, ::com::sun::star::lang::WrappedTargetException,
                    ::com::sun::star::uno::RuntimeException);
            virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getElementNames()
                throw(::com::sun::star::uno::RuntimeException);
            virtual sal_Bool SAL_CALL hasByName( const ::rtl::OUString& aName )
                throw(::com::sun::star::uno::RuntimeException);

            // XElementAccess
            virtual sal_Bool SAL_CALL hasElements()
                throw(::com::sun::star::uno::RuntimeException);
            virtual ::com::sun::star::uno::Type SAL_CALL getElementType(  )
                throw(::com::sun::star::uno::RuntimeException);

        private:
            typedef BaseHash< ::com::sun::star::uno::Reference< ::com::sun::star::graphic::XGraphic > > NameGraphicHashMap;
            NameGraphicHashMap m_aNameToElementMap;
            ::com::sun::star::uno::Sequence< rtl::OUString > m_aSeq;
    };
}

#endif // __FRAMEWORK_UICONFIGURATION_GRAPHICNAMEACCESS_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
