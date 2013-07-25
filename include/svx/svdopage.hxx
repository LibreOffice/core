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

#ifndef _SVDOPAGE_HXX
#define _SVDOPAGE_HXX

#include <svx/svdobj.hxx>
#include <svx/sdrpageuser.hxx>
#include "svx/svxdllapi.h"

class SVX_DLLPUBLIC SdrPageObj : public SdrObject, public sdr::PageUser
{
public:
    // this method is called form the destructor of the referenced page.
    // do all necessary action to forget the page. It is not necessary to call
    // RemovePageUser(), that is done form the destructor.
    virtual void PageInDestruction(const SdrPage& rPage);

private:
    // #111111#
    // To make things more safe, remember the page, not a number
    SdrPage*                                mpShownPage;

protected:
    virtual sdr::contact::ViewContact* CreateObjectSpecificViewContact();
    virtual sdr::properties::BaseProperties* CreateObjectSpecificProperties();

public:
    TYPEINFO();
    SdrPageObj(SdrPage* pNewPage = 0L);
    SdrPageObj(const Rectangle& rRect, SdrPage* pNewPage = 0L);
    ~SdrPageObj();

    // #111111#
    SdrPage* GetReferencedPage() const;
    void SetReferencedPage(SdrPage* pNewPage);

    // #i96598#
    virtual void SetBoundRectDirty();

    virtual sal_uInt16 GetObjIdentifier() const;
    virtual void TakeObjInfo(SdrObjTransformInfoRec& rInfo) const;
    virtual SdrPageObj* Clone() const;
    SdrPageObj& operator=(const SdrPageObj& rObj);

    virtual OUString TakeObjNameSingul() const;
    virtual void TakeObjNamePlural(String& rName) const;
};

////////////////////////////////////////////////////////////////////////////////////////////////////

#endif //_SVDOPAGE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
