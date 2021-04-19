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
#ifndef INCLUDED_SW_SOURCE_CORE_INC_UNOEVENT_HXX
#define INCLUDED_SW_SOURCE_CORE_INC_UNOEVENT_HXX

#include <svtools/unoevent.hxx>

class SvxMacroItem;
class SwXFrame;
class SwXTextFrame;
class SwXTextGraphicObject;
class SwXTextEmbeddedObject;
class SwFormatINetFormat;
namespace sw
{
class ICoreFrameStyle;
}

class SwHyperlinkEventDescriptor final : public SvDetachedEventDescriptor
{
    //XServiceInfo
    virtual OUString SAL_CALL getImplementationName() override;

    virtual ~SwHyperlinkEventDescriptor() override;

public:
    SwHyperlinkEventDescriptor();

    void copyMacrosFromINetFormat(const SwFormatINetFormat& aFormat);
    void copyMacrosIntoINetFormat(SwFormatINetFormat& aFormat);

    void
    copyMacrosFromNameReplace(css::uno::Reference<css::container::XNameReplace> const& xReplace);
};

// SwEventDescriptor for
// 1) SwXTextFrame
// 2) SwXGraphicObject
// 3) SwXEmbeddedObject
// All these objects are an SwXFrame, so they can use a common implementation
class SwFrameEventDescriptor final : public SvEventDescriptor
{
    SwXFrame& m_rFrame;

public:
    SwFrameEventDescriptor(SwXTextFrame& rFrameRef);
    SwFrameEventDescriptor(SwXTextGraphicObject& rGraphicRef);
    SwFrameEventDescriptor(SwXTextEmbeddedObject& rObjectRef);

    virtual ~SwFrameEventDescriptor() override;

    virtual OUString SAL_CALL getImplementationName() override;

private:
    virtual void setMacroItem(const SvxMacroItem& rItem) override;
    virtual const SvxMacroItem& getMacroItem() override;
    virtual sal_uInt16 getMacroItemWhich() const override;
};

class SwFrameStyleEventDescriptor final : public SvEventDescriptor
{
    sw::ICoreFrameStyle& m_rStyle;

public:
    SwFrameStyleEventDescriptor(sw::ICoreFrameStyle& rStyle);

    virtual ~SwFrameStyleEventDescriptor() override;

    virtual OUString SAL_CALL getImplementationName() override;

private:
    virtual void setMacroItem(const SvxMacroItem& rItem) override;
    virtual const SvxMacroItem& getMacroItem() override;
    virtual sal_uInt16 getMacroItemWhich() const override;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
