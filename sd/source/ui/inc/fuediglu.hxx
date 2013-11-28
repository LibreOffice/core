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



#ifndef SD_FU_EDIT_GLUE_POINTS_HXX
#define SD_FU_EDIT_GLUE_POINTS_HXX

#include "fudraw.hxx"

namespace sd {

/*************************************************************************
|*
|* FuEditGluePoints
|*
\************************************************************************/

class FuEditGluePoints
    : public FuDraw
{
public:
    TYPEINFO();

    static FunctionReference Create( ViewShell* pViewSh, ::sd::Window* pWin, ::sd::View* pView, SdDrawDocument* pDoc, SfxRequest& rReq, bool bPermanent );
    virtual void DoExecute( SfxRequest& rReq );

    // Mouse- & Key-Events
    virtual sal_Bool KeyInput(const KeyEvent& rKEvt);
    virtual sal_Bool MouseMove(const MouseEvent& rMEvt);
    virtual sal_Bool MouseButtonUp(const MouseEvent& rMEvt);
    virtual sal_Bool MouseButtonDown(const MouseEvent& rMEvt);
    virtual sal_Bool Command(const CommandEvent& rCEvt);
    virtual void ReceiveRequest(SfxRequest& rReq);

    virtual void Activate();           // Function aktivieren
    virtual void Deactivate();         // Function deaktivieren

  //Solution: Add Shift+UP/DOWN/LEFT/RIGHT key to move the position of insert point,
  //and SHIFT+ENTER key to decide the postion and draw the new insert point
    virtual void ForcePointer(const MouseEvent* pMEvt = NULL);
private:
    sal_Bool   bBeginInsertPoint;
    Point    oldPoint;
protected:
    FuEditGluePoints (
        ViewShell* pViewSh,
        ::sd::Window* pWin,
        ::sd::View* pView,
        SdDrawDocument* pDoc,
        SfxRequest& rReq);
    virtual ~FuEditGluePoints (void);
};

} // end of namespace sd

#endif

