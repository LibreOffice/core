/*************************************************************************
 *
 *  $RCSfile: unotext.cxx,v $
 *
 *  $Revision: 1.19 $
 *
 *  last change: $Author: cl $ $Date: 2001-03-01 09:08:33 $
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

#ifndef _COM_SUN_STAR_STYLE_LINESPACING_HPP_
#include <com/sun/star/style/LineSpacing.hpp>
#endif
#ifndef _COM_SUN_STAR_TEXT_HORIORIENTATION_HPP_
#include <com/sun/star/text/HoriOrientation.hpp>
#endif
#ifndef _COM_SUN_STAR_TEXT_CONTROLCHARACTER_HPP_
#include <com/sun/star/text/ControlCharacter.hpp>
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

#ifndef _SV_SVAPP_HXX //autogen
#include <vcl/svapp.hxx>
#endif
#ifndef _VOS_MUTEX_HXX_ //autogen
#include <vos/mutex.hxx>
#endif

#ifndef _SFXITEMSET_HXX //autogen
#include <svtools/itemset.hxx>
#endif

#include <eeitem.hxx>

#ifndef _SFXITEMPOOL_HXX
#include <svtools/itempool.hxx>
#endif

#define ITEMID_FONT EE_CHAR_FONTINFO
#ifndef _SVX_FONTITEM_HXX //autogen
#include <fontitem.hxx>
#endif

#define ITEMID_TABSTOP EE_PARA_TABS
#ifndef _SVX_TSPTITEM_HXX //autogen
#include <tstpitem.hxx>
#endif

#ifndef _SVDOBJ_HXX
#include "svdobj.hxx"
#endif

#ifndef _SFXINTITEM_HXX //autogen
#include <svtools/intitem.hxx>
#endif

#define ITEMID_FIELD EE_FEATURE_FIELD
#include <rtl/uuid.h>
#include <rtl/memory.h>

#include "unoshtxt.hxx"
#include "unoprnms.hxx"
#include "unotext.hxx"
#include "unoedsrc.hxx"
#include "unomid.hxx"
#include "unonrule.hxx"
#include "unofdesc.hxx"
#include "unoapi.hxx"
#include "unofield.hxx"
#include "flditem.hxx"
#include "unoshprp.hxx"
#include "numitem.hxx"

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
        else if( rSel.nStartPos > pForwarder->GetTextLen( rSel.nStartPara ) )
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

// ====================================================================
// class SvxUnoTextRangeBase
// ====================================================================

UNO3_GETIMPLEMENTATION_IMPL( SvxUnoTextRangeBase );

SvxUnoTextRangeBase::SvxUnoTextRangeBase( const SvxEditSource* pSource, const SfxItemPropertyMap* _pMap ) throw()
: aPropSet(_pMap)
{
    OGuard aGuard( Application::GetSolarMutex() );

    DBG_ASSERT(pSource,"SvxUnoTextRangeBase: Source darf nicht 0 sein");

    pEditSource = pSource->Clone();
    ESelection aSelection;
    ::GetSelection( aSelection, pEditSource->GetTextForwarder() );
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

        SvxUnoText* pText = SvxUnoText::getImplementation( getText() );

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

        SvxUnoText* pText = SvxUnoText::getImplementation( getText() );

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
        aConverted.ConvertLineEnd( LINEEND_LF );        // Zeilenenden nur einfach zaehlen

        pForwarder->QuickInsertText( aConverted, aSelection );
        pEditSource->UpdateData();

        //  Selektion anpassen
        //! Wenn die EditEngine bei QuickInsertText die Selektion zurueckgeben wuerde,
        //! waer's einfacher...
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
    return new SfxItemPropertySetInfo( aPropSet.getPropertyMap() );
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

                if(!SetPropertyValueHelper( aOldSet, pMap, aValue, aNewSet, &aSelection, (SvxTextEditSource*)GetEditSource() ))
                {
                    //  Fuer Teile von zusammengesetzten Items mit mehreren Properties (z.B. Hintergrund)
                    //  muss vorher das alte Item aus dem Dokument geholt werden
                    aNewSet.Put(aOldSet.Get(pMap->nWID));           // altes Item in neuen Set
                    aPropSet.setPropertyValue(pMap, aValue, aNewSet);
                }

                pForwarder->QuickSetAttribs( aNewSet, GetSelection() );
            }
            else
            {
                sal_Int32 nEndPara = nPara;

                if( nPara == -1 )
                {
                    nPara = aSel.nStartPara;
                    nEndPara = aSel.nEndPara;
                }

                do
                {
                    // we have a paragraph
                    SfxItemSet aNewSet( pForwarder->GetParaAttribs( (USHORT)nPara ) );

                    if(!SetPropertyValueHelper( aNewSet, pMap, aValue, aNewSet, &aSelection, (SvxTextEditSource*)GetEditSource() ))
                    {
                        aPropSet.setPropertyValue(pMap, aValue, aNewSet);
                    }

                    pForwarder->SetParaAttribs( (USHORT)nPara, aNewSet );
                    nPara++;
                }
                while( nPara < nEndPara );
            }

            GetEditSource()->UpdateData();
            return;
        }
    }

    throw beans::UnknownPropertyException();
}

sal_Bool SvxUnoTextRangeBase::SetPropertyValueHelper( const SfxItemSet& rOldSet, const SfxItemPropertyMap* pMap, const uno::Any& aValue, SfxItemSet& rNewSet, const ESelection* pSelection /* = NULL */, SvxTextEditSource* pEditSource /* = NULL*/ ) throw( ::com::sun::star::uno::RuntimeException )
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
                    SdrObject* pObj = pEditSource->GetSdrObject();

                    if(pObj == NULL)
                        return sal_False;

                    const sal_Bool bOutlinerText = (pObj->GetObjInventor() == SdrInventor) && (pObj->GetObjIdentifier() == OBJ_OUTLINETEXT);
                    if(bOutlinerText)
                        nLevel++;

                    if(nLevel >= 0 && nLevel <= 9)
                    {
                        Outliner& rOutliner = ((SvxOutlinerForwarder*)pForwarder)->GetOutliner();

                        Paragraph* pPara = rOutliner.GetParagraph( pSelection->nStartPara );
                        if( pPara )
                        {
                            rOutliner.SetDepth( pPara, nLevel );
                            if( bOutlinerText )
                                rOutliner.SetLevelDependendStyleSheet( pSelection->nStartPara );
                            return sal_True;
                        }
                    }
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

            //  Dontcare durch Default ersetzen, damit man immer eine Reflection hat
            pAttribs->ClearInvalidItems();

            if( pMap->nWID == EE_FEATURE_FIELD )
            {
                if ( pAttribs->GetItemState( EE_FEATURE_FIELD, sal_False ) == SFX_ITEM_SET )
                {
                    SvxFieldItem* pItem = (SvxFieldItem*)pAttribs->GetItem( EE_FEATURE_FIELD );
                    const SvxFieldData* pData = pItem->GetField();
                    uno::Reference< text::XTextRange > xAnchor( this );

                    // get presentation string for field
                    Color* pTColor = NULL;
                    Color* pFColor = NULL;

                    OUString aPresentation( pForwarder->CalcFieldValue( *pData, aSelection.nStartPara, aSelection.nStartPos, pTColor, pFColor ) );

                    delete pTColor;
                    delete pFColor;

                    uno::Reference< text::XTextField > xField( new SvxUnoTextField( xAnchor, aPresentation, pData ) );
                    aAny <<= xField;
                }
            }
            else if( pMap->nWID == WID_PORTIONTYPE )
            {
                if ( pAttribs->GetItemState( EE_FEATURE_FIELD, sal_False ) == SFX_ITEM_SET )
                {
                    OUString aType( RTL_CONSTASCII_USTRINGPARAM("TextField") );
                    aAny <<= aType;
                }
                else
                {
                    OUString aType( RTL_CONSTASCII_USTRINGPARAM("Text") );
                    aAny <<= aType;
                }
            }
            else
            {
                if(!GetPropertyValueHelper( *pAttribs, pMap, aAny, &aSelection, (SvxTextEditSource*)GetEditSource() ))
                    aAny = aPropSet.getPropertyValue(pMap, *pAttribs);
            }

            delete pAttribs;
            return aAny;
        }
    }

    throw beans::UnknownPropertyException();
    return aAny;
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

                Outliner& rOutliner = ((SvxOutlinerForwarder*)pForwarder)->GetOutliner();
                Paragraph* pPara = rOutliner.GetParagraph( pSelection->nStartPara );

                sal_Int16 nLevel = 0;
                if( pPara )
                    nLevel = rOutliner.GetDepth( pSelection->nStartPara );

                SdrObject* pObj = pEditSource->GetSdrObject();

                if(pObj != NULL)
                {
                    if((pObj->GetObjInventor() == SdrInventor) &&
                       (pObj->GetObjIdentifier() == OBJ_OUTLINETEXT))
                            nLevel--;
                }
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
//          case SFX_ITEM_UNKNOWN:
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

    for( sal_Int32 nIdx = 0; nIdx < nCount; nIdx++ )
        *pState++ = getPropertyState( *pNames++ );

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
            sal_Int16 nLevel = 0;

            SvxTextEditSource* pEditSource = (SvxTextEditSource*)GetEditSource();
            SdrObject* pObj = pEditSource->GetSdrObject();

            if(pObj)
            {
                if((pObj->GetObjInventor() == SdrInventor) &&
                   (pObj->GetObjIdentifier() == OBJ_OUTLINETEXT))
                    nLevel++;

                Outliner& rOutliner = ((SvxOutlinerForwarder*)pForwarder)->GetOutliner();
                Paragraph* pPara =  rOutliner.GetParagraph( aSelection.nStartPara );
                if( pPara )
                    rOutliner.SetDepth( pPara, nLevel );
                return;
            }
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
                        SfxItemSet aSet( *pPool,    pMap->nWID, pMap->nWID);
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

    //  #75098# use end position, as in Writer (start is anchor, end is cursor)
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
                pForwarder = pEditSource->GetTextForwarder();   // erst hier, wenn's noetig ist...

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


        sal_uInt16 nNewPos = aSelection.nEndPos + nCount;           //! Ueberlauf ???
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
    uno::Sequence< OUString >   aSeq;
    SvxServiceInfoHelper::addToSequence( aSeq, 1, "com.sun.star.style.CharacterProperties" );
    return aSeq;
}


