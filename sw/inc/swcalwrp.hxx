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

#ifndef INCLUDED_SW_INC_SWCALWRP_HXX
#define INCLUDED_SW_INC_SWCALWRP_HXX

#include <i18nlangtag/lang.h>
#include <rtl/ustring.hxx>
#include <unotools/calendarwrapper.hxx>
#include <salhelper/singletonref.hxx>

class SwCalendarWrapper : public CalendarWrapper
{
    OUString sUniqueId;
    sal_uInt16 nLang;

public:
    SwCalendarWrapper( const css::uno::Reference< css::uno::XComponentContext > & rxContext = ::comphelper::getProcessComponentContext() )
        : CalendarWrapper( rxContext ), nLang( LANGUAGE_SYSTEM )
    {}

    void LoadDefaultCalendar( sal_uInt16 nLang );
};

salhelper::SingletonRef<SwCalendarWrapper>* s_getCalendarWrapper();

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
