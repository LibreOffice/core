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

#include <com/sun/star/beans/PropertyState.hpp>
#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <com/sun/star/awt/FontDescriptor.hpp>
#include <com/sun/star/awt/FontWidth.hpp>
#include <com/sun/star/awt/FontWeight.hpp>
#include <com/sun/star/awt/FontSlant.hpp>
#include <com/sun/star/awt/MouseWheelBehavior.hpp>
#include <com/sun/star/graphic/XGraphicProvider.hpp>
#include <com/sun/star/awt/XDevice.hpp>
#include <com/sun/star/text/WritingMode2.hpp>
#include <com/sun/star/io/XMarkableStream.hpp>
#include <toolkit/controls/unocontrolmodel.hxx>
#include <toolkit/helper/macros.hxx>
#include <cppuhelper/supportsservice.hxx>
#include <cppuhelper/typeprovider.hxx>
#include <rtl/uuid.h>
#include <tools/diagnose_ex.h>
#include <tools/date.hxx>
#include <tools/time.hxx>
#include <tools/debug.hxx>
#include <toolkit/helper/property.hxx>
#include <toolkit/helper/vclunohelper.hxx>
#include <toolkit/helper/emptyfontdescriptor.hxx>
#include <com/sun/star/lang/Locale.hpp>
#include <unotools/localedatawrapper.hxx>
#include <unotools/configmgr.hxx>
#include <comphelper/processfactory.hxx>
#include <comphelper/sequence.hxx>
#include <comphelper/extract.hxx>
#include <vcl/svapp.hxx>
#include <uno/data.h>

#include <memory>
#include <set>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::i18n;
using ::com::sun::star::awt::FontDescriptor;


#define UNOCONTROL_STREAMVERSION    (short)2

static void lcl_ImplMergeFontProperty( FontDescriptor& rFD, sal_uInt16 nPropId, const Any& rValue )
{
    // some props are defined with other types than the matching FontDescriptor members have
    // (e.g. FontWidth, FontSlant)
    // 78474 - 09/19/2000 - FS
    float       nExtractFloat = 0;
    sal_Int16   nExtractShort = 0;

    switch ( nPropId )
    {
        case BASEPROPERTY_FONTDESCRIPTORPART_NAME:          rValue >>= rFD.Name;
                                                            break;
        case BASEPROPERTY_FONTDESCRIPTORPART_STYLENAME:     rValue >>= rFD.StyleName;
                                                            break;
        case BASEPROPERTY_FONTDESCRIPTORPART_FAMILY:        rValue >>= rFD.Family;
                                                            break;
        case BASEPROPERTY_FONTDESCRIPTORPART_CHARSET:       rValue >>= rFD.CharSet;
                                                            break;
        case BASEPROPERTY_FONTDESCRIPTORPART_HEIGHT:        rValue >>= nExtractFloat; rFD.Height = (sal_Int16)nExtractFloat;
                                                            break;
        case BASEPROPERTY_FONTDESCRIPTORPART_WEIGHT:        rValue >>= rFD.Weight;
                                                            break;
        case BASEPROPERTY_FONTDESCRIPTORPART_SLANT:         if ( rValue >>= nExtractShort )
                                                                rFD.Slant = (css::awt::FontSlant)nExtractShort;
                                                            else
                                                                rValue >>= rFD.Slant;
                                                            break;
        case BASEPROPERTY_FONTDESCRIPTORPART_UNDERLINE:     rValue >>= rFD.Underline;
                                                            break;
        case BASEPROPERTY_FONTDESCRIPTORPART_STRIKEOUT:     rValue >>= rFD.Strikeout;
                                                            break;
        case BASEPROPERTY_FONTDESCRIPTORPART_WIDTH:         rValue >>= rFD.Width;
                                                            break;
        case BASEPROPERTY_FONTDESCRIPTORPART_PITCH:         rValue >>= rFD.Pitch;
                                                            break;
        case BASEPROPERTY_FONTDESCRIPTORPART_CHARWIDTH:     rValue >>= rFD.CharacterWidth;
                                                            break;
        case BASEPROPERTY_FONTDESCRIPTORPART_ORIENTATION:   rValue >>= rFD.Orientation;
                                                            break;
        case BASEPROPERTY_FONTDESCRIPTORPART_KERNING:       rValue >>= rFD.Kerning;
                                                            break;
        case BASEPROPERTY_FONTDESCRIPTORPART_WORDLINEMODE:  rValue >>= rFD.WordLineMode;
                                                            break;
        case BASEPROPERTY_FONTDESCRIPTORPART_TYPE:          rValue >>= rFD.Type;
                                                            break;
        default:                                            OSL_FAIL( "FontProperty?!" );
    }
}


//  class UnoControlModel

UnoControlModel::UnoControlModel( const Reference< XComponentContext >& rxContext )
    :UnoControlModel_Base()
    ,MutexAndBroadcastHelper()
    ,OPropertySetHelper( BrdcstHelper )
    ,maDisposeListeners( *this )
    ,m_xContext( rxContext )
{
    // Insert properties from Model into table,
    // only existing properties are valid, even if they're VOID
}

UnoControlModel::UnoControlModel( const UnoControlModel& rModel )
    : UnoControlModel_Base()
    , MutexAndBroadcastHelper()
    , OPropertySetHelper( BrdcstHelper )
    , maData( rModel.maData )
    , maDisposeListeners( *this )
    , m_xContext( rModel.m_xContext )
{
}

css::uno::Sequence<sal_Int32> UnoControlModel::ImplGetPropertyIds() const
{
    sal_uInt32 nIDs = maData.size();
    css::uno::Sequence<sal_Int32>  aIDs( nIDs );
    sal_Int32* pIDs = aIDs.getArray();
    sal_uInt32 n = 0;
    for ( ImplPropertyTable::const_iterator it = maData.begin(); it != maData.end(); ++it )
        pIDs[n++] = it->first;
    return aIDs;
}

bool UnoControlModel::ImplHasProperty( sal_uInt16 nPropId ) const
{
    if ( ( nPropId >= BASEPROPERTY_FONTDESCRIPTORPART_START ) && ( nPropId <= BASEPROPERTY_FONTDESCRIPTORPART_END ) )
        nPropId = BASEPROPERTY_FONTDESCRIPTOR;

    return maData.find( nPropId ) != maData.end();
}

