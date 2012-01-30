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
#ifndef _SVTPLACEDIALOG_HXX
#define _SVTPLACEDIALOG_HXX

#include <vcl/button.hxx>
#include <vcl/dialog.hxx>
#include <vcl/fixed.hxx>
#include <vcl/edit.hxx>

#include <svtools/inettbc.hxx>

#include <svl/restrictedpaths.hxx>

class Place;
class SvtPlaceDialog : public ModalDialog
{
private :

    FixedText m_aFTServerUrl;
    FixedText m_aFTServerName;
    FixedText m_aFTServerType;
    FixedText m_aFTServerLogin;
    FixedText m_aFTServerPassword;

    SvtURLBox m_aEDServerUrl;

    Edit m_aEDServerName;
    Edit m_aEDServerType;
    Edit m_aEDServerLogin;
    Edit m_aEDServerPassword;

    OKButton m_aBTOk;
    CancelButton m_aBTCancel;

    PushButton m_aBTDelete;

	::svt::RestrictedPaths m_UrlFilter;

	DECL_LINK( OKHdl, Button *);
	DECL_LINK ( DelHdl, Button *);

	DECL_LINK ( EditHdl, Edit *);

public :

     SvtPlaceDialog( Window* pParent);
     SvtPlaceDialog( Window* pParent, PlacePtr pPlace );
     ~SvtPlaceDialog();

     // Returns a place instance with given informations
     PlacePtr GetPlace();

     rtl::OUString GetServerName() 	{ return m_aEDServerName.GetText(); }
     rtl::OUString GetServerUrl() 	{ return m_aEDServerUrl.GetText(); }

};

#endif //_SVTPLACEDIALOG_HXX
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
