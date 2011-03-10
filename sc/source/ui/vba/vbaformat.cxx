/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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
#include "vbaformat.hxx"
#include <ooo/vba/excel/XStyle.hpp>
#include <ooo/vba/excel/XlVAlign.hpp>
#include <ooo/vba/excel/XlHAlign.hpp>
#include <ooo/vba/excel/XlOrientation.hpp>
#include <ooo/vba/excel/Constants.hpp>
#include <ooo/vba/excel/XRange.hpp>
#include <com/sun/star/table/CellVertJustify2.hpp>
#include <com/sun/star/table/CellHoriJustify.hpp>
#include <com/sun/star/table/CellOrientation.hpp>
#include <com/sun/star/table/XCellRange.hpp>
#include <com/sun/star/text/WritingMode.hpp>
#include <com/sun/star/util/CellProtection.hpp>

#include <rtl/math.hxx>

#include "excelvbahelper.hxx"
#include "vbaborders.hxx"
#include "vbapalette.hxx"
#include "vbafont.hxx"
#include "vbainterior.hxx"

#include <unonames.hxx>
#include <cellsuno.hxx>
#include <scitems.hxx>
#include <attrib.hxx>

using namespace ::ooo::vba;
using namespace ::com::sun::star;

#define FORMATSTRING "FormatString"
#define LOCALE "Locale"

template< typename Ifc1 >
ScVbaFormat< Ifc1 >::ScVbaFormat( const uno::Reference< XHelperInterface >& xParent, const uno::Reference< uno::XComponentContext > & xContext, const uno::Reference< beans::XPropertySet >& _xPropertySet, const uno::Reference< frame::XModel >& xModel, bool bCheckAmbiguoity ) throw ( script::BasicErrorException ) : ScVbaFormat_BASE( xParent, xContext ), m_aDefaultLocale( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("en") ), rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "US") ), rtl::OUString() ), mxPropertySet( _xPropertySet ), mxModel( xModel ), mbCheckAmbiguoity( bCheckAmbiguoity ), mbAddIndent( false )
{
    try
    {
        mxServiceInfo.set( mxPropertySet, uno::UNO_QUERY_THROW );
        if ( !mxModel.is() )
            DebugHelper::exception(SbERR_METHOD_FAILED, rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "XModel Interface could not be retrieved") ) );
        mxNumberFormatsSupplier.set( mxModel, uno::UNO_QUERY_THROW );
    }
    catch (uno::Exception& )
    {
        DebugHelper::exception(SbERR_METHOD_FAILED, rtl::OUString() );
    }
}

template< typename Ifc1 >
void SAL_CALL
ScVbaFormat<Ifc1>::setVerticalAlignment( const uno::Any& _oAlignment)   throw (script::BasicErrorException, uno::RuntimeException)
{
    try
    {
        uno::Any aVal;
        sal_Int32 nAlignment = 0;
        if ( !(_oAlignment >>= nAlignment ))
            throw uno::RuntimeException();
        switch (nAlignment)
        {
            case excel::XlVAlign::xlVAlignBottom :
                aVal =  uno::makeAny( table::CellVertJustify2::BOTTOM );
                break;
            case excel::XlVAlign::xlVAlignCenter :
                aVal = uno::makeAny( table::CellVertJustify2::CENTER );
                break;
            case excel::XlVAlign::xlVAlignDistributed:
            case excel::XlVAlign::xlVAlignJustify:
                aVal = uno::makeAny( table::CellVertJustify2::STANDARD );
                break;

            case excel::XlVAlign::xlVAlignTop:
                aVal = uno::makeAny( table::CellVertJustify2::TOP);
                break;
            default:
                aVal = uno::makeAny( table::CellVertJustify2::STANDARD );
                break;
        }
        mxPropertySet->setPropertyValue( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( SC_UNONAME_CELLVJUS ) ), aVal );
    }
    catch (uno::Exception& )
    {
        DebugHelper::exception(SbERR_METHOD_FAILED, rtl::OUString());
    }
}

