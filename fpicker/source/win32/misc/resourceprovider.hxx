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




#ifndef _RESOURCEPROVIDER_HXX_
#define _RESOURCEPROVIDER_HXX_

//------------------------------------------------------------------------
// includes
//------------------------------------------------------------------------

#include <sal/types.h>

#ifndef _RTL_USTRING_HXX_
#include <rtl/ustring>
#endif

//------------------------------------------------------------------------
// deklarations
//------------------------------------------------------------------------

class CResourceProvider_Impl;

class CResourceProvider
{
public:
    CResourceProvider( );
    ~CResourceProvider( );

    rtl::OUString getResString( sal_Int16 aId );

private:
    CResourceProvider_Impl* m_pImpl;
};

#endif
