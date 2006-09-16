/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: unopool.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: obo $ $Date: 2006-09-16 19:28:04 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sd.hxx"

#ifndef INCLUDED_I18NPOOL_MSLANGID_HXX
#include <i18npool/mslangid.hxx>
#endif

#ifndef _COMPHELPER_PROPERTSETINFO_HXX_
#include <comphelper/propertysetinfo.hxx>
#endif

#ifndef _EEITEM_HXX
#include <svx/eeitem.hxx>
#endif

#ifndef _SVX_UNOPOOL_HXX_
#include <svx/unopool.hxx>
#endif

#include "drawdoc.hxx"

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::beans;
using namespace ::rtl;
using namespace ::cppu;
using namespace ::comphelper;

LanguageType SdUnoGetLanguage( const lang::Locale& rLocale )
{
    //  empty language -> LANGUAGE_SYSTEM
    if ( rLocale.Language.getLength() == 0 )
        return LANGUAGE_SYSTEM;

    LanguageType eRet = MsLangId::convertLocaleToLanguage( rLocale );
    if ( eRet == LANGUAGE_NONE )
        eRet = LANGUAGE_SYSTEM;         //! or throw an exception?

    return eRet;
}

class SdUnoDrawPool :   public SvxUnoDrawPool
{
public:
    SdUnoDrawPool( SdDrawDocument* pModel ) throw();
    virtual ~SdUnoDrawPool() throw();

protected:
    virtual void putAny( SfxItemPool* pPool, const PropertyMapEntry* pEntry, const Any& rValue ) throw( UnknownPropertyException, IllegalArgumentException);

private:
    SdDrawDocument* mpDrawModel;
};

SdUnoDrawPool::SdUnoDrawPool( SdDrawDocument* pModel ) throw()
: SvxUnoDrawPool( pModel ), mpDrawModel( pModel )
{
}

SdUnoDrawPool::~SdUnoDrawPool() throw()
{
}

void SdUnoDrawPool::putAny( SfxItemPool* pPool, const comphelper::PropertyMapEntry* pEntry, const Any& rValue )
    throw(UnknownPropertyException, IllegalArgumentException)
{
    switch( pEntry->mnHandle )
    {
    case EE_CHAR_LANGUAGE:
    case EE_CHAR_LANGUAGE_CJK:
    case EE_CHAR_LANGUAGE_CTL:
        {
            lang::Locale aLocale;
            if( rValue >>= aLocale )
                mpDrawModel->SetLanguage(
                    SdUnoGetLanguage( aLocale ),
                    (const USHORT)pEntry->mnHandle );
        }
    }
    SvxUnoDrawPool::putAny( pPool, pEntry, rValue );
}

Reference< XInterface > SdUnoCreatePool( SdDrawDocument* pDrawModel )
{
    return (uno::XAggregation*)new SdUnoDrawPool( pDrawModel );
}
