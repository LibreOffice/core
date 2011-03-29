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

#ifndef _SFXQUERYSTATUS_HXX
#define _SFXQUERYSTATUS_HXX

#include "sal/config.h"
#include "sfx2/dllapi.h"
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
        SfxQueryStatus( const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XDispatchProvider >& rDispatchProvider, sal_uInt16 nSlotId, const rtl::OUString& aCommand );
        ~SfxQueryStatus();

        // Query method
        SfxItemState QueryState( SfxPoolItem*& pPoolItem );

    private:
        SfxQueryStatus( const SfxQueryStatus& );
        SfxQueryStatus();
        SfxQueryStatus& operator=( const SfxQueryStatus& );

        com::sun::star::uno::Reference< com::sun::star::frame::XStatusListener >   m_xStatusListener;
        SfxQueryStatus_Impl*                                                       m_pSfxQueryStatusImpl;
};

#endif // _SFXQUERYSTATUS_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
