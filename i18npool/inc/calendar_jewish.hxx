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
#ifndef INCLUDED_I18NPOOL_INC_CALENDAR_JEWISH_HXX
#define INCLUDED_I18NPOOL_INC_CALENDAR_JEWISH_HXX

#include "calendar_gregorian.hxx"


//  class Calendar_jewish


namespace i18npool {

class Calendar_jewish : public Calendar_gregorian
{
public:
    // Constructors
    Calendar_jewish();

    // Methods in XExtendedCalendar
    virtual OUString SAL_CALL getDisplayString( sal_Int32 nCalendarDisplayCode, sal_Int16 nNativeNumberMode ) override;

protected:
    void mapToGregorian() override;
    void mapFromGregorian() override;
};

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
