/*************************************************************************
 *
 *  $RCSfile: unonrule.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 17:01:27 $
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

#ifndef _COM_SUN_STAR_TEXT_RELORIENTATION_HPP_
#include <com/sun/star/text/RelOrientation.hpp>
#endif
#ifndef _COM_SUN_STAR_TEXT_VERTORIENTATION_HPP_
#include <com/sun/star/text/VertOrientation.hpp>
#endif
#ifndef _COM_SUN_STAR_TEXT_HORIZONTALADJUST_HPP_
#include <com/sun/star/text/HorizontalAdjust.hpp>
#endif
#ifndef _COM_SUN_STAR_TEXT_DOCUMENTSTATISTIC_HPP_
#include <com/sun/star/text/DocumentStatistic.hpp>
#endif
#ifndef _COM_SUN_STAR_TEXT_HORIORIENTATION_HPP_
#include <com/sun/star/text/HoriOrientation.hpp>
#endif
#ifndef _COM_SUN_STAR_TEXT_HORIORIENTATIONFORMAT_HPP_
#include <com/sun/star/text/HoriOrientationFormat.hpp>
#endif
#ifndef _COM_SUN_STAR_TEXT_NOTEPRINTMODE_HPP_
#include <com/sun/star/text/NotePrintMode.hpp>
#endif
#ifndef _COM_SUN_STAR_TEXT_SIZETYPE_HPP_
#include <com/sun/star/text/SizeType.hpp>
#endif
#ifndef _COM_SUN_STAR_TEXT_VERTORIENTATIONFORMAT_HPP_
#include <com/sun/star/text/VertOrientationFormat.hpp>
#endif
#ifndef _COM_SUN_STAR_TEXT_WRAPTEXTMODE_HPP_
#include <com/sun/star/text/WrapTextMode.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_XBITMAP_HPP_
#include <com/sun/star/awt/XBitmap.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_PROPERTYSTATE_HPP_
#include <com/sun/star/beans/PropertyState.hpp>
#endif
#ifndef _GRFMGR_HXX
#include <goodies/grfmgr.hxx>
#endif

#ifndef _SVX_NUMITEM_HXX
#include <numitem.hxx>
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

#include <toolkit/unohlp.hxx>

#include <rtl/uuid.h>
#include <rtl/memory.h>

#include "unoprnms.hxx"
#include "unofdesc.hxx"
#include "unonrule.hxx"
#include "unotext.hxx"
#include "svdmodel.hxx"
#include "unoapi.hxx"

using namespace ::com::sun::star;
using namespace ::rtl;
using namespace ::vos;

/******************************************************************
 * SvxUnoNumberingRules
 ******************************************************************/

UNO3_GETIMPLEMENTATION_IMPL( SvxUnoNumberingRules );

SvxUnoNumberingRules::SvxUnoNumberingRules( SdrModel* pModel ) throw() :
    pNumRule(0)
{
    if( pModel )
    {
        SvxNumRule* pDefaultRule = ((SvxNumBulletItem*) pModel->GetItemPool().GetSecondaryPool()->GetPoolDefaultItem(EE_PARA_NUMBULLET))->GetNumRule();
        DBG_ASSERT( pDefaultRule, "No default SvxNumRule!" );
        pNumRule = new SvxNumRule( *pDefaultRule );
    }
}

SvxUnoNumberingRules::SvxUnoNumberingRules(const SvxNumRule& rRule) throw() :
    pNumRule(new SvxNumRule(rRule))
{
}

SvxUnoNumberingRules::~SvxUnoNumberingRules() throw()
{
    delete pNumRule;
}


