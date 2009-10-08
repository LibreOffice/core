/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: ControllerLockGuard.hxx,v $
 * $Revision: 1.3 $
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
#ifndef CHART2_CONTROLLERLOCKGUARD_HXX
#define CHART2_CONTROLLERLOCKGUARD_HXX

#include <com/sun/star/frame/XModel.hpp>
#include "charttoolsdllapi.hxx"

namespace chart
{

/** This guard calls lockControllers at the given Model in the CTOR and
    unlockControllers in the DTOR.  Using this ensures that controllers do not
    remain locked when leaving a function even in case an exception is thrown.
 */
class OOO_DLLPUBLIC_CHARTTOOLS ControllerLockGuard
{
public:
    explicit ControllerLockGuard(
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::frame::XModel > & xModel );
    ~ControllerLockGuard();

private:
    ::com::sun::star::uno::Reference<
            ::com::sun::star::frame::XModel > m_xModel;
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

    void lockControllers();
    void unlockControllers();

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

} //  namespace chart

// CHART2_CONTROLLERLOCKGUARD_HXX
#endif