css::uno::Any UnoControlModel::ImplGetDefaultValue( sal_uInt16 nPropId ) const
{
    css::uno::Any aDefault;

    if (
        (nPropId == BASEPROPERTY_FONTDESCRIPTOR) ||
        (
         (nPropId >= BASEPROPERTY_FONTDESCRIPTORPART_START) &&
         (nPropId <= BASEPROPERTY_FONTDESCRIPTORPART_END)
        )
       )
    {
        EmptyFontDescriptor aFD;
        switch ( nPropId )
        {
            case BASEPROPERTY_FONTDESCRIPTOR:                   aDefault <<= aFD;                   break;
            case BASEPROPERTY_FONTDESCRIPTORPART_NAME:          aDefault <<= aFD.Name;              break;
            case BASEPROPERTY_FONTDESCRIPTORPART_STYLENAME:     aDefault <<= aFD.StyleName;         break;
            case BASEPROPERTY_FONTDESCRIPTORPART_FAMILY:        aDefault <<= aFD.Family;            break;
            case BASEPROPERTY_FONTDESCRIPTORPART_CHARSET:       aDefault <<= aFD.CharSet;           break;
            case BASEPROPERTY_FONTDESCRIPTORPART_HEIGHT:        aDefault <<= (float)aFD.Height;     break;
            case BASEPROPERTY_FONTDESCRIPTORPART_WEIGHT:        aDefault <<= aFD.Weight;            break;
            case BASEPROPERTY_FONTDESCRIPTORPART_SLANT:         aDefault <<= (sal_Int16)aFD.Slant;  break;
            case BASEPROPERTY_FONTDESCRIPTORPART_UNDERLINE:     aDefault <<= aFD.Underline;         break;
            case BASEPROPERTY_FONTDESCRIPTORPART_STRIKEOUT:     aDefault <<= aFD.Strikeout;         break;
            case BASEPROPERTY_FONTDESCRIPTORPART_WIDTH:         aDefault <<= aFD.Width;             break;
            case BASEPROPERTY_FONTDESCRIPTORPART_PITCH:         aDefault <<= aFD.Pitch;             break;
            case BASEPROPERTY_FONTDESCRIPTORPART_CHARWIDTH:     aDefault <<= aFD.CharacterWidth;    break;
            case BASEPROPERTY_FONTDESCRIPTORPART_ORIENTATION:   aDefault <<= aFD.Orientation;       break;
            case BASEPROPERTY_FONTDESCRIPTORPART_KERNING:       aDefault <<= aFD.Kerning;           break;
            case BASEPROPERTY_FONTDESCRIPTORPART_WORDLINEMODE:  aDefault <<= aFD.WordLineMode;      break;
            case BASEPROPERTY_FONTDESCRIPTORPART_TYPE:          aDefault <<= aFD.Type;              break;
            default: OSL_FAIL( "FontProperty?!" );
        }
    }
    else
    {
        switch ( nPropId )
        {
            case BASEPROPERTY_GRAPHIC:
                aDefault <<= Reference< graphic::XGraphic >();
                break;

            case BASEPROPERTY_REFERENCE_DEVICE:
                aDefault <<= Reference< awt::XDevice >();
                break;

            case BASEPROPERTY_ITEM_SEPARATOR_POS:
            case BASEPROPERTY_VERTICALALIGN:
            case BASEPROPERTY_BORDERCOLOR:
            case BASEPROPERTY_SYMBOL_COLOR:
            case BASEPROPERTY_TABSTOP:
            case BASEPROPERTY_TEXTCOLOR:
            case BASEPROPERTY_TEXTLINECOLOR:
            case BASEPROPERTY_DATE:
            case BASEPROPERTY_DATESHOWCENTURY:
            case BASEPROPERTY_TIME:
            case BASEPROPERTY_VALUE_DOUBLE:
            case BASEPROPERTY_PROGRESSVALUE:
            case BASEPROPERTY_SCROLLVALUE:
            case BASEPROPERTY_VISIBLESIZE:
            case BASEPROPERTY_BACKGROUNDCOLOR:
            case BASEPROPERTY_FILLCOLOR:            break;  // Void

            case BASEPROPERTY_FONTRELIEF:
            case BASEPROPERTY_FONTEMPHASISMARK:
            case BASEPROPERTY_MAXTEXTLEN:
            case BASEPROPERTY_STATE:
            case BASEPROPERTY_EXTDATEFORMAT:
            case BASEPROPERTY_EXTTIMEFORMAT:
            case BASEPROPERTY_ECHOCHAR:             aDefault <<= (sal_Int16) 0; break;
            case BASEPROPERTY_BORDER:               aDefault <<= (sal_Int16) 1; break;
            case BASEPROPERTY_DECIMALACCURACY:      aDefault <<= (sal_Int16) 2; break;
            case BASEPROPERTY_LINECOUNT:            aDefault <<= (sal_Int16) 5; break;
            case BASEPROPERTY_ALIGN:                aDefault <<= (sal_Int16) PROPERTY_ALIGN_LEFT; break;
            case BASEPROPERTY_IMAGEALIGN:           aDefault <<= (sal_Int16) 1 /*ImageAlign::Top*/; break;
            case BASEPROPERTY_IMAGEPOSITION:        aDefault <<= (sal_Int16) 12 /*ImagePosition::Centered*/; break;
            case BASEPROPERTY_PUSHBUTTONTYPE:       aDefault <<= (sal_Int16) 0 /*PushButtonType::STANDARD*/; break;
            case BASEPROPERTY_MOUSE_WHEEL_BEHAVIOUR:aDefault <<= (sal_Int16) awt::MouseWheelBehavior::SCROLL_FOCUS_ONLY; break;

            case BASEPROPERTY_DATEMAX:              aDefault <<= util::Date( 31, 12, 2200 );    break;
            case BASEPROPERTY_DATEMIN:              aDefault <<= util::Date( 1, 1, 1900 );  break;
            case BASEPROPERTY_TIMEMAX:              aDefault <<= util::Time(0, 0, 59, 23, false);  break;
            case BASEPROPERTY_TIMEMIN:              aDefault <<= util::Time();      break;
            case BASEPROPERTY_VALUEMAX_DOUBLE:      aDefault <<= (double) 1000000;  break;
            case BASEPROPERTY_VALUEMIN_DOUBLE:      aDefault <<= (double) -1000000; break;
            case BASEPROPERTY_VALUESTEP_DOUBLE:     aDefault <<= (double ) 1;       break;
            case BASEPROPERTY_PROGRESSVALUE_MAX:    aDefault <<= (sal_Int32) 100;   break;
            case BASEPROPERTY_PROGRESSVALUE_MIN:    aDefault <<= (sal_Int32)   0;   break;
            case BASEPROPERTY_SCROLLVALUE_MAX:      aDefault <<= (sal_Int32) 100;   break;
            case BASEPROPERTY_SCROLLVALUE_MIN:      aDefault <<= (sal_Int32)   0;   break;
            case BASEPROPERTY_LINEINCREMENT:        aDefault <<= (sal_Int32)   1;   break;
            case BASEPROPERTY_BLOCKINCREMENT:       aDefault <<= (sal_Int32)  10;   break;
            case BASEPROPERTY_ORIENTATION:          aDefault <<= (sal_Int32)   0;   break;
            case BASEPROPERTY_SPINVALUE:            aDefault <<= (sal_Int32)   0;   break;
            case BASEPROPERTY_SPININCREMENT:        aDefault <<= (sal_Int32)   1;   break;
            case BASEPROPERTY_SPINVALUE_MIN:        aDefault <<= (sal_Int32)   0;   break;
            case BASEPROPERTY_SPINVALUE_MAX:        aDefault <<= (sal_Int32) 100;   break;
            case BASEPROPERTY_REPEAT_DELAY:         aDefault <<= (sal_Int32)  50;   break;    // 50 milliseconds
            case BASEPROPERTY_DEFAULTCONTROL:       aDefault <<= const_cast<UnoControlModel*>(this)->getServiceName();    break;

            case BASEPROPERTY_AUTOHSCROLL:
            case BASEPROPERTY_AUTOVSCROLL:
            case BASEPROPERTY_MOVEABLE:
            case BASEPROPERTY_CLOSEABLE:
            case BASEPROPERTY_SIZEABLE:
            case BASEPROPERTY_HSCROLL:
            case BASEPROPERTY_DEFAULTBUTTON:
            case BASEPROPERTY_MULTILINE:
            case BASEPROPERTY_MULTISELECTION:
            case BASEPROPERTY_TRISTATE:
            case BASEPROPERTY_DROPDOWN:
            case BASEPROPERTY_SPIN:
            case BASEPROPERTY_READONLY:
            case BASEPROPERTY_VSCROLL:
            case BASEPROPERTY_NUMSHOWTHOUSANDSEP:
            case BASEPROPERTY_STRICTFORMAT:
            case BASEPROPERTY_REPEAT:
            case BASEPROPERTY_PAINTTRANSPARENT:
            case BASEPROPERTY_DESKTOP_AS_PARENT:
            case BASEPROPERTY_HARDLINEBREAKS:
            case BASEPROPERTY_NOLABEL:              aDefault <<= false; break;

            case BASEPROPERTY_MULTISELECTION_SIMPLEMODE:
            case BASEPROPERTY_HIDEINACTIVESELECTION:
            case BASEPROPERTY_ENFORCE_FORMAT:
            case BASEPROPERTY_AUTOCOMPLETE:
            case BASEPROPERTY_SCALEIMAGE:
            case BASEPROPERTY_ENABLED:
            case BASEPROPERTY_PRINTABLE:
            case BASEPROPERTY_ENABLEVISIBLE:
            case BASEPROPERTY_DECORATION:           aDefault <<= true; break;

            case BASEPROPERTY_GROUPNAME:
            case BASEPROPERTY_HELPTEXT:
            case BASEPROPERTY_HELPURL:
            case BASEPROPERTY_IMAGEURL:
            case BASEPROPERTY_DIALOGSOURCEURL:
            case BASEPROPERTY_EDITMASK:
            case BASEPROPERTY_LITERALMASK:
            case BASEPROPERTY_LABEL:
            case BASEPROPERTY_TITLE:
            case BASEPROPERTY_TEXT:                 aDefault <<= OUString(); break;

            case BASEPROPERTY_WRITING_MODE:
            case BASEPROPERTY_CONTEXT_WRITING_MODE:
                aDefault <<= text::WritingMode2::CONTEXT;
                break;

            case BASEPROPERTY_STRINGITEMLIST:
            {
                css::uno::Sequence< OUString> aStringSeq;
                aDefault <<= aStringSeq;

            }
            break;
            case BASEPROPERTY_TYPEDITEMLIST:
            {
                css::uno::Sequence< css::uno::Any > aAnySeq;
                aDefault <<= aAnySeq;

            }
            break;
            case BASEPROPERTY_SELECTEDITEMS:
            {
                css::uno::Sequence<sal_Int16> aINT16Seq;
                aDefault <<= aINT16Seq;
            }
            break;
            case BASEPROPERTY_CURRENCYSYMBOL:
            {
                OUString sDefaultCurrency(
                    utl::ConfigManager::getDefaultCurrency() );

                // extract the bank symbol
                sal_Int32 nSepPos = sDefaultCurrency.indexOf( '-' );
                OUString sBankSymbol;
                if ( nSepPos >= 0 )
                {
                    sBankSymbol = sDefaultCurrency.copy( 0, nSepPos );
                    sDefaultCurrency = sDefaultCurrency.copy( nSepPos + 1 );
                }

                // the remaining is the locale
                LanguageTag aLanguageTag( sDefaultCurrency);
                LocaleDataWrapper aLocaleInfo( m_xContext, aLanguageTag );
                if ( sBankSymbol.isEmpty() )
                    sBankSymbol = aLocaleInfo.getCurrBankSymbol();

                // look for the currency entry (for this language) which has the given bank symbol
                Sequence< Currency2 > aAllCurrencies = aLocaleInfo.getAllCurrencies();
                const Currency2* pAllCurrencies     =                       aAllCurrencies.getConstArray();
                const Currency2* pAllCurrenciesEnd  =   pAllCurrencies  +   aAllCurrencies.getLength();

                OUString sCurrencySymbol = aLocaleInfo.getCurrSymbol();
                if ( sBankSymbol.isEmpty() )
                {
                    DBG_ASSERT( pAllCurrencies != pAllCurrenciesEnd, "UnoControlModel::ImplGetDefaultValue: no currencies at all!" );
                    if ( pAllCurrencies != pAllCurrenciesEnd )
                    {
                        sBankSymbol = pAllCurrencies->BankSymbol;
                        sCurrencySymbol = pAllCurrencies->Symbol;
                    }
                }

                if ( !sBankSymbol.isEmpty() )
                {
                    bool bLegacy = false;
                    for ( ;pAllCurrencies != pAllCurrenciesEnd; ++pAllCurrencies )
                        if ( pAllCurrencies->BankSymbol == sBankSymbol )
                        {
                            sCurrencySymbol = pAllCurrencies->Symbol;
                            if ( pAllCurrencies->LegacyOnly )
                                bLegacy = true;
                            else
                                break;
                        }
                    DBG_ASSERT( bLegacy || pAllCurrencies != pAllCurrenciesEnd, "UnoControlModel::ImplGetDefaultValue: did not find the given bank symbol!" );
                    (void)bLegacy;
                }

                aDefault <<= sCurrencySymbol;
            }
            break;

            default:    OSL_FAIL( "ImplGetDefaultValue - unknown Property" );
        }
    }

    return aDefault;
}

