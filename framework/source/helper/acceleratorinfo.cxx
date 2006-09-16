/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: acceleratorinfo.cxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: obo $ $Date: 2006-09-16 13:56:32 $
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
#include "precompiled_framework.hxx"

#include <helper/acceleratorinfo.hxx>

namespace framework
{

static pfunc_getCommandURLFromKeyCode   _pGetCommandURLFromKeyCode = NULL;
static pfunc_getKeyCodeFromCommandURL   _pGetKeyCodeFromCommandURL = NULL;

pfunc_getCommandURLFromKeyCode SAL_CALL SetCommandURLFromKeyCode( pfunc_getCommandURLFromKeyCode pNewFunc )
{
    pfunc_getCommandURLFromKeyCode  pOldFunc = _pGetCommandURLFromKeyCode;
    _pGetCommandURLFromKeyCode = pNewFunc;

    return pOldFunc;
}

::rtl::OUString SAL_CALL GetCommandURLFromKeyCode( const KeyCode& aKeyCode )
{
    if ( _pGetCommandURLFromKeyCode )
        return _pGetCommandURLFromKeyCode( aKeyCode );
    else
        return rtl::OUString();
}

pfunc_getKeyCodeFromCommandURL SAL_CALL SetKeyCodeFromCommandURL( pfunc_getKeyCodeFromCommandURL pNewFunc )
{
    pfunc_getKeyCodeFromCommandURL  pOldFunc = _pGetKeyCodeFromCommandURL;
    _pGetKeyCodeFromCommandURL = pNewFunc;

    return pOldFunc;
}

KeyCode SAL_CALL GetKeyCodeFromCommandURL( ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame >& rFrame, const rtl::OUString& aCommandURL )
{
    if ( _pGetKeyCodeFromCommandURL )
        return _pGetKeyCodeFromCommandURL( rFrame, aCommandURL );
    else
        return KeyCode();
}

}
