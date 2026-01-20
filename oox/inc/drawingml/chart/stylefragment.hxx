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

#include <drawingml/chart/chartcontextbase.hxx>

namespace model
{
struct StyleColor;
}

namespace oox::drawingml::chart
{
/** Handler for a cs:CT_StyleReference or cs:CT_FontReference element.
 */
class StyleReferenceContext final : public ContextBase<model::StyleColor>
{
public:
    StyleReferenceContext(ContextHandler2Helper& rParent, sal_Int32 nIdx,
                          model::StyleColor& rModel);
    virtual ~StyleReferenceContext() override;

    virtual ::oox::core::ContextHandlerRef onCreateContext(sal_Int32 nElement,
                                                           const AttributeList& rAttribs) override;
};

struct StyleEntryModel;

/** Handler for a cs:CT_StyleEntry element.
 */
class StyleEntryContext final : public ContextBase<StyleEntryModel>
{
public:
    StyleEntryContext(ContextHandler2Helper& rParent, StyleEntryModel& rModel);
    virtual ~StyleEntryContext() override;

    virtual ::oox::core::ContextHandlerRef onCreateContext(sal_Int32 nElement,
                                                           const AttributeList& rAttribs) override;
    virtual void onCharacters(const OUString& rChars) override;
};

struct StyleModel;

/** Handler for a style fragment (cs:chartStyle root element).
 */
class StyleFragment final : public FragmentBase<StyleModel>
{
public:
    explicit StyleFragment(::oox::core::XmlFilterBase& rFilter, const OUString& rFragmentPath,
                           StyleModel& rModel);
    virtual ~StyleFragment() override;

    virtual ::oox::core::ContextHandlerRef onCreateContext(sal_Int32 nElement,
                                                           const AttributeList& rAttribs) override;
};

} // namespace oox::drawingml::chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
