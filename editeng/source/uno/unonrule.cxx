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

#define PROPERTY_NONE 0

#include <com/sun/star/text/HoriOrientation.hpp>
#include <com/sun/star/awt/XBitmap.hpp>

#include <vcl/svapp.hxx>
#include <vos/mutex.hxx>
#include <vcl/graph.hxx>
#include <svtools/grfmgr.hxx>
#include <toolkit/unohlp.hxx>
#include <rtl/uuid.h>
#include <rtl/memory.h>

#include <editeng/brshitem.hxx>
#include <editeng/unoprnms.hxx>
#include <editeng/numitem.hxx>
#include <editeng/eeitem.hxx>
#include <editeng/unotext.hxx>
#include <editeng/numitem.hxx>
#include <editeng/unofdesc.hxx>
#include <editeng/unonrule.hxx>
#include <editeng/editids.hrc>

using ::rtl::OUString;
using ::com::sun::star::util::XCloneable;
using ::com::sun::star::ucb::XAnyCompare;


using namespace ::vos;
using namespace ::std;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::container;

const SvxAdjust aUnoToSvxAdjust[] =
{
    SVX_ADJUST_LEFT,
    SVX_ADJUST_RIGHT,
    SVX_ADJUST_CENTER,
    SVX_ADJUST_LEFT,
    SVX_ADJUST_LEFT,
    SVX_ADJUST_LEFT,
    SVX_ADJUST_BLOCK
};

const unsigned short aSvxToUnoAdjust[] =
{
    text::HoriOrientation::LEFT,
    text::HoriOrientation::RIGHT,
    text::HoriOrientation::FULL,
    text::HoriOrientation::CENTER,
    text::HoriOrientation::FULL,
    text::HoriOrientation::LEFT
};

SvxAdjust ConvertUnoAdjust( unsigned short nAdjust )
{
    DBG_ASSERT( nAdjust <= 7, "Enum hat sich geaendert! [CL]" );
    return aUnoToSvxAdjust[nAdjust];
}

unsigned short ConvertUnoAdjust( SvxAdjust eAdjust )
{
    DBG_ASSERT( eAdjust <= 6, "Enum hat sich geaendert! [CL]" );
    return aSvxToUnoAdjust[eAdjust];
}

/******************************************************************
 * SvxUnoNumberingRules
 ******************************************************************/

UNO3_GETIMPLEMENTATION_IMPL( SvxUnoNumberingRules );

SvxUnoNumberingRules::SvxUnoNumberingRules( const SvxNumRule& rRule ) throw()
: maRule( rRule )
{
}

SvxUnoNumberingRules::~SvxUnoNumberingRules() throw()
{
}

//XIndexReplace
void SAL_CALL SvxUnoNumberingRules::replaceByIndex( sal_Int32 Index, const uno::Any& Element )
    throw( IllegalArgumentException, IndexOutOfBoundsException, WrappedTargetException, RuntimeException )
{
    OGuard aGuard( Application::GetSolarMutex() );

    if( Index < 0 || Index >= maRule.GetLevelCount() )
        throw IndexOutOfBoundsException();

    Sequence< beans::PropertyValue > aSeq;

    if( !( Element >>= aSeq) )
        throw IllegalArgumentException();
    setNumberingRuleByIndex( aSeq, Index );
}

// XIndexAccess
sal_Int32 SAL_CALL SvxUnoNumberingRules::getCount() throw( RuntimeException )
{
    OGuard aGuard( Application::GetSolarMutex() );

    return maRule.GetLevelCount();
}

Any SAL_CALL SvxUnoNumberingRules::getByIndex( sal_Int32 Index )
    throw( IndexOutOfBoundsException, WrappedTargetException, RuntimeException )
{
    OGuard aGuard( Application::GetSolarMutex() );

    if( Index < 0 || Index >= maRule.GetLevelCount() )
        throw IndexOutOfBoundsException();

    return Any( getNumberingRuleByIndex(Index) );
}

//XElementAccess
Type SAL_CALL SvxUnoNumberingRules::getElementType()
    throw( RuntimeException )
{
    return ::getCppuType(( const Sequence< beans::PropertyValue >*)0);
}

sal_Bool SAL_CALL SvxUnoNumberingRules::hasElements() throw( RuntimeException )
{
    return sal_True;
}

// XAnyCompare
sal_Int16 SAL_CALL SvxUnoNumberingRules::compare( const Any& rAny1, const Any& rAny2 ) throw(RuntimeException)
{
    return SvxUnoNumberingRules::Compare( rAny1, rAny2 );
}

