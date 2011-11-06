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



#ifndef _SVTOOLS_LOCALRESACCESS_HXX_
#define _SVTOOLS_LOCALRESACCESS_HXX_

#include <tools/rc.hxx>
#include <tools/rcid.h>
#include <osl/diagnose.h>

//.........................................................................
namespace svt
{
//.........................................................................

    //=========================================================================
    //= OLocalResourceAccess
    //=========================================================================
    /** helper class for acessing local resources
    */
    class OLocalResourceAccess : public Resource
    {
    protected:
        ResMgr*     m_pManager;

    public:
        OLocalResourceAccess( const ResId& _rId )
            :Resource( _rId.SetAutoRelease( sal_False ) )
            ,m_pManager( _rId.GetResMgr() )
        {
        }

        OLocalResourceAccess(const ResId& _rId, RESOURCE_TYPE _rType)
            :Resource(_rId.SetRT(_rType).SetAutoRelease(sal_False))
            ,m_pManager(_rId.GetResMgr())
        {
            OSL_ENSURE( m_pManager != NULL, "OLocalResourceAccess::OLocalResourceAccess: invalid resource manager!" );
        }

        ~OLocalResourceAccess()
        {
            if ( m_pManager )
                m_pManager->Increment( m_pManager->GetRemainSize() );
            FreeResource();
        }

        inline sal_Bool IsAvailableRes( const ResId& _rId ) const
        {
            return Resource::IsAvailableRes( _rId );
        }
    };

//.........................................................................
}   // namespace svt
//.........................................................................

#endif // _SVTOOLS_LOCALRESACCESS_HXX_

