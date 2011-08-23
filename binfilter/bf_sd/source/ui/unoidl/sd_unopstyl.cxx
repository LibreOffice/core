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



#ifndef _VOS_MUTEX_HXX_ //autogen
#include <vos/mutex.hxx>
#endif

#ifndef _SV_SVAPP_HXX
#include <vcl/svapp.hxx>
#endif


#include <bf_svx/unoshprp.hxx>


#ifndef _SD_STLSHEET_HXX
#include "stlsheet.hxx"
#endif
#ifndef _SFXSMPLHINT_HXX
#include <bf_svtools/smplhint.hxx>
#endif

#include <rtl/uuid.h>
#include <rtl/memory.h>

#ifndef _SVX_XFLBSTIT_HXX 
#include <bf_svx/xflbstit.hxx>
#endif
#ifndef _SVX_XFLBMTIT_HXX 
#include <bf_svx/xflbmtit.hxx>
#endif

#include <bf_svx/unoshape.hxx>
#ifndef _SVX_XIT_HXX
#include <bf_svx/xit.hxx>
#endif

#ifndef _SD_STLPOOL_HXX
#include "stlpool.hxx"
#endif

#include <unopsfm.hxx>
#include <unopstyl.hxx>
#include <unokywds.hxx>
#include <unoprnms.hxx>
#include <glob.hxx>
#include <helpids.h>
namespace binfilter {

using namespace ::rtl;
using namespace ::vos;
using namespace ::com::sun::star;

#define WID_STYLE_DISPNAME	7998
#define WID_STYLE_FAMILY	7999

const SfxItemPropertyMap* ImplGetFullPropertyMap()
{
    static const SfxItemPropertyMap aFullPropertyMap_Impl[] =
    {
        { MAP_CHAR_LEN(UNO_NAME_STYLE_FAMILY),				WID_STYLE_FAMILY, &::getCppuType((const ::rtl::OUString*)0),	::com::sun::star::beans::PropertyAttribute::READONLY,	0},
        { MAP_CHAR_LEN(sUNO_Prop_UserDefinedAttributes),		SDRATTR_XMLATTRIBUTES,		&::getCppuType((const ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameContainer >*)0)  , 		0,     0},
        { MAP_CHAR_LEN("DisplayName"),						WID_STYLE_DISPNAME, &::getCppuType((const ::rtl::OUString*)0),	::com::sun::star::beans::PropertyAttribute::READONLY,	0},

        SVX_UNOEDIT_NUMBERING_PROPERTIE,
        SHADOW_PROPERTIES
        LINE_PROPERTIES
        LINE_PROPERTIES_START_END
        FILL_PROPERTIES
        EDGERADIUS_PROPERTIES
        TEXT_PROPERTIES
        CONNECTOR_PROPERTIES
        SPECIAL_DIMENSIONING_PROPERTIES
        {0,0,0,0,0}
    };

    return aFullPropertyMap_Impl;
}

//UNO3_GETIMPLEMENTATION_IMPL( SdUnoPseudoStyle );

SdUnoPseudoStyle::SdUnoPseudoStyle( SdXImpressDocument* pModel, SfxStyleSheetBase* pStyleSheet ) throw()
: mxModel( pModel ),
  mpModel( pModel ),
  mpPage( NULL ),
  mpStyleSheet( pStyleSheet ),
  meObject( PO_TITLE ),
  maPropSet( ImplGetFullPropertyMap() )
{
    if( mpStyleSheet )
        StartListening( mpStyleSheet->GetPool() );
}

SdUnoPseudoStyle::SdUnoPseudoStyle( SdXImpressDocument* pModel, SdPage* pPage,
                                    SfxStyleSheetBase* pStyleSheet, PresentationObjects eObject ) throw()
: mpModel( pModel ),
  mxModel( pModel ),
  mpPage( pPage ),
  mpStyleSheet( pStyleSheet ),
  meObject( eObject ),
  maPropSet( (eObject >= PO_OUTLINE_2 && eObject <= PO_OUTLINE_9) ? &ImplGetFullPropertyMap()[1] : ImplGetFullPropertyMap() )
{
    if( mpStyleSheet )
        StartListening( mpStyleSheet->GetPool() );
}

SdUnoPseudoStyle::~SdUnoPseudoStyle() throw()
{
}

const ::com::sun::star::uno::Sequence< sal_Int8 > & SdUnoPseudoStyle::getUnoTunnelId() throw()
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

SdUnoPseudoStyle* SdUnoPseudoStyle::getImplementation( const uno::Reference< uno::XInterface >& xInt )
{
    ::com::sun::star::uno::Reference< ::com::sun::star::lang::XUnoTunnel > xUT( xInt, ::com::sun::star::uno::UNO_QUERY );
    if( xUT.is() )
        return (SdUnoPseudoStyle*)xUT->getSomething( SdUnoPseudoStyle::getUnoTunnelId() );
    else
        return NULL;
}

sal_Int64 SAL_CALL SdUnoPseudoStyle::getSomething( const ::com::sun::star::uno::Sequence< sal_Int8 >& rId ) throw(::com::sun::star::uno::RuntimeException)
{
    if( rId.getLength() == 16 && 0 == rtl_compareMemory( getUnoTunnelId().getConstArray(), rId.getConstArray(), 16 ) )
    {
        return (sal_Int64)this;
    }
    else
    {
        return NULL;
    }
}

// XServiceInfo
OUString SAL_CALL SdUnoPseudoStyle::getImplementationName()
    throw(uno::RuntimeException)
{
    return OUString::createFromAscii( sUNO_SdUnoPseudoStyle );
}

sal_Bool SAL_CALL SdUnoPseudoStyle::supportsService( const OUString& ServiceName )
    throw(uno::RuntimeException)
{
    return SvxServiceInfoHelper::supportsService( ServiceName, getSupportedServiceNames() );
}

uno::Sequence< OUString > SAL_CALL SdUnoPseudoStyle::getSupportedServiceNames()
    throw(uno::RuntimeException)
{
    uno::Sequence< OUString > aNameSequence( 10 );
    OUString* pStrings = aNameSequence.getArray();

    *pStrings++ = OUString( RTL_CONSTASCII_USTRINGPARAM( sUNO_Service_Style ) );
    *pStrings++ = OUString( RTL_CONSTASCII_USTRINGPARAM( sUNO_Service_FillProperties ) );
    *pStrings++ = OUString( RTL_CONSTASCII_USTRINGPARAM( sUNO_Service_LineProperties ) );
    *pStrings++ = OUString( RTL_CONSTASCII_USTRINGPARAM( sUNO_Service_ShadowProperties ) );
    *pStrings++ = OUString( RTL_CONSTASCII_USTRINGPARAM( sUNO_Service_ConnectorProperties ) );
    *pStrings++ = OUString( RTL_CONSTASCII_USTRINGPARAM( sUNO_Service_MeasureProperties ) );
    *pStrings++ = OUString( RTL_CONSTASCII_USTRINGPARAM( sUNO_Service_ParagraphProperties ) );
    *pStrings++ = OUString( RTL_CONSTASCII_USTRINGPARAM( sUNO_Service_CharacterProperties ) );
    *pStrings++ = OUString( RTL_CONSTASCII_USTRINGPARAM( sUNO_Service_TextProperties ) );
    *pStrings++ = OUString( RTL_CONSTASCII_USTRINGPARAM( sUNO_Service_Text ) );

    return aNameSequence;
}

// XNamed
OUString SAL_CALL SdUnoPseudoStyle::getName(  )
    throw(uno::RuntimeException)
{
    OGuard aGuard( Application::GetSolarMutex() );

    if( isValid() )
        return SdUnoPseudoStyleFamily::getExternalStyleName( mpStyleSheet->GetName() );

    return OUString();
}

void SAL_CALL SdUnoPseudoStyle::setName( const OUString& aName )
    throw(uno::RuntimeException)
{
}

// XStyle
sal_Bool SAL_CALL SdUnoPseudoStyle::isUserDefined(  )
    throw(uno::RuntimeException)
{
    OGuard aGuard( Application::GetSolarMutex() );
    return mpStyleSheet && mpStyleSheet->IsUserDefined(); 
}

sal_Bool SAL_CALL SdUnoPseudoStyle::isInUse(  )
    throw(uno::RuntimeException)
{
    OGuard aGuard( Application::GetSolarMutex() );

    return mpStyleSheet && mpStyleSheet->IsUsed(); 
}

OUString SAL_CALL SdUnoPseudoStyle::getParentStyle(  )
    throw(uno::RuntimeException)
{
    OGuard aGuard( Application::GetSolarMutex() );

    OUString aName;
    if( isValid() )
        aName = SdUnoPseudoStyleFamily::getExternalStyleName( mpStyleSheet->GetParent() );

    return aName;
}

void SAL_CALL SdUnoPseudoStyle::setParentStyle( const OUString& aParentStyle )
    throw(container::NoSuchElementException, uno::RuntimeException)
{
}

// XPropertySet
uno::Reference< beans::XPropertySetInfo > SAL_CALL SdUnoPseudoStyle::getPropertySetInfo()
    throw(uno::RuntimeException)
{
    return maPropSet.getPropertySetInfo();
}

void SAL_CALL SdUnoPseudoStyle::setPropertyValue( const OUString& aPropertyName, const uno::Any& aValue )
    throw(beans::UnknownPropertyException, beans::PropertyVetoException, lang::IllegalArgumentException, lang::WrappedTargetException, uno::RuntimeException)
{
    OGuard aGuard( Application::GetSolarMutex() );

    const SfxItemPropertyMap* pMap = getPropertyMapEntry( aPropertyName );

    if( pMap == NULL )
    {
        throw beans::UnknownPropertyException();
    }
    else
    {
        if( pMap->nWID == SDRATTR_TEXTDIRECTION )
            return; // not yet implemented for styles

        if( mpStyleSheet )
        {
            if( (pMap->nWID == EE_PARA_NUMBULLET) && (mpStyleSheet->GetFamily() == (SfxStyleFamily) SD_LT_FAMILY) )
            {
                String aEmptyStr;
                const sal_uInt32 nHelpId = mpStyleSheet->GetHelpId( aEmptyStr );

                if( (nHelpId >= HID_PSEUDOSHEET_OUTLINE2) && (nHelpId <= HID_PSEUDOSHEET_OUTLINE9) )
                    return;
            }

            SfxItemSet &rStyleSet = mpStyleSheet->GetItemSet();
    
            if( pMap->nWID == OWN_ATTR_FILLBMP_MODE )
            {
                drawing::BitmapMode eMode;
                if( aValue >>= eMode )
                {
                    rStyleSet.Put( XFillBmpStretchItem( eMode == drawing::BitmapMode_STRETCH ) );
                    rStyleSet.Put( XFillBmpTileItem( eMode == drawing::BitmapMode_REPEAT ) );
                    return;
                }
                throw lang::IllegalArgumentException();
            }
            
            SfxItemPool& rPool = mpStyleSheet->GetPool().GetPool();
            SfxItemSet aSet( rPool,	pMap->nWID, pMap->nWID);
            aSet.Put( rStyleSet );

            if( !aSet.Count() )
            {
                if( EE_PARA_NUMBULLET == pMap->nWID )
                {
                    Font aBulletFont;
                    SdStyleSheetPool::PutNumBulletItem( mpStyleSheet, aBulletFont );
                    aSet.Put( rStyleSet );
                }
                else
                {
                    aSet.Put( rPool.GetDefaultItem( pMap->nWID ) );
                }
            }

            if( pMap->nMemberId == MID_NAME &&
                ( pMap->nWID == XATTR_FILLBITMAP || pMap->nWID == XATTR_FILLGRADIENT ||
                  pMap->nWID == XATTR_FILLHATCH || pMap->nWID == XATTR_FILLFLOATTRANSPARENCE ||
                  pMap->nWID == XATTR_LINESTART || pMap->nWID == XATTR_LINEEND || pMap->nWID == XATTR_LINEDASH) )
            {
                OUString aName;
                if(!(aValue >>= aName ))
                    throw lang::IllegalArgumentException();

                SvxShape::SetFillAttribute( pMap->nWID, aName, aSet );
            }
            else if(!SvxUnoTextRangeBase::SetPropertyValueHelper( aSet, pMap, aValue, aSet ))
            {
                maPropSet.setPropertyValue( pMap, aValue, aSet );
            }

            rStyleSet.Put( aSet );
            ((SdStyleSheet*)mpStyleSheet)->Broadcast(SfxSimpleHint(SFX_HINT_DATACHANGED));
        }
        else
        {
            if(pMap && pMap->nWID)
                maPropSet.setPropertyValue( pMap, aValue );
        }

        if( mpModel )
            mpModel->SetModified();
    }
}

uno::Any SAL_CALL SdUnoPseudoStyle::getPropertyValue( const OUString& PropertyName ) throw(beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException)
{
    OGuard aGuard( Application::GetSolarMutex() );

    uno::Any aAny;
    const SfxItemPropertyMap* pMap = getPropertyMapEntry( PropertyName );

    if( pMap == NULL )
    {
        throw beans::UnknownPropertyException();
    }
    else
    {
        if( mpStyleSheet )
        {
            if( pMap->nWID == WID_STYLE_FAMILY )
            {
                aAny <<= OUString::createFromAscii( "presentation" );
            }
            else if( pMap->nWID == WID_STYLE_DISPNAME )
            {
                String aStyleName( mpStyleSheet->GetName() );
                xub_StrLen nPos = aStyleName.Search(String( RTL_CONSTASCII_USTRINGPARAM(SD_LT_SEPARATOR)));
                if( nPos != STRING_NOTFOUND )
                    aStyleName.Erase(0, nPos+4);

                aAny <<= OUString( aStyleName );
            }
            else if( pMap->nWID == SDRATTR_TEXTDIRECTION )
            {
                sal_Bool bWritingMode = sal_False;
                aAny = uno::Any( &bWritingMode, ::getBooleanCppuType() );
            }
            else if( pMap->nWID == OWN_ATTR_FILLBMP_MODE )
            {
                SfxItemSet &rStyleSet = mpStyleSheet->GetItemSet();

                XFillBmpStretchItem* pStretchItem = (XFillBmpStretchItem*)rStyleSet.GetItem(XATTR_FILLBMP_STRETCH);
                XFillBmpTileItem* pTileItem = (XFillBmpTileItem*)rStyleSet.GetItem(XATTR_FILLBMP_TILE);

                if( pStretchItem && pTileItem )
                {
                    if( pTileItem->GetValue() )
                        aAny <<= drawing::BitmapMode_REPEAT;
                    else if( pStretchItem->GetValue() )
                        aAny <<= drawing::BitmapMode_STRETCH;
                    else
                        aAny <<= drawing::BitmapMode_NO_REPEAT;
                }
            }
            else
            {
                SfxItemPool& rPool = mpStyleSheet->GetPool().GetPool();
                SfxItemSet aSet( rPool,	pMap->nWID, pMap->nWID);

                const SfxPoolItem* pItem;
                SfxItemSet& rStyleSet = mpStyleSheet->GetItemSet();

                if( rStyleSet.GetItemState(	pMap->nWID, sal_True, &pItem ) == SFX_ITEM_SET )
                    aSet.Put(  *pItem );

                if( !aSet.Count() )
                    aSet.Put( rPool.GetDefaultItem( pMap->nWID ) );

                if(SvxUnoTextRangeBase::GetPropertyValueHelper( aSet, pMap, aAny ))
                    return aAny;

                // Hole Wert aus ItemSet
                aAny = maPropSet.getPropertyValue( pMap, aSet );	
            }
        }
        else
        {
            if(pMap->nWID)
                aAny = maPropSet.getPropertyValue( pMap );
        }

        if( *pMap->pType != aAny.getValueType() )
        {
            // since the sfx uint16 item now exports a sal_Int32, we may have to fix this here
            if( ( *pMap->pType == ::getCppuType((const sal_Int16*)0)) && aAny.getValueType() == ::getCppuType((const sal_Int32*)0) )
            {
                sal_Int32 nValue;
                aAny >>= nValue;
                aAny <<= (sal_Int16)nValue;
            }
            else
            {
                DBG_ERROR("SvxShape::GetAnyForItem() Returnvalue has wrong Type!" );
            }
        }
    }
    return aAny;
}

void SAL_CALL SdUnoPseudoStyle::addPropertyChangeListener( const OUString& aPropertyName, const uno::Reference< beans::XPropertyChangeListener >& xListener ) throw(beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException) {}
void SAL_CALL SdUnoPseudoStyle::removePropertyChangeListener( const OUString& aPropertyName, const uno::Reference< beans::XPropertyChangeListener >& aListener ) throw(beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException) {}
void SAL_CALL SdUnoPseudoStyle::addVetoableChangeListener( const OUString& PropertyName, const uno::Reference< beans::XVetoableChangeListener >& aListener ) throw(beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException) {}
void SAL_CALL SdUnoPseudoStyle::removeVetoableChangeListener( const OUString& PropertyName, const uno::Reference< beans::XVetoableChangeListener >& aListener ) throw(beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException) {}

// XPropertyState
beans::PropertyState SAL_CALL SdUnoPseudoStyle::getPropertyState( const OUString& PropertyName ) throw(beans::UnknownPropertyException, uno::RuntimeException)
{
    OGuard aGuard( Application::GetSolarMutex() );

    const SfxItemPropertyMap* pMap = getPropertyMapEntry( PropertyName );

    if( pMap == NULL )
        throw beans::UnknownPropertyException();

    if( pMap->nWID == WID_STYLE_FAMILY )
    {
        return beans::PropertyState_DIRECT_VALUE;
    }
    else if( pMap->nWID == SDRATTR_TEXTDIRECTION )
    {
        return beans::PropertyState_DEFAULT_VALUE;
    }
    else if( pMap->nWID == OWN_ATTR_FILLBMP_MODE )
    {
        const SfxItemSet& rSet = mpStyleSheet->GetItemSet();

        if( rSet.GetItemState( XATTR_FILLBMP_STRETCH, false ) == SFX_ITEM_SET ||
            rSet.GetItemState( XATTR_FILLBMP_TILE, false ) == SFX_ITEM_SET )
        {
            return beans::PropertyState_DIRECT_VALUE;
        }
        else
        {
            return beans::PropertyState_AMBIGUOUS_VALUE;
        }
    }
    else if( mpStyleSheet )
    {
        SfxItemSet &rStyleSet = mpStyleSheet->GetItemSet();

        beans::PropertyState eState;

        switch( rStyleSet.GetItemState( pMap->nWID, sal_False ) )
        {
        case SFX_ITEM_READONLY:
        case SFX_ITEM_SET:
            eState = beans::PropertyState_DIRECT_VALUE;
            break;
        case SFX_ITEM_DEFAULT:
            eState = beans::PropertyState_DEFAULT_VALUE;
            break;
//		case SFX_ITEM_DONTCARE:
//		case SFX_ITEM_DISABLED:
        default:
            eState = beans::PropertyState_AMBIGUOUS_VALUE;
            break;
        }
        
        // if a item is set, this doesn't mean we want it :)
        if( ( beans::PropertyState_DIRECT_VALUE == eState ) )
        {
            switch( pMap->nWID )
            {
            case XATTR_FILLBITMAP:
            case XATTR_FILLGRADIENT:
            case XATTR_FILLHATCH:
            case XATTR_FILLFLOATTRANSPARENCE:
            case XATTR_LINEEND:
            case XATTR_LINESTART:
            case XATTR_LINEDASH:
                {
                    NameOrIndex* pItem = (NameOrIndex*)rStyleSet.GetItem((USHORT)pMap->nWID);
                    if( ( pItem == NULL ) || ( pItem->GetName().Len() == 0) )
                        eState = beans::PropertyState_DEFAULT_VALUE;
                }
            }	
        }

        return eState;
    }
    else
    {
        if( NULL == maPropSet.GetUsrAnyForID(pMap->nWID) )
            return beans::PropertyState_DEFAULT_VALUE;
        else
            return beans::PropertyState_DIRECT_VALUE;
    }
}

uno::Sequence< beans::PropertyState > SAL_CALL SdUnoPseudoStyle::getPropertyStates( const uno::Sequence< OUString >& aPropertyName ) throw(beans::UnknownPropertyException, uno::RuntimeException)
{
    OGuard aGuard( Application::GetSolarMutex() );

    sal_Int32 nCount = aPropertyName.getLength();
    const OUString* pNames = aPropertyName.getConstArray();

    uno::Sequence< beans::PropertyState > aPropertyStateSequence( nCount );
    beans::PropertyState* pState = aPropertyStateSequence.getArray();

    while( nCount-- )
        *pState++ = getPropertyState( *pNames++ );

    return aPropertyStateSequence;
}

void SAL_CALL SdUnoPseudoStyle::setPropertyToDefault( const OUString& PropertyName ) throw(beans::UnknownPropertyException, uno::RuntimeException)
{
    OGuard aGuard( Application::GetSolarMutex() );

    uno::Any aAny;
    const SfxItemPropertyMap* pMap = getPropertyMapEntry( PropertyName );

    if( pMap == NULL )
        throw beans::UnknownPropertyException();

    if( mpStyleSheet )
    {
        SfxItemSet &rStyleSet = mpStyleSheet->GetItemSet();

        if( pMap->nWID == OWN_ATTR_FILLBMP_MODE )
        {
            rStyleSet.ClearItem( XATTR_FILLBMP_STRETCH );
            rStyleSet.ClearItem( XATTR_FILLBMP_TILE );
        }
        else
        {
            rStyleSet.ClearItem( pMap->nWID );
            ((SdStyleSheet*)mpStyleSheet)->Broadcast(SfxSimpleHint(SFX_HINT_DATACHANGED));
        }
    }
    if( mpModel )
        mpModel->SetModified();
}

uno::Any SAL_CALL SdUnoPseudoStyle::getPropertyDefault( const OUString& aPropertyName ) throw(beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException)
{
    OGuard aGuard( Application::GetSolarMutex() );

    uno::Any aAny;
    const SfxItemPropertyMap* pMap = getPropertyMapEntry( aPropertyName );

    if( pMap == NULL || mpStyleSheet == NULL )
        throw beans::UnknownPropertyException();

    if( pMap->nWID == WID_STYLE_FAMILY )
    {
        aAny <<= OUString::createFromAscii( "presentation" );
    }
    else if( pMap->nWID == SDRATTR_TEXTDIRECTION )
    {
        sal_Bool bWritingMode = sal_False;
        aAny = uno::Any( &bWritingMode, ::getBooleanCppuType() );
    }
    else if( pMap->nWID == OWN_ATTR_FILLBMP_MODE )
    {
        aAny <<= drawing::BitmapMode_REPEAT;
    }
    else if( mpStyleSheet )
    {
        SfxItemPool& rPool = mpStyleSheet->GetPool().GetPool();
        SfxItemSet aSet( rPool,	pMap->nWID, pMap->nWID);
        aSet.Put( rPool.GetDefaultItem( pMap->nWID ) );

        aAny = maPropSet.getPropertyValue( pMap, aSet );	
    }
    return aAny;
}

/** detect if the StyleSheetPool dies or if this instances style sheet is erased */
void SdUnoPseudoStyle::Notify( SfxBroadcaster& rBC, const SfxHint& rHint )
{
    sal_Bool bGoneDead = sal_False;

    const SfxSimpleHint* pSimpleHint = PTR_CAST( SfxSimpleHint, &rHint );
    if( pSimpleHint && pSimpleHint->GetId() == SFX_HINT_DYING )
        bGoneDead = sal_True;

    const SfxStyleSheetHint* pStyleSheetHint = PTR_CAST( SfxStyleSheetHint, &rHint );
    if( pStyleSheetHint && pStyleSheetHint->GetHint() == SFX_STYLESHEET_ERASED )
    {
        if( pStyleSheetHint->GetStyleSheet() == mpStyleSheet )
            bGoneDead = sal_True;
    }

    if( bGoneDead )
        mpStyleSheet = NULL;
}

/** this is used because our property map is not sorted yet */
const SfxItemPropertyMap* SdUnoPseudoStyle::getPropertyMapEntry( const OUString& rPropertyName ) const throw()
{
    const SfxItemPropertyMap*pMap = maPropSet.getPropertyMap();
    while( pMap->pName )
    {
        if( rPropertyName.compareToAscii( pMap->pName ) == 0 )
            return pMap;
        ++pMap;
    }

    return NULL;
}

}
