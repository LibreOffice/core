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



#ifndef SD_PRESENTATION_VIEW_SHELL_HXX
#define SD_PRESENTATION_VIEW_SHELL_HXX

#ifndef SD_DRAW_VIEW_SHELL
#include "DrawViewShell.hxx"
#endif

namespace sd {

/** This view shell is responsible for showing the presentation of an
    Impress document.
*/
class PresentationViewShell : public DrawViewShell
{
public:
    SFX_DECL_VIEWFACTORY(PresViewShell);
    SFX_DECL_INTERFACE( SD_IF_SDPRESVIEWSHELL )

    PresentationViewShell( SfxViewFrame* pFrame, ViewShellBase& rViewShellBase, ::Window* pParentWindow, FrameView* pFrameView = NULL);
    virtual ~PresentationViewShell (void);

    /** This method is used by a simple class that passes some
        arguments from the creator of the new view shell to the new view
        shell object by waiting for its asynchronous creation.
        @param pFrameView
            The frame view that is typically used by the creating object and
            that shall be shared by the created view shell.
    */
    void FinishInitialization( FrameView* pFrameView );

    virtual void Resize (void);

protected:
    virtual SvxRuler* CreateHRuler(::sd::Window* pWin, bool bIsFirst);
    virtual SvxRuler* CreateVRuler(::sd::Window* pWin);

private:
    Rectangle       maOldVisArea;

    virtual void Activate (bool bIsMDIActivate);
    virtual void Paint (const Rectangle& rRect, ::sd::Window* pWin);
};

} // end of namespace sd

#endif