void UnoControlModel::ImplRegisterProperty( sal_uInt16 nPropId, const css::uno::Any& rDefault )
{
    maData[ nPropId ] = rDefault;
}

void UnoControlModel::ImplRegisterProperty( sal_uInt16 nPropId )
{
    ImplRegisterProperty( nPropId, ImplGetDefaultValue( nPropId ) );

    if ( nPropId == BASEPROPERTY_FONTDESCRIPTOR )
    {
        // some properties are not included in the FontDescriptor, but every time
        // when we have a FontDescriptor we want to have these properties too.
        // => Easier to register the here, instead everywhere where I register the FontDescriptor...

        ImplRegisterProperty( BASEPROPERTY_TEXTCOLOR );
        ImplRegisterProperty( BASEPROPERTY_TEXTLINECOLOR );
        ImplRegisterProperty( BASEPROPERTY_FONTRELIEF );
        ImplRegisterProperty( BASEPROPERTY_FONTEMPHASISMARK );
    }
}

void UnoControlModel::ImplRegisterProperties( const std::vector< sal_uInt16 > &rIds )
{
    std::vector< sal_uInt16 >::const_iterator iter;
    for( iter = rIds.begin(); iter != rIds.end(); ++iter)
    {
        if( !ImplHasProperty( *iter ) )
            ImplRegisterProperty( *iter, ImplGetDefaultValue( *iter ) );
    }
}

// css::uno::XInterface
css::uno::Any UnoControlModel::queryAggregation( const css::uno::Type & rType )
{
    Any aRet = UnoControlModel_Base::queryAggregation( rType );
    if ( !aRet.hasValue() )
        aRet = ::cppu::OPropertySetHelper::queryInterface( rType );
    return aRet;
}

// css::lang::XUnoTunnel
IMPL_XUNOTUNNEL_MINIMAL( UnoControlModel )

// XInterface
IMPLEMENT_FORWARD_REFCOUNT( UnoControlModel, UnoControlModel_Base )

// css::lang::XTypeProvider
IMPLEMENT_FORWARD_XTYPEPROVIDER2( UnoControlModel, UnoControlModel_Base, ::cppu::OPropertySetHelper )


uno::Reference< util::XCloneable > UnoControlModel::createClone()
{
    UnoControlModel* pClone = Clone();
    uno::Reference< util::XCloneable > xClone( static_cast<cppu::OWeakObject*>(pClone), uno::UNO_QUERY );
    return xClone;
}

// css::lang::XComponent
void UnoControlModel::dispose(  )
{
    ::osl::Guard< ::osl::Mutex > aGuard( GetMutex() );

    css::lang::EventObject aEvt;
    aEvt.Source = static_cast<css::uno::XAggregation*>(static_cast<cppu::OWeakAggObject*>(this));
    maDisposeListeners.disposeAndClear( aEvt );

    BrdcstHelper.aLC.disposeAndClear( aEvt );

    // let the property set helper notify our property listeners
    OPropertySetHelper::disposing();
}

