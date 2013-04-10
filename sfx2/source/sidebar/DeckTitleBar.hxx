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
#ifndef SFX_SIDEBAR_DECK_TITLE_BAR_HXX
#define SFX_SIDEBAR_DECK_TITLE_BAR_HXX

#include "TitleBar.hxx"

#include <boost/function.hpp>


namespace sfx2 { namespace sidebar {

class DeckTitleBar
    : public TitleBar
{
public:
    DeckTitleBar (
        const ::rtl::OUString& rsTitle,
        Window* pParentWindow,
        const ::boost::function<void(void)>& rCloserAction);
    virtual ~DeckTitleBar (void);

    void SetCloserVisible (const bool bIsCloserVisible);

    virtual void DataChanged (const DataChangedEvent& rEvent);

protected:
    virtual Rectangle GetTitleArea (const Rectangle& rTitleBarBox);
    virtual void PaintDecoration (const Rectangle& rTitleBarBox);
    virtual sidebar::Paint GetBackgroundPaint (void);
    virtual Color GetTextColor (void);
    virtual void HandleToolBoxItemClick (const sal_uInt16 nItemIndex);

private:
    const sal_uInt16 mnCloserItemIndex;
    const ::boost::function<void(void)> maCloserAction;
    bool mbIsCloserVisible;
};

} } // end of namespace sfx2::sidebar

#endif
