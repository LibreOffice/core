/*************************************************************************
 *
 *  $RCSfile: forbiddencharacterstable.cxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: mt $ $Date: 2001-03-09 16:48:03 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
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
        pInf = new ForbiddenCharactersInfo;
        ((SvxForbiddenCharactersTableImpl*)this)->Insert( nLanguage, pInf );
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
