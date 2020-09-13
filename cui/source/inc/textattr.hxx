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
#pragma once

#include <svx/dlgctrl.hxx>
#include <svx/svdobj.hxx>

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

    const SfxItemSet&   rOutAttrs;
    SdrObjKind    m_eObjKind;

    bool                bAutoGrowSizeEnabled;
    bool                bContourEnabled;
    bool                bAutoGrowWidthEnabled;
    bool                bAutoGrowHeightEnabled;
    bool                bWordWrapTextEnabled;
    bool                bFitToSizeEnabled;

    SvxRectCtl m_aCtlPosition;

    std::unique_ptr<weld::Widget> m_xDrawingText;
    std::unique_ptr<weld::Widget> m_xCustomShapeText;
    std::unique_ptr<weld::CheckButton> m_xTsbAutoGrowWidth;
    std::unique_ptr<weld::CheckButton> m_xTsbAutoGrowHeight;
    std::unique_ptr<weld::CheckButton> m_xTsbFitToSize;
    std::unique_ptr<weld::CheckButton> m_xTsbContour;
    std::unique_ptr<weld::CheckButton> m_xTsbWordWrapText;
    std::unique_ptr<weld::CheckButton> m_xTsbAutoGrowSize;
    std::unique_ptr<weld::Frame> m_xFlDistance;
    std::unique_ptr<weld::MetricSpinButton> m_xMtrFldLeft;
    std::unique_ptr<weld::MetricSpinButton> m_xMtrFldRight;
    std::unique_ptr<weld::MetricSpinButton> m_xMtrFldTop;
    std::unique_ptr<weld::MetricSpinButton> m_xMtrFldBottom;
    std::unique_ptr<weld::Frame> m_xFlPosition;
    std::unique_ptr<weld::CustomWeld> m_xCtlPosition;
    std::unique_ptr<weld::CheckButton> m_xTsbFullWidth;

    DECL_LINK(ClickFullWidthHdl_Impl, weld::Button&, void);
    DECL_LINK(ClickHdl_Impl, weld::Button&, void);

    /** Return whether the text direction is from left to right (</sal_True>) or
        top to bottom (</sal_False>).
    */
    bool IsTextDirectionLeftToRight() const;

public:

    SvxTextAttrPage(weld::Container* pPage, weld::DialogController* pController, const SfxItemSet& rInAttrs);
    virtual ~SvxTextAttrPage() override;

    static std::unique_ptr<SfxTabPage> Create( weld::Container* pPage, weld::DialogController* pController, const SfxItemSet* );
    static const sal_uInt16*  GetRanges() { return pRanges; }

    virtual bool        FillItemSet( SfxItemSet* ) override;
    virtual void        Reset( const SfxItemSet * ) override;

    virtual void        PointChanged( weld::DrawingArea* pWindow, RectPoint eRP ) override;

    void         Construct();
    void         SetObjKind(SdrObjKind eObjKind) { m_eObjKind = eObjKind; }
    virtual void PageCreated(const SfxAllItemSet& aSet) override;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
