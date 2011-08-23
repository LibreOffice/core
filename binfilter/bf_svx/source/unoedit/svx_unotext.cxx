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

#ifndef _SV_SVAPP_HXX //autogen
#include <vcl/svapp.hxx>
#endif

#ifndef _COM_SUN_STAR_TEXT_HORIORIENTATION_HPP_
#include <com/sun/star/text/HoriOrientation.hpp>
#endif
#ifndef _COM_SUN_STAR_TEXT_CONTROLCHARACTER_HPP_
#include <com/sun/star/text/ControlCharacter.hpp>
#endif
#ifndef _COM_SUN_STAR_TEXT_XTEXTFIELD_HDL_
#include <com/sun/star/text/XTextField.hdl>
#endif



#include <eeitem.hxx>


#define ITEMID_FONT	EE_CHAR_FONTINFO

#define ITEMID_TABSTOP EE_PARA_TABS

#ifndef _SVDOBJ_HXX
#include "svdobj.hxx"
#endif

#ifndef _SFXINTITEM_HXX //autogen
#include <bf_svtools/intitem.hxx>
#endif

#define ITEMID_FIELD EE_FEATURE_FIELD
#include <rtl/uuid.h>
#include <rtl/memory.h>

#include "unoshtxt.hxx"
#include "unonrule.hxx"
#include "unofdesc.hxx"
#include "unofield.hxx"



#ifndef _SVX_ITEMDATA_HXX
#include <bf_svx/itemdata.hxx>
#endif

