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

#include <i18npool/mslangid.hxx>

#include <comphelper/propertysetinfo.hxx>

#include <bf_svx/eeitem.hxx>

#include <bf_svx/unopool.hxx>

#include "drawdoc.hxx"
namespace binfilter {

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::beans;
using namespace ::cppu;
using namespace ::comphelper;

LanguageType SdUnoGetLanguage( const lang::Locale& rLocale )
{
    //	empty language -> LANGUAGE_SYSTEM
    if ( rLocale.Language.getLength() == 0 )
        return LANGUAGE_SYSTEM;

    LanguageType eRet = MsLangId::convertLocaleToLanguage( rLocale );
    if ( eRet == LANGUAGE_NONE )
        eRet = LANGUAGE_SYSTEM;			//! or throw an exception?

    return eRet;
}

class SdUnoDrawPool :	public SvxUnoDrawPool
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
                mpDrawModel->SetLanguage( SdUnoGetLanguage( aLocale ), pEntry->mnHandle );
        }
    }
    SvxUnoDrawPool::putAny( pPool, pEntry, rValue );
}

Reference< XInterface > SdUnoCreatePool( SdDrawDocument* pDrawModel )
{
    return (uno::XAggregation*)new SdUnoDrawPool( pDrawModel );
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
