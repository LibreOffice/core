/*************************************************************************
 *
 *  $RCSfile: unopool.cxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: cl $ $Date: 2001-08-06 12:52:34 $
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

#ifndef _ISOLANG_HXX
#include <tools/isolang.hxx>
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

    String aLangStr = rLocale.Language;
    String aCtryStr = rLocale.Country;
    //  Variant is ignored

    LanguageType eRet = ConvertIsoNamesToLanguage( aLangStr, aCtryStr );
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
                mpDrawModel->SetLanguage( SdUnoGetLanguage( aLocale ), pEntry->mnHandle );
        }
    }
    SvxUnoDrawPool::putAny( pPool, pEntry, rValue );
}

Reference< XInterface > SdUnoCreatePool( SdDrawDocument* pDrawModel )
{
    return (uno::XAggregation*)new SdUnoDrawPool( pDrawModel );
}