//XIndexReplace
void SAL_CALL SvxUnoNumberingRules::replaceByIndex( sal_Int32 Index, const uno::Any& Element )
    throw( lang::IllegalArgumentException, lang::IndexOutOfBoundsException, lang::WrappedTargetException, uno::RuntimeException )
{
    OGuard aGuard( Application::GetSolarMutex() );

    if(pNumRule == NULL)
        throw uno::RuntimeException();

    if(pNumRule->GetNumRuleType() == SVX_RULETYPE_PRESENTATION_NUMBERING)
        Index++;

    if( Index < 0 || Index >= pNumRule->GetLevelCount() )
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

    if(pNumRule == NULL)
        throw uno::RuntimeException();

    sal_Int32 nCount = pNumRule->GetLevelCount();
    if(pNumRule->GetNumRuleType() == SVX_RULETYPE_PRESENTATION_NUMBERING)
        nCount -= 1;

    return nCount;
}

uno::Any SAL_CALL SvxUnoNumberingRules::getByIndex( sal_Int32 Index )
    throw( lang::IndexOutOfBoundsException, lang::WrappedTargetException, uno::RuntimeException )
{
    OGuard aGuard( Application::GetSolarMutex() );

    if(pNumRule->GetNumRuleType() == SVX_RULETYPE_PRESENTATION_NUMBERING)
        Index++;

    if( Index < 0 || Index >= pNumRule->GetLevelCount() )
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
    return pNumRule != NULL;
}

