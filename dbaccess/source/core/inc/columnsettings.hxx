/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
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

#ifndef COLUMNSETTINGS_HXX
#define COLUMNSETTINGS_HXX

/** === begin UNO includes === **/
#include <com/sun/star/beans/XPropertySet.hpp>
/** === end UNO includes === **/

namespace dbaccess
{

    // TODO: move the following to comphelper/propertycontainerhelper.hxx
    class IPropertyContainer
    {
    public:
        virtual void registerProperty(
                    const ::rtl::OUString& _rName,
                    sal_Int32 _nHandle,
                    sal_Int32 _nAttributes,
                    void* _pPointerToMember,
                    const ::com::sun::star::uno::Type& _rMemberType
                ) = 0;

        virtual void registerMayBeVoidProperty(
                    const ::rtl::OUString& _rName,
                    sal_Int32 _nHandle,
                    sal_Int32 _nAttributes,
                    ::com::sun::star::uno::Any* _pPointerToMember,
                    const ::com::sun::star::uno::Type& _rExpectedType
                ) = 0;

        virtual void registerPropertyNoMember(
                    const ::rtl::OUString& _rName,
                    sal_Int32 _nHandle,
                    sal_Int32 _nAttributes,
                    const ::com::sun::star::uno::Type& _rType,
                    const void* _pInitialValue
                ) = 0;
    };

    //====================================================================
    //= OColumnSettings
    //====================================================================
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
