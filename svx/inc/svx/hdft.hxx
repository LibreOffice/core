/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: hdft.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2007-04-11 15:57:01 $
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
#ifndef _SVX_HDFT_HXX
#define _SVX_HDFT_HXX

// include ---------------------------------------------------------------

#ifndef _SFXTABDLG_HXX //autogen
#include <sfx2/tabdlg.hxx>
#endif

#ifndef _FIXED_HXX //autogen
#include <vcl/fixed.hxx>
#endif

#ifndef _FIELD_HXX //autogen
#include <vcl/field.hxx>
#endif

#ifndef _GROUP_HXX //autogen
#include <vcl/group.hxx>
#endif

#ifndef _SVX_PAGECTRL_HXX //autogen
#include <svx/pagectrl.hxx>
#endif

#ifndef INCLUDED_SVXDLLAPI_H
#include "svx/svxdllapi.h"
#endif

// class  SvxHFPage ------------------------------------------------------

class SVX_DLLPUBLIC SvxHFPage: public SfxTabPage
{
    using TabPage::ActivatePage;
    using TabPage::DeactivatePage;

public:

    virtual BOOL    FillItemSet( SfxItemSet& rOutSet );
    virtual void    Reset( const SfxItemSet& rSet );

    virtual         ~SvxHFPage();

    void            DisableDeleteQueryBox() { bDisableQueryBox = TRUE; }
    void            EnableBackgroundSelector( BOOL bNew ) { bEnableBackgroundSelector = bNew; }
    void            EnableDynamicSpacing();

protected:
    virtual void    ActivatePage( const SfxItemSet& rSet );
    virtual int     DeactivatePage( SfxItemSet* pSet = 0 );

    SvxHFPage( Window* pParent, USHORT nResId, const SfxItemSet& rSet,
               USHORT nSetId );

    CheckBox        aTurnOnBox;
    CheckBox        aCntSharedBox;
    FixedText       aLMLbl;
    MetricField     aLMEdit;
    FixedText       aRMLbl;
    MetricField     aRMEdit;
    FixedText       aDistFT;
    MetricField     aDistEdit;
    CheckBox        aDynSpacingCB;
    FixedText       aHeightFT;
    MetricField     aHeightEdit;
    CheckBox        aHeightDynBtn;
    FixedLine       aFrm;
    SvxPageWindow   aBspWin;
    PushButton      aBackgroundBtn;

    USHORT          nId;
    SfxItemSet*     pBBSet;
    BOOL            bDisableQueryBox;
    BOOL            bEnableBackgroundSelector;
    FASTBOOL        bInReset;

    void            InitHandler();
    DECL_LINK( TurnOnHdl, CheckBox*);
    DECL_LINK( DistModify, MetricField*);
    DECL_LINK( HeightModify, MetricField*);
    DECL_LINK( BorderModify, MetricField*);
    DECL_LINK( BackgroundHdl, Button*);

    void            UpdateExample();
    DECL_LINK( RangeHdl, Edit*);

private:
#ifdef _SVX_HDFT_CXX
    SVX_DLLPRIVATE void         ResetBackground_Impl( const SfxItemSet& rSet );
#endif
};

// class SvxHeaderPage ---------------------------------------------------

class SVX_DLLPUBLIC SvxHeaderPage : public SvxHFPage
{
public:
    static SfxTabPage*  Create( Window* pParent, const SfxItemSet& rSet );
    static USHORT*      GetRanges();

private:
    SvxHeaderPage( Window* pParent, const SfxItemSet& rSet );
};

// class SvxFooterPage ---------------------------------------------------

class SVX_DLLPUBLIC SvxFooterPage : public SvxHFPage
{
public:
    static SfxTabPage*  Create( Window* pParent, const SfxItemSet& rSet );
    static USHORT*      GetRanges();

private:
    SVX_DLLPRIVATE SvxFooterPage(   Window* pParent, const SfxItemSet& rSet );
};

#endif

