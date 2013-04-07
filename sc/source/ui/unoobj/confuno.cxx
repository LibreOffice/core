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
#include <sfx2/printer.hxx>
#include <xmloff/xmluconv.hxx>
#include <rtl/ustrbuf.hxx>
#include <vcl/svapp.hxx>

using namespace com::sun::star;

#define SCCOMPSCPREADSHEETSETTINGS_SERVICE      "com.sun.star.comp.SpreadsheetSettings"
#define SCDOCUMENTSETTINGS_SERVICE              "com.sun.star.document.Settings"
#define SCSAVEVERSION                           "SaveVersionOnClose"


static const SfxItemPropertyMapEntry* lcl_GetConfigPropertyMap()
{
    static SfxItemPropertyMapEntry aConfigPropertyMap_Impl[] =
    {
        {MAP_CHAR_LEN(SC_UNO_SHOWZERO),     0,  &getBooleanCppuType(),              0, 0},
        {MAP_CHAR_LEN(SC_UNO_SHOWNOTES),    0,  &getBooleanCppuType(),              0, 0},
        {MAP_CHAR_LEN(SC_UNO_SHOWGRID),     0,  &getBooleanCppuType(),              0, 0},
        {MAP_CHAR_LEN(SC_UNO_GRIDCOLOR),    0,  &getCppuType((sal_Int32*)0),        0, 0},
        {MAP_CHAR_LEN(SC_UNO_SHOWPAGEBR),   0,  &getBooleanCppuType(),              0, 0},
        {MAP_CHAR_LEN(SC_UNONAME_LINKUPD),  0,  &getCppuType((sal_Int16*)0),        0, 0},
        {MAP_CHAR_LEN(SC_UNO_COLROWHDR),    0,  &getBooleanCppuType(),              0, 0},
        {MAP_CHAR_LEN(SC_UNO_SHEETTABS),    0,  &getBooleanCppuType(),              0, 0},
        {MAP_CHAR_LEN(SC_UNO_OUTLSYMB),     0,  &getBooleanCppuType(),              0, 0},
        {MAP_CHAR_LEN(SC_UNO_SNAPTORASTER), 0,  &getBooleanCppuType(),              0, 0},
        {MAP_CHAR_LEN(SC_UNO_RASTERVIS),    0,  &getBooleanCppuType(),              0, 0},
        {MAP_CHAR_LEN(SC_UNO_RASTERRESX),   0,  &getCppuType((sal_Int32*)0),        0, 0},
        {MAP_CHAR_LEN(SC_UNO_RASTERRESY),   0,  &getCppuType((sal_Int32*)0),        0, 0},
        {MAP_CHAR_LEN(SC_UNO_RASTERSUBX),   0,  &getCppuType((sal_Int32*)0),        0, 0},
        {MAP_CHAR_LEN(SC_UNO_RASTERSUBY),   0,  &getCppuType((sal_Int32*)0),        0, 0},
        {MAP_CHAR_LEN(SC_UNO_RASTERSYNC),   0,  &getBooleanCppuType(),              0, 0},
        {MAP_CHAR_LEN(SC_UNO_AUTOCALC),     0,  &getBooleanCppuType(),              0, 0},
        {MAP_CHAR_LEN(SC_UNO_PRINTERNAME),  0,  &getCppuType((OUString*)0),    0, 0},
        {MAP_CHAR_LEN(SC_UNO_PRINTERSETUP), 0,  &getCppuType((uno::Sequence<sal_Int8>*)0), 0, 0},
        {MAP_CHAR_LEN(SC_UNO_APPLYDOCINF),  0,  &getBooleanCppuType(),              0, 0},
        {MAP_CHAR_LEN(SC_UNO_FORBIDDEN),    0,  &getCppuType((uno::Reference<i18n::XForbiddenCharacters>*)0), beans::PropertyAttribute::READONLY, 0},
        {MAP_CHAR_LEN(SC_UNO_CHARCOMP),     0,  &getCppuType((sal_Int16*)0),        0, 0},
        {MAP_CHAR_LEN(SC_UNO_ASIANKERN),    0,  &getBooleanCppuType(),              0, 0},
        {MAP_CHAR_LEN(SCSAVEVERSION),       0,  &getBooleanCppuType(),              0, 0},
        {MAP_CHAR_LEN(SC_UNO_UPDTEMPL),     0,  &getBooleanCppuType(),              0, 0},
        /*Stampit enable/disable print cancel */
        {MAP_CHAR_LEN(SC_UNO_ALLOWPRINTJOBCANCEL), 0, &getBooleanCppuType(),        0, 0},
        {MAP_CHAR_LEN(SC_UNO_LOADREADONLY), 0,  &getBooleanCppuType(),              0, 0},
        {MAP_CHAR_LEN(SC_UNO_SHAREDOC),     0,  &getBooleanCppuType(),              0, 0},
        {MAP_CHAR_LEN(SC_UNO_MODIFYPASSWORDINFO), 0,  &getCppuType((uno::Sequence< beans::PropertyValue >*)0),              0, 0},
        {MAP_CHAR_LEN(SC_UNO_EMBED_FONTS), 0,  &getBooleanCppuType(),              0, 0},
        {0,0,0,0,0,0}
    };
    return aConfigPropertyMap_Impl;
}

