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

#include <config_features.h>

#include "confuno.hxx"
#include "unonames.hxx"
#include "scdll.hxx"
#include "docsh.hxx"
#include "miscuno.hxx"
#include "forbiuno.hxx"
#include "viewopti.hxx"
#include "docpool.hxx"
#include "sc.hrc"

#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <cppuhelper/supportsservice.hxx>
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
        {OUString(SC_UNO_SHOWZERO),     0,  getBooleanCppuType(),              0, 0},
        {OUString(SC_UNO_SHOWNOTES),    0,  getBooleanCppuType(),              0, 0},
        {OUString(SC_UNO_SHOWGRID),     0,  getBooleanCppuType(),              0, 0},
        {OUString(SC_UNO_GRIDCOLOR),    0,  getCppuType((sal_Int32*)0),        0, 0},
        {OUString(SC_UNO_SHOWPAGEBR),   0,  getBooleanCppuType(),              0, 0},
        {OUString(SC_UNONAME_LINKUPD),  0,  getCppuType((sal_Int16*)0),        0, 0},
        {OUString(SC_UNO_COLROWHDR),    0,  getBooleanCppuType(),              0, 0},
        {OUString(SC_UNO_SHEETTABS),    0,  getBooleanCppuType(),              0, 0},
        {OUString(SC_UNO_OUTLSYMB),     0,  getBooleanCppuType(),              0, 0},
        {OUString(SC_UNO_SNAPTORASTER), 0,  getBooleanCppuType(),              0, 0},
        {OUString(SC_UNO_RASTERVIS),    0,  getBooleanCppuType(),              0, 0},
        {OUString(SC_UNO_RASTERRESX),   0,  getCppuType((sal_Int32*)0),        0, 0},
        {OUString(SC_UNO_RASTERRESY),   0,  getCppuType((sal_Int32*)0),        0, 0},
        {OUString(SC_UNO_RASTERSUBX),   0,  getCppuType((sal_Int32*)0),        0, 0},
        {OUString(SC_UNO_RASTERSUBY),   0,  getCppuType((sal_Int32*)0),        0, 0},
        {OUString(SC_UNO_RASTERSYNC),   0,  getBooleanCppuType(),              0, 0},
        {OUString(SC_UNO_AUTOCALC),     0,  getBooleanCppuType(),              0, 0},
        {OUString(SC_UNO_PRINTERNAME),  0,  getCppuType((OUString*)0),    0, 0},
        {OUString(SC_UNO_PRINTERSETUP), 0,  getCppuType((uno::Sequence<sal_Int8>*)0), 0, 0},
        {OUString(SC_UNO_APPLYDOCINF),  0,  getBooleanCppuType(),              0, 0},
        {OUString(SC_UNO_FORBIDDEN),    0,  getCppuType((uno::Reference<i18n::XForbiddenCharacters>*)0), beans::PropertyAttribute::READONLY, 0},
        {OUString(SC_UNO_CHARCOMP),     0,  getCppuType((sal_Int16*)0),        0, 0},
        {OUString(SC_UNO_ASIANKERN),    0,  getBooleanCppuType(),              0, 0},
        {OUString(SCSAVEVERSION),       0,  getBooleanCppuType(),              0, 0},
        {OUString(SC_UNO_UPDTEMPL),     0,  getBooleanCppuType(),              0, 0},
        /*Stampit enable/disable print cancel */
        {OUString(SC_UNO_ALLOWPRINTJOBCANCEL), 0, getBooleanCppuType(),        0, 0},
        {OUString(SC_UNO_LOADREADONLY), 0,  getBooleanCppuType(),              0, 0},
        {OUString(SC_UNO_SHAREDOC),     0,  getBooleanCppuType(),              0, 0},
        {OUString(SC_UNO_MODIFYPASSWORDINFO), 0,  getCppuType((uno::Sequence< beans::PropertyValue >*)0),              0, 0},
        {OUString(SC_UNO_EMBED_FONTS), 0,  getBooleanCppuType(),              0, 0},
        { OUString(), 0, css::uno::Type(), 0, 0 }
    };
    return aConfigPropertyMap_Impl;
}

