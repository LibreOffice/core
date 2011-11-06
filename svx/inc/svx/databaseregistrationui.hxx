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



#ifndef SVX_DATABASE_REGISTRATION_UI_HXX
#define SVX_DATABASE_REGISTRATION_UI_HXX

#include "svx/svxdllapi.h"
#include <tools/solar.h>

class Window;

#define SID_SB_POOLING_ENABLED          (RID_OFA_START + 247)
#define SID_SB_DRIVER_TIMEOUTS          (RID_OFA_START + 248)
#define SID_SB_DB_REGISTER              (RID_OFA_START + 249)

//........................................................................
namespace svx
{
//........................................................................

    /** opens a dialog which allows the user to administrate the database registrations
    */
    sal_uInt16  SVX_DLLPUBLIC  administrateDatabaseRegistration( Window* _parentWindow );

//........................................................................
}   // namespace svx
//........................................................................

#endif // SVX_DATABASE_REGISTRATION_UI_HXX
