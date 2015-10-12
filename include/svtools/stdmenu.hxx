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

#ifndef INCLUDED_SVTOOLS_STDMENU_HXX
#define INCLUDED_SVTOOLS_STDMENU_HXX

#include <svtools/svtdllapi.h>
#include <tools/link.hxx>
#include <vcl/menu.hxx>

class FontList;
namespace vcl {
    class FontInfo;
}

/*************************************************************************

Description
============

class FontNameMenu

Description

Allows the selection of fonts. The menu is filled with the FontNames by
the method Fill(). Fill() automatically sorts the FontNames (incl. all
umlauts and language-dependent). Using SetCurName()/GetCurName(), the
current FontName can be set/requested. If SetCurName() is called with
an empty string, no entry is displayed as the current one.
Before the Select call, the selected name is automatically set as the
current one and would be displayed as the current name on the next call.
Because of this, the current FontName should be set using SetCurName()
if necessary before calling PopupMenu::Execute().

As the Ids and the internal structure of the menu are not known, a
Select handler must be set in order to notice the selection of a name.

No further items can be inserted into that menu.

Later on, the menu shall display the same bitmaps as the FontNameBox.
On many systems, where menues don't scroll automatically, an A-Z menu
might be interposed. As a menu has always needed long when many Fonts
are installed, this menu should only be generated once.

References

FontList; FontSizeMenu; FontNameBox

--------------------------------------------------------------------------

class FontSizeMenu

Description

Allows the selection of font sizes. The FontSizeMenu is filled using
Fill(). The selected font size can be queried using GetCurHeight().
Using SetCurHeight()/GetCurHeight(), the current font size can be set/
requested. If SetCurHeight() is called with 0, no entry is displayed
as the current one.
Before the Select call, the selected size is automatically set as the
current one and would be displayed as the current size on the next call.
Because of this, the current font size should be set using SetCurHeight()
if necessary before calling PopupMenu::Execute(). As the font sizes
depend on the selected Font, the Menu should be re-filled with the
sizes of the font using Fill() after a change to the font name.

As the Ids and the internal structure of the menu are not known, a
Select handler must be set in order to notice the selection of a size.

All sizes are given in 1/10 points.

No further items can be inserted into that menu.

Later on, the menu shall display the sizes in respect to the system.
For example, Mac could possibly show an outline of the fonts that are
available as bitmaps.

References

FontList; FontNameMenu; FontSizeBox

*************************************************************************/

class SVT_DLLPUBLIC FontNameMenu : public PopupMenu
{
private:
    OUString                     maCurName;
    Link<FontNameMenu*,void>     maSelectHdl;

public:
                    FontNameMenu();
    virtual         ~FontNameMenu();

    virtual void    Select() override;
    virtual void    Highlight() override;

    void            Fill( const FontList* pList );

    void            SetCurName( const OUString& rName );
    const OUString& GetCurName() const { return maCurName; }

    void            SetSelectHdl( const Link<FontNameMenu*,void>& rLink ) { maSelectHdl = rLink; }
};

class SVT_DLLPUBLIC FontSizeMenu : public PopupMenu
{
private:
    long*                     mpHeightAry;
    long                      mnCurHeight;
    Link<FontSizeMenu*,void>  maSelectHdl;

public:
                    FontSizeMenu();
                    virtual ~FontSizeMenu();

    virtual void    Select() override;
    virtual void    Highlight() override;

    void            Fill( const vcl::FontInfo& rInfo, const FontList* pList );

    void            SetCurHeight( long nHeight );
    long            GetCurHeight() const { return mnCurHeight; }

    void            SetSelectHdl( const Link<FontSizeMenu*,void>& rLink ) { maSelectHdl = rLink; }
};

#endif // INCLUDED_SVTOOLS_STDMENU_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
