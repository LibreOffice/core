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
    TriStateBox*         m_pTsbAutoGrowWidth;
    TriStateBox*         m_pTsbAutoGrowHeight;
    TriStateBox*         m_pTsbFitToSize;
    TriStateBox*         m_pTsbContour;
    TriStateBox*         m_pTsbWordWrapText;
    TriStateBox*         m_pTsbAutoGrowSize;

    VclFrame*            m_pFlDistance;
    MetricField*         m_pMtrFldLeft;
    MetricField*         m_pMtrFldRight;
    MetricField*         m_pMtrFldTop;
    MetricField*         m_pMtrFldBottom;

    VclFrame*            m_pFlPosition;
    SvxRectCtl*          m_pCtlPosition;
    TriStateBox*         m_pTsbFullWidth;

    const SfxItemSet&   rOutAttrs;
    const SdrView*      pView;

    bool                bAutoGrowSizeEnabled;
    bool                bContourEnabled;
    bool                bAutoGrowWidthEnabled;
    bool                bAutoGrowHeightEnabled;
    bool                bWordWrapTextEnabled;
    bool                bFitToSizeEnabled;

    DECL_LINK( ClickFullWidthHdl_Impl, void * );
    DECL_LINK( ClickHdl_Impl, void * );

    /** Return whether the text direction is from left to right (</sal_True>) or
        top to bottom (</sal_False>).
    */
    bool IsTextDirectionLeftToRight (void) const;

public:

    SvxTextAttrPage( Window* pWindow, const SfxItemSet& rInAttrs );
    virtual ~SvxTextAttrPage();

    static SfxTabPage*  Create( Window*, const SfxItemSet& );
    static  const sal_uInt16*     GetRanges();

    virtual bool        FillItemSet( SfxItemSet& ) SAL_OVERRIDE;
    virtual void        Reset( const SfxItemSet & ) SAL_OVERRIDE;

    virtual void        PointChanged( Window* pWindow, RECT_POINT eRP ) SAL_OVERRIDE;

    void         Construct();
    void         SetView( const SdrView* pSdrView ) { pView = pSdrView; }
    virtual void PageCreated(const SfxAllItemSet& aSet) SAL_OVERRIDE;
};

#endif // INCLUDED_CUI_SOURCE_INC_TEXTATTR_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
