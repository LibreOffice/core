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

#ifndef INCLUDED_DESKTOP_SOURCE_DEPLOYMENT_INC_DP_RESOURCE_H
#define INCLUDED_DESKTOP_SOURCE_DEPLOYMENT_INC_DP_RESOURCE_H

#include <tools/resid.hxx>
#include <i18nlangtag/languagetag.hxx>
#include "dp_misc.h"
#include "dp_misc_api.hxx"

namespace dp_misc {


ResId getResId( sal_uInt16 id );


DESKTOP_DEPLOYMENTMISC_DLLPUBLIC OUString getResourceString( sal_uInt16 id );

template <typename Unique, sal_uInt16 id>
struct StaticResourceString :
        public ::rtl::StaticWithInit< OUString, Unique > {
    const OUString operator () () { return getResourceString(id); }
};


DESKTOP_DEPLOYMENTMISC_DLLPUBLIC
const LanguageTag & getOfficeLanguageTag();

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
