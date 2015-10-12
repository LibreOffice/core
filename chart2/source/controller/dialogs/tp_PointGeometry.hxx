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
#ifndef INCLUDED_CHART2_SOURCE_CONTROLLER_DIALOGS_TP_POINTGEOMETRY_HXX
#define INCLUDED_CHART2_SOURCE_CONTROLLER_DIALOGS_TP_POINTGEOMETRY_HXX

#include <sfx2/tabdlg.hxx>

namespace chart
{

class BarGeometryResources;
class SchLayoutTabPage : public SfxTabPage
{
public:
    SchLayoutTabPage(vcl::Window* pParent, const SfxItemSet& rInAttrs);
    virtual ~SchLayoutTabPage();
    virtual void dispose() override;

    static VclPtr<SfxTabPage> Create(vcl::Window* pParent, const SfxItemSet* rInAttrs);
    virtual bool FillItemSet(SfxItemSet* rOutAttrs) override;
    virtual void Reset(const SfxItemSet* rInAttrs) override;

private:
    BarGeometryResources*   m_pGeometryResources;
};

} //namespace chart

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
