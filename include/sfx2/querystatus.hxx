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

#ifndef INCLUDED_SFX2_QUERYSTATUS_HXX
#define INCLUDED_SFX2_QUERYSTATUS_HXX

#include <sal/config.h>
#include <sfx2/dllapi.h>
#include <svl/poolitem.hxx>
#include <cppuhelper/weak.hxx>
#include <osl/conditn.hxx>
#include <com/sun/star/frame/FeatureStateEvent.hpp>
#include <com/sun/star/frame/XDispatchProvider.hpp>
#include <com/sun/star/frame/XStatusListener.hpp>

#include <sfx2/sfxuno.hxx>

class SfxQueryStatus_Impl;
class SFX2_DLLPUBLIC SfxQueryStatus
{
    public:
        SfxQueryStatus( const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XDispatchProvider >& rDispatchProvider, sal_uInt16 nSlotId, const OUString& aCommand );
        ~SfxQueryStatus();

        // Query method
        SfxItemState QueryState( SfxPoolItem*& pPoolItem );

    private:
        SfxQueryStatus( const SfxQueryStatus& ) = delete;
        SfxQueryStatus& operator=( const SfxQueryStatus& ) = delete;

        com::sun::star::uno::Reference< com::sun::star::frame::XStatusListener >   m_xStatusListener;
        SfxQueryStatus_Impl*                                                       m_pSfxQueryStatusImpl;
};

#endif // INCLUDED_SFX2_QUERYSTATUS_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