template< typename Ifc1 >
uno::Any SAL_CALL
ScVbaFormat<Ifc1>::getVerticalAlignment(  ) throw (script::BasicErrorException, uno::RuntimeException)
{
    uno::Any aResult = aNULL();
    try
    {
        if (!isAmbiguous( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( SC_UNONAME_CELLVJUS ) ) ) )
        {
            sal_Int32 aAPIAlignment = table::CellVertJustify2::STANDARD;
            mxPropertySet->getPropertyValue( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( SC_UNONAME_CELLVJUS ) ) ) >>= aAPIAlignment;
            switch( aAPIAlignment )
            {
                case table::CellVertJustify2::BOTTOM:
                    aResult = uno::makeAny( excel::XlVAlign::xlVAlignBottom );
                    break;
                case table::CellVertJustify2::CENTER:
                    aResult = uno::makeAny( excel::XlVAlign::xlVAlignCenter );
                    break;
                case table::CellVertJustify2::STANDARD:
                    aResult = uno::makeAny( excel::XlVAlign::xlVAlignBottom );
                    break;
                case table::CellVertJustify2::TOP:
                    aResult = uno::makeAny( excel::XlVAlign::xlVAlignTop );
                    break;
                default:
                    break;
            }
        }
    }
    catch (uno::Exception& )
    {
        DebugHelper::exception(SbERR_METHOD_FAILED, rtl::OUString());
    }
    return aResult;
}

template< typename Ifc1 >
void SAL_CALL
ScVbaFormat<Ifc1>::setHorizontalAlignment( const uno::Any& HorizontalAlignment ) throw (script::BasicErrorException, uno::RuntimeException)
{
    try
    {
        uno::Any aVal;
        sal_Int32 nAlignment = 0;
        if ( !( HorizontalAlignment >>= nAlignment ) )
            throw uno::RuntimeException();
        switch ( nAlignment )
        {
            case excel::XlHAlign::xlHAlignJustify:
                aVal = uno::makeAny( table::CellHoriJustify_BLOCK);
                break;
            case excel::XlHAlign::xlHAlignCenter:
                aVal = uno::makeAny( table::CellHoriJustify_CENTER );
                break;
            case excel::XlHAlign::xlHAlignDistributed:
                aVal = uno::makeAny( table::CellHoriJustify_BLOCK);
                break;
            case excel::XlHAlign::xlHAlignLeft:
                aVal = uno::makeAny( table::CellHoriJustify_LEFT);
                break;
            case excel::XlHAlign::xlHAlignRight:
                aVal = uno::makeAny( table::CellHoriJustify_RIGHT);
                break;
        }
        // #FIXME what about the default case above?
        // shouldn't need the test below
        if ( aVal.hasValue() )
            mxPropertySet->setPropertyValue( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( SC_UNONAME_CELLHJUS ) ), aVal );
    }
    catch (uno::Exception& )
    {
        DebugHelper::exception(SbERR_METHOD_FAILED, rtl::OUString() );
    }

}

template< typename Ifc1 >
uno::Any SAL_CALL
ScVbaFormat<Ifc1>::getHorizontalAlignment(  ) throw (script::BasicErrorException, uno::RuntimeException)
{
    uno::Any NRetAlignment = aNULL();
    try
    {
        rtl::OUString sHoriJust( RTL_CONSTASCII_USTRINGPARAM( SC_UNONAME_CELLHJUS ) );
        if (!isAmbiguous(sHoriJust))
        {
            table::CellHoriJustify aAPIAlignment = table::CellHoriJustify_BLOCK;

            if ( mxPropertySet->getPropertyValue(sHoriJust) >>= aAPIAlignment )
            {
                switch( aAPIAlignment )
                {
                    case table::CellHoriJustify_BLOCK:
                        NRetAlignment = uno::makeAny( excel::XlHAlign::xlHAlignJustify );
                        break;
                    case table::CellHoriJustify_CENTER:
                        NRetAlignment = uno::makeAny( excel::XlHAlign::xlHAlignCenter );
                        break;
                    case table::CellHoriJustify_LEFT:
                        NRetAlignment = uno::makeAny( excel::XlHAlign::xlHAlignLeft );
                        break;
                    case table::CellHoriJustify_RIGHT:
                        NRetAlignment =  uno::makeAny( excel::XlHAlign::xlHAlignRight );
                        break;
                     default: // handle those other cases with a NULL return
                        break;
                }
            }
        }
    }
    catch (uno::Exception& )
    {
        DebugHelper::exception(SbERR_METHOD_FAILED, rtl::OUString() );
    }
    return NRetAlignment;
}

