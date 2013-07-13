/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include <i18nlangtag/languagetag.hxx>
#include <comphelper/propertysetinfo.hxx>
#include <editeng/eeitem.hxx>
#include <svx/unopool.hxx>

#include "drawdoc.hxx"

using namespace ::com::sun::star;
using namespace ::rtl;
using namespace ::cppu;
using namespace ::comphelper;

LanguageType SdUnoGetLanguage( const lang::Locale& rLocale )
{
    //  empty language -> LANGUAGE_SYSTEM
    if ( rLocale.Language.getLength() == 0 )
        return LANGUAGE_SYSTEM;

    LanguageType eRet = LanguageTag::convertToLanguageType( rLocale, false);
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
    virtual void putAny( SfxItemPool* pPool, const PropertyMapEntry* pEntry, const uno::Any& rValue ) throw( beans::UnknownPropertyException, lang::IllegalArgumentException);

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

void SdUnoDrawPool::putAny( SfxItemPool* pPool, const comphelper::PropertyMapEntry* pEntry, const uno::Any& rValue )
    throw(beans::UnknownPropertyException, lang::IllegalArgumentException)
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
                    (const sal_uInt16)pEntry->mnHandle );
        }
    }
    SvxUnoDrawPool::putAny( pPool, pEntry, rValue );
}

uno::Reference< uno::XInterface > SdUnoCreatePool( SdDrawDocument* pDrawModel )
{
    return (uno::XAggregation*)new SdUnoDrawPool( pDrawModel );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
