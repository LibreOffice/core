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



#ifndef DBACCESS_CLOSEVETO_HXX
#define DBACCESS_CLOSEVETO_HXX

#include "dbaccessdllapi.h"

/** === begin UNO includes === **/
#include <com/sun/star/uno/XInterface.hpp>
/** === end UNO includes === **/

#include <boost/scoped_ptr.hpp>

//......................................................................................................................
namespace dbaui
{
//......................................................................................................................

    //==================================================================================================================
    //= CloseVeto
    //==================================================================================================================
    struct CloseVeto_Data;
    /** will add a XCloseListener to a given component, and veto its closing as long as the <code>CloseVeto</code>
        instance is alive.

        If closing has been requested and vetoed while the <code>CloseVeto</code> instance is alive, and the ownership
        went to the <code>CloseVeto</code> instance, then it will close the component in its dtor.
    */
    class DBACCESS_DLLPRIVATE CloseVeto
    {
    public:
        CloseVeto( const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >& i_closeable );
        ~CloseVeto();

    private:
        ::boost::scoped_ptr< CloseVeto_Data >   m_pData;
    };

//......................................................................................................................
} // namespace dbaui
//......................................................................................................................

#endif // DBACCESS_CLOSEVETO_HXX