ScDocumentConfiguration::ScDocumentConfiguration(ScDocShell* pDocSh)
    : pDocShell(pDocSh) ,
    aPropSet ( lcl_GetConfigPropertyMap() )
{
    pDocShell->GetDocument()->AddUnoObject(*this);
}

ScDocumentConfiguration::~ScDocumentConfiguration()
{
    if (pDocShell)
        pDocShell->GetDocument()->RemoveUnoObject(*this);
}

void ScDocumentConfiguration::Notify( SfxBroadcaster&, const SfxHint& rHint )
{
    //  Referenz-Update interessiert hier nicht

    if ( rHint.ISA( SfxSimpleHint ) &&
            ((const SfxSimpleHint&)rHint).GetId() == SFX_HINT_DYING )
    {
        pDocShell = NULL;       // ungueltig geworden
    }
}

// XPropertySet

uno::Reference<beans::XPropertySetInfo> SAL_CALL ScDocumentConfiguration::getPropertySetInfo()
                                                        throw(uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    static uno::Reference<beans::XPropertySetInfo> aRef(
        new SfxItemPropertySetInfo( aPropSet.getPropertyMap() ));
    return aRef;
}

void SAL_CALL ScDocumentConfiguration::setPropertyValue(
                        const OUString& aPropertyName, const uno::Any& aValue )
                throw(beans::UnknownPropertyException, beans::PropertyVetoException,
                        lang::IllegalArgumentException, lang::WrappedTargetException,
                        uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;

    if(pDocShell)
    {
        ScDocument* pDoc = pDocShell->GetDocument();
        if (pDoc)
        {
            sal_Bool bUpdateHeights = false;

            ScViewOptions aViewOpt(pDoc->GetViewOptions());

            /*Stampit enable/disable print cancel */
            if ( aPropertyName.equalsAscii( SC_UNO_ALLOWPRINTJOBCANCEL ) )
                pDocShell->Stamp_SetPrintCancelState( ScUnoHelpFunctions::GetBoolFromAny( aValue ) );
            /*Stampit enable/disable print cancel */

            else if ( aPropertyName.equalsAscii( SC_UNO_SHOWZERO ) )
                aViewOpt.SetOption(VOPT_NULLVALS, ScUnoHelpFunctions::GetBoolFromAny( aValue ) );
            else if ( aPropertyName.equalsAscii( SC_UNO_SHOWNOTES ) )
                aViewOpt.SetOption(VOPT_NOTES, ScUnoHelpFunctions::GetBoolFromAny( aValue ) );
            else if ( aPropertyName.equalsAscii( SC_UNO_SHOWGRID ) )
                aViewOpt.SetOption(VOPT_GRID, ScUnoHelpFunctions::GetBoolFromAny( aValue ) );
            else if ( aPropertyName.equalsAscii( SC_UNO_GRIDCOLOR ) )
            {
                sal_Int64 nColor = 0;
                if (aValue >>= nColor)
                {
                    OUString aColorName;
                    Color aColor(static_cast<sal_uInt32>(nColor));
                    aViewOpt.SetGridColor(aColor, aColorName);
                }
            }
            else if ( aPropertyName.equalsAscii( SC_UNO_SHOWPAGEBR ) )
                aViewOpt.SetOption(VOPT_PAGEBREAKS, ScUnoHelpFunctions::GetBoolFromAny( aValue ) );
            else if ( aPropertyName.equalsAscii( SC_UNONAME_LINKUPD ) )
                pDoc->SetLinkMode( static_cast<ScLkUpdMode> ( ScUnoHelpFunctions::GetInt16FromAny( aValue ) ) );
            else if ( aPropertyName.equalsAscii( SC_UNO_COLROWHDR ) )
                aViewOpt.SetOption(VOPT_HEADER, ScUnoHelpFunctions::GetBoolFromAny( aValue ) );
            else if ( aPropertyName.equalsAscii( SC_UNO_SHEETTABS ) )
                aViewOpt.SetOption(VOPT_TABCONTROLS, ScUnoHelpFunctions::GetBoolFromAny( aValue ) );
            else if ( aPropertyName.equalsAscii( SC_UNO_OUTLSYMB ) )
                aViewOpt.SetOption(VOPT_OUTLINER, ScUnoHelpFunctions::GetBoolFromAny( aValue ) );
            else if ( aPropertyName.equalsAscii( SC_UNO_AUTOCALC ) )
                pDoc->SetAutoCalc( ScUnoHelpFunctions::GetBoolFromAny( aValue ) );
            else if ( aPropertyName.equalsAscii( SC_UNO_PRINTERNAME ) )
            {
                OUString sPrinterName;
                if ( aValue >>= sPrinterName )
                {
                    // #i75610# if the name is empty, do nothing (don't create any printer)
                    if ( !sPrinterName.isEmpty() && pDocShell->GetCreateMode() != SFX_CREATE_MODE_EMBEDDED )
                    {
                        SfxPrinter* pPrinter = pDocShell->GetPrinter();
                        if (pPrinter)
                        {
                            if (pPrinter->GetName() != sPrinterName)
                            {
                                SfxPrinter* pNewPrinter = new SfxPrinter( pPrinter->GetOptions().Clone(), sPrinterName );
                                if (pNewPrinter->IsKnown())
                                    pDocShell->SetPrinter( pNewPrinter, SFX_PRINTER_PRINTER );
                                else
                                    delete pNewPrinter;
                            }
                        }
                        else
                            throw uno::RuntimeException();
                    }
                }
                else
                    throw lang::IllegalArgumentException();
            }
            else if ( aPropertyName.equalsAscii( SC_UNO_PRINTERSETUP ) )
            {
                uno::Sequence<sal_Int8> aSequence;
                if ( aValue >>= aSequence )
                {
                    sal_uInt32 nSize = aSequence.getLength();
                    // #i75610# if the sequence is empty, do nothing (don't create any printer)
                    if ( nSize != 0 )
                    {
                        SvMemoryStream aStream (aSequence.getArray(), nSize, STREAM_READ );
                        aStream.Seek ( STREAM_SEEK_TO_BEGIN );
                        SfxItemSet* pSet = new SfxItemSet( *pDoc->GetPool(),
                                SID_PRINTER_NOTFOUND_WARN, SID_PRINTER_NOTFOUND_WARN,
                                SID_PRINTER_CHANGESTODOC,  SID_PRINTER_CHANGESTODOC,
                                SID_PRINT_SELECTEDSHEET,   SID_PRINT_SELECTEDSHEET,
                                SID_SCPRINTOPTIONS,        SID_SCPRINTOPTIONS,
                                NULL );
                        pDocShell->SetPrinter( SfxPrinter::Create( aStream, pSet ) );
                    }
                }
            }
            else if ( aPropertyName.equalsAscii( SC_UNO_APPLYDOCINF ) )
            {
                sal_Bool bTmp=sal_True;
                if ( aValue >>= bTmp )
                    pDocShell->SetUseUserData( bTmp );
            }
            else if ( aPropertyName.equalsAscii( SC_UNO_FORBIDDEN ) )
            {
                //  read-only - should not be set
            }
            else if ( aPropertyName.equalsAscii( SC_UNO_CHARCOMP ) )
            {
                // Int16 contains CharacterCompressionType values
                sal_Int16 nUno = ScUnoHelpFunctions::GetInt16FromAny( aValue );
                pDoc->SetAsianCompression( (sal_uInt8) nUno );
                bUpdateHeights = sal_True;
            }
            else if ( aPropertyName.equalsAscii( SC_UNO_ASIANKERN ) )
            {
                pDoc->SetAsianKerning( ScUnoHelpFunctions::GetBoolFromAny( aValue ) );
                bUpdateHeights = sal_True;
            }
            else if ( aPropertyName.equalsAscii( SCSAVEVERSION ) )
            {
                sal_Bool bTmp=false;
                if ( aValue >>= bTmp )
                    pDocShell->SetSaveVersionOnClose( bTmp );
            }
            else if ( aPropertyName.equalsAscii( SC_UNO_UPDTEMPL ) )
            {
                sal_Bool bTmp=sal_True;
                if ( aValue >>= bTmp )
                    pDocShell->SetQueryLoadTemplate( bTmp );
            }
            else if ( aPropertyName.equalsAscii( SC_UNO_LOADREADONLY ) )
            {
                sal_Bool bTmp=false;
                if ( aValue >>= bTmp )
                    pDocShell->SetLoadReadonly( bTmp );
            }
            else if ( aPropertyName.equalsAscii( SC_UNO_SHAREDOC ) )
            {
#if HAVE_FEATURE_MULTIUSER_ENVIRONMENT
                sal_Bool bDocShared = false;
                if ( aValue >>= bDocShared )
                {
                    pDocShell->SetSharedXMLFlag( bDocShared );
                }
#endif
            }
            else if ( aPropertyName.equalsAscii( SC_UNO_MODIFYPASSWORDINFO ) )
            {
                uno::Sequence< beans::PropertyValue > aInfo;
                if ( !( aValue >>= aInfo ) )
                    throw lang::IllegalArgumentException(
                        OUString( "Value of type Sequence<PropertyValue> expected!" ),
                        uno::Reference< uno::XInterface >(),
                        2 );

                if ( !pDocShell->SetModifyPasswordInfo( aInfo ) )
                    throw beans::PropertyVetoException(
                        OUString( "The hash is not allowed to be changed now!" ),
                        uno::Reference< uno::XInterface >() );
            }
            else if ( aPropertyName.equalsAscii( SC_UNO_EMBED_FONTS ) )
            {
                sal_Bool bVal = sal_False;
                if ( aValue >>=bVal )
                {
                    pDoc->SetIsUsingEmbededFonts(bVal);
                }
            }

            else
            {
                ScGridOptions aGridOpt(aViewOpt.GetGridOptions());
                if ( aPropertyName.equalsAscii( SC_UNO_SNAPTORASTER ) )
                    aGridOpt.SetUseGridSnap( ScUnoHelpFunctions::GetBoolFromAny( aValue ) );
                else if ( aPropertyName.equalsAscii( SC_UNO_RASTERVIS ) )
                    aGridOpt.SetGridVisible( ScUnoHelpFunctions::GetBoolFromAny( aValue ) );
                else if ( aPropertyName.equalsAscii( SC_UNO_RASTERRESX ) )
                    aGridOpt.SetFldDrawX( static_cast <sal_uInt32> ( ScUnoHelpFunctions::GetInt32FromAny( aValue ) ) );
                else if ( aPropertyName.equalsAscii( SC_UNO_RASTERRESY ) )
                    aGridOpt.SetFldDrawY( static_cast <sal_uInt32> ( ScUnoHelpFunctions::GetInt32FromAny( aValue ) ) );
                else if ( aPropertyName.equalsAscii( SC_UNO_RASTERSUBX ) )
                    aGridOpt.SetFldDivisionX( static_cast <sal_uInt32> ( ScUnoHelpFunctions::GetInt32FromAny( aValue ) ) );
                else if ( aPropertyName.equalsAscii( SC_UNO_RASTERSUBY ) )
                    aGridOpt.SetFldDivisionY( static_cast <sal_uInt32> ( ScUnoHelpFunctions::GetInt32FromAny( aValue ) ) );
                else if ( aPropertyName.equalsAscii( SC_UNO_RASTERSYNC ) )
                    aGridOpt.SetSynchronize( ScUnoHelpFunctions::GetBoolFromAny( aValue ) );
                else
                    throw beans::UnknownPropertyException();
                aViewOpt.SetGridOptions(aGridOpt);
            }
            pDoc->SetViewOptions(aViewOpt);

            if ( bUpdateHeights && !pDoc->IsImportingXML() )
            {
                //  update automatic row heights and repaint
                SCTAB nTabCount = pDoc->GetTableCount();
                for (SCTAB nTab=0; nTab<nTabCount; nTab++)
                    if ( !pDocShell->AdjustRowHeight( 0, MAXROW, nTab ) )
                        pDocShell->PostPaint(ScRange(0, 0, nTab, MAXCOL, MAXROW, nTab), PAINT_GRID);
                pDocShell->SetDocumentModified();
            }
        }
        else
            throw uno::RuntimeException();
    }
    else
        throw uno::RuntimeException();
}

