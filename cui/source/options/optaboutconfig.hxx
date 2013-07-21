/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_CUI_OPTABOUTCONFIG_HXX
#define INCLUDED_CUI_OPTABOUTCONFIG_HXX

#include <sfx2/tabdlg.hxx>
#include <svtools/simptabl.hxx>
#include "optHeaderTabListbox.hxx"

namespace svx
{
    class OptHeaderTabListBox;
}
struct CuiAboutConfigTabPage_Impl;

class CuiAboutConfigTabPage : public SfxTabPage
{
private:
    SvxSimpleTableContainer* m_pPrefCtrl;
    PushButton* m_pDefaultBtn;
    PushButton* m_pEditBtn;

    ::svx::OptHeaderTabListBox* pPrefBox;

    CuiAboutConfigTabPage_Impl* m_pImpl;
public:
   CuiAboutConfigTabPage( Window* pParent, const SfxItemSet& rItemSet );
   ~CuiAboutConfigTabPage();

   static SfxTabPage* Create( Window* pParent, const SfxItemSet& rItemset );

};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
