/*************************************************************************
 *
 *  $RCSfile: svdscrol.hxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 17:01:25 $
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

#ifndef _SVDSCROL_HXX
#define _SVDSCROL_HXX

#ifndef _VIRDEV_HXX //autogen
#include <vcl/virdev.hxx>
#endif

#ifndef _TIMER_HXX //autogen
#include <vcl/timer.hxx>
#endif

#ifndef _SVDATTR_HXX
#include "svdattr.hxx"
#endif

//************************************************************
//   Vorausdeklarationen
//************************************************************

class SfxItemSet;
class ImpSdrMtfAnimator;
class SdrPageView;

//************************************************************
//   ImpMtfAnimationInfo
//************************************************************

class ImpMtfAnimationInfo
{
public:
    VirtualDevice               aBackground; // Der gesicherte Hintergrund
    VirtualDevice               aVirDev;     // VDev, um Flackern bei der Ausgabe zu vermeiden
    Point                       aOffset;     // Der PageView-Offset
    OutputDevice*               pOutDev;     // Das OutDev in dem letztenendes ausgegeben wird
    const SdrPageView*          pPageView; // fuer ShownXor
    long                        nExtraData;  // Userdata fuer den Aufrufer
    long                        nLoopNum;    // Nummer des Durchlaufs
    long                        nLoopStep;   // Schrittnummer
    FASTBOOL                    bPause;      // Pause, weil Obj. z.B. selektiert ist
    FASTBOOL                    bBackSaved;  // FALSE=Neuer Entry, Hintergrund ist noch zu sichern,...
    FASTBOOL                    bShown;      // Schon hingemalt?
    FASTBOOL                    bReady;      // Fertig animiert?
    FASTBOOL                    bBackTrack;  // Ruekweg bei Alternate
    FASTBOOL                    bNeu;        // Neuer Scrollbeginn?

    ImpMtfAnimationInfo()
    :   pOutDev(NULL),
        pPageView(NULL),
        nExtraData(0),
        nLoopNum(0),
        nLoopStep(0),
        bPause(FALSE),
        bBackSaved(FALSE),
        bShown(FALSE),
        bReady(FALSE),
        bBackTrack(FALSE),
        bNeu(FALSE)
    {}

    void Restart();
    void SaveBackground(const ImpSdrMtfAnimator& rAnimator, const Region* pClip=NULL);
    void Paint(const ImpSdrMtfAnimator& rAnimator, OutputDevice& rOut) const;
    void AnimateOneStep(ImpSdrMtfAnimator& rAnimator);
};

//************************************************************
//   ImpSdrMtfAnimator
//************************************************************

class ImpSdrMtfAnimator
{
    friend class                ImpMtfAnimationInfo;

    Brush                       aBackBrush;
    AutoTimer                   aTimer;
    Container                   aInfoList;
    GDIMetaFile*                pMtf;
    Link                        aNotifyLink;
    Rectangle                   aScrollFrameRect;
    Rectangle                   aMtfBoundRect;
    Rectangle                   aOutputRect;
    Point                       a__RotateRef;
    Region                      aClipRegion;
    long                        nRotateAngle;
    double                      nSin;
    double                      nCos;
    FASTBOOL                    bClipRegion;

    // Attribute
    SdrTextAniKind              eAniKind;
    SdrTextAniDirection         eDirection;
    FASTBOOL                    bStartInside;
    FASTBOOL                    bStopInside;
    USHORT                      nMaxCount;
    USHORT                      nDelay;
    short                       nAmount;

private:
    void ImpInsertInfo(ImpMtfAnimationInfo* pE) { aInfoList.Insert(pE,CONTAINER_APPEND); }
    void ImpClearInfoList();
    DECL_LINK(ImpTimerHdl,AutoTimer*);

public:
    ULONG GetInfoCount() const { return aInfoList.Count(); }
    const ImpMtfAnimationInfo* GetInfo(ULONG nPos) const { return (ImpMtfAnimationInfo*)aInfoList.GetObject(nPos); }
    ImpMtfAnimationInfo* GetInfo(ULONG nPos) { return (ImpMtfAnimationInfo*)aInfoList.GetObject(nPos); }
    ULONG FindInfo(const OutputDevice& rOut, const Point& rOffset, long nExtraData) const;
    void  RemoveInfo(ULONG nNum) { delete (ImpMtfAnimationInfo*)aInfoList.Remove(nNum); }

    ImpSdrMtfAnimator();
    ~ImpSdrMtfAnimator();
    void SetAnimationNotifyHdl(const Link& rLink) { aNotifyLink=rLink; }
    const Link& GetAnimationNotifyHdl() const { return aNotifyLink; }
    void SetGDIMetaFile(GDIMetaFile* pMetaFile);
    const GDIMetaFile* GetGDIMetaFile() const { return pMtf; }
    ImpMtfAnimationInfo* Start(OutputDevice& rOutDev, const Point& rOffset, long nExtraData=0);
    void Stop();
    void Stop(OutputDevice& rOutDev);
    void Stop(OutputDevice& rOutDev, const Point& rOffset);
    void SetAttributes(const SfxItemSet& rSet);
    void SetOutputRect(const Rectangle& rRect) { aOutputRect=rRect; }
    void SetScrollFrameRect(const Rectangle& rRect) { aScrollFrameRect=rRect; }
    void SetMtfFrameRect(const Rectangle& rRect) { aMtfBoundRect=rRect; }
    void SetRotateRef(const Point& rPt) { a__RotateRef=rPt; }
    void SetRotateAngle(long nWink);
    void SetClipRegion() { bClipRegion=FALSE; }
    void SetClipRegion(const Region& rReg) { aClipRegion=rReg; bClipRegion=TRUE; }
};

////////////////////////////////////////////////////////////////////////////////////////////////////

#endif //_SVDSCROL_HXX

