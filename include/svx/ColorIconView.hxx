/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <svx/IColorSet.hxx>
#include <svx/svxdllapi.h>
#include <tools/link.hxx>
#include <vcl/vclptr.hxx>

class VirtualDevice;

namespace weld
{
class IconView;
class TreeIter;
}

class SVXCORE_DLLPUBLIC ColorIconView : public IColorSet
{
    std::unique_ptr<weld::IconView> m_pIconView;

    Link<const Color&, void> m_aColorActivatedHdl;

    DECL_LINK(ItemActivatedHdl, const weld::TreeIter&, bool);

public:
    ColorIconView(std::unique_ptr<weld::IconView> pIconView);
    virtual ~ColorIconView() = default;

    virtual void insert(int nIndex, const Color& rColor, const OUString& rColorName) override;
    virtual int getItemCount() const override;

    Color getColor(int nIndex) const;
    OUString getColorName(int nIndex) const;

    int get_selected_index() const;
    void select(int nIndex);
    void unselect_all();

    void remove(int nIndex);
    virtual void clear() override;

    void setColorActivatedHdl(const Link<const Color&, void>& rLink)
    {
        m_aColorActivatedHdl = rLink;
    }

private:
    ScopedVclPtr<VirtualDevice> createIcon(const Color& rColor);
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