// XCloneable
Reference< XCloneable > SAL_CALL SvxUnoNumberingRules::createClone(  ) throw (RuntimeException)
{
    return new SvxUnoNumberingRules(maRule);
}

// XServiceInfo
sal_Char pSvxUnoNumberingRulesService[sizeof("com.sun.star.text.NumberingRules")] = "com.sun.star.text.NumberingRules";

OUString SAL_CALL SvxUnoNumberingRules::getImplementationName(  ) throw(RuntimeException)
{
    return OUString( RTL_CONSTASCII_USTRINGPARAM( "SvxUnoNumberingRules" ) );
}

sal_Bool SAL_CALL SvxUnoNumberingRules::supportsService( const OUString& ServiceName ) throw(RuntimeException)
{
    return ServiceName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( pSvxUnoNumberingRulesService ) );
}

Sequence< OUString > SAL_CALL SvxUnoNumberingRules::getSupportedServiceNames(  ) throw(RuntimeException)
{
    OUString aService( RTL_CONSTASCII_USTRINGPARAM( pSvxUnoNumberingRulesService ) );
    Sequence< OUString > aSeq( &aService, 1 );
    return aSeq;
}

Sequence<beans::PropertyValue> SvxUnoNumberingRules::getNumberingRuleByIndex( sal_Int32 nIndex) const throw()
{
    //  NumberingRule aRule;
    const SvxNumberFormat& rFmt = maRule.GetLevel((sal_uInt16) nIndex);
    sal_uInt16 nIdx = 0;

    const int nProps = 15;
    beans::PropertyValue* pArray = new beans::PropertyValue[nProps];

    Any aVal;
    {
        aVal <<= rFmt.GetNumberingType();
        beans::PropertyValue aAlignProp( OUString(RTL_CONSTASCII_USTRINGPARAM(UNO_NAME_NRULE_NUMBERINGTYPE)), -1, aVal, beans::PropertyState_DIRECT_VALUE);
        pArray[nIdx++] = aAlignProp;
    }

    {
        SvxAdjust eAdj = rFmt.GetNumAdjust();
        aVal <<= ConvertUnoAdjust(eAdj);
        pArray[nIdx++] = beans::PropertyValue( OUString(RTL_CONSTASCII_USTRINGPARAM(UNO_NAME_NRULE_ADJUST)), -1, aVal, beans::PropertyState_DIRECT_VALUE);
    }

    {
        aVal <<= OUString(rFmt.GetPrefix());
        beans::PropertyValue aPrefixProp( OUString(RTL_CONSTASCII_USTRINGPARAM(UNO_NAME_NRULE_PREFIX)), -1, aVal, beans::PropertyState_DIRECT_VALUE);
        pArray[nIdx++] = aPrefixProp;
    }

    {
        aVal <<= OUString(rFmt.GetSuffix());
        beans::PropertyValue aSuffixProp( OUString(RTL_CONSTASCII_USTRINGPARAM(UNO_NAME_NRULE_SUFFIX)), -1, aVal, beans::PropertyState_DIRECT_VALUE);
        pArray[nIdx++] = aSuffixProp;
    }

    {
        sal_Unicode nCode = rFmt.GetBulletChar();
        OUString aStr( &nCode, 1 );
        aVal <<= aStr;
        beans::PropertyValue aBulletProp( OUString(RTL_CONSTASCII_USTRINGPARAM("BulletChar")), -1, aVal, beans::PropertyState_DIRECT_VALUE);
        pArray[nIdx++] = aBulletProp;
    }

    if( rFmt.GetBulletFont() )
    {
        awt::FontDescriptor aDesc;
        SvxUnoFontDescriptor::ConvertFromFont( *rFmt.GetBulletFont(), aDesc );
        aVal.setValue(&aDesc, ::getCppuType((const awt::FontDescriptor*)0));
        pArray[nIdx++] = beans::PropertyValue( OUString(RTL_CONSTASCII_USTRINGPARAM(UNO_NAME_NRULE_BULLET_FONT)), -1, aVal, beans::PropertyState_DIRECT_VALUE);
    }

    {
        const SvxBrushItem* pBrush = rFmt.GetBrush();
        if(pBrush && pBrush->GetGraphicObject())
        {
            const GraphicObject* pGrafObj = pBrush->GetGraphicObject();
            OUString aURL( RTL_CONSTASCII_USTRINGPARAM(UNO_NAME_GRAPHOBJ_URLPREFIX));
            aURL += OUString::createFromAscii( pGrafObj->GetUniqueID().GetBuffer() );

            aVal <<= aURL;
            const beans::PropertyValue aGraphicProp( OUString(RTL_CONSTASCII_USTRINGPARAM("GraphicURL")), -1, aVal, beans::PropertyState_DIRECT_VALUE);
            pArray[nIdx++] = aGraphicProp;
        }
    }

    {
        const Size aSize( rFmt.GetGraphicSize() );
        const awt::Size aUnoSize( aSize.Width(), aSize.Height() );
        aVal <<= aUnoSize;
        const beans::PropertyValue aGraphicSizeProp(OUString(RTL_CONSTASCII_USTRINGPARAM("GraphicSize")), -1, aVal, beans::PropertyState_DIRECT_VALUE );
        pArray[nIdx++] = aGraphicSizeProp;
    }

    aVal <<= (sal_Int16)rFmt.GetStart();
    pArray[nIdx++] = beans::PropertyValue(OUString(RTL_CONSTASCII_USTRINGPARAM(UNO_NAME_NRULE_START_WITH)), -1, aVal, beans::PropertyState_DIRECT_VALUE);

    aVal <<= (sal_Int32)rFmt.GetAbsLSpace();
    pArray[nIdx++] = beans::PropertyValue(OUString(RTL_CONSTASCII_USTRINGPARAM(UNO_NAME_NRULE_LEFT_MARGIN)), -1, aVal, beans::PropertyState_DIRECT_VALUE);

    aVal <<= (sal_Int32)rFmt.GetFirstLineOffset();
    pArray[nIdx++] = beans::PropertyValue(OUString(RTL_CONSTASCII_USTRINGPARAM(UNO_NAME_NRULE_FIRST_LINE_OFFSET)), -1, aVal, beans::PropertyState_DIRECT_VALUE);

    pArray[nIdx++] = beans::PropertyValue(OUString(RTL_CONSTASCII_USTRINGPARAM("SymbolTextDistance")), -1, aVal, beans::PropertyState_DIRECT_VALUE);

    aVal <<= (sal_Int32)rFmt.GetBulletColor().GetColor();
    pArray[nIdx++] = beans::PropertyValue(OUString(RTL_CONSTASCII_USTRINGPARAM(UNO_NAME_NRULE_BULLET_COLOR)), -1, aVal, beans::PropertyState_DIRECT_VALUE);

    aVal <<= (sal_Int16)rFmt.GetBulletRelSize();
    pArray[nIdx++] = beans::PropertyValue(OUString(RTL_CONSTASCII_USTRINGPARAM(UNO_NAME_NRULE_BULLET_RELSIZE)), -1, aVal, beans::PropertyState_DIRECT_VALUE);

    DBG_ASSERT( nIdx <= nProps, "FixMe: Array uebergelaufen!!!! [CL]" );
    Sequence< beans::PropertyValue> aSeq(pArray, nIdx);

    delete [] pArray;
    return aSeq;
}