uno::Sequence<beans::PropertyValue> SvxUnoNumberingRules::getNumberingRuleByIndex( sal_Int32 nIndex) const throw()
{
    if(pNumRule == NULL)
        return uno::Sequence< beans::PropertyValue >(0);

    //  NumberingRule aRule;
    const SvxNumberFormat& rFmt = pNumRule->GetLevel((sal_uInt16) nIndex);
    sal_uInt16 nIdx = 0;

    const int nProps = 14;
    beans::PropertyValue* pArray = new beans::PropertyValue[nProps];

    uno::Any aVal;
    {
        aVal <<= (sal_Int16)rFmt.GetNumType();
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
        aVal <<= (sal_Int16)rFmt.GetBulletChar();
        beans::PropertyValue aBulletProp( OUString(RTL_CONSTASCII_USTRINGPARAM(UNO_NAME_NRULE_BULLETID)), -1, aVal, beans::PropertyState_DIRECT_VALUE);
        pArray[nIdx++] = aBulletProp;
    }

    if( rFmt.GetBulletFont() )
    {
        awt::FontDescriptor aDesc;
        SvxUnoFontDescriptor::ConvertFromFont( *rFmt.GetBulletFont(), aDesc );
        aVal.setValue(&aDesc, ::getCppuType((const ::com::sun::star::awt::FontDescriptor*)0));
        pArray[nIdx++] = beans::PropertyValue( OUString(RTL_CONSTASCII_USTRINGPARAM(UNO_NAME_NRULE_BULLET_FONT)), -1, aVal, beans::PropertyState_DIRECT_VALUE);
    }

    {
        const SvxBrushItem* pBrush = rFmt.GetBrush();
        uno::Reference< ::com::sun::star::awt::XBitmap >  xBmp;

        if(pBrush && pBrush->GetGraphic())
        {
            const Graphic* pGraphic = pBrush->GetGraphic();
            if( pGraphic )
                xBmp = VCLUnoHelper::CreateBitmap( pGraphic->GetBitmapEx() );
        }

        aVal <<= xBmp;
        const beans::PropertyValue aGraphicProp( OUString(RTL_CONSTASCII_USTRINGPARAM("Graphic")), -1, aVal, beans::PropertyState_DIRECT_VALUE);
        pArray[nIdx++] = aGraphicProp;

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
    if(pNumRule == NULL)
        throw uno::RuntimeException();

    SvxNumberFormat aFmt(pNumRule->GetLevel( (sal_uInt16)nIndex ));
    const beans::PropertyValue* pPropArray = rProperties.getConstArray();
    for(int i = 0; i < rProperties.getLength(); i++)
    {
        const beans::PropertyValue& rProp = pPropArray[i];
        const OUString& rPropName = rProp.Name;
        const uno::Any& aVal = rProp.Value;

        if(rPropName.compareToAscii( RTL_CONSTASCII_STRINGPARAM(UNO_NAME_NRULE_NUMBERINGTYPE)) == 0)
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
                aFmt.SetNumType((SvxExtNumType)nSet);
                continue;
            }
        }
        else if(rPropName.compareToAscii( RTL_CONSTASCII_STRINGPARAM(UNO_NAME_NRULE_PREFIX)) == 0)
        {
            OUString aPrefix;
            if( aVal >>= aPrefix )
            {
                aFmt.SetPrefix(aPrefix);
                continue;
            }
        }
        else if(rPropName.compareToAscii( RTL_CONSTASCII_STRINGPARAM(UNO_NAME_NRULE_SUFFIX)) == 0)
        {
            OUString aSuffix;
            if( aVal >>= aSuffix )
            {
                aFmt.SetSuffix(aSuffix);
                continue;
            }
        }
        else if(rPropName.compareToAscii( RTL_CONSTASCII_STRINGPARAM(UNO_NAME_NRULE_BULLETID)) == 0)
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
        else if(rPropName.compareToAscii( RTL_CONSTASCII_STRINGPARAM(UNO_NAME_NRULE_ADJUST)) == 0)
        {
            sal_Int16 nAdjust;
            if( aVal >>= nAdjust )
            {
                aFmt.SetNumAdjust(ConvertUnoAdjust( (unsigned short)nAdjust ));
                continue;
            }
        }
        else if(rPropName.compareToAscii( RTL_CONSTASCII_STRINGPARAM(UNO_NAME_NRULE_BULLET_FONT)) == 0)
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
        else if(rPropName.compareToAscii( RTL_CONSTASCII_STRINGPARAM("Graphic")) == 0)
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
        else if(rPropName.compareToAscii( RTL_CONSTASCII_STRINGPARAM("GraphicURL")) == 0)
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
        else if(rPropName.compareToAscii( RTL_CONSTASCII_STRINGPARAM("GraphicSize")) == 0)
        {
            awt::Size aUnoSize;
            if( aVal >>= aUnoSize )
            {
                aFmt.SetGraphicSize( Size( aUnoSize.Width, aUnoSize.Height ) );
                continue;
            }
        }
        else if(rPropName.compareToAscii( RTL_CONSTASCII_STRINGPARAM(UNO_NAME_NRULE_START_WITH)) == 0)
        {
            sal_Int16 nStart;
            if( aVal >>= nStart )
            {
                aFmt.SetStart( nStart );
                continue;
            }
        }
        else if(rPropName.compareToAscii( RTL_CONSTASCII_STRINGPARAM(UNO_NAME_NRULE_LEFT_MARGIN)) == 0)
        {
            sal_Int32 nMargin;
            if( aVal >>= nMargin )
            {
                aFmt.SetAbsLSpace((sal_uInt16)nMargin);
                continue;
            }
        }
        else if(rPropName.compareToAscii( RTL_CONSTASCII_STRINGPARAM(UNO_NAME_NRULE_FIRST_LINE_OFFSET)) == 0)
        {
            sal_Int32 nMargin;
            if( aVal >>= nMargin )
            {
                aFmt.SetFirstLineOffset((sal_uInt16)nMargin);
                continue;
            }
        }
        else if(rPropName.compareToAscii( RTL_CONSTASCII_STRINGPARAM(UNO_NAME_NRULE_BULLET_COLOR)) == 0)
        {
            sal_Int32 nColor;
            if( aVal >>= nColor )
            {
                aFmt.SetBulletColor( (Color) nColor );
                continue;
            }
        }
        else if(rPropName.compareToAscii( RTL_CONSTASCII_STRINGPARAM(UNO_NAME_NRULE_BULLET_RELSIZE)) == 0)
        {
            sal_Int16 nSize;
            if( aVal >>= nSize )
            {
                aFmt.SetBulletRelSize( (short)nSize );
                continue;
            }
        }

        throw lang::IllegalArgumentException();
    }
}


