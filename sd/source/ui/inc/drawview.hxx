/*************************************************************************
 *
 *  $RCSfile: drawview.hxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:48:38 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef _SD_DRAWVIEW_HXX
#define _SD_DRAWVIEW_HXX


//#ifndef _SD_SDVIEW_HXX
#include "sdview.hxx"
//#endif

class SdDrawDocShell;
class SdDrawViewShell;
class FuSlideShow;

#define SDDRAWVIEW_MAGIC  0x456789BA

/*************************************************************************
|*
|* Ableitung von SdView; enthaelt auch einen Zeiger auf das Dokument
|*
\************************************************************************/

class SdDrawView : public SdView
{
    SdDrawDocShell*     pDocShell;
    SdDrawViewShell*    pDrawViewShell;
    VirtualDevice*      pVDev;

    USHORT              nPOCHSmph;  // zum blockieren des PageOrderChangedHint
    USHORT              nPresPaintSmph;     // zum Blockieren des Zeichnens
                                            // in der Diashow
    BOOL                bPixelMode;
    FuSlideShow*        pSlideShow;
    BOOL                bInAnimation;
    ULONG               nMagic;

    BOOL                bActionMode;

 protected:
    virtual void ModelHasChanged();

 public:
    TYPEINFO();

     SdDrawView(SdDrawDocShell* pDocSh, OutputDevice* pOutDev,
                SdDrawViewShell* pShell);
    ~SdDrawView();

    virtual void MarkListHasChanged();
    void InitRedraw(OutputDevice* pOutDev, const Region& rReg);

    virtual BOOL SetAttributes(const SfxItemSet& rSet, BOOL bReplaceAll = FALSE);

    virtual void SFX_NOTIFY(SfxBroadcaster& rBC, const TypeId& rBCType,
                            const SfxHint& rHint, const TypeId& rHintType);

    void    BlockPageOrderChangedHint(BOOL bBlock);

    BOOL    SetStyleSheet(SfxStyleSheet* pStyleSheet, BOOL bDontRemoveHardAttr = FALSE);
    virtual BOOL IsObjMarkable(SdrObject* pObj, SdrPageView* pPV) const;

    void    AllowPresPaint(BOOL bAllowed);
    BOOL    IsPresPaintAllowed() const { return nPresPaintSmph == 0; }
    void    SetPixelMode(BOOL bOn);
    BOOL    IsPixelMode() const { return bPixelMode; }
    void    SetActionMode(BOOL bOn) { bActionMode = bOn; }
    BOOL    IsActionMode() const { return bActionMode; }

    virtual void MakeVisible(const Rectangle&, Window& rWin);
    virtual void HidePage(SdrPageView* pPV);

    void    PresPaint(const Region& rRegion);
    DECL_LINK(PaintProc, SdrPaintProcRec*);

    void    SetAnimationMode(BOOL bStart);
    void    HideAndAnimateObject(SdrObject* pObj);
    void    AnimatePage();
    BOOL    IsInAnimation() { return bInAnimation; }
    FuSlideShow* GetSlideShow() { return pSlideShow; }

    virtual SdrObject* GetMaxToBtmObj(SdrObject* pObj) const;
};



#endif      // _SD_DRAWVIEW_HXX
