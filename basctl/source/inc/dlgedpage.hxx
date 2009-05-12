/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: dlgedpage.hxx,v $
 * $Revision: 1.6 $
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#ifndef _BASCTL_DLGEDPAGE_HXX
#define _BASCTL_DLGEDPAGE_HXX

#include "svx/svdpage.hxx"

//============================================================================
// DlgEdPage
//============================================================================

class DlgEdModel;
class DlgEdForm;

class DlgEdPage : public SdrPage
{
private:
    DlgEdForm*      pDlgEdForm;

public:
    TYPEINFO();

    DlgEdPage( DlgEdModel& rModel, FASTBOOL bMasterPage=FALSE );
    DlgEdPage( const DlgEdPage& );
    virtual ~DlgEdPage();

    using SdrPage::Clone;
    virtual SdrPage* Clone() const;

    void            SetDlgEdForm( DlgEdForm* pForm ) { pDlgEdForm = pForm; }
    DlgEdForm*      GetDlgEdForm() const { return pDlgEdForm; }

    virtual SdrObject* SetObjectOrdNum(ULONG nOldObjNum, ULONG nNewObjNum);
};

#endif //_BASCTL_DLGEDPAGE_HXX
