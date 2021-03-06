/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <vcl/weld.hxx>
#include <map>
#include <vector>

#define SPACING 6

enum SvtBreadcrumbMode
{
    ONLY_CURRENT_PATH = 0,
    ALL_VISITED = 1
};

struct BreadcrumbPath
{
    BreadcrumbPath(weld::Container* pParent);
    std::unique_ptr<weld::Builder> m_xBuilder;
    std::unique_ptr<weld::Container> m_xContainer;
    std::unique_ptr<weld::LinkButton> m_xLink;
    std::unique_ptr<weld::Label> m_xSeparator;
};

class Breadcrumb
{
private:
    weld::Container* m_pParent;
    int m_nMaxWidth;

    std::vector<std::unique_ptr<BreadcrumbPath>> m_aSegments;
    std::map<weld::LinkButton*, OUString> m_aUris;

    OUString m_sRootName;
    OUString m_sClickedURL;
    OUString m_aCurrentURL;

    SvtBreadcrumbMode m_eMode;

    Link<Breadcrumb*, bool> m_aClickHdl;

    void appendField();
    bool showField(unsigned int nIndex, unsigned int nWidthMax);

    DECL_LINK(SizeAllocHdl, const Size&, void);
    DECL_LINK(ClickLinkHdl, weld::LinkButton&, bool);

public:
    Breadcrumb(weld::Container* pParent);
    ~Breadcrumb();

    void EnableFields(bool bEnable);

    void connect_clicked(const Link<Breadcrumb*, bool>& rLink);
    const OUString& GetHdlURL() const;

    void SetRootName(const OUString& rURL);
    void SetURL(const OUString& rURL);
    void SetMode(SvtBreadcrumbMode eMode);
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
