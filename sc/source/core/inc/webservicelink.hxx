/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_SC_SOURCE_CORE_INC_WEBSERVICE_HXX
#define INCLUDED_SC_SOURCE_CORE_INC_WEBSERVICE_HXX

#include <sfx2/lnkbase.hxx>
#include <svl/broadcast.hxx>

namespace com::sun::star::uno
{
class Any;
}

class ScDocument;

class ScWebServiceLink final : public ::sfx2::SvBaseLink, public SvtBroadcaster
{
private:
    ScDocument* pDoc;
    OUString aURL; // connection/ link data
    bool bHasResult; // is set aResult is useful
    OUString aResult;

public:
    ScWebServiceLink(ScDocument* pD, const OUString& rURL);
    virtual ~ScWebServiceLink() override;

    // SvBaseLink override:
    virtual ::sfx2::SvBaseLink::UpdateResult DataChanged(const OUString& rMimeType,
                                                         const css::uno::Any& rValue) override;

    // SvtBroadcaster override:
    virtual void ListenersGone() override;

    // for interpreter:

    const OUString& GetResult() const { return aResult; }
    bool HasResult() const { return bHasResult; }

    const OUString& GetURL() const { return aURL; }
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
