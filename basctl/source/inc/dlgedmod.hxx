/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: dlgedmod.hxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: obo $ $Date: 2007-03-15 16:02:20 $
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


#ifndef _BASCTL_DLGEDMOD_HXX
#define _BASCTL_DLGEDMOD_HXX

#ifndef _SVDMODEL_HXX
#include <svx/svdmodel.hxx>
#endif


//============================================================================
// DlgEdModel
//============================================================================

class DlgEdPage;
class Window;
class SfxObjectShell;

class DlgEdModel : public SdrModel
{
    friend class DlgEdPage;

private:
    DlgEdModel( const DlgEdModel& );                // not implemented
    void operator=(const DlgEdModel& rSrcModel);    // not implemented

public:
    TYPEINFO();

    DlgEdModel();
    virtual ~DlgEdModel();

    virtual void DlgEdModelChanged( FASTBOOL bChanged = TRUE );

    virtual SdrPage*  AllocPage(FASTBOOL bMasterPage);

    virtual Window* GetCurDocViewWin();
};

#endif

