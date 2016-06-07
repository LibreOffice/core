/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4     -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_SFX2_TEMPLATEDEFAULTVIEW_HXX
#define INCLUDED_SFX2_TEMPLATEDEFAULTVIEW_HXX

#include <sfx2/templatelocalview.hxx>

class SFX2_DLLPUBLIC TemplateDefaultView : public TemplateLocalView
{
public:
    TemplateDefaultView(Window *pParent);

    virtual void reload() override;

    virtual void showAllTemplates () override;

    virtual void KeyInput( const KeyEvent& rKEvt ) override;

    void createContextMenu();

    DECL_LINK_TYPED(ContextMenuSelectHdl, Menu*, void);
protected:
    long    mnItemMaxSize;
    long    mnTextHeight;
    long    mnItemPadding;
};

#endif // INCLUDED_SFX2_TEMPLATEDEFAULTVIEW_HXX
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
