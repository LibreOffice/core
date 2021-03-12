/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <sfx2/dllapi.h>
#include <vcl/weld.hxx>

class DataChangedEvent;
class VclSimpleEvent;
namespace sfx2 { namespace sidebar { class Panel; } }
namespace tools { class JsonWriter; }

/// This class is the base for the Widget Layout-based sidebar panels.
class SFX2_DLLPUBLIC PanelLayout
{
protected:
    std::unique_ptr<weld::Builder> m_xBuilder;
    std::unique_ptr<weld::Container> m_xContainer;
    sfx2::sidebar::Panel* m_pPanel;

    virtual void DataChanged(const DataChangedEvent& rEvent);
    virtual void DumpAsPropertyTree(tools::JsonWriter&);

    virtual weld::Window* GetFrameWeld() const;

private:
    DECL_LINK(DataChangedEventListener, VclSimpleEvent&, void);
    DECL_LINK(DumpAsPropertyTreeHdl, tools::JsonWriter&, void);

public:
    PanelLayout(weld::Widget* pParent, const OString& rID, const OUString& rUIXMLDescription);

    void SetPanel(sfx2::sidebar::Panel* pPanel);

    virtual ~PanelLayout();

    Size get_preferred_size() const
    {
        return m_xContainer->get_preferred_size();
    }

    void queue_resize();
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
