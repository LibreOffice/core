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

#include <sal/config.h>

#include <string_view>

#include <svtools/tabbar.hxx>
#include <vcl/transfer.hxx>
#include <sddllapi.h>

namespace sd {

/**
 * TabBar for layer administration
 */
class DrawViewShell;

class SAL_DLLPUBLIC_RTTI LayerTabBar final
    : public TabBar,
      public DropTargetHelper
{
public:
    LayerTabBar (
        DrawViewShell* pDrViewSh,
        vcl::Window* pParent);
    virtual void dispose() override;
    virtual ~LayerTabBar() override;

    /** Inform all listeners of this control that the current layer has been
        activated.  Call this method after switching the current layer and is
        not done elsewhere (like when using ctrl + page up/down keys).
    */
    void SendActivatePageEvent();

    /** Inform all listeners of this control that the current layer has been
        deactivated.  Call this method before switching the current layer
        and is not done elsewhere (like when using ctrl page up/down keys).
    */
    void SendDeactivatePageEvent();

    // Expects not-localized, real layer name in rText. Generates a localized layer name
    // that will be displayed on the tab of the LayerTabBar and writes the real name
    // to maAuxiliaryText. In case you want no entry in maAuxiliaryText, use method from TabBar.
    virtual void        InsertPage( sal_uInt16 nPageId, const OUString& rText,
                                TabBarPageBits nBits = TabBarPageBits::NONE,
                                sal_uInt16 nPos = TabBar::APPEND ) override;
    virtual void        SetPageText( sal_uInt16 nPageId, const OUString& rText ) override;

    // Returns the real layer name if exists and empty OUString otherwise.
    OUString            GetLayerName(sal_uInt16 nPageId) const;

    // Used e.g. in DeleteActualLayer() to test whether deleting is allowed.
    static bool IsRealNameOfStandardLayer(std::u16string_view rName);

    // Used e.g. in validity test of user entered names
    static bool IsLocalizedNameOfStandardLayer(std::u16string_view rName);

    // In case rName is one of the sUNO_LayerName_*, it generates a localized name,
    // otherwise it returns value of rName.
    static OUString convertToLocalizedName(const OUString& rName);

    // TabBar
    virtual void        Select() override;
    virtual void        DoubleClick() override;

    SD_DLLPUBLIC virtual void MouseButtonDown(const MouseEvent& rMEvt) override; // export for unit test

    virtual void        Command(const CommandEvent& rCEvt) override;

    virtual bool        StartRenaming() override;
    virtual TabBarAllowRenamingReturnCode  AllowRenaming() override;
    virtual void        EndRenaming() override;

    virtual void        ActivatePage() override;

    // DropTargetHelper
    virtual sal_Int8    AcceptDrop( const AcceptDropEvent& rEvt ) override;
    virtual sal_Int8    ExecuteDrop( const ExecuteDropEvent& rEvt ) override;

private:
    DrawViewShell* pDrViewSh;

    // Expects not-localized, real layer name in rText and writes it to maAuxiliaryText.
    void SetLayerName( sal_uInt16 nPageId, const OUString& rText );
};

} // end of namespace sd

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
