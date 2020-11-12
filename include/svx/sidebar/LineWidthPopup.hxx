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
#ifndef INCLUDED_SVX_SOURCE_SIDEBAR_LINE_LINEWIDTHPOPUP_HXX
#define INCLUDED_SVX_SOURCE_SIDEBAR_LINE_LINEWIDTHPOPUP_HXX

#include <tools/mapunit.hxx>
#include <vcl/customweld.hxx>
#include <vcl/image.hxx>
#include <vcl/weld.hxx>
#include <array>

class ValueSet;

namespace svx::sidebar
{
class LinePropertyPanelBase;
class LineWidthValueSet;

class LineWidthPopup final
{
public:
    LineWidthPopup(weld::Widget* pParent, LinePropertyPanelBase& rParent);
    ~LineWidthPopup();

    void SetWidthSelect(tools::Long lValue, bool bValuable, MapUnit eMapUnit);

    weld::Container* getTopLevel() const { return m_xTopLevel.get(); }

private:
    LinePropertyPanelBase& m_rParent;
    std::array<OUString, 9> maStrUnits;
    OUString m_sPt;
    MapUnit m_eMapUnit;
    bool m_bVSFocus;
    bool m_bCustom;
    tools::Long m_nCustomWidth;
    Image m_aIMGCus;
    Image m_aIMGCusGray;

    std::unique_ptr<weld::Builder> m_xBuilder;
    std::unique_ptr<weld::Container> m_xTopLevel;
    std::unique_ptr<weld::MetricSpinButton> m_xMFWidth;
    std::unique_ptr<LineWidthValueSet> m_xVSWidth;
    std::unique_ptr<weld::CustomWeld> m_xVSWidthWin;

    DECL_LINK(VSSelectHdl, ValueSet*, void);
    DECL_LINK(MFModifyHdl, weld::MetricSpinButton&, void);
    DECL_LINK(FocusHdl, weld::Widget&, void);
};

} // end of namespace svx::sidebar

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
