/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: unomlstr.cxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: vg $ $Date: 2007-05-22 15:21:19 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_svx.hxx"

#ifndef _SVDOBJ_HXX
#include <svdobj.hxx>
#endif
// header for class OGuard
#ifndef _VOS_MUTEX_HXX_
#include <vos/mutex.hxx>
#endif
// header for class Application
#ifndef _SV_SVAPP_HXX
#include <vcl/svapp.hxx>
#endif

#include "unomlstr.hxx"

using namespace ::com::sun::star;

SvxUnoShapeModifyListener::SvxUnoShapeModifyListener( SdrObject* pObj ) throw()
{
    mpObj = pObj;
}

SvxUnoShapeModifyListener::~SvxUnoShapeModifyListener() throw()
{
}

// ::com::sun::star::util::XModifyListener
void SAL_CALL SvxUnoShapeModifyListener::modified(const lang::EventObject& ) throw( uno::RuntimeException )
{
    ::vos::OGuard aGuard( Application::GetSolarMutex() );
    if( mpObj )
    {
        mpObj->SetChanged();
        mpObj->BroadcastObjectChange();
    }
}

// ::com::sun::star::lang::XEventListener
void SvxUnoShapeModifyListener::disposing(const lang::EventObject& ) throw( uno::RuntimeException )
{
    invalidate();
}

// internal
void SvxUnoShapeModifyListener::invalidate() throw()
{
    mpObj = NULL;
}


