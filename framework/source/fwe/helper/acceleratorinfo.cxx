/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_framework.hxx"

#include <framework/acceleratorinfo.hxx>

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
