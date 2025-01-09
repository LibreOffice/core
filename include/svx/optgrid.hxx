/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */
#ifndef INCLUDED_SVX_OPTGRID_HXX
#define INCLUDED_SVX_OPTGRID_HXX

#include <sfx2/tabdlg.hxx>
#include <sfx2/htmlmode.hxx>
#include <svx/svxdllapi.h>

namespace weld { class CheckButton; }
namespace weld { class MetricSpinButton; }
namespace weld { class SpinButton; }
namespace weld { class ToggleButton; }
namespace weld { class Widget; }

class SVX_DLLPUBLIC SvxOptionsGrid
{
protected:
    sal_uInt32  nFldDrawX;
    sal_uInt32  nFldDivisionX;
    sal_uInt32  nFldDrawY;
    sal_uInt32  nFldDivisionY;
    bool        bUseGridsnap:1;
    bool        bSynchronize:1;
    bool        bGridVisible:1;
    bool        bEqualGrid:  1;

public:
    SvxOptionsGrid();

    void    SetFieldDrawX(    sal_uInt32 nSet){nFldDrawX      = nSet;}
    void    SetFieldDivisionX(sal_uInt32 nSet){nFldDivisionX  = nSet;}
    void    SetFieldDrawY   ( sal_uInt32 nSet){nFldDrawY      = nSet;}
    void    SetFieldDivisionY(sal_uInt32 nSet){nFldDivisionY  = nSet;}
    void    SetUseGridSnap( bool bSet ) {bUseGridsnap   = bSet;}
    void    SetSynchronize( bool bSet ) {bSynchronize   = bSet;}
    void    SetGridVisible( bool bSet ) {bGridVisible   = bSet;}
    void    SetEqualGrid( bool bSet )   {bEqualGrid     = bSet;}

    sal_uInt32  GetFieldDrawX(    ) const {  return nFldDrawX;    }
    sal_uInt32  GetFieldDivisionX() const {  return nFldDivisionX;}
    sal_uInt32  GetFieldDrawY   ( ) const {  return nFldDrawY;    }
    sal_uInt32  GetFieldDivisionY() const {  return nFldDivisionY;}
    bool        GetUseGridSnap( ) const {  return bUseGridsnap; }
    bool        GetSynchronize( ) const {  return bSynchronize; }
    bool        GetGridVisible( ) const {  return bGridVisible; }
    bool        GetEqualGrid()    const {  return bEqualGrid;   }
};

class SVX_DLLPUBLIC SvxGridItem : public SvxOptionsGrid, public SfxPoolItem
{
    // #i9076#
    friend class SvxGridTabPage;

public:
    DECLARE_ITEM_TYPE_FUNCTION(SvxGridItem)
    SvxGridItem( TypedWhichId<SvxGridItem> _nWhich)
        : SfxPoolItem(_nWhich){};

    virtual SvxGridItem*    Clone( SfxItemPool *pPool = nullptr ) const override;
    virtual bool            operator==( const SfxPoolItem& ) const override;

    virtual bool GetPresentation( SfxItemPresentation ePres,
                                  MapUnit eCoreMetric,
                                  MapUnit ePresMetric,
                                  OUString &rText, const IntlWrapper& ) const override;

};

class SVX_DLLPUBLIC SvxGridTabPage : public SfxTabPage
{
public:
    SvxGridTabPage(weld::Container* pPage, weld::DialogController* pController, const SfxItemSet& rSet);
    virtual ~SvxGridTabPage() override;

    static std::unique_ptr<SfxTabPage> Create(weld::Container* pPage, weld::DialogController* pController, const SfxItemSet& rAttrSet);

    virtual OUString GetAllStrings() override;

    virtual bool        FillItemSet( SfxItemSet* rSet ) override;
    virtual void        Reset( const SfxItemSet* rSet ) override;

    virtual void        ActivatePage( const SfxItemSet& rSet ) override;
    virtual DeactivateRC   DeactivatePage( SfxItemSet* pSet ) override;

private:
    enum ModuleMode
    {
        WRITER_MODE = 0,
        CALC_MODE = 1,
        IMPRESS_MODE = 2,
        DRAW_MODE = 3,
        HTML_MODE = 4
    };
    bool                bAttrModified;
    ModuleMode          m_Emode;

    std::unique_ptr<weld::CheckButton> m_xCbxUseGridsnap;
    std::unique_ptr<weld::Widget> m_xCbxUseGridsnapImg;
    std::unique_ptr<weld::CheckButton> m_xCbxGridVisible;
    std::unique_ptr<weld::Widget> m_xCbxGridVisibleImg;
    std::unique_ptr<weld::MetricSpinButton> m_xMtrFldDrawX;
    std::unique_ptr<weld::Widget> m_xMtrFldDrawXImg;
    std::unique_ptr<weld::MetricSpinButton> m_xMtrFldDrawY;
    std::unique_ptr<weld::Widget> m_xMtrFldDrawYImg;
    std::unique_ptr<weld::SpinButton> m_xNumFldDivisionX;
    std::unique_ptr<weld::Widget> m_xNumFldDivisionXImg;
    std::unique_ptr<weld::SpinButton> m_xNumFldDivisionY;
    std::unique_ptr<weld::Widget> m_xNumFldDivisionYImg;
    std::unique_ptr<weld::CheckButton> m_xCbxSynchronize;
    std::unique_ptr<weld::Widget> m_xCbxSynchronizeImg;
protected:
    //these controls are used in draw and impress
    std::unique_ptr<weld::Widget> m_xSnapFrames;
    std::unique_ptr<weld::CheckButton> m_xCbxSnapHelplines;
    std::unique_ptr<weld::Widget> m_xCbxSnapHelplinesImg;
    std::unique_ptr<weld::CheckButton> m_xCbxSnapBorder;
    std::unique_ptr<weld::Widget> m_xCbxSnapBorderImg;
    std::unique_ptr<weld::CheckButton> m_xCbxSnapFrame;
    std::unique_ptr<weld::Widget> m_xCbxSnapFrameImg;
    std::unique_ptr<weld::CheckButton> m_xCbxSnapPoints;
    std::unique_ptr<weld::Widget> m_xCbxSnapPointsImg;
    std::unique_ptr<weld::MetricSpinButton> m_xMtrFldSnapArea;
    std::unique_ptr<weld::Widget> m_xMtrFldSnapAreaImg;
    std::unique_ptr<weld::CheckButton> m_xCbxOrtho;
    std::unique_ptr<weld::Widget> m_xCbxOrthoImg;
    std::unique_ptr<weld::CheckButton> m_xCbxBigOrtho;
    std::unique_ptr<weld::Widget> m_xCbxBigOrthoImg;
    std::unique_ptr<weld::CheckButton> m_xCbxRotate;
    std::unique_ptr<weld::Widget> m_xCbxRotateImg;
    std::unique_ptr<weld::MetricSpinButton> m_xMtrFldAngle;
    std::unique_ptr<weld::MetricSpinButton> m_xMtrFldBezAngle;
    std::unique_ptr<weld::Widget> m_xMtrFldBezAngleImg;

    bool IsDrawMode() const { return m_Emode == DRAW_MODE; }

    DECL_LINK(ClickRotateHdl_Impl, weld::Toggleable&, void);
private:
    DECL_DLLPRIVATE_LINK(ChangeDrawHdl_Impl, weld::MetricSpinButton&, void);
    DECL_DLLPRIVATE_LINK(ChangeGridsnapHdl_Impl, weld::Toggleable&, void);
    DECL_DLLPRIVATE_LINK(ChangeDivisionHdl_Impl, weld::SpinButton&, void);
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