void SvxUnoNumberingRules::setNumberingRuleByIndex( const Sequence< beans::PropertyValue >& rProperties, sal_Int32 nIndex)
    throw( RuntimeException, IllegalArgumentException )
{
    SvxNumberFormat aFmt(maRule.GetLevel( (sal_uInt16)nIndex ));
    const beans::PropertyValue* pPropArray = rProperties.getConstArray();
    for(int i = 0; i < rProperties.getLength(); i++)
    {
        const beans::PropertyValue& rProp = pPropArray[i];
        const OUString& rPropName = rProp.Name;
        const Any& aVal = rProp.Value;

        if(rPropName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM(UNO_NAME_NRULE_NUMBERINGTYPE)))
        {
            sal_Int16 nSet = sal_Int16();
            aVal >>= nSet;

            switch(nSet)
            {
            case SVX_NUM_BITMAP:
            case SVX_NUM_CHAR_SPECIAL:
            case SVX_NUM_ROMAN_UPPER:
            case SVX_NUM_ROMAN_LOWER:
            case SVX_NUM_CHARS_UPPER_LETTER:
            case SVX_NUM_CHARS_LOWER_LETTER:
            case SVX_NUM_ARABIC:
            case SVX_NUM_NUMBER_NONE:
            case SVX_NUM_CHARS_UPPER_LETTER_N:
            case SVX_NUM_CHARS_LOWER_LETTER_N:
                aFmt.SetNumberingType(nSet);
                continue;
            }
        }
        else if(rPropName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM(UNO_NAME_NRULE_PREFIX)))
        {
            OUString aPrefix;
            if( aVal >>= aPrefix )
            {
                aFmt.SetPrefix(aPrefix);
                continue;
            }
        }
        else if(rPropName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM(UNO_NAME_NRULE_SUFFIX)))
        {
            OUString aSuffix;
            if( aVal >>= aSuffix )
            {
                aFmt.SetSuffix(aSuffix);
                continue;
            }
        }
        else if(rPropName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM(UNO_NAME_NRULE_BULLETID)))
        {
            sal_Int16 nSet = sal_Int16();
            if( aVal >>= nSet )
            {
                if(nSet < 0x100)
                {
                    aFmt.SetBulletChar(nSet);
                    continue;
                }
            }
        }
        else if(rPropName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM("BulletChar")))
        {
            OUString aStr;
            if( aVal >>= aStr )
            {
                if(aStr.getLength())
                {
                    aFmt.SetBulletChar(aStr[0]);
                }
                else
                {
                    aFmt.SetBulletChar(0);
                }
                continue;
            }
        }
        else if(rPropName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM(UNO_NAME_NRULE_ADJUST)))
        {
            sal_Int16 nAdjust = sal_Int16();
            if( aVal >>= nAdjust )
            {
                aFmt.SetNumAdjust(ConvertUnoAdjust( (unsigned short)nAdjust ));
                continue;
            }
        }
        else if(rPropName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM(UNO_NAME_NRULE_BULLET_FONT)))
        {
            awt::FontDescriptor aDesc;
            if( aVal >>= aDesc )
            {
                Font aFont;
                SvxUnoFontDescriptor::ConvertToFont( aDesc, aFont );
                aFmt.SetBulletFont(&aFont);
                continue;
            }
        }
        else if(rPropName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM("Graphic")))
        {
            Reference< awt::XBitmap > xBmp;
            if( aVal >>= xBmp )
            {
                Graphic aGraf( VCLUnoHelper::GetBitmap( xBmp ) );
                SvxBrushItem aBrushItem(aGraf, GPOS_AREA, SID_ATTR_BRUSH);
                aFmt.SetGraphicBrush( &aBrushItem );
                continue;
            }
        }
        else if(rPropName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM("GraphicURL")))
        {
            OUString aURL;
            if( aVal >>= aURL )
            {
                GraphicObject aGrafObj( GraphicObject::CreateGraphicObjectFromURL( aURL ) );
                SvxBrushItem aBrushItem( aGrafObj, GPOS_AREA, SID_ATTR_BRUSH );
                aFmt.SetGraphicBrush( &aBrushItem );
                continue;
            }
        }
        else if(rPropName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM("GraphicSize")))
        {
            awt::Size aUnoSize;
            if( aVal >>= aUnoSize )
            {
                aFmt.SetGraphicSize( Size( aUnoSize.Width, aUnoSize.Height ) );
                continue;
            }
        }
        else if(rPropName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM(UNO_NAME_NRULE_START_WITH)))
        {
            sal_Int16 nStart = sal_Int16();
            if( aVal >>= nStart )
            {
                aFmt.SetStart( nStart );
                continue;
            }
        }
        else if(rPropName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM(UNO_NAME_NRULE_LEFT_MARGIN)))
        {
            sal_Int32 nMargin = 0;
            if( aVal >>= nMargin )
            {
                aFmt.SetAbsLSpace((sal_uInt16)nMargin);
                continue;
            }
        }
        else if(rPropName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM(UNO_NAME_NRULE_FIRST_LINE_OFFSET)))
        {
            sal_Int32 nMargin = 0;
            if( aVal >>= nMargin )
            {
                aFmt.SetFirstLineOffset((sal_uInt16)nMargin);
                continue;
            }
        }
        else if(rPropName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM("SymbolTextDistance")))
        {
            sal_Int32 nTextDistance = 0;
            if( aVal >>= nTextDistance )
            {
                aFmt.SetCharTextDistance((sal_uInt16)nTextDistance);
                continue;
            }
        }
        else if(rPropName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM(UNO_NAME_NRULE_BULLET_COLOR)))
        {
            sal_Int32 nColor = 0;
            if( aVal >>= nColor )
            {
                aFmt.SetBulletColor( (Color) nColor );
                continue;
            }
        }
        else if(rPropName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM(UNO_NAME_NRULE_BULLET_RELSIZE)))
        {
            sal_Int16 nSize = sal_Int16();
            if( aVal >>= nSize )
            {
                aFmt.SetBulletRelSize( (short)nSize );
                continue;
            }
        }
        else
        {
            continue;
        }

        throw IllegalArgumentException();
    }

    // check that we always have a brush item for bitmap numbering
    if( aFmt.GetNumberingType() == SVX_NUM_BITMAP )
    {
        if( NULL == aFmt.GetBrush() )
        {
            GraphicObject aGrafObj;
            SvxBrushItem aBrushItem( aGrafObj, GPOS_AREA, SID_ATTR_BRUSH );
            aFmt.SetGraphicBrush( &aBrushItem );
        }
    }
    maRule.SetLevel( (sal_uInt16)nIndex, aFmt );
}

