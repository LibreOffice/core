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

#include <vcl/floatwin.hxx>
#include <vcl/layout.hxx>
#include <array>

class Edit;
class MetricField;
class ValueSet;

namespace svx { namespace sidebar {

class LinePropertyPanelBase;
class LineWidthValueSet;

class LineWidthPopup : public FloatingWindow
{
public:
    LineWidthPopup(LinePropertyPanelBase& rParent);
    virtual void dispose() override;
    virtual ~LineWidthPopup() override;

    void SetWidthSelect (long lValue, bool bValuable, MapUnit eMapUnit);

private:
    LinePropertyPanelBase& m_rParent;
    std::array<OUString,9> maStrUnits;
    OUString const m_sPt;
    MapUnit m_eMapUnit;
    bool m_bVSFocus;
    bool m_bCustom;
    long m_nCustomWidth;
    VclPtr<MetricField> m_xMFWidth;
    VclPtr<VclContainer> m_xBox;
    VclPtr<LineWidthValueSet> m_xVSWidth;
    Image const m_aIMGCus;
    Image const m_aIMGCusGray;

    DECL_LINK(VSSelectHdl, ValueSet*, void);
    DECL_LINK(MFModifyHdl, Edit&, void);
};

} } // end of namespace svx::sidebar

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
