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

#ifndef INCLUDED_SFX2_SFXUNO_HXX
#define INCLUDED_SFX2_SFXUNO_HXX

#include <sal/config.h>
#include <sfx2/dllapi.h>
#include <com/sun/star/uno/Exception.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/util/URL.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/lang/XSingleServiceFactory.hpp>
#include <com/sun/star/lang/XTypeProvider.hpp>
#include <com/sun/star/beans/NamedValue.hpp>

#include <com/sun/star/uno/Type.h>
#include <com/sun/star/uno/Any.h>
#include <com/sun/star/uno/Reference.h>
#include <com/sun/star/uno/Sequence.hxx>
#include <cppuhelper/typeprovider.hxx>
#include <cppuhelper/queryinterface.hxx>
#include <cppuhelper/factory.hxx>
#include <cppuhelper/supportsservice.hxx>
#include <osl/mutex.hxx>
#include <rtl/ustring.hxx>


class SfxAllItemSet ;
class SfxItemSet    ;
class SfxSlot       ;

SFX2_DLLPUBLIC void TransformParameters(            sal_uInt16                          nSlotId     ,
                            const   css::uno::Sequence< css::beans::PropertyValue >&    seqArgs     ,
                                    SfxAllItemSet&                                      aSet        ,
                            const   SfxSlot*                                            pSlot = nullptr   );

SFX2_DLLPUBLIC void TransformItems(         sal_uInt16                                  nSlotId     ,
                            const   SfxItemSet&                                         aSet        ,
                                    css::uno::Sequence< css::beans::PropertyValue >&    seqArgs     ,
                            const   SfxSlot*                            pSlot = nullptr   );

bool GetEncryptionData_Impl( const SfxItemSet* pSet, css::uno::Sequence< css::beans::NamedValue >& aEncryptionData );

typedef sal_Int32 FrameSearchFlags;

#endif // INCLUDED_SFX2_SFXUNO_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
