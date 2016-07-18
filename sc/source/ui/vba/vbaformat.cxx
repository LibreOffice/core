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

#include <basic/sberrors.hxx>
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

template< typename... Ifc >
ScVbaFormat< Ifc... >::ScVbaFormat( const uno::Reference< XHelperInterface >& xParent,
                                    const uno::Reference< uno::XComponentContext > & xContext,
                                    const uno::Reference< beans::XPropertySet >& _xPropertySet,
                                    const uno::Reference< frame::XModel >& xModel,
                                    bool bCheckAmbiguoity ) throw ( script::BasicErrorException )
    : ScVbaFormat_BASE( xParent, xContext ),
      m_aDefaultLocale( OUString("en"), OUString( "US"), OUString() ),
      mxPropertySet( _xPropertySet ),
      mxModel( xModel ),
      mbCheckAmbiguoity( bCheckAmbiguoity ),
      mbAddIndent( false )
{
    try
    {
        if ( !mxModel.is() )
            DebugHelper::basicexception(ERRCODE_BASIC_METHOD_FAILED, "XModel Interface could not be retrieved" );
        // mxServiceInfo is unused,
        // mxNumberFormatsSupplier is initialized when needed in initializeNumberFormats.
    }
    catch (const uno::Exception& )
    {
        DebugHelper::basicexception(ERRCODE_BASIC_METHOD_FAILED, OUString() );
    }
}

template< typename... Ifc >
void SAL_CALL
ScVbaFormat< Ifc... >::setVerticalAlignment( const uno::Any& _oAlignment)   throw (script::BasicErrorException, uno::RuntimeException)
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
        mxPropertySet->setPropertyValue( SC_UNONAME_CELLVJUS, aVal );
    }
    catch (const uno::Exception&)
    {
        DebugHelper::basicexception(ERRCODE_BASIC_METHOD_FAILED, OUString());
    }
}

template< typename... Ifc >
uno::Any SAL_CALL
ScVbaFormat< Ifc... >::getVerticalAlignment(  ) throw (script::BasicErrorException, uno::RuntimeException)
{
    uno::Any aResult = aNULL();
    try
    {
        if (!isAmbiguous( SC_UNONAME_CELLVJUS ) )
        {
            sal_Int32 aAPIAlignment = table::CellVertJustify2::STANDARD;
            mxPropertySet->getPropertyValue( SC_UNONAME_CELLVJUS ) >>= aAPIAlignment;
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
    catch (const uno::Exception& )
    {
        DebugHelper::basicexception(ERRCODE_BASIC_METHOD_FAILED, OUString());
    }
    return aResult;
}

template< typename... Ifc >
void SAL_CALL
ScVbaFormat< Ifc... >::setHorizontalAlignment( const uno::Any& HorizontalAlignment ) throw (script::BasicErrorException, uno::RuntimeException)
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
            mxPropertySet->setPropertyValue( SC_UNONAME_CELLHJUS, aVal );
    }
    catch (const uno::Exception& )
    {
        DebugHelper::basicexception(ERRCODE_BASIC_METHOD_FAILED, OUString() );
    }

}