void UnoControlModel::addEventListener( const css::uno::Reference< css::lang::XEventListener >& rxListener )
{
    ::osl::Guard< ::osl::Mutex > aGuard( GetMutex() );

    maDisposeListeners.addInterface( rxListener );
}

void UnoControlModel::removeEventListener( const css::uno::Reference< css::lang::XEventListener >& rxListener )
{
    ::osl::Guard< ::osl::Mutex > aGuard( GetMutex() );

    maDisposeListeners.removeInterface( rxListener );
}


// css::beans::XPropertyState
css::beans::PropertyState UnoControlModel::getPropertyState( const OUString& PropertyName )
{
    ::osl::Guard< ::osl::Mutex > aGuard( GetMutex() );

    sal_uInt16 nPropId = GetPropertyId( PropertyName );

    css::uno::Any aValue = getPropertyValue( PropertyName );
    css::uno::Any aDefault = ImplGetDefaultValue( nPropId );

    return CompareProperties( aValue, aDefault ) ? css::beans::PropertyState_DEFAULT_VALUE : css::beans::PropertyState_DIRECT_VALUE;
}

css::uno::Sequence< css::beans::PropertyState > UnoControlModel::getPropertyStates( const css::uno::Sequence< OUString >& PropertyNames )
{
    ::osl::Guard< ::osl::Mutex > aGuard( GetMutex() );

    sal_uInt32 nNames = PropertyNames.getLength();
    const OUString* pNames = PropertyNames.getConstArray();

    css::uno::Sequence< css::beans::PropertyState > aStates( nNames );
    css::beans::PropertyState* pStates = aStates.getArray();

    for ( sal_uInt32 n = 0; n < nNames; n++ )
        pStates[n] = getPropertyState( pNames[n] );

    return aStates;
}

void UnoControlModel::setPropertyToDefault( const OUString& PropertyName )
{
    Any aDefaultValue;
    {
        ::osl::Guard< ::osl::Mutex > aGuard( GetMutex() );
        aDefaultValue = ImplGetDefaultValue( GetPropertyId( PropertyName ) );
    }
    setPropertyValue( PropertyName, aDefaultValue );
}

css::uno::Any UnoControlModel::getPropertyDefault( const OUString& rPropertyName )
{
    ::osl::Guard< ::osl::Mutex > aGuard( GetMutex() );

    return ImplGetDefaultValue( GetPropertyId( rPropertyName ) );
}


// css::io::XPersistObjec
OUString UnoControlModel::getServiceName(  )
{
    ::osl::Guard< ::osl::Mutex > aGuard( GetMutex() );

    OSL_FAIL( "ServiceName von UnoControlModel ?!" );
    return OUString();
}

