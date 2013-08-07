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
#include <com/sun/star/beans/NamedValue.hpp>
#include <vcl/edit.hxx>
#include <vector>

namespace svx
{
    class OptHeaderTabListBox;
}
class CuiAboutConfigTabPage;
class CuiAboutConfigValueDialog;
struct Prop_Impl;

class CuiAboutConfigTabPage : public SfxTabPage
{
private:
    SvSimpleTableContainer* m_pPrefCtrl;
    PushButton* m_pDefaultBtn;
    PushButton* m_pEditBtn;

    //std::vector< com::sun::star::beans::NamedValue > VectorOfModified;
    std::vector< Prop_Impl* > VectorOfModified;

    ::svx::OptHeaderTabListBox* pPrefBox;
    CuiAboutConfigTabPage( Window* pParent, const SfxItemSet& rItemSet );
    ~CuiAboutConfigTabPage();
    //void AddToModifiedVector( com::sun::star::beans::NamedValue& rProp );
    void AddToModifiedVector( Prop_Impl* rProp );

    DECL_LINK( HeaderSelect_Impl, HeaderBar * );
    DECL_LINK( StandardHdl_Impl, void * );
public:
   static SfxTabPage* Create( Window* pParent, const SfxItemSet& rItemset );

   void     InsertEntry(OUString& rProp, OUString&  rStatus, OUString& rType, OUString& rValue);
   void     Reset( const SfxItemSet& );
   void     FillItems( com::sun::star::uno::Reference < com::sun::star::container::XNameAccess > xNameAccess, OUString sPath);
   com::sun::star::uno::Reference< com::sun::star::container::XNameAccess > getConfigAccess( OUString sNodePath, sal_Bool bUpdate );
   virtual sal_Bool FillItemSet( SfxItemSet& rSet );
};

class CuiAboutConfigValueDialog : public ModalDialog
{
private:
    OKButton*            m_pBtnOK;
    CancelButton*        m_pBtnCancel;

    VclMultiLineEdit*    m_pEDValue;

public:
    CuiAboutConfigValueDialog( Window* pWindow, const OUString& rValue );
    ~CuiAboutConfigValueDialog();

    OUString getValue()
    {
        return m_pEDValue->GetText();
    }
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