template< typename Ifc1 >
void SAL_CALL
ScVbaFormat<Ifc1>::setOrientation( const uno::Any& _aOrientation ) throw (script::BasicErrorException, uno::RuntimeException)
{
    try
    {
        sal_Int32 nOrientation = 0;
        if ( !( _aOrientation >>= nOrientation ) )
            throw uno::RuntimeException();
        uno::Any aVal;
        switch( nOrientation )
        {
            case excel::XlOrientation::xlDownward:
                aVal = uno::makeAny( table::CellOrientation_TOPBOTTOM);
                break;
            case excel::XlOrientation::xlHorizontal:
                aVal = uno::makeAny( table::CellOrientation_STANDARD );
                mxPropertySet->setPropertyValue( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( SC_UNONAME_ROTANG ) ), uno::makeAny( sal_Int32(0) ) );
                break;
            case excel::XlOrientation::xlUpward:
                aVal = uno::makeAny( table::CellOrientation_BOTTOMTOP);
                break;
            case excel::XlOrientation::xlVertical:
                aVal = uno::makeAny( table::CellOrientation_STACKED);
                break;
        }
        // #FIXME what about the default case above?
        // shouldn't need the test below
        if ( aVal.hasValue() )
            mxPropertySet->setPropertyValue( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( SC_UNONAME_CELLORI ) ), aVal );

    }
    catch (uno::Exception& )
    {
        DebugHelper::exception(SbERR_METHOD_FAILED, rtl::OUString() );
    }
}
template< typename Ifc1 >
uno::Any SAL_CALL
ScVbaFormat<Ifc1>::getOrientation(  ) throw (script::BasicErrorException, uno::RuntimeException)
{
    uno::Any NRetOrientation = aNULL();
    try
    {
        if (!isAmbiguous(rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( SC_UNONAME_CELLORI ) )))
        {
            table::CellOrientation aOrientation = table::CellOrientation_STANDARD;
            if ( !(  mxPropertySet->getPropertyValue( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( SC_UNONAME_CELLORI ) ) ) >>= aOrientation ) )
                throw uno::RuntimeException();

            switch(aOrientation)
            {
                case table::CellOrientation_STANDARD:
                    NRetOrientation = uno::makeAny( excel::XlOrientation::xlHorizontal );
                    break;
                case table::CellOrientation_BOTTOMTOP:
                    NRetOrientation = uno::makeAny( excel::XlOrientation::xlUpward );
                    break;
                case table::CellOrientation_TOPBOTTOM:
                    NRetOrientation = uno::makeAny( excel::XlOrientation::xlDownward );
                    break;
                case table::CellOrientation_STACKED:
                    NRetOrientation = uno::makeAny( excel::XlOrientation::xlVertical );
                    break;
                default:
                    NRetOrientation = uno::makeAny( excel::XlOrientation::xlHorizontal );
            }
        }
    }
    catch (uno::Exception& )
    {
        DebugHelper::exception(SbERR_METHOD_FAILED, rtl::OUString());
    }
    return NRetOrientation;
}

template< typename Ifc1 >
void SAL_CALL
ScVbaFormat<Ifc1>::setWrapText( const uno::Any& _aWrapText ) throw (script::BasicErrorException, uno::RuntimeException)
{
    try
    {
        mxPropertySet->setPropertyValue( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( SC_UNONAME_WRAP ) ), _aWrapText);
    }
    catch (uno::Exception& )
    {
        DebugHelper::exception(SbERR_METHOD_FAILED, rtl::OUString() );
    }
}

template< typename Ifc1 >
uno::Any SAL_CALL
ScVbaFormat<Ifc1>::getWrapText(  ) throw (script::BasicErrorException, uno::RuntimeException)
{
    uno::Any aWrap = aNULL();
    try
    {
        rtl::OUString aPropName( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( SC_UNONAME_WRAP ) ) );
        if (!isAmbiguous( aPropName ))
        {
            aWrap = mxPropertySet->getPropertyValue(aPropName);
        }
    }
    catch (uno::Exception& )
    {
        DebugHelper::exception(SbERR_METHOD_FAILED, rtl::OUString() );
    }
    return aWrap;
}

