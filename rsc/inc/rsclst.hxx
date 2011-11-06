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



#ifndef _RSCLST_HXX
#define _RSCLST_HXX

#ifndef _TOOLS_LIST_HXX
#include <tools/list.hxx>
#endif
#include <rscall.h>

class REResourceList;

DECLARE_LIST( RESubResourceList, REResourceList * )

class REResourceList : public RESubResourceList
{
protected:
    REResourceList*  pParent;
    RscId            aRscId;    //Id und Name des Resourceobjektes
    ByteString       aClassName;
    sal_Bool             bVisible;

public:
                     REResourceList();
                     REResourceList( REResourceList * pParentList,
                                     ByteString& rClassName,
                                     const RscId & rResourceID,
                                     sal_Bool   bVisible = sal_False );
                     ~REResourceList();

    REResourceList*  GetParent()     { return pParent; }
    ByteString       GetObjName()    { return aRscId.GetName(); }
    ByteString       GetClassName()  { return aClassName; }
    RscId            GetRscId()      { return aRscId; }
    void             SetRscId( const RscId & rId ){ aRscId = rId; }

    void             SetVisible( sal_Bool bVis )
                                     { bVisible = bVis; }
    sal_Bool             IsVisible()     { return bVisible; }
};

#endif // _RSCLST_HXX
