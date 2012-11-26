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



#ifndef SD_FU_CONSTRUCT_HXX
#define SD_FU_CONSTRUCT_HXX

#include "fudraw.hxx"

class KeyEvent;
class SdrObject;
class SfxItemSet;

namespace sd {

/*************************************************************************
|*
|* Rechteck zeichnen
|*
\************************************************************************/

class FuConstruct
    : public FuDraw
{
public:
    static const int MIN_FREEHAND_DISTANCE = 10;

    virtual void DoExecute( SfxRequest& rReq );

    // Mouse- & Key-Events
    virtual bool KeyInput(const KeyEvent& rKEvt);
    virtual bool MouseMove(const MouseEvent& rMEvt);
    virtual bool MouseButtonUp(const MouseEvent& rMEvt);
    virtual bool MouseButtonDown(const MouseEvent& rMEvt);

    virtual void Activate();           // Function aktivieren
    virtual void Deactivate();         // Function deaktivieren

    virtual void SelectionHasChanged() { bSelectionChanged = true; }

    // SJ: setting stylesheet, the use of a filled or unfilled style
    // is determined by the member nSlotId :
    void SetStyleSheet(SfxItemSet& rAttr, SdrObject* pObj);

    // SJ: setting stylesheet, the use of a filled or unfilled style
    // is determinded by the parameters bUseFillStyle and bUseNoFillStyle :
    void SetStyleSheet( SfxItemSet& rAttr, SdrObject* pObj,
            const sal_Bool bUseFillStyle, const sal_Bool bUseNoFillStyle );

protected:
    FuConstruct (ViewShell* pViewSh,
        ::sd::Window* pWin,
        ::sd::View* pView,
        SdDrawDocument* pDoc,
        SfxRequest& rReq);

    bool bSelectionChanged;
};

} // end of namespace sd

#endif      // _SD_FUCONSTR_HXX