void UnoControlModel::write( const css::uno::Reference< css::io::XObjectOutputStream >& OutStream )
{
    ::osl::Guard< ::osl::Mutex > aGuard( GetMutex() );

    css::uno::Reference< css::io::XMarkableStream > xMark( OutStream, css::uno::UNO_QUERY );
    DBG_ASSERT( xMark.is(), "write: no css::io::XMarkableStream!" );

    OutStream->writeShort( UNOCONTROL_STREAMVERSION );

    std::set<sal_uInt16> aProps;

    for (ImplPropertyTable::const_iterator it = maData.begin(); it != maData.end(); ++it )
    {
        if ( ( ( GetPropertyAttribs( it->first ) & css::beans::PropertyAttribute::TRANSIENT ) == 0 )
            && ( getPropertyState( GetPropertyName( it->first ) ) != css::beans::PropertyState_DEFAULT_VALUE ) )
        {
            aProps.insert( it->first );
        }
    }

    sal_uInt32 nProps = aProps.size();

    // Save FontProperty always in the old format (due to missing distinction
    // between 5.0 and 5.1)
    OutStream->writeLong( ( aProps.find( BASEPROPERTY_FONTDESCRIPTOR ) != aProps.end() ) ? ( nProps + 3 ) : nProps );
    for ( std::set<sal_uInt16>::const_iterator it = aProps.begin(); it != aProps.end(); ++it )
    {
        sal_Int32 nPropDataBeginMark = xMark->createMark();
        OutStream->writeLong( 0L ); // DataLen

        const css::uno::Any* pProp = &(maData[*it]);
        OutStream->writeShort( *it );

        bool bVoid = pProp->getValueType().getTypeClass() == css::uno::TypeClass_VOID;

        OutStream->writeBoolean( bVoid );

        if ( !bVoid )
        {
            const css::uno::Any& rValue = *pProp;
            const css::uno::Type& rType = rValue.getValueType();

            if ( rType == cppu::UnoType< bool >::get() )
            {
                bool b = false;
                rValue >>= b;
                OutStream->writeBoolean( b );
            }
            else if ( rType == ::cppu::UnoType< OUString >::get() )
            {
                OUString aUString;
                rValue >>= aUString;
                OutStream->writeUTF( aUString );
            }
            else if ( rType == ::cppu::UnoType< ::cppu::UnoUnsignedShortType >::get() )
            {
                sal_uInt16 n = 0;
                rValue >>= n;
                OutStream->writeShort( n );
            }
            else if ( rType == cppu::UnoType<sal_Int16>::get() )
            {
                sal_Int16 n = 0;
                rValue >>= n;
                OutStream->writeShort( n );
            }
            else if ( rType == cppu::UnoType<sal_uInt32>::get() )
            {
                sal_uInt32 n = 0;
                rValue >>= n;
                OutStream->writeLong( n );
            }
            else if ( rType == cppu::UnoType<sal_Int32>::get() )
            {
                sal_Int32 n = 0;
                rValue >>= n;
                OutStream->writeLong( n );
            }
            else if ( rType == cppu::UnoType<double>::get() )
            {
                double n = 0;
                rValue >>= n;
                OutStream->writeDouble( n );
            }
            else if ( rType == cppu::UnoType< css::awt::FontDescriptor >::get() )
            {
                css::awt::FontDescriptor aFD;
                rValue >>= aFD;
                OutStream->writeUTF( aFD.Name );
                OutStream->writeShort( aFD.Height );
                OutStream->writeShort( aFD.Width );
                OutStream->writeUTF( aFD.StyleName );
                OutStream->writeShort( aFD.Family );
                OutStream->writeShort( aFD.CharSet );
                OutStream->writeShort( aFD.Pitch );
                OutStream->writeDouble( aFD.CharacterWidth );
                OutStream->writeDouble( aFD.Weight );
                OutStream->writeShort(
                    sal::static_int_cast< sal_Int16 >(aFD.Slant) );
                OutStream->writeShort( aFD.Underline );
                OutStream->writeShort( aFD.Strikeout );
                OutStream->writeDouble( aFD.Orientation );
                OutStream->writeBoolean( aFD.Kerning );
                OutStream->writeBoolean( aFD.WordLineMode );
                OutStream->writeShort( aFD.Type );
            }
            else if ( rType == cppu::UnoType<css::util::Date>::get() )
            {
                css::util::Date d;
                rValue >>= d;
                OutStream->writeLong(d.Day + 100 * d.Month + 10000 * d.Year);
                    // YYYYMMDD
            }
            else if ( rType == cppu::UnoType<css::util::Time>::get() )
            {
                css::util::Time t;
                rValue >>= t;
                OutStream->writeLong(
                    t.NanoSeconds / 1000000 + 100 * t.Seconds
                    + 10000 * t.Minutes + 1000000 * t.Hours); // HHMMSShh
            }
            else if ( rType == cppu::UnoType< css::uno::Sequence< OUString> >::get() )
            {
                css::uno::Sequence< OUString> aSeq;
                rValue >>= aSeq;
                long nEntries = aSeq.getLength();
                OutStream->writeLong( nEntries );
                for ( long n = 0; n < nEntries; n++ )
                    OutStream->writeUTF( aSeq.getConstArray()[n] );
            }
            else if ( rType == cppu::UnoType< cppu::UnoSequenceType<cppu::UnoUnsignedShortType> >::get() )
            {
                css::uno::Sequence<sal_uInt16> aSeq;
                rValue >>= aSeq;
                long nEntries = aSeq.getLength();
                OutStream->writeLong( nEntries );
                for ( long n = 0; n < nEntries; n++ )
                    OutStream->writeShort( aSeq.getConstArray()[n] );
            }
            else if ( rType == cppu::UnoType< css::uno::Sequence<sal_Int16> >::get() )
            {
                css::uno::Sequence<sal_Int16> aSeq;
                rValue >>= aSeq;
                long nEntries = aSeq.getLength();
                OutStream->writeLong( nEntries );
                for ( long n = 0; n < nEntries; n++ )
                    OutStream->writeShort( aSeq.getConstArray()[n] );
            }
            else if ( rType.getTypeClass() == TypeClass_ENUM )
            {
                sal_Int32 nAsInt = 0;
                ::cppu::enum2int( nAsInt, rValue );
                OutStream->writeLong( nAsInt );
            }
#if OSL_DEBUG_LEVEL > 0
            else
            {
                OString sMessage( "UnoControlModel::write: don't know how to handle a property of type '" );
                OUString sTypeName( rType.getTypeName() );
                sMessage += OString( sTypeName.getStr(), sTypeName.getLength(), RTL_TEXTENCODING_ASCII_US );
                sMessage += "'.\n(Currently handling property '";
                const OUString& sPropertyName( GetPropertyName( *it ) );
                sMessage += OString( sPropertyName.getStr(), sPropertyName.getLength(), osl_getThreadTextEncoding() );
                sMessage += "'.)";
                OSL_FAIL( sMessage.getStr() );
            }
#endif
        }

        sal_Int32 nPropDataLen = xMark->offsetToMark( nPropDataBeginMark );
        xMark->jumpToMark( nPropDataBeginMark );
        OutStream->writeLong( nPropDataLen );
        xMark->jumpToFurthest();
        xMark->deleteMark(nPropDataBeginMark);
    }

    if ( aProps.find( BASEPROPERTY_FONTDESCRIPTOR ) != aProps.end() )
    {
        const css::uno::Any* pProp = &maData[ BASEPROPERTY_FONTDESCRIPTOR ];
        // Until 5.0 export arrives, write old format..
        css::awt::FontDescriptor aFD;
        (*pProp) >>= aFD;

        for ( sal_uInt16 n = BASEPROPERTY_FONT_TYPE; n <= BASEPROPERTY_FONT_ATTRIBS; n++ )
        {
            sal_Int32 nPropDataBeginMark = xMark->createMark();
            OutStream->writeLong( 0L ); // DataLen
            OutStream->writeShort( n ); // PropId
            OutStream->writeBoolean( false );   // Void

            if ( n == BASEPROPERTY_FONT_TYPE )
            {
                OutStream->writeUTF( aFD.Name );
                OutStream->writeUTF( aFD.StyleName );
                OutStream->writeShort( aFD.Family );
                OutStream->writeShort( aFD.CharSet );
                OutStream->writeShort( aFD.Pitch );
            }
            else if ( n == BASEPROPERTY_FONT_SIZE )
            {
                OutStream->writeLong( aFD.Width );
                OutStream->writeLong( aFD.Height );
                OutStream->writeShort(
                    sal::static_int_cast< sal_Int16 >(
                        VCLUnoHelper::ConvertFontWidth( aFD.CharacterWidth )) );
            }
            else if ( n == BASEPROPERTY_FONT_ATTRIBS )
            {
                OutStream->writeShort(
                    sal::static_int_cast< sal_Int16 >(
                        VCLUnoHelper::ConvertFontWeight( aFD.Weight )) );
                OutStream->writeShort(
                    sal::static_int_cast< sal_Int16 >(aFD.Slant) );
                OutStream->writeShort( aFD.Underline );
                OutStream->writeShort( aFD.Strikeout );
                OutStream->writeShort( (short)(aFD.Orientation * 10) );
                OutStream->writeBoolean( aFD.Kerning );
                OutStream->writeBoolean( aFD.WordLineMode );
            }
            else
            {
                OSL_FAIL( "Property?!" );
            }

            sal_Int32 nPropDataLen = xMark->offsetToMark( nPropDataBeginMark );
            xMark->jumpToMark( nPropDataBeginMark );
            OutStream->writeLong( nPropDataLen );
            xMark->jumpToFurthest();
            xMark->deleteMark(nPropDataBeginMark);
        }
    }
}

