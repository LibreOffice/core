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
#include <svl/macitem.hxx>

class SvxMacroItem;
class SvxMacro;
class SwXFrame;
class SwXTextFrame;
class SwXTextGraphicObject;
class SwXTextEmbeddedObject;
class SwXFrameStyle;
class SwFormatINetFormat;

class SwHyperlinkEventDescriptor : public SvDetachedEventDescriptor
{
    const OUString sImplName;

    //XServiceInfo
    virtual OUString SAL_CALL getImplementationName()
        throw( css::uno::RuntimeException, std::exception ) override;
protected:
    virtual ~SwHyperlinkEventDescriptor();
public:

     SwHyperlinkEventDescriptor();

    void copyMacrosFromINetFormat(const SwFormatINetFormat& aFormat);
    void copyMacrosIntoINetFormat(SwFormatINetFormat& aFormat);

    void copyMacrosFromNameReplace(
        css::uno::Reference<
            css::container::XNameReplace> & xReplace);
};

// SwEventDescriptor for
// 1) SwXTextFrame
// 2) SwXGraphicObject
// 3) SwXEmbeddedObject
// All these objects are an SwXFrame, so they can use a common implementation
class SwFrameEventDescriptor : public SvEventDescriptor
{
    OUString sSwFrameEventDescriptor;

    SwXFrame& rFrame;

public:
    SwFrameEventDescriptor( SwXTextFrame& rFrameRef );
    SwFrameEventDescriptor( SwXTextGraphicObject& rGraphicRef );
    SwFrameEventDescriptor( SwXTextEmbeddedObject& rObjectRef );

    virtual ~SwFrameEventDescriptor();

    virtual OUString SAL_CALL getImplementationName()
        throw( css::uno::RuntimeException, std::exception ) override;

protected:
    virtual void setMacroItem(const SvxMacroItem& rItem) override;
    virtual const SvxMacroItem& getMacroItem() override;
    virtual sal_uInt16 getMacroItemWhich() const override;
};

class SwFrameStyleEventDescriptor : public SvEventDescriptor
{
    OUString sSwFrameStyleEventDescriptor;

    SwXFrameStyle& rStyle;

public:
    SwFrameStyleEventDescriptor( SwXFrameStyle& rStyleRef );

    virtual ~SwFrameStyleEventDescriptor();

    virtual OUString SAL_CALL getImplementationName()
        throw( css::uno::RuntimeException, std::exception ) override;

protected:
    virtual void setMacroItem(const SvxMacroItem& rItem) override;
    virtual const SvxMacroItem& getMacroItem() override;
    virtual sal_uInt16 getMacroItemWhich() const override;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
