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
#include "precompiled_sfx2.hxx"
// INCLUDE ---------------------------------------------------------------

#include <sfx2/app.hxx>
#include "sfx2/viewfac.hxx"
#include <rtl/ustrbuf.hxx>

// STATIC DATA -----------------------------------------------------------

DBG_NAME(SfxViewFactory)

SfxViewShell *SfxViewFactory::CreateInstance(SfxViewFrame *pFrame, SfxViewShell *pOldSh )
{
    DBG_CHKTHIS(SfxViewFactory, 0);
    return (*fnCreate)(pFrame, pOldSh);
}

void SfxViewFactory::InitFactory()
{
    DBG_CHKTHIS(SfxViewFactory, 0);
    (*fnInit)();
}

String SfxViewFactory::GetLegacyViewName() const
{
    ::rtl::OUStringBuffer aViewName;
    aViewName.appendAscii( "view" );
    aViewName.append( sal_Int32( GetOrdinal() ) );
    return aViewName.makeStringAndClear();
}

String SfxViewFactory::GetAPIViewName() const
{
    if ( m_sViewName.Len() > 0 )
        return m_sViewName;

    if ( GetOrdinal() == 0 )
        return String::CreateFromAscii( "Default" );

    return GetLegacyViewName();
}

// CTOR / DTOR -----------------------------------------------------------

SfxViewFactory::SfxViewFactory( SfxViewCtor fnC, SfxViewInit fnI,
                                sal_uInt16 nOrdinal, const sal_Char* asciiViewName ):
    fnCreate(fnC),
    fnInit(fnI),
    nOrd(nOrdinal),
    m_sViewName( String::CreateFromAscii( asciiViewName ) )
{
    DBG_CTOR(SfxViewFactory, 0);
}

SfxViewFactory::~SfxViewFactory()
{
    DBG_DTOR(SfxViewFactory, 0);
}


