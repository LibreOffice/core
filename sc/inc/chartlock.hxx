/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



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
