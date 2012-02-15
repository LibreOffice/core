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



#ifndef SD_DRAW_VIEW_HXX
#define SD_DRAW_VIEW_HXX

#include "View.hxx"


namespace sd {

class DrawDocShell;
class DrawViewShell;
class FuSlideShow;
class SlideShow;

#define SDDRAWVIEW_MAGIC  0x456789BA

/*************************************************************************
|*
|* Ableitung von ::sd::View; enthaelt auch einen Zeiger auf das Dokument
|*
\************************************************************************/

class DrawView : public ::sd::View
{
public:
    TYPEINFO();

    DrawView (
        DrawDocShell* pDocSh,
        OutputDevice* pOutDev,
        DrawViewShell* pShell);
    virtual ~DrawView (void);

    virtual void MarkListHasChanged();
    void CompleteRedraw(OutputDevice* pOutDev, const Region& rReg, sdr::contact::ViewObjectContactRedirector* pRedirector = 0L);

    virtual sal_Bool SetAttributes(const SfxItemSet& rSet, sal_Bool bReplaceAll = sal_False);

    virtual void Notify(SfxBroadcaster& rBC, const SfxHint& rHint);

    void    BlockPageOrderChangedHint(sal_Bool bBlock);

    sal_Bool    SetStyleSheet(SfxStyleSheet* pStyleSheet, sal_Bool bDontRemoveHardAttr = sal_False);
    virtual sal_Bool IsObjMarkable(SdrObject* pObj, SdrPageView* pPV) const;

    virtual void MakeVisible(const Rectangle& rRect, ::Window& rWin);
    virtual void HideSdrPage(); // SdrPageView* pPV);

    void    PresPaint(const Region& rRegion);

    virtual void DeleteMarked(); // from SdrView
protected:
    virtual void ModelHasChanged();

private:
    friend class DrawViewRedirector;

    DrawDocShell*   mpDocShell;
    DrawViewShell*  mpDrawViewShell;
    VirtualDevice*  mpVDev;

    sal_uInt16          mnPOCHSmph; // zum blockieren des PageOrderChangedHint
};

} // end of namespace sd

#endif
