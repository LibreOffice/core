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


#ifndef INCLUDED_VCL_FILTERCONFIGITEM_HXX
#define INCLUDED_VCL_FILTERCONFIGITEM_HXX

#include <vcl/dllapi.h>

#include <rtl/ustring.hxx>
#include <com/sun/star/awt/Size.hpp>
#include <com/sun/star/uno/Any.h>
#include <com/sun/star/uno/Reference.h>
#include <com/sun/star/uno/XInterface.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/beans/XPropertyAccess.hpp>
#include <com/sun/star/task/XStatusIndicator.hpp>

class VCL_DLLPUBLIC FilterConfigItem
{
    ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > xUpdatableView;
    ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet > xPropSet;
    ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue > aFilterData;

    bool    bModified;

    bool ImplGetPropertyValue( ::com::sun::star::uno::Any& rAny,
                    const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& rXPropSet,
                        const OUString& rPropName,
                            bool bTestPropertyAvailability );

    void     ImpInitTree( const OUString& rTree );


    static ::com::sun::star::beans::PropertyValue* GetPropertyValue(
                ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& rPropSeq,
                    const OUString& rName );
    static  bool WritePropertyValue(
                ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& rPropSeq,
                    const ::com::sun::star::beans::PropertyValue& rPropValue );

public :

    FilterConfigItem( const OUString& rSubTree );
    FilterConfigItem( ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >* pFilterData );
    FilterConfigItem( const OUString& rSubTree, ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >* pFilterData );
    ~FilterConfigItem();

    // all read methods are trying to return values in following order:
    // 1. FilterData PropertySequence
    // 2. configuration
    // 3. given default
    bool    ReadBool( const OUString& rKey, bool bDefault );
    sal_Int32   ReadInt32( const OUString& rKey, sal_Int32 nDefault );
    OUString
                ReadString( const OUString& rKey, const OUString& rDefault );

    // try to store to configuration
    // and always stores into the FilterData sequence
    void        WriteBool( const OUString& rKey, bool bValue );
    void        WriteInt32( const OUString& rKey, sal_Int32 nValue );

    ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue > GetFilterData() const { return aFilterData;}

    // GetStatusIndicator is returning the "StatusIndicator" property of the FilterData sequence
    ::com::sun::star::uno::Reference< ::com::sun::star::task::XStatusIndicator > GetStatusIndicator() const;
};

#endif // INCLUDED_VCL_FILTERCONFIGITEM_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
