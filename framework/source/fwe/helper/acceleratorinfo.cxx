/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



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
