/*************************************************************************
 *
 *  $RCSfile: unonrule.cxx,v $
 *
 *  $Revision: 1.11 $
 *
 *  last change: $Author: os $ $Date: 2001-02-23 12:29:33 $
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

#define PROPERTY_NONE 0
#define ITEMID_BRUSH        0

#ifndef _SVX_BRSHITEM_HXX //autogen
#include <brshitem.hxx>
#endif

#ifndef _COM_SUN_STAR_LANG_XUNOTUNNEL_HPP_
#include <com/sun/star/lang/XUnoTunnel.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_XBITMAP_HPP_
#include <com/sun/star/awt/XBitmap.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_PROPERTYVALUE_HPP_
#include <com/sun/star/beans/PropertyValue.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XSERVICEINFO_HPP_
#include <com/sun/star/lang/XServiceInfo.hpp>
#endif

#ifndef _SFXITEMPOOL_HXX
#include <svtools/itempool.hxx>
#endif
#ifndef _GRFMGR_HXX
#include <goodies/grfmgr.hxx>
#endif

#ifndef _SV_SVAPP_HXX //autogen
#include <vcl/svapp.hxx>
#endif
#ifndef _VOS_MUTEX_HXX_ //autogen
#include <vos/mutex.hxx>
#endif
#ifndef _SV_GRAPH_HXX
#include <vcl/graph.hxx>
#endif

#include <unotools/servicehelper.hxx>
#include <toolkit/unohlp.hxx>
#include <rtl/uuid.h>
#include <rtl/memory.h>

#include "unoprnms.hxx"
#include "numitem.hxx"
#include "eeitem.hxx"
#include "unofdesc.hxx"
#include "unonrule.hxx"
#include "unotext.hxx"
#include "svdmodel.hxx"
#include "numitem.hxx"
#include "unoapi.hxx"

using namespace ::com::sun::star;
using namespace ::rtl;
using namespace ::vos;
using namespace ::std;

/******************************************************************
 * SvxUnoNumberingRules
 ******************************************************************/

#include <cppuhelper/implbase3.hxx>


class SvxUnoNumberingRules : public ::cppu::WeakAggImplHelper3< container::XIndexReplace, lang::XUnoTunnel, lang::XServiceInfo >
{
private:
    SvxNumRule maRule;
public:
    SvxUnoNumberingRules( const SvxNumRule& rRule ) throw();
    virtual ~SvxUnoNumberingRules() throw();

    UNO3_GETIMPLEMENTATION_DECL( SvxUnoNumberingRules )

    //XIndexReplace
    virtual void SAL_CALL replaceByIndex( sal_Int32 Index, const uno::Any& Element ) throw(lang::IllegalArgumentException, lang::IndexOutOfBoundsException, lang::WrappedTargetException, uno::RuntimeException);

    //XIndexAccess
    virtual sal_Int32 SAL_CALL getCount() throw(uno::RuntimeException) ;
    virtual uno::Any SAL_CALL getByIndex( sal_Int32 Index ) throw(lang::IndexOutOfBoundsException, lang::WrappedTargetException, uno::RuntimeException);

    //XElementAccess
    virtual uno::Type SAL_CALL getElementType() throw(uno::RuntimeException);
    virtual sal_Bool SAL_CALL hasElements() throw(uno::RuntimeException);

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName(  ) throw(uno::RuntimeException);
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) throw(uno::RuntimeException);
    virtual uno::Sequence< OUString > SAL_CALL getSupportedServiceNames(  ) throw(uno::RuntimeException);

    // intern
    uno::Sequence<beans::PropertyValue> getNumberingRuleByIndex( sal_Int32 nIndex) const throw();
    void setNumberingRuleByIndex( const uno::Sequence< beans::PropertyValue >& rProperties, sal_Int32 nIndex) throw( uno::RuntimeException, lang::IllegalArgumentException );

    const SvxNumRule& getNumRule() const { return maRule; }
};

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
    throw( lang::IllegalArgumentException, lang::IndexOutOfBoundsException, lang::WrappedTargetException, uno::RuntimeException )
{
    OGuard aGuard( Application::GetSolarMutex() );

    if(maRule.GetNumRuleType() == SVX_RULETYPE_PRESENTATION_NUMBERING)
        Index++;

    if( Index < 0 || Index >= maRule.GetLevelCount() )
        throw lang::IndexOutOfBoundsException();

    uno::Sequence< beans::PropertyValue > aSeq;

    if( !( Element >>= aSeq) )
        throw lang::IllegalArgumentException();
    setNumberingRuleByIndex( aSeq, Index );
}