template< typename Ifc1 >
uno::Any SAL_CALL
ScVbaFormat<Ifc1>::Borders( const uno::Any& Index ) throw (script::BasicErrorException, uno::RuntimeException )
{
    ScVbaPalette aPalette( excel::getDocShell( mxModel ) );
    uno::Reference< XCollection > xColl =  new ScVbaBorders( thisHelperIface(), ScVbaFormat_BASE::mxContext, uno::Reference< table::XCellRange >( mxPropertySet, uno::UNO_QUERY_THROW ), aPalette );

    if ( Index.hasValue() )
    {
        return xColl->Item( Index, uno::Any() );
    }
    return uno::makeAny( xColl );
}

template< typename Ifc1 >
uno::Reference< excel::XFont > SAL_CALL
ScVbaFormat<Ifc1>::Font(  ) throw (script::BasicErrorException, uno::RuntimeException)
{
    ScVbaPalette aPalette( excel::getDocShell( mxModel ) );
    return new ScVbaFont( thisHelperIface(), ScVbaFormat_BASE::mxContext, aPalette, mxPropertySet );
}

template< typename Ifc1 >
uno::Reference< excel::XInterior > SAL_CALL
ScVbaFormat<Ifc1>::Interior(  ) throw (script::BasicErrorException, uno::RuntimeException)
{
    return new ScVbaInterior( thisHelperIface(), ScVbaFormat_BASE::mxContext, mxPropertySet );
}

template< typename Ifc1 >
uno::Any SAL_CALL
ScVbaFormat<Ifc1>::getNumberFormatLocal(  ) throw (script::BasicErrorException, uno::RuntimeException)
{
    uno::Any aRet = uno::makeAny( rtl::OUString() );
    try
    {
        rtl::OUString sPropName( RTL_CONSTASCII_USTRINGPARAM( SC_UNO_NUMBERFO ) );
        if (!isAmbiguous( sPropName ))
        {

            initializeNumberFormats();

            sal_Int32 nFormat = 0;
            if ( ! (mxPropertySet->getPropertyValue( sPropName ) >>= nFormat ) )
                throw uno::RuntimeException();

            rtl::OUString sFormat;
            xNumberFormats->getByKey(nFormat)->getPropertyValue( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( FORMATSTRING ))) >>= sFormat;
            aRet = uno::makeAny( sFormat.toAsciiLowerCase() );

        }
    }
    catch (uno::Exception& )
    {
        DebugHelper::exception(SbERR_METHOD_FAILED, rtl::OUString());
    }
    return aRet;

}

template< typename Ifc1 >
void
ScVbaFormat<Ifc1>::setNumberFormat( lang::Locale _aLocale, const rtl::OUString& _sFormatString) throw( script::BasicErrorException )
{
    try
    {
        initializeNumberFormats();
        sal_Int32 nFormat = xNumberFormats->queryKey(_sFormatString, _aLocale , sal_True);
        if (nFormat == -1)
        {
            xNumberFormats->addNew(_sFormatString, _aLocale);
        }
        mxPropertySet->setPropertyValue( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( SC_UNO_NUMBERFO ) ), uno::makeAny( nFormat ) );
    }
    catch (uno::Exception& )
    {
        DebugHelper::exception(SbERR_METHOD_FAILED, rtl::OUString());
    }
}

