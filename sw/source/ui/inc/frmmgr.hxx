/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: frmmgr.hxx,v $
 *
 *  $Revision: 1.11 $
 *
 *  last change: $Author: hr $ $Date: 2007-09-27 12:00:10 $
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
#ifndef _FRMMGR_HXX
#define _FRMMGR_HXX

#ifndef _SWTYPES_HXX
#include "swtypes.hxx"
#endif
#ifndef _FRMATR_HXX
#include "frmatr.hxx"
#endif

#ifndef _SVX_SVXENUM_HXX //autogen
#include <svx/svxenum.hxx>
#endif
#ifndef _GEN_HXX //autogen
#include <tools/gen.hxx>
#endif
#ifndef _FMTFSIZE_HXX //autogen
#include <fmtfsize.hxx>
#endif
#ifndef _FMTSRND_HXX //autogen
#include <fmtsrnd.hxx>
#endif
#ifndef _FMTORNT_HXX //autogen
#include <fmtornt.hxx>
#endif
#ifndef _FMTANCHR_HXX //autogen
#include <fmtanchr.hxx>
#endif

#ifndef INCLUDED_SWDLLAPI_H
#include "swdllapi.h"
#endif

class   SwWrtShell;
struct  SvxSwFrameValidation;
struct  SwPosition;

class   SwFmt;
class   SwFmtCol;

const SwTwips   DFLT_WIDTH      = MM50 * 4;
const SwTwips   DFLT_HEIGHT     = MM50;

#define FULL_ATTRSET    0xffff

#define FRMMGR_TYPE_NONE    0x00
#define FRMMGR_TYPE_TEXT    0x01
#define FRMMGR_TYPE_GRF     0x02
#define FRMMGR_TYPE_OLE     0x04
#define FRMMGR_TYPE_LABEL   0x08
#define FRMMGR_TYPE_ENVELP  0x10

class SW_DLLPUBLIC SwFlyFrmAttrMgr
{
    SfxItemSet  aSet;
    Point       aAbsPos;
    SwWrtShell  *pOwnSh;

    BOOL        bAbsPos,
                bNewFrm;
    BOOL        bIsInVertical;

    // interne Verrechnung fuer Umrandung
    SW_DLLPRIVATE SwTwips           CalcTopSpace();
    SW_DLLPRIVATE SwTwips           CalcBottomSpace();
    SW_DLLPRIVATE SwTwips           CalcLeftSpace();
    SW_DLLPRIVATE SwTwips           CalcRightSpace();

    SW_DLLPRIVATE void _UpdateFlyFrm(); //Nacharbeit nach Einfuegen oder Update

public:
    SwFlyFrmAttrMgr( BOOL bNew, SwWrtShell* pSh, BYTE nType );

    //CopyCtor fuer die Dialoge, zum Pruefen der Metrics
    SwFlyFrmAttrMgr( BOOL bNew, SwWrtShell *pSh, const SfxItemSet &rSet );

    inline SwWrtShell*  GetShell() { return pOwnSh; }

    void                SetAnchor(RndStdIds eId);
    inline RndStdIds    GetAnchor()  const;

    void                SetHorzOrientation(sal_Int16 eOrient);
    void                SetVertOrientation(sal_Int16 eOrient);

    // Absolute Position
    void                SetAbsPos(const Point& rLPoint);

    // Relative Position vom Anker
    void                SetPos(const Point& rLPoint);
    inline Point        GetPos() const;

    // Groesse
    void                SetSize(const Size& rLSize);
    inline const Size&  GetSize() const;

    inline USHORT       GetHeightPercent() const;

    void                SetHeightSizeType(SwFrmSize eType);

    // Abstand zum Inhalt
    void                SetLRSpace( long nLeft  = LONG_MAX,
                                    long nRight = LONG_MAX );
    void                SetULSpace( long nTop   = LONG_MAX,
                                    long nBottom= LONG_MAX );

    void                SetCol( const SwFmtCol &rCol);

    // Attribute aendern und erfragen
    void                UpdateAttrMgr();
    void                UpdateFlyFrm();

    // neuen Rahmen erzeugen
    BOOL                InsertFlyFrm();
    void                InsertFlyFrm(RndStdIds      eAnchorType,
                                   const Point    &rPos,
                                   const Size     &rSize,
                                   BOOL           bAbsPos = FALSE);

    // Metriken pruefen und  aendern
    void                ValidateMetrics(SvxSwFrameValidation& rVal,
                            const SwPosition* pToCharCntntPos,
                            BOOL bOnlyPercentRefValue = FALSE);

    void                DelAttr(USHORT nId);

    // Set rausreichen
    inline const SfxItemSet &GetAttrSet() const { return aSet; }
    inline       SfxItemSet &GetAttrSet()       { return aSet; }
    void                     SetAttrSet(const SfxItemSet& rSet);

    inline const SwFmtVertOrient &GetVertOrient() const;
    inline const SwFmtHoriOrient &GetHoriOrient() const;
    inline const SvxShadowItem   &GetShadow() const;
    inline const SvxBoxItem      &GetBox() const;
    inline const SwFmtSurround   &GetSurround() const;
    inline const SwFmtFrmSize    &GetFrmSize() const;

    long CalcWidthBorder()  { return CalcLeftSpace()+CalcRightSpace(); }
    long CalcHeightBorder() { return CalcTopSpace()+CalcBottomSpace(); }
};

inline const Size& SwFlyFrmAttrMgr::GetSize() const
{
    return ((SwFmtFrmSize&)aSet.Get(RES_FRM_SIZE)).GetSize();
}

inline const SwFmtVertOrient &SwFlyFrmAttrMgr::GetVertOrient() const
{
    return ((SwFmtVertOrient&)aSet.Get(RES_VERT_ORIENT));
}
inline const SwFmtHoriOrient &SwFlyFrmAttrMgr::GetHoriOrient() const
{
    return ((SwFmtHoriOrient &)aSet.Get(RES_HORI_ORIENT));
}
inline const SwFmtFrmSize& SwFlyFrmAttrMgr::GetFrmSize() const
{
    return ((SwFmtFrmSize&)aSet.Get(RES_FRM_SIZE));
}
inline const SvxShadowItem &SwFlyFrmAttrMgr::GetShadow() const
{
    return ((SvxShadowItem&)aSet.Get(RES_SHADOW));
}
inline const SvxBoxItem &SwFlyFrmAttrMgr::GetBox() const
{
    return ((SvxBoxItem&)aSet.Get(RES_BOX));
}
inline const SwFmtSurround &SwFlyFrmAttrMgr::GetSurround() const
{
    return ((SwFmtSurround&)aSet.Get(RES_SURROUND));
}

inline Point SwFlyFrmAttrMgr::GetPos() const
{
    return Point( GetHoriOrient().GetPos(), GetVertOrient().GetPos() );
}

inline RndStdIds SwFlyFrmAttrMgr::GetAnchor()  const
{
    return ((SwFmtAnchor&)aSet.Get(RES_ANCHOR)).GetAnchorId();
}

inline USHORT SwFlyFrmAttrMgr::GetHeightPercent() const
{
    return GetFrmSize().GetHeightPercent();
}

#endif