void UnoControlModel::read( const css::uno::Reference< css::io::XObjectInputStream >& InStream )
{
    ::osl::Guard< ::osl::Mutex > aGuard( GetMutex() );

    css::uno::Reference< css::io::XMarkableStream > xMark( InStream, css::uno::UNO_QUERY );
    DBG_ASSERT( xMark.is(), "read: no css::io::XMarkableStream!" );

    short nVersion = InStream->readShort();
    sal_uInt32 nProps = (sal_uInt32)InStream->readLong();
    css::uno::Sequence< OUString> aProps( nProps );
    css::uno::Sequence< css::uno::Any> aValues( nProps );
    bool bInvalidEntries = false;

    // Unfortunately, there's no mark for the whole block, thus only properties may be changed.
    // No data for the model may be added following the properties

    // Used for import of old parts in css::awt::FontDescriptor
    css::awt::FontDescriptor* pFD = nullptr;

    sal_uInt32 i;
    for ( i = 0; i < nProps; i++ )
    {
        sal_Int32 nPropDataBeginMark = xMark->createMark();
        sal_Int32 nPropDataLen = InStream->readLong();

        sal_uInt16 nPropId = (sal_uInt16)InStream->readShort();

        css::uno::Any aValue;
        bool bIsVoid = InStream->readBoolean();
        if ( !bIsVoid )
        {
            if ( maData.find( nPropId ) != maData.end() )
            {
                const css::uno::Type* pType = GetPropertyType( nPropId );
                if ( *pType == cppu::UnoType<bool>::get() )
                {
                    bool b = InStream->readBoolean();
                    aValue <<= b;
                }
                else if ( *pType == cppu::UnoType<OUString>::get() )
                {
                    OUString aUTF = InStream->readUTF();
                    aValue <<= aUTF;
                }
                else if ( *pType == ::cppu::UnoType< ::cppu::UnoUnsignedShortType >::get() )
                {
                    sal_uInt16 n = InStream->readShort();
                    aValue <<= n;
                }
                else if ( *pType == cppu::UnoType<sal_Int16>::get() )
                {
                    sal_Int16 n = InStream->readShort();
                    aValue <<= n;
                }
                else if ( *pType == cppu::UnoType<sal_uInt32>::get() )
                {
                    sal_uInt32 n = InStream->readLong();
                    aValue <<= n;
                }
                else if ( *pType == cppu::UnoType<sal_Int32>::get() )
                {
                    sal_Int32 n = InStream->readLong();
                    aValue <<= n;
                }
                else if ( *pType == cppu::UnoType<double>::get() )
                {
                    double n = InStream->readDouble();
                    aValue <<= n;
                }
                else if ( *pType == cppu::UnoType< css::awt::FontDescriptor >::get() )
                {
                    css::awt::FontDescriptor aFD;
                    aFD.Name = InStream->readUTF();
                    aFD.Height = InStream->readShort();
                    aFD.Width = InStream->readShort();
                    aFD.StyleName = InStream->readUTF();
                    aFD.Family = InStream->readShort();
                    aFD.CharSet = InStream->readShort();
                    aFD.Pitch = InStream->readShort();
                    aFD.CharacterWidth = (float)InStream->readDouble();
                    aFD.Weight = (float)InStream->readDouble();
                    aFD.Slant =  (css::awt::FontSlant)InStream->readShort();
                    aFD.Underline = InStream->readShort();
                    aFD.Strikeout = InStream->readShort();
                    aFD.Orientation = (float)InStream->readDouble();
                    aFD.Kerning = InStream->readBoolean() != 0;
                    aFD.WordLineMode = InStream->readBoolean() != 0;
                    aFD.Type = InStream->readShort();
                    aValue <<= aFD;
                }
                else if ( *pType == cppu::UnoType<css::util::Date>::get() )
                {
                    sal_Int32 n = InStream->readLong(); // YYYYMMDD
                    aValue <<= css::util::Date(
                        n % 100, (n / 100) % 100, n / 10000);
                }
                else if ( *pType == cppu::UnoType<css::util::Time>::get() )
                {
                    sal_Int32 n = InStream->readLong(); // HHMMSShh
                    aValue <<= css::util::Time(
                        (n % 100) * 1000000, (n / 100) % 100, (n / 10000) % 100,
                        n / 1000000, false);
                }
                else if ( *pType == cppu::UnoType< css::uno::Sequence< OUString> >::get() )
                {
                    long nEntries = InStream->readLong();
                    css::uno::Sequence< OUString> aSeq( nEntries );
                    for ( long n = 0; n < nEntries; n++ )
                        aSeq.getArray()[n] = InStream->readUTF();
                    aValue <<= aSeq;

                }
                else if ( *pType == cppu::UnoType< cppu::UnoSequenceType<cppu::UnoUnsignedShortType> >::get() )

                {
                    long nEntries = InStream->readLong();
                    css::uno::Sequence<sal_uInt16> aSeq( nEntries );
                    for ( long n = 0; n < nEntries; n++ )
                        aSeq.getArray()[n] = (sal_uInt16)InStream->readShort();
                    aValue <<= aSeq;
                }
                else if ( *pType == cppu::UnoType< css::uno::Sequence<sal_Int16> >::get() )
                {
                    long nEntries = InStream->readLong();
                    css::uno::Sequence<sal_Int16> aSeq( nEntries );
                    for ( long n = 0; n < nEntries; n++ )
                        aSeq.getArray()[n] = (sal_Int16)InStream->readShort();
                    aValue <<= aSeq;
                }
                else if ( pType->getTypeClass() == TypeClass_ENUM )
                {
                    sal_Int32 nAsInt = InStream->readLong();
                    aValue = ::cppu::int2enum( nAsInt, *pType );
                }
                else
                {
                    OString sMessage( "UnoControlModel::read: don't know how to handle a property of type '" );
                    OUString sTypeName( pType->getTypeName() );
                    sMessage += OString( sTypeName.getStr(), sTypeName.getLength(), RTL_TEXTENCODING_ASCII_US );
                    sMessage += "'.\n(Currently handling property '";
                    const OUString& sPropertyName( GetPropertyName( nPropId ) );
                    sMessage += OString( sPropertyName.getStr(), sPropertyName.getLength(), osl_getThreadTextEncoding() );
                    sMessage += "'.)";
                    OSL_FAIL( sMessage.getStr() );
                }
            }
            else
            {
                // Old trash from 5.0
                if ( nPropId == BASEPROPERTY_FONT_TYPE )
                {
                    // Redundant information for older versions
                    // is skipped by MarkableStream
                    if ( nVersion < 2 )
                    {
                        if ( !pFD )
                        {
                            pFD = new css::awt::FontDescriptor;
                            if ( maData.find( BASEPROPERTY_FONTDESCRIPTOR ) != maData.end() ) // wegen den Defaults...
                                maData[ BASEPROPERTY_FONTDESCRIPTOR ] >>= *pFD;
                        }
                        pFD->Name = InStream->readUTF();
                        pFD->StyleName = InStream->readUTF();
                        pFD->Family = InStream->readShort();
                        pFD->CharSet = InStream->readShort();
                        pFD->Pitch = InStream->readShort();
                    }
                }
                else if ( nPropId == BASEPROPERTY_FONT_SIZE )
                {
                    if ( nVersion < 2 )
                    {
                        if ( !pFD )
                        {
                            pFD = new css::awt::FontDescriptor;
                            if ( maData.find(BASEPROPERTY_FONTDESCRIPTOR) != maData.end() ) // due to defaults...
                                maData[BASEPROPERTY_FONTDESCRIPTOR] >>= *pFD;
                        }
                        pFD->Width = (sal_Int16)InStream->readLong();
                        pFD->Height = (sal_Int16)InStream->readLong();
                        InStream->readShort(); // ignore css::awt::FontWidth - it was
                                               // misspelled and is no longer needed
                        pFD->CharacterWidth = css::awt::FontWidth::DONTKNOW;
                    }
                }
                else if ( nPropId == BASEPROPERTY_FONT_ATTRIBS )
                {
                    if ( nVersion < 2 )
                    {
                         if ( !pFD )
                        {
                            pFD = new css::awt::FontDescriptor;
                            if ( maData.find(BASEPROPERTY_FONTDESCRIPTOR) != maData.end() ) // due to defaults...
                                maData[BASEPROPERTY_FONTDESCRIPTOR] >>= *pFD;
                        }
                        pFD->Weight = VCLUnoHelper::ConvertFontWeight( (FontWeight) InStream->readShort() );
                        pFD->Slant =  (css::awt::FontSlant)InStream->readShort();
                        pFD->Underline = InStream->readShort();
                        pFD->Strikeout = InStream->readShort();
                        pFD->Orientation = ( (float)(double)InStream->readShort() ) / 10;
                        pFD->Kerning = InStream->readBoolean() != 0;
                        pFD->WordLineMode = InStream->readBoolean() != 0;
                    }
                }
                else
                {
                    OSL_FAIL( "read: unknown Property!" );
                }
            }
        }
        else // bVoid
        {
            if ( nPropId == BASEPROPERTY_FONTDESCRIPTOR )
            {
                EmptyFontDescriptor aFD;
                aValue <<= aFD;
            }
        }

        if ( maData.find( nPropId ) != maData.end() )
        {
            aProps.getArray()[i] = GetPropertyName( nPropId );
            aValues.getArray()[i] = aValue;
        }
        else
        {
            bInvalidEntries = true;
        }

        // Skip rest of input if there is more data in stream than this version can handle
        xMark->jumpToMark( nPropDataBeginMark );
        InStream->skipBytes( nPropDataLen );
        xMark->deleteMark(nPropDataBeginMark);
    }
    if ( bInvalidEntries )
    {
        for ( i = 0; i < (sal_uInt32)aProps.getLength(); i++ )
        {
            if ( aProps.getConstArray()[i].isEmpty() )
            {
                ::comphelper::removeElementAt( aProps, i );
                ::comphelper::removeElementAt( aValues, i );
                i--;
            }
        }
    }

    try
    {
        setPropertyValues( aProps, aValues );
    }
    catch ( const Exception& )
    {
        DBG_UNHANDLED_EXCEPTION();
    }

    if ( pFD )
    {
        css::uno::Any aValue;
        aValue <<= *pFD;
        setPropertyValue( GetPropertyName( BASEPROPERTY_FONTDESCRIPTOR ), aValue );
        delete pFD;
    }
}