template< typename Ifc1 >
void SAL_CALL
ScVbaFormat<Ifc1>::setNumberFormatLocal( const uno::Any& _oLocalFormatString ) throw (script::BasicErrorException, uno::RuntimeException)
{
    try
    {
        rtl::OUString sLocalFormatString;
        sal_Int32 nFormat = -1;
        rtl::OUString sNumFormat( RTL_CONSTASCII_USTRINGPARAM( SC_UNO_NUMBERFO ) );
        if ( !(_oLocalFormatString >>= sLocalFormatString )
        || !( mxPropertySet->getPropertyValue(sNumFormat) >>= nFormat ) )
            throw uno::RuntimeException();

        sLocalFormatString = sLocalFormatString.toAsciiUpperCase();
        initializeNumberFormats();
        lang::Locale aRangeLocale;
        xNumberFormats->getByKey(nFormat)->getPropertyValue( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( LOCALE ) ) ) >>= aRangeLocale;
        sal_Int32 nNewFormat = xNumberFormats->queryKey(sLocalFormatString, aRangeLocale, sal_True);

        if (nNewFormat == -1)
            nNewFormat = xNumberFormats->addNew(sLocalFormatString, aRangeLocale);
        mxPropertySet->setPropertyValue(sNumFormat, uno::makeAny( nNewFormat ));
    }
    catch (uno::Exception& )
    {
        DebugHelper::exception(SbERR_METHOD_FAILED, rtl::OUString() );
    }
}

template< typename Ifc1 >
void SAL_CALL
ScVbaFormat<Ifc1>::setNumberFormat( const uno::Any& _oFormatString ) throw (script::BasicErrorException, uno::RuntimeException)
{
    try
    {
        rtl::OUString sFormatString;
        if ( !( _oFormatString >>= sFormatString ) )
            throw uno::RuntimeException();

        sFormatString = sFormatString.toAsciiUpperCase();

        lang::Locale aDefaultLocale = m_aDefaultLocale;
        initializeNumberFormats();
        sal_Int32 nFormat = xNumberFormats->queryKey(sFormatString, aDefaultLocale, sal_True);

        if (nFormat == -1)
            nFormat = xNumberFormats->addNew(sFormatString, aDefaultLocale);

        lang::Locale aRangeLocale;
        xNumberFormats->getByKey(nFormat)->getPropertyValue( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( LOCALE ) ) ) >>= aRangeLocale;
        sal_Int32 nNewFormat = xNumberFormatTypes->getFormatForLocale(nFormat, aRangeLocale);
        mxPropertySet->setPropertyValue( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( SC_UNO_NUMBERFO ) ), uno::makeAny( nNewFormat));
    }
    catch (uno::Exception& )
    {
        DebugHelper::exception(SbERR_METHOD_FAILED, rtl::OUString());
    }

}

template< typename Ifc1 >
void SAL_CALL
ScVbaFormat<Ifc1>::setIndentLevel( const uno::Any& _aLevel ) throw (script::BasicErrorException, uno::RuntimeException)
{
    try
    {
        sal_Int32 nLevel = 0;
        if ( !(_aLevel >>= nLevel ) )
            throw uno::RuntimeException();
        table::CellHoriJustify aAPIAlignment = table::CellHoriJustify_STANDARD;

        rtl::OUString sHoriJust( RTL_CONSTASCII_USTRINGPARAM( SC_UNONAME_CELLHJUS ) );
        if ( !( mxPropertySet->getPropertyValue(sHoriJust) >>= aAPIAlignment ) )
            throw uno::RuntimeException();
        if (aAPIAlignment == table::CellHoriJustify_STANDARD)
            mxPropertySet->setPropertyValue( sHoriJust, uno::makeAny( table::CellHoriJustify_LEFT) ) ;
        mxPropertySet->setPropertyValue( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( SC_UNONAME_PINDENT ) ), uno::makeAny( sal_Int16(nLevel * 352.8) ) );
    }
    catch (uno::Exception& )
    {
        DebugHelper::exception(SbERR_METHOD_FAILED, rtl::OUString());
    }
}

template< typename Ifc1 >
uno::Any SAL_CALL
ScVbaFormat<Ifc1>::getIndentLevel(  ) throw (script::BasicErrorException, uno::RuntimeException)
{
    uno::Any NRetIndentLevel = aNULL();
    try
    {
        rtl::OUString sParaIndent( RTL_CONSTASCII_USTRINGPARAM( SC_UNONAME_PINDENT ) );
        if (!isAmbiguous(sParaIndent))
        {
            sal_Int16 IndentLevel = 0;
            if ( ( mxPropertySet->getPropertyValue(sParaIndent) >>= IndentLevel  ) )
                NRetIndentLevel = uno::makeAny( sal_Int32( rtl::math::round(static_cast<double>( IndentLevel ) / 352.8)) );
            else
                NRetIndentLevel = uno::makeAny( sal_Int32(0) );
        }
    }
    catch (uno::Exception& )
    {
        DebugHelper::exception(SbERR_METHOD_FAILED, rtl::OUString());
    }
    return NRetIndentLevel;
}

