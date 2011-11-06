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



#ifndef _SWSMARTTAGMGR_HXX
#define _SWSMARTTAGMGR_HXX

#include <svx/SmartTagMgr.hxx>


/*************************************************************************
 *                      class SwSmartTagMgr
 *
 * Wrapper for the svx SmartTagMgr
 *************************************************************************/

class SwSmartTagMgr : public SmartTagMgr
{
private:
    static SwSmartTagMgr* mpTheSwSmartTagMgr;

    SwSmartTagMgr( const rtl::OUString& rModuleName );
    virtual ~SwSmartTagMgr();

public:
    static SwSmartTagMgr& Get();

    // ::com::sun::star::util::XModifyListener
    virtual void SAL_CALL modified( const ::com::sun::star::lang::EventObject& aEvent ) throw(::com::sun::star::uno::RuntimeException);

    // ::com::sun::star::util::XChangesListener
      virtual void SAL_CALL changesOccurred( const ::com::sun::star::util::ChangesEvent& Event ) throw(::com::sun::star::uno::RuntimeException);
};

/*
namespace SwSmartTagMgr
{
    SmartTagMgr& Get();
}
*/

#endif
