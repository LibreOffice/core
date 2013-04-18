/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * Version: MPL 1.1 / GPLv3+ / LGPLv3+
 *
 * The contents of this file are subject to the Mozilla Public License Version
 * 1.1 (the "License"); you may not use this file except in compliance with
 * the License or as specified alternatively below. You may obtain a copy of
 * the License at http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * Major Contributor(s):
 * [ Copyright (C) 2012 Julien Levesy <jlevesy@gmail.com> (initial developer) ]
 *
 * All Rights Reserved.
 *
 * For minor contributions see the git repository.
 *
 * Alternatively, the contents of this file may be used under the terms of
 * either the GNU General Public License Version 3 or later (the "GPLv3+"), or
 * the GNU Lesser General Public License Version 3 or later (the "LGPLv3+"),
 * in which case the provisions of the GPLv3+ or the LGPLv3+ are applicable
 * instead of those above.
 */
#ifndef _PLACEEDITDIALOG_HXX
#define _PLACEEDITDIALOG_HXX

#include "ServerDetailsControls.hxx"

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
     ~PlaceEditDialog();

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

#endif //_PLACEEDITDIALOG_HXX
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
