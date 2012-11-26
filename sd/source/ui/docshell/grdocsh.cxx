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
#include "precompiled_sd.hxx"

#ifndef _SVXIDS_HRC
#include <svx/svxids.hrc>
#endif
#include <sfx2/app.hxx>
#include <svl/srchitem.hxx>
#include <tools/globname.hxx>

#ifndef _SO_CLSIDS_HXX //autogen
#include <sot/clsids.hxx>
#endif

#include <sfx2/objface.hxx>


#include "app.hrc"
#include "strings.hrc"
#include "glob.hrc"
#include "GraphicDocShell.hxx"
#include "DrawDocShell.hxx"
#include "drawdoc.hxx"
#include "sdresid.hxx"

using namespace sd;
#define GraphicDocShell
#include "sdgslots.hxx"

namespace sd
{

SFX_IMPL_INTERFACE(GraphicDocShell, SfxObjectShell, SdResId(0))
{
    SFX_CHILDWINDOW_REGISTRATION(SID_SEARCH_DLG);
    SFX_CHILDWINDOW_REGISTRATION( SID_HYPERLINK_INSERT );
}

SFX_IMPL_OBJECTFACTORY( GraphicDocShell, SvGlobalName(SO3_SDRAW_CLASSID_60), SFXOBJECTSHELL_STD_NORMAL, "sdraw" )

GraphicDocShell::GraphicDocShell(SfxObjectCreateMode eMode,
                                     bool bDataObject,
                                     DocumentType eDocType) :
    DrawDocShell(eMode, bDataObject, eDocType)
{
    SetStyleFamily( SD_STYLE_FAMILY_GRAPHICS );
}

GraphicDocShell::GraphicDocShell(const sal_uInt64 nModelCreationFlags,
                                     bool bDataObject,
                                     DocumentType eDocType) :
    DrawDocShell(nModelCreationFlags, bDataObject, eDocType)
{
    SetStyleFamily( SD_STYLE_FAMILY_GRAPHICS );
}

GraphicDocShell::~GraphicDocShell()
{
}


} // end of namespace sd
