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
    DrawView (
        DrawDocShell* pDocSh,
        OutputDevice* pOutDev,
        DrawViewShell* pShell);
    virtual ~DrawView (void);

    virtual void handleSelectionChange();
    void CompleteRedraw(OutputDevice* pOutDev, const Region& rReg, sdr::contact::ViewObjectContactRedirector* pRedirector = 0L);

    virtual bool SetAttributes(const SfxItemSet& rSet, bool bReplaceAll = false);

    virtual void Notify(SfxBroadcaster& rBC, const SfxHint& rHint);

    void    BlockPageOrderChangedHint(bool bBlock);

    bool    SetStyleSheet(SfxStyleSheet* pStyleSheet, bool bDontRemoveHardAttr = false);
    virtual bool IsObjMarkable(const SdrObject& rObj) const;

    virtual void MakeVisibleAtView(const basegfx::B2DRange& rRange, ::Window& rWin);
    virtual void HideSdrPage();

    void    PresPaint(const Region& rRegion);

    virtual void DeleteMarked(); // from SdrView

protected:
    virtual void LazyReactOnObjectChanges();

private:
    friend class DrawViewRedirector;

    DrawDocShell*   mpDocShell;
    DrawViewShell*  mpDrawViewShell;
    VirtualDevice*  mpVDev;

    sal_uInt16          mnPOCHSmph; // zum blockieren des PageOrderChangedHint
};

} // end of namespace sd

#endif
