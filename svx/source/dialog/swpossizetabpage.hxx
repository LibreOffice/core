/*************************************************************************
 *
 *  $RCSfile: swpossizetabpage.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: hjs $ $Date: 2004-06-28 14:06:16 $
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
#ifndef _SVX_SWPOSSIZETABPAGE_HXX
#define _SVX_SWPOSSIZETABPAGE_HXX

#ifndef _SFXTABDLG_HXX
#include <sfx2/tabdlg.hxx>
#endif
#ifndef _FIXED_HXX //autogen
#include <vcl/fixed.hxx>
#endif
#ifndef _SV_FIELD_HXX
#include <vcl/field.hxx>
#endif
#ifndef _SV_LSTBOX_HXX
#include <vcl/lstbox.hxx>
#endif
#ifndef _SVXSWFRAMEPOSSTRINGS_HXX
#include <swframeposstrings.hxx>
#endif
#ifndef _SVXSWFRAMEEXAMPLE_HXX
#include <swframeexample.hxx>
#endif

/*-- 01.03.2004 15:45:01---------------------------------------------------
  SvxSwPosSizeTabPage - position and size page for Writer drawing objects
  -----------------------------------------------------------------------*/
struct FrmMap;
class SdrView;
class SvxSwPosSizeTabPage : public SfxTabPage
{
    FixedLine   m_aSizeFL;
    FixedText   m_aWidthFT;
    MetricField m_aWidthMF;
    FixedText   m_aHeightFT;
    MetricField m_aHeightMF;
    CheckBox    m_aKeepRatioCB;

    FixedLine   m_aSeparatorFL;

    FixedLine   m_aAnchorFL;
    RadioButton m_aToPageRB;
    RadioButton m_aToParaRB;
    RadioButton m_aToCharRB;
    RadioButton m_aAsCharRB;
    RadioButton m_aToFrameRB;

    FixedLine   m_aProtectionFL;
    TriStateBox m_aPositionCB;
    TriStateBox m_aSizeCB;

    FixedLine   m_aPositionFL;
    FixedText   m_aHoriFT;
    ListBox     m_aHoriLB;
    FixedText   m_aHoriByFT;
    MetricField m_aHoriByMF;
    FixedText   m_aHoriToFT;
    ListBox     m_aHoriToLB;

    CheckBox    m_aHoriMirrorCB;

    FixedText   m_aVertFT;
    ListBox     m_aVertLB;
    FixedText   m_aVertByFT;
    MetricField m_aVertByMF;
    FixedText   m_aVertToFT;
    ListBox     m_aVertToLB;

    CheckBox    m_aFollowCB;

    SvxSwFrameExample m_aExampleWN;

    Link        m_aValidateLink;

    //'string provider'
    SvxSwFramePosString m_aFramePosString;

    Rectangle           m_aRect; //size of all selected objects
    Rectangle           m_aWorkArea;
    Point               m_aAnchorPos;

    FrmMap* m_pVMap;
    FrmMap* m_pHMap;
    const SdrView*      m_pSdrView;

    // initial values
    short   m_nOldH;
    short   m_nOldHRel;
    short   m_nOldV;
    short   m_nOldVRel;

    double  m_fWidthHeightRatio; //width-to-height ratio to support the KeepRatio button
    USHORT  m_nHtmlMode;
    bool    m_bHtmlMode;
    bool    m_bAtHoriPosModified;
    bool    m_bAtVertPosModified;
    bool    m_bIsVerticalFrame;
    bool    m_bPositioningDisabled;
    bool    m_bIsMultiSelection;
    bool    m_bIsInRightToLeft;



    DECL_LINK( RangeModifyHdl, Edit * );
    DECL_LINK( AnchorTypeHdl, RadioButton * );
    DECL_LINK( PosHdl, ListBox * );
    DECL_LINK( RelHdl, ListBox * );
    DECL_LINK( MirrorHdl, CheckBox * );
    DECL_LINK( ModifyHdl, Edit * );
    DECL_LINK( ProtectHdl, TriStateBox *);

    void            InitPos(short nAnchorType, USHORT nH, USHORT nHRel,
                            USHORT nV,  USHORT nVRel,
                            long   nX,  long   nY);
    USHORT          GetMapPos(FrmMap *pMap, ListBox &rAlignLB);
    short           GetAlignment(FrmMap *pMap, USHORT nMapPos, ListBox &rAlignLB, ListBox &rRelationLB);
    short           GetRelation(FrmMap *pMap, ListBox &rRelationLB);
    short           GetAnchorType(bool* pbHasChanged = 0);
    ULONG           FillRelLB(FrmMap *pMap, USHORT nLBSelPos, USHORT nAlign, USHORT nRel, ListBox &rLB, FixedText &rFT);
    USHORT          FillPosLB(FrmMap *pMap, USHORT nAlign, const USHORT _nRel, ListBox &rLB);

    void            UpdateExample();

public:
    SvxSwPosSizeTabPage( Window* pParent, const SfxItemSet& rInAttrs  );
    ~SvxSwPosSizeTabPage();

    static SfxTabPage* Create( Window*, const SfxItemSet& );
    static USHORT*     GetRanges();

    virtual BOOL FillItemSet( SfxItemSet& );
    virtual void Reset( const SfxItemSet & );

    virtual void ActivatePage( const SfxItemSet& rSet );
    virtual int  DeactivatePage( SfxItemSet* pSet );

    void    EnableAnchorTypes(USHORT nAnchorEnable);

    void SetValidateFramePosLink( const Link& rLink )
            {m_aValidateLink = rLink;}

    void SetView( const SdrView* pSdrView );
};

#endif
