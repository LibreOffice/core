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

#ifndef INCLUDED_SVX_SVDOPAGE_HXX
#define INCLUDED_SVX_SVDOPAGE_HXX

#include <svx/svdobj.hxx>
#include <svx/sdrpageuser.hxx>
#include <svx/svxdllapi.h>

class SVXCORE_DLLPUBLIC SdrPageObj final : public SdrObject, public sdr::PageUser
{
public:
    // this method is called from the destructor of the referenced page.
    // do all necessary action to forget the page. It is not necessary to call
    // RemovePageUser(), that is done from the destructor.
    virtual void PageInDestruction(const SdrPage& rPage) override;

private:
    // To make things more safe, remember the page, not a number
    SdrPage*                                mpShownPage;

    virtual std::unique_ptr<sdr::contact::ViewContact> CreateObjectSpecificViewContact() override;
    virtual std::unique_ptr<sdr::properties::BaseProperties> CreateObjectSpecificProperties() override;

    // protected destructor
    virtual ~SdrPageObj() override;

public:
    SdrPageObj(
        SdrModel& rSdrModel,
        SdrPage* pNewPage = nullptr);
    // Copy constructor
    SdrPageObj(SdrModel& rSdrModel, SdrPageObj const & rSource);
    SdrPageObj(
        SdrModel& rSdrModel,
        const tools::Rectangle& rRect,
        SdrPage* pNewPage = nullptr);

    SdrPage* GetReferencedPage() const { return mpShownPage;}
    void SetReferencedPage(SdrPage* pNewPage);

    // #i96598#
    virtual void SetBoundRectDirty() override;

    virtual SdrObjKind GetObjIdentifier() const override;
    virtual void TakeObjInfo(SdrObjTransformInfoRec& rInfo) const override;
    virtual SdrPageObj* CloneSdrObject(SdrModel& rTargetModel) const override;

    virtual OUString TakeObjNameSingul() const override;
    virtual OUString TakeObjNamePlural() const override;
};


#endif // INCLUDED_SVX_SVDOPAGE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
