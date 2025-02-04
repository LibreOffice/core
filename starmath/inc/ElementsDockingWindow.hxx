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
struct ElementData;

class SmElementsControl
{
    std::unique_ptr<AbstractSmParser> maParser;

    SmDocShell*   mpDocShell;
    SmFormat      maFormat;
    int           mnCurrentSetIndex;
    sal_Int16     m_nSmSyntaxVersion;
    bool          m_bAllowDelete;
    OUString      m_sHoveredItem;

    std::vector<std::unique_ptr<ElementData>> maItemDatas;
    std::unique_ptr<weld::IconView> mpIconView;
    std::unique_ptr<weld::Menu> mxPopup;

    Link<const OUString&, void> maSelectHdlLink;

    void addElement(const OUString& aElementVisual, const OUString& aElementSource, const OUString& aHelpText);
    void addElements(int nCategory);

    void build();

    DECL_LINK(QueryTooltipHandler, const weld::TreeIter&, OUString);
    DECL_LINK(ElementActivatedHandler, weld::IconView&, bool);
    DECL_LINK(MousePressHdl, const MouseEvent&, bool);

    static OUString GetElementSource(const OUString& itemId);
    static OUString GetElementHelpText(const OUString& itemId);
    static int GetElementPos(const OUString& itemId);

public:

    explicit SmElementsControl(std::unique_ptr<weld::IconView> pIconView,
                               std::unique_ptr<weld::Menu> pMenu);
    ~SmElementsControl();

    static const std::vector<TranslateId>& categories();
    void setElementSetIndex(int nSetIndex, bool bForceBuild = false);

    void setSmSyntaxVersion(sal_Int16 nSmSyntaxVersion);

    void SetSelectHdl(const Link<const OUString&, void>& rLink) { maSelectHdlLink = rLink; }

    void SetAllowDelete(bool bAllow) { m_bAllowDelete = bAllow; }

    static Color GetTextColor();
    static Color GetControlBackground();
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
