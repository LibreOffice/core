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

#include <sal/config.h>

#include <utility>

#include <config_features.h>

#include <confuno.hxx>
#include <unonames.hxx>
#include <scdll.hxx>
#include <docsh.hxx>
#include <miscuno.hxx>
#include <forbiuno.hxx>
#include <appoptio.hxx>
#include <viewopti.hxx>
#include <docpool.hxx>
#include <sc.hrc>
#include <scmod.hxx>

#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <com/sun/star/document/LinkUpdateModes.hpp>
#include <cppuhelper/supportsservice.hxx>
#include <formula/grammar.hxx>
#include <sfx2/printer.hxx>
#include <xmloff/xmluconv.hxx>
#include <rtl/ustrbuf.hxx>
#include <vcl/svapp.hxx>

using namespace com::sun::star;

#define SCSAVEVERSION                           "SaveVersionOnClose"

static const SfxItemPropertyMapEntry* lcl_GetConfigPropertyMap()
{
    static const SfxItemPropertyMapEntry aConfigPropertyMap_Impl[] =
    {
        {OUString(SC_UNO_SHOWZERO),     0,  cppu::UnoType<bool>::get(),              0, 0},
        {OUString(SC_UNO_SHOWNOTES),    0,  cppu::UnoType<bool>::get(),              0, 0},
        {OUString(SC_UNO_SHOWGRID),     0,  cppu::UnoType<bool>::get(),              0, 0},
        {OUString(SC_UNO_GRIDCOLOR),    0,  cppu::UnoType<sal_Int32>::get(),        0, 0},
        {OUString(SC_UNO_SHOWPAGEBR),   0,  cppu::UnoType<bool>::get(),              0, 0},
        {OUString(SC_UNONAME_LINKUPD),  0,  cppu::UnoType<sal_Int16>::get(),        0, 0},
        {OUString(SC_UNO_COLROWHDR),    0,  cppu::UnoType<bool>::get(),              0, 0},
        {OUString(SC_UNO_SHEETTABS),    0,  cppu::UnoType<bool>::get(),              0, 0},
        {OUString(SC_UNO_OUTLSYMB),     0,  cppu::UnoType<bool>::get(),              0, 0},
        {OUString(SC_UNO_SNAPTORASTER), 0,  cppu::UnoType<bool>::get(),              0, 0},
        {OUString(SC_UNO_RASTERVIS),    0,  cppu::UnoType<bool>::get(),              0, 0},
        {OUString(SC_UNO_RASTERRESX),   0,  cppu::UnoType<sal_Int32>::get(),        0, 0},
        {OUString(SC_UNO_RASTERRESY),   0,  cppu::UnoType<sal_Int32>::get(),        0, 0},
        {OUString(SC_UNO_RASTERSUBX),   0,  cppu::UnoType<sal_Int32>::get(),        0, 0},
        {OUString(SC_UNO_RASTERSUBY),   0,  cppu::UnoType<sal_Int32>::get(),        0, 0},
        {OUString(SC_UNO_RASTERSYNC),   0,  cppu::UnoType<bool>::get(),              0, 0},
        {OUString(SC_UNO_AUTOCALC),     0,  cppu::UnoType<bool>::get(),              0, 0},
        {OUString(SC_UNO_PRINTERNAME),  0,  cppu::UnoType<OUString>::get(),    0, 0},
        {OUString(SC_UNO_PRINTERSETUP), 0,  cppu::UnoType<uno::Sequence<sal_Int8>>::get(), 0, 0},
        {OUString(SC_UNO_PRINTERPAPER), 0,  cppu::UnoType<bool>::get(),              0, 0},
        {OUString(SC_UNO_APPLYDOCINF),  0,  cppu::UnoType<bool>::get(),              0, 0 },
        {OUString(SC_UNO_SAVE_THUMBNAIL),  0,  cppu::UnoType<bool>::get(),              0, 0 },
        {OUString(SC_UNO_FORBIDDEN),    0,  cppu::UnoType<i18n::XForbiddenCharacters>::get(), beans::PropertyAttribute::READONLY, 0},
        {OUString(SC_UNO_CHARCOMP),     0,  cppu::UnoType<sal_Int16>::get(),        0, 0},
        {OUString(SC_UNO_ASIANKERN),    0,  cppu::UnoType<bool>::get(),              0, 0},
        {OUString(SCSAVEVERSION),       0,  cppu::UnoType<bool>::get(),              0, 0},
        {OUString(SC_UNO_UPDTEMPL),     0,  cppu::UnoType<bool>::get(),              0, 0},
        /*Stampit enable/disable print cancel */
        {OUString(SC_UNO_ALLOWPRINTJOBCANCEL), 0, cppu::UnoType<bool>::get(),        0, 0},
        {OUString(SC_UNO_LOADREADONLY), 0,  cppu::UnoType<bool>::get(),              0, 0},
        {OUString(SC_UNO_SHAREDOC),     0,  cppu::UnoType<bool>::get(),              0, 0},
        {OUString(SC_UNO_MODIFYPASSWORDINFO), 0,  cppu::UnoType<uno::Sequence< beans::PropertyValue >>::get(),              0, 0},
        {OUString(SC_UNO_EMBED_FONTS),               0,  cppu::UnoType<bool>::get(), 0, 0},
        {OUString(SC_UNO_EMBED_ONLY_USED_FONTS),     0,  cppu::UnoType<bool>::get(), 0, 0},
        {OUString(SC_UNO_EMBED_FONT_SCRIPT_LATIN),   0,  cppu::UnoType<bool>::get(), 0, 0},
        {OUString(SC_UNO_EMBED_FONT_SCRIPT_ASIAN),   0,  cppu::UnoType<bool>::get(), 0, 0},
        {OUString(SC_UNO_EMBED_FONT_SCRIPT_COMPLEX), 0,  cppu::UnoType<bool>::get(), 0, 0},
        {OUString(SC_UNO_SYNTAXSTRINGREF), 0,  cppu::UnoType<sal_Int16>::get(),     0, 0},
        { OUString(), 0, css::uno::Type(), 0, 0 }
    };
    return aConfigPropertyMap_Impl;
}

