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



#ifndef SD_FU_OUTLINE_TEXT_HXX
#define SD_FU_OUTLINE_TEXT_HXX

#include "fuoutl.hxx"

class SdDrawDocument;
class SfxRequest;

namespace sd {

class View;
class ViewShell;
class Window;

/*************************************************************************
|*
|* Textfunktionen im Outlinermodus
|*
\************************************************************************/
class FuOutlineText
    : public FuOutline
{
public:
    static FunctionReference Create( ViewShell* pViewSh, ::sd::Window* pWin, ::sd::View* pView, SdDrawDocument* pDoc, SfxRequest& rReq );

    virtual bool KeyInput(const KeyEvent& rKEvt);
    virtual bool MouseMove(const MouseEvent& rMEvt);
    virtual bool MouseButtonUp(const MouseEvent& rMEvt);
    virtual bool MouseButtonDown(const MouseEvent& rMEvt);

    virtual void DoCut();
    virtual void DoCopy();
    virtual void DoPaste();

    virtual void Activate();           // Function aktivieren
    virtual void Deactivate();         // Function deaktivieren

    /** Call this method when the text in the outliner (may) has changed.
        It will invalidate some slots of the view frame and update the
        preview in the slide sorter.
    */
    void UpdateForKeyPress (const KeyEvent& rEvent);

protected:
    FuOutlineText (
        ViewShell* pViewShell,
        ::sd::Window* pWin,
        ::sd::View* pView,
        SdDrawDocument* pDoc,
        SfxRequest& rReq);

};

} // end of namespace sd

#endif
