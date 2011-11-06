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



#ifndef _SV_UNOWCNTR_HXX_
#define _SV_UNOWCNTR_HXX_

#include <cppuhelper/weakref.hxx>

typedef sal_Bool (*weakref_searchfunc)( ::com::sun::star::uno::WeakReference< ::com::sun::star::uno::XInterface > xRef, void* pSearchData );

class WeakRefList;

class SvUnoWeakContainer
{
private:
    WeakRefList*    mpList;

public:
    SvUnoWeakContainer() throw();
    ~SvUnoWeakContainer() throw();

    /** inserts the given ref into this container */
    void    insert( ::com::sun::star::uno::WeakReference< ::com::sun::star::uno::XInterface > xRef ) throw();

    /** searches the container for a ref that returns true on the given
        search function
    */
    sal_Bool findRef( ::com::sun::star::uno::WeakReference< ::com::sun::star::uno::XInterface >& rRef, void* pSearchData, weakref_searchfunc pSearchFunc );

    void dispose();
};

#endif // _SV_UNOWCNTR_HXX_

