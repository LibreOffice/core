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


#ifndef _FILTER_CONFIG_ITEM_HXX_
#define _FILTER_CONFIG_ITEM_HXX_

#include "svtools/svtdllapi.h"
#include <tools/string.hxx>
#include <com/sun/star/awt/Size.hpp>
#include <com/sun/star/uno/Any.h>
#include <com/sun/star/uno/Reference.h>
#include <com/sun/star/uno/XInterface.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/beans/XPropertyAccess.hpp>
#include <com/sun/star/task/XStatusIndicator.hpp>

class SVT_DLLPUBLIC FilterConfigItem
{
        ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > xUpdatableView;
        ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet > xPropSet;
        ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue > aFilterData;

        sal_Bool    bModified;

        SVT_DLLPRIVATE sal_Bool ImplGetPropertyValue( ::com::sun::star::uno::Any& rAny,
                        const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& rXPropSet,
                            const ::rtl::OUString& rPropName,
                                sal_Bool bTestPropertyAvailability );

        SVT_DLLPRIVATE void     ImpInitTree( const String& rTree );


        SVT_DLLPRIVATE static ::com::sun::star::beans::PropertyValue* GetPropertyValue(
                    ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& rPropSeq,
                        const ::rtl::OUString& rName );
        SVT_DLLPRIVATE static  sal_Bool WritePropertyValue(
                    ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& rPropSeq,
                        const ::com::sun::star::beans::PropertyValue& rPropValue );

    public :

        FilterConfigItem( const ::rtl::OUString& rSubTree );
        FilterConfigItem( ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >* pFilterData );
        FilterConfigItem( const ::rtl::OUString& rSubTree, ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >* pFilterData );
        ~FilterConfigItem();

        // all read methods are trying to return values in following order:
        // 1. FilterData PropertySequence
        // 2. configuration
        // 3. given default
        sal_Bool    ReadBool( const ::rtl::OUString& rKey, sal_Bool bDefault );
        sal_Int32   ReadInt32( const ::rtl::OUString& rKey, sal_Int32 nDefault );
        ::com::sun::star::awt::Size
                    ReadSize( const ::rtl::OUString& rKey, const ::com::sun::star::awt::Size& rDefault );
        ::rtl::OUString
                    ReadString( const ::rtl::OUString& rKey, const ::rtl::OUString& rDefault );
        ::com::sun::star::uno::Any
                    ReadAny( const ::rtl::OUString& rKey, const ::com::sun::star::uno::Any& rDefault );

        // try to store to configuration
        // and always stores into the FilterData sequence
        void        WriteBool( const ::rtl::OUString& rKey, sal_Bool bValue );
        void        WriteInt32( const ::rtl::OUString& rKey, sal_Int32 nValue );
        void        WriteSize( const ::rtl::OUString& rKey, const ::com::sun::star::awt::Size& rSize );
        void        WriteString( const ::rtl::OUString& rKey, const ::rtl::OUString& rString );
        void        WriteAny( const rtl::OUString& rKey, const ::com::sun::star::uno::Any& rAny );

        ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue > GetFilterData() const;

        // GetStatusIndicator is returning the "StatusIndicator" property of the FilterData sequence
        ::com::sun::star::uno::Reference< ::com::sun::star::task::XStatusIndicator > GetStatusIndicator() const;
};

#endif  // _FILTER_CONFIG_ITEM_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
