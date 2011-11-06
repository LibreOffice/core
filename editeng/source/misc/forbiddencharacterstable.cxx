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
#include "precompiled_editeng.hxx"

#include <editeng/forbiddencharacterstable.hxx>

#include <unotools/localedatawrapper.hxx>
#include <editeng/unolingu.hxx>

#include <com/sun/star/lang/XMultiServiceFactory.hpp>

SvxForbiddenCharactersTable::SvxForbiddenCharactersTable( ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory > xMSF, sal_uInt16 nISize, sal_uInt16 nGrow )
 : SvxForbiddenCharactersTableImpl( nISize, nGrow )
{
    mxMSF = xMSF;
}


SvxForbiddenCharactersTable::~SvxForbiddenCharactersTable()
{
    for ( sal_uLong n = Count(); n; )
        delete GetObject( --n );
}



const com::sun::star::i18n::ForbiddenCharacters* SvxForbiddenCharactersTable::GetForbiddenCharacters( sal_uInt16 nLanguage, sal_Bool bGetDefault ) const
{
    ForbiddenCharactersInfo* pInf = Get( nLanguage );
    if ( !pInf && bGetDefault && mxMSF.is() )
    {
        const SvxForbiddenCharactersTableImpl *pConstImpl = dynamic_cast<const SvxForbiddenCharactersTableImpl*>(this);
        SvxForbiddenCharactersTableImpl* pImpl = const_cast<SvxForbiddenCharactersTableImpl*>(pConstImpl);
         pInf = new ForbiddenCharactersInfo;
        pImpl->Insert( nLanguage, pInf );

        pInf->bTemporary = sal_True;
        LocaleDataWrapper aWrapper( mxMSF, SvxCreateLocale( nLanguage ) );
        pInf->aForbiddenChars = aWrapper.getForbiddenCharacters();
    }
    return pInf ? &pInf->aForbiddenChars : NULL;
}



void SvxForbiddenCharactersTable::SetForbiddenCharacters( sal_uInt16 nLanguage, const com::sun::star::i18n::ForbiddenCharacters& rForbiddenChars )
{
    ForbiddenCharactersInfo* pInf = Get( nLanguage );
    if ( !pInf )
    {
        pInf = new ForbiddenCharactersInfo;
        Insert( nLanguage, pInf );
    }
    pInf->bTemporary = sal_False;
    pInf->aForbiddenChars = rForbiddenChars;
}

void SvxForbiddenCharactersTable::ClearForbiddenCharacters( sal_uInt16 nLanguage )
{
    ForbiddenCharactersInfo* pInf = Get( nLanguage );
    if ( pInf )
    {
        Remove( nLanguage );
        delete pInf;
    }
}
