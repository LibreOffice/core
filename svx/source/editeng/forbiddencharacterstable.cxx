/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: forbiddencharacterstable.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-11-08 09:09:58 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#include <forbiddencharacterstable.hxx>

#include <unotools/localedatawrapper.hxx>
#include <unolingu.hxx>

#ifndef _COM_SUN_STAR_LANG_XSINGLESERVICEFACTORY_HPP_
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#endif

SvxForbiddenCharactersTable::SvxForbiddenCharactersTable( ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory > xMSF, USHORT nISize, USHORT nGrow )
 : SvxForbiddenCharactersTableImpl( nISize, nGrow )
{
    mxMSF = xMSF;
}


SvxForbiddenCharactersTable::~SvxForbiddenCharactersTable()
{
    for ( ULONG n = Count(); n; )
        delete GetObject( --n );
}



const com::sun::star::i18n::ForbiddenCharacters* SvxForbiddenCharactersTable::GetForbiddenCharacters( USHORT nLanguage, BOOL bGetDefault ) const
{
    ForbiddenCharactersInfo* pInf = Get( nLanguage );
    if ( !pInf && bGetDefault && mxMSF.is() )
    {
        const SvxForbiddenCharactersTableImpl *pConstImpl = dynamic_cast<const SvxForbiddenCharactersTableImpl*>(this);
        SvxForbiddenCharactersTableImpl* pImpl = const_cast<SvxForbiddenCharactersTableImpl*>(pConstImpl);
         pInf = new ForbiddenCharactersInfo;
        pImpl->Insert( nLanguage, pInf );

        pInf->bTemporary = TRUE;
        LocaleDataWrapper aWrapper( mxMSF, SvxCreateLocale( nLanguage ) );
        pInf->aForbiddenChars = aWrapper.getForbiddenCharacters();
    }
    return pInf ? &pInf->aForbiddenChars : NULL;
}



void SvxForbiddenCharactersTable::SetForbiddenCharacters( USHORT nLanguage, const com::sun::star::i18n::ForbiddenCharacters& rForbiddenChars )
{
    ForbiddenCharactersInfo* pInf = Get( nLanguage );
    if ( !pInf )
    {
        pInf = new ForbiddenCharactersInfo;
        Insert( nLanguage, pInf );
    }
    pInf->bTemporary = FALSE;
    pInf->aForbiddenChars = rForbiddenChars;
}

void SvxForbiddenCharactersTable::ClearForbiddenCharacters( USHORT nLanguage )
{
    ForbiddenCharactersInfo* pInf = Get( nLanguage );
    if ( pInf )
    {
        Remove( nLanguage );
        delete pInf;
    }
}
