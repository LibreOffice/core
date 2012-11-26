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
#include "precompiled_sc.hxx"



#include <editeng/forbiddencharacterstable.hxx>

#include "forbiuno.hxx"
#include "docsh.hxx"

using namespace ::com::sun::star;

//------------------------------------------------------------------------

vos::ORef<SvxForbiddenCharactersTable> lcl_GetForbidden( ScDocShell* pDocSh )
{
    vos::ORef<SvxForbiddenCharactersTable> xRet;
    if ( pDocSh )
    {
        ScDocument* pDoc = pDocSh->GetDocument();
        xRet = pDoc->GetForbiddenCharacters();
        if ( !xRet.isValid() )
        {
            //  create an empty SvxForbiddenCharactersTable for SvxUnoForbiddenCharsTable,
            //  so changes can be stored.

            xRet = new SvxForbiddenCharactersTable( pDoc->GetServiceManager() );
            pDoc->SetForbiddenCharacters( xRet );
        }
    }
    return xRet;
}

ScForbiddenCharsObj::ScForbiddenCharsObj( ScDocShell* pDocSh ) :
    SvxUnoForbiddenCharsTable( lcl_GetForbidden( pDocSh ) ),
    pDocShell( pDocSh )
{
    if (pDocShell)
        pDocShell->GetDocument()->AddUnoObject(*this);
}

ScForbiddenCharsObj::~ScForbiddenCharsObj()
{
    if (pDocShell)
        pDocShell->GetDocument()->RemoveUnoObject(*this);
}

void ScForbiddenCharsObj::Notify( SfxBroadcaster&, const SfxHint& rHint )
{
    const SfxSimpleHint* pSfxSimpleHint = dynamic_cast< const SfxSimpleHint* >(&rHint);

    if ( pSfxSimpleHint && SFX_HINT_DYING == pSfxSimpleHint->GetId() )
    {
        pDocShell = NULL;       // document gone
    }
}

void ScForbiddenCharsObj::onChange()
{
    if (pDocShell)
    {
        pDocShell->GetDocument()->SetForbiddenCharacters( mxForbiddenChars );
        pDocShell->PostPaintGridAll();
        pDocShell->SetDocumentModified();
    }
}