template< typename... Ifc >
uno::Any SAL_CALL
ScVbaFormat< Ifc... >::getHorizontalAlignment(  ) throw (script::BasicErrorException, uno::RuntimeException)
{
    uno::Any NRetAlignment = aNULL();
    try
    {
        OUString sHoriJust( SC_UNONAME_CELLHJUS );
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
    catch (const uno::Exception& )
    {
        DebugHelper::basicexception(ERRCODE_BASIC_METHOD_FAILED, OUString() );
    }
    return NRetAlignment;
}

template< typename... Ifc >
void SAL_CALL
ScVbaFormat< Ifc... >::setOrientation( const uno::Any& _aOrientation ) throw (script::BasicErrorException, uno::RuntimeException)
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
                mxPropertySet->setPropertyValue( SC_UNONAME_ROTANG, uno::makeAny( sal_Int32(0) ) );
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
            mxPropertySet->setPropertyValue( SC_UNONAME_CELLORI, aVal );

    }
    catch (const uno::Exception& )
    {
        DebugHelper::basicexception(ERRCODE_BASIC_METHOD_FAILED, OUString() );
    }
}
template< typename... Ifc >
uno::Any SAL_CALL
ScVbaFormat< Ifc... >::getOrientation(  ) throw (script::BasicErrorException, uno::RuntimeException)
{
    uno::Any NRetOrientation = aNULL();
    try
    {
        if (!isAmbiguous(SC_UNONAME_CELLORI))
        {
            table::CellOrientation aOrientation = table::CellOrientation_STANDARD;
            if ( !(  mxPropertySet->getPropertyValue( SC_UNONAME_CELLORI ) >>= aOrientation ) )
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
    catch (const uno::Exception& )
    {
        DebugHelper::basicexception(ERRCODE_BASIC_METHOD_FAILED, OUString());
    }
    return NRetOrientation;
}

template< typename... Ifc >
void SAL_CALL
ScVbaFormat< Ifc... >::setWrapText( const uno::Any& _aWrapText ) throw (script::BasicErrorException, uno::RuntimeException)
{
    try
    {
        mxPropertySet->setPropertyValue( SC_UNONAME_WRAP, _aWrapText);
    }
    catch (const uno::Exception& )
    {
        DebugHelper::basicexception(ERRCODE_BASIC_METHOD_FAILED, OUString() );
    }
}

template< typename... Ifc >
uno::Any SAL_CALL
ScVbaFormat< Ifc... >::getWrapText(  ) throw (script::BasicErrorException, uno::RuntimeException, std::exception)
{
    uno::Any aWrap = aNULL();
    try
    {
        OUString aPropName( SC_UNONAME_WRAP );
        if (!isAmbiguous( aPropName ))
        {
            aWrap = mxPropertySet->getPropertyValue(aPropName);
        }
    }
    catch (const uno::Exception&)
    {
        DebugHelper::basicexception(ERRCODE_BASIC_METHOD_FAILED, OUString() );
    }
    return aWrap;
}

template< typename... Ifc >
uno::Any SAL_CALL
ScVbaFormat< Ifc... >::Borders( const uno::Any& Index ) throw (script::BasicErrorException, uno::RuntimeException )
{
    ScVbaPalette aPalette( excel::getDocShell( mxModel ) );
    uno::Reference< XCollection > xColl =  new ScVbaBorders( thisHelperIface(), ScVbaFormat_BASE::mxContext, uno::Reference< table::XCellRange >( mxPropertySet, uno::UNO_QUERY_THROW ), aPalette );

    if ( Index.hasValue() )
    {
        return xColl->Item( Index, uno::Any() );
    }
    return uno::makeAny( xColl );
}

template< typename... Ifc >
uno::Reference< excel::XFont > SAL_CALL
ScVbaFormat< Ifc... >::Font(  ) throw (script::BasicErrorException, uno::RuntimeException)
{
    ScVbaPalette aPalette( excel::getDocShell( mxModel ) );
    return new ScVbaFont( thisHelperIface(), ScVbaFormat_BASE::mxContext, aPalette, mxPropertySet );
}

template< typename... Ifc >
uno::Reference< excel::XInterior > SAL_CALL
ScVbaFormat< Ifc... >::Interior(  ) throw (script::BasicErrorException, uno::RuntimeException)
{
    return new ScVbaInterior( thisHelperIface(), ScVbaFormat_BASE::mxContext, mxPropertySet );
}

template< typename... Ifc >
uno::Any SAL_CALL
ScVbaFormat< Ifc... >::getNumberFormatLocal(  ) throw (script::BasicErrorException, uno::RuntimeException)
{
    uno::Any aRet = uno::makeAny( OUString() );
    try
    {
        OUString sPropName( SC_UNO_DP_NUMBERFO );
        if (!isAmbiguous( sPropName ))
        {

            initializeNumberFormats();

            sal_Int32 nFormat = 0;
            if ( ! (mxPropertySet->getPropertyValue( sPropName ) >>= nFormat ) )
                throw uno::RuntimeException();

            OUString sFormat;
            xNumberFormats->getByKey(nFormat)->getPropertyValue( FORMATSTRING ) >>= sFormat;
            aRet = uno::makeAny( sFormat.toAsciiLowerCase() );

        }
    }
    catch (const uno::Exception&)
    {
        DebugHelper::basicexception(ERRCODE_BASIC_METHOD_FAILED, OUString());
    }
    return aRet;

}

template< typename... Ifc >
void SAL_CALL
ScVbaFormat< Ifc... >::setNumberFormatLocal( const uno::Any& _oLocalFormatString ) throw (script::BasicErrorException, uno::RuntimeException)
{
    try
    {
        OUString sLocalFormatString;
        sal_Int32 nFormat = -1;
        OUString sNumFormat( SC_UNO_DP_NUMBERFO );
        if ( !(_oLocalFormatString >>= sLocalFormatString )
        || !( mxPropertySet->getPropertyValue(sNumFormat) >>= nFormat ) )
            throw uno::RuntimeException();

        sLocalFormatString = sLocalFormatString.toAsciiUpperCase();
        initializeNumberFormats();
        lang::Locale aRangeLocale;
        xNumberFormats->getByKey(nFormat)->getPropertyValue( LOCALE ) >>= aRangeLocale;
        sal_Int32 nNewFormat = xNumberFormats->queryKey(sLocalFormatString, aRangeLocale, true);

        if (nNewFormat == -1)
            nNewFormat = xNumberFormats->addNew(sLocalFormatString, aRangeLocale);
        mxPropertySet->setPropertyValue(sNumFormat, uno::makeAny( nNewFormat ));
    }
    catch (const uno::Exception& )
    {
        DebugHelper::basicexception(ERRCODE_BASIC_METHOD_FAILED, OUString() );
    }
}

template< typename... Ifc >
void SAL_CALL
ScVbaFormat< Ifc... >::setNumberFormat( const uno::Any& _oFormatString ) throw (script::BasicErrorException, uno::RuntimeException)
{
    try
    {
        OUString sFormatString;
        if ( !( _oFormatString >>= sFormatString ) )
            throw uno::RuntimeException();

        sFormatString = sFormatString.toAsciiUpperCase();

        lang::Locale aDefaultLocale = m_aDefaultLocale;
        initializeNumberFormats();
        sal_Int32 nFormat = xNumberFormats->queryKey(sFormatString, aDefaultLocale, true);

        if (nFormat == -1)
            nFormat = xNumberFormats->addNew(sFormatString, aDefaultLocale);

        lang::Locale aRangeLocale;
        xNumberFormats->getByKey(nFormat)->getPropertyValue( LOCALE ) >>= aRangeLocale;
        sal_Int32 nNewFormat = xNumberFormatTypes->getFormatForLocale(nFormat, aRangeLocale);
        mxPropertySet->setPropertyValue( SC_UNO_DP_NUMBERFO, uno::makeAny( nNewFormat));
    }
    catch (const uno::Exception& )
    {
        DebugHelper::basicexception(ERRCODE_BASIC_METHOD_FAILED, OUString());
    }

}

template< typename... Ifc >
void SAL_CALL
ScVbaFormat< Ifc... >::setIndentLevel( const uno::Any& _aLevel ) throw (script::BasicErrorException, uno::RuntimeException)
{
    try
    {
        sal_Int32 nLevel = 0;
        if ( !(_aLevel >>= nLevel ) )
            throw uno::RuntimeException();
        table::CellHoriJustify aAPIAlignment = table::CellHoriJustify_STANDARD;

        OUString sHoriJust( SC_UNONAME_CELLHJUS );
        if ( !( mxPropertySet->getPropertyValue(sHoriJust) >>= aAPIAlignment ) )
            throw uno::RuntimeException();
        if (aAPIAlignment == table::CellHoriJustify_STANDARD)
            mxPropertySet->setPropertyValue( sHoriJust, uno::makeAny( table::CellHoriJustify_LEFT) ) ;
        mxPropertySet->setPropertyValue( SC_UNONAME_PINDENT, uno::makeAny( sal_Int16(nLevel * 352.8) ) );
    }
    catch (const uno::Exception&)
    {
        DebugHelper::basicexception(ERRCODE_BASIC_METHOD_FAILED, OUString());
    }
}

template< typename... Ifc >
uno::Any SAL_CALL
ScVbaFormat< Ifc... >::getIndentLevel(  ) throw (script::BasicErrorException, uno::RuntimeException)
{
    uno::Any NRetIndentLevel = aNULL();
    try
    {
        OUString sParaIndent( SC_UNONAME_PINDENT );
        if (!isAmbiguous(sParaIndent))
        {
            sal_Int16 IndentLevel = 0;
            if ( ( mxPropertySet->getPropertyValue(sParaIndent) >>= IndentLevel  ) )
                NRetIndentLevel = uno::makeAny( sal_Int32( rtl::math::round(static_cast<double>( IndentLevel ) / 352.8)) );
            else
                NRetIndentLevel = uno::makeAny( sal_Int32(0) );
        }
    }
    catch (const uno::Exception&)
    {
        DebugHelper::basicexception(ERRCODE_BASIC_METHOD_FAILED, OUString());
    }
    return NRetIndentLevel;
}

template< typename... Ifc >
void SAL_CALL
ScVbaFormat< Ifc... >::setLocked( const uno::Any& _aLocked ) throw (script::BasicErrorException, uno::RuntimeException)
{
    try
    {
        bool bIsLocked = false;
        if ( !( _aLocked >>= bIsLocked ) )
            throw uno::RuntimeException();
        util::CellProtection aCellProtection;
        OUString sCellProt( SC_UNONAME_CELLPRO );
        mxPropertySet->getPropertyValue(sCellProt) >>= aCellProtection;
        aCellProtection.IsLocked = bIsLocked;
        mxPropertySet->setPropertyValue(sCellProt, uno::makeAny( aCellProtection ) );
    }
    catch (const uno::Exception&)
    {
        DebugHelper::basicexception(ERRCODE_BASIC_METHOD_FAILED, OUString() );
    }
}

template< typename... Ifc >
void SAL_CALL
ScVbaFormat< Ifc... >::setFormulaHidden( const uno::Any& FormulaHidden ) throw (script::BasicErrorException, uno::RuntimeException)
{
    try
    {
        bool bIsFormulaHidden = false;
        FormulaHidden >>= bIsFormulaHidden;
        util::CellProtection aCellProtection;
        OUString sCellProt( SC_UNONAME_CELLPRO );
        mxPropertySet->getPropertyValue(sCellProt) >>= aCellProtection;
        aCellProtection.IsFormulaHidden = bIsFormulaHidden;
        mxPropertySet->setPropertyValue(sCellProt,uno::makeAny(aCellProtection));
    }
    catch (const uno::Exception&)
    {
        DebugHelper::basicexception( ERRCODE_BASIC_METHOD_FAILED, OUString() );
    }
}

template< typename... Ifc >
uno::Any SAL_CALL
ScVbaFormat< Ifc... >::getLocked(  ) throw (script::BasicErrorException, uno::RuntimeException, std::exception)
{
    uno::Any aCellProtection = aNULL();
    try
    {
        OUString sCellProt( SC_UNONAME_CELLPRO );

        if (!isAmbiguous(sCellProt))
        {
            SfxItemSet* pDataSet = getCurrentDataSet();
            if ( pDataSet )
            {
                const ScProtectionAttr& rProtAttr = static_cast<const ScProtectionAttr &>( pDataSet->Get(ATTR_PROTECTION) );
                SfxItemState eState = pDataSet->GetItemState(ATTR_PROTECTION);
                if(eState != SfxItemState::DONTCARE)
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
    catch (const uno::Exception&)
    {
        DebugHelper::basicexception(ERRCODE_BASIC_METHOD_FAILED, OUString());
    }
    return aCellProtection;
}

template< typename... Ifc >
uno::Any SAL_CALL
ScVbaFormat< Ifc... >::getFormulaHidden(  ) throw (script::BasicErrorException, uno::RuntimeException, std::exception)
{
    uno::Any aBoolRet = aNULL();
    try
    {
        OUString sCellProt( SC_UNONAME_CELLPRO );
        if (!isAmbiguous(sCellProt))
        {
            SfxItemSet* pDataSet = getCurrentDataSet();
            if ( pDataSet )
            {
                const ScProtectionAttr& rProtAttr = static_cast<const ScProtectionAttr &>( pDataSet->Get(ATTR_PROTECTION) );
                SfxItemState eState = pDataSet->GetItemState(ATTR_PROTECTION);
                if(eState != SfxItemState::DONTCARE)
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
    catch (const uno::Exception&)
    {
        DebugHelper::basicexception(ERRCODE_BASIC_METHOD_FAILED, OUString());
    }
    return aBoolRet;
}

template< typename... Ifc >
void SAL_CALL
ScVbaFormat< Ifc... >::setShrinkToFit( const uno::Any& ShrinkToFit ) throw (script::BasicErrorException, uno::RuntimeException)
{
    try
    {
        mxPropertySet->setPropertyValue( SC_UNONAME_SHRINK_TO_FIT, ShrinkToFit);
    }
    catch (const uno::Exception& )
    {
        DebugHelper::basicexception(ERRCODE_BASIC_NOT_IMPLEMENTED, OUString() );
    }

}

template< typename... Ifc >
uno::Any SAL_CALL
ScVbaFormat< Ifc... >::getShrinkToFit(  ) throw (script::BasicErrorException, uno::RuntimeException)
{
    uno::Any aRet = aNULL();
    try
    {
        OUString sShrinkToFit( SC_UNONAME_SHRINK_TO_FIT );
        if (!isAmbiguous(sShrinkToFit))
            aRet = mxPropertySet->getPropertyValue(sShrinkToFit);
    }
    catch (const uno::Exception& )
    {
        DebugHelper::basicexception(ERRCODE_BASIC_NOT_IMPLEMENTED, OUString());
    }
    return aRet;
}

template< typename... Ifc >
void SAL_CALL
ScVbaFormat< Ifc... >::setReadingOrder( const uno::Any& ReadingOrder ) throw (script::BasicErrorException, uno::RuntimeException)
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
                DebugHelper::basicexception(ERRCODE_BASIC_NOT_IMPLEMENTED, OUString());
                break;
            default:
                DebugHelper::basicexception(ERRCODE_BASIC_METHOD_FAILED, OUString());
        }
        mxPropertySet->setPropertyValue( SC_UNONAME_WRITING, aVal );
    }
    catch (const uno::Exception& )
    {
        DebugHelper::basicexception(ERRCODE_BASIC_METHOD_FAILED, OUString());
    }

}

template< typename... Ifc >
uno::Any SAL_CALL
ScVbaFormat< Ifc... >::getReadingOrder(  ) throw (script::BasicErrorException, uno::RuntimeException)
{
    uno::Any NRetReadingOrder = aNULL();
    try
    {
        OUString sWritingMode( SC_UNONAME_WRITING );
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
    catch (const uno::Exception& )
    {
        DebugHelper::basicexception(ERRCODE_BASIC_NOT_IMPLEMENTED, OUString());
    }
    return NRetReadingOrder;

}

template< typename... Ifc >
uno::Any SAL_CALL
ScVbaFormat< Ifc... >::getNumberFormat(  ) throw (script::BasicErrorException, uno::RuntimeException, std::exception)
{
    uno::Any aFormat = aNULL();
    try
    {
        sal_Int32 nFormat = -1;
        OUString sNumFormat( SC_UNO_DP_NUMBERFO );
        if (!isAmbiguous(sNumFormat) &&
            ( mxPropertySet->getPropertyValue(sNumFormat) >>= nFormat) )
        {
            initializeNumberFormats();

            sal_Int32 nNewFormat = xNumberFormatTypes->getFormatForLocale(nFormat, getDefaultLocale() );
            OUString sFormat;
            xNumberFormats->getByKey(nNewFormat)->getPropertyValue( FORMATSTRING ) >>= sFormat;
            aFormat = uno::makeAny( sFormat );
        }
    }
    catch (const uno::Exception& )
    {
        DebugHelper::basicexception(ERRCODE_BASIC_METHOD_FAILED, OUString());
    }
    return aFormat;
}

template< typename... Ifc >
bool
ScVbaFormat< Ifc... >::isAmbiguous(const OUString& _sPropertyName) throw ( script::BasicErrorException )
{
    bool bResult = false;
    try
    {
        if (mbCheckAmbiguoity)
            bResult = ( getXPropertyState()->getPropertyState(_sPropertyName) == beans::PropertyState_AMBIGUOUS_VALUE );
    }
    catch (const uno::Exception& )
    {
        DebugHelper::basicexception(ERRCODE_BASIC_METHOD_FAILED, OUString());
    }
    return bResult;
}

template< typename... Ifc >
void
ScVbaFormat< Ifc... >::initializeNumberFormats() throw ( script::BasicErrorException, uno::RuntimeException )
{
    if ( !xNumberFormats.is() )
    {
        mxNumberFormatsSupplier.set( mxModel, uno::UNO_QUERY_THROW );
        xNumberFormats = mxNumberFormatsSupplier->getNumberFormats();
        xNumberFormatTypes.set( xNumberFormats, uno::UNO_QUERY ); // _THROW?
    }
}

template< typename... Ifc >
uno::Reference< beans::XPropertyState > const &
ScVbaFormat< Ifc... >::getXPropertyState() throw ( uno::RuntimeException )
{
    if ( !xPropertyState.is() )
        xPropertyState.set( mxPropertySet, uno::UNO_QUERY_THROW );
    return xPropertyState;
}

template< typename... Ifc >
ScCellRangesBase*
ScVbaFormat< Ifc... >::getCellRangesBase() throw ( ::uno::RuntimeException )
{
    return ScCellRangesBase::getImplementation( mxPropertySet );
}

template< typename... Ifc >
SfxItemSet*
ScVbaFormat< Ifc... >::getCurrentDataSet() throw (uno::RuntimeException, std::exception)
{
    SfxItemSet* pDataSet = excel::ScVbaCellRangeAccess::GetDataSet( getCellRangesBase() );
    if ( !pDataSet )
        throw uno::RuntimeException("Can't access Itemset for XPropertySet" );
    return pDataSet;
}

template class ScVbaFormat< excel::XStyle >;
template class ScVbaFormat< excel::XRange >;

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
