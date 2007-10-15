/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: sdabstdlg.cxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: vg $ $Date: 2007-10-15 12:11:40 $
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
#include "precompiled_sd.hxx"

#include "sdabstdlg.hxx"
#include "sduilib.hxx"

#include <osl/module.hxx>
#include <tools/string.hxx>

typedef SdAbstractDialogFactory* (__LOADONCALLAPI *SdFuncPtrCreateDialogFactory)();

extern "C" { static void SAL_CALL thisModule() {} }

SdAbstractDialogFactory* SdAbstractDialogFactory::Create()
{
    SdFuncPtrCreateDialogFactory fp = 0;
    static ::osl::Module aDialogLibrary;
    if ( aDialogLibrary.is() || aDialogLibrary.loadRelative( &thisModule, String( RTL_CONSTASCII_USTRINGPARAM( DLL_NAME ) ) ) )
        fp = ( SdAbstractDialogFactory* (__LOADONCALLAPI*)() )
            aDialogLibrary.getFunctionSymbol( ::rtl::OUString::createFromAscii("CreateDialogFactory") );
    if ( fp )
        return fp();
    return 0;
}


