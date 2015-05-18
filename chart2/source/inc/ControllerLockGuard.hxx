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
#ifndef INCLUDED_CHART2_SOURCE_INC_CONTROLLERLOCKGUARD_HXX
#define INCLUDED_CHART2_SOURCE_INC_CONTROLLERLOCKGUARD_HXX

#include <com/sun/star/frame/XModel.hpp>
#include "charttoolsdllapi.hxx"
#include "ChartModel.hxx"

namespace chart
{

/** This guard calls lockControllers at the given Model in the CTOR and
    unlockControllers in the DTOR.  Using this ensures that controllers do not
    remain locked when leaving a function even in case an exception is thrown.
 */
class OOO_DLLPUBLIC_CHARTTOOLS ControllerLockGuardUNO
{
public:
    explicit ControllerLockGuardUNO(
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::frame::XModel > & xModel );
    ~ControllerLockGuardUNO();

private:
    com::sun::star::uno::Reference< com::sun::star::frame::XModel > mxModel;
};

class OOO_DLLPUBLIC_CHARTTOOLS ControllerLockGuard
{
public:
    explicit ControllerLockGuard( ChartModel& rModel );
    ~ControllerLockGuard();

private:
    ChartModel& mrModel;
};

/** This helper class can be used to pass a locking mechanism to other objects
    without exposing the full XModel to it.

    Use the ControllerLockHelperGuard to lock/unlock the model during a block of
    instructions.
 */
class OOO_DLLPUBLIC_CHARTTOOLS ControllerLockHelper
{
public:
    explicit ControllerLockHelper(
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::frame::XModel > & xModel );
    ~ControllerLockHelper();

    SAL_DLLPRIVATE void lockControllers();
    SAL_DLLPRIVATE void unlockControllers();

private:
    ::com::sun::star::uno::Reference<
            ::com::sun::star::frame::XModel > m_xModel;
};

/** This guard calls lockControllers at the given ControllerLockHelper in the
    CTOR and unlockControllers in the DTOR.  Using this ensures that controllers
    do not remain locked when leaving a function even in case an exception is
    thrown.
 */
class OOO_DLLPUBLIC_CHARTTOOLS ControllerLockHelperGuard
{
public:
    explicit ControllerLockHelperGuard( ControllerLockHelper & rHelper );
    ~ControllerLockHelperGuard();

private:
    ControllerLockHelper & m_rHelper;
};

}

// INCLUDED_CHART2_SOURCE_INC_CONTROLLERLOCKGUARD_HXX
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
