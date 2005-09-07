/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: dlgedpage.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 20:20:48 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#ifndef _BASCTL_DLGEDPAGE_HXX
#define _BASCTL_DLGEDPAGE_HXX

#ifndef _SVDPAGE_HXX
#include "svx/svdpage.hxx"
#endif

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

    virtual SdrPage* Clone() const;

    void            SetDlgEdForm( DlgEdForm* pForm ) { pDlgEdForm = pForm; }
    DlgEdForm*      GetDlgEdForm() const { return pDlgEdForm; }

    virtual SdrObject* SetObjectOrdNum(ULONG nOldObjNum, ULONG nNewObjNum);
};

#endif //_BASCTL_DLGEDPAGE_HXX
