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



#ifndef _DBAUI_LOCALRESACCESS_HXX_
#define _DBAUI_LOCALRESACCESS_HXX_

#ifndef _SVTOOLS_LOCALRESACCESS_HXX_
#include <svtools/localresaccess.hxx>
#endif
#ifndef _DBAUI_MODULE_DBU_HXX_
#include "moduledbu.hxx"
#endif

//.........................................................................
namespace dbaui
{
//.........................................................................

//=========================================================================
//= LocalResourceAccess
//=========================================================================
/** helper class for acessing local resources
*/
typedef ::svt::OLocalResourceAccess LRA_Base;
class LocalResourceAccess : protected LRA_Base
{
    OModuleClient m_aModuleClient;
public:
    inline LocalResourceAccess( sal_uInt16 _nId, RESOURCE_TYPE _rType )
        :LRA_Base( ModuleRes( _nId ), _rType )
    {
    }
};

//.........................................................................
}   // namespace dbaui
//.........................................................................

#endif // _DBAUI_LOCALRESACCESS_HXX_

