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



#ifndef SD_FU_CONSTRUCT_3D_OBJECT_HXX
#define SD_FU_CONSTRUCT_3D_OBJECT_HXX

#include "fuconstr.hxx"

class E3dCompoundObject;
class E3dScene;
class SdDrawDocument;
class SfxRequest;

namespace sd {

class FuConstruct3dObject
    : public FuConstruct
{
public:
    static FunctionReference Create( ViewShell* pViewSh, ::sd::Window* pWin, ::sd::View* pView, SdDrawDocument* pDoc, SfxRequest& rReq, bool bPermanent );
    virtual void DoExecute( SfxRequest& rReq );

    // Mouse- & Key-Events
    virtual bool KeyInput(const KeyEvent& rKEvt);
    virtual bool MouseMove(const MouseEvent& rMEvt);
    virtual bool MouseButtonUp(const MouseEvent& rMEvt);
    virtual bool MouseButtonDown(const MouseEvent& rMEvt);

    virtual void Activate();           // Function aktivieren
    virtual void Deactivate();         // Function deaktivieren

    // #97016#
    virtual SdrObject* CreateDefaultObject(const sal_uInt16 nID, const basegfx::B2DRange& rRange);

private:
    FuConstruct3dObject (
        ViewShell* pViewSh,
        ::sd::Window* pWin,
        ::sd::View* pView,
        SdDrawDocument* pDoc,
        SfxRequest& rReq);

    // #97016#
    void ImpPrepareBasic3DShape(E3dCompoundObject* p3DObj, E3dScene *pScene);
    E3dCompoundObject* ImpCreateBasic3DShape();
};

} // end of namespace sd

#endif

