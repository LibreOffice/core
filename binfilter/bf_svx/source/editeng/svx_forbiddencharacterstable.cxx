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

#include <forbiddencharacterstable.hxx>

#include <unotools/localedatawrapper.hxx>
#include <unolingu.hxx>

/*STRIP001*/#include <tools/debug.hxx>
namespace binfilter {
/*N*/ SvxForbiddenCharactersTable::SvxForbiddenCharactersTable( ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory > xMSF, USHORT nISize, USHORT nGrow )
/*N*/  : SvxForbiddenCharactersTableImpl( nISize, nGrow )
/*N*/ {
/*N*/ 	mxMSF = xMSF;
/*N*/ }


/*N*/ SvxForbiddenCharactersTable::~SvxForbiddenCharactersTable()
/*N*/ {
/*N*/ 	for ( ULONG n = Count(); n; )
/*N*/ 		delete GetObject( --n );
/*N*/ }



const ::com::sun::star::i18n::ForbiddenCharacters* SvxForbiddenCharactersTable::GetForbiddenCharacters( USHORT nLanguage, BOOL bGetDefault ) const
{
    ForbiddenCharactersInfo* pInf = Get( nLanguage );
    if ( !pInf && bGetDefault && mxMSF.is() )
    {
        const SvxForbiddenCharactersTableImpl *pConstImpl =
            dynamic_cast<const SvxForbiddenCharactersTableImpl*>(this);
        SvxForbiddenCharactersTableImpl* pImpl =
            const_cast<SvxForbiddenCharactersTableImpl*>(pConstImpl); 
        pInf = new ForbiddenCharactersInfo;
        pImpl->Insert( nLanguage, pInf );
        pInf->bTemporary = TRUE;
        LocaleDataWrapper aWrapper( mxMSF, SvxCreateLocale( nLanguage ) );
        pInf->aForbiddenChars = aWrapper.getForbiddenCharacters();
    }
    return pInf ? &pInf->aForbiddenChars : NULL;
}



/*N*/ void SvxForbiddenCharactersTable::SetForbiddenCharacters( USHORT nLanguage, const ::com::sun::star::i18n::ForbiddenCharacters& rForbiddenChars )
/*N*/ {
/*N*/ 	ForbiddenCharactersInfo* pInf = Get( nLanguage );
/*N*/ 	if ( !pInf )
/*N*/ 	{
/*N*/ 		pInf = new ForbiddenCharactersInfo;
/*N*/ 		Insert( nLanguage, pInf );
/*N*/ 	}
/*N*/ 	pInf->bTemporary = FALSE;
/*N*/ 	pInf->aForbiddenChars = rForbiddenChars;
/*N*/ }

/*N*/ void SvxForbiddenCharactersTable::ClearForbiddenCharacters( USHORT nLanguage )
/*N*/ {DBG_BF_ASSERT(0, "STRIP"); //STRIP001 
/*N*/ }
}
