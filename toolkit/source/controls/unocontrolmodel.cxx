/*************************************************************************
 *
 *  $RCSfile: unocontrolmodel.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: fs $ $Date: 2000-09-19 15:09:44 $
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

#ifndef _COM_SUN_STAR_BEANS_PROPERTYSTATE_HPP_
#include <com/sun/star/beans/PropertyState.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_PROPERTYATTRIBUTE_HPP_
#include <com/sun/star/beans/PropertyAttribute.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_FONTDESCRIPTOR_HPP_
#include <com/sun/star/awt/FontDescriptor.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_FONTWIDTH_HPP_
#include <com/sun/star/awt/FontWidth.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_FONTWEIGHT_HPP_
#include <com/sun/star/awt/FontWeight.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_FONTSLANT_HPP_
#include <com/sun/star/awt/FontSlant.hpp>
#endif


#ifndef _COM_SUN_STAR_IO_XMARKABLESTREAM_HPP_
#include <com/sun/star/io/XMarkableStream.hpp>
#endif

#include <toolkit/controls/unocontrolmodel.hxx>
#include <toolkit/helper/macros.hxx>
#include <cppuhelper/typeprovider.hxx>
#include <rtl/memory.h>
#include <rtl/uuid.h>

#include <tools/string.hxx>
#include <tools/table.hxx>
#include <tools/date.hxx>
#include <tools/time.hxx>
#include <tools/urlobj.hxx>
#include <tools/debug.hxx>

#include <toolkit/helper/property.hxx>
#include <toolkit/helper/vclunohelper.hxx>
#include <toolkit/helper/emptyfontdescriptor.hxx>

#include <unotools/sequence.hxx>

#include <vcl/svapp.hxx>
#include <tools/intn.hxx>

struct ImplPropertyInfo
{
private:
    sal_uInt16                  nId;
    ::com::sun::star::uno::Any  aValue;

public:
    ImplPropertyInfo( sal_uInt16 nT )
    {
        nId = nT;
    }

    ImplPropertyInfo( sal_uInt16 nT, const ::com::sun::star::uno::Any& rValue ) : aValue( rValue )
    {
        nId = nT;
    }

    sal_uInt16                          GetId() const                                           { return nId; }
    const ::com::sun::star::uno::Any&   GetValue() const                                        { return aValue; }
    void                                SetValue( const ::com::sun::star::uno::Any& rValue )    { aValue = rValue; }
};

DECLARE_TABLE( ImplPropertyTable, ImplPropertyInfo* );

#define UNOCONTROL_STREAMVERSION    (short)2

static void lcl_ImplMergeFontProperty( ::com::sun::star::awt::FontDescriptor& rFD, sal_uInt16 nPropId, const ::com::sun::star::uno::Any& rValue )
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
        case BASEPROPERTY_FONTDESCRIPTORPART_HEIGHT:        rValue >>= nExtractFloat; rFD.Height = nExtractFloat;
                                                            break;
        case BASEPROPERTY_FONTDESCRIPTORPART_WEIGHT:        rValue >>= rFD.Weight;
                                                            break;
        case BASEPROPERTY_FONTDESCRIPTORPART_SLANT:         rValue >>= nExtractShort; rFD.Slant = (::com::sun::star::awt::FontSlant)nExtractShort;
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
        default:                                            DBG_ERROR( "FontProperty?!" );
    }
}

//  ----------------------------------------------------
//  class UnoControlModel
//  ----------------------------------------------------
UnoControlModel::UnoControlModel()
    : OPropertySetHelper( BrdcstHelper ), maDisposeListeners( *this )
{
    // Die Properties muessen vom Model in die Tabelle gestopft werden,
    // nur vorhandene Properties sind gueltige Properties, auch wenn VOID.
    mpData = new ImplPropertyTable;
}

UnoControlModel::~UnoControlModel()
{
    for ( sal_uInt32 n = mpData->Count(); n--; )
        delete mpData->GetObject( n );
    delete mpData;
}

::com::sun::star::uno::Sequence<sal_Int32> UnoControlModel::ImplGetPropertyIds() const
{
    sal_uInt32 nIDs = mpData->Count();
    ::com::sun::star::uno::Sequence<sal_Int32>  aIDs( nIDs );
    sal_Int32* pIDs = aIDs.getArray();
    for ( sal_uInt32 n = 0; n < nIDs; n++ )
        pIDs[n] = mpData->GetObjectKey( n );
    return aIDs;
}

sal_Bool UnoControlModel::ImplHasProperty( sal_uInt16 nPropId ) const
{
    if ( ( nPropId >= BASEPROPERTY_FONTDESCRIPTORPART_START ) && ( nPropId <= BASEPROPERTY_FONTDESCRIPTORPART_END ) )
        nPropId = BASEPROPERTY_FONTDESCRIPTOR;

    return mpData->Get( nPropId ) ? sal_True : sal_False;
}

void UnoControlModel::ImplPropertyChanged( sal_uInt16 nPropId )
{
}

::com::sun::star::uno::Any UnoControlModel::ImplGetDefaultValue( sal_uInt16 nPropId ) const
{
    ::com::sun::star::uno::Any aDefault;

    if ( ( nPropId == BASEPROPERTY_FONTDESCRIPTOR ) ||
         ( nPropId >= BASEPROPERTY_FONTDESCRIPTORPART_START ) && ( nPropId <= BASEPROPERTY_FONTDESCRIPTORPART_END ) )
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
            default: DBG_ERROR( "FontProperty?!" );
        }
    }
    else
    {
        switch ( nPropId )
        {
            case BASEPROPERTY_TABSTOP:
            case BASEPROPERTY_TEXTCOLOR:
            case BASEPROPERTY_DATE:
            case BASEPROPERTY_DATESHOWCENTURY:
            case BASEPROPERTY_TIME:
            case BASEPROPERTY_VALUE_DOUBLE:
            case BASEPROPERTY_BACKGROUNDCOLOR:  break;

            case BASEPROPERTY_MAXTEXTLEN:
            case BASEPROPERTY_STATE:
            case BASEPROPERTY_EXTDATEFORMAT:
            case BASEPROPERTY_EXTTIMEFORMAT:
            case BASEPROPERTY_ECHOCHAR:         aDefault <<= (sal_Int16) 0; break;
            case BASEPROPERTY_BORDER:           aDefault <<= (sal_Int16) 1; break;
            case BASEPROPERTY_DECIMALACCURACY:  aDefault <<= (sal_Int16) 2; break;
            case BASEPROPERTY_LINECOUNT:        aDefault <<= (sal_Int16) 5; break;
            case BASEPROPERTY_ALIGN:            aDefault <<= (sal_Int16) PROPERTY_ALIGN_LEFT; break;

            case BASEPROPERTY_DATEMAX:          aDefault <<= (sal_Int32) Date( 31, 12, 2200 ).GetDate();    break;
            case BASEPROPERTY_DATEMIN:          aDefault <<= (sal_Int32) Date( 1, 1, 1900 ).GetDate();  break;
            case BASEPROPERTY_TIMEMAX:          aDefault <<= (sal_Int32) Time( 23, 59 ).GetTime();  break;
            case BASEPROPERTY_TIMEMIN:          aDefault <<= (sal_Int32) 0;     break;
            case BASEPROPERTY_VALUEMAX_DOUBLE:  aDefault <<= (double) 1000000;  break;
            case BASEPROPERTY_VALUEMIN_DOUBLE:  aDefault <<= (double) -1000000; break;
            case BASEPROPERTY_VALUESTEP_DOUBLE: aDefault <<= (double ) 1;       break;
            case BASEPROPERTY_DEFAULTCONTROL:   aDefault <<= ((UnoControlModel*)this)->getServiceName();    break;

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
            case BASEPROPERTY_HARDLINEBREAKS:   aDefault <<= (sal_Bool) sal_False; break;

            case BASEPROPERTY_AUTOCOMPLETE:
            case BASEPROPERTY_ENABLED:
            case BASEPROPERTY_PRINTABLE:        aDefault <<= (sal_Bool) sal_True;   break;

            case BASEPROPERTY_HELPURL:
            case BASEPROPERTY_IMAGEURL:
            case BASEPROPERTY_EDITMASK:
            case BASEPROPERTY_LITERALMASK:
            case BASEPROPERTY_LABEL:
            case BASEPROPERTY_TEXT:             aDefault <<= ::rtl::OUString(); break;

            case BASEPROPERTY_STRINGITEMLIST:
            {
                ::com::sun::star::uno::Sequence< ::rtl::OUString> aStringSeq;
                aDefault <<= aStringSeq;

            }
            break;
            case BASEPROPERTY_SELECTEDITEMS:
            {
                ::com::sun::star::uno::Sequence<sal_Int16> aINT16Seq;
                aDefault <<= aINT16Seq;
            }
            break;
            case BASEPROPERTY_CURRENCYSYMBOL:
            {
                XubString aSymbol = Application::GetAppInternational().GetCurrSymbol();
                aDefault <<= ::rtl::OUString( aSymbol );
            }
            break;
            default:    DBG_ERROR( "ImplGetDefaultValue - unknown Property" );
        }
    }

    return aDefault;
}

void UnoControlModel::ImplRegisterProperty( sal_uInt16 nPropId, const ::com::sun::star::uno::Any& rDefault )
{
    ImplPropertyInfo* pProp = new ImplPropertyInfo( nPropId, rDefault );
    mpData->Insert( nPropId, pProp );
}

void UnoControlModel::ImplRegisterProperty( sal_uInt16 nPropId )
{
    ImplRegisterProperty( nPropId, ImplGetDefaultValue( nPropId ) );
}

// ::com::sun::star::uno::XInterface
::com::sun::star::uno::Any UnoControlModel::queryAggregation( const ::com::sun::star::uno::Type & rType ) throw(::com::sun::star::uno::RuntimeException)
{
    ::com::sun::star::uno::Any aRet = ::cppu::queryInterface( rType,
                                        SAL_STATIC_CAST( ::com::sun::star::awt::XControlModel*, this ),
                                        SAL_STATIC_CAST( ::com::sun::star::io::XPersistObject*, this ),
                                        SAL_STATIC_CAST( ::com::sun::star::lang::XComponent*, this ),
                                        SAL_STATIC_CAST( ::com::sun::star::lang::XServiceInfo*, this ),
                                        SAL_STATIC_CAST( ::com::sun::star::beans::XPropertyState*, this ),
                                        SAL_STATIC_CAST( ::com::sun::star::beans::XMultiPropertySet*, this ),
                                        SAL_STATIC_CAST( ::com::sun::star::beans::XFastPropertySet*, this ),
                                        SAL_STATIC_CAST( ::com::sun::star::beans::XPropertySet*, this ),
                                        SAL_STATIC_CAST( ::com::sun::star::lang::XTypeProvider*, this ),
                                        SAL_STATIC_CAST( ::com::sun::star::lang::XUnoTunnel*, this ) );
    return (aRet.hasValue() ? aRet : OWeakAggObject::queryAggregation( rType ));
}

// ::com::sun::star::lang::XUnoTunnel
IMPL_XUNOTUNNEL( UnoControlModel )

// ::com::sun::star::lang::XTypeProvider
IMPL_XTYPEPROVIDER_START( UnoControlModel )
    getCppuType( ( ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControlModel>* ) NULL ),
    getCppuType( ( ::com::sun::star::uno::Reference< ::com::sun::star::io::XPersistObject>* ) NULL ),
    getCppuType( ( ::com::sun::star::uno::Reference< ::com::sun::star::lang::XComponent>* ) NULL ),
    getCppuType( ( ::com::sun::star::uno::Reference< ::com::sun::star::lang::XServiceInfo>* ) NULL ),
    getCppuType( ( ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertyState>* ) NULL ),
    getCppuType( ( ::com::sun::star::uno::Reference< ::com::sun::star::beans::XMultiPropertySet>* ) NULL ),
    getCppuType( ( ::com::sun::star::uno::Reference< ::com::sun::star::beans::XFastPropertySet>* ) NULL ),
    getCppuType( ( ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet>* ) NULL )
IMPL_XTYPEPROVIDER_END

// ::com::sun::star::lang::XComponent
void UnoControlModel::dispose(  ) throw(::com::sun::star::uno::RuntimeException)
{
    ::osl::Guard< ::osl::Mutex > aGuard( GetMutex() );

    ::com::sun::star::lang::EventObject aEvt;
    aEvt.Source = (::com::sun::star::uno::XAggregation*)(::cppu::OWeakAggObject*)this;
    maDisposeListeners.disposeAndClear( aEvt );
}

void UnoControlModel::addEventListener( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XEventListener >& rxListener ) throw(::com::sun::star::uno::RuntimeException)
{
    ::osl::Guard< ::osl::Mutex > aGuard( GetMutex() );

    maDisposeListeners.addInterface( rxListener );
}

void UnoControlModel::removeEventListener( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XEventListener >& rxListener ) throw(::com::sun::star::uno::RuntimeException)
{
    ::osl::Guard< ::osl::Mutex > aGuard( GetMutex() );

    maDisposeListeners.removeInterface( rxListener );
}


// ::com::sun::star::beans::XPropertyState
::com::sun::star::beans::PropertyState UnoControlModel::getPropertyState( const ::rtl::OUString& PropertyName ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::uno::RuntimeException)
{
    ::osl::Guard< ::osl::Mutex > aGuard( GetMutex() );

    sal_uInt16 nPropId = GetPropertyId( PropertyName );

    ::com::sun::star::uno::Any aValue = getPropertyValue( PropertyName );
    ::com::sun::star::uno::Any aDefault = ImplGetDefaultValue( nPropId );

    return CompareProperties( aValue, aDefault ) ? ::com::sun::star::beans::PropertyState_DEFAULT_VALUE : ::com::sun::star::beans::PropertyState_DIRECT_VALUE;
}

::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyState > UnoControlModel::getPropertyStates( const ::com::sun::star::uno::Sequence< ::rtl::OUString >& PropertyNames ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::uno::RuntimeException)
{
    ::osl::Guard< ::osl::Mutex > aGuard( GetMutex() );

    sal_uInt32 nNames = PropertyNames.getLength();
    const ::rtl::OUString* pNames = PropertyNames.getConstArray();

    ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyState > aStates( nNames );
    ::com::sun::star::beans::PropertyState* pStates = aStates.getArray();

    for ( sal_uInt32 n = 0; n < nNames; n++ )
        pStates[n] = getPropertyState( pNames[n] );

    return aStates;
}

void UnoControlModel::setPropertyToDefault( const ::rtl::OUString& PropertyName ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::uno::RuntimeException)
{
    ::osl::Guard< ::osl::Mutex > aGuard( GetMutex() );

    setPropertyValue( PropertyName, ImplGetDefaultValue( GetPropertyId( PropertyName ) ) );
}

::com::sun::star::uno::Any UnoControlModel::getPropertyDefault( const ::rtl::OUString& rPropertyName ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException)
{
    ::osl::Guard< ::osl::Mutex > aGuard( GetMutex() );

    return ImplGetDefaultValue( GetPropertyId( rPropertyName ) );
}


// ::com::sun::star::io::XPersistObjec
::rtl::OUString UnoControlModel::getServiceName(  ) throw(::com::sun::star::uno::RuntimeException)
{
    ::osl::Guard< ::osl::Mutex > aGuard( GetMutex() );

    DBG_ERROR( "ServiceName von UnoControlModel ?!" );
    return ::rtl::OUString();
}

void UnoControlModel::write( const ::com::sun::star::uno::Reference< ::com::sun::star::io::XObjectOutputStream >& OutStream ) throw(::com::sun::star::io::IOException, ::com::sun::star::uno::RuntimeException)
{
    ::osl::Guard< ::osl::Mutex > aGuard( GetMutex() );

    ::com::sun::star::uno::Reference< ::com::sun::star::io::XMarkableStream > xMark( OutStream, ::com::sun::star::uno::UNO_QUERY );
    DBG_ASSERT( xMark.is(), "write: no ::com::sun::star::io::XMarkableStream!" );

    OutStream->writeShort( UNOCONTROL_STREAMVERSION );

    ImplPropertyTable aProps;
    sal_uInt32 n;
    for ( n = mpData->Count(); n; )
    {
        ImplPropertyInfo* pProp = mpData->GetObject( --n );
        if ( ( ( GetPropertyAttribs( pProp->GetId() ) & ::com::sun::star::beans::PropertyAttribute::TRANSIENT ) == 0 )
            && ( getPropertyState( GetPropertyName( pProp->GetId() ) ) != ::com::sun::star::beans::PropertyState_DEFAULT_VALUE ) )
        {
            aProps.Insert( pProp->GetId(), pProp );
        }
    }

    sal_uInt32 nProps = aProps.Count();

    // FontProperty wegen fehlender Unterscheidung zwischen 5.0 / 5.1
    // immer im alten Format mitspeichern.
    OutStream->writeLong( (long) aProps.IsKeyValid( BASEPROPERTY_FONTDESCRIPTOR ) ? ( nProps + 3 ) : nProps );
    for ( n = 0; n < nProps; n++ )
    {
        sal_Int32 nPropDataBeginMark = xMark->createMark();
        OutStream->writeLong( 0L ); // DataLen

        ImplPropertyInfo* pProp = aProps.GetObject( n );
        OutStream->writeShort( pProp->GetId() );

        sal_Bool bVoid = pProp->GetValue().getValueType().getTypeClass() == ::com::sun::star::uno::TypeClass_VOID;

        OutStream->writeBoolean( bVoid );

        if ( !bVoid )
        {
            const ::com::sun::star::uno::Any& rValue = pProp->GetValue();
            const ::com::sun::star::uno::Type& rType = rValue.getValueType();

            if ( rType == ::getBooleanCppuType() )
            {
                sal_Bool b;
                rValue >>= b;
                OutStream->writeBoolean( b );
            }
            else if ( rType == ::getCppuType((const ::rtl::OUString*)0) )
            {
                // #HACK 52041# Pfade relativ speichern
                ::rtl::OUString aUString;
                rValue >>= aUString;
                if ( pProp->GetId() == BASEPROPERTY_IMAGEURL )
                    aUString= INetURLObject::AbsToRel( aUString );
                OutStream->writeUTF( aUString );
            }
            else if ( rType == ::getCppuType((const sal_uInt16*)0) )
            {
                sal_uInt16 n;
                rValue >>= n;
                OutStream->writeShort( n );
            }
            else if ( rType == ::getCppuType((const sal_Int16*)0) )
            {
                sal_Int16 n;
                rValue >>= n;
                OutStream->writeShort( n );
            }
            else if ( rType == ::getCppuType((const sal_uInt32*)0) )
            {
                sal_uInt32 n;
                rValue >>= n;
                OutStream->writeLong( n );
            }
            else if ( rType == ::getCppuType((const sal_Int32*)0) )
            {
                sal_Int32 n;
                rValue >>= n;
                OutStream->writeLong( n );
            }
            else if ( rType == ::getCppuType((const double*)0) )
            {
                double n;
                rValue >>= n;
                OutStream->writeDouble( n );
            }
            else if ( rType == ::getCppuType((const ::com::sun::star::awt::FontDescriptor*)0) )
            {
                ::com::sun::star::awt::FontDescriptor aFD;
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
                OutStream->writeShort( aFD.Slant );
                OutStream->writeShort( aFD.Underline );
                OutStream->writeShort( aFD.Strikeout );
                OutStream->writeDouble( aFD.Orientation );
                OutStream->writeBoolean( aFD.Kerning );
                OutStream->writeBoolean( aFD.WordLineMode );
                OutStream->writeShort( aFD.Type );
            }
            else if ( rType == ::getCppuType((const ::com::sun::star::uno::Sequence< ::rtl::OUString>*)0 ) )
            {
                ::com::sun::star::uno::Sequence< ::rtl::OUString> aSeq;
                rValue >>= aSeq;
                long nEntries = aSeq.getLength();
                OutStream->writeLong( nEntries );
                for ( long n = 0; n < nEntries; n++ )
                    OutStream->writeUTF( aSeq.getConstArray()[n] );
            }
            else if ( rType == ::getCppuType((const ::com::sun::star::uno::Sequence<sal_uInt16>*)0 ) )
            {
                ::com::sun::star::uno::Sequence<sal_uInt16> aSeq;
                rValue >>= aSeq;
                long nEntries = aSeq.getLength();
                OutStream->writeLong( nEntries );
                for ( long n = 0; n < nEntries; n++ )
                    OutStream->writeShort( aSeq.getConstArray()[n] );
            }
            else if ( rType == ::getCppuType((const ::com::sun::star::uno::Sequence<sal_Int16>*)0 ) )
            {
                ::com::sun::star::uno::Sequence<sal_Int16> aSeq;
                rValue >>= aSeq;
                long nEntries = aSeq.getLength();
                OutStream->writeLong( nEntries );
                for ( long n = 0; n < nEntries; n++ )
                    OutStream->writeShort( aSeq.getConstArray()[n] );
            }
            else
            {
                DBG_ERROR( "write: unknown Property!" );
            }
        }

        sal_Int32 nPropDataLen = xMark->offsetToMark( nPropDataBeginMark );
        xMark->jumpToMark( nPropDataBeginMark );
        OutStream->writeLong( nPropDataLen );
        xMark->jumpToFurthest();
        xMark->deleteMark(nPropDataBeginMark);
    }

    ImplPropertyInfo* pProp = aProps.Get( BASEPROPERTY_FONTDESCRIPTOR );
    if ( pProp )
    {
        // Solange wir keinen 5.0-Export haben, muss das alte
        // Format mit rausgeschrieben werden...
        ::com::sun::star::awt::FontDescriptor aFD;
        pProp->GetValue() >>= aFD;

        for ( sal_uInt16 n = BASEPROPERTY_FONT_TYPE; n <= BASEPROPERTY_FONT_ATTRIBS; n++ )
        {
            sal_Int32 nPropDataBeginMark = xMark->createMark();
            OutStream->writeLong( 0L ); // DataLen
            OutStream->writeShort( n ); // PropId
            OutStream->writeBoolean( sal_False );   // Void

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
                OutStream->writeShort( VCLUnoHelper::ConvertFontWidth( aFD.CharacterWidth ) );
            }
            else if ( n == BASEPROPERTY_FONT_ATTRIBS )
            {
                OutStream->writeShort( VCLUnoHelper::ConvertFontWeight( aFD.Weight ) );
                OutStream->writeShort( aFD.Slant );
                OutStream->writeShort( aFD.Underline );
                OutStream->writeShort( aFD.Strikeout );
                OutStream->writeShort( aFD.Orientation * 10 );
                OutStream->writeBoolean( aFD.Kerning );
                OutStream->writeBoolean( aFD.WordLineMode );
            }
            else
            {
                DBG_ERROR( "Property?!" );
            }

            sal_Int32 nPropDataLen = xMark->offsetToMark( nPropDataBeginMark );
            xMark->jumpToMark( nPropDataBeginMark );
            OutStream->writeLong( nPropDataLen );
            xMark->jumpToFurthest();
            xMark->deleteMark(nPropDataBeginMark);
        }
    }
}

void UnoControlModel::read( const ::com::sun::star::uno::Reference< ::com::sun::star::io::XObjectInputStream >& InStream ) throw(::com::sun::star::io::IOException, ::com::sun::star::uno::RuntimeException)
{
    ::osl::Guard< ::osl::Mutex > aGuard( GetMutex() );

    ::com::sun::star::uno::Reference< ::com::sun::star::io::XMarkableStream > xMark( InStream, ::com::sun::star::uno::UNO_QUERY );
    DBG_ASSERT( xMark.is(), "read: no ::com::sun::star::io::XMarkableStream!" );

    short nVersion = InStream->readShort();
    sal_uInt32 nProps = (sal_uInt32)InStream->readLong();
    ::com::sun::star::uno::Sequence< ::rtl::OUString> aProps( nProps );
    ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any> aValues( nProps );
    sal_Bool bInvalidEntries = sal_False;

    // Dummerweise kein Mark fuer den gesamten Block, es koennen also
    // nur Properties geaendert werden, es koennen aber nicht spaeter mal Daten
    // fuer das Model hinter den Properties geschrieben werden.

    // Fuer den Import der alten ::com::sun::star::awt::FontDescriptor-Teile
    ::com::sun::star::awt::FontDescriptor* pFD = NULL;

    sal_uInt32 n;
    for ( n = 0; n < nProps; n++ )
    {
        sal_Int32 nPropDataBeginMark = xMark->createMark();
        sal_Int32 nPropDataLen = InStream->readLong();

        sal_uInt16 nPropId = (sal_uInt16)InStream->readShort();

        ::com::sun::star::uno::Any aValue;
        sal_Bool bIsVoid = InStream->readBoolean();
        if ( !bIsVoid )
        {
            const ::com::sun::star::uno::Type* pType = mpData->Get( nPropId ) ? GetPropertyType( nPropId ) : NULL;
            if ( pType )
            {
                if ( *pType == ::getBooleanCppuType() )
                {
                    sal_Bool b = InStream->readBoolean();
                    aValue <<= b;
                }
                else if ( *pType == ::getCppuType((const ::rtl::OUString*)0) )
                {
                    ::rtl::OUString aUTF = InStream->readUTF();
                    // #HACK 52041# Pfade relativ speichern
                    if ( nPropId == BASEPROPERTY_IMAGEURL )
                        aUTF = INetURLObject::RelToAbs( aUTF );
                    aValue <<= aUTF;
                }
                else if ( *pType == ::getCppuType((const sal_uInt16*)0) )
                {
                    sal_uInt16 n = InStream->readShort();
                    aValue <<= n;
                }
                else if ( *pType == ::getCppuType((const sal_Int16*)0) )
                {
                    sal_Int16 n = InStream->readShort();
                    aValue <<= n;
                }
                else if ( *pType == ::getCppuType((const sal_uInt32*)0) )
                {
                    sal_uInt32 n = InStream->readLong();
                    aValue <<= n;
                }
                else if ( *pType == ::getCppuType((const sal_Int32*)0) )
                {
                    sal_Int32 n = InStream->readLong();
                    aValue <<= n;
                }
                else if ( *pType == ::getCppuType((const Double*)0) )
                {
                    double n = InStream->readDouble();
                    aValue <<= n;
                }
                else if ( *pType == ::getCppuType((const ::com::sun::star::awt::FontDescriptor*)0) )
                {
                    ::com::sun::star::awt::FontDescriptor aFD;
                    aFD.Name = InStream->readUTF();
                    aFD.Height = InStream->readShort();
                    aFD.Width = InStream->readShort();
                    aFD.StyleName = InStream->readUTF();
                    aFD.Family = InStream->readShort();
                    aFD.CharSet = InStream->readShort();
                    aFD.Pitch = InStream->readShort();
                    aFD.CharacterWidth = InStream->readDouble();
                    aFD.Weight = InStream->readDouble();
                    aFD.Slant =  (::com::sun::star::awt::FontSlant)InStream->readShort();
                    aFD.Underline = InStream->readShort();
                    aFD.Strikeout = InStream->readShort();
                    aFD.Orientation = InStream->readDouble();
                    aFD.Kerning = InStream->readBoolean();
                    aFD.WordLineMode = InStream->readBoolean();
                    aFD.Type = InStream->readShort();
                    aValue <<= aFD;
                }
                else if ( *pType == ::getCppuType((const ::com::sun::star::uno::Sequence< ::rtl::OUString>*)0 ) )
                {
                    long nEntries = InStream->readLong();
                    ::com::sun::star::uno::Sequence< ::rtl::OUString> aSeq( nEntries );
                    for ( long n = 0; n < nEntries; n++ )
                        aSeq.getArray()[n] = InStream->readUTF();
                    aValue <<= aSeq;

                }
                else if ( *pType == ::getCppuType((const ::com::sun::star::uno::Sequence<sal_uInt16>*)0 ) )

                {
                    long nEntries = InStream->readLong();
                    ::com::sun::star::uno::Sequence<sal_uInt16> aSeq( nEntries );
                    for ( long n = 0; n < nEntries; n++ )
                        aSeq.getArray()[n] = (sal_uInt16)InStream->readShort();
                    aValue <<= aSeq;
                }
                else if ( *pType == ::getCppuType((const ::com::sun::star::uno::Sequence<sal_Int16>*)0 ) )
                {
                    long nEntries = InStream->readLong();
                    ::com::sun::star::uno::Sequence<sal_Int16> aSeq( nEntries );
                    for ( long n = 0; n < nEntries; n++ )
                        aSeq.getArray()[n] = (sal_Int16)InStream->readShort();
                    aValue <<= aSeq;
                }
            }
            else
            {
                // Altes Geraffel aus 5.0
                if ( nPropId == BASEPROPERTY_FONT_TYPE )
                {
                    // Sonst ist es nur die redundante Info fuer alte Versionen
                    // Daten werden durch MarkableStream geskippt.
                    if ( nVersion < 2 )
                    {
                        if ( !pFD )
                        {
                            pFD = new ::com::sun::star::awt::FontDescriptor;
                            ImplPropertyInfo* pProp = mpData->Get( BASEPROPERTY_FONTDESCRIPTOR );
                            if ( pProp ) // wegen den Defaults...
                            {
                                pFD = new ::com::sun::star::awt::FontDescriptor;
                                pProp->GetValue() >>= *pFD;
                            }
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
                            pFD = new ::com::sun::star::awt::FontDescriptor;
                            ImplPropertyInfo* pProp = mpData->Get( BASEPROPERTY_FONTDESCRIPTOR );
                            if ( pProp ) // wegen den Defaults...
                                pProp->GetValue() >>= *pFD;
                        }
                        pFD->Width = InStream->readLong();
                        pFD->Height = InStream->readLong();
                        InStream->readShort();  // ::com::sun::star::awt::FontWidth ignorieren - wurde mal falsch geschrieben und wird nicht gebraucht.
                        pFD->CharacterWidth = ::com::sun::star::awt::FontWidth::DONTKNOW;
                    }
                }
                else if ( nPropId == BASEPROPERTY_FONT_ATTRIBS )
                {
                    if ( nVersion < 2 )
                    {
                         if ( !pFD )
                        {
                            pFD = new ::com::sun::star::awt::FontDescriptor;
                            ImplPropertyInfo* pProp = mpData->Get( BASEPROPERTY_FONTDESCRIPTOR );
                            if ( pProp ) // wegen den Defaults...
                                pProp->GetValue() >>= *pFD;
                        }
                        pFD->Weight = VCLUnoHelper::ConvertFontWeight( (FontWeight) InStream->readShort() );
                        pFD->Slant =  (::com::sun::star::awt::FontSlant)InStream->readShort();
                        pFD->Underline = InStream->readShort();
                        pFD->Strikeout = InStream->readShort();
                        pFD->Orientation = ( (double)InStream->readShort() ) / 10;
                        pFD->Kerning = InStream->readBoolean();
                        pFD->WordLineMode = InStream->readBoolean();
                    }
                }
                else
                {
                    DBG_ERROR( "read: unknown Property!" );
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

        if ( mpData->Get( nPropId ) )
        {
            aProps.getArray()[n] = GetPropertyName( nPropId );
            aValues.getArray()[n] = aValue;
        }
        else
        {
            bInvalidEntries = sal_True;
        }

        // Falls bereits mehr drinsteht als diese Version kennt:
        xMark->jumpToMark( nPropDataBeginMark );
        InStream->skipBytes( nPropDataLen );
        xMark->deleteMark(nPropDataBeginMark);
    }
    if ( bInvalidEntries )
    {
        for ( n = 0; n < aProps.getLength(); n++ )
        {
            if ( !aProps.getConstArray()[n].len() )
            {
                ::utl::removeElementAt( aProps, n );
                ::utl::removeElementAt( aValues, n );
                n--;
            }
        }
    }

    setPropertyValues( aProps, aValues );

    if ( pFD )
    {
        ::com::sun::star::uno::Any aValue;
        aValue <<= *pFD;
        setPropertyValue( GetPropertyName( BASEPROPERTY_FONTDESCRIPTOR ), aValue );
        delete pFD;
    }
}


// ::com::sun::star::lang::XServiceInfo
::rtl::OUString UnoControlModel::getImplementationName(  ) throw(::com::sun::star::uno::RuntimeException)
{
    DBG_ERROR( "This method should be overloaded!" );
    return ::rtl::OUString();

}

sal_Bool UnoControlModel::supportsService( const ::rtl::OUString& rServiceName ) throw(::com::sun::star::uno::RuntimeException)
{
    ::osl::Guard< ::osl::Mutex > aGuard( GetMutex() );

    ::com::sun::star::uno::Sequence< ::rtl::OUString > aSNL = getSupportedServiceNames();
    const ::rtl::OUString * pArray = aSNL.getConstArray();
    for( sal_Int32 i = 0; i < aSNL.getLength(); i++ )
        if( pArray[i] == rServiceName )
            return sal_True;
    return sal_False;
}

::com::sun::star::uno::Sequence< ::rtl::OUString > UnoControlModel::getSupportedServiceNames(  ) throw(::com::sun::star::uno::RuntimeException)
{
    ::osl::Guard< ::osl::Mutex > aGuard( GetMutex() );

    DBG_ERROR( "This method should be overloaded!" );
    return ::com::sun::star::uno::Sequence< ::rtl::OUString >();
}

// ::cppu::OPropertySetHelper
::cppu::IPropertyArrayHelper& UnoControlModel::getInfoHelper()
{
    DBG_ERROR( "UnoControlModel::getInfoHelper() not possible!" );
    return *(::cppu::IPropertyArrayHelper*) NULL;
}

sal_Bool UnoControlModel::convertFastPropertyValue( ::com::sun::star::uno::Any & rConvertedValue, ::com::sun::star::uno::Any & rOldValue, sal_Int32 nPropId, const ::com::sun::star::uno::Any& rValue ) throw (::com::sun::star::lang::IllegalArgumentException)
{
    ::osl::Guard< ::osl::Mutex > aGuard( GetMutex() );

    sal_Bool bVoid = rValue.getValueType().getTypeClass() == ::com::sun::star::uno::TypeClass_VOID;
    if ( bVoid )
    {
        rConvertedValue.clear();
    }
    else
    {
        const ::com::sun::star::uno::Type* pDestType = GetPropertyType( nPropId );
        if ( pDestType->getTypeClass() == ::com::sun::star::uno::TypeClass_ANY )
        {
            rConvertedValue = rValue;
        }
        else
        {
            rConvertedValue.setValue( rValue.getValue(), *pDestType );
        }
    }
    getFastPropertyValue( rOldValue, nPropId );
    return !CompareProperties( rConvertedValue, rOldValue );
}

void UnoControlModel::setFastPropertyValue_NoBroadcast( sal_Int32 nPropId, const ::com::sun::star::uno::Any& rValue ) throw (::com::sun::star::uno::Exception)
{
    ::osl::Guard< ::osl::Mutex > aGuard( GetMutex() );

    // Fehlt: Die gefakten Einzelproperties des FontDescriptors...

    ImplPropertyInfo* pProp = mpData->Get( nPropId );
    if ( pProp )
    {
        sal_Bool bVoid = rValue.getValueType().getTypeClass() == ::com::sun::star::uno::TypeClass_VOID;

        DBG_ASSERT( !bVoid || ( GetPropertyAttribs( nPropId ) & ::com::sun::star::beans::PropertyAttribute::MAYBEVOID ), "Property darf nicht VOID sein!" );
        ImplPropertyChanged( nPropId );
        pProp->SetValue( rValue );
    }
    else
    {
        // exception...
        DBG_ERROR( "SetPropertyValues: Invalid Property!" );
    }
}

void UnoControlModel::getFastPropertyValue( ::com::sun::star::uno::Any& rValue, sal_Int32 nPropId ) const
{
    ::osl::Guard< ::osl::Mutex > aGuard( ((UnoControlModel*)this)->GetMutex() );

    ImplPropertyInfo* pProp = mpData->Get( nPropId );

    if ( pProp )
        rValue = pProp->GetValue();
    else if ( ( nPropId >= BASEPROPERTY_FONTDESCRIPTORPART_START ) && ( nPropId <= BASEPROPERTY_FONTDESCRIPTORPART_END ) )
    {
        pProp = mpData->Get( BASEPROPERTY_FONTDESCRIPTOR );
        ::com::sun::star::awt::FontDescriptor aFD;
        pProp->GetValue() >>= aFD;
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
            default: DBG_ERROR( "FontProperty?!" );
        }
    }
    else
    {
        DBG_ERROR( "getFastPropertyValue - invalid Property!" );
    }
}

// ::com::sun::star::beans::XPropertySet
void UnoControlModel::setPropertyValue( const ::rtl::OUString& rPropertyName, const ::com::sun::star::uno::Any& rValue ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::beans::PropertyVetoException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException)
{
    ::osl::Guard< ::osl::Mutex > aGuard( GetMutex() );

    sal_Int32 nPropId = (sal_Int32) GetPropertyId( rPropertyName );
    DBG_ASSERT( nPropId, "Invalid ID in UnoControlModel::setPropertyValue" );
    setFastPropertyValue( nPropId, rValue );
}

// ::com::sun::star::beans::XFastPropertySet
void UnoControlModel::setFastPropertyValue( sal_Int32 nPropId, const ::com::sun::star::uno::Any& rValue ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::beans::PropertyVetoException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException)
{
    ::osl::Guard< ::osl::Mutex > aGuard( GetMutex() );

    if ( ( nPropId >= BASEPROPERTY_FONTDESCRIPTORPART_START ) && ( nPropId <= BASEPROPERTY_FONTDESCRIPTORPART_END ) )
    {
        ImplPropertyInfo* pProp = mpData->Get( BASEPROPERTY_FONTDESCRIPTOR );
        ::com::sun::star::awt::FontDescriptor aFD;
        pProp->GetValue() >>= aFD;
        lcl_ImplMergeFontProperty( aFD, nPropId, rValue );

        ::com::sun::star::uno::Any aFDValue;
        aFDValue <<= aFD;
        nPropId = BASEPROPERTY_FONTDESCRIPTOR;
        setFastPropertyValues( 1, &nPropId, &aFDValue, 1 );
       }
    else
        setFastPropertyValues( 1, &nPropId, &rValue, 1 );
}

// ::com::sun::star::beans::XMultiPropertySet
::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo > UnoControlModel::getPropertySetInfo(  ) throw(::com::sun::star::uno::RuntimeException)
{
    DBG_ERROR( "UnoControlModel::getPropertySetInfo() not possible!" );
    return ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo >();
}

void UnoControlModel::setPropertyValues( const ::com::sun::star::uno::Sequence< ::rtl::OUString >& rPropertyNames, const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any >& Values ) throw(::com::sun::star::beans::PropertyVetoException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException)
{
    ::osl::Guard< ::osl::Mutex > aGuard( GetMutex() );

    sal_uInt16 nProps = rPropertyNames.getLength();
    sal_Int32* pHandles = new sal_Int32[nProps];
    const ::com::sun::star::uno::Any* pValues = Values.getConstArray();
    sal_uInt16 nValidHandles = getInfoHelper().fillHandles( pHandles, rPropertyNames );

    if ( nValidHandles )
    {
        // Einzelproperties vom ::com::sun::star::awt::FontDescriptor rausfischen...
     ::com::sun::star::awt::FontDescriptor* pFD = NULL;
        for ( sal_uInt16 n = 0; n < nProps; n++ )
        {
            if ( ( pHandles[n] >= BASEPROPERTY_FONTDESCRIPTORPART_START ) && ( pHandles[n] <= BASEPROPERTY_FONTDESCRIPTORPART_END ) )
            {
                if ( !pFD )
                {
                    ImplPropertyInfo* pProp = mpData->Get( BASEPROPERTY_FONTDESCRIPTOR );
                    pFD = new ::com::sun::star::awt::FontDescriptor;
                    pProp->GetValue() >>= *pFD;
                }
                lcl_ImplMergeFontProperty( *pFD, pHandles[n], pValues[n] );
                pHandles[n] = -1;
                nValidHandles--;
            }
        }

        if ( nValidHandles )
            setFastPropertyValues( nProps, pHandles, pValues, nValidHandles );

        // FD-Propertie nicht in das Array mergen, weil sortiert...
        if ( pFD )
        {
            ::com::sun::star::uno::Any aValue;
            aValue <<= *pFD;
            sal_Int32 nHandle = BASEPROPERTY_FONTDESCRIPTOR;
            setFastPropertyValues( 1, &nHandle, &aValue, 1 );
            delete pFD;
        }
    }

    delete pHandles;
}



