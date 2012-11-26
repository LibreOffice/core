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



#include <sot/clsids.hxx>
#include <unotools/moduleoptions.hxx>

#include <unomid.h>

#include "swtypes.hxx"
#include "shellio.hxx"
#include "globdoc.hxx"
#include "globdoc.hrc"
#include "cfgid.h"


/*--------------------------------------------------------------------
    Beschreibung:   Alle Filter registrieren
 --------------------------------------------------------------------*/

SFX_IMPL_OBJECTFACTORY( SwGlobalDocShell, SvGlobalName(SO3_SWGLOB_CLASSID), SFXOBJECTSHELL_STD_NORMAL|SFXOBJECTSHELL_HASMENU, "swriter/GlobalDocument" )

SwGlobalDocShell::SwGlobalDocShell(SfxObjectCreateMode eMode ) :
        SwDocShell(eMode)
{
}

SwGlobalDocShell::~SwGlobalDocShell()
{
}

void SwGlobalDocShell::FillClass( SvGlobalName * pClassName,
                                   sal_uInt32 * pClipFormat,
                                   String * /*pAppName*/,
                                   String * pLongUserName,
                                   String * pUserName,
                                   sal_Int32 nVersion,
                                   sal_Bool bTemplate /* = sal_False */) const
{
    (void)bTemplate;
    DBG_ASSERT( bTemplate == sal_False, "No template for Writer Global" );

    if (nVersion == SOFFICE_FILEFORMAT_60)
    {
        *pClassName = SvGlobalName( SO3_SWGLOB_CLASSID_60 );
        *pClipFormat = SOT_FORMATSTR_ID_STARWRITERGLOB_60;
        *pLongUserName = SW_RESSTR(STR_WRITER_GLOBALDOC_FULLTYPE);
    }
    else if (nVersion == SOFFICE_FILEFORMAT_8)
    {
        *pClassName     = SvGlobalName( SO3_SWGLOB_CLASSID_60 );
        *pClipFormat    = SOT_FORMATSTR_ID_STARWRITERGLOB_8;
        *pLongUserName = SW_RESSTR(STR_WRITER_GLOBALDOC_FULLTYPE);
    }

    *pUserName = SW_RESSTR(STR_HUMAN_SWGLOBDOC_NAME);
}