template< typename Ifc1 >
void SAL_CALL
ScVbaFormat<Ifc1>::setLocked( const uno::Any& _aLocked ) throw (script::BasicErrorException, uno::RuntimeException)
{
    try
    {
        sal_Bool bIsLocked = false;
        if ( !( _aLocked >>= bIsLocked ) )
            throw uno::RuntimeException();
        util::CellProtection aCellProtection;
        rtl::OUString sCellProt( RTL_CONSTASCII_USTRINGPARAM( SC_UNONAME_CELLPRO ) );
        mxPropertySet->getPropertyValue(sCellProt) >>= aCellProtection;
        aCellProtection.IsLocked = bIsLocked;
        mxPropertySet->setPropertyValue(sCellProt, uno::makeAny( aCellProtection ) );
    }
    catch (uno::Exception& )
    {
        DebugHelper::exception(SbERR_METHOD_FAILED, rtl::OUString() );
    }
}

template< typename Ifc1 >
void SAL_CALL
ScVbaFormat<Ifc1>::setFormulaHidden( const uno::Any& FormulaHidden ) throw (script::BasicErrorException, uno::RuntimeException)
{
    try
    {
        sal_Bool bIsFormulaHidden = false;
        FormulaHidden >>= bIsFormulaHidden;
        util::CellProtection aCellProtection;
        rtl::OUString sCellProt( RTL_CONSTASCII_USTRINGPARAM( SC_UNONAME_CELLPRO ) );
        mxPropertySet->getPropertyValue(sCellProt) >>= aCellProtection;
        aCellProtection.IsFormulaHidden = bIsFormulaHidden;
        mxPropertySet->setPropertyValue(sCellProt,uno::makeAny(aCellProtection));
    }
    catch (uno::Exception& )
    {
        DebugHelper::exception( SbERR_METHOD_FAILED, rtl::OUString() );
    }
}

template< typename Ifc1 >
uno::Any SAL_CALL
ScVbaFormat<Ifc1>::getLocked(  ) throw (script::BasicErrorException, uno::RuntimeException)
{
    uno::Any aCellProtection = aNULL();
    try
    {
        rtl::OUString sCellProt( RTL_CONSTASCII_USTRINGPARAM( SC_UNONAME_CELLPRO ) );

        if (!isAmbiguous(sCellProt))
        {
            SfxItemSet* pDataSet = getCurrentDataSet();
            if ( pDataSet )
            {
                const ScProtectionAttr& rProtAttr = (const ScProtectionAttr &) pDataSet->Get(ATTR_PROTECTION, sal_True);
                SfxItemState eState = pDataSet->GetItemState(ATTR_PROTECTION, sal_True, NULL);
                if(eState != SFX_ITEM_DONTCARE)
                    aCellProtection =  uno::makeAny(rProtAttr.GetProtection());
            }
            else // fallback to propertyset
            {
                util::CellProtection cellProtection;
                mxPropertySet->getPropertyValue(sCellProt) >>= aCellProtection;
                aCellProtection = uno::makeAny( cellProtection.IsLocked );
            }
        }
    }
    catch (uno::Exception& )
    {
        DebugHelper::exception(SbERR_METHOD_FAILED, rtl::OUString());
    }
    return aCellProtection;
}

template< typename Ifc1 >
uno::Any SAL_CALL
ScVbaFormat<Ifc1>::getFormulaHidden(  ) throw (script::BasicErrorException, uno::RuntimeException)
{
    uno::Any aBoolRet = aNULL();
    try
    {
        rtl::OUString sCellProt( RTL_CONSTASCII_USTRINGPARAM( SC_UNONAME_CELLPRO ) );
        if (!isAmbiguous(sCellProt))
        {
            SfxItemSet* pDataSet = getCurrentDataSet();
            if ( pDataSet )
            {
                const ScProtectionAttr& rProtAttr = (const ScProtectionAttr &) pDataSet->Get(ATTR_PROTECTION, sal_True);
                SfxItemState eState = pDataSet->GetItemState(ATTR_PROTECTION, sal_True, NULL);
                if(eState != SFX_ITEM_DONTCARE)
                    aBoolRet = uno::makeAny(rProtAttr.GetHideFormula());
            }
            else
            {
                util::CellProtection aCellProtection;
                mxPropertySet->getPropertyValue(sCellProt) >>= aCellProtection;
                aBoolRet = uno::makeAny( aCellProtection.IsFormulaHidden );
            }
        }
    }
    catch (uno::Exception e)
    {
        DebugHelper::exception(SbERR_METHOD_FAILED, rtl::OUString());
    }
    return aBoolRet;
}

