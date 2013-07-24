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
#include <com/sun/star/container/XNameAccess.hpp>

namespace svx
{
    class OptHeaderTabListBox;
}
class CuiAboutConfigTabPage;

class CuiAboutConfigTabPage : public SfxTabPage
{
private:
    SvxSimpleTableContainer* m_pPrefCtrl;
    PushButton* m_pDefaultBtn;
    PushButton* m_pEditBtn;

    ::svx::OptHeaderTabListBox* pPrefBox;
    CuiAboutConfigTabPage( Window* pParent, const SfxItemSet& rItemSet );
    ~CuiAboutConfigTabPage();
public:
   static SfxTabPage* Create( Window* pParent, const SfxItemSet& rItemset );

   void     InsertEntry(OUString& rProp, OUString&  rStatus, OUString& rType, OUString& rValue);
   void     Reset( const SfxItemSet& );
   void     FillItems( com::sun::star::uno::Reference < com::sun::star::container::XNameAccess > xNameAccess, OUString sPath);
   com::sun::star::uno::Reference< com::sun::star::container::XNameAccess > getConfigAccess( OUString sNodePath );

};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
