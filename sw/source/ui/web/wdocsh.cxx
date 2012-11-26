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
#include "precompiled_sw.hxx"




#include <svl/srchitem.hxx>
#include <sfx2/app.hxx>
#ifndef _SVX_SVXIDS_HRC //autogen
#include <svx/svxids.hrc>
#endif

#include <sot/clsids.hxx>
#include <sfx2/objface.hxx>

#include <sfx2/msg.hxx>
#include "cfgid.h"
#include "cmdid.h"
#include "swtypes.hxx"

#include "shellio.hxx"
// nur wegen des Itemtypes
#include "wdocsh.hxx"
#include "web.hrc"

#define SwWebDocShell
#include "swslots.hxx"

#include <unomid.h>


SFX_IMPL_INTERFACE( SwWebDocShell, SfxObjectShell, SW_RES(0) )
{
}

SFX_IMPL_OBJECTFACTORY(SwWebDocShell, SvGlobalName(SO3_SWWEB_CLASSID), SFXOBJECTSHELL_STD_NORMAL|SFXOBJECTSHELL_HASMENU, "swriter/web" )

/*-----------------22.01.97 09.29-------------------

--------------------------------------------------*/

SwWebDocShell::SwWebDocShell(SfxObjectCreateMode eMode ) :
        SwDocShell(eMode),
        nSourcePara(0)
{
}

/*-----------------22.01.97 09.29-------------------

--------------------------------------------------*/

SwWebDocShell::~SwWebDocShell()
{
}



void SwWebDocShell::FillClass( SvGlobalName * pClassName,
                                   sal_uInt32 * pClipFormat,
                                   String * /*pAppName*/,
                                   String * pLongUserName,
                                   String * pUserName,
                                   sal_Int32 nVersion,
                                   sal_Bool bTemplate /* = sal_False */) const
{
    (void)bTemplate;
    DBG_ASSERT( bTemplate == sal_False, "No template for Writer Web" );

    if (nVersion == SOFFICE_FILEFORMAT_60)
    {
        *pClassName = SvGlobalName( SO3_SWWEB_CLASSID_60 );
        *pClipFormat = SOT_FORMATSTR_ID_STARWRITERWEB_60;
        *pLongUserName = SW_RESSTR(STR_WRITER_WEBDOC_FULLTYPE);
    }
    else if (nVersion == SOFFICE_FILEFORMAT_8)
    {
        *pClassName     = SvGlobalName( SO3_SWWEB_CLASSID_60 );
        *pClipFormat    = SOT_FORMATSTR_ID_STARWRITERWEB_8;
        *pLongUserName = SW_RESSTR(STR_WRITER_WEBDOC_FULLTYPE);
    }
    *pUserName = SW_RESSTR(STR_HUMAN_SWWEBDOC_NAME);
}



