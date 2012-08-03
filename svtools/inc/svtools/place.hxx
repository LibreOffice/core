/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef __SVTOOLS_PLACE_HXX__
#define __SVTOOLS_PLACE_HXX__

#include <tools/urlobj.hxx>

class Place
{
private:
    OUString msName;
    INetURLObject maUrl;

    bool mbEditable;

public:

    Place( OUString sName, rtl::OUString sUrl, bool bEditable = false ) :
        msName( sName ),
        maUrl( sUrl ),
        mbEditable( bEditable ) {};

    void SetName(const OUString& aName ) { msName = aName; }
    void SetUrl(const  OUString& aUrl ) { maUrl.SetURL( aUrl ); }

    OUString& GetName( ) { return msName; }
    OUString GetUrl( ) { return maUrl.GetMainURL( INetURLObject::NO_DECODE ); }
    INetURLObject& GetUrlObject( ) { return maUrl; }
    bool  IsLocal( ) { return maUrl.GetProtocol() == INET_PROT_FILE; }
    bool  IsEditable( ) { return mbEditable; }
};

#endif // __SVTOOLS_PLACE_HXX__

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
