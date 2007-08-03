/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: chartlock.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: hr $ $Date: 2007-08-03 13:06:24 $
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

#ifndef SC_CHARTLOCK_HXX
#define SC_CHARTLOCK_HXX


#include <vcl/timer.hxx>

#include <cppuhelper/weakref.hxx>
#include <com/sun/star/frame/XModel.hpp>

#include <memory>

class ScDocument;

/** All current charts in the calc will be locked in constructor and unlocked in destructor.
*/
class ScChartLockGuard
{
public:
                    ScChartLockGuard( ScDocument* pDoc );
    virtual         ~ScChartLockGuard();

    void            AlsoLockThisChart( const ::com::sun::star::uno::Reference<
                        ::com::sun::star::frame::XModel >& xModel );

private:
    std::vector< ::com::sun::star::uno::WeakReference<
        ::com::sun::star::frame::XModel > > maChartModels;

    ScChartLockGuard();
    ScChartLockGuard( const ScChartLockGuard& );
};

/** Use this to lock all charts in the calc for a little time.
    They will unlock automatically unless you call StartOrContinueLocking() again.
*/
class ScTemporaryChartLock
{
public:
                    ScTemporaryChartLock( ScDocument* pDoc );
    virtual         ~ScTemporaryChartLock();

    void            StartOrContinueLocking();
    void            StopLocking();
    void            AlsoLockThisChart( const ::com::sun::star::uno::Reference<
                        ::com::sun::star::frame::XModel >& xModel );

private:
    ScDocument*                         mpDoc;
    Timer                               maTimer;
    std::auto_ptr< ScChartLockGuard >   mapScChartLockGuard;

    DECL_LINK( TimeoutHdl, Timer* );

    ScTemporaryChartLock();
    ScTemporaryChartLock( const ScTemporaryChartLock& );
};


#endif