uno::Any SAL_CALL ScDocumentConfiguration::getPropertyValue( const OUString& aPropertyName )
                throw(beans::UnknownPropertyException, lang::WrappedTargetException,
                        uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    uno::Any aRet;

    if(pDocShell)
    {
        ScDocument* pDoc = pDocShell->GetDocument();
        if (pDoc)
        {
            const ScViewOptions& aViewOpt = pDoc->GetViewOptions();

            /*Stampit enable/disable print cancel */
            if ( aPropertyName.equalsAscii( SC_UNO_ALLOWPRINTJOBCANCEL ) )
                ScUnoHelpFunctions::SetBoolInAny( aRet, pDocShell->Stamp_GetPrintCancelState() );
            /*Stampit enable/disable print cancel */

            else if ( aPropertyName.equalsAscii( SC_UNO_SHOWZERO ) )
                ScUnoHelpFunctions::SetBoolInAny( aRet, aViewOpt.GetOption( VOPT_NULLVALS ) );
            else if ( aPropertyName.equalsAscii( SC_UNO_SHOWNOTES ) )
                ScUnoHelpFunctions::SetBoolInAny( aRet, aViewOpt.GetOption( VOPT_NOTES ) );
            else if ( aPropertyName.equalsAscii( SC_UNO_SHOWGRID ) )
                ScUnoHelpFunctions::SetBoolInAny( aRet, aViewOpt.GetOption( VOPT_GRID ) );
            else if ( aPropertyName.equalsAscii( SC_UNO_GRIDCOLOR ) )
            {
                OUString aColorName;
                Color aColor = aViewOpt.GetGridColor(&aColorName);
                aRet <<= static_cast<sal_Int64>(aColor.GetColor());
            }
            else if ( aPropertyName.equalsAscii( SC_UNO_SHOWPAGEBR ) )
                ScUnoHelpFunctions::SetBoolInAny( aRet, aViewOpt.GetOption( VOPT_PAGEBREAKS ) );
            else if ( aPropertyName.equalsAscii( SC_UNONAME_LINKUPD ) )
                aRet <<= static_cast<sal_Int16> ( pDoc->GetLinkMode() );
            else if ( aPropertyName.equalsAscii( SC_UNO_COLROWHDR ) )
                ScUnoHelpFunctions::SetBoolInAny( aRet, aViewOpt.GetOption( VOPT_HEADER ) );
            else if ( aPropertyName.equalsAscii( SC_UNO_SHEETTABS ) )
                ScUnoHelpFunctions::SetBoolInAny( aRet, aViewOpt.GetOption( VOPT_TABCONTROLS ) );
            else if ( aPropertyName.equalsAscii( SC_UNO_OUTLSYMB ) )
                ScUnoHelpFunctions::SetBoolInAny( aRet, aViewOpt.GetOption( VOPT_OUTLINER ) );
            else if ( aPropertyName.equalsAscii( SC_UNO_AUTOCALC ) )
                ScUnoHelpFunctions::SetBoolInAny( aRet, pDoc->GetAutoCalc() );
            else if ( aPropertyName.equalsAscii( SC_UNO_PRINTERNAME ) )
            {
                // #i75610# don't create the printer, return empty string if no printer created yet
                // (as in SwXDocumentSettings)
                SfxPrinter* pPrinter = pDoc->GetPrinter( false );
                if (pPrinter)
                    aRet <<= OUString ( pPrinter->GetName());
                else
                    aRet <<= OUString();
            }
            else if ( aPropertyName.equalsAscii( SC_UNO_PRINTERSETUP ) )
            {
                // #i75610# don't create the printer, return empty sequence if no printer created yet
                // (as in SwXDocumentSettings)
                SfxPrinter* pPrinter = pDoc->GetPrinter( false );
                if (pPrinter)
                {
                    SvMemoryStream aStream;
                    pPrinter->Store( aStream );
                    aStream.Seek ( STREAM_SEEK_TO_END );
                    sal_uInt32 nSize = aStream.Tell();
                    aStream.Seek ( STREAM_SEEK_TO_BEGIN );
                    uno::Sequence < sal_Int8 > aSequence( nSize );
                    aStream.Read ( aSequence.getArray(), nSize );
                    aRet <<= aSequence;
                }
                else
                    aRet <<= uno::Sequence<sal_Int8>();
            }
            else if ( aPropertyName.equalsAscii( SC_UNO_APPLYDOCINF ) )
                aRet <<= pDocShell->IsUseUserData();
            else if ( aPropertyName.equalsAscii( SC_UNO_FORBIDDEN ) )
            {
                aRet <<= uno::Reference<i18n::XForbiddenCharacters>(new ScForbiddenCharsObj( pDocShell ));
            }
            else if ( aPropertyName.equalsAscii( SC_UNO_CHARCOMP ) )
                aRet <<= static_cast<sal_Int16> ( pDoc->GetAsianCompression() );
            else if ( aPropertyName.equalsAscii( SC_UNO_ASIANKERN ) )
                ScUnoHelpFunctions::SetBoolInAny( aRet, pDoc->GetAsianKerning() );
            else if ( aPropertyName.equalsAscii( SCSAVEVERSION ) )
                aRet <<= pDocShell->IsSaveVersionOnClose();
            else if ( aPropertyName.equalsAscii( SC_UNO_UPDTEMPL ) )
                aRet <<= pDocShell->IsQueryLoadTemplate();
            else if ( aPropertyName.equalsAscii( SC_UNO_LOADREADONLY ) )
                aRet <<= pDocShell->IsLoadReadonly();
            else if ( aPropertyName.equalsAscii( SC_UNO_SHAREDOC ) )
            {
#if HAVE_FEATURE_MULTIUSER_ENVIRONMENT
                ScUnoHelpFunctions::SetBoolInAny( aRet, pDocShell->HasSharedXMLFlagSet() );
#endif
            }
            else if ( aPropertyName.equalsAscii( SC_UNO_MODIFYPASSWORDINFO ) )
                aRet <<= pDocShell->GetModifyPasswordInfo();
            else if ( aPropertyName.equalsAscii( SC_UNO_EMBED_FONTS ) )
            {
                aRet <<= pDoc->IsUsingEmbededFonts();
            }

            else
            {
                const ScGridOptions& aGridOpt = aViewOpt.GetGridOptions();
                if ( aPropertyName.equalsAscii( SC_UNO_SNAPTORASTER ) )
                    ScUnoHelpFunctions::SetBoolInAny( aRet, aGridOpt.GetUseGridSnap() );
                else if ( aPropertyName.equalsAscii( SC_UNO_RASTERVIS ) )
                    ScUnoHelpFunctions::SetBoolInAny( aRet, aGridOpt.GetGridVisible() );
                else if ( aPropertyName.equalsAscii( SC_UNO_RASTERRESX ) )
                    aRet <<= static_cast<sal_Int32> ( aGridOpt.GetFldDrawX() );
                else if ( aPropertyName.equalsAscii( SC_UNO_RASTERRESY ) )
                    aRet <<= static_cast<sal_Int32> ( aGridOpt.GetFldDrawY() );
                else if ( aPropertyName.equalsAscii( SC_UNO_RASTERSUBX ) )
                    aRet <<= static_cast<sal_Int32> ( aGridOpt.GetFldDivisionX() );
                else if ( aPropertyName.equalsAscii( SC_UNO_RASTERSUBY ) )
                    aRet <<= static_cast<sal_Int32> ( aGridOpt.GetFldDivisionY() );
                else if ( aPropertyName.equalsAscii( SC_UNO_RASTERSYNC ) )
                    ScUnoHelpFunctions::SetBoolInAny( aRet, aGridOpt.GetSynchronize() );
                else
                    throw beans::UnknownPropertyException();
            }
        }
        else
            throw uno::RuntimeException();
    }
    else
        throw uno::RuntimeException();

    return aRet;
}

SC_IMPL_DUMMY_PROPERTY_LISTENER( ScDocumentConfiguration )

// XServiceInfo
OUString SAL_CALL ScDocumentConfiguration::getImplementationName() throw(uno::RuntimeException, std::exception)
{
    return OUString( "ScDocumentConfiguration" );
}

sal_Bool SAL_CALL ScDocumentConfiguration::supportsService( const OUString& rServiceName )
                                                    throw(uno::RuntimeException, std::exception)
{
    return cppu::supportsService(this, rServiceName);
}

uno::Sequence<OUString> SAL_CALL ScDocumentConfiguration::getSupportedServiceNames()
                                                    throw(uno::RuntimeException, std::exception)
{
    uno::Sequence<OUString> aRet(2);
    OUString* pArray = aRet.getArray();
    pArray[0] = "com.sun.star.comp.SpreadsheetSettings";
    pArray[1] = "com.sun.star.document.Settings";
    return aRet;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
