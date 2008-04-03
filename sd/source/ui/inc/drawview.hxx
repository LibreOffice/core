/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: drawview.hxx,v $
 *
 *  $Revision: 1.13 $
 *
 *  last change: $Author: kz $ $Date: 2008-04-03 14:01:00 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#ifndef SD_DRAW_VIEW_HXX
#define SD_DRAW_VIEW_HXX

#ifndef SD_VIEW_HXX
#include "View.hxx"
#endif


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
    void CompleteRedraw(OutputDevice* pOutDev, const Region& rReg, USHORT nPaintMode = 0, ::sdr::contact::ViewObjectContactRedirector* pRedirector = 0L);

    virtual BOOL SetAttributes(const SfxItemSet& rSet, BOOL bReplaceAll = FALSE);

    virtual void SFX_NOTIFY(SfxBroadcaster& rBC, const TypeId& rBCType,
                            const SfxHint& rHint, const TypeId& rHintType);

    void    BlockPageOrderChangedHint(BOOL bBlock);

    BOOL    SetStyleSheet(SfxStyleSheet* pStyleSheet, BOOL bDontRemoveHardAttr = FALSE);
    virtual BOOL IsObjMarkable(SdrObject* pObj, SdrPageView* pPV) const;

    virtual void MakeVisible(const Rectangle& rRect, ::Window& rWin);
    virtual void HideSdrPage(); // SdrPageView* pPV);

    void    PresPaint(const Region& rRegion);

    virtual SdrObject* GetMaxToBtmObj(SdrObject* pObj) const;

    virtual void DeleteMarked(); // from SdrView
protected:
    virtual void ModelHasChanged();

private:
    friend class DrawViewRedirector;

    DrawDocShell*   mpDocShell;
    DrawViewShell*  mpDrawViewShell;
    VirtualDevice*  mpVDev;

    USHORT          mnPOCHSmph; // zum blockieren des PageOrderChangedHint
};

} // end of namespace sd

#endif
