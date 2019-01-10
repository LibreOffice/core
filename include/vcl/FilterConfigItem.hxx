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
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/uno/Reference.h>
#include <com/sun/star/uno/Sequence.hxx>

namespace com { namespace sun { namespace star { namespace beans { class XPropertySet; } } } }
namespace com { namespace sun { namespace star { namespace task { class XStatusIndicator; } } } }
namespace com { namespace sun { namespace star { namespace uno { class Any; } } } }
namespace com { namespace sun { namespace star { namespace uno { class XInterface; } } } }

class VCL_DLLPUBLIC FilterConfigItem
{
    css::uno::Reference< css::uno::XInterface > xUpdatableView;
    css::uno::Reference< css::beans::XPropertySet > xPropSet;
    css::uno::Sequence< css::beans::PropertyValue > aFilterData;

    bool    bModified;

    static bool ImplGetPropertyValue( css::uno::Any& rAny,
                            const css::uno::Reference< css::beans::XPropertySet >& rXPropSet,
                            const OUString& rPropName );

    void     ImpInitTree( const OUString& rTree );


    static css::beans::PropertyValue* GetPropertyValue(
                css::uno::Sequence< css::beans::PropertyValue >& rPropSeq,
                    const OUString& rName );
    static  bool WritePropertyValue(
                css::uno::Sequence< css::beans::PropertyValue >& rPropSeq,
                    const css::beans::PropertyValue& rPropValue );

public:

    FilterConfigItem( const OUString& rSubTree );
    FilterConfigItem( css::uno::Sequence< css::beans::PropertyValue > const * pFilterData );
    FilterConfigItem( const OUString& rSubTree, css::uno::Sequence< css::beans::PropertyValue > const * pFilterData );
    /// Writes config in destructor
    ~FilterConfigItem();
    /// Writes config and sets unmodified state again.
    void WriteModifiedConfig();

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

    const css::uno::Sequence< css::beans::PropertyValue >& GetFilterData() const { return aFilterData;}

    // GetStatusIndicator is returning the "StatusIndicator" property of the FilterData sequence
    css::uno::Reference< css::task::XStatusIndicator > GetStatusIndicator() const;
};

#endif // INCLUDED_VCL_FILTERCONFIGITEM_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