//------------------------------------------------------------------------

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
                                                        throw(uno::RuntimeException)
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
                        uno::RuntimeException)
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
            if ( aPropertyName.compareToAscii( SC_UNO_ALLOWPRINTJOBCANCEL ) == 0 )
                pDocShell->Stamp_SetPrintCancelState( ScUnoHelpFunctions::GetBoolFromAny( aValue ) );
            /*Stampit enable/disable print cancel */

            else if ( aPropertyName.compareToAscii( SC_UNO_SHOWZERO ) == 0 )
                aViewOpt.SetOption(VOPT_NULLVALS, ScUnoHelpFunctions::GetBoolFromAny( aValue ) );
            else if ( aPropertyName.compareToAscii( SC_UNO_SHOWNOTES ) == 0 )
                aViewOpt.SetOption(VOPT_NOTES, ScUnoHelpFunctions::GetBoolFromAny( aValue ) );
            else if ( aPropertyName.compareToAscii( SC_UNO_SHOWGRID ) == 0 )
                aViewOpt.SetOption(VOPT_GRID, ScUnoHelpFunctions::GetBoolFromAny( aValue ) );
            else if ( aPropertyName.compareToAscii( SC_UNO_GRIDCOLOR ) == 0 )
            {
                sal_Int64 nColor = 0;
                if (aValue >>= nColor)
                {
                    String aColorName;
                    Color aColor(static_cast<sal_uInt32>(nColor));
                    aViewOpt.SetGridColor(aColor, aColorName);
                }
            }
            else if ( aPropertyName.compareToAscii( SC_UNO_SHOWPAGEBR ) == 0 )
                aViewOpt.SetOption(VOPT_PAGEBREAKS, ScUnoHelpFunctions::GetBoolFromAny( aValue ) );
            else if ( aPropertyName.compareToAscii( SC_UNONAME_LINKUPD ) == 0 )
                pDoc->SetLinkMode( static_cast<ScLkUpdMode> ( ScUnoHelpFunctions::GetInt16FromAny( aValue ) ) );
            else if ( aPropertyName.compareToAscii( SC_UNO_COLROWHDR ) == 0 )
                aViewOpt.SetOption(VOPT_HEADER, ScUnoHelpFunctions::GetBoolFromAny( aValue ) );
            else if ( aPropertyName.compareToAscii( SC_UNO_SHEETTABS ) == 0 )
                aViewOpt.SetOption(VOPT_TABCONTROLS, ScUnoHelpFunctions::GetBoolFromAny( aValue ) );
            else if ( aPropertyName.compareToAscii( SC_UNO_OUTLSYMB ) == 0 )
                aViewOpt.SetOption(VOPT_OUTLINER, ScUnoHelpFunctions::GetBoolFromAny( aValue ) );
            else if ( aPropertyName.compareToAscii( SC_UNO_AUTOCALC ) == 0 )
                pDoc->SetAutoCalc( ScUnoHelpFunctions::GetBoolFromAny( aValue ) );
            else if ( aPropertyName.compareToAscii( SC_UNO_PRINTERNAME ) == 0 )
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
            else if ( aPropertyName.compareToAscii( SC_UNO_PRINTERSETUP ) == 0 )
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
            else if ( aPropertyName.compareToAscii( SC_UNO_APPLYDOCINF ) == 0 )
            {
                sal_Bool bTmp=sal_True;
                if ( aValue >>= bTmp )
                    pDocShell->SetUseUserData( bTmp );
            }
            else if ( aPropertyName.compareToAscii( SC_UNO_FORBIDDEN ) == 0 )
            {
                //  read-only - should not be set
            }
            else if ( aPropertyName.compareToAscii( SC_UNO_CHARCOMP ) == 0 )
            {
                // Int16 contains CharacterCompressionType values
                sal_Int16 nUno = ScUnoHelpFunctions::GetInt16FromAny( aValue );
                pDoc->SetAsianCompression( (sal_uInt8) nUno );
                bUpdateHeights = sal_True;
            }
            else if ( aPropertyName.compareToAscii( SC_UNO_ASIANKERN ) == 0 )
            {
                pDoc->SetAsianKerning( ScUnoHelpFunctions::GetBoolFromAny( aValue ) );
                bUpdateHeights = sal_True;
            }
            else if ( aPropertyName.compareToAscii( SCSAVEVERSION ) == 0)
            {
                sal_Bool bTmp=false;
                if ( aValue >>= bTmp )
                    pDocShell->SetSaveVersionOnClose( bTmp );
            }
            else if ( aPropertyName.compareToAscii( SC_UNO_UPDTEMPL ) == 0 )
            {
                sal_Bool bTmp=sal_True;
                if ( aValue >>= bTmp )
                    pDocShell->SetQueryLoadTemplate( bTmp );
            }
            else if ( aPropertyName.compareToAscii( SC_UNO_LOADREADONLY ) == 0 )
            {
                sal_Bool bTmp=false;
                if ( aValue >>= bTmp )
                    pDocShell->SetLoadReadonly( bTmp );
            }
            else if ( aPropertyName.compareToAscii( SC_UNO_SHAREDOC ) == 0 )
            {
#if HAVE_FEATURE_MULTIUSER_ENVIRONMENT
                sal_Bool bDocShared = false;
                if ( aValue >>= bDocShared )
                {
                    pDocShell->SetSharedXMLFlag( bDocShared );
                }
#endif
            }
            else if ( aPropertyName.compareToAscii( SC_UNO_MODIFYPASSWORDINFO ) == 0 )
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
            else if ( aPropertyName.compareToAscii( SC_UNO_EMBED_FONTS ) == 0 )
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
                if ( aPropertyName.compareToAscii( SC_UNO_SNAPTORASTER ) == 0 )
                    aGridOpt.SetUseGridSnap( ScUnoHelpFunctions::GetBoolFromAny( aValue ) );
                else if ( aPropertyName.compareToAscii( SC_UNO_RASTERVIS ) == 0 )
                    aGridOpt.SetGridVisible( ScUnoHelpFunctions::GetBoolFromAny( aValue ) );
                else if ( aPropertyName.compareToAscii( SC_UNO_RASTERRESX ) == 0 )
                    aGridOpt.SetFldDrawX( static_cast <sal_uInt32> ( ScUnoHelpFunctions::GetInt32FromAny( aValue ) ) );
                else if ( aPropertyName.compareToAscii( SC_UNO_RASTERRESY ) == 0 )
                    aGridOpt.SetFldDrawY( static_cast <sal_uInt32> ( ScUnoHelpFunctions::GetInt32FromAny( aValue ) ) );
                else if ( aPropertyName.compareToAscii( SC_UNO_RASTERSUBX ) == 0 )
                    aGridOpt.SetFldDivisionX( static_cast <sal_uInt32> ( ScUnoHelpFunctions::GetInt32FromAny( aValue ) ) );
                else if ( aPropertyName.compareToAscii( SC_UNO_RASTERSUBY ) == 0 )
                    aGridOpt.SetFldDivisionY( static_cast <sal_uInt32> ( ScUnoHelpFunctions::GetInt32FromAny( aValue ) ) );
                else if ( aPropertyName.compareToAscii( SC_UNO_RASTERSYNC ) == 0 )
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
                        uno::RuntimeException)
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
            if ( aPropertyName.compareToAscii( SC_UNO_ALLOWPRINTJOBCANCEL ) == 0 )
                ScUnoHelpFunctions::SetBoolInAny( aRet, pDocShell->Stamp_GetPrintCancelState() );
            /*Stampit enable/disable print cancel */

            else if ( aPropertyName.compareToAscii( SC_UNO_SHOWZERO ) == 0 )
                ScUnoHelpFunctions::SetBoolInAny( aRet, aViewOpt.GetOption( VOPT_NULLVALS ) );
            else if ( aPropertyName.compareToAscii( SC_UNO_SHOWNOTES ) == 0 )
                ScUnoHelpFunctions::SetBoolInAny( aRet, aViewOpt.GetOption( VOPT_NOTES ) );
            else if ( aPropertyName.compareToAscii( SC_UNO_SHOWGRID ) == 0 )
                ScUnoHelpFunctions::SetBoolInAny( aRet, aViewOpt.GetOption( VOPT_GRID ) );
            else if ( aPropertyName.compareToAscii( SC_UNO_GRIDCOLOR ) == 0 )
            {
                String aColorName;
                Color aColor = aViewOpt.GetGridColor(&aColorName);
                aRet <<= static_cast<sal_Int64>(aColor.GetColor());
            }
            else if ( aPropertyName.compareToAscii( SC_UNO_SHOWPAGEBR ) == 0 )
                ScUnoHelpFunctions::SetBoolInAny( aRet, aViewOpt.GetOption( VOPT_PAGEBREAKS ) );
            else if ( aPropertyName.compareToAscii( SC_UNONAME_LINKUPD ) == 0 )
                aRet <<= static_cast<sal_Int16> ( pDoc->GetLinkMode() );
            else if ( aPropertyName.compareToAscii( SC_UNO_COLROWHDR ) == 0 )
                ScUnoHelpFunctions::SetBoolInAny( aRet, aViewOpt.GetOption( VOPT_HEADER ) );
            else if ( aPropertyName.compareToAscii( SC_UNO_SHEETTABS ) == 0 )
                ScUnoHelpFunctions::SetBoolInAny( aRet, aViewOpt.GetOption( VOPT_TABCONTROLS ) );
            else if ( aPropertyName.compareToAscii( SC_UNO_OUTLSYMB ) == 0 )
                ScUnoHelpFunctions::SetBoolInAny( aRet, aViewOpt.GetOption( VOPT_OUTLINER ) );
            else if ( aPropertyName.compareToAscii( SC_UNO_AUTOCALC ) == 0 )
                ScUnoHelpFunctions::SetBoolInAny( aRet, pDoc->GetAutoCalc() );
            else if ( aPropertyName.compareToAscii( SC_UNO_PRINTERNAME ) == 0 )
            {
                // #i75610# don't create the printer, return empty string if no printer created yet
                // (as in SwXDocumentSettings)
                SfxPrinter* pPrinter = pDoc->GetPrinter( false );
                if (pPrinter)
                    aRet <<= OUString ( pPrinter->GetName());
                else
                    aRet <<= OUString();
            }
            else if ( aPropertyName.compareToAscii( SC_UNO_PRINTERSETUP ) == 0 )
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
            else if ( aPropertyName.compareToAscii( SC_UNO_APPLYDOCINF ) == 0 )
                aRet <<= pDocShell->IsUseUserData();
            else if ( aPropertyName.compareToAscii( SC_UNO_FORBIDDEN ) == 0 )
            {
                aRet <<= uno::Reference<i18n::XForbiddenCharacters>(new ScForbiddenCharsObj( pDocShell ));
            }
            else if ( aPropertyName.compareToAscii( SC_UNO_CHARCOMP ) == 0 )
                aRet <<= static_cast<sal_Int16> ( pDoc->GetAsianCompression() );
            else if ( aPropertyName.compareToAscii( SC_UNO_ASIANKERN ) == 0 )
                ScUnoHelpFunctions::SetBoolInAny( aRet, pDoc->GetAsianKerning() );
            else if ( aPropertyName.compareToAscii( SCSAVEVERSION ) == 0)
                aRet <<= pDocShell->IsSaveVersionOnClose();
            else if ( aPropertyName.compareToAscii( SC_UNO_UPDTEMPL ) == 0 )
                aRet <<= pDocShell->IsQueryLoadTemplate();
            else if ( aPropertyName.compareToAscii( SC_UNO_LOADREADONLY ) == 0 )
                aRet <<= pDocShell->IsLoadReadonly();
            else if ( aPropertyName.compareToAscii( SC_UNO_SHAREDOC ) == 0 )
            {
#if HAVE_FEATURE_MULTIUSER_ENVIRONMENT
                ScUnoHelpFunctions::SetBoolInAny( aRet, pDocShell->HasSharedXMLFlagSet() );
#endif
            }
            else if ( aPropertyName.compareToAscii( SC_UNO_MODIFYPASSWORDINFO ) == 0 )
                aRet <<= pDocShell->GetModifyPasswordInfo();
            else if ( aPropertyName.compareToAscii( SC_UNO_EMBED_FONTS ) == 0 )
            {
                aRet <<= pDoc->IsUsingEmbededFonts();
            }

            else
            {
                const ScGridOptions& aGridOpt = aViewOpt.GetGridOptions();
                if ( aPropertyName.compareToAscii( SC_UNO_SNAPTORASTER ) == 0 )
                    ScUnoHelpFunctions::SetBoolInAny( aRet, aGridOpt.GetUseGridSnap() );
                else if ( aPropertyName.compareToAscii( SC_UNO_RASTERVIS ) == 0 )
                    ScUnoHelpFunctions::SetBoolInAny( aRet, aGridOpt.GetGridVisible() );
                else if ( aPropertyName.compareToAscii( SC_UNO_RASTERRESX ) == 0 )
                    aRet <<= static_cast<sal_Int32> ( aGridOpt.GetFldDrawX() );
                else if ( aPropertyName.compareToAscii( SC_UNO_RASTERRESY ) == 0 )
                    aRet <<= static_cast<sal_Int32> ( aGridOpt.GetFldDrawY() );
                else if ( aPropertyName.compareToAscii( SC_UNO_RASTERSUBX ) == 0 )
                    aRet <<= static_cast<sal_Int32> ( aGridOpt.GetFldDivisionX() );
                else if ( aPropertyName.compareToAscii( SC_UNO_RASTERSUBY ) == 0 )
                    aRet <<= static_cast<sal_Int32> ( aGridOpt.GetFldDivisionY() );
                else if ( aPropertyName.compareToAscii( SC_UNO_RASTERSYNC ) == 0 )
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

OUString SAL_CALL ScDocumentConfiguration::getImplementationName() throw(uno::RuntimeException)
{
    return OUString( "ScDocumentConfiguration" );
}

sal_Bool SAL_CALL ScDocumentConfiguration::supportsService( const OUString& rServiceName )
                                                    throw(uno::RuntimeException)
{
    String aServiceStr( rServiceName );
    return aServiceStr.EqualsAscii( SCCOMPSCPREADSHEETSETTINGS_SERVICE ) ||
           aServiceStr.EqualsAscii( SCDOCUMENTSETTINGS_SERVICE );
}

uno::Sequence<OUString> SAL_CALL ScDocumentConfiguration::getSupportedServiceNames()
                                                    throw(uno::RuntimeException)
{
    uno::Sequence<OUString> aRet(2);
    OUString* pArray = aRet.getArray();
    pArray[0] = OUString(RTL_CONSTASCII_USTRINGPARAM( SCCOMPSCPREADSHEETSETTINGS_SERVICE ));
    pArray[1] = OUString(RTL_CONSTASCII_USTRINGPARAM( SCDOCUMENTSETTINGS_SERVICE ));
    return aRet;
}

//-------------------------------------------------------------------------

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
