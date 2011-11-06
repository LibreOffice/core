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
#include "precompiled_vcl.hxx"

#include <tools/urlobj.hxx>
#include <tools/debug.hxx>

#include <unotools/localfilehelper.hxx>
#include <unotools/ucbstreamhelper.hxx>

#include <vcl/svapp.hxx>
#include <vcl/window.hxx>
#include <vcl/salbtype.hxx>
#include <vcl/sound.hxx>

#include <salframe.hxx>
#include <svdata.hxx>
#include <salinst.hxx>

void Sound::Beep( SoundType eType, Window* pWindow )
{
    // #i91990#
    if ( Application::IsHeadlessModeEnabled() )
        return;

    if( !pWindow )
    {
        Window* pDefWindow = ImplGetDefaultWindow();
        pDefWindow->ImplGetFrame()->Beep( eType );
    }
    else
        pWindow->ImplGetFrame()->Beep( eType );
}
