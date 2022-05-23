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

class SmElement
{
    std::unique_ptr<SmNode> mpNode;
    OUString                maText;
    OUString                maHelpText;

public:
    Point mBoxLocation;
    Size  mBoxSize;

    SmElement(std::unique_ptr<SmNode>&& pNode, const OUString& aText, const OUString& aHelpText);
    virtual ~SmElement();

    const std::unique_ptr<SmNode>& getNode() const;
    const OUString& getText() const { return maText; }
    const OUString& getHelpText() const { return maHelpText; }

    virtual bool isSeparator() const { return false; }
};

class SmElementSeparator final : public SmElement
{
public:
    SmElementSeparator();

    bool isSeparator() const override { return true; }
};

struct ElementData;

class SmElementsControl
{
    friend class ElementSelectorUIObject;
    friend class ElementUIObject;

    std::unique_ptr<AbstractSmParser> maParser;

    SmDocShell*   mpDocShell;
    SmFormat      maFormat;
    TranslateId   msCurrentSetId;
    sal_uInt16    m_nSmSyntaxVersion;
    Link<weld::IconView&,bool> maSelectHdlLink;

    bool          mbVerticalMode;
    std::vector<std::unique_ptr<weld::Builder>> maBuilders;
    std::unique_ptr<weld::Container> mpParentContainer;
    std::vector<std::unique_ptr<weld::Box>> maElementBoxes;
    std::vector<std::unique_ptr<weld::IconView>> maElementIconViews;
    std::vector<std::unique_ptr<ElementData>> maItemDatas;

    bool m_bFirstPaintAfterLayout;

    void addElement(const OUString& aElementVisual, const OUString& aElementSource, const OUString& aHelpText);
    void addElements(const TranslateId& rCategory);

    void build();

    DECL_LINK(QueryTooltipHandler, const weld::TreeIter&, OUString);

public:

    explicit SmElementsControl(std::unique_ptr<weld::Container> xContainer);
    ~SmElementsControl();

    static const std::vector<TranslateId>& categories();
    const TranslateId& elementSetId() const { return msCurrentSetId; }
    void setElementSetId(TranslateId pSetId);

    void setVerticalMode(bool bVertical);

    void setSmSyntaxVersion(sal_uInt16 nSmSyntaxVersion);

    void SetSelectHdl(const Link<weld::IconView&, bool>& rLink) { maSelectHdlLink = rLink; }

    static Color GetTextColor();
    static Color GetControlBackground();
};

class SmElementsDockingWindow final : public SfxDockingWindow
{
    std::unique_ptr<SmElementsControl> mxElementsControl;
    std::unique_ptr<weld::ComboBox> mxElementListBox;

    virtual void Resize() override;
    SmViewShell* GetView();

    DECL_LINK(SelectClickHandler, weld::IconView&, bool);
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