//XIndexAccess
sal_Int32 SAL_CALL SvxUnoNumberingRules::getCount() throw( uno::RuntimeException )
{
    OGuard aGuard( Application::GetSolarMutex() );

    sal_Int32 nCount = maRule.GetLevelCount();
    if(maRule.GetNumRuleType() == SVX_RULETYPE_PRESENTATION_NUMBERING)
        nCount -= 1;

    return nCount;
}

uno::Any SAL_CALL SvxUnoNumberingRules::getByIndex( sal_Int32 Index )
    throw( lang::IndexOutOfBoundsException, lang::WrappedTargetException, uno::RuntimeException )
{
    OGuard aGuard( Application::GetSolarMutex() );

    if(maRule.GetNumRuleType() == SVX_RULETYPE_PRESENTATION_NUMBERING)
        Index++;

    if( Index < 0 || Index >= maRule.GetLevelCount() )
        throw lang::IndexOutOfBoundsException();

    uno::Sequence<beans::PropertyValue> aRet = getNumberingRuleByIndex(Index);
    return uno::Any( &aRet, getElementType() );
}

//XElementAccess
uno::Type SAL_CALL SvxUnoNumberingRules::getElementType()
    throw( uno::RuntimeException )
{
    return ::getCppuType(( const uno::Sequence< beans::PropertyValue >*)0);
}

sal_Bool SAL_CALL SvxUnoNumberingRules::hasElements() throw( uno::RuntimeException )
{
    return sal_True;
}

// XServiceInfo
sal_Char pSvxUnoNumberingRulesService[sizeof("com.sun.star.text.NumberingRules")] = "com.sun.star.text.NumberingRules";

OUString SAL_CALL SvxUnoNumberingRules::getImplementationName(  ) throw(uno::RuntimeException)
{
    return OUString( RTL_CONSTASCII_USTRINGPARAM( "SvxUnoNumberingRules" ) );
}

sal_Bool SAL_CALL SvxUnoNumberingRules::supportsService( const OUString& ServiceName ) throw(uno::RuntimeException)
{
    return ServiceName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( pSvxUnoNumberingRulesService ) );
}

uno::Sequence< OUString > SAL_CALL SvxUnoNumberingRules::getSupportedServiceNames(  ) throw(uno::RuntimeException)
{
    OUString aService( RTL_CONSTASCII_USTRINGPARAM( pSvxUnoNumberingRulesService ) );
    uno::Sequence< OUString > aSeq( &aService, 1 );
    return aSeq;
}

uno::Sequence<beans::PropertyValue> SvxUnoNumberingRules::getNumberingRuleByIndex( sal_Int32 nIndex) const throw()
{
    //  NumberingRule aRule;
    const SvxNumberFormat& rFmt = maRule.GetLevel((sal_uInt16) nIndex);
    sal_uInt16 nIdx = 0;

    const int nProps = 15;
    beans::PropertyValue* pArray = new beans::PropertyValue[nProps];

    uno::Any aVal;
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
    uno::Sequence< beans::PropertyValue> aSeq(pArray, nIdx);

    delete [] pArray;
    return aSeq;
}

