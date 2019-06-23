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
#ifndef INCLUDED_CUI_SOURCE_INC_ICONCDLG_HXX
#define INCLUDED_CUI_SOURCE_INC_ICONCDLG_HXX

#include <rtl/ustring.hxx>
#include <svtools/ivctrl.hxx>
#include <vcl/tabpage.hxx>
#include <vcl/dialog.hxx>
#include <vcl/button.hxx>
#include <vcl/image.hxx>
#include <vcl/layout.hxx>
#include <sfx2/tabdlg.hxx>
#include <sfx2/basedlgs.hxx>
#include <vector>

// forward-declarations
struct IconChoicePageData;
class SvxHpLinkDlg;
class IconChoicePage;
class SfxItemPool;
class SfxItemSet;
enum class HyperLinkPageType;

// Create-Function
typedef VclPtr<IconChoicePage> (*CreatePage)(vcl::Window *pParent, SvxHpLinkDlg* pDlg, const SfxItemSet* pAttrSet);

/// Data-structure for pages in dialog
struct IconChoicePageData
{
    HyperLinkPageType nId;
    CreatePage fnCreatePage;    ///< pointer to the factory
    VclPtr<IconChoicePage> pPage;      ///< the TabPage itself
    bool bRefresh;          ///< Flag: page has to be newly initialized

    // constructor
    IconChoicePageData( HyperLinkPageType Id, CreatePage fnPage )
        : nId           ( Id ),
          fnCreatePage  ( fnPage ),
          pPage         ( nullptr ),
          bRefresh      ( false )
    {}
};

class IconChoicePage : public TabPage
{
private:
    const SfxItemSet*   pSet;
    bool                bHasExchangeSupport;

    void                ImplInitSettings();

protected:
    using TabPage::ActivatePage;
    using TabPage::DeactivatePage;

    IconChoicePage( vcl::Window *pParent, const OString& rID, const OUString& rUIXMLDescription, const SfxItemSet* pItemSet );

public:
    virtual ~IconChoicePage() override;

    const SfxItemSet&   GetItemSet() const { return *pSet; }

    virtual bool        FillItemSet( SfxItemSet* ) = 0;
    virtual void        Reset( const SfxItemSet& ) = 0;

    bool                HasExchangeSupport() const { return bHasExchangeSupport; }
    void                SetExchangeSupport()       { bHasExchangeSupport = true; }

    virtual void        ActivatePage( const SfxItemSet& );
    virtual DeactivateRC DeactivatePage( SfxItemSet* pSet );
    virtual bool        QueryClose();

    void                StateChanged( StateChangedType nType ) override;
    void                DataChanged( const DataChangedEvent& rDCEvt ) override;
};

#endif // INCLUDED_CUI_SOURCE_INC_ICONCDLG_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
