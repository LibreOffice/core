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

#ifndef INCLUDED_COMPHELPER_PROPERTYSETINFO_HXX
#define INCLUDED_COMPHELPER_PROPERTYSETINFO_HXX

#include <sal/config.h>

#include <map>

#include <com/sun/star/beans/XPropertySetInfo.hpp>
#include <cppuhelper/implbase.hxx>
#include <comphelper/comphelperdllapi.h>
#include <o3tl/typed_flags_set.hxx>
#include <memory>

enum class PropertyMoreFlags : sal_uInt8 {
    NONE            = 0x00,
    METRIC_ITEM     = 0x01,
};
namespace o3tl {
    template<> struct typed_flags<PropertyMoreFlags> : is_typed_flags<PropertyMoreFlags, 0x1> {};
}

namespace comphelper
{

struct PropertyMapEntry
{
    OUString       maName;
    css::uno::Type maType;
    sal_Int32      mnHandle;
    /// flag bitmap, @see css::beans::PropertyAttribute
    sal_Int16      mnAttributes;
    sal_uInt8      mnMemberId;
    PropertyMoreFlags mnMoreFlags;

    PropertyMapEntry(OUString _aName, sal_Int32 _nHandle, css::uno::Type const & _rType,
                     sal_Int16 _nAttributes, sal_uInt8 _nMemberId, PropertyMoreFlags _nMoreFlags = PropertyMoreFlags::NONE)
        : maName( _aName )
        , maType( _rType )
        , mnHandle( _nHandle )
        , mnAttributes( _nAttributes )
        , mnMemberId( _nMemberId )
        , mnMoreFlags( _nMoreFlags )
    {
        assert(mnAttributes <= 0x1ff );
        assert( (_nMemberId & 0x40) == 0 );
        // Verify that if METRIC_ITEM is set, we are one of the types supported by
        // SvxUnoConvertToMM.
        assert(!(_nMoreFlags & PropertyMoreFlags::METRIC_ITEM) ||
            ( (maType.getTypeClass() == css::uno::TypeClass_BYTE)
              || (maType.getTypeClass() == css::uno::TypeClass_SHORT)
              || (maType.getTypeClass() == css::uno::TypeClass_UNSIGNED_SHORT)
              || (maType.getTypeClass() == css::uno::TypeClass_LONG)
              || (maType.getTypeClass() == css::uno::TypeClass_UNSIGNED_LONG)
            ) );
    }
    PropertyMapEntry() = default;
};

typedef std::map<OUString, PropertyMapEntry const *> PropertyMap;

class PropertyMapImpl;

// don't export to avoid duplicate WeakImplHelper definitions with MSVC
class SAL_DLLPUBLIC_TEMPLATE PropertySetInfo_BASE
    : public ::cppu::WeakImplHelper< css::beans::XPropertySetInfo >
{};

/** this class implements a XPropertySetInfo that is initialized with arrays of PropertyMapEntry.
    It is used by the class PropertySetHelper.
*/
class COMPHELPER_DLLPUBLIC PropertySetInfo final
    : public PropertySetInfo_BASE
{
private:
    std::unique_ptr<PropertyMapImpl> mpImpl;
public:
    PropertySetInfo() noexcept;
    PropertySetInfo( PropertyMapEntry const * pMap ) noexcept;
    PropertySetInfo(css::uno::Sequence<css::beans::Property> const &) noexcept;
    virtual ~PropertySetInfo() noexcept override;

    /** returns a stl map with all PropertyMapEntry pointer.<p>
        The key is the property name.
    */
    const PropertyMap& getPropertyMap() const noexcept;

    /** adds an array of PropertyMapEntry to this instance.<p>
        The end is marked with a PropertyMapEntry where mpName equals NULL</p>
    */
    void add( PropertyMapEntry const * pMap ) noexcept;

    /** removes an already added PropertyMapEntry which string in mpName equals to aName */
    void remove( const OUString& aName ) noexcept;

    virtual css::uno::Sequence< css::beans::Property > SAL_CALL getProperties() override;
    virtual css::beans::Property SAL_CALL getPropertyByName( const OUString& aName ) override;
    virtual sal_Bool SAL_CALL hasPropertyByName( const OUString& Name ) override;
};

}

#endif // _UTL_PROPERTSETINFO_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
