/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: abstdlg.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2008-03-12 13:04:00 $
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
#ifndef _VCL_ABSTDLG_HXX
#define _VCL_ABSTDLG_HXX

// include ---------------------------------------------------------------

#include <tools/solar.h>
#include <tools/string.hxx>
#ifndef _VCL_DLLAPI_H
#include <vcl/dllapi.h>
#endif

class Window;
class ResId;
class Link;

class VCL_DLLPUBLIC VclAbstractDialog
{
public:
    virtual short           Execute() = 0;
    //virtual void          Show( BOOL bVisible = TRUE, USHORT nFlags = 0 ) = 0;
    virtual                 ~VclAbstractDialog();
};

class VCL_DLLPUBLIC VclAbstractDialog2
{
public:
    virtual void            StartExecuteModal( const Link& rEndDialogHdl ) = 0;
    virtual long            GetResult() = 0;
    virtual                 ~VclAbstractDialog2();
};

class VCL_DLLPUBLIC VclAbstractTerminatedDialog : public VclAbstractDialog
{
public:
    virtual void            EndDialog(long nResult =0) = 0;
};

class VCL_DLLPUBLIC VclAbstractRefreshableDialog : public VclAbstractDialog
{
public:
    virtual void            Update() = 0;
    virtual void            Sync() = 0;
};

class VCL_DLLPUBLIC VclAbstractDialogFactory
{
public:
    static VclAbstractDialogFactory*    Create();
    // nDialogId was previously a ResId without ResMgr; the ResourceId is now
    // an implementation detail of the factory
    virtual VclAbstractDialog*          CreateVclDialog( Window* pParent, sal_uInt32 nResId ) = 0;
};

#endif