#include "flditem.hxx"
#include "unoshprp.hxx"
#include "numitem.hxx"
#include "editeng.hxx"
namespace binfilter {

using namespace ::rtl;
using namespace ::vos;
using namespace ::cppu;
using namespace ::com::sun::star;

#define QUERYINT( xint ) \
    if( rType == ::getCppuType((const uno::Reference< xint >*)0) ) \
        aAny <<= uno::Reference< xint >(this)

SfxItemPropertyMap aEmptyPropMap[] =
{
    {0,0}
};

extern const SfxItemPropertyMap* ImplGetSvxTextPortionPropertyMap();

// ====================================================================
// helper fuer Item/Property Konvertierung
// ====================================================================

// Numerierung
const unsigned short aSvxToUnoAdjust[] =
{
    text::HoriOrientation::LEFT,
    text::HoriOrientation::RIGHT,
    text::HoriOrientation::FULL,
    text::HoriOrientation::CENTER,
    text::HoriOrientation::FULL,
    text::HoriOrientation::LEFT
};

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

SvxAdjust ConvertUnoAdjust( unsigned short nAdjust ) throw()
{
    DBG_ASSERT( nAdjust <= 7, "Enum hat sich geaendert! [CL]" );
    return aUnoToSvxAdjust[nAdjust];
}

unsigned short ConvertUnoAdjust( SvxAdjust eAdjust ) throw()
{
    DBG_ASSERT( eAdjust <= 6, "Enum hat sich geaendert! [CL]" );
    return aSvxToUnoAdjust[eAdjust];
}

void GetSelection( struct ESelection& rSel, SvxTextForwarder* pForwarder ) throw()
{
    DBG_ASSERT( pForwarder, "I need a valid SvxTextForwarder!" );
    if( pForwarder )
    {
        sal_Int16 nParaCount = pForwarder->GetParagraphCount();
        if(nParaCount>0)
            nParaCount--;

        rSel = ESelection( 0,0, nParaCount, pForwarder->GetTextLen( nParaCount ));
    }
}

void CheckSelection( struct ESelection& rSel, SvxTextForwarder* pForwarder ) throw()
{
    DBG_ASSERT( pForwarder, "I need a valid SvxTextForwarder!" );
    if( pForwarder )
    {
        if( rSel.nStartPara == 0xffff )
        {
            ::binfilter::GetSelection( rSel, pForwarder );//STRIP008 			::GetSelection( rSel, pForwarder );
        }
        else
        {
            ESelection aMaxSelection;
            GetSelection( aMaxSelection, pForwarder );

            // check start position
            if( rSel.nStartPara < aMaxSelection.nStartPara )
            {
                rSel.nStartPara = aMaxSelection.nStartPara;
                rSel.nStartPos = aMaxSelection.nStartPos;
            }
            else if( rSel.nStartPara > aMaxSelection.nEndPara )
            {
                rSel.nStartPara = aMaxSelection.nEndPara;
                rSel.nStartPos = aMaxSelection.nEndPos;
            }
            else if( rSel.nStartPos  > pForwarder->GetTextLen( rSel.nStartPara ) )
            {
                rSel.nStartPos = pForwarder->GetTextLen( rSel.nStartPara );
            }

            // check end position
            if( rSel.nEndPara < aMaxSelection.nStartPara )
            {
                rSel.nEndPara = aMaxSelection.nStartPara;
                rSel.nEndPos = aMaxSelection.nStartPos;
            }
            else if( rSel.nEndPara > aMaxSelection.nEndPara )
            {
                rSel.nEndPara = aMaxSelection.nEndPara;
                rSel.nEndPos = aMaxSelection.nEndPos;
            }
            else if( rSel.nEndPos > pForwarder->GetTextLen( rSel.nEndPara ) )
            {
                rSel.nEndPos = pForwarder->GetTextLen( rSel.nEndPara );
            }
        }
    }
}

// ====================================================================
// class SvxUnoTextRangeBase
// ====================================================================

UNO3_GETIMPLEMENTATION_IMPL( SvxUnoTextRangeBase );

SvxUnoTextRangeBase::SvxUnoTextRangeBase( const SfxItemPropertyMap* _pMap ) throw()
: aPropSet(_pMap), pEditSource(NULL)
{
}

SvxUnoTextRangeBase::SvxUnoTextRangeBase( const SvxEditSource* pSource, const SfxItemPropertyMap* _pMap ) throw()
: aPropSet(_pMap)
{
    OGuard aGuard( Application::GetSolarMutex() );

    DBG_ASSERT(pSource,"SvxUnoTextRangeBase: I need a valid SvxEditSource!");

    pEditSource = pSource->Clone();
    ESelection aSelection;
    ::binfilter::GetSelection( aSelection, pEditSource->GetTextForwarder() );//STRIP008 ::GetSelection( aSelection, pEditSource->GetTextForwarder() );
    SetSelection( aSelection );
}

SvxUnoTextRangeBase::SvxUnoTextRangeBase( const SvxUnoTextRangeBase& rRange ) throw()
: aPropSet(rRange.getPropertyMap())
{
    OGuard aGuard( Application::GetSolarMutex() );

    pEditSource = rRange.pEditSource ? rRange.pEditSource->Clone() : NULL;

    SvxTextForwarder* pForwarder = pEditSource ? pEditSource->GetTextForwarder() : NULL;
    if( pForwarder )
    {
        aSelection  = rRange.aSelection;
        CheckSelection( aSelection, pForwarder );
    }
}

SvxUnoTextRangeBase::~SvxUnoTextRangeBase() throw()
{
    delete pEditSource;
}

void SvxUnoTextRangeBase::SetEditSource( SvxEditSource* pSource ) throw()
{
    DBG_ASSERT(pSource,"SvxUnoTextRangeBase: I need a valid SvxEditSource!");
    DBG_ASSERT(pEditSource==NULL,"SvxUnoTextRangeBase::SetEditSource called while SvxEditSource already set" );

    pEditSource = pSource;

    aSelection.nStartPara = 0xffff;
}

/** puts a field item with a copy of the given FieldData into the itemset
    corresponding with this range */
void SvxUnoTextRangeBase::attachField( const SvxFieldData* pData ) throw()
{
    OGuard aGuard( Application::GetSolarMutex() );

    if( pData )
    {
        SvxTextForwarder* pForwarder = pEditSource ? pEditSource->GetTextForwarder() : NULL;
        if( pForwarder )
        {
            SvxFieldItem aField( *pData );
            pForwarder->QuickInsertField( aField, aSelection );
        }
    }
}

void SvxUnoTextRangeBase::SetSelection( const ESelection& rSelection ) throw()
{
    OGuard aGuard( Application::GetSolarMutex() );

    aSelection = rSelection;
    CheckSelection( aSelection, pEditSource->GetTextForwarder() );
}

// Interface XTextRange ( XText )

uno::Reference< text::XTextRange > SAL_CALL SvxUnoTextRangeBase::getStart(void)
    throw( uno::RuntimeException )
{
    OGuard aGuard( Application::GetSolarMutex() );

    uno::Reference< text::XTextRange > xRange;

    SvxTextForwarder* pForwarder = pEditSource ? pEditSource->GetTextForwarder() : NULL;
    if( pForwarder )
    {

        CheckSelection( aSelection, pForwarder );

        SvxUnoTextBase* pText = SvxUnoTextBase::getImplementation( getText() );

        if(pText == NULL)
            throw uno::RuntimeException();

        SvxUnoTextRange* pRange = new SvxUnoTextRange( *pText );
        xRange = pRange;

        ESelection aNewSel = aSelection;
        aNewSel.nEndPara = aNewSel.nStartPara;
        aNewSel.nEndPos  = aNewSel.nStartPos;
        pRange->SetSelection( aNewSel );
    }

    return xRange;
}

uno::Reference< text::XTextRange > SAL_CALL SvxUnoTextRangeBase::getEnd(void)
    throw( uno::RuntimeException )
{
    OGuard aGuard( Application::GetSolarMutex() );

    uno::Reference< text::XTextRange > xRet;

    SvxTextForwarder* pForwarder = pEditSource ? pEditSource->GetTextForwarder() : NULL;
    if( pForwarder )
    {
        CheckSelection( aSelection, pForwarder );

        SvxUnoTextBase* pText = SvxUnoTextBase::getImplementation( getText() );

        if(pText == NULL)
            throw uno::RuntimeException();

        SvxUnoTextRange* pNew = new SvxUnoTextRange( *pText );
        xRet = pNew;

        ESelection aNewSel = aSelection;
        aNewSel.nStartPara = aNewSel.nEndPara;
        aNewSel.nStartPos  = aNewSel.nEndPos;
        pNew->SetSelection( aNewSel );
    }
    return xRet;
}

OUString SAL_CALL SvxUnoTextRangeBase::getString(void)
    throw( uno::RuntimeException )
{
    OGuard aGuard( Application::GetSolarMutex() );

    SvxTextForwarder* pForwarder = pEditSource ? pEditSource->GetTextForwarder() : NULL;
    if( pForwarder )
    {
        CheckSelection( aSelection, pForwarder );

        return pForwarder->GetText( aSelection );
    }
    else
    {
        const OUString aEmpty;
        return aEmpty;
    }
}

void SAL_CALL SvxUnoTextRangeBase::setString(const OUString& aString)
    throw( uno::RuntimeException )
{
    OGuard aGuard( Application::GetSolarMutex() );

    SvxTextForwarder* pForwarder = pEditSource ? pEditSource->GetTextForwarder() : NULL;
    if( pForwarder )
    {
        CheckSelection( aSelection, pForwarder );

        String aConverted( aString );
        aConverted.ConvertLineEnd( LINEEND_LF );		// Zeilenenden nur einfach zaehlen

        pForwarder->QuickInsertText( aConverted, aSelection );
        pEditSource->UpdateData();

        //	Selektion anpassen
        //!	Wenn die EditEngine bei QuickInsertText die Selektion zurueckgeben wuerde,
        //!	waer's einfacher...
        CollapseToStart();

        sal_uInt16 nLen = aConverted.Len();
        if (nLen)
            GoRight( nLen, sal_True );
    }
}

// Interface beans::XPropertySet
uno::Reference< beans::XPropertySetInfo > SAL_CALL SvxUnoTextRangeBase::getPropertySetInfo(void)
    throw( uno::RuntimeException )
{
    return aPropSet.getPropertySetInfo();
}

void SAL_CALL SvxUnoTextRangeBase::setPropertyValue(const OUString& PropertyName, const uno::Any& aValue)
    throw( beans::UnknownPropertyException, beans::PropertyVetoException, lang::IllegalArgumentException, lang::WrappedTargetException, uno::RuntimeException )
{
    _setPropertyValue( PropertyName, aValue, -1 );
}

void SAL_CALL SvxUnoTextRangeBase::_setPropertyValue( const OUString& PropertyName, const uno::Any& aValue, sal_Int32 nPara )
    throw( beans::UnknownPropertyException, beans::PropertyVetoException, lang::IllegalArgumentException, lang::WrappedTargetException, uno::RuntimeException )
{
    OGuard aGuard( Application::GetSolarMutex() );

    SvxTextForwarder* pForwarder = pEditSource ? pEditSource->GetTextForwarder() : NULL;
    if( pForwarder )
    {

        CheckSelection( aSelection, pForwarder );

        const SfxItemPropertyMap* pMap = SfxItemPropertyMap::GetByName(aPropSet.getPropertyMap(), PropertyName );
        if ( pMap )
        {
            ESelection aSel( GetSelection() );
            sal_Bool bParaAttrib = (pMap->nWID >= EE_PARA_START) && ( pMap->nWID <= EE_PARA_END );

            if( nPara == -1 && !bParaAttrib )
            {
                SfxItemSet aOldSet( pForwarder->GetAttribs( aSel ) );
                // we have a selection and no para attribute
                SfxItemSet aNewSet( *aOldSet.GetPool(), aOldSet.GetRanges() );

                setPropertyValue( pMap, aValue, aSelection, aOldSet, aNewSet );


                pForwarder->QuickSetAttribs( aNewSet, GetSelection() );
            }
            else
            {
                sal_Int32 nEndPara;

                if( nPara == -1 )
                {
                    nPara = aSel.nStartPara;
                    nEndPara = aSel.nEndPara;
                }
                else
                {
                    // only one paragraph
                    nEndPara = nPara;
                }

                while( nPara <= nEndPara )
                {
                    // we have a paragraph
                    SfxItemSet aSet( pForwarder->GetParaAttribs( (USHORT)nPara ) );
                    setPropertyValue( pMap, aValue, aSelection, aSet, aSet );
                    pForwarder->SetParaAttribs( (USHORT)nPara, aSet );
                    nPara++;
                }
            }

            GetEditSource()->UpdateData();
            return;
        }
    }

    throw beans::UnknownPropertyException();
}

void SvxUnoTextRangeBase::setPropertyValue( const SfxItemPropertyMap* pMap, const uno::Any& rValue, const ESelection& rSelection, const SfxItemSet& rOldSet, SfxItemSet& rNewSet ) throw( beans::UnknownPropertyException, lang::IllegalArgumentException )
{
    if(!SetPropertyValueHelper( rOldSet, pMap, rValue, rNewSet, &rSelection, (SvxTextEditSource*)GetEditSource() ))
    {
        //	Fuer Teile von zusammengesetzten Items mit mehreren Properties (z.B. Hintergrund)
        //	muss vorher das alte Item aus dem Dokument geholt werden
        rNewSet.Put(rOldSet.Get(pMap->nWID));			// altes Item in neuen Set
        aPropSet.setPropertyValue(pMap, rValue, rNewSet);
    }
}

sal_Bool SvxUnoTextRangeBase::SetPropertyValueHelper( const SfxItemSet& rOldSet, const SfxItemPropertyMap* pMap, const uno::Any& aValue, SfxItemSet& rNewSet, const ESelection* pSelection /* = NULL */, SvxTextEditSource* pEditSource /* = NULL*/ ) throw( uno::RuntimeException )
{
    switch( pMap->nWID )
    {
    case WID_FONTDESC:
        {
            awt::FontDescriptor aDesc;
            if(aValue >>= aDesc)
            {
                SvxUnoFontDescriptor::FillItemSet( aDesc, rNewSet );
                return sal_True;
            }
        }
        break;

    case EE_PARA_NUMBULLET:
        {
            uno::Reference< container::XIndexReplace > xRule;
            if( !aValue.hasValue() || ((aValue >>= xRule) && !xRule.is()) )
                return sal_True;

            return sal_False;
        }
        break;

    case WID_NUMLEVEL:
        {
            SvxTextForwarder* pForwarder = pEditSource? pEditSource->GetTextForwarder() : NULL;
            if(pForwarder && pSelection)
            {
                sal_Int16 nLevel;
                if( aValue >>= nLevel )
                {
                    // #101004# Call interface method instead of unsafe cast
                    if(! pForwarder->SetDepth( pSelection->nStartPara, nLevel ) )
                        throw lang::IllegalArgumentException();

                    return sal_True;
                }
            }
        }
        break;
    case EE_PARA_BULLETSTATE:
        {
            sal_Bool bBullet;
            if( aValue >>= bBullet )
            {
                SfxUInt16Item aItem( EE_PARA_BULLETSTATE, bBullet );
                rNewSet.Put(aItem);
                return sal_True;
            }
        }
        break;

    default:
        return sal_False;
    }

    throw lang::IllegalArgumentException();
    return sal_False;
}

uno::Any SAL_CALL SvxUnoTextRangeBase::getPropertyValue(const OUString& PropertyName)
    throw( beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException )
{
    return _getPropertyValue( PropertyName, -1 );
}

uno::Any SAL_CALL SvxUnoTextRangeBase::_getPropertyValue(const OUString& PropertyName, sal_Int32 nPara )
    throw( beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException )
{
    OGuard aGuard( Application::GetSolarMutex() );

    uno::Any aAny;

    SvxTextForwarder* pForwarder = pEditSource ? pEditSource->GetTextForwarder() : NULL;
    if( pForwarder )
    {
        const SfxItemPropertyMap* pMap = SfxItemPropertyMap::GetByName(aPropSet.getPropertyMap(), PropertyName );
        if( pMap )
        {
            SfxItemSet* pAttribs = NULL;
            if( nPara != -1 )
                pAttribs = pForwarder->GetParaAttribs( (USHORT)nPara ).Clone();
            else
                pAttribs = pForwarder->GetAttribs( GetSelection() ).Clone();

            //	Dontcare durch Default ersetzen, damit man immer eine Reflection hat
            pAttribs->ClearInvalidItems();

            getPropertyValue( pMap, aAny, *pAttribs );

            delete pAttribs;
            return aAny;
        }
    }

    throw beans::UnknownPropertyException();
    return aAny;
}

void SvxUnoTextRangeBase::getPropertyValue( const SfxItemPropertyMap* pMap, uno::Any& rAny, const SfxItemSet& rSet ) throw( beans::UnknownPropertyException )
{
    switch( pMap->nWID )
    {
    case EE_FEATURE_FIELD:
        if ( rSet.GetItemState( EE_FEATURE_FIELD, sal_False ) == SFX_ITEM_SET )
        {
            SvxFieldItem* pItem = (SvxFieldItem*)rSet.GetItem( EE_FEATURE_FIELD );
            const SvxFieldData* pData = pItem->GetField();
            uno::Reference< text::XTextRange > xAnchor( this );

            // get presentation string for field
            Color* pTColor = NULL;
            Color* pFColor = NULL;

            SvxTextForwarder* pForwarder = pEditSource->GetTextForwarder();
            OUString aPresentation( pForwarder->CalcFieldValue( *pData, aSelection.nStartPara, aSelection.nStartPos, pTColor, pFColor ) );

            delete pTColor;
            delete pFColor;

            uno::Reference< text::XTextField > xField( new SvxUnoTextField( xAnchor, aPresentation, pData ) );
            rAny <<= xField;
        }
        break;

    case WID_PORTIONTYPE:
        if ( rSet.GetItemState( EE_FEATURE_FIELD, sal_False ) == SFX_ITEM_SET )
        {
            OUString aType( RTL_CONSTASCII_USTRINGPARAM("TextField") );
            rAny <<= aType;
        }
        else
        {
            OUString aType( RTL_CONSTASCII_USTRINGPARAM("Text") );
            rAny <<= aType;
        }
        break;

    default:
        if(!GetPropertyValueHelper( *((SfxItemSet*)(&rSet)), pMap, rAny, &aSelection, (SvxTextEditSource*)GetEditSource() ))
            rAny = aPropSet.getPropertyValue(pMap, rSet);
    }
}

sal_Bool SvxUnoTextRangeBase::GetPropertyValueHelper(  SfxItemSet& rSet, const SfxItemPropertyMap* pMap, uno::Any& aAny, const ESelection* pSelection /* = NULL */, SvxTextEditSource* pEditSource /* = NULL */ )
    throw( uno::RuntimeException )
{
    switch( pMap->nWID )
    {
    case WID_FONTDESC:
        {
            awt::FontDescriptor aDesc;
            SvxUnoFontDescriptor::FillFromItemSet( rSet, aDesc );
            aAny <<= aDesc;
        }
        break;

    case EE_PARA_NUMBULLET:
        {
            if((rSet.GetItemState( EE_PARA_NUMBULLET, sal_True ) & (SFX_ITEM_SET|SFX_ITEM_DEFAULT)) == 0)
                throw uno::RuntimeException();

            SvxNumBulletItem* pBulletItem = (SvxNumBulletItem*)rSet.GetItem( EE_PARA_NUMBULLET, sal_True );

            if( pBulletItem == NULL )
                throw uno::RuntimeException();

            aAny <<= SvxCreateNumRule( pBulletItem->GetNumRule() );
        }
        break;

    case WID_NUMLEVEL:
        {
            SvxTextForwarder* pForwarder = pEditSource? pEditSource->GetTextForwarder() : NULL;
            if(pForwarder && pSelection)
            {
                // #101004# Call interface method instead of unsafe cast
                sal_Int16 nLevel( pForwarder->GetDepth( pSelection->nStartPara ) );
                aAny <<= nLevel;
            }
        }
        break;
    case EE_PARA_BULLETSTATE:
        {
            sal_Bool bState = sal_False;
            if( rSet.GetItemState( EE_PARA_BULLETSTATE, sal_True ) & (SFX_ITEM_SET|SFX_ITEM_DEFAULT))
            {
                SfxUInt16Item* pItem = (SfxUInt16Item*)rSet.GetItem( EE_PARA_BULLETSTATE, sal_True );
                bState = pItem->GetValue() == TRUE;
            }

            aAny.setValue( &bState, ::getCppuBooleanType() );
        }
        break;
    default:

        return sal_False;
    }

    return sal_True;
}

// wird (noch) nicht unterstuetzt
void SAL_CALL SvxUnoTextRangeBase::addPropertyChangeListener( const OUString& aPropertyName, const uno::Reference< beans::XPropertyChangeListener >& xListener ) throw(beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException) {}
void SAL_CALL SvxUnoTextRangeBase::removePropertyChangeListener( const OUString& aPropertyName, const uno::Reference< beans::XPropertyChangeListener >& aListener ) throw(beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException) {}
void SAL_CALL SvxUnoTextRangeBase::addVetoableChangeListener( const OUString& PropertyName, const uno::Reference< beans::XVetoableChangeListener >& aListener ) throw(beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException) {}
void SAL_CALL SvxUnoTextRangeBase::removeVetoableChangeListener( const OUString& PropertyName, const uno::Reference< beans::XVetoableChangeListener >& aListener ) throw(beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException) {}

// XMultiPropertySet
void SAL_CALL SvxUnoTextRangeBase::setPropertyValues( const uno::Sequence< ::rtl::OUString >& aPropertyNames, const uno::Sequence< uno::Any >& aValues ) throw (beans::PropertyVetoException, lang::IllegalArgumentException, lang::WrappedTargetException, uno::RuntimeException)
{
    _setPropertyValues( aPropertyNames, aValues, -1 );
}

void SAL_CALL SvxUnoTextRangeBase::_setPropertyValues( const uno::Sequence< ::rtl::OUString >& aPropertyNames, const uno::Sequence< uno::Any >& aValues, sal_Int32 nPara ) throw (beans::PropertyVetoException, lang::IllegalArgumentException, lang::WrappedTargetException, uno::RuntimeException)
{
    OGuard aGuard( Application::GetSolarMutex() );

    SvxTextForwarder* pForwarder = pEditSource ? pEditSource->GetTextForwarder() : NULL;
    if( pForwarder )
    {
        CheckSelection( aSelection, pForwarder );

        ESelection aSel( GetSelection() );

        sal_Bool bUnknownProperty = sal_False;

        const OUString* pPropertyNames = aPropertyNames.getConstArray();
        const uno::Any* pValues = aValues.getConstArray();
        sal_Int32 nCount = aPropertyNames.getLength();

        sal_Int32 nEndPara = nPara;
        sal_Int32 nTempPara = nPara;

        if( nTempPara == -1 )
        {
            nTempPara = aSel.nStartPara;
            nEndPara = aSel.nEndPara;
        }

        SfxItemSet* pOldAttrSet = NULL;
        SfxItemSet* pNewAttrSet = NULL;

        SfxItemSet* pOldParaSet = NULL;
        SfxItemSet* pNewParaSet = NULL;

        const SfxItemPropertyMap* pMap = aPropSet.getPropertyMap();

        for( ; nCount; nCount--, pPropertyNames++, pValues++ )
        {
            pMap = SfxItemPropertyMap::GetByName(pMap, *pPropertyNames );
            if( NULL == pMap )
            {
                bUnknownProperty = sal_True;
                break;
            }

            sal_Bool bParaAttrib = (pMap->nWID >= EE_PARA_START) && ( pMap->nWID <= EE_PARA_END );

            if( (nPara == -1) && !bParaAttrib )
            {
                if( NULL == pNewAttrSet )
                {
                    const SfxItemSet aSet( pForwarder->GetAttribs( aSel ) );
                    pOldAttrSet = new SfxItemSet( aSet );
                    pNewAttrSet = new SfxItemSet( *pOldAttrSet->GetPool(), pOldAttrSet->GetRanges() );
                }

                setPropertyValue( pMap, *pValues, GetSelection(), *pOldAttrSet, *pNewAttrSet );

                if( pMap->nWID >= EE_ITEMS_START && pMap->nWID <= EE_ITEMS_END )
                {
                    const SfxPoolItem* pItem;
                    if( pNewAttrSet->GetItemState( pMap->nWID, sal_True, &pItem ) == SFX_ITEM_SET )
                    {
                        pOldAttrSet->Put( *pItem );
                    }
                }
            }
            else
            {
                if( NULL == pNewParaSet )
                {
                    const SfxItemSet aSet( pForwarder->GetParaAttribs( (USHORT)nTempPara ) );
                    pOldParaSet = new SfxItemSet( aSet );
                    pNewParaSet = new SfxItemSet( *pOldParaSet->GetPool(), pOldParaSet->GetRanges() );
                }

                setPropertyValue( pMap, *pValues, GetSelection(), *pOldParaSet, *pNewParaSet );

                if( pMap->nWID >= EE_ITEMS_START && pMap->nWID <= EE_ITEMS_END )
                {
                    const SfxPoolItem* pItem;
                    if( pNewParaSet->GetItemState( pMap->nWID, sal_True, &pItem ) == SFX_ITEM_SET )
                    {
                        pOldParaSet->Put( *pItem );
                    }
                }

            }

            if (pMap)
                pMap++;
            else
                pMap = aPropSet.getPropertyMap();
        }

        if( !bUnknownProperty )
        {
            sal_Bool bNeedsUpdate = sal_False;

            if( pNewParaSet )
            {

                if( pNewParaSet->Count() )
                {
                    while( nTempPara <= nEndPara )
                    {
                        SfxItemSet aSet( pForwarder->GetParaAttribs( (USHORT)nTempPara ) );
                        aSet.Put( *pNewParaSet );
                        pForwarder->SetParaAttribs( (USHORT)nTempPara, aSet );
                        nTempPara++;
                    }
                    bNeedsUpdate = sal_True;
                }

                delete pNewParaSet;
                delete pOldParaSet;
            }

            if( pNewAttrSet )
            {
                if( pNewAttrSet->Count() )
                {
                    pForwarder->QuickSetAttribs( *pNewAttrSet, GetSelection() );
                    bNeedsUpdate = sal_True;
                }
                delete pNewAttrSet;
                delete pOldAttrSet;

            }

            if( bNeedsUpdate )
                GetEditSource()->UpdateData();
        }

        if( bUnknownProperty )
            throw beans::UnknownPropertyException();
    }
}

uno::Sequence< uno::Any > SAL_CALL SvxUnoTextRangeBase::getPropertyValues( const uno::Sequence< ::rtl::OUString >& aPropertyNames ) throw (uno::RuntimeException)
{
    return _getPropertyValues( aPropertyNames, -1 );
}

uno::Sequence< uno::Any > SAL_CALL SvxUnoTextRangeBase::_getPropertyValues( const uno::Sequence< ::rtl::OUString >& aPropertyNames, sal_Int32 nPara ) throw (uno::RuntimeException)
{
    OGuard aGuard( Application::GetSolarMutex() );

    sal_Int32 nCount = aPropertyNames.getLength();


    uno::Sequence< uno::Any > aValues( nCount );

    SvxTextForwarder* pForwarder = pEditSource ? pEditSource->GetTextForwarder() : NULL;
    if( pForwarder )
    {
        SfxItemSet* pAttribs = NULL;
        if( nPara != -1 )
            pAttribs = pForwarder->GetParaAttribs( (USHORT)nPara ).Clone();
        else
            pAttribs = pForwarder->GetAttribs( GetSelection() ).Clone();

        pAttribs->ClearInvalidItems();

        const OUString* pPropertyNames = aPropertyNames.getConstArray();
        uno::Any* pValues = aValues.getArray();

        const SfxItemPropertyMap* pMap = aPropSet.getPropertyMap();

        for( ; nCount; nCount--, pPropertyNames++, pValues++ )
        {
            pMap = SfxItemPropertyMap::GetByName(pMap, *pPropertyNames );
            if( NULL == pMap )
                throw beans::UnknownPropertyException();

            getPropertyValue( pMap, *pValues, *pAttribs );

            if (pMap)
                pMap++;
            else
                pMap = aPropSet.getPropertyMap();
        }

        delete pAttribs;

    }

    return aValues;
}

void SAL_CALL SvxUnoTextRangeBase::addPropertiesChangeListener( const uno::Sequence< ::rtl::OUString >& aPropertyNames, const uno::Reference< beans::XPropertiesChangeListener >& xListener ) throw (uno::RuntimeException)
{
}

void SAL_CALL SvxUnoTextRangeBase::removePropertiesChangeListener( const uno::Reference< beans::XPropertiesChangeListener >& xListener ) throw (uno::RuntimeException)
{
}

void SAL_CALL SvxUnoTextRangeBase::firePropertiesChangeEvent( const uno::Sequence< ::rtl::OUString >& aPropertyNames, const uno::Reference< beans::XPropertiesChangeListener >& xListener ) throw (uno::RuntimeException)
{
}

// beans::XPropertyState
beans::PropertyState SAL_CALL SvxUnoTextRangeBase::getPropertyState( const OUString& PropertyName )
    throw(beans::UnknownPropertyException, uno::RuntimeException)
{
    return _getPropertyState( PropertyName, -1 );
}

static sal_uInt16 aSvxUnoFontDescriptorWhichMap[] = { EE_CHAR_FONTINFO, EE_CHAR_FONTHEIGHT, EE_CHAR_ITALIC,
                                                  EE_CHAR_UNDERLINE, EE_CHAR_WEIGHT, EE_CHAR_STRIKEOUT,
                                                  EE_CHAR_WLM, 0 };

beans::PropertyState SAL_CALL SvxUnoTextRangeBase::_getPropertyState(const OUString& PropertyName, sal_Int32 nPara /* = -1 */)
    throw( beans::UnknownPropertyException, uno::RuntimeException )
{
    OGuard aGuard( Application::GetSolarMutex() );

    SvxTextForwarder* pForwarder = pEditSource ? pEditSource->GetTextForwarder() : NULL;
    if( pForwarder )
    {
        const SfxItemPropertyMap* pMap = SfxItemPropertyMap::GetByName(aPropSet.getPropertyMap(), PropertyName );
        if ( pMap )
        {
            SfxItemState eItemState = SFX_ITEM_UNKNOWN;
            sal_uInt16 nWID = 0;

            switch( pMap->nWID )
            {
            case WID_FONTDESC:
                {
                    sal_uInt16* pWhichId = aSvxUnoFontDescriptorWhichMap;
                    SfxItemState eTempItemState;
                    while( *pWhichId )
                    {
                        if(nPara != -1)
                            eTempItemState = pForwarder->GetItemState( (USHORT)nPara, *pWhichId );
                        else
                            eTempItemState = pForwarder->GetItemState( GetSelection(), *pWhichId );

                        switch( eTempItemState )
                        {
                        case SFX_ITEM_DISABLED:
                        case SFX_ITEM_DONTCARE:
                            eItemState = SFX_ITEM_DONTCARE;
                            break;

                        case SFX_ITEM_DEFAULT:
                            if( eItemState != SFX_ITEM_DEFAULT )
                            {
                                if( eItemState == SFX_ITEM_UNKNOWN )
                                    eItemState = SFX_ITEM_DEFAULT;
                            }
                            break;

                        case SFX_ITEM_READONLY:
                        case SFX_ITEM_SET:
                            if( eItemState != SFX_ITEM_SET )
                            {
                                if( eItemState == SFX_ITEM_UNKNOWN )
                                    eItemState = SFX_ITEM_SET;
                            }
                            break;
                        default:
                            throw beans::UnknownPropertyException();
                        }

                        pWhichId++;
                    }
                }
                break;

            case WID_NUMLEVEL:
                eItemState = SFX_ITEM_SET;
                break;

            default:
                nWID = pMap->nWID;
            }

            if( nWID != 0 )
            {
                if( nPara != -1 )
                    eItemState = pForwarder->GetItemState( (USHORT)nPara, nWID );
                else
                    eItemState = pForwarder->GetItemState( GetSelection(), nWID );
            }

            switch( eItemState )
            {
            case SFX_ITEM_DONTCARE:
            case SFX_ITEM_DISABLED:
                return beans::PropertyState_AMBIGUOUS_VALUE;
            case SFX_ITEM_READONLY:
            case SFX_ITEM_SET:
                return beans::PropertyState_DIRECT_VALUE;
            case SFX_ITEM_DEFAULT:
                return beans::PropertyState_DEFAULT_VALUE;
//			case SFX_ITEM_UNKNOWN:
            }
        }
    }
    throw beans::UnknownPropertyException();
}

uno::Sequence< beans::PropertyState > SAL_CALL SvxUnoTextRangeBase::getPropertyStates( const uno::Sequence< OUString >& aPropertyName )
    throw(beans::UnknownPropertyException, uno::RuntimeException)
{
    return _getPropertyStates( aPropertyName, -1 );
}

uno::Sequence< beans::PropertyState > SvxUnoTextRangeBase::_getPropertyStates(const uno::Sequence< OUString >& PropertyName, sal_Int32 nPara /* = -1 */)
    throw( beans::UnknownPropertyException, uno::RuntimeException )
{
    const sal_Int32 nCount = PropertyName.getLength();
    const OUString* pNames = PropertyName.getConstArray();

    uno::Sequence< beans::PropertyState > aRet( nCount );
    beans::PropertyState* pState = aRet.getArray();



    const sal_Bool bPara = nPara != -1;

    SvxTextForwarder* pForwarder = pEditSource ? pEditSource->GetTextForwarder() : NULL;
    if( pForwarder )
    {
        SfxItemSet* pSet = NULL;
        if( nPara != -1 )
        {
            const SfxItemSet aSet( pForwarder->GetParaAttribs( (USHORT)nPara ) );
            pSet = new SfxItemSet( aSet );
        }
        else
        {
            ESelection aSel( GetSelection() );
            CheckSelection( aSel, pForwarder );
            const SfxItemSet aSet(pForwarder->GetAttribs( aSel, EditEngineAttribs_OnlyHard ) );
            pSet = new SfxItemSet( aSet );
        }

        sal_Bool bUnknownPropertyFound = sal_False;
        const SfxItemPropertyMap* pMap = aPropSet.getPropertyMap();
        for( sal_Int32 nIdx = 0; nIdx < nCount; nIdx++ )
        {
            pMap = SfxItemPropertyMap::GetByName(pMap, *pNames++ );
            if( NULL == pMap )
            {
                bUnknownPropertyFound = sal_True;
                break;
            }

            SfxItemState eItemState = SFX_ITEM_UNKNOWN;
            sal_uInt16 nWID = 0;

            switch( pMap->nWID )
            {
                case WID_FONTDESC:
                    {
                        sal_uInt16* pWhichId = aSvxUnoFontDescriptorWhichMap;
                        SfxItemState eTempItemState;
                        while( *pWhichId )
                        {
                            eTempItemState = pSet->GetItemState( *pWhichId );

                            switch( eTempItemState )
                            {
                            case SFX_ITEM_DISABLED:
                            case SFX_ITEM_DONTCARE:
                                eItemState = SFX_ITEM_DONTCARE;
                                break;

                            case SFX_ITEM_DEFAULT:
                                if( eItemState != SFX_ITEM_DEFAULT )
                                {
                                    if( eItemState == SFX_ITEM_UNKNOWN )
                                        eItemState = SFX_ITEM_DEFAULT;
                                }
                                break;

                            case SFX_ITEM_READONLY:
                            case SFX_ITEM_SET:
                                if( eItemState != SFX_ITEM_SET )
                                {
                                    if( eItemState == SFX_ITEM_UNKNOWN )
                                        eItemState = SFX_ITEM_SET;
                                }
                                break;
                            default:
                                bUnknownPropertyFound = sal_True;
                                break;
                            }

                            pWhichId++;
                        }
                    }
                    break;

                case WID_NUMLEVEL:
                    eItemState = SFX_ITEM_SET;
                    break;

                default:
                    nWID = pMap->nWID;
            }

            if( bUnknownPropertyFound )
                break;

            if( nWID != 0 )
                eItemState = pSet->GetItemState( nWID, sal_False );

            switch( eItemState )
            {
                    case SFX_ITEM_READONLY:
                    case SFX_ITEM_SET:
                        *pState++ = beans::PropertyState_DIRECT_VALUE;
                        break;
                    case SFX_ITEM_DEFAULT:
                        *pState++ = beans::PropertyState_DEFAULT_VALUE;
                        break;
//					case SFX_ITEM_UNKNOWN:
//					case SFX_ITEM_DONTCARE:
//					case SFX_ITEM_DISABLED:
                    default:
                        *pState++ = beans::PropertyState_AMBIGUOUS_VALUE;
            }

            if (pMap)
                pMap++;
            else
                pMap = aPropSet.getPropertyMap();
        }

        delete pSet;

        if( bUnknownPropertyFound )
            throw beans::UnknownPropertyException();
    }

    return aRet;
}

void SAL_CALL SvxUnoTextRangeBase::setPropertyToDefault( const OUString& PropertyName )
    throw(beans::UnknownPropertyException, uno::RuntimeException)
{
    _setPropertyToDefault( PropertyName, -1 );
}

void SvxUnoTextRangeBase::_setPropertyToDefault(const OUString& PropertyName, sal_Int32 nPara /* = -1 */)
    throw( beans::UnknownPropertyException, uno::RuntimeException )
{
    OGuard aGuard( Application::GetSolarMutex() );

    do
    {
        SvxTextForwarder* pForwarder = pEditSource ? pEditSource->GetTextForwarder() : NULL;

        if( NULL == pForwarder )
            break;

        CheckSelection( aSelection, pEditSource->GetTextForwarder() );

        const SfxItemPropertyMap* pMap = SfxItemPropertyMap::GetByName(aPropSet.getPropertyMap(), PropertyName );
        if( NULL == pMap )
            break;

        SfxItemSet aSet( *pForwarder->GetPool(), TRUE );

        if( pMap->nWID == WID_FONTDESC )
        {
            SvxUnoFontDescriptor::setPropertyToDefault( aSet );
        }
        else if( pMap->nWID == WID_NUMLEVEL )
        {
            // #101004# Call interface method instead of unsafe cast
            pForwarder->SetDepth( aSelection.nStartPara, 0 );
            return;
        }
        else
        {
            aSet.InvalidateItem( pMap->nWID );
        }

        if(nPara != -1)
            pForwarder->SetParaAttribs( (USHORT)nPara, aSet );
        else
            pForwarder->QuickSetAttribs( aSet, GetSelection() );

        GetEditSource()->UpdateData();

        return;
    }
    while(0);

    throw beans::UnknownPropertyException();
}

uno::Any SAL_CALL SvxUnoTextRangeBase::getPropertyDefault( const OUString& aPropertyName )
    throw(beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException)
{
    OGuard aGuard( Application::GetSolarMutex() );

    SvxTextForwarder* pForwarder = pEditSource ? pEditSource->GetTextForwarder() : NULL;
    if( pForwarder )
    {
        const SfxItemPropertyMap* pMap = SfxItemPropertyMap::GetByName(aPropSet.getPropertyMap(), aPropertyName );
        if( pMap )
        {
            SfxItemPool* pPool = pForwarder->GetPool();

            switch( pMap->nWID )
            {
            case WID_FONTDESC:
                return SvxUnoFontDescriptor::getPropertyDefault( pPool );

            case WID_NUMLEVEL:
                {
                    uno::Any aValue;
                    aValue <<= (sal_Int16)0;
                    return aValue;
                }
            default:
                {
                    // Default aus ItemPool holen
                    if(pPool->IsWhich(pMap->nWID))
                    {
                        SfxItemSet aSet( *pPool,	pMap->nWID, pMap->nWID);
                        aSet.Put(pPool->GetDefaultItem(pMap->nWID));
                        return aPropSet.getPropertyValue(pMap, aSet);
                    }
                }
            }
        }
    }
    throw beans::UnknownPropertyException();
}

// internal
void SvxUnoTextRangeBase::CollapseToStart(void) throw()
{
    CheckSelection( aSelection, pEditSource->GetTextForwarder() );

    aSelection.nEndPara = aSelection.nStartPara;
    aSelection.nEndPos  = aSelection.nStartPos;
}

void SvxUnoTextRangeBase::CollapseToEnd(void) throw()
{
    CheckSelection( aSelection, pEditSource->GetTextForwarder() );

    aSelection.nStartPara = aSelection.nEndPara;
    aSelection.nStartPos  = aSelection.nEndPos;
}

sal_Bool SvxUnoTextRangeBase::IsCollapsed(void) throw()
{
    CheckSelection( aSelection, pEditSource->GetTextForwarder() );

    return ( aSelection.nStartPara == aSelection.nEndPara &&
             aSelection.nStartPos  == aSelection.nEndPos );
}

sal_Bool SvxUnoTextRangeBase::GoLeft(sal_Int16 nCount, sal_Bool Expand) throw()
{
    CheckSelection( aSelection, pEditSource->GetTextForwarder() );

    //	#75098# use end position, as in Writer (start is anchor, end is cursor)
    sal_uInt16 nNewPos = aSelection.nEndPos;
    sal_uInt16 nNewPar = aSelection.nEndPara;

    sal_Bool bOk = sal_True;
    SvxTextForwarder* pForwarder = NULL;
    while ( nCount > nNewPos && bOk )
    {
        if ( nNewPar == 0 )
            bOk = sal_False;
        else
        {
            if ( !pForwarder )
                pForwarder = pEditSource->GetTextForwarder();	// erst hier, wenn's noetig ist...

            --nNewPar;
            nCount -= nNewPos + 1;
            nNewPos = pForwarder->GetTextLen( nNewPar );
        }
    }

    if ( bOk )
    {
        nNewPos -= nCount;
        aSelection.nStartPara = nNewPar;
        aSelection.nStartPos  = nNewPos;
    }

    if (!Expand)
        CollapseToStart();

    return bOk;
}

sal_Bool SvxUnoTextRangeBase::GoRight(sal_Int16 nCount, sal_Bool Expand)  throw()
{
    SvxTextForwarder* pForwarder = pEditSource ? pEditSource->GetTextForwarder() : NULL;
    if( pForwarder )
    {
        CheckSelection( aSelection, pForwarder );


        sal_uInt16 nNewPos = aSelection.nEndPos + nCount;			//! Ueberlauf ???
        sal_uInt16 nNewPar = aSelection.nEndPara;

        sal_Bool bOk = sal_True;
        sal_uInt16 nParCount = pForwarder->GetParagraphCount();
        sal_uInt16 nThisLen = pForwarder->GetTextLen( nNewPar );
        while ( nNewPos > nThisLen && bOk )
        {
            if ( nNewPar + 1 >= nParCount )
                bOk = sal_False;
            else
            {
                nNewPos -= nThisLen+1;
                ++nNewPar;
                nThisLen = pForwarder->GetTextLen( nNewPar );
            }
        }

        if (bOk)
        {
            aSelection.nEndPara = nNewPar;
            aSelection.nEndPos  = nNewPos;
        }

        if (!Expand)
            CollapseToEnd();

        return bOk;
    }
    return sal_False;
}

void SvxUnoTextRangeBase::GotoStart(sal_Bool Expand) throw()
{
    aSelection.nStartPara = 0;
    aSelection.nStartPos  = 0;

    if (!Expand)
        CollapseToStart();
}

void SvxUnoTextRangeBase::GotoEnd(sal_Bool Expand) throw()
{
    CheckSelection( aSelection, pEditSource->GetTextForwarder() );

    SvxTextForwarder* pForwarder = pEditSource ? pEditSource->GetTextForwarder() : NULL;
    if( pForwarder )
    {

        sal_uInt16 nPar = pForwarder->GetParagraphCount();
        if (nPar)
            --nPar;

        aSelection.nEndPara = nPar;
        aSelection.nEndPos  = pForwarder->GetTextLen( nPar );

        if (!Expand)
            CollapseToEnd();
    }
}

// lang::XServiceInfo
sal_Bool SAL_CALL SvxUnoTextRangeBase::supportsService( const OUString& ServiceName )
    throw(uno::RuntimeException)
{
    return SvxServiceInfoHelper::supportsService( ServiceName, getSupportedServiceNames() );
}

uno::Sequence< OUString > SAL_CALL SvxUnoTextRangeBase::getSupportedServiceNames()
    throw(uno::RuntimeException)
{
    uno::Sequence< OUString >	aSeq;
    SvxServiceInfoHelper::addToSequence( aSeq, 3, "com.sun.star.style.CharacterProperties",
                                                  "com.sun.star.style.CharacterPropertiesComplex",
                                                  "com.sun.star.style.CharacterPropertiesAsian");
    return aSeq;
}


// ====================================================================
// class SvxUnoTextRange
// ====================================================================

uno::Sequence< uno::Type > SvxUnoTextRange::maTypeSequence;

SvxUnoTextRange::SvxUnoTextRange( const SvxUnoTextBase& rParent, sal_Bool bPortion /* = sal_False */ ) throw()
:SvxUnoTextRangeBase( rParent.GetEditSource(), bPortion ? ImplGetSvxTextPortionPropertyMap() : rParent.getPropertyMap() ),
 mbPortion( bPortion )
{
    xParentText =  (text::XText*)&rParent;
}

SvxUnoTextRange::~SvxUnoTextRange() throw()
{
}

uno::Any SAL_CALL SvxUnoTextRange::queryAggregation( const uno::Type & rType )
    throw(uno::RuntimeException)
{
    uno::Any aAny;

    QUERYINT( text::XTextRange );
    else if( rType == ::getCppuType((const uno::Reference< beans::XPropertySet >*)0) )
        aAny <<= uno::Reference< beans::XPropertySet >(this);
    else QUERYINT( beans::XPropertyState );
    else if( rType == ::getCppuType((const uno::Reference< beans::XMultiPropertySet >*)0) )
        aAny <<= uno::Reference< beans::XMultiPropertySet >(this);
    else QUERYINT( lang::XServiceInfo );
    else QUERYINT( lang::XTypeProvider );
    else QUERYINT( lang::XUnoTunnel );
    else
        return OWeakAggObject::queryAggregation( rType );

    return aAny;
}

uno::Any SAL_CALL SvxUnoTextRange::queryInterface( const uno::Type & rType )
    throw(uno::RuntimeException)
{
    return OWeakAggObject::queryInterface(rType);
}

void SAL_CALL SvxUnoTextRange::acquire()
    throw( )
{
    OWeakAggObject::acquire();
}

void SAL_CALL SvxUnoTextRange::release()
    throw( )
{
    OWeakAggObject::release();
}

// XTypeProvider

uno::Sequence< uno::Type > SAL_CALL SvxUnoTextRange::getTypes()
    throw (uno::RuntimeException)
{
    if( maTypeSequence.getLength() == 0 )
    {
        maTypeSequence.realloc( 7 ); // !DANGER! keep this updated
        uno::Type* pTypes = maTypeSequence.getArray();

        *pTypes++ = ::getCppuType(( const uno::Reference< text::XTextRange >*)0);
        *pTypes++ = ::getCppuType(( const uno::Reference< beans::XPropertySet >*)0);
        *pTypes++ = ::getCppuType(( const uno::Reference< beans::XMultiPropertySet >*)0);
        *pTypes++ = ::getCppuType(( const uno::Reference< beans::XPropertyState >*)0);
        *pTypes++ = ::getCppuType(( const uno::Reference< lang::XServiceInfo >*)0);
        *pTypes++ = ::getCppuType(( const uno::Reference< lang::XTypeProvider >*)0);
        *pTypes++ = ::getCppuType(( const uno::Reference< lang::XUnoTunnel >*)0);
    }
    return maTypeSequence;
}

uno::Sequence< sal_Int8 > SAL_CALL SvxUnoTextRange::getImplementationId()
    throw (uno::RuntimeException)
{
    static uno::Sequence< sal_Int8 > aId;
    if( aId.getLength() == 0 )
    {
        aId.realloc( 16 );
        rtl_createUuid( (sal_uInt8 *)aId.getArray(), 0, sal_True );
    }
    return aId;
}

// XTextRange
uno::Reference< text::XText > SAL_CALL SvxUnoTextRange::getText()
    throw(uno::RuntimeException)
{
    return xParentText;
}

// lang::XServiceInfo
OUString SAL_CALL SvxUnoTextRange::getImplementationName()
    throw(uno::RuntimeException)
{
    return OUString(RTL_CONSTASCII_USTRINGPARAM("SvxUnoTextRange"));
}

// ====================================================================
// class SvxUnoText
// ====================================================================

// UNO3_GETIMPLEMENTATION2_IMPL( SvxUnoText, SvxUnoTextRangeBase );

uno::Sequence< uno::Type > SvxUnoTextBase::maTypeSequence;

SvxUnoTextBase::SvxUnoTextBase( const SfxItemPropertyMap* _pMap  ) throw()
: SvxUnoTextRangeBase( _pMap )
{
}

SvxUnoTextBase::SvxUnoTextBase( const SvxEditSource* pSource, const SfxItemPropertyMap* _pMap, uno::Reference < text::XText > xParent ) throw()
: SvxUnoTextRangeBase( pSource, _pMap )
{
    xParentText = xParent;
    ESelection aSelection;
    ::binfilter::GetSelection( aSelection, GetEditSource()->GetTextForwarder() );//STRIP008 	::GetSelection( aSelection, GetEditSource()->GetTextForwarder() );
    SetSelection( aSelection );
}

SvxUnoTextBase::SvxUnoTextBase( const SvxUnoTextBase& rText ) throw()
:SvxUnoTextRangeBase( rText )
{
    xParentText = rText.xParentText;
}

SvxUnoTextBase::~SvxUnoTextBase() throw()
{
}

sal_Bool SvxUnoTextBase::queryAggregation( const uno::Type & rType, uno::Any& aAny )
{
    QUERYINT( text::XText );
    else QUERYINT( text::XSimpleText );
    else if( rType == ::getCppuType((const uno::Reference< text::XTextRange >*)0) )
        aAny <<= uno::Reference< text::XTextRange >((text::XText*)(this));
    else QUERYINT(container::XEnumerationAccess );
    else QUERYINT( container::XElementAccess );
    else QUERYINT( beans::XPropertySet );
    else QUERYINT( beans::XMultiPropertySet );
    else QUERYINT( beans::XPropertyState );
    else QUERYINT( lang::XServiceInfo );
    else QUERYINT( text::XTextRangeMover );
    else QUERYINT( lang::XTypeProvider );
    else QUERYINT( lang::XUnoTunnel );
    else
        return sal_False;

    return sal_True;
}

// XInterface
uno::Any SAL_CALL SvxUnoTextBase::queryAggregation( const uno::Type & rType )
    throw(uno::RuntimeException)
{
    uno::Any aAny;
    queryAggregation( rType, aAny );
    return aAny;
}

// XTypeProvider

uno::Sequence< uno::Type > SAL_CALL SvxUnoTextBase::getStaticTypes() throw()
{
    if( maTypeSequence.getLength() == 0 )
    {
        maTypeSequence.realloc( 9 ); // !DANGER! keep this updated
        uno::Type* pTypes = maTypeSequence.getArray();

        *pTypes++ = ::getCppuType(( const uno::Reference< text::XText >*)0);
        *pTypes++ = ::getCppuType(( const uno::Reference< container::XEnumerationAccess >*)0);
        *pTypes++ = ::getCppuType(( const uno::Reference< beans::XPropertySet >*)0);
        *pTypes++ = ::getCppuType(( const uno::Reference< beans::XMultiPropertySet >*)0);
        *pTypes++ = ::getCppuType(( const uno::Reference< beans::XPropertyState >*)0);
        *pTypes++ = ::getCppuType(( const uno::Reference< text::XTextRangeMover >*)0);
        *pTypes++ = ::getCppuType(( const uno::Reference< lang::XServiceInfo >*)0);
        *pTypes++ = ::getCppuType(( const uno::Reference< lang::XTypeProvider >*)0);
        *pTypes++ = ::getCppuType(( const uno::Reference< lang::XUnoTunnel >*)0);
    }
    return maTypeSequence;
}

uno::Sequence< uno::Type > SAL_CALL SvxUnoTextBase::getTypes()
    throw (uno::RuntimeException)
{
    return getStaticTypes();
}

uno::Sequence< sal_Int8 > SAL_CALL SvxUnoTextBase::getImplementationId()
    throw (uno::RuntimeException)
{
    static uno::Sequence< sal_Int8 > aId;
    if( aId.getLength() == 0 )
    {
        aId.realloc( 16 );
        rtl_createUuid( (sal_uInt8 *)aId.getArray(), 0, sal_True );
    }
    return aId;
}

// XSimpleText

uno::Reference< text::XTextCursor > SAL_CALL SvxUnoTextBase::createTextCursor()
    throw(uno::RuntimeException)
{
    OGuard aGuard( Application::GetSolarMutex() );
    return new SvxUnoTextCursor( *this );
}

uno::Reference< text::XTextCursor > SAL_CALL SvxUnoTextBase::createTextCursorByRange( const uno::Reference< text::XTextRange >& aTextPosition )
    throw(uno::RuntimeException)
{
    OGuard aGuard( Application::GetSolarMutex() );

    SvxUnoTextCursor* pCursor = new SvxUnoTextCursor( *this );
    uno::Reference< text::XTextCursor >  xCursor( pCursor );

    if( aTextPosition.is() )
    {
        SvxUnoTextRangeBase* pRange = SvxUnoTextRangeBase::getImplementation( aTextPosition );
        if(pRange)
            pCursor->SetSelection( pRange->GetSelection() );
    }

    return xCursor;
}

void SAL_CALL SvxUnoTextBase::insertString( const uno::Reference< text::XTextRange >& xRange, const OUString& aString, sal_Bool bAbsorb )
    throw(uno::RuntimeException)
{
    OGuard aGuard( Application::GetSolarMutex() );

    if( !xRange.is() )
        return;

    ESelection aSelection;
    ::binfilter::GetSelection( aSelection, GetEditSource()->GetTextForwarder() );//STRIP008 	::GetSelection( aSelection, GetEditSource()->GetTextForwarder() );
    SetSelection( aSelection );

    SvxUnoTextRangeBase* pRange = SvxUnoTextRange::getImplementation( xRange );
    if(pRange)
    {
        //	setString am SvxUnoTextRangeBase statt selber QuickInsertText und UpdateData,
        //	damit die Selektion am SvxUnoTextRangeBase angepasst wird.
        //!	Eigentlich muessten alle Cursor-Objekte dieses Textes angepasst werden!

        if (!bAbsorb)					// nicht ersetzen -> hinten anhaengen
            pRange->CollapseToEnd();

        pRange->setString( aString );

        pRange->CollapseToEnd();
    }
}

void SAL_CALL SvxUnoTextBase::insertControlCharacter( const uno::Reference< text::XTextRange >& xRange, sal_Int16 nControlCharacter, sal_Bool bAbsorb )
    throw(lang::IllegalArgumentException, uno::RuntimeException)
{
    OGuard aGuard( Application::GetSolarMutex() );

    SvxTextForwarder* pForwarder = GetEditSource() ? GetEditSource()->GetTextForwarder() : NULL;

    if( pForwarder )
    {
        ESelection aSelection;
        ::binfilter::GetSelection( aSelection, pForwarder );//STRIP008 		::GetSelection( aSelection, pForwarder );
        SetSelection( aSelection );

        switch( nControlCharacter )
        {
        case text::ControlCharacter::PARAGRAPH_BREAK:
        {
            const String aText( (char)13, 1 );	// '\r' geht auf'm Mac nicht
            insertString( xRange, aText, bAbsorb );

            return;
        }
        case text::ControlCharacter::LINE_BREAK:
        {
            SvxUnoTextRangeBase* pRange = SvxUnoTextRange::getImplementation( xRange );
            if(pRange)
            {
                ESelection aRange = pRange->GetSelection();

                if( bAbsorb )
                {
                    const String aEmpty;
                    pForwarder->QuickInsertText( aEmpty, aRange );

                    aRange.nEndPos = aRange.nStartPos;
                    aRange.nEndPara = aRange.nStartPara;
                }
                else
                {
                    aRange.nStartPos = aRange.nEndPos;
                    aRange.nStartPara = aRange.nStartPara;
                }

                pForwarder->QuickInsertLineBreak( aRange );
                GetEditSource()->UpdateData();

                aRange.nEndPos += 1;
                if( !bAbsorb )
                    aRange.nStartPos += 1;

                pRange->SetSelection( aRange );
            }
            return;
        }
        case text::ControlCharacter::APPEND_PARAGRAPH:
        {
            SvxUnoTextRangeBase* pRange = SvxUnoTextRange::getImplementation( xRange );
            if(pRange)
            {
                ESelection aRange = pRange->GetSelection();
//				ESelection aOldSelection = aRange;

                aRange.nStartPos  = pForwarder->GetTextLen( aRange.nStartPara );

                aRange.nEndPara = aRange.nStartPara;
                aRange.nEndPos  = aRange.nStartPos;

                pRange->SetSelection( aRange );
                const String aText( (char)13, 1 );	// '\r' geht auf'm Mac nicht
                pRange->setString( aText );

                aRange.nStartPos = 0;
                aRange.nStartPara += 1;
                aRange.nEndPos = 0;
                aRange.nEndPara += 1;

                pRange->SetSelection( aRange );

                return;
            }
        }
        }
    }

    throw lang::IllegalArgumentException();
}

// XText
void SAL_CALL SvxUnoTextBase::insertTextContent( const uno::Reference< text::XTextRange >& xRange, const uno::Reference< text::XTextContent >& xContent, sal_Bool bAbsorb )
    throw(lang::IllegalArgumentException, uno::RuntimeException)
{
    OGuard aGuard( Application::GetSolarMutex() );

    SvxTextForwarder* pForwarder = GetEditSource() ? GetEditSource()->GetTextForwarder() : NULL;
    if( pForwarder )
    {

        SvxUnoTextRangeBase* pRange = SvxUnoTextRange::getImplementation( xRange );
        SvxUnoTextField* pField = SvxUnoTextField::getImplementation( xContent );

        if( pRange == NULL || pField == NULL )
            throw lang::IllegalArgumentException();

        ESelection aSelection = pRange->GetSelection();
        if( !bAbsorb )
        {
            aSelection.nStartPara = aSelection.nEndPara;
            aSelection.nStartPos  = aSelection.nEndPos;
        }

        SvxFieldData* pFieldData = pField->CreateFieldData();
        if( pField == NULL )
            throw lang::IllegalArgumentException();

        SvxFieldItem aField( *pFieldData );
        pForwarder->QuickInsertField( aField, aSelection );
        GetEditSource()->UpdateData();

        pField->SetAnchor( uno::Reference< text::XTextRange >::query( (cppu::OWeakObject*)this ) );

        aSelection.nEndPos += 1;
        aSelection.nStartPos = aSelection.nEndPos;
        pRange->SetSelection( aSelection );

        delete pFieldData;
    }
}

void SAL_CALL SvxUnoTextBase::removeTextContent( const uno::Reference< text::XTextContent >& xContent ) throw(container::NoSuchElementException, uno::RuntimeException)
{
}

// XTextRange

uno::Reference< text::XText > SAL_CALL SvxUnoTextBase::getText()
    throw(uno::RuntimeException)
{
    OGuard aGuard( Application::GetSolarMutex() );

    ESelection aSelection;
    ::binfilter::GetSelection( aSelection, GetEditSource()->GetTextForwarder() );//STRIP008 	::GetSelection( aSelection, GetEditSource()->GetTextForwarder() );
    ((SvxUnoTextBase*)this)->SetSelection( aSelection );

    return (text::XText*)this;
}

uno::Reference< text::XTextRange > SAL_CALL SvxUnoTextBase::getStart()
    throw(uno::RuntimeException)
{
    return SvxUnoTextRangeBase::getStart();
}

uno::Reference< text::XTextRange > SAL_CALL SvxUnoTextBase::getEnd()
    throw(uno::RuntimeException)
{
    return SvxUnoTextRangeBase::getEnd();
}

OUString SAL_CALL SvxUnoTextBase::getString() throw( uno::RuntimeException )
{
    return SvxUnoTextRangeBase::getString();
}

void SAL_CALL SvxUnoTextBase::setString( const OUString& aString ) throw(uno::RuntimeException)
{
    SvxUnoTextRangeBase::setString(aString);
}


// XEnumerationAccess
uno::Reference< container::XEnumeration > SAL_CALL SvxUnoTextBase::createEnumeration()
    throw(uno::RuntimeException)
{
    OGuard aGuard( Application::GetSolarMutex() );

    ESelection aSelection;
    ::binfilter::GetSelection( aSelection, GetEditSource()->GetTextForwarder() );//STRIP008 	::GetSelection( aSelection, GetEditSource()->GetTextForwarder() );
    SetSelection( aSelection );

    uno::Reference< container::XEnumeration > xEnum( (container::XEnumeration*) new SvxUnoTextContentEnumeration( *this ) );
    return xEnum;
}

// XElementAccess ( container::XEnumerationAccess )
uno::Type SAL_CALL SvxUnoTextBase::getElementType(  ) throw(uno::RuntimeException)
{
    return ::getCppuType((const uno::Reference< text::XTextRange >*)0 );
}

sal_Bool SAL_CALL SvxUnoTextBase::hasElements(  ) throw(uno::RuntimeException)
{
    OGuard aGuard( Application::GetSolarMutex() );

    if(GetEditSource())
    {
        SvxTextForwarder* pForwarder = GetEditSource()->GetTextForwarder();
        if(pForwarder)
            return pForwarder->GetParagraphCount() != 0;
    }

    return sal_False;
}

// text::XTextRangeMover
void SAL_CALL SvxUnoTextBase::moveTextRange( const uno::Reference< text::XTextRange >& xRange, sal_Int16 nParagraphs )
    throw(uno::RuntimeException)
{
}

// lang::XServiceInfo
OUString SAL_CALL SvxUnoTextBase::getImplementationName()
    throw(uno::RuntimeException)
{
    return OUString(RTL_CONSTASCII_USTRINGPARAM("SvxUnoTextBase"));
}

uno::Sequence< OUString > SAL_CALL SvxUnoTextBase::getSupportedServiceNames(  )
    throw(uno::RuntimeException)
{
    uno::Sequence< OUString > aSeq( SvxUnoTextRangeBase::getSupportedServiceNames() );
    SvxServiceInfoHelper::addToSequence( aSeq, 1, "com.sun.star.text.Text" );
    return aSeq;
}

const uno::Sequence< sal_Int8 > & SvxUnoTextBase::getUnoTunnelId() throw()
{
    static uno::Sequence< sal_Int8 > * pSeq = 0;
    if( !pSeq )
    {
        ::osl::Guard< ::osl::Mutex > aGuard( ::osl::Mutex::getGlobalMutex() );
        if( !pSeq )
        {
            static uno::Sequence< sal_Int8 > aSeq( 16 );
            rtl_createUuid( (sal_uInt8*)aSeq.getArray(), 0, sal_True );
            pSeq = &aSeq;
        }
    }
    return *pSeq;
}

SvxUnoTextBase* SvxUnoTextBase::getImplementation( const uno::Reference< uno::XInterface >& xInt )
{
    uno::Reference< lang::XUnoTunnel > xUT( xInt, uno::UNO_QUERY );
    if( xUT.is() )
        return (SvxUnoTextBase*)xUT->getSomething( SvxUnoTextBase::getUnoTunnelId() );
    else
        return NULL;
}

sal_Int64 SAL_CALL SvxUnoTextBase::getSomething( const uno::Sequence< sal_Int8 >& rId ) throw(uno::RuntimeException) \
{
    if( rId.getLength() == 16 && 0 == rtl_compareMemory( getUnoTunnelId().getConstArray(),
                                                         rId.getConstArray(), 16 ) )
    {
        return (sal_Int64)this;
    }
    else
    {
        return SvxUnoTextRangeBase::getSomething( rId );
    }
}

// --------------------------------------------------------------------

SvxUnoText::SvxUnoText( const SvxEditSource* pSource, const SfxItemPropertyMap* _pMap, uno::Reference < text::XText > xParent ) throw()
: SvxUnoTextBase( pSource, _pMap, xParent )
{
}

SvxUnoText::SvxUnoText( const SvxUnoText& rText ) throw()
: SvxUnoTextBase( rText )
{
}

SvxUnoText::~SvxUnoText() throw()
{
}

// uno::XInterface
uno::Any SAL_CALL SvxUnoText::queryAggregation( const uno::Type & rType ) throw( uno::RuntimeException )
{
    uno::Any aAny( SvxUnoTextBase::queryAggregation( rType ) );
    if( !aAny.hasValue() )
        aAny = OWeakAggObject::queryAggregation( rType );

    return aAny;
}

uno::Any SAL_CALL SvxUnoText::queryInterface( const uno::Type & rType ) throw( uno::RuntimeException )
{
    return OWeakAggObject::queryInterface( rType );
}

void SAL_CALL SvxUnoText::acquire() throw( )
{
    OWeakAggObject::acquire();
}

void SAL_CALL SvxUnoText::release() throw( )
{
    OWeakAggObject::release();
}

// lang::XTypeProvider
uno::Sequence< uno::Type > SAL_CALL SvxUnoText::getTypes(  ) throw( uno::RuntimeException )
{
    return SvxUnoTextBase::getTypes();
}

uno::Sequence< sal_Int8 > SAL_CALL SvxUnoText::getImplementationId(  ) throw( uno::RuntimeException )
{
    static uno::Sequence< sal_Int8 > aId;
    if( aId.getLength() == 0 )
    {
        aId.realloc( 16 );
        rtl_createUuid( (sal_uInt8 *)aId.getArray(), 0, sal_True );
    }
    return aId;
}

const uno::Sequence< sal_Int8 > & SvxUnoText::getUnoTunnelId() throw()
{
    static uno::Sequence< sal_Int8 > * pSeq = 0;
    if( !pSeq )
    {
        ::osl::Guard< ::osl::Mutex > aGuard( ::osl::Mutex::getGlobalMutex() );
        if( !pSeq )
        {
            static uno::Sequence< sal_Int8 > aSeq( 16 );
            rtl_createUuid( (sal_uInt8*)aSeq.getArray(), 0, sal_True );
            pSeq = &aSeq;
        }
    }
    return *pSeq;
}

sal_Int64 SAL_CALL SvxUnoText::getSomething( const uno::Sequence< sal_Int8 >& rId ) throw(uno::RuntimeException) \
{
    if( rId.getLength() == 16 && 0 == rtl_compareMemory( getUnoTunnelId().getConstArray(),
                                                         rId.getConstArray(), 16 ) )
    {
        return (sal_Int64)this;
    }
    else
    {
        return SvxUnoTextBase::getSomething( rId );
    }
}


// --------------------------------------------------------------------

SvxDummyTextSource::~SvxDummyTextSource()
{
};

SvxEditSource* SvxDummyTextSource::Clone() const
{
    return new SvxDummyTextSource();
}

SvxTextForwarder* SvxDummyTextSource::GetTextForwarder()
{
    return this;
}

void SvxDummyTextSource::UpdateData()
{
}

sal_uInt16 SvxDummyTextSource::GetParagraphCount() const
{
    return 0;
}

sal_uInt16 SvxDummyTextSource::GetTextLen( sal_uInt16 nParagraph ) const
{
    return 0;
}

String SvxDummyTextSource::GetText( const ESelection& rSel ) const
{
    return String();
}

SfxItemSet SvxDummyTextSource::GetAttribs( const ESelection& rSel, BOOL bOnlyHardAttrib ) const
{
    String aDummyStr(RTL_CONSTASCII_USTRINGPARAM("Dummy"));
    SfxItemPool aPool(aDummyStr,0,0,NULL);
    return SfxItemSet(aPool);
}

SfxItemSet SvxDummyTextSource::GetParaAttribs( sal_uInt16 nPara ) const
{
    return GetAttribs(ESelection());
}

void SvxDummyTextSource::SetParaAttribs( sal_uInt16 nPara, const SfxItemSet& rSet )
{
}

void SvxDummyTextSource::GetPortions( sal_uInt16 nPara, SvUShorts& rList ) const
{
}

sal_uInt16 SvxDummyTextSource::GetItemState( const ESelection& rSel, sal_uInt16 nWhich ) const
{
    return 0;
}

sal_uInt16 SvxDummyTextSource::GetItemState( sal_uInt16 nPara, sal_uInt16 nWhich ) const
{
    return 0;
}

SfxItemPool* SvxDummyTextSource::GetPool() const
{
    return NULL;
}

void SvxDummyTextSource::QuickInsertText( const String& rText, const ESelection& rSel )
{
}

void SvxDummyTextSource::QuickInsertField( const SvxFieldItem& rFld, const ESelection& rSel )
{
}

void SvxDummyTextSource::QuickSetAttribs( const SfxItemSet& rSet, const ESelection& rSel )
{
}

void SvxDummyTextSource::QuickInsertLineBreak( const ESelection& rSel )
{
};

XubString SvxDummyTextSource::CalcFieldValue( const SvxFieldItem& rField, sal_uInt16 nPara, sal_uInt16 nPos, Color*& rpTxtColor, Color*& rpFldColor )
{
    return XubString();
}

sal_Bool SvxDummyTextSource::IsValid() const
{
    return sal_False;
}

void SvxDummyTextSource::SetNotifyHdl( const Link& )
{
}

LanguageType SvxDummyTextSource::GetLanguage( USHORT nPara, USHORT nIndex ) const
{
    return LANGUAGE_DONTKNOW;
}

USHORT SvxDummyTextSource::GetFieldCount( USHORT nPara ) const
{
    return 0; 
}

EFieldInfo SvxDummyTextSource::GetFieldInfo( USHORT nPara, USHORT nField ) const
{
    return EFieldInfo();
}

EBulletInfo SvxDummyTextSource::GetBulletInfo( USHORT nPara ) const
{
    return EBulletInfo();
}

Rectangle SvxDummyTextSource::GetCharBounds( USHORT nPara, USHORT nIndex ) const
{
    return Rectangle();
}

Rectangle SvxDummyTextSource::GetParaBounds( USHORT nPara ) const
{
    return Rectangle();
}

MapMode SvxDummyTextSource::GetMapMode() const
{
    return MapMode();
}

OutputDevice* SvxDummyTextSource::GetRefDevice() const
{
    return NULL;
}

sal_Bool SvxDummyTextSource::GetIndexAtPoint( const Point&, USHORT& nPara, USHORT& nIndex ) const
{
    return sal_False;
}

sal_Bool SvxDummyTextSource::GetWordIndices( USHORT nPara, USHORT nIndex, USHORT& nStart, USHORT& nEnd ) const
{
    return sal_False;
}

sal_Bool SvxDummyTextSource::GetAttributeRun( USHORT& nStartIndex, USHORT& nEndIndex, USHORT nPara, USHORT nIndex ) const
{
    return sal_False;
}

USHORT SvxDummyTextSource::GetLineCount( USHORT nPara ) const
{
    return 0;
}

USHORT SvxDummyTextSource::GetLineLen( USHORT nPara, USHORT nLine ) const
{
    return 0;
}

sal_Bool SvxDummyTextSource::QuickFormatDoc( BOOL bFull )
{
    return sal_False;
}

USHORT SvxDummyTextSource::GetDepth( USHORT nPara ) const
{
    return 0;
}

sal_Bool SvxDummyTextSource::SetDepth( USHORT nPara, USHORT nNewDepth )
{
    return nNewDepth == 0 ? sal_True : sal_False;
}

sal_Bool SvxDummyTextSource::Delete( const ESelection& )
{
    return sal_False;
}

sal_Bool SvxDummyTextSource::InsertText( const String&, const ESelection& )
{
    return sal_False;
}
}