///////////////////////////////////////////////////////////////////////

const SvxNumRule& SvxGetNumRule( Reference< XIndexReplace > xRule ) throw( IllegalArgumentException )
{
    SvxUnoNumberingRules* pRule = SvxUnoNumberingRules::getImplementation( xRule );
    if( pRule == NULL )
        throw IllegalArgumentException();

    return pRule->getNumRule();
}

bool SvxGetNumRule( Reference< XIndexReplace > xRule, SvxNumRule& rNumRule )
{
    SvxUnoNumberingRules* pRule = SvxUnoNumberingRules::getImplementation( xRule );
    if( pRule )
    {
        rNumRule = pRule->getNumRule();
    }
    else if( xRule.is() )
    {
        try
        {
            pRule = new SvxUnoNumberingRules( rNumRule );

            Reference< XIndexReplace > xDestRule( pRule );

            const sal_Int32 nCount = min( xRule->getCount(), xDestRule->getCount() );
            sal_Int32 nLevel;
            for( nLevel = 0; nLevel < nCount; nLevel++ )
            {
                xDestRule->replaceByIndex( nLevel, xRule->getByIndex( nLevel ) );
            }

            rNumRule = pRule->getNumRule();
        }
        catch( Exception& )
        {
            return false;
        }
    }
    else
    {
        return false;
    }

    return true;
}

