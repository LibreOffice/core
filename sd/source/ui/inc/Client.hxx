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



#ifndef SD_CLIENT_HXX
#define SD_CLIENT_HXX


#ifndef _SFX_CLIENTSH_HXX //autogen
#include <sfx2/ipclient.hxx>
#endif
class SdrGrafObj;
class SdrOle2Obj;
class OutlinerParaObject;

namespace sd {

class ViewShell;

/*************************************************************************
|*
|* Client
|*
\************************************************************************/

class Client : public SfxInPlaceClient
{
    ViewShell*      mpViewShell;
    SdrOle2Obj*     pSdrOle2Obj;
    SdrGrafObj*     pSdrGrafObj;
    OutlinerParaObject* pOutlinerParaObj;

    virtual void    ObjectAreaChanged();
    virtual void    RequestNewObjectArea( Rectangle& );
    virtual void    ViewChanged();
    virtual void    MakeVisible();

public:
    Client (SdrOle2Obj* pObj, ViewShell* pSdViewShell, ::Window* pWindow);
    virtual ~Client (void);

    SdrOle2Obj*     GetSdrOle2Obj() const { return pSdrOle2Obj; }
    void            SetSdrGrafObj(SdrGrafObj* pObj) { pSdrGrafObj = pObj; }
    SdrGrafObj*     GetSdrGrafObj() const { return pSdrGrafObj; }
    void            SetOutlinerParaObj (OutlinerParaObject* pObj) { pOutlinerParaObj = pObj; }
    OutlinerParaObject*     GetOutlinerParaObject () const { return pOutlinerParaObj; }
};

} // end of namespace sd

#endif

