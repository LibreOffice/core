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

#include <editeng/ulspitem.hxx>
#include <vcl/EnumContext.hxx>
#include <vcl/InterimItemWindow.hxx>
#include <svx/relfld.hxx>

namespace svx
{
class ParaULSpacingWindow : public InterimItemWindow
{
public:
    virtual ~ParaULSpacingWindow() override;
    virtual void dispose() override;

    void SetValue(const SvxULSpaceItem* pItem);
    void SetUnit(FieldUnit eUnit);

protected:
    ParaULSpacingWindow(vcl::Window* pParent);

    std::optional<SvxRelativeField> m_xAboveSpacing;
    std::optional<SvxRelativeField> m_xBelowSpacing;
    std::unique_ptr<weld::Container> m_xAboveContainer;
    std::unique_ptr<weld::Container> m_xBelowContainer;

    MapUnit m_eUnit;

    DECL_LINK(ModifySpacingHdl, weld::MetricSpinButton&, void);
};

class ParaAboveSpacingWindow : public ParaULSpacingWindow
{
public:
    explicit ParaAboveSpacingWindow(vcl::Window* pParent);
};

class ParaBelowSpacingWindow : public ParaULSpacingWindow
{
public:
    explicit ParaBelowSpacingWindow(vcl::Window* pParent);
};

class ParaLRSpacingWindow : public InterimItemWindow
{
public:
    virtual ~ParaLRSpacingWindow() override;
    virtual void dispose() override;

    void SetValue(SfxItemState eState, const SfxPoolItem* pState);
    void SetUnit(FieldUnit eUnit);
    void SetContext(const vcl::EnumContext& eContext);

protected:
    ParaLRSpacingWindow(vcl::Window* pParent);

    std::optional<SvxRelativeField> m_xBeforeSpacing;
    std::optional<SvxRelativeField> m_xAfterSpacing;
    std::optional<SvxRelativeField> m_xFLSpacing;
    std::unique_ptr<weld::Container> m_xBeforeContainer;
    std::unique_ptr<weld::Container> m_xAfterContainer;
    std::unique_ptr<weld::Container> m_xFirstLineContainer;

    MapUnit m_eUnit;

    vcl::EnumContext m_aContext;

    DECL_LINK(ModifySpacingHdl, weld::MetricSpinButton&, void);
};

class ParaLeftSpacingWindow : public ParaLRSpacingWindow
{
public:
    explicit ParaLeftSpacingWindow(vcl::Window* pParent);
};

class ParaRightSpacingWindow : public ParaLRSpacingWindow
{
public:
    explicit ParaRightSpacingWindow(vcl::Window* pParent);
};

class ParaFirstLineSpacingWindow : public ParaLRSpacingWindow
{
public:
    explicit ParaFirstLineSpacingWindow(vcl::Window* pParent);
};
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
