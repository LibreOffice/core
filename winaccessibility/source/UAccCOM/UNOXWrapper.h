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

#ifndef __UNOXWRAPPER_H_
#define __UNOXWRAPPER_H_

#define WNT

#include "resource.h"       // main symbols


#include <com/sun/star/accessibility/XAccessible.hpp>
#include "UAccCOM2.h"

/**
 * CUNOXWrapper implements IUNOXWrapper interface.
 */
class ATL_NO_VTABLE CUNOXWrapper : public IUNOXWrapper
{
public:
    CUNOXWrapper()
    {   }

public:
    // IUNOXWrapper

    STDMETHOD(put_XInterface)(long pXInterface)
    {
        pUNOInterface = (com::sun::star::accessibility::XAccessible*)pXInterface;
        return S_OK;
    }

    STDMETHOD(put_XSubInterface)(long)
    {
        return S_OK;
    }

    STDMETHOD(get_XInterface)(/*[out,retval]*/long *pXInterface)
    {
        *pXInterface = (long)pUNOInterface;//.get();
        return S_OK;
    }

    STDMETHOD(get_XSubInterface)(/*[out,retval]*/long *)
    {
        return S_OK;
    }

protected:

    com::sun::star::accessibility::XAccessible* pUNOInterface;
};

#endif //__UNOXWRAPPER_H_
