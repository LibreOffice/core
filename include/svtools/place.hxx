/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_SVTOOLS_PLACE_HXX
#define INCLUDED_SVTOOLS_PLACE_HXX

#include <tools/urlobj.hxx>

class Place
{
private:
    OUString msName;
    INetURLObject maUrl;

    bool const mbEditable;

public:

    Place( const OUString& sName, const OUString& sUrl, bool bEditable = false ) :
        msName( sName ),
        maUrl( sUrl ),
        mbEditable( bEditable ) {};

    void SetName(const OUString& aName ) { msName = aName; }
    void SetUrl(const  OUString& aUrl ) { maUrl.SetURL( aUrl ); }

    OUString& GetName( ) { return msName; }
    OUString GetUrl( ) const { return maUrl.GetMainURL( INetURLObject::DecodeMechanism::NONE ); }
    INetURLObject& GetUrlObject( ) { return maUrl; }
    bool  IsLocal( ) const { return maUrl.GetProtocol() == INetProtocol::File; }
    bool  IsEditable( ) const { return mbEditable; }
};

#endif // INCLUDED_SVTOOLS_PLACE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
