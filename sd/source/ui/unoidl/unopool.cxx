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
#include "precompiled_sd.hxx"
#include <i18npool/mslangid.hxx>
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