template< typename Ifc1 >
void SAL_CALL
ScVbaFormat<Ifc1>::setShrinkToFit( const uno::Any& ShrinkToFit ) throw (script::BasicErrorException, uno::RuntimeException)
{
    try
    {
        mxPropertySet->setPropertyValue(rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( SC_UNONAME_SHRINK_TO_FIT ) ), ShrinkToFit);
    }
    catch (uno::Exception& )
    {
        DebugHelper::exception(SbERR_NOT_IMPLEMENTED, rtl::OUString() );
    }

}

template< typename Ifc1 >
uno::Any SAL_CALL
ScVbaFormat<Ifc1>::getShrinkToFit(  ) throw (script::BasicErrorException, uno::RuntimeException)
{
    uno::Any aRet = aNULL();
    try
    {
        rtl::OUString sShrinkToFit( RTL_CONSTASCII_USTRINGPARAM( SC_UNONAME_SHRINK_TO_FIT ) );
        if (!isAmbiguous(sShrinkToFit))
            aRet = mxPropertySet->getPropertyValue(sShrinkToFit);
    }
    catch (uno::Exception& )
    {
        DebugHelper::exception(SbERR_NOT_IMPLEMENTED, rtl::OUString());
    }
    return aRet;
}

template< typename Ifc1 >
void SAL_CALL
ScVbaFormat<Ifc1>::setReadingOrder( const uno::Any& ReadingOrder ) throw (script::BasicErrorException, uno::RuntimeException)
{
    try
    {
        sal_Int32 nReadingOrder = 0;
        if ( !(ReadingOrder >>= nReadingOrder ))
            throw uno::RuntimeException();
        uno::Any aVal;
        switch(nReadingOrder)
        {
            case excel::Constants::xlLTR:
                aVal = uno::makeAny( text::WritingMode_LR_TB );
                break;
            case excel::Constants::xlRTL:
                aVal = uno::makeAny( text::WritingMode_RL_TB );
                break;
            case excel::Constants::xlContext:
                DebugHelper::exception(SbERR_NOT_IMPLEMENTED, rtl::OUString());
                break;
            default:
                DebugHelper::exception(SbERR_METHOD_FAILED, rtl::OUString());
        }
        mxPropertySet->setPropertyValue( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( SC_UNONAME_WRITING ) ), aVal );
    }
    catch (uno::Exception& )
    {
        DebugHelper::exception(SbERR_METHOD_FAILED, rtl::OUString());
    }

}

template< typename Ifc1 >
uno::Any SAL_CALL
ScVbaFormat<Ifc1>::getReadingOrder(  ) throw (script::BasicErrorException, uno::RuntimeException)
{
    uno::Any NRetReadingOrder = aNULL();
    try
    {
        rtl::OUString sWritingMode( RTL_CONSTASCII_USTRINGPARAM( SC_UNONAME_WRITING ) );
        if (!isAmbiguous(sWritingMode))
        {
            text::WritingMode aWritingMode = text::WritingMode_LR_TB;
            if ( ( mxPropertySet->getPropertyValue(sWritingMode) ) >>= aWritingMode )
            switch (aWritingMode){
                case text::WritingMode_LR_TB:
                    NRetReadingOrder = uno::makeAny(excel::Constants::xlLTR);
                    break;
                case text::WritingMode_RL_TB:
                    NRetReadingOrder = uno::makeAny(excel::Constants::xlRTL);
                    break;
                default:
                    NRetReadingOrder = uno::makeAny(excel::Constants::xlRTL);
            }
        }
    }
    catch (uno::Exception& )
    {
        DebugHelper::exception(SbERR_NOT_IMPLEMENTED, rtl::OUString());
    }
    return NRetReadingOrder;

}

