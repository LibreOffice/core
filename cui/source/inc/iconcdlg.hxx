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

#include <rtl/ustring.hxx>
#include <sfx2/tabdlg.hxx>

// forward-declarations
struct IconChoicePageData;
class SvxHpLinkDlg;
class IconChoicePage;
class SfxItemPool;
class SfxItemSet;

// Create-Function
typedef std::unique_ptr<IconChoicePage> (*CreatePage)(weld::Container* pParent, SvxHpLinkDlg* pDlg, const SfxItemSet* pAttrSet);

/// Data-structure for pages in dialog
struct IconChoicePageData
{
    OString sId;
    std::unique_ptr<IconChoicePage> xPage;      ///< the TabPage itself
    bool bRefresh;          ///< Flag: page has to be newly initialized

    // constructor
    IconChoicePageData(const OString& rId, std::unique_ptr<IconChoicePage> xInPage)
        : sId(rId)
        , xPage(std::move(xInPage))
        , bRefresh(false)
    {}
};

class IconChoicePage
{
protected:
    std::unique_ptr<weld::Builder> xBuilder;
    std::unique_ptr<weld::Container> xContainer;

private:
    const SfxItemSet*   pSet;
    bool                bHasExchangeSupport;

protected:

    IconChoicePage(weld::Container* pParent, const OUString& rUIXMLDescription, const OString& rID, const SfxItemSet* pItemSet);

public:
    virtual ~IconChoicePage();

    OString GetHelpId() const { return xContainer->get_help_id(); }

    const SfxItemSet&   GetItemSet() const { return *pSet; }

    virtual bool        FillItemSet( SfxItemSet* ) = 0;
    virtual void        Reset( const SfxItemSet& ) = 0;

    bool                HasExchangeSupport() const { return bHasExchangeSupport; }
    void                SetExchangeSupport()       { bHasExchangeSupport = true; }

    virtual void        ActivatePage( const SfxItemSet& );
    virtual DeactivateRC DeactivatePage( SfxItemSet* pSet );
    virtual bool        QueryClose();
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
