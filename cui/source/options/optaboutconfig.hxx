/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_CUI_SOURCE_OPTIONS_OPTABOUTCONFIG_HXX
#define INCLUDED_CUI_SOURCE_OPTIONS_OPTABOUTCONFIG_HXX

#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/beans/NamedValue.hpp>

#include <sfx2/tabdlg.hxx>
#include <svtools/simptabl.hxx>
#include <vcl/edit.hxx>

#include <vector>
#include <boost/scoped_ptr.hpp>
#include <boost/shared_ptr.hpp>

namespace svx { class OptHeaderTabListBox; }
class CuiAboutConfigTabPage;
class CuiAboutConfigValueDialog;
struct Prop_Impl;

class CuiCustomMultilineEdit : public Edit
{
public:
    bool bNumericOnly;
    CuiCustomMultilineEdit( Window* pParent, WinBits nStyle )
        : Edit( pParent, nStyle )
        , bNumericOnly(false)
    {}

    virtual void KeyInput( const KeyEvent& rKeyEvent );
    virtual Size GetOptimalSize() const;
    //void setBehaviour( bool bNumeric, int nLengthLimit);
};

class CuiAboutConfigTabPage : public ModalDialog
{
private:
    SvSimpleTableContainer* m_pPrefCtrl;
    PushButton* m_pResetBtn;
    PushButton* m_pEditBtn;

    std::vector< boost::shared_ptr< Prop_Impl > > m_vectorOfModified;
    boost::scoped_ptr< svx::OptHeaderTabListBox > m_pPrefBox;

    void AddToModifiedVector( const boost::shared_ptr< Prop_Impl >& rProp );
    std::vector< OUString > commaStringToSequence( const OUString& rCommaSepString );

    DECL_LINK( StandardHdl_Impl, void * );
    DECL_LINK( ResetBtnHdl_Impl, void * );

public:
   //static ModalDialog* Create( Window* pParent, const SfxItemSet& rItemset );
   CuiAboutConfigTabPage( Window* pParent/*, const SfxItemSet& rItemSet*/ );
   void     InsertEntry(const OUString& rProp, const OUString& rStatus, const OUString& rType, const OUString& rValue);
   void     Reset(/* const SfxItemSet&*/ );
   void     FillItems(const com::sun::star::uno::Reference<com::sun::star::container::XNameAccess>& xNameAccess);
   com::sun::star::uno::Reference< com::sun::star::container::XNameAccess > getConfigAccess( const OUString& sNodePath, sal_Bool bUpdate );
   virtual sal_Bool FillItemSet( /* SfxItemSet& rSet*/ );

   virtual Size GetOptimalSize() const;
};

class CuiAboutConfigValueDialog : public ModalDialog
{
private:
    CuiCustomMultilineEdit* m_pEDValue;

public:
    CuiAboutConfigValueDialog( Window* pWindow, const OUString& rValue , int limit = 0);

    OUString getValue()
    {
        return m_pEDValue->GetText();
    }
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