// css::lang::XServiceInfo
OUString UnoControlModel::getImplementationName(  )
{
    OSL_FAIL( "This method should be overridden!" );
    return OUString();

}

sal_Bool UnoControlModel::supportsService( const OUString& rServiceName )
{
    return cppu::supportsService(this, rServiceName);
}

css::uno::Sequence< OUString > UnoControlModel::getSupportedServiceNames(  )
{
    OUString sName( "com.sun.star.awt.UnoControlModel" );
    return Sequence< OUString >( &sName, 1 );
}

sal_Bool UnoControlModel::convertFastPropertyValue( Any & rConvertedValue, Any & rOldValue, sal_Int32 nPropId, const Any& rValue )
{
    ::osl::Guard< ::osl::Mutex > aGuard( GetMutex() );

    bool bVoid = rValue.getValueType().getTypeClass() == css::uno::TypeClass_VOID;
    if ( bVoid )
    {
        rConvertedValue.clear();
    }
    else
    {
        const css::uno::Type* pDestType = GetPropertyType( (sal_uInt16)nPropId );
        if ( pDestType->getTypeClass() == TypeClass_ANY )
        {
            rConvertedValue = rValue;
        }
        else
        {
            if ( pDestType->equals( rValue.getValueType() ) )
            {
                rConvertedValue = rValue;
            }
            else
            {
                bool bConverted = false;
                // 13.03.2001 - 84923 - frank.schoenheit@germany.sun.com

                switch (pDestType->getTypeClass())
                {
                    case TypeClass_DOUBLE:
                    {
                        // try as double
                        double nAsDouble = 0;
                        bConverted = ( rValue >>= nAsDouble );
                        if ( bConverted )
                            rConvertedValue <<= nAsDouble;
                        else
                        {   // try as integer
                            sal_Int32 nAsInteger = 0;
                            bConverted = ( rValue >>= nAsInteger );
                            if ( bConverted )
                                rConvertedValue <<= (double)nAsInteger;
                        }
                    }
                    break;
                    case TypeClass_SHORT:
                    {
                        sal_Int16 n;
                        bConverted = ( rValue >>= n );
                        if ( bConverted )
                            rConvertedValue <<= n;
                    }
                    break;
                    case TypeClass_UNSIGNED_SHORT:
                    {
                        sal_uInt16 n;
                        bConverted = ( rValue >>= n );
                        if ( bConverted )
                            rConvertedValue <<= n;
                    }
                    break;
                    case TypeClass_LONG:
                    {
                        sal_Int32 n;
                        bConverted = ( rValue >>= n );
                        if ( bConverted )
                            rConvertedValue <<= n;
                    }
                    break;
                    case TypeClass_UNSIGNED_LONG:
                    {
                        sal_uInt32 n;
                        bConverted = ( rValue >>= n );
                        if ( bConverted )
                            rConvertedValue <<= n;
                    }
                    break;
                    case TypeClass_INTERFACE:
                    {
                        if ( rValue.getValueType().getTypeClass() == TypeClass_INTERFACE )
                        {
                            Reference< XInterface > xPure( rValue, UNO_QUERY );
                            if ( xPure.is() )
                                rConvertedValue = xPure->queryInterface( *pDestType );
                            else
                                rConvertedValue.setValue( nullptr, *pDestType );
                            bConverted = true;
                        }
                    }
                    break;
                    case TypeClass_ENUM:
                    {
                        sal_Int32 nValue = 0;
                        bConverted = ( rValue >>= nValue );
                        if ( bConverted )
                            rConvertedValue = ::cppu::int2enum( nValue, *pDestType );
                    }
                    break;
                    default: ; // avoid compiler warning
                }

                if (!bConverted)
                {
                    throw css::lang::IllegalArgumentException(
                        "Unable to convert the given value for the property "
                        + GetPropertyName( (sal_uInt16)nPropId )
                        + ".\nExpected type: " + pDestType->getTypeName()
                        + "\nFound type: " + rValue.getValueType().getTypeName(),
                        static_cast< css::beans::XPropertySet* >(this),
                        1);
                }
            }
        }
    }

    // the current value
    getFastPropertyValue( rOldValue, nPropId );
    return !CompareProperties( rConvertedValue, rOldValue );
}

void UnoControlModel::setFastPropertyValue_NoBroadcast( sal_Int32 nPropId, const css::uno::Any& rValue )
{
    // Missing: the fake solo properties of the FontDescriptor

    ImplPropertyTable::const_iterator it = maData.find( nPropId );
    const css::uno::Any* pProp = it == maData.end() ? nullptr : &(it->second);
    ENSURE_OR_RETURN_VOID( pProp, "UnoControlModel::setFastPropertyValue_NoBroadcast: invalid property id!" );

    DBG_ASSERT( ( rValue.getValueType().getTypeClass() != css::uno::TypeClass_VOID ) || ( GetPropertyAttribs( (sal_uInt16)nPropId ) & css::beans::PropertyAttribute::MAYBEVOID ), "Property darf nicht VOID sein!" );
    maData[ nPropId ] = rValue;
}

void UnoControlModel::getFastPropertyValue( css::uno::Any& rValue, sal_Int32 nPropId ) const
{
    ::osl::Guard< ::osl::Mutex > aGuard( const_cast<UnoControlModel*>(this)->GetMutex() );

    ImplPropertyTable::const_iterator it = maData.find( nPropId );
    const css::uno::Any* pProp = it == maData.end() ? nullptr : &(it->second);

    if ( pProp )
        rValue = *pProp;
    else if ( ( nPropId >= BASEPROPERTY_FONTDESCRIPTORPART_START ) && ( nPropId <= BASEPROPERTY_FONTDESCRIPTORPART_END ) )
    {
        pProp = &( maData.find( BASEPROPERTY_FONTDESCRIPTOR )->second );
        css::awt::FontDescriptor aFD;
        (*pProp) >>= aFD;
        switch ( nPropId )
        {
            case BASEPROPERTY_FONTDESCRIPTORPART_NAME:          rValue <<= aFD.Name;
                                                                break;
            case BASEPROPERTY_FONTDESCRIPTORPART_STYLENAME:     rValue <<= aFD.StyleName;
                                                                break;
            case BASEPROPERTY_FONTDESCRIPTORPART_FAMILY:        rValue <<= aFD.Family;
                                                                break;
            case BASEPROPERTY_FONTDESCRIPTORPART_CHARSET:       rValue <<= aFD.CharSet;
                                                                break;
            case BASEPROPERTY_FONTDESCRIPTORPART_HEIGHT:        rValue <<= (float)aFD.Height;
                                                                break;
            case BASEPROPERTY_FONTDESCRIPTORPART_WEIGHT:        rValue <<= aFD.Weight;
                                                                break;
            case BASEPROPERTY_FONTDESCRIPTORPART_SLANT:         rValue <<= (sal_Int16)aFD.Slant;
                                                                break;
            case BASEPROPERTY_FONTDESCRIPTORPART_UNDERLINE:     rValue <<= aFD.Underline;
                                                                break;
            case BASEPROPERTY_FONTDESCRIPTORPART_STRIKEOUT:     rValue <<= aFD.Strikeout;
                                                                break;
            case BASEPROPERTY_FONTDESCRIPTORPART_WIDTH:         rValue <<= aFD.Width;
                                                                break;
            case BASEPROPERTY_FONTDESCRIPTORPART_PITCH:         rValue <<= aFD.Pitch;
                                                                break;
            case BASEPROPERTY_FONTDESCRIPTORPART_CHARWIDTH:     rValue <<= aFD.CharacterWidth;
                                                                break;
            case BASEPROPERTY_FONTDESCRIPTORPART_ORIENTATION:   rValue <<= aFD.Orientation;
                                                                break;
            case BASEPROPERTY_FONTDESCRIPTORPART_KERNING:       rValue <<= aFD.Kerning;
                                                                break;
            case BASEPROPERTY_FONTDESCRIPTORPART_WORDLINEMODE:  rValue <<= aFD.WordLineMode;
                                                                break;
            case BASEPROPERTY_FONTDESCRIPTORPART_TYPE:          rValue <<= aFD.Type;
                                                                break;
            default: OSL_FAIL( "FontProperty?!" );
        }
    }
    else
    {
        OSL_FAIL( "getFastPropertyValue - invalid Property!" );
    }
}

