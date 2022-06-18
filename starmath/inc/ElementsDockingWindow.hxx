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

#include <sfx2/dockwin.hxx>
#include <vcl/customweld.hxx>
#include <vcl/weld.hxx>
#include <unotools/resmgr.hxx>

#include "node.hxx"
#include "parsebase.hxx"

#include <memory>
#include <vector>

class SmDocShell;
class SvTreeListBox;
struct ElementData;

class SmElementsControl
{
    std::unique_ptr<AbstractSmParser> maParser;

    SmDocShell*   mpDocShell;
    SmFormat      maFormat;
    size_t        mnCurrentSetIndex;
    sal_uInt16    m_nSmSyntaxVersion;

    bool          mbVerticalMode;
    std::vector<std::unique_ptr<ElementData>> maItemDatas;
    std::unique_ptr<weld::IconView> mpIconView;

    Link<OUString, void> maSelectHdlLink;

    void addElement(const OUString& aElementVisual, const OUString& aElementSource, const OUString& aHelpText);
    void addElements(size_t nCategory);

    void build();

    DECL_LINK(QueryTooltipHandler, const weld::TreeIter&, OUString);
    DECL_LINK(ElementActivatedHandler, weld::IconView&, bool);

    static OUString GetElementSource(const OUString& itemId);
    static OUString GetElementHelpText(const OUString& itemId);

public:

    explicit SmElementsControl(std::unique_ptr<weld::IconView> pIconView);
    ~SmElementsControl();

    static const std::vector<TranslateId>& categories();
    void setElementSetIndex(size_t nSetIndex);

    void setVerticalMode(bool bVertical);

    void setSmSyntaxVersion(sal_uInt16 nSmSyntaxVersion);

    void SetSelectHdl(const Link<OUString, void>& rLink) { maSelectHdlLink = rLink; }

    static Color GetTextColor();
    static Color GetControlBackground();
};

class SmElementsDockingWindow final : public SfxDockingWindow
{
    std::unique_ptr<SmElementsControl> mxElementsControl;
    std::unique_ptr<weld::ComboBox> mxElementListBox;

    virtual void Resize() override;
    SmViewShell* GetView();

    DECL_LINK(SelectClickHandler, OUString, void);
    DECL_LINK(ElementSelectedHandle, weld::ComboBox&, void);

public:

    SmElementsDockingWindow( SfxBindings* pBindings,
                             SfxChildWindow* pChildWindow,
                             vcl::Window* pParent );
    virtual ~SmElementsDockingWindow() override;
    virtual void dispose() override;

    virtual void EndDocking( const tools::Rectangle& rReactangle, bool bFloatMode) override;
    virtual void ToggleFloatingMode() override;
    virtual void GetFocus() override;

    void setSmSyntaxVersion(sal_uInt16 nSmSyntaxVersion);
};

class SmElementsDockingWindowWrapper final : public SfxChildWindow
{
    SFX_DECL_CHILDWINDOW_WITHID(SmElementsDockingWindowWrapper);

    SmElementsDockingWindowWrapper( vcl::Window* pParentWindow,
                                    sal_uInt16 nId,
                                    SfxBindings* pBindings,
                                    SfxChildWinInfo* pInfo );
    virtual ~SmElementsDockingWindowWrapper() override;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
