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
#ifndef INCLUDED_CUI_SOURCE_INC_TEXTATTR_HXX
#define INCLUDED_CUI_SOURCE_INC_TEXTATTR_HXX

#include <svx/dlgctrl.hxx>

#include <vcl/group.hxx>

#include <vcl/fixed.hxx>
#include <sfx2/basedlgs.hxx>

class SdrView;

/*************************************************************************
|*
|* Dialog for changing TextAttributes
|*
\************************************************************************/

class SvxTextAttrPage : public SvxTabPage
{
private:
    static const sal_uInt16 pRanges[];

    VclPtr<TriStateBox>         m_pTsbAutoGrowWidth;
    VclPtr<TriStateBox>         m_pTsbAutoGrowHeight;
    VclPtr<TriStateBox>         m_pTsbFitToSize;
    VclPtr<TriStateBox>         m_pTsbContour;
    VclPtr<TriStateBox>         m_pTsbWordWrapText;
    VclPtr<TriStateBox>         m_pTsbAutoGrowSize;

    VclPtr<VclFrame>            m_pFlDistance;
    VclPtr<MetricField>         m_pMtrFldLeft;
    VclPtr<MetricField>         m_pMtrFldRight;
    VclPtr<MetricField>         m_pMtrFldTop;
    VclPtr<MetricField>         m_pMtrFldBottom;

    VclPtr<VclFrame>            m_pFlPosition;
    VclPtr<SvxRectCtl>          m_pCtlPosition;
    VclPtr<TriStateBox>         m_pTsbFullWidth;

    const SfxItemSet&   rOutAttrs;
    const SdrView*      pView;

    bool                bAutoGrowSizeEnabled;
    bool                bContourEnabled;
    bool                bAutoGrowWidthEnabled;
    bool                bAutoGrowHeightEnabled;
    bool                bWordWrapTextEnabled;
    bool                bFitToSizeEnabled;

    DECL_LINK_TYPED( ClickFullWidthHdl_Impl, Button*, void );
    DECL_LINK_TYPED( ClickHdl_Impl, Button*, void );

    /** Return whether the text direction is from left to right (</sal_True>) or
        top to bottom (</sal_False>).
    */
    bool IsTextDirectionLeftToRight() const;

public:

    SvxTextAttrPage( vcl::Window* pWindow, const SfxItemSet& rInAttrs );
    virtual ~SvxTextAttrPage();
    virtual void dispose() override;

    static VclPtr<SfxTabPage>  Create( vcl::Window*, const SfxItemSet* );
    static const sal_uInt16*  GetRanges() { return pRanges; }

    virtual bool        FillItemSet( SfxItemSet* ) override;
    virtual void        Reset( const SfxItemSet * ) override;

    virtual void        PointChanged( vcl::Window* pWindow, RECT_POINT eRP ) override;

    void         Construct();
    void         SetView( const SdrView* pSdrView ) { pView = pSdrView; }
    virtual void PageCreated(const SfxAllItemSet& aSet) override;
};

#endif // INCLUDED_CUI_SOURCE_INC_TEXTATTR_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
