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


#ifndef _SVX_HDFT_HXX
#define _SVX_HDFT_HXX

// include ---------------------------------------------------------------

#include <sfx2/tabdlg.hxx>

#ifndef _FIXED_HXX //autogen
#include <vcl/fixed.hxx>
#endif

#ifndef _FIELD_HXX //autogen
#include <vcl/field.hxx>
#endif

#ifndef _GROUP_HXX //autogen
#include <vcl/group.hxx>
#endif
#include <svx/pagectrl.hxx>
#include "svx/svxdllapi.h"

// class  SvxHFPage ------------------------------------------------------

class SVX_DLLPUBLIC SvxHFPage : public SfxTabPage
{
    using TabPage::ActivatePage;
    using TabPage::DeactivatePage;

private:
    //UUUU
    void EnableDrawingLayerFillStyles(bool bNew) { mbEnableDrawingLayerFillStyles = bNew; }

public:

    virtual sal_Bool    FillItemSet( SfxItemSet& rOutSet );
    virtual void    Reset( const SfxItemSet& rSet );

    virtual         ~SvxHFPage();

    void DisableDeleteQueryBox() { mbDisableQueryBox = true; }
    void EnableBackgroundSelector(bool bNew) { mbEnableBackgroundSelector = bNew; }

    //UUUU
    virtual void PageCreated(SfxAllItemSet aSet);

    void            EnableDynamicSpacing();

protected:
    virtual void    ActivatePage( const SfxItemSet& rSet );
    virtual int     DeactivatePage( SfxItemSet* pSet = 0 );

    SvxHFPage( Window* pParent, sal_uInt16 nResId, const SfxItemSet& rSet,
               sal_uInt16 nSetId );

    FixedLine       aFrm;
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
    SvxPageWindow   aBspWin;
    PushButton      aBackgroundBtn;

    sal_uInt16          nId;
    SfxItemSet*     pBBSet;

    /// bitfield
    bool            mbDisableQueryBox : 1;
    bool            mbEnableBackgroundSelector : 1;
    bool            mbEnableDrawingLayerFillStyles : 1;

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
    static sal_uInt16*      GetRanges();

private:
    SvxHeaderPage( Window* pParent, const SfxItemSet& rSet );
};

// class SvxFooterPage ---------------------------------------------------

class SVX_DLLPUBLIC SvxFooterPage : public SvxHFPage
{
public:
    static SfxTabPage*  Create( Window* pParent, const SfxItemSet& rSet );
    static sal_uInt16*      GetRanges();

private:
    SVX_DLLPRIVATE SvxFooterPage(   Window* pParent, const SfxItemSet& rSet );
};

#endif