ScDocumentConfiguration::ScDocumentConfiguration(ScDocShell* pDocSh)
    : pDocShell(pDocSh) ,
    aPropSet ( lcl_GetConfigPropertyMap() )
{
    pDocShell->GetDocument().AddUnoObject(*this);
}

ScDocumentConfiguration::~ScDocumentConfiguration()
{
    SolarMutexGuard g;

    if (pDocShell)
        pDocShell->GetDocument().RemoveUnoObject(*this);
}

void ScDocumentConfiguration::Notify( SfxBroadcaster&, const SfxHint& rHint )
{
    //  reference update does not matter here

    if ( rHint.GetId() == SfxHintId::Dying )
    {
        pDocShell = nullptr;
    }
}

// XPropertySet

uno::Reference<beans::XPropertySetInfo> SAL_CALL ScDocumentConfiguration::getPropertySetInfo()
{
    SolarMutexGuard aGuard;
    static uno::Reference<beans::XPropertySetInfo> aRef(
        new SfxItemPropertySetInfo( aPropSet.getPropertyMap() ));
    return aRef;
}

void SAL_CALL ScDocumentConfiguration::setPropertyValue(
                        const OUString& aPropertyName, const uno::Any& aValue )
{
    SolarMutexGuard aGuard;

    if(!pDocShell)
        throw uno::RuntimeException();

    ScDocument& rDoc = pDocShell->GetDocument();
    bool bUpdateHeights = false;

    ScViewOptions aViewOpt(rDoc.GetViewOptions());

    /*Stampit enable/disable print cancel */
    if ( aPropertyName == SC_UNO_ALLOWPRINTJOBCANCEL )
        pDocShell->Stamp_SetPrintCancelState( ScUnoHelpFunctions::GetBoolFromAny( aValue ) );
    /*Stampit enable/disable print cancel */

    else if ( aPropertyName == SC_UNO_SHOWZERO )
        aViewOpt.SetOption(VOPT_NULLVALS, ScUnoHelpFunctions::GetBoolFromAny( aValue ) );
    else if ( aPropertyName == SC_UNO_SHOWNOTES )
        aViewOpt.SetOption(VOPT_NOTES, ScUnoHelpFunctions::GetBoolFromAny( aValue ) );
    else if ( aPropertyName == SC_UNO_SHOWGRID )
        aViewOpt.SetOption(VOPT_GRID, ScUnoHelpFunctions::GetBoolFromAny( aValue ) );
    else if ( aPropertyName == SC_UNO_GRIDCOLOR )
    {
        sal_Int64 nColor = 0;
        if (aValue >>= nColor)
        {
            Color aColor(static_cast<sal_uInt32>(nColor));
            aViewOpt.SetGridColor(aColor, OUString());
        }
    }
    else if ( aPropertyName == SC_UNO_SHOWPAGEBR )
        aViewOpt.SetOption(VOPT_PAGEBREAKS, ScUnoHelpFunctions::GetBoolFromAny( aValue ) );
    else if ( aPropertyName == SC_UNONAME_LINKUPD )
    {
        // XXX NOTE: this is the css::document::Settings property
        // LinkUpdateMode, not the css::sheet::XGlobalSheetSettings
        // attribute LinkUpdateMode.
        sal_Int16 n;
        if (!(aValue >>= n) || n < css::document::LinkUpdateModes::NEVER ||
                n > css::document::LinkUpdateModes::GLOBAL_SETTING)
        {
            throw css::lang::IllegalArgumentException(
                ("LinkUpdateMode property value must be a SHORT with a value in"
                 " the range of the css::document::LinkUpdateModes constants"),
                css::uno::Reference<css::uno::XInterface>(), -1);
        }
        ScLkUpdMode eMode;
        switch (n)
        {
            case css::document::LinkUpdateModes::NEVER:
                eMode = LM_NEVER;
            break;
            case css::document::LinkUpdateModes::MANUAL:
                eMode = LM_ON_DEMAND;
            break;
            case css::document::LinkUpdateModes::AUTO:
                eMode = LM_ALWAYS;
            break;
            case css::document::LinkUpdateModes::GLOBAL_SETTING:
            default:
                eMode = SC_MOD()->GetAppOptions().GetLinkMode();
            break;
        }
        rDoc.SetLinkMode( eMode );
    }
    else if ( aPropertyName == SC_UNO_COLROWHDR )
        aViewOpt.SetOption(VOPT_HEADER, ScUnoHelpFunctions::GetBoolFromAny( aValue ) );
    else if ( aPropertyName == SC_UNO_SHEETTABS )
        aViewOpt.SetOption(VOPT_TABCONTROLS, ScUnoHelpFunctions::GetBoolFromAny( aValue ) );
    else if ( aPropertyName == SC_UNO_OUTLSYMB )
        aViewOpt.SetOption(VOPT_OUTLINER, ScUnoHelpFunctions::GetBoolFromAny( aValue ) );
    else if ( aPropertyName == SC_UNO_AUTOCALC )
        rDoc.SetAutoCalc( ScUnoHelpFunctions::GetBoolFromAny( aValue ) );
    else if ( aPropertyName == SC_UNO_PRINTERNAME )
    {
        OUString sPrinterName;
        if ( !(aValue >>= sPrinterName) )
            throw lang::IllegalArgumentException();

        // #i75610# if the name is empty, do nothing (don't create any printer)
        if ( !sPrinterName.isEmpty() && pDocShell->GetCreateMode() != SfxObjectCreateMode::EMBEDDED )
        {
            SfxPrinter* pPrinter = pDocShell->GetPrinter();
            if (!pPrinter)
                throw uno::RuntimeException();

            if (pPrinter->GetName() != sPrinterName)
            {
                VclPtrInstance<SfxPrinter> pNewPrinter( pPrinter->GetOptions().Clone(), sPrinterName );
                if (pNewPrinter->IsKnown())
                    pDocShell->SetPrinter( pNewPrinter, SfxPrinterChangeFlags::PRINTER );
                else
                    pNewPrinter.disposeAndClear();
            }

        }

    }
    else if ( aPropertyName == SC_UNO_PRINTERSETUP )
    {
        uno::Sequence<sal_Int8> aSequence;
        if ( aValue >>= aSequence )
        {
            sal_uInt32 nSize = aSequence.getLength();
            // #i75610# if the sequence is empty, do nothing (don't create any printer)
            if ( nSize != 0 )
            {
                SvMemoryStream aStream (aSequence.getArray(), nSize, StreamMode::READ );
                aStream.Seek ( STREAM_SEEK_TO_BEGIN );
                auto pSet = std::make_unique<SfxItemSet>( *rDoc.GetPool(),
                        svl::Items<SID_PRINTER_NOTFOUND_WARN, SID_PRINTER_NOTFOUND_WARN,
                        SID_PRINTER_CHANGESTODOC,  SID_PRINTER_CHANGESTODOC,
                        SID_PRINT_SELECTEDSHEET,   SID_PRINT_SELECTEDSHEET,
                        SID_SCPRINTOPTIONS,        SID_SCPRINTOPTIONS>{} );

                SfxPrinter* pPrinter = pDocShell->GetPrinter();
                bool bPreferPrinterPapersize = false;
                if ( pPrinter )
                    bPreferPrinterPapersize = pPrinter->GetPrinterSettingsPreferred();

                VclPtr<SfxPrinter> pTempPrinter = SfxPrinter::Create( aStream, std::move(pSet) );
                pTempPrinter->SetPrinterSettingsPreferred( bPreferPrinterPapersize );
                pDocShell->SetPrinter( pTempPrinter );
            }
        }
    }
    else if ( aPropertyName == SC_UNO_PRINTERPAPER )
    {
        bool bPreferPrinterPapersize;
        if( aValue >>= bPreferPrinterPapersize )
        {
            if( pDocShell->GetCreateMode() != SfxObjectCreateMode::EMBEDDED )
            {
                SfxPrinter *pTempPrinter = pDocShell->GetPrinter( true );
                if (pTempPrinter)
                    pTempPrinter->SetPrinterSettingsPreferred( bPreferPrinterPapersize );
            }
        }
    }
    else if ( aPropertyName == SC_UNO_APPLYDOCINF )
    {
        bool bTmp=true;
        if ( aValue >>= bTmp )
            pDocShell->SetUseUserData( bTmp );
    }
    else if ( aPropertyName == SC_UNO_SAVE_THUMBNAIL)
    {
        bool bTmp = true;
        if (aValue >>= bTmp)
            pDocShell->SetUseThumbnailSave( bTmp );
    }
    else if ( aPropertyName == SC_UNO_FORBIDDEN )
    {
        //  read-only - should not be set
    }
    else if ( aPropertyName == SC_UNO_CHARCOMP )
    {
        // Int16 contains CharacterCompressionType values
        sal_Int16 nUno = ScUnoHelpFunctions::GetInt16FromAny( aValue );
        rDoc.SetAsianCompression( static_cast<CharCompressType>(nUno) );
        bUpdateHeights = true;
    }
    else if ( aPropertyName == SC_UNO_ASIANKERN )
    {
        rDoc.SetAsianKerning( ScUnoHelpFunctions::GetBoolFromAny( aValue ) );
        bUpdateHeights = true;
    }
    else if ( aPropertyName == SCSAVEVERSION )
    {
        bool bTmp=false;
        if ( aValue >>= bTmp )
            pDocShell->SetSaveVersionOnClose( bTmp );
    }
    else if ( aPropertyName == SC_UNO_UPDTEMPL )
    {
        bool bTmp=true;
        if ( aValue >>= bTmp )
            pDocShell->SetQueryLoadTemplate( bTmp );
    }
    else if ( aPropertyName == SC_UNO_LOADREADONLY )
    {
        bool bTmp=false;
        if ( aValue >>= bTmp )
            pDocShell->SetLoadReadonly( bTmp );
    }
    else if ( aPropertyName == SC_UNO_SHAREDOC )
    {
#if HAVE_FEATURE_MULTIUSER_ENVIRONMENT
        bool bDocShared = false;
        if ( aValue >>= bDocShared )
        {
            pDocShell->SetSharedXMLFlag( bDocShared );
        }
#endif
    }
    else if ( aPropertyName == SC_UNO_MODIFYPASSWORDINFO )
    {
        uno::Sequence< beans::PropertyValue > aInfo;
        if ( !( aValue >>= aInfo ) )
            throw lang::IllegalArgumentException(
                "Value of type Sequence<PropertyValue> expected!",
                uno::Reference< uno::XInterface >(),
                2 );

        if ( !pDocShell->SetModifyPasswordInfo( aInfo ) )
            throw beans::PropertyVetoException(
                "The hash is not allowed to be changed now!" );
    }
    else if (aPropertyName == SC_UNO_EMBED_FONTS)
    {
        bool bVal = aValue.has<bool>() && aValue.get<bool>();
        rDoc.SetEmbedFonts(bVal);
    }
    else if (aPropertyName == SC_UNO_EMBED_ONLY_USED_FONTS)
    {
        bool bVal = aValue.has<bool>() && aValue.get<bool>();
        rDoc.SetEmbedUsedFontsOnly(bVal);
    }
    else if (aPropertyName == SC_UNO_EMBED_FONT_SCRIPT_LATIN)
    {
        bool bVal = aValue.has<bool>() && aValue.get<bool>();
        rDoc.SetEmbedFontScriptLatin(bVal);
    }
    else if (aPropertyName == SC_UNO_EMBED_FONT_SCRIPT_ASIAN)
    {
        bool bVal = aValue.has<bool>() && aValue.get<bool>();
        rDoc.SetEmbedFontScriptAsian(bVal);
    }
    else if (aPropertyName == SC_UNO_EMBED_FONT_SCRIPT_COMPLEX)
    {
        bool bVal = aValue.has<bool>() && aValue.get<bool>();
        rDoc.SetEmbedFontScriptComplex(bVal);
    }
    else if ( aPropertyName == SC_UNO_SYNTAXSTRINGREF )
    {
        ScCalcConfig aCalcConfig = rDoc.GetCalcConfig();
        sal_Int16 nUno = 0;

        if( aValue >>= nUno )
        {
            switch (nUno)
            {
                case 0: // CONV_OOO
                case 2: // CONV_XL_A1
                case 3: // CONV_XL_R1C1
                case 7: // CONV_A1_XL_A1
                    aCalcConfig.SetStringRefSyntax( static_cast<formula::FormulaGrammar::AddressConvention>( nUno ) );
                    break;
                default:
                    aCalcConfig.SetStringRefSyntax( formula::FormulaGrammar::CONV_UNSPECIFIED );
                    break;

            }
            rDoc.SetCalcConfig( aCalcConfig );
        }
    }

    else
    {
        ScGridOptions aGridOpt(aViewOpt.GetGridOptions());
        if ( aPropertyName == SC_UNO_SNAPTORASTER )
            aGridOpt.SetUseGridSnap( ScUnoHelpFunctions::GetBoolFromAny( aValue ) );
        else if ( aPropertyName == SC_UNO_RASTERVIS )
            aGridOpt.SetGridVisible( ScUnoHelpFunctions::GetBoolFromAny( aValue ) );
        else if ( aPropertyName == SC_UNO_RASTERRESX )
            aGridOpt.SetFieldDrawX( static_cast <sal_uInt32> ( ScUnoHelpFunctions::GetInt32FromAny( aValue ) ) );
        else if ( aPropertyName == SC_UNO_RASTERRESY )
            aGridOpt.SetFieldDrawY( static_cast <sal_uInt32> ( ScUnoHelpFunctions::GetInt32FromAny( aValue ) ) );
        else if ( aPropertyName == SC_UNO_RASTERSUBX )
            aGridOpt.SetFieldDivisionX( static_cast <sal_uInt32> ( ScUnoHelpFunctions::GetInt32FromAny( aValue ) ) );
        else if ( aPropertyName == SC_UNO_RASTERSUBY )
            aGridOpt.SetFieldDivisionY( static_cast <sal_uInt32> ( ScUnoHelpFunctions::GetInt32FromAny( aValue ) ) );
        else if ( aPropertyName == SC_UNO_RASTERSYNC )
            aGridOpt.SetSynchronize( ScUnoHelpFunctions::GetBoolFromAny( aValue ) );
        else
            throw beans::UnknownPropertyException();
        aViewOpt.SetGridOptions(aGridOpt);
    }
    rDoc.SetViewOptions(aViewOpt);

    if ( bUpdateHeights && !rDoc.IsImportingXML() )
    {
        //  update automatic row heights and repaint
        SCTAB nTabCount = rDoc.GetTableCount();
        for (SCTAB nTab=0; nTab<nTabCount; nTab++)
            if ( !pDocShell->AdjustRowHeight( 0, MAXROW, nTab ) )
                pDocShell->PostPaint(ScRange(0, 0, nTab, MAXCOL, MAXROW, nTab), PaintPartFlags::Grid);
        pDocShell->SetDocumentModified();
    }

}

