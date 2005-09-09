/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: unomlstr.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 01:05:20 $
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

#ifndef _SVDOBJ_HXX
#include <svdobj.hxx>
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
void SAL_CALL SvxUnoShapeModifyListener::modified(const lang::EventObject& aEvent) throw( uno::RuntimeException )
{
    if( mpObj )
    {
        mpObj->SetChanged();
        mpObj->BroadcastObjectChange();
    }
}

// ::com::sun::star::lang::XEventListener
void SvxUnoShapeModifyListener::disposing(const lang::EventObject& Source) throw( uno::RuntimeException )
{
    invalidate();
}

// internal
void SvxUnoShapeModifyListener::invalidate() throw()
{
    mpObj = NULL;
}


