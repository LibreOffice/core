/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_CUI_OPTABOUTCONFIG_HXX
#define INCLUDED_CUI_OPTBABOUTCONFIG_HXX

#include <svx/simpltabl.hxx>

namespace cui
{
    class OptHeaderTabListBox;
}


class CuiAboutConfigTabPage : public SfxTabPage
{
private:
    SvxSimpleTableContainer* m_pPrefCtrl;
    //In case of tab page we can think a revert button
    //PushButton* m_pDefaultBtn
    PushButton* m_pEditBtn;

    ::cui::OptHeaderTabListBox* pPrefBox;
public:
   CuiAboutConfigTabPage( Window* pParent, const SfxItemSet& rItemSet );
   ~CuiAboutConfigTabPage();

   static SfxTabPage* create( Window* pParent, const SfxItemSet& rItemset );

}
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
