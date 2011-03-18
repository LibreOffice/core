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

#if !defined INCLUDED_DP_GUI_SHARED_HXX
#define INCLUDED_DP_GUI_SHARED_HXX

#include "unotools/configmgr.hxx"
#include "rtl/instance.hxx"
#include "tools/resmgr.hxx"


namespace css = ::com::sun::star;

namespace dp_gui {

struct DeploymentGuiResMgr :
    public ::rtl::StaticWithInit< ResMgr *, DeploymentGuiResMgr > {
        ResMgr * operator () () {
            return ResMgr::CreateResMgr( "deploymentgui" );
    }
};

struct BrandName : public ::rtl::StaticWithInit<const ::rtl::OUString, BrandName> {
    const ::rtl::OUString operator () () {
        return ::utl::ConfigManager::GetDirectConfigProperty(
            ::utl::ConfigManager::PRODUCTNAME ).get< ::rtl::OUString >();
    }
};

class DpGuiResId : public ResId
{
public:
    DpGuiResId( sal_uInt16 nId ):ResId( nId, *DeploymentGuiResMgr::get() ) {}
};

} // namespace dp_gui

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