///////////////////////////////////////////////////////////////////////
com::sun::star::uno::Reference< com::sun::star::container::XIndexReplace > SvxCreateNumRule( const SvxNumRule* pRule ) throw()
{
    DBG_ASSERT( pRule, "No default SvxNumRule!" );
    if( pRule )
    {
        return new SvxUnoNumberingRules( *pRule );
    }
    else
    {
        SvxNumRule aDefaultRule( NUM_BULLET_REL_SIZE|NUM_BULLET_COLOR|NUM_CHAR_TEXT_DISTANCE, 10 , sal_False);
        return new SvxUnoNumberingRules( aDefaultRule );
    }
}


///////////////////////////////////////////////////////////////////////

class SvxUnoNumberingRulesCompare : public ::cppu::WeakAggImplHelper1< XAnyCompare >
{
public:
    virtual sal_Int16 SAL_CALL compare( const Any& Any1, const Any& Any2 ) throw(RuntimeException);
};

sal_Int16 SAL_CALL SvxUnoNumberingRulesCompare::compare( const Any& Any1, const Any& Any2 ) throw(RuntimeException)
{
    return SvxUnoNumberingRules::Compare( Any1, Any2 );
}

sal_Int16 SvxUnoNumberingRules::Compare( const Any& Any1, const Any& Any2 )
{
    Reference< XIndexReplace > x1( Any1, UNO_QUERY ), x2( Any2, UNO_QUERY );
    if( x1.is() && x2.is() )
    {
        if( x1.get() == x2.get() )
            return 0;

        SvxUnoNumberingRules* pRule1 = SvxUnoNumberingRules::getImplementation( x1 );
        if( pRule1 )
        {
            SvxUnoNumberingRules* pRule2 = SvxUnoNumberingRules::getImplementation( x2 );
            if( pRule2 )
            {
                const SvxNumRule& rRule1 = pRule1->getNumRule();
                const SvxNumRule& rRule2 = pRule2->getNumRule();

                const sal_uInt16 nLevelCount1 = rRule1.GetLevelCount();
                const sal_uInt16 nLevelCount2 = rRule2.GetLevelCount();

                if( nLevelCount1 == 0 || nLevelCount2 == 0 )
                    return -1;

                for( sal_uInt16 i = 0; (i < nLevelCount1) && (i < nLevelCount2); i++ )
                {
                    if( rRule1.GetLevel(i) != rRule2.GetLevel(i) )
                        return -1;
                }
                return  0;
            }
        }
    }

    return -1;
}

Reference< XAnyCompare > SvxCreateNumRuleCompare() throw()
{
    return new SvxUnoNumberingRulesCompare();
}

::com::sun::star::uno::Reference< ::com::sun::star::container::XIndexReplace > SvxCreateNumRule() throw()
{
    SvxNumRule aTempRule( 0, 10, false );
    return SvxCreateNumRule( &aTempRule );
}
