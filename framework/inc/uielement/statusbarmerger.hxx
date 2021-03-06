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
#pragma once

#include <sal/config.h>

#include <string_view>

#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/uno/Sequence.hxx>
#include <rtl/ustring.hxx>
#include <vcl/status.hxx>

namespace framework
{

struct AddonStatusbarItemData
{
    OUString     aLabel;
};

struct AddonStatusbarItem
{
    OUString     aCommandURL;
    OUString     aLabel;
    OUString     aContext;
    StatusBarItemBits nItemBits;
    sal_Int16         nWidth;
};

typedef ::std::vector< AddonStatusbarItem > AddonStatusbarItemContainer;

namespace StatusbarMerger
{
    bool IsCorrectContext( std::u16string_view aContext );

    bool ConvertSeqSeqToVector( const css::uno::Sequence< css::uno::Sequence< css::beans::PropertyValue > >& rSequence,
                                       AddonStatusbarItemContainer& rContainer );

    sal_uInt16 FindReferencePos( StatusBar* pStatusbar,
                                        std::u16string_view rReferencePoint );

    bool ProcessMergeOperation( StatusBar* pStatusbar,
                                       sal_uInt16 nPos,
                                       sal_uInt16& rItemId,
                                       std::u16string_view rMergeCommand,
                                       const OUString& rMergeCommandParameter,
                                       const AddonStatusbarItemContainer& rItems );

    bool ProcessMergeFallback( StatusBar* pStatusbar,
                                      sal_uInt16& rItemId,
                                      std::u16string_view rMergeCommand,
                                      std::u16string_view rMergeFallback,
                                      const AddonStatusbarItemContainer& rItems );
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
