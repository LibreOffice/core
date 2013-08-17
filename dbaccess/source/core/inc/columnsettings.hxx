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

#ifndef COLUMNSETTINGS_HXX
#define COLUMNSETTINGS_HXX

#include <com/sun/star/beans/XPropertySet.hpp>

namespace dbaccess
{

    // TODO: move the following to comphelper/propertycontainerhelper.hxx
    class IPropertyContainer
    {
    public:
        virtual void registerProperty(
                    const OUString& _rName,
                    sal_Int32 _nHandle,
                    sal_Int32 _nAttributes,
                    void* _pPointerToMember,
                    const ::com::sun::star::uno::Type& _rMemberType
                ) = 0;

        virtual void registerMayBeVoidProperty(
                    const OUString& _rName,
                    sal_Int32 _nHandle,
                    sal_Int32 _nAttributes,
                    ::com::sun::star::uno::Any* _pPointerToMember,
                    const ::com::sun::star::uno::Type& _rExpectedType
                ) = 0;

        virtual void registerPropertyNoMember(
                    const OUString& _rName,
                    sal_Int32 _nHandle,
                    sal_Int32 _nAttributes,
                    const ::com::sun::star::uno::Type& _rType,
                    const void* _pInitialValue
                ) = 0;

    protected:
        ~IPropertyContainer() {}
    };

    // OColumnSettings
    class OColumnSettings
    {
        //  <properties>
        ::com::sun::star::uno::Any  m_aWidth;               // sal_Int32 or void
        ::com::sun::star::uno::Any  m_aFormatKey;           // sal_Int32 or void
        ::com::sun::star::uno::Any  m_aRelativePosition;    // sal_Int32 or void
        ::com::sun::star::uno::Any  m_aAlignment;           // sal_Int32 (::com::sun::star::awt::TextAlign) or void
        ::com::sun::star::uno::Any  m_aHelpText;            // the description of the column which is visible in the helptext of the column
        ::com::sun::star::uno::Any  m_aControlDefault;      // the default value which should be displayed as by a control when moving to a new row
        ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >
                                    m_xControlModel;
        sal_Bool                    m_bHidden;
        //  </properties>

    protected:
        virtual ~OColumnSettings();

    public:
        OColumnSettings();

    protected:
        void registerProperties( IPropertyContainer& _rPropertyContainer );

        /** determines whether the property with the given handle is handled by the class
        */
        static bool isColumnSettingProperty( const sal_Int32 _nPropertyHandle );
        static bool isDefaulted( const sal_Int32 _nPropertyHandle, const ::com::sun::star::uno::Any& _rPropertyValue );

    public:
        /** check if the persistent settings have their default value
        */
        static bool hasDefaultSettings( const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& _rxColumn );
    };

} // namespace dbaccess

#endif // COLUMNSETTINGS_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
