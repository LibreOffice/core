/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: SwSmartTagMgr.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: hr $ $Date: 2007-06-27 13:16:45 $
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
#include "precompiled_sw.hxx"

#ifndef _SWSMARTTAGMGR_HXX
#include "SwSmartTagMgr.hxx"
#endif

#include <vos/mutex.hxx>
#include <vcl/svapp.hxx>

#ifndef _SWMODULE_HXX
#include <swmodule.hxx>
#endif

#ifndef _DOCSH_HXX
#include <docsh.hxx>
#endif

using namespace com::sun::star;
using namespace com::sun::star::uno;

SwSmartTagMgr* SwSmartTagMgr::mpTheSwSmartTagMgr = 0;

SwSmartTagMgr& SwSmartTagMgr::Get()
{
    if ( !mpTheSwSmartTagMgr )
    {
        mpTheSwSmartTagMgr = new SwSmartTagMgr( SwDocShell::Factory().GetModuleName() );
        mpTheSwSmartTagMgr->Init( rtl::OUString::createFromAscii("Writer") );
    }
    return *mpTheSwSmartTagMgr;
}

SwSmartTagMgr::SwSmartTagMgr( const rtl::OUString& rModuleName ) :
    SmartTagMgr( rModuleName )
{
}

SwSmartTagMgr::~SwSmartTagMgr()
{
}

// ::com::sun::star::util::XModifyListener
void SwSmartTagMgr::modified( const lang::EventObject& rEO ) throw( RuntimeException )
{
    vos::OGuard aGuard(Application::GetSolarMutex());

    // Installed recognizers have changed. We remove all existing smart tags:
    SW_MOD()->CheckSpellChanges( sal_False, sal_True, sal_True, sal_True );

    SmartTagMgr::modified( rEO );
}

// ::com::sun::star::util::XChangesListener
void SwSmartTagMgr::changesOccurred( const util::ChangesEvent& rEvent ) throw( RuntimeException)
{
    vos::OGuard aGuard(Application::GetSolarMutex());

    // Configuration has changed. We remove all existing smart tags:
    SW_MOD()->CheckSpellChanges( sal_False, sal_True, sal_True, sal_True );

    SmartTagMgr::changesOccurred( rEvent );
}

/*
SmartTagMgr& SwSmartTagMgr::Get()
{
    if ( !pSmartTagMgr )
        pSmartTagMgr = new SmartTagMgr( SwDocShell::Factory().GetModuleName() );

     return *pSmartTagMgr;
}
*/

