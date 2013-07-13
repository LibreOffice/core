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


#include "dp_misc.h"
#include "dp_resource.h"
#include "osl/module.hxx"
#include "osl/mutex.hxx"
#include <tools/resmgr.hxx>
#include "rtl/ustring.h"
#include "cppuhelper/implbase1.hxx"
#include "unotools/configmgr.hxx"
#include <i18nlangtag/languagetag.hxx>


using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;

namespace dp_misc {
namespace {

struct OfficeLocale :
        public rtl::StaticWithInit<LanguageTag, OfficeLocale> {
    const LanguageTag operator () () {
        OUString slang(utl::ConfigManager::getLocale());
        //fallback, the locale is currently only set when the user starts the
        //office for the first time.
        if (slang.isEmpty())
            slang = "en-US";
        return LanguageTag( slang);
    }
};

struct DeploymentResMgr : public rtl::StaticWithInit<
    ResMgr *, DeploymentResMgr> {
    ResMgr * operator () () {
        return ResMgr::CreateResMgr( "deployment", OfficeLocale::get() );
    }
};

class theResourceMutex : public rtl::Static<osl::Mutex, theResourceMutex> {};

} // anon namespace

//==============================================================================
ResId getResId( sal_uInt16 id )
{
    const osl::MutexGuard guard( theResourceMutex::get() );
    return ResId( id, *DeploymentResMgr::get() );
}

//==============================================================================
OUString getResourceString( sal_uInt16 id )
{
    const osl::MutexGuard guard(theResourceMutex::get());
    OUString ret(ResId(id, *DeploymentResMgr::get()).toString());
    return ret.replaceAll("%PRODUCTNAME", utl::ConfigManager::getProductName());
}

//==============================================================================
const LanguageTag & getOfficeLanguageTag()
{
    return OfficeLocale::get();
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
