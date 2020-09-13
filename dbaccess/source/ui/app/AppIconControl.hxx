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

#include <sfx2/thumbnailview.hxx>
#include <sfx2/thumbnailviewitem.hxx>
#include <vcl/transfer.hxx>
#include <vcl/customweld.hxx>
#include <vcl/weld.hxx>
#include <AppElementType.hxx>

class MnemonicGenerator;

namespace dbaui
{
    class IControlActionListener;
    class IconControl;
    class OApplicationIconControlDropTarget;

    class OApplicationIconControl final : public SfxThumbnailView
    {
        std::unique_ptr<OApplicationIconControlDropTarget> m_xDropTarget;
        IControlActionListener* m_pActionListener;

        long m_nMaxWidth;
        long m_nMaxHeight;

        bool IsMnemonicChar(sal_Unicode cChar, ElementType& rType) const;

    public:
        explicit OApplicationIconControl(std::unique_ptr<weld::ScrolledWindow> xScroll);
        virtual void SetDrawingArea(weld::DrawingArea* pDrawingArea) override;
        virtual void Resize() override;
        virtual tools::Rectangle GetFocusRect() override;
        virtual void GetFocus() override;
        virtual void LoseFocus() override;
        bool DoKeyShortCut(const KeyEvent& rKEvt);
        virtual bool KeyInput(const KeyEvent& rKEvt) override;
        virtual ~OApplicationIconControl() override;

        ElementType GetSelectedItem() const;

        void setControlActionListener( IControlActionListener* _pListener ) { m_pActionListener = _pListener; }
        void Fill();

        void createIconAutoMnemonics(MnemonicGenerator& rMnemonics);

        sal_Int8 AcceptDrop(const AcceptDropEvent& rEvt);
        sal_Int8 ExecuteDrop(const ExecuteDropEvent& rEvt);
    };
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
