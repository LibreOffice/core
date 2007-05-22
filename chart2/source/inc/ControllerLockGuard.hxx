/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: ControllerLockGuard.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2007-05-22 18:14:45 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/
#ifndef CHART2_CONTROLLERLOCKGUARD_HXX
#define CHART2_CONTROLLERLOCKGUARD_HXX

#ifndef _COM_SUN_STAR_FRAME_XMODEL_HPP_
#include <com/sun/star/frame/XModel.hpp>
#endif

namespace chart
{

/** This guard calls lockControllers at the given Model in the CTOR and
    unlockControllers in the DTOR.  Using this ensures that controllers do not
    remain locked when leaving a function even in case an exception is thrown.
 */
class ControllerLockGuard
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
class ControllerLockHelper
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
class ControllerLockHelperGuard
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
