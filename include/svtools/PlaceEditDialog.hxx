/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_SVTOOLS_PLACEEDITDIALOG_HXX
#define INCLUDED_SVTOOLS_PLACEEDITDIALOG_HXX

#include <svtools/ServerDetailsControls.hxx>

#include <vcl/button.hxx>
#include <vcl/dialog.hxx>
#include <vcl/edit.hxx>
#include <vcl/fixed.hxx>
#include <vcl/lstbox.hxx>

#include <svtools/inettbc.hxx>
#include <svtools/place.hxx>

#include <config_oauth2.h>

#include <memory>
#include <vector>

class SVT_DLLPUBLIC PlaceEditDialog : public ModalDialog
{
private:
    VclPtr<Edit>         m_pEDServerName;
    VclPtr<ListBox>      m_pLBServerType;
    std::shared_ptr< DetailsContainer > m_xCurrentDetails;

    VclPtr<Edit>         m_pEDUsername;
    VclPtr<CheckBox>     m_pCBPassword;
    VclPtr<Edit>         m_pEDPassword;
    VclPtr<FixedText>    m_pFTPasswordLabel;
    VclPtr<OKButton>     m_pBTOk;
    VclPtr<CancelButton> m_pBTCancel;

    VclPtr<PushButton>   m_pBTDelete;

    VclPtr<Button>       m_pBTRepoRefresh;

    VclPtr<VclGrid>      m_pTypeGrid;

    /** Vector holding the details UI control for each server type.

        The elements in this vector need to match the order in the type listbox, e.g.
        the m_aDetailsContainer[0] will be shown for the type corresponding to entry 0
        in the listbox.
      */
    std::vector< std::shared_ptr< DetailsContainer > > m_aDetailsContainers;

    sal_Int32 m_nCurrentType;

    bool bLabelChanged;
    bool m_bShowPassword;

public :

     PlaceEditDialog( vcl::Window* pParent);
     PlaceEditDialog(vcl::Window* pParent, const std::shared_ptr<Place> &rPlace );
     virtual ~PlaceEditDialog();
     virtual void dispose() SAL_OVERRIDE;

     // Returns a place instance with given information
     std::shared_ptr<Place> GetPlace();

     OUString GetServerName() { return m_pEDServerName->GetText(); }
     OUString GetServerUrl();
     OUString GetPassword() { return m_pEDPassword->GetText(); };
     OUString GetUser() { return m_pEDUsername->GetText(); };
     bool     IsRememberChecked() { return m_pCBPassword->IsChecked(); }

     void ShowPasswordControl( bool bShow = true ) { m_bShowPassword = bShow; }

private:

    void InitDetails( );
    void UpdateLabel( );

    DECL_LINK_TYPED ( OKHdl, Button *, void );
    DECL_LINK_TYPED ( DelHdl, Button *, void );
    DECL_LINK_TYPED ( EditHdl, DetailsContainer*, void );
    DECL_LINK ( ModifyHdl, void* );
    DECL_LINK_TYPED ( SelectTypeHdl, ListBox&, void );
    DECL_LINK ( EditLabelHdl, void * );
    DECL_LINK ( EditUsernameHdl, void * );

};

#endif // INCLUDED_SVTOOLS_PLACEEDITDIALOG_HXX
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
