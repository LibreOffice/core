/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: textanim.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: obo $ $Date: 2006-10-12 12:29:26 $
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
#ifndef _SVX_TEXTANIM_HXX
#define _SVX_TEXTANIM_HXX

// include ---------------------------------------------------------------

#ifndef _SV_FIELD_HXX
#include <vcl/field.hxx>
#endif

#ifndef _SFXTABDLG_HXX
#include <sfx2/tabdlg.hxx>
#endif

#ifndef _SVDATTR_HXX
#include "svdattr.hxx"
#endif

#ifndef _SV_FIXED_HXX
#include <vcl/fixed.hxx>
#endif

#ifndef _SV_LSTBOX_HXX //autogen
#include <vcl/lstbox.hxx>
#endif

#ifndef _SV_GROUP_HXX //autogen
#include <vcl/group.hxx>
#endif

class SdrView;

/*************************************************************************
|*
|* Page zum "Andern von TextAnimationen (Lauftext etc.)
|*
\************************************************************************/

class SvxTextAnimationPage : public SfxTabPage
{
private:
    FixedLine           aFlEffect;
    FixedText           aFtEffects;
    ListBox             aLbEffect;
    //ValueSet          aCtlEffect;
    FixedText           aFtDirection;
    ImageButton         aBtnUp;
    ImageButton         aBtnLeft;
    ImageButton         aBtnRight;
    ImageButton         aBtnDown;

    FixedLine           aFlProperties;
    TriStateBox         aTsbStartInside;
    TriStateBox         aTsbStopInside;

    FixedText           aFtCount;
    TriStateBox         aTsbEndless;
    NumericField        aNumFldCount;

    FixedText           aFtAmount;
    TriStateBox         aTsbPixel;
    MetricField         aMtrFldAmount;

    FixedText           aFtDelay;
    TriStateBox         aTsbAuto;
    MetricField         aMtrFldDelay;

    const SfxItemSet&   rOutAttrs;
    SdrTextAniKind      eAniKind;
    FieldUnit           eFUnit;
    SfxMapUnit          eUnit;

#ifdef _SVX_TEXTANIM_CXX
    DECL_LINK( SelectEffectHdl_Impl, void * );
    DECL_LINK( ClickEndlessHdl_Impl, void * );
    DECL_LINK( ClickAutoHdl_Impl, void * );
    DECL_LINK( ClickPixelHdl_Impl, void * );
    DECL_LINK( ClickDirectionHdl_Impl, ImageButton * );

    void                SelectDirection( SdrTextAniDirection nValue );
    USHORT              GetSelectedDirection();
#endif

public:

    SvxTextAnimationPage( Window* pWindow, const SfxItemSet& rInAttrs );
    ~SvxTextAnimationPage();

    static SfxTabPage*  Create( Window*, const SfxItemSet& );
    static  USHORT*     GetRanges();

    virtual BOOL        FillItemSet( SfxItemSet& );
    virtual void        Reset( const SfxItemSet & );

    void         Construct();
};

/*************************************************************************
|*
|* Text-Tab-Dialog
|*
\************************************************************************/
class SvxTextTabDialog : public SfxTabDialog
{
private:
    const SfxItemSet&   rOutAttrs;
    const SdrView*      pView;

    virtual void        PageCreated( USHORT nId, SfxTabPage &rPage );

public:

            SvxTextTabDialog( Window* pParent, const SfxItemSet* pAttr,
                                const SdrView* pView );
            ~SvxTextTabDialog() {};
};


#endif // _SVX_TEXTANIM_HXX

