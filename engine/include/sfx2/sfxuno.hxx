/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
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

#pragma once

#include <sal/config.h>
#include <sfx2/dllapi.h>

#include <cpo/uno/Any.hxx>

#include <string_view>
#include <utility>
#include <vector>

namespace com::sun::star::beans { struct NamedValue; }
namespace com::sun::star::beans { struct PropertyValue; }
namespace cpo::uno { template <class E> class Sequence; }

class SfxAllItemSet ;
class SfxItemSet    ;
class SfxSlot       ;

/// The UNO arguments that an SfxItemSet was transformed into
/// This is similar to SequenceAsHashMap, but maintains the order of arguments
class SFX2_DLLPUBLIC SfxUnoArguments
{
public:
    /// Returns the named argument, appending it to the end when it is not there yet. Note that
    /// the reference is invalidated on following insertions / deletions
    cpo::uno::Any& operator[](const OUString& rName);

    /// Returns the named argument, or an empty Any when there is none
    cpo::uno::Any getValue(std::u16string_view aName) const;

    template <class T>
    T getUnpackedValueOrDefault(std::u16string_view aName, const T& rDefault) const
    {
        if (T aValue = T(); getValue(aName) >>= aValue)
            return aValue;
        return rDefault;
    }

    bool contains(std::u16string_view aName) const;

    /// Drops the named argument; the remaining ones keep their relative order
    void erase(std::u16string_view aName);

    cpo::uno::Sequence<css::beans::PropertyValue> getAsConstPropertyValueList() const;

private:
    std::vector<std::pair<OUString, cpo::uno::Any>> m_aArgs;
};

SFX2_DLLPUBLIC void TransformParameters(            sal_uInt16                          nSlotId     ,
                            const   cpo::uno::Sequence< css::beans::PropertyValue >&    seqArgs     ,
                                    SfxAllItemSet&                                      aSet        ,
                            const   SfxSlot*                                            pSlot = nullptr   );

SFX2_DLLPUBLIC SfxUnoArguments
TransformItems(sal_uInt16 nSlotId, const SfxItemSet& aSet, const SfxSlot* pSlot = nullptr);

bool GetEncryptionData_Impl( const SfxItemSet* pSet, cpo::uno::Sequence< css::beans::NamedValue >& aEncryptionData );

typedef sal_Int32 FrameSearchFlags;

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