// ====================================================================
// class SvxUnoTextRange
// ====================================================================

uno::Sequence< uno::Type > SvxUnoTextRange::maTypeSequence;

uno::Reference< uno::XInterface > SvxUnoTextRange_NewInstance()
{
    SvxUnoText aText;
    uno::Reference< text::XTextRange > xRange( new SvxUnoTextRange( aText ) );
    return xRange;
}

SvxUnoTextRange::SvxUnoTextRange( const SvxUnoText& rParent, sal_Bool bPortion /* = sal_False */ ) throw()
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
    else QUERYINT( beans::XPropertySet );
    else QUERYINT( beans::XPropertyState );
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
    throw(uno::RuntimeException)
{
    OWeakAggObject::acquire();
}

void SAL_CALL SvxUnoTextRange::release()
    throw(uno::RuntimeException)
{
    OWeakAggObject::release();
}

// XTypeProvider

uno::Sequence< uno::Type > SAL_CALL SvxUnoTextRange::getTypes()
    throw (uno::RuntimeException)
{
    if( maTypeSequence.getLength() == 0 )
    {
        maTypeSequence.realloc( 6 ); // !DANGER! keep this updated
        uno::Type* pTypes = maTypeSequence.getArray();

        *pTypes++ = ::getCppuType(( const uno::Reference< text::XTextRange >*)0);
        *pTypes++ = ::getCppuType(( const uno::Reference< beans::XPropertySet >*)0);
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

uno::Sequence< uno::Type > SvxUnoText::maTypeSequence;

SvxUnoText::SvxUnoText() throw()
: SvxUnoTextRangeBase(NULL, aEmptyPropMap )
{

}

SvxUnoText::SvxUnoText( const SvxEditSource* pSource, const SfxItemPropertyMap* _pMap, uno::Reference < text::XText > xParent ) throw()
: SvxUnoTextRangeBase( pSource, _pMap )
{
    xParentText = xParent;
    ESelection aSelection;
    ::GetSelection( aSelection, GetEditSource()->GetTextForwarder() );
    SetSelection( aSelection );
}

SvxUnoText::SvxUnoText( const SvxUnoText& rText ) throw()
:SvxUnoTextRangeBase( rText )
{
    xParentText = rText.xParentText;
}

SvxUnoText::~SvxUnoText() throw()
{
}

// Internal
ESelection SvxUnoText::InsertField( const SvxFieldItem& rField ) throw()
{
    SvxTextForwarder* pForwarder = GetEditSource() ? GetEditSource()->GetTextForwarder() : NULL;
    if( pForwarder )
    {
        pForwarder->QuickInsertField( rField, GetSelection() );
        GetEditSource()->UpdateData();

        //  Selektion anpassen
        //! Wenn die EditEngine bei QuickInsertText die Selektion zurueckgeben wuerde,
        //! waer's einfacher...

        CollapseToStart();
        GoRight( 1, sal_True );     // Feld ist immer 1 Zeichen
    }

    return GetSelection();  // Selektion mit dem Feld
}

// XInterface
uno::Any SAL_CALL SvxUnoText::queryAggregation( const uno::Type & rType )
    throw(uno::RuntimeException)
{
    uno::Any aAny;

    QUERYINT( text::XText );
    else QUERYINT( text::XSimpleText );
    else if( rType == ::getCppuType((const uno::Reference< text::XTextRange >*)0) )
        aAny <<= uno::Reference< text::XTextRange >((text::XText*)(this));
    else QUERYINT(container::XEnumerationAccess );
    else QUERYINT( container::XElementAccess );
    else QUERYINT( beans::XPropertySet );
    else QUERYINT( beans::XPropertyState );
    else QUERYINT( lang::XServiceInfo );
    else QUERYINT( text::XTextRangeMover );
    else QUERYINT( lang::XTypeProvider );
    else QUERYINT( lang::XUnoTunnel );
    else
        return OWeakAggObject::queryAggregation( rType );

    return aAny;
}

uno::Any SAL_CALL SvxUnoText::queryInterface( const uno::Type & rType )
    throw(uno::RuntimeException)
{
    return OWeakAggObject::queryInterface(rType);
}

void SAL_CALL SvxUnoText::acquire()
    throw(uno::RuntimeException)
{
    OWeakAggObject::acquire();
}

void SAL_CALL SvxUnoText::release()
    throw(uno::RuntimeException)
{
    OWeakAggObject::release();
}

// XTypeProvider

uno::Sequence< uno::Type > SAL_CALL SvxUnoText::getStaticTypes() throw()
{
    if( maTypeSequence.getLength() == 0 )
    {
        maTypeSequence.realloc( 8 ); // !DANGER! keep this updated
        uno::Type* pTypes = maTypeSequence.getArray();

        *pTypes++ = ::getCppuType(( const uno::Reference< text::XText >*)0);
        *pTypes++ = ::getCppuType(( const uno::Reference< container::XEnumerationAccess >*)0);
        *pTypes++ = ::getCppuType(( const uno::Reference< beans::XPropertySet >*)0);
        *pTypes++ = ::getCppuType(( const uno::Reference< beans::XPropertyState >*)0);
        *pTypes++ = ::getCppuType(( const uno::Reference< text::XTextRangeMover >*)0);
        *pTypes++ = ::getCppuType(( const uno::Reference< lang::XServiceInfo >*)0);
        *pTypes++ = ::getCppuType(( const uno::Reference< lang::XTypeProvider >*)0);
        *pTypes++ = ::getCppuType(( const uno::Reference< lang::XUnoTunnel >*)0);
    }
    return maTypeSequence;
}

uno::Sequence< uno::Type > SAL_CALL SvxUnoText::getTypes()
    throw (uno::RuntimeException)
{
    return getStaticTypes();
}

uno::Sequence< sal_Int8 > SAL_CALL SvxUnoText::getImplementationId()
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

uno::Reference< text::XTextCursor > SAL_CALL SvxUnoText::createTextCursor()
    throw(uno::RuntimeException)
{
    OGuard aGuard( Application::GetSolarMutex() );
    return new SvxUnoTextCursor( *this );
}

uno::Reference< text::XTextCursor > SAL_CALL SvxUnoText::createTextCursorByRange( const uno::Reference< text::XTextRange >& aTextPosition )
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

void SAL_CALL SvxUnoText::insertString( const uno::Reference< text::XTextRange >& xRange, const OUString& aString, sal_Bool bAbsorb )
    throw(uno::RuntimeException)
{
    OGuard aGuard( Application::GetSolarMutex() );

    if( !xRange.is() )
        return;

    ESelection aSelection;
    ::GetSelection( aSelection, GetEditSource()->GetTextForwarder() );
    SetSelection( aSelection );

    SvxUnoTextRangeBase* pRange = SvxUnoTextRange::getImplementation( xRange );
    if(pRange)
    {
        //  setString am SvxUnoTextRangeBase statt selber QuickInsertText und UpdateData,
        //  damit die Selektion am SvxUnoTextRangeBase angepasst wird.
        //! Eigentlich muessten alle Cursor-Objekte dieses Textes angepasst werden!

        if (!bAbsorb)                   // nicht ersetzen -> hinten anhaengen
            pRange->CollapseToEnd();

        pRange->setString( aString );

        pRange->CollapseToEnd();
    }
}

void SAL_CALL SvxUnoText::insertControlCharacter( const uno::Reference< text::XTextRange >& xRange, sal_Int16 nControlCharacter, sal_Bool bAbsorb )
    throw(lang::IllegalArgumentException, uno::RuntimeException)
{
    OGuard aGuard( Application::GetSolarMutex() );

    SvxTextForwarder* pForwarder = GetEditSource() ? GetEditSource()->GetTextForwarder() : NULL;

    if( pForwarder )
    {
        ESelection aSelection;
        ::GetSelection( aSelection, pForwarder );
        SetSelection( aSelection );

        switch( nControlCharacter )
        {
        case text::ControlCharacter::PARAGRAPH_BREAK:
        {
            const String aText( (char)13, 1 );  // '\r' geht auf'm Mac nicht
            insertString( xRange, aText, bAbsorb );

            return;
        }
        case text::ControlCharacter::LINE_BREAK:
        {
            pForwarder->QuickInsertLineBreak( aSelection );
            return;
        }
        case text::ControlCharacter::APPEND_PARAGRAPH:
        {
            SvxUnoTextRangeBase* pRange = SvxUnoTextRange::getImplementation( xRange );
            if(pRange)
            {
                ESelection aRange = pRange->GetSelection();
//              ESelection aOldSelection = aRange;

                aRange.nStartPos  = pForwarder->GetTextLen( aRange.nStartPara );

                aRange.nEndPara = aRange.nStartPara;
                aRange.nEndPos  = aRange.nStartPos;

                pRange->SetSelection( aRange );
                const String aText( (char)13, 1 );  // '\r' geht auf'm Mac nicht
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
void SAL_CALL SvxUnoText::insertTextContent( const uno::Reference< text::XTextRange >& xRange, const uno::Reference< text::XTextContent >& xContent, sal_Bool bAbsorb )
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

void SAL_CALL SvxUnoText::removeTextContent( const uno::Reference< text::XTextContent >& xContent ) throw(container::NoSuchElementException, uno::RuntimeException)
{
}

// XTextRange

uno::Reference< text::XText > SAL_CALL SvxUnoText::getText()
    throw(uno::RuntimeException)
{
    ESelection aSelection;
    ::GetSelection( aSelection, GetEditSource()->GetTextForwarder() );
    ((SvxUnoText*)this)->SetSelection( aSelection );

    return (text::XText*)this;
}

uno::Reference< text::XTextRange > SAL_CALL SvxUnoText::getStart()
    throw(uno::RuntimeException)
{
    return SvxUnoTextRangeBase::getStart();
}

uno::Reference< text::XTextRange > SAL_CALL SvxUnoText::getEnd()
    throw(uno::RuntimeException)
{
    return SvxUnoTextRangeBase::getEnd();
}

OUString SAL_CALL SvxUnoText::getString() throw( uno::RuntimeException )
{
    return SvxUnoTextRangeBase::getString();
}

void SAL_CALL SvxUnoText::setString( const OUString& aString ) throw(uno::RuntimeException)
{
    SvxUnoTextRangeBase::setString(aString);
}


// XEnumerationAccess
uno::Reference< container::XEnumeration > SAL_CALL SvxUnoText::createEnumeration()
    throw(uno::RuntimeException)
{
    OGuard aGuard( Application::GetSolarMutex() );

    ESelection aSelection;
    ::GetSelection( aSelection, GetEditSource()->GetTextForwarder() );
    SetSelection( aSelection );

    uno::Reference< container::XEnumeration > xEnum( (container::XEnumeration*) new SvxUnoTextContentEnumeration( *this ) );
    return xEnum;
}

// XElementAccess ( container::XEnumerationAccess )
uno::Type SAL_CALL SvxUnoText::getElementType(  ) throw(uno::RuntimeException)
{
    return ::getCppuType((const uno::Reference< text::XTextRange >*)0 );
}

sal_Bool SAL_CALL SvxUnoText::hasElements(  ) throw(uno::RuntimeException)
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
void SAL_CALL SvxUnoText::moveTextRange( const uno::Reference< text::XTextRange >& xRange, sal_Int16 nParagraphs )
    throw(uno::RuntimeException)
{
}

// lang::XServiceInfo
OUString SAL_CALL SvxUnoText::getImplementationName()
    throw(uno::RuntimeException)
{
    return OUString(RTL_CONSTASCII_USTRINGPARAM("SvxUnoText"));
}

uno::Sequence< OUString > SAL_CALL SvxUnoText::getSupportedServiceNames(  )
    throw(uno::RuntimeException)
{
    uno::Sequence< OUString > aSeq( SvxUnoTextRangeBase::getSupportedServiceNames() );
    SvxServiceInfoHelper::addToSequence( aSeq, 1, "com.sun.star.text.Text" );
    return aSeq;
}

const ::com::sun::star::uno::Sequence< sal_Int8 > & SvxUnoText::getUnoTunnelId() throw()
{
    static ::com::sun::star::uno::Sequence< sal_Int8 > * pSeq = 0;
    if( !pSeq )
    {
        ::osl::Guard< ::osl::Mutex > aGuard( ::osl::Mutex::getGlobalMutex() );
        if( !pSeq )
        {
            static ::com::sun::star::uno::Sequence< sal_Int8 > aSeq( 16 );
            rtl_createUuid( (sal_uInt8*)aSeq.getArray(), 0, sal_True );
            pSeq = &aSeq;
        }
    }
    return *pSeq;
}

SvxUnoText* SvxUnoText::getImplementation( uno::Reference< uno::XInterface > xInt ) throw()
{
    ::com::sun::star::uno::Reference< ::com::sun::star::lang::XUnoTunnel > xUT( xInt, ::com::sun::star::uno::UNO_QUERY );
    if( xUT.is() )
        return (SvxUnoText*)xUT->getSomething( SvxUnoText::getUnoTunnelId() );
    else
        return NULL;
}

sal_Int64 SAL_CALL SvxUnoText::getSomething( const ::com::sun::star::uno::Sequence< sal_Int8 >& rId ) throw(::com::sun::star::uno::RuntimeException) \
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



