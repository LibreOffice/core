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

#include <svtools/svtdllapi.h>
#include <vcl/weld.hxx>

#include <memory>
#include <vector>

class Place;
class DetailsContainer;

class SVT_DLLPUBLIC PlaceEditDialog : public weld::GenericDialogController
{
private:
    std::shared_ptr< DetailsContainer > m_xCurrentDetails;
    /** Vector holding the details UI control for each server type.

        The elements in this vector need to match the order in the type listbox, e.g.
        the m_aDetailsContainer[0] will be shown for the type corresponding to entry 0
        in the listbox.
      */
    std::vector< std::shared_ptr< DetailsContainer > > m_aDetailsContainers;

    sal_Int32 m_nCurrentType;

    bool m_bLabelChanged;
    bool m_bShowPassword;

public:
    std::unique_ptr<weld::Entry> m_xEDServerName;
    std::unique_ptr<weld::ComboBox> m_xLBServerType;
    std::unique_ptr<weld::Entry> m_xEDUsername;
    std::unique_ptr<weld::Label> m_xFTUsernameLabel;
    std::unique_ptr<weld::Button> m_xBTOk;
    std::unique_ptr<weld::Button> m_xBTCancel;
    std::unique_ptr<weld::Button> m_xBTDelete;
    std::unique_ptr<weld::Button> m_xBTRepoRefresh;
    std::unique_ptr<weld::CheckButton> m_xCBPassword;
    std::unique_ptr<weld::Entry> m_xEDPassword;
    std::unique_ptr<weld::Label> m_xFTPasswordLabel;
    std::unique_ptr<weld::Widget> m_xTypeGrid;

    std::unique_ptr<weld::Widget> m_xRepositoryBox;
    std::unique_ptr<weld::Label> m_xFTRepository;
    std::unique_ptr<weld::ComboBox> m_xLBRepository;

    std::unique_ptr<weld::Entry> m_xEDShare;
    std::unique_ptr<weld::Label> m_xFTShare;

    std::unique_ptr<weld::Widget> m_xDetailsGrid;
    std::unique_ptr<weld::Widget> m_xHostBox;
    std::unique_ptr<weld::Entry> m_xEDHost;
    std::unique_ptr<weld::Label> m_xFTHost;
    std::unique_ptr<weld::SpinButton> m_xEDPort;
    std::unique_ptr<weld::Label> m_xFTPort;
    std::unique_ptr<weld::Entry> m_xEDRoot;
    std::unique_ptr<weld::Label> m_xFTRoot;

    std::unique_ptr<weld::CheckButton> m_xCBDavs;

public:
    PlaceEditDialog(weld::Window* pParent);
    PlaceEditDialog(weld::Window* pParent, const std::shared_ptr<Place> &rPlace );
    virtual ~PlaceEditDialog() override;

    // Returns a place instance with given information
    std::shared_ptr<Place> GetPlace();

    OUString GetServerName() { return m_xEDServerName->get_text(); }
    OUString GetServerUrl();
    OUString GetPassword() { return m_xEDPassword->get_text(); };
    OUString GetUser() { return m_xEDUsername->get_text(); };
    bool     IsRememberChecked() { return m_xCBPassword->get_active(); }

    void ShowPasswordControl() { m_bShowPassword = true; }

private:

    void InitDetails( );

    DECL_LINK ( OKHdl, weld::Button&, void );
    DECL_LINK ( DelHdl, weld::Button&, void );
    DECL_LINK ( EditHdl, DetailsContainer*, void );
    DECL_LINK ( ModifyHdl, weld::Entry&, void );
    void SelectType(bool bSkipSeparator);
    DECL_LINK ( SelectTypeHdl, weld::ComboBox&, void );
    DECL_LINK ( EditLabelHdl, weld::Entry&, void );
    DECL_LINK ( EditUsernameHdl, weld::Entry&, void );

};

#endif // INCLUDED_SVTOOLS_PLACEEDITDIALOG_HXX
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