uno::Any SAL_CALL ScDocumentConfiguration::getPropertyValue( const OUString& aPropertyName )
{
    SolarMutexGuard aGuard;
    uno::Any aRet;

    if(!pDocShell)
        throw uno::RuntimeException();

    ScDocument& rDoc = pDocShell->GetDocument();
    const ScViewOptions& aViewOpt = rDoc.GetViewOptions();

    /*Stampit enable/disable print cancel */
    if ( aPropertyName == SC_UNO_ALLOWPRINTJOBCANCEL )
        aRet <<= pDocShell->Stamp_GetPrintCancelState();
    /*Stampit enable/disable print cancel */

    else if ( aPropertyName == SC_UNO_SHOWZERO )
        aRet <<= aViewOpt.GetOption( VOPT_NULLVALS );
    else if ( aPropertyName == SC_UNO_SHOWNOTES )
        aRet <<= aViewOpt.GetOption( VOPT_NOTES );
    else if ( aPropertyName == SC_UNO_SHOWGRID )
        aRet <<= aViewOpt.GetOption( VOPT_GRID );
    else if ( aPropertyName == SC_UNO_GRIDCOLOR )
    {
        OUString aColorName;
        Color aColor = aViewOpt.GetGridColor(&aColorName);
        aRet <<= aColor;
    }
    else if ( aPropertyName == SC_UNO_SHOWPAGEBR )
        aRet <<= aViewOpt.GetOption( VOPT_PAGEBREAKS );
    else if ( aPropertyName == SC_UNONAME_LINKUPD )
    {
        sal_Int16 nLUM;
        switch (rDoc.GetLinkMode())
        {
            case LM_ALWAYS:
                nLUM = css::document::LinkUpdateModes::AUTO;
            break;
            case LM_NEVER:
                nLUM = css::document::LinkUpdateModes::NEVER;
            break;
            case LM_ON_DEMAND:
                nLUM = css::document::LinkUpdateModes::MANUAL;
            break;
            case LM_UNKNOWN:
            default:
                nLUM = css::document::LinkUpdateModes::GLOBAL_SETTING;
            break;
        }
        aRet <<= nLUM;
    }
    else if ( aPropertyName == SC_UNO_COLROWHDR )
        aRet <<= aViewOpt.GetOption( VOPT_HEADER );
    else if ( aPropertyName == SC_UNO_SHEETTABS )
        aRet <<= aViewOpt.GetOption( VOPT_TABCONTROLS );
    else if ( aPropertyName == SC_UNO_OUTLSYMB )
        aRet <<= aViewOpt.GetOption( VOPT_OUTLINER );
    else if ( aPropertyName == SC_UNO_AUTOCALC )
        aRet <<= rDoc.GetAutoCalc();
    else if ( aPropertyName == SC_UNO_PRINTERNAME )
    {
        // #i75610# don't create the printer, return empty string if no printer created yet
        // (as in SwXDocumentSettings)
        SfxPrinter* pPrinter = rDoc.GetPrinter( false );
        if (pPrinter)
            aRet <<= pPrinter->GetName();
        else
            aRet <<= OUString();
    }
    else if ( aPropertyName == SC_UNO_PRINTERSETUP )
    {
        // #i75610# don't create the printer, return empty sequence if no printer created yet
        // (as in SwXDocumentSettings)
        SfxPrinter* pPrinter = rDoc.GetPrinter( false );
        if (pPrinter)
        {
            SvMemoryStream aStream;
            pPrinter->Store( aStream );
            aRet <<= uno::Sequence< sal_Int8 >( static_cast< const sal_Int8* >( aStream.GetData() ),
                                                        aStream.TellEnd() );
        }
        else
            aRet <<= uno::Sequence<sal_Int8>();
    }
    else if ( aPropertyName == SC_UNO_PRINTERPAPER)
    {
        SfxPrinter *pTempPrinter = pDocShell->GetPrinter( false );
        aRet <<= pTempPrinter && pTempPrinter->GetPrinterSettingsPreferred();

    }
    else if ( aPropertyName == SC_UNO_APPLYDOCINF )
        aRet <<= pDocShell->IsUseUserData();
    else if ( aPropertyName == SC_UNO_SAVE_THUMBNAIL )
        aRet <<= pDocShell->IsUseThumbnailSave();
    else if ( aPropertyName == SC_UNO_FORBIDDEN )
    {
        aRet <<= uno::Reference<i18n::XForbiddenCharacters>(new ScForbiddenCharsObj( pDocShell ));
    }
    else if ( aPropertyName == SC_UNO_CHARCOMP )
        aRet <<= static_cast<sal_Int16> ( rDoc.GetAsianCompression() );
    else if ( aPropertyName == SC_UNO_ASIANKERN )
        aRet <<= rDoc.GetAsianKerning();
    else if ( aPropertyName == SCSAVEVERSION )
        aRet <<= pDocShell->IsSaveVersionOnClose();
    else if ( aPropertyName == SC_UNO_UPDTEMPL )
        aRet <<= pDocShell->IsQueryLoadTemplate();
    else if ( aPropertyName == SC_UNO_LOADREADONLY )
        aRet <<= pDocShell->IsLoadReadonly();
    else if ( aPropertyName == SC_UNO_SHAREDOC )
    {
#if HAVE_FEATURE_MULTIUSER_ENVIRONMENT
        aRet <<= pDocShell->HasSharedXMLFlagSet();
#endif
    }
    else if ( aPropertyName == SC_UNO_MODIFYPASSWORDINFO )
        aRet <<= pDocShell->GetModifyPasswordInfo();
    else if (aPropertyName == SC_UNO_EMBED_FONTS)
        aRet <<= rDoc.IsEmbedFonts();
    else if (aPropertyName == SC_UNO_EMBED_ONLY_USED_FONTS)
        aRet <<= rDoc.IsEmbedUsedFontsOnly();
    else if (aPropertyName == SC_UNO_EMBED_FONT_SCRIPT_LATIN)
        aRet <<= rDoc.IsEmbedFontScriptLatin();
    else if (aPropertyName == SC_UNO_EMBED_FONT_SCRIPT_ASIAN)
        aRet <<= rDoc.IsEmbedFontScriptAsian();
    else if (aPropertyName == SC_UNO_EMBED_FONT_SCRIPT_COMPLEX)
        aRet <<= rDoc.IsEmbedFontScriptComplex();
    else if ( aPropertyName == SC_UNO_SYNTAXSTRINGREF )
    {
        ScCalcConfig aCalcConfig = rDoc.GetCalcConfig();
        formula::FormulaGrammar::AddressConvention eConv = aCalcConfig.meStringRefAddressSyntax;

        // don't save "unspecified" string ref syntax ... query formula grammar
        // and save that instead
        if( eConv == formula::FormulaGrammar::CONV_UNSPECIFIED)
        {
            eConv = rDoc.GetAddressConvention();
        }

        // write if it has been read|imported or explicitly changed
        // or if ref syntax isn't what would be native for our file format
        // i.e. CalcA1 in this case
        if ( aCalcConfig.mbHasStringRefSyntax ||
             (eConv != formula::FormulaGrammar::CONV_OOO) )
        {
            switch (eConv)
            {
                case formula::FormulaGrammar::CONV_OOO:
                case formula::FormulaGrammar::CONV_XL_A1:
                case formula::FormulaGrammar::CONV_XL_R1C1:
                case formula::FormulaGrammar::CONV_A1_XL_A1:
                     aRet <<= static_cast<sal_Int16>( eConv );
                     break;

                case formula::FormulaGrammar::CONV_UNSPECIFIED:
                case formula::FormulaGrammar::CONV_ODF:
                case formula::FormulaGrammar::CONV_XL_OOX:
                case formula::FormulaGrammar::CONV_LOTUS_A1:
                case formula::FormulaGrammar::CONV_LAST:
                {
                    aRet <<= sal_Int16(9999);
                    break;
                }
             }
         }
    }

    else
    {
        const ScGridOptions& aGridOpt = aViewOpt.GetGridOptions();
        if ( aPropertyName == SC_UNO_SNAPTORASTER )
            aRet <<= aGridOpt.GetUseGridSnap();
        else if ( aPropertyName == SC_UNO_RASTERVIS )
            aRet <<= aGridOpt.GetGridVisible();
        else if ( aPropertyName == SC_UNO_RASTERRESX )
            aRet <<= static_cast<sal_Int32> ( aGridOpt.GetFieldDrawX() );
        else if ( aPropertyName == SC_UNO_RASTERRESY )
            aRet <<= static_cast<sal_Int32> ( aGridOpt.GetFieldDrawY() );
        else if ( aPropertyName == SC_UNO_RASTERSUBX )
            aRet <<= static_cast<sal_Int32> ( aGridOpt.GetFieldDivisionX() );
        else if ( aPropertyName == SC_UNO_RASTERSUBY )
            aRet <<= static_cast<sal_Int32> ( aGridOpt.GetFieldDivisionY() );
        else if ( aPropertyName == SC_UNO_RASTERSYNC )
            aRet <<= aGridOpt.GetSynchronize();
        else
            throw beans::UnknownPropertyException();
    }


    return aRet;
}

SC_IMPL_DUMMY_PROPERTY_LISTENER( ScDocumentConfiguration )

// XServiceInfo
OUString SAL_CALL ScDocumentConfiguration::getImplementationName()
{
    return OUString( "ScDocumentConfiguration" );
}

sal_Bool SAL_CALL ScDocumentConfiguration::supportsService( const OUString& rServiceName )
{
    return cppu::supportsService(this, rServiceName);
}

uno::Sequence<OUString> SAL_CALL ScDocumentConfiguration::getSupportedServiceNames()
{
    return {"com.sun.star.comp.SpreadsheetSettings",
            "com.sun.star.document.Settings"};
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
