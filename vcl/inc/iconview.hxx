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

#ifndef INCLUDED_SVTOOLS_ICONVIEW_HXX
#define INCLUDED_SVTOOLS_ICONVIEW_HXX

#include <tools/json_writer.hxx>
#include <vcl/toolkit/treelistbox.hxx>

class IconView final : public SvTreeListBox
{
public:
    IconView(vcl::Window* pParent, WinBits nBits);

    Size GetEntrySize(const SvTreeListEntry&) const;

    virtual void Resize() override;

    virtual tools::Rectangle GetFocusRect(const SvTreeListEntry*, tools::Long) override;

    void PaintEntry(SvTreeListEntry&, tools::Long nX, tools::Long nY,
                    vcl::RenderContext& rRenderContext);

    virtual css::uno::Reference<css::accessibility::XAccessible> CreateAccessible() override;

    virtual FactoryFunction GetUITestFactory() const override;
    virtual void DumpAsPropertyTree(tools::JsonWriter& rJsonWriter) override;
    typedef std::tuple<OUString&, SvTreeListEntry*> encoded_image_query;

    void SetDumpImageHdl(const Link<const encoded_image_query&, bool>& rLink)
    {
        maDumpImageHdl = rLink;
    }

    /// returns string with encoded image for an entry
    OUString renderEntry(int pos, int dpix, int dpiy) const;

protected:
    virtual void CalcEntryHeight(SvTreeListEntry const* pEntry) override;

private:
    Link<const encoded_image_query&, bool> maDumpImageHdl;
    void DumpEntryAndSiblings(tools::JsonWriter& rJsonWriter, SvTreeListEntry* pEntry);
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
