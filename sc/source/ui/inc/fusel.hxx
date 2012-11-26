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



#ifndef SC_FUSEL_HXX
#define SC_FUSEL_HXX

#ifndef _SV_HXX
#endif

#include "fudraw.hxx"

//class Outliner;
//class OutlinerView;
class SdrPageView;


/*************************************************************************
|*
|* Basisklasse fuer alle Funktionen
|*
\************************************************************************/

class FuSelection : public FuDraw
{
public:
    FuSelection(ScTabViewShell* pViewSh, Window* pWin, ScDrawView* pView,
           SdrModel* pDoc, SfxRequest& rReq );

    virtual ~FuSelection();
                                       // Mouse- & Key-Events
    virtual sal_Bool KeyInput(const KeyEvent& rKEvt);
    virtual sal_Bool MouseMove(const MouseEvent& rMEvt);
    virtual sal_Bool MouseButtonUp(const MouseEvent& rMEvt);
    virtual sal_Bool MouseButtonDown(const MouseEvent& rMEvt);
    virtual sal_uInt8 Command(const CommandEvent& rCEvt);

    virtual void Activate();           // Function aktivieren
    virtual void Deactivate();         // Function deaktivieren

    void    ActivateNoteHandles(SdrObject* pObj);

protected:
//  Outliner*       pOutliner;
//  OutlinerView*   pOutlinerView;
    sal_Bool            bVCAction;

private:
    bool TestDetective( SdrPageView* pPV, const basegfx::B2DPoint& rPos );  // -> fusel2

    bool                IsNoteCaptionMarked() const;
    bool                IsNoteCaptionClicked( const basegfx::B2DPoint& rPos ) const;
};



#endif      // _SD_FUSEL_HXX

