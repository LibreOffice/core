/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
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

#ifndef INCLUDED_SVX_SDRMASTERPAGEDESCRIPTOR_HXX
#define INCLUDED_SVX_SDRMASTERPAGEDESCRIPTOR_HXX

#include <svx/sdrpageuser.hxx>
#include <svx/svdsob.hxx>
#include <svx/svxdllapi.h>
#include <memory>

class SdrPageProperties;
namespace sdr::contact { class ViewContact; }


namespace sdr
{
    class SVXCORE_DLLPUBLIC MasterPageDescriptor final : public sdr::PageUser
    {
    private:
        SdrPage&                                        maOwnerPage;
        SdrPage&                                        maUsedPage;
        SdrLayerIDSet                                       maVisibleLayers;

        // ViewContact part
        mutable std::unique_ptr<sdr::contact::ViewContact>      mpViewContact;

        void operator=(const MasterPageDescriptor& rCandidate) = delete;

    public:
        MasterPageDescriptor(SdrPage& aOwnerPage, SdrPage& aUsedPage);
        virtual ~MasterPageDescriptor();

        // ViewContact part
        sdr::contact::ViewContact& GetViewContact() const;

        // this method is called from the destructor of the referenced page.
        // do all necessary action to forget the page. It is not necessary to call
        // RemovePageUser(), that is done from the destructor.
        virtual void PageInDestruction(const SdrPage& rPage) override;

        // member access to UsedPage
        SdrPage& GetUsedPage() const { return maUsedPage; }

        // member access to OwnerPage
        SdrPage& GetOwnerPage() const { return maOwnerPage; }

        // member access to VisibleLayers
        const SdrLayerIDSet& GetVisibleLayers() const { return maVisibleLayers; }
        void SetVisibleLayers(const SdrLayerIDSet& rNew);

        const SdrPageProperties* getCorrectSdrPageProperties() const;
    };
} // end of namespace sdr

#endif // INCLUDED_SVX_SDRMASTERPAGEDESCRIPTOR_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
