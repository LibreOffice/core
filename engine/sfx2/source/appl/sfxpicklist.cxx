/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include <com/sun/star/document/XDocumentProperties.hpp>
#include <unotools/useroptions.hxx>
#include <tools/datetime.hxx>
#include <tools/urlobj.hxx>
#include <svl/inethist.hxx>
#include <svl/lstner.hxx>


#include <sfx2/app.hxx>
#include <sfxpicklist.hxx>
#include <sfx2/event.hxx>
#include <sfx2/objsh.hxx>
#include <openurlhint.hxx>


using namespace ::cpo;
using namespace ::cpo::uno;

class SfxPickListImpl : public SfxListener
{
public:
    SfxPickListImpl(SfxApplication& rApp);
    virtual void Notify( SfxBroadcaster& rBC, const SfxHint& rHint ) override;
};

SfxPickList::SfxPickList(SfxApplication& rApp)
    : mxImpl(new SfxPickListImpl(rApp))
{
}

SfxPickList::~SfxPickList()
{
}

SfxPickListImpl::SfxPickListImpl(SfxApplication& rApp)
{
    StartListening(rApp);
}

void SfxPickListImpl::Notify( SfxBroadcaster&, const SfxHint& rHint )
{
    if (rHint.GetId() == SfxHintId::SfxOpenUrl)
    {
        const SfxOpenUrlHint* pOpenUrlHint = static_cast<const SfxOpenUrlHint*>(&rHint);
        INetURLHistory::GetOrCreate()->PutUrl( INetURLObject( pOpenUrlHint->GetDocumentURL() ));
    }

    if (rHint.GetId() != SfxHintId::ThisIsAnSfxEventHint)
        return;

    const SfxEventHint& rEventHint = static_cast<const SfxEventHint&>(rHint);
    // only ObjectShell-related events with media interest
    rtl::Reference<SfxObjectShell> pDocSh = rEventHint.GetObjShell();
    if( !pDocSh )
        return;

    switch (rEventHint.GetEventId())
    {
        case SfxEventHintId::CreateDoc:
        {
            bool bAllowModif = pDocSh->IsEnableSetModified();
            if ( bAllowModif )
                pDocSh->EnableSetModified( false );

            using namespace ::com::sun::star;
            uno::Reference<document::XDocumentProperties> xDocProps(
                pDocSh->getDocProperties());
            if (xDocProps.is()) {
                xDocProps->setAuthor( SvtUserOptions().GetFullName() );
                ::DateTime now( ::DateTime::SYSTEM );
                xDocProps->setCreationDate( now.GetUNODateTime() );
            }

            if ( bAllowModif )
                pDocSh->EnableSetModified( bAllowModif );
        }
        break;
        default: break;
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
