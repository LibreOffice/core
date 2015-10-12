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

#ifndef INCLUDED_BASCTL_SOURCE_INC_DLGEDPAGE_HXX
#define INCLUDED_BASCTL_SOURCE_INC_DLGEDPAGE_HXX

#include <svx/svdpage.hxx>

namespace basctl
{


// DlgEdPage


class DlgEdModel;
class DlgEdForm;

class DlgEdPage : public SdrPage
{
    DlgEdPage& operator=(const DlgEdPage&) = delete;

private:
    DlgEdForm*      pDlgEdForm;

public:
    TYPEINFO_OVERRIDE();

    explicit DlgEdPage( DlgEdModel& rModel, bool bMasterPage = false );
    virtual ~DlgEdPage();

    virtual SdrPage* Clone() const SAL_OVERRIDE;
    virtual SdrPage* Clone( SdrModel* pNewModel ) const SAL_OVERRIDE;

    void            SetDlgEdForm( DlgEdForm* pForm ) { pDlgEdForm = pForm; }
    DlgEdForm*      GetDlgEdForm() const { return pDlgEdForm; }

    virtual SdrObject* SetObjectOrdNum(size_t nOldObjNum, size_t nNewObjNum) SAL_OVERRIDE;

protected:
    DlgEdPage(const DlgEdPage& rSrcPage);
};

} // namespace basctl

#endif // INCLUDED_BASCTL_SOURCE_INC_DLGEDPAGE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
