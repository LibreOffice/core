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

#include <boost/shared_ptr.hpp>
#include <vector>

class SVT_DLLPUBLIC PlaceEditDialog : public ModalDialog
{
private :

    Edit*      m_pEDServerName;
    ListBox*   m_pLBServerType;
    boost::shared_ptr< DetailsContainer > m_pCurrentDetails;

    Edit*         m_pEDUsername;
    OKButton*     m_pBTOk;
    CancelButton* m_pBTCancel;

    PushButton*   m_pBTDelete;

    /** Vector holding the details UI control for each server type.

        The elements in this vector need to match the order in the type listbox, e.g.
        the m_aDetailsContainer[0] will be shown for the type corresponding to entry 0
        in the listbox.
      */
    std::vector< boost::shared_ptr< DetailsContainer > > m_aDetailsContainers;

public :

     PlaceEditDialog( Window* pParent);
     PlaceEditDialog(Window* pParent, const boost::shared_ptr<Place> &pPlace );
     virtual ~PlaceEditDialog();

     // Returns a place instance with given information
     boost::shared_ptr<Place> GetPlace();

    OUString GetServerName() { return m_pEDServerName->GetText(); }
     OUString GetServerUrl();

private:

    void InitDetails( );

    DECL_LINK ( OKHdl, Button * );
    DECL_LINK ( DelHdl, Button * );
    DECL_LINK ( EditHdl, void * );
    DECL_LINK ( SelectTypeHdl, void * );
    DECL_LINK ( EditUsernameHdl, void * );

};

#endif // INCLUDED_SVTOOLS_PLACEEDITDIALOG_HXX
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