// css::beans::XPropertySet
void UnoControlModel::setPropertyValue( const OUString& rPropertyName, const css::uno::Any& rValue )
{
    sal_Int32 nPropId = 0;
    {
        ::osl::Guard< ::osl::Mutex > aGuard( GetMutex() );
        nPropId = (sal_Int32) GetPropertyId( rPropertyName );
        DBG_ASSERT( nPropId, "Invalid ID in UnoControlModel::setPropertyValue" );
    }
    if( nPropId )
        setFastPropertyValue( nPropId, rValue );
    else
        throw css::beans::UnknownPropertyException();
}

// css::beans::XFastPropertySet
void UnoControlModel::setFastPropertyValue( sal_Int32 nPropId, const css::uno::Any& rValue )
{
    if ( ( nPropId >= BASEPROPERTY_FONTDESCRIPTORPART_START ) && ( nPropId <= BASEPROPERTY_FONTDESCRIPTORPART_END ) )
    {
        ::osl::ClearableMutexGuard aGuard( GetMutex() );

        Any aOldSingleValue;
        getFastPropertyValue( aOldSingleValue, BASEPROPERTY_FONTDESCRIPTORPART_START );

        css::uno::Any* pProp = &maData[ BASEPROPERTY_FONTDESCRIPTOR ];
        FontDescriptor aOldFontDescriptor;
        (*pProp) >>= aOldFontDescriptor;

        FontDescriptor aNewFontDescriptor( aOldFontDescriptor );
        lcl_ImplMergeFontProperty( aNewFontDescriptor, (sal_uInt16)nPropId, rValue );

        Any aNewValue;
        aNewValue <<= aNewFontDescriptor;
        sal_Int32 nDescriptorId( BASEPROPERTY_FONTDESCRIPTOR );
        nDescriptorId = BASEPROPERTY_FONTDESCRIPTOR;

        // also, we need  fire a propertyChange event for the single property, since with
        // the above line, only an event for the FontDescriptor property will be fired
        Any aNewSingleValue;
        getFastPropertyValue( aNewSingleValue, BASEPROPERTY_FONTDESCRIPTORPART_START );

        aGuard.clear();
        setFastPropertyValues( 1, &nDescriptorId, &aNewValue, 1 );
        fire( &nPropId, &aNewSingleValue, &aOldSingleValue, 1, false );
       }
    else
        setFastPropertyValues( 1, &nPropId, &rValue, 1 );
}

// css::beans::XMultiPropertySet
css::uno::Reference< css::beans::XPropertySetInfo > UnoControlModel::getPropertySetInfo(  )
{
    OSL_FAIL( "UnoControlModel::getPropertySetInfo() not possible!" );
    return css::uno::Reference< css::beans::XPropertySetInfo >();
}

void UnoControlModel::setPropertyValues( const css::uno::Sequence< OUString >& rPropertyNames, const css::uno::Sequence< css::uno::Any >& Values )
{
    ::osl::ClearableMutexGuard aGuard( GetMutex() );

    sal_Int32 nProps = rPropertyNames.getLength();

//  sal_Int32* pHandles = new sal_Int32[nProps];
        // don't do this - it leaks in case of an exception
    Sequence< sal_Int32 > aHandles( nProps );
    sal_Int32* pHandles = aHandles.getArray();

    // may need to change the order in the sequence, for this we need a non-const value sequence
    uno::Sequence< uno::Any > aValues( Values );
    uno::Any* pValues = aValues.getArray();

    sal_Int32 nValidHandles = getInfoHelper().fillHandles( pHandles, rPropertyNames );

    if ( nValidHandles )
    {
        // if somebody sets properties which are single aspects of a font descriptor,
        // remove them, and build a font descriptor instead
        std::unique_ptr< awt::FontDescriptor > pFD;
        for ( sal_Int32 n = 0; n < nProps; ++n )
        {
            if ( ( pHandles[n] >= BASEPROPERTY_FONTDESCRIPTORPART_START ) && ( pHandles[n] <= BASEPROPERTY_FONTDESCRIPTORPART_END ) )
            {
                if ( !pFD.get() )
                {
                    css::uno::Any* pProp = &maData[ BASEPROPERTY_FONTDESCRIPTOR ];
                    pFD.reset( new awt::FontDescriptor );
                    (*pProp) >>= *pFD;
                }
                lcl_ImplMergeFontProperty( *pFD, (sal_uInt16)pHandles[n], pValues[n] );
                pHandles[n] = -1;
                nValidHandles--;
            }
        }

        if ( nValidHandles )
        {
            ImplNormalizePropertySequence( nProps, pHandles, pValues, &nValidHandles );
            aGuard.clear();
                // clear our guard before calling into setFastPropertyValues - this method
                // will implicitly call property listeners, and this should not happen with
                // our mutex locked
                // #i23451#
             setFastPropertyValues( nProps, pHandles, pValues, nValidHandles );
        }
        else
            aGuard.clear();
            // same as a few lines above

        // Don't merge FD property into array, as it is sorted
        if ( pFD.get() )
        {
            css::uno::Any aValue;
            aValue <<= *pFD;
            sal_Int32 nHandle = BASEPROPERTY_FONTDESCRIPTOR;
            setFastPropertyValues( 1, &nHandle, &aValue, 1 );
        }
    }
}


void UnoControlModel::ImplNormalizePropertySequence( const sal_Int32, sal_Int32*,
    uno::Any*, sal_Int32* ) const
{
    // nothing to do here
}

void UnoControlModel::ImplEnsureHandleOrder( const sal_Int32 _nCount, sal_Int32* _pHandles,
        uno::Any* _pValues, sal_Int32 _nFirstHandle, sal_Int32 _nSecondHandle )
{
    for ( sal_Int32 i=0; i < _nCount; ++_pHandles, ++_pValues, ++i )
    {
        if ( _nSecondHandle  == *_pHandles )
        {
            sal_Int32* pLaterHandles = _pHandles + 1;
            uno::Any* pLaterValues = _pValues + 1;
            for ( sal_Int32 j = i + 1; j < _nCount; ++j, ++pLaterHandles, ++pLaterValues )
            {
                if ( _nFirstHandle == *pLaterHandles )
                {
                    // indeed it is -> exchange the both places in the sequences
                    sal_Int32 nHandle( *_pHandles );
                    *_pHandles = *pLaterHandles;
                    *pLaterHandles = nHandle;

                    uno::Any aValue( *_pValues );
                    *_pValues = *pLaterValues;
                    *pLaterValues = aValue;

                    break;
                    // this will leave the inner loop, and continue with the outer loop.
                    // Note that this means we will encounter the _nSecondHandle handle, again, once we reached
                    // (in the outer loop) the place where we just put it.
                }
            }
        }
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
