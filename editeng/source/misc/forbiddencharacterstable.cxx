/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
