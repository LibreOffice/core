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

#ifndef INCLUDED_FPICKER_SOURCE_WIN32_FILEPICKER_EVENTNOTIFICATION_HXX
#define INCLUDED_FPICKER_SOURCE_WIN32_FILEPICKER_EVENTNOTIFICATION_HXX

#include <com/sun/star/uno/XInterface.hpp>
#include <com/sun/star/uno/Reference.hxx>


// encapsulate a filepicker event
// notification, because there are
// two types of filepicker notifications
// with and without parameter
// this is an application of the
// "command" pattern see GoF


class CEventNotification
{
public:
    virtual ~CEventNotification() { };

    virtual void SAL_CALL notifyEventListener( css::uno::Reference< css::uno::XInterface > xListener ) = 0;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
