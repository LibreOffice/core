/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4     -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_SFX2_SOURCE_INC_BACKINGWINDOWSEARCHVIEW_HXX
#define INCLUDED_SFX2_SOURCE_INC_BACKINGWINDOWSEARCHVIEW_HXX

#include "templatesearchview.hxx"
class PopupMenu;

class SFX2_DLLPUBLIC BackingWindowTemplateSearchView : public TemplateSearchView
{
public:
    BackingWindowTemplateSearchView(Window *pParent);

    virtual void KeyInput( const KeyEvent& rKEvt ) override;

    void createContextMenu();

    void AppendItem(sal_uInt16 nAssocItemId, sal_uInt16 nRegionId, sal_uInt16 nIdx,
                    const OUString &rTitle, const OUString &rSubtitle,
                    const OUString &rPath, const BitmapEx &rImage );

    DECL_LINK_TYPED(ContextMenuSelectHdl, Menu*, bool);
protected:
    long    mnItemMaxSize;
    long    mnTextHeight;
    long    mnItemPadding;
};

#endif // INCLUDED_SFX2_SOURCE_INC_BACKINGWINDOWSEARCHVIEW_HXX
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
