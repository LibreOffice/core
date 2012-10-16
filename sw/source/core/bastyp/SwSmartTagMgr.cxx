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

#include "SwSmartTagMgr.hxx"

#include <docsh.hxx>
#include <osl/mutex.hxx>
#include <swmodule.hxx>
#include <vcl/svapp.hxx>

using namespace com::sun::star;
using namespace com::sun::star::uno;

SwSmartTagMgr* SwSmartTagMgr::mpTheSwSmartTagMgr = 0;

SwSmartTagMgr& SwSmartTagMgr::Get()
{
    if ( !mpTheSwSmartTagMgr )
    {
        mpTheSwSmartTagMgr = new SwSmartTagMgr( SwDocShell::Factory().GetModuleName() );
        mpTheSwSmartTagMgr->Init(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Writer")));
    }
    return *mpTheSwSmartTagMgr;
}

SwSmartTagMgr::SwSmartTagMgr( const rtl::OUString& rModuleName ) :
    SmartTagMgr( rModuleName )
{
}

SwSmartTagMgr::~SwSmartTagMgr()
{
}

void SwSmartTagMgr::modified( const lang::EventObject& rEO ) throw( RuntimeException )
{
    SolarMutexGuard aGuard;

    // Installed recognizers have changed. We remove all existing smart tags:
    SW_MOD()->CheckSpellChanges( false, true, true, true );

    SmartTagMgr::modified( rEO );
}

void SwSmartTagMgr::changesOccurred( const util::ChangesEvent& rEvent ) throw( RuntimeException)
{
    SolarMutexGuard aGuard;

    // Configuration has changed. We remove all existing smart tags:
    SW_MOD()->CheckSpellChanges( false, true, true, true );

    SmartTagMgr::changesOccurred( rEvent );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
