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
#ifndef __FRAMEWORK_UIELEMENT_STATUSBARMERGER_HXX_
#define __FRAMEWORK_UIELEMENT_STATUSBARMERGER_HXX_

#include <com/sun/star/beans/PropertyValue.hpp>
#include <rtl/ustring.hxx>
#include <uielement/statusbar.hxx>

namespace framework
{

struct AddonStatusbarItemData
{
    rtl::OUString aLabel;
    sal_uInt16    nItemBits;
};

struct AddonStatusbarItem
{
    rtl::OUString aCommandURL;
    rtl::OUString aLabel;
    rtl::OUString aContext;
    sal_uInt16    nItemBits;
    sal_Int16     nWidth;
};

typedef ::std::vector< AddonStatusbarItem > AddonStatusbarItemContainer;

class StatusbarMerger
{
public:
    static bool IsCorrectContext( const ::rtl::OUString& aContext,
                                  const ::rtl::OUString& aModuleIdentifier );

    static bool ConvertSeqSeqToVector( const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue > > &rSequence,
                                       AddonStatusbarItemContainer& rContainer );

    static sal_uInt16 FindReferencePos( StatusBar* pStatusbar,
                                        const ::rtl::OUString& rReferencePoint );

    static bool ProcessMergeOperation( StatusBar* pStatusbar,
                                       sal_uInt16 nPos,
                                       sal_uInt16& rItemId,
                                       const ::rtl::OUString& rModuleIdentifier,
                                       const ::rtl::OUString& rMergeCommand,
                                       const ::rtl::OUString& rMergeCommandParameter,
                                       const AddonStatusbarItemContainer& rItems );

    static bool ProcessMergeFallback( StatusBar* pStatusbar,
                                      sal_uInt16 nPos,
                                      sal_uInt16& rItemId,
                                      const ::rtl::OUString& rModuleIdentifier,
                                      const ::rtl::OUString& rMergeCommand,
                                      const ::rtl::OUString& rMergeFallback,
                                      const AddonStatusbarItemContainer& rItems );

private:
    StatusbarMerger();
    StatusbarMerger( const StatusbarMerger& );
    StatusbarMerger& operator=( const StatusbarMerger& );
};

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