template< typename Ifc1 >
uno::Any SAL_CALL
ScVbaFormat< Ifc1 >::getNumberFormat(  ) throw (script::BasicErrorException, uno::RuntimeException)
{
    uno::Any aFormat = aNULL();
    try
    {
        sal_Int32 nFormat = -1;
        rtl::OUString sNumFormat( RTL_CONSTASCII_USTRINGPARAM( SC_UNO_NUMBERFO ) );
        if (!isAmbiguous(sNumFormat) &&
            ( mxPropertySet->getPropertyValue(sNumFormat) >>= nFormat) )
        {
            initializeNumberFormats();

            sal_Int32 nNewFormat = xNumberFormatTypes->getFormatForLocale(nFormat, getDefaultLocale() );
            rtl::OUString sFormat;
            xNumberFormats->getByKey(nNewFormat)->getPropertyValue( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( FORMATSTRING ))) >>= sFormat;
            aFormat = uno::makeAny( sFormat );
        }
    }
    catch (uno::Exception& )
    {
        DebugHelper::exception(SbERR_METHOD_FAILED, rtl::OUString());
    }
    return aFormat;
}

template< typename Ifc1 >
bool
ScVbaFormat<Ifc1>::isAmbiguous(const rtl::OUString& _sPropertyName) throw ( script::BasicErrorException )
{
    bool bResult = false;
    try
    {
        if (mbCheckAmbiguoity)
            bResult = ( getXPropertyState()->getPropertyState(_sPropertyName) == beans::PropertyState_AMBIGUOUS_VALUE );
    }
    catch (uno::Exception& )
    {
        DebugHelper::exception(SbERR_METHOD_FAILED, rtl::OUString());
    }
    return bResult;
}

template< typename Ifc1 >
void
ScVbaFormat<Ifc1>::initializeNumberFormats() throw ( script::BasicErrorException )
{
    if ( !xNumberFormats.is() )
    {
        mxNumberFormatsSupplier.set( mxModel, uno::UNO_QUERY_THROW );
        xNumberFormats = mxNumberFormatsSupplier->getNumberFormats();
        xNumberFormatTypes.set( xNumberFormats, uno::UNO_QUERY ); // _THROW?
    }
}

template< typename Ifc1 >
uno::Reference< beans::XPropertyState >
ScVbaFormat<Ifc1>::getXPropertyState() throw ( uno::RuntimeException )
{
    if ( !xPropertyState.is() )
        xPropertyState.set( mxPropertySet, uno::UNO_QUERY_THROW );
    return xPropertyState;
}

template< typename Ifc1 >
rtl::OUString&
ScVbaFormat<Ifc1>::getServiceImplName()
{
        static rtl::OUString sImplName( RTL_CONSTASCII_USTRINGPARAM("ScVbaFormat") );
        return sImplName;
}

template< typename Ifc1 >
uno::Sequence< rtl::OUString >
ScVbaFormat<Ifc1>::getServiceNames()
{
        static uno::Sequence< rtl::OUString > aServiceNames;
        if ( aServiceNames.getLength() == 0 )
        {
                aServiceNames.realloc( 1 );
                aServiceNames[ 0 ] = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("ooo.vba.excel.Format" ) );
        }
        return aServiceNames;
}

template< typename Ifc1 >
ScCellRangesBase*
ScVbaFormat<Ifc1>::getCellRangesBase() throw ( ::uno::RuntimeException )
{
    return ScCellRangesBase::getImplementation( mxPropertySet );
}

template< typename Ifc1 >
SfxItemSet*
ScVbaFormat<Ifc1>::getCurrentDataSet( ) throw ( uno::RuntimeException )
{
    SfxItemSet* pDataSet = excel::ScVbaCellRangeAccess::GetDataSet( getCellRangesBase() );
    if ( !pDataSet )
        throw uno::RuntimeException( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "Can't access Itemset for XPropertySet" ) ), uno::Reference< uno::XInterface >() );
    return pDataSet;
}


template class ScVbaFormat< excel::XStyle >;
template class ScVbaFormat< excel::XRange >;


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
