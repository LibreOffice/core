/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
* This file is part of the LibreOffice project.
*
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

#include "slidehack.hxx"

namespace SlideHack {
namespace {

class ImplStore : public Store {
public:
    ImplStore();

    virtual sal_uInt32 search( OUString aSearchEntry )
    {
        // queue maSearchCompleted at idle ...
        return 0;
    }
    virtual void  cancelSearch( sal_uInt32 nHandle ) { (void) nHandle; }

    /// used to create a group handle from a stored slide, so we can
    /// check for updated versions etc.
    virtual GroupPtr lookupGroup( OriginPtr pOrigin )
    {
        return GroupPtr();
    }
    virtual GroupPtr createGroup( const OUString &rName,
                                  const OUString &rTitle,
                                  const OUString &rKeywords,
                                  const std::vector< SdPage * > &rPages )
    {
        return GroupPtr();
    }
};

} // end anonymous namespace

StorePtr Store::getStore()
{
    return StorePtr( new ImplStore() );
}

} // end SlideHack namespace

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
