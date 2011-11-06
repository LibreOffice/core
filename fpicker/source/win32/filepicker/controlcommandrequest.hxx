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



#ifndef _CONTROLCOMMANDREQUEST_HXX_
#define _CONTROLCOMMANDREQUEST_HXX_

//------------------------------------------------------------------------
// includes
//------------------------------------------------------------------------

#include <sal/types.h>

//---------------------------------------------
// declaration
//---------------------------------------------

class CControlCommandRequest
{
public:
    CControlCommandRequest( sal_Int16 aControlId ) :
        m_aControlId( aControlId )
    {
    }

    virtual ~CControlCommandRequest( )
    {
    }

    sal_Int16 SAL_CALL getControlId( ) const
    {
        return m_aControlId;
    }

private:
    sal_Int16 m_aControlId;
};

//---------------------------------------------
//
//---------------------------------------------

class CValueControlCommandRequest : public CControlCommandRequest
{
public:
    CValueControlCommandRequest(
        sal_Int16 aControlId,
        sal_Int16 aControlAction ) :
        CControlCommandRequest( aControlId ),
        m_aControlAction( aControlAction )
    {
    }

    sal_Int16 SAL_CALL getControlAction( ) const
    {
        return m_aControlAction;
    }

private:
    sal_Int16 m_aControlAction;
};

#endif