void SvxUnoNumberingRules::setNumberingRuleByIndex( const uno::Sequence< beans::PropertyValue >& rProperties, sal_Int32 nIndex)
    throw( uno::RuntimeException, lang::IllegalArgumentException )
{
    SvxNumberFormat aFmt(maRule.GetLevel( (sal_uInt16)nIndex ));
    const beans::PropertyValue* pPropArray = rProperties.getConstArray();
    for(int i = 0; i < rProperties.getLength(); i++)
    {
        const beans::PropertyValue& rProp = pPropArray[i];
        const OUString& rPropName = rProp.Name;
        const uno::Any& aVal = rProp.Value;

        if(rPropName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM(UNO_NAME_NRULE_NUMBERINGTYPE)))
        {
            sal_Int16 nSet;
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
                aFmt.SetNumberingType((SvxExtNumType)nSet);
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
            sal_Int16 nSet;
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
                    continue;
                }
            }
        }
        else if(rPropName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM(UNO_NAME_NRULE_ADJUST)))
        {
            sal_Int16 nAdjust;
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
            uno::Reference< awt::XBitmap > xBmp;
            if( aVal >>= xBmp )
            {
                Graphic aGraf( VCLUnoHelper::GetBitmap( xBmp ) );
                SvxBrushItem aBrushItem(aGraf, GPOS_AREA);
                aFmt.SetGraphicBrush( &aBrushItem );
                continue;
            }
        }
        else if(rPropName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM("GraphicURL")))
        {
            OUString aURL;
            if( aVal >>= aURL )
            {
                GraphicObject aGrafObj( CreateGraphicObjectFromURL( aURL ) );
                SvxBrushItem aBrushItem( aGrafObj, GPOS_AREA );
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
            sal_Int16 nStart;
            if( aVal >>= nStart )
            {
                aFmt.SetStart( nStart );
                continue;
            }
        }
        else if(rPropName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM(UNO_NAME_NRULE_LEFT_MARGIN)))
        {
            sal_Int32 nMargin;
            if( aVal >>= nMargin )
            {
                aFmt.SetAbsLSpace((sal_uInt16)nMargin);
                continue;
            }
        }
        else if(rPropName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM(UNO_NAME_NRULE_FIRST_LINE_OFFSET)))
        {
            sal_Int32 nMargin;
            if( aVal >>= nMargin )
            {
                aFmt.SetFirstLineOffset((sal_uInt16)nMargin);
                continue;
            }
        }
        else if(rPropName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM("SymbolTextDistance")))
        {
            sal_Int32 nTextDistance;
            if( aVal >>= nTextDistance )
            {
                aFmt.SetCharTextDistance((sal_uInt16)nTextDistance);
                continue;
            }
        }
        else if(rPropName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM(UNO_NAME_NRULE_BULLET_COLOR)))
        {
            sal_Int32 nColor;
            if( aVal >>= nColor )
            {
                aFmt.SetBulletColor( (Color) nColor );
                continue;
            }
        }
        else if(rPropName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM(UNO_NAME_NRULE_BULLET_RELSIZE)))
        {
            sal_Int16 nSize;
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

        throw lang::IllegalArgumentException();
    }

    maRule.SetLevel( (sal_uInt16)nIndex, aFmt );
}

///////////////////////////////////////////////////////////////////////

const SvxNumRule& SvxGetNumRule( uno::Reference< container::XIndexReplace > xRule ) throw( lang::IllegalArgumentException )
{
    SvxUnoNumberingRules* pRule = SvxUnoNumberingRules::getImplementation( xRule );
    if( pRule == NULL )
        throw lang::IllegalArgumentException();

    return pRule->getNumRule();
}

///////////////////////////////////////////////////////////////////////

uno::Reference< container::XIndexReplace > SvxCreateNumRule( const SvxNumRule* pRule ) throw()
{
    DBG_ASSERT( pRule, "No default SvxNumRule!" );
    if( pRule )
    {
        return new SvxUnoNumberingRules( *pRule );
    }
    else
    {
        SvxNumRule aDefaultRule( NUM_BULLET_REL_SIZE|NUM_BULLET_COLOR|NUM_CHAR_TEXT_DISTANCE, 10 , FALSE);
        return new SvxUnoNumberingRules( aDefaultRule );
    }
}

///////////////////////////////////////////////////////////////////////

uno::Reference< container::XIndexReplace > SvxCreateNumRule( SdrModel* pModel ) throw()
{
    SvxNumRule* pDefaultRule = NULL;
    if( pModel )
    {
        SvxNumBulletItem* pItem = (SvxNumBulletItem*) pModel->GetItemPool().GetSecondaryPool()->GetPoolDefaultItem(EE_PARA_NUMBULLET);
        if( pItem )
        {
            pDefaultRule = pItem->GetNumRule();
        }
    }

    if( pDefaultRule )
    {
        return SvxCreateNumRule( pDefaultRule );
    }
    else
    {
        SvxNumRule aTempRule( 0, 10, false );
        return SvxCreateNumRule( &aTempRule );
    }
}

///////////////////////////////////////////////////////////////////////

#include <cppuhelper/implbase1.hxx>

class SvxUnoNumberingRulesCompare : public ::cppu::WeakAggImplHelper1< ucb::XAnyCompare >
{
public:
    virtual sal_Int16 SAL_CALL compare( const uno::Any& Any1, const uno::Any& Any2 ) throw(uno::RuntimeException);
};

sal_Int16 SAL_CALL SvxUnoNumberingRulesCompare::compare( const uno::Any& Any1, const uno::Any& Any2 ) throw(uno::RuntimeException)
{
    uno::Reference< container::XIndexReplace > x1, x2;
    Any1 >>= x1;
    Any2 >>= x2;
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

                const USHORT nLevelCount = rRule1.GetLevelCount();
                if( nLevelCount != rRule2.GetLevelCount() )
                    return -1;

                for(USHORT i = 0; i < nLevelCount; i++)
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

uno::Reference< ucb::XAnyCompare > SvxCreateNumRuleCompare() throw()
{
    return new SvxUnoNumberingRulesCompare();
}
