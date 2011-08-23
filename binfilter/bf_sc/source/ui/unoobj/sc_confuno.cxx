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

#ifdef PCH
#endif

#ifdef _MSC_VER
#pragma hdrstop
#endif

#include "confuno.hxx"
#include "unonames.hxx"
#include "unoguard.hxx"
#include "docsh.hxx"
#include "miscuno.hxx"
#include "forbiuno.hxx"
#include "viewopti.hxx"
#ifndef SC_SCDOCPOL_HXX
#include "docpool.hxx"
#endif
#include "bf_sc.hrc"

#include <com/sun/star/beans/PropertyAttribute.hpp>

#ifndef _SFX_PRINTER_HXX
#include <bf_sfx2/printer.hxx>
#endif
#ifndef _SFXDOCINF_HXX
#include <bf_sfx2/docinf.hxx>
#endif
namespace binfilter {

using namespace ::com::sun::star;

#define SCCOMPSCPREADSHEETSETTINGS_SERVICE		"com.sun.star.comp.SpreadsheetSettings"
#define SCDOCUMENTSETTINGS_SERVICE				"com.sun.star.document.Settings"
#define SCSAVEVERSION							"SaveVersionOnClose"


const SfxItemPropertyMap* lcl_GetConfigPropertyMap()
{
    static SfxItemPropertyMap aConfigPropertyMap_Impl[] =
    {
        {MAP_CHAR_LEN(SC_UNO_SHOWZERO),		0,	&getBooleanCppuType(),				0},
        {MAP_CHAR_LEN(SC_UNO_SHOWNOTES),	0,	&getBooleanCppuType(),				0},
        {MAP_CHAR_LEN(SC_UNO_SHOWGRID),		0,	&getBooleanCppuType(),				0},
        {MAP_CHAR_LEN(SC_UNO_GRIDCOLOR),	0,	&getCppuType((sal_Int32*)0),		0},
        {MAP_CHAR_LEN(SC_UNO_SHOWPAGEBR),	0,	&getBooleanCppuType(),				0},
        {MAP_CHAR_LEN(SC_UNONAME_LINKUPD),	0,	&getCppuType((sal_Int16*)0),		0},
        {MAP_CHAR_LEN(SC_UNO_COLROWHDR),	0,	&getBooleanCppuType(),				0},
        {MAP_CHAR_LEN(SC_UNO_SHEETTABS),	0,	&getBooleanCppuType(),				0},
        {MAP_CHAR_LEN(SC_UNO_OUTLSYMB),		0,	&getBooleanCppuType(),				0},
        {MAP_CHAR_LEN(SC_UNO_SNAPTORASTER),	0,	&getBooleanCppuType(),				0},
        {MAP_CHAR_LEN(SC_UNO_RASTERVIS),	0,	&getBooleanCppuType(),				0},
        {MAP_CHAR_LEN(SC_UNO_RASTERRESX),	0,	&getCppuType((sal_Int32*)0),		0},
        {MAP_CHAR_LEN(SC_UNO_RASTERRESY),	0,	&getCppuType((sal_Int32*)0),		0},
        {MAP_CHAR_LEN(SC_UNO_RASTERSUBX),	0,	&getCppuType((sal_Int32*)0),		0},
        {MAP_CHAR_LEN(SC_UNO_RASTERSUBY),	0,	&getCppuType((sal_Int32*)0),		0},
        {MAP_CHAR_LEN(SC_UNO_RASTERSYNC),	0,	&getBooleanCppuType(),				0},
        {MAP_CHAR_LEN(SC_UNO_AUTOCALC),		0,	&getBooleanCppuType(),				0},
        {MAP_CHAR_LEN(SC_UNO_PRINTERNAME),	0,	&getCppuType((::rtl::OUString*)0),	0},
        {MAP_CHAR_LEN(SC_UNO_PRINTERSETUP),	0,	&getCppuType((uno::Sequence<sal_Int8>*)0),	0},
        {MAP_CHAR_LEN(SC_UNO_APPLYDOCINF),	0,	&getBooleanCppuType(),				0},
        {MAP_CHAR_LEN(SC_UNO_FORBIDDEN),	0,	&getCppuType((uno::Reference<i18n::XForbiddenCharacters>*)0), beans::PropertyAttribute::READONLY},
        {MAP_CHAR_LEN(SC_UNO_CHARCOMP),		0,	&getCppuType((sal_Int16*)0),		0},
        {MAP_CHAR_LEN(SC_UNO_ASIANKERN),	0,	&getBooleanCppuType(),				0},
        {MAP_CHAR_LEN(SCSAVEVERSION),		0,	&getBooleanCppuType(),				0},
        {MAP_CHAR_LEN(SC_UNO_UPDTEMPL),     0,  &getBooleanCppuType(),              0},
        {0,0,0,0}
    };
    return aConfigPropertyMap_Impl;
}

//------------------------------------------------------------------------

ScDocumentConfiguration::ScDocumentConfiguration(ScDocShell* pDocSh)
    : aPropSet ( lcl_GetConfigPropertyMap() ) ,
    pDocShell(pDocSh)
{
    pDocShell->GetDocument()->AddUnoObject(*this);
}

ScDocumentConfiguration::~ScDocumentConfiguration()
{
    if (pDocShell)
        pDocShell->GetDocument()->RemoveUnoObject(*this);
}

void ScDocumentConfiguration::Notify( SfxBroadcaster& rBC, const SfxHint& rHint )
{
    //	Referenz-Update interessiert hier nicht

    if ( rHint.ISA( SfxSimpleHint ) &&
            ((const SfxSimpleHint&)rHint).GetId() == SFX_HINT_DYING )
    {
        pDocShell = NULL;		// ungueltig geworden
    }
}

// XPropertySet

uno::Reference<beans::XPropertySetInfo> SAL_CALL ScDocumentConfiguration::getPropertySetInfo()
                                                        throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
    static uno::Reference<beans::XPropertySetInfo> aRef =
        new SfxItemPropertySetInfo( aPropSet.getPropertyMap() );
    return aRef;
}

void SAL_CALL ScDocumentConfiguration::setPropertyValue(
                        const ::rtl::OUString& aPropertyName, const uno::Any& aValue )
                throw(beans::UnknownPropertyException, beans::PropertyVetoException,
                        lang::IllegalArgumentException, lang::WrappedTargetException,
                        uno::RuntimeException)
{
    ScUnoGuard aGuard;

    if(pDocShell)
    {
        ScDocument* pDoc = pDocShell->GetDocument();
        if (pDoc)
        {
            sal_Bool bUpdateHeights = sal_False;

            ScViewOptions aViewOpt(pDoc->GetViewOptions());
            if ( aPropertyName.compareToAscii( SC_UNO_SHOWZERO ) == 0 )
                aViewOpt.SetOption(VOPT_NULLVALS, ScUnoHelpFunctions::GetBoolFromAny( aValue ) );
            else if ( aPropertyName.compareToAscii( SC_UNO_SHOWNOTES ) == 0 )
                aViewOpt.SetOption(VOPT_NOTES, ScUnoHelpFunctions::GetBoolFromAny( aValue ) );
            else if ( aPropertyName.compareToAscii( SC_UNO_SHOWGRID ) == 0 )
                aViewOpt.SetOption(VOPT_GRID, ScUnoHelpFunctions::GetBoolFromAny( aValue ) );
            else if ( aPropertyName.compareToAscii( SC_UNO_GRIDCOLOR ) == 0 )
            {
                sal_Int64 nColor;
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
                SfxPrinter* pPrinter = pDocShell->GetPrinter();
                if (pPrinter)
                {
                    ::rtl::OUString sPrinterName;
                    if (aValue >>= sPrinterName)
                    {
                        String aString(sPrinterName);
                        SfxPrinter* pNewPrinter = new SfxPrinter( pPrinter->GetOptions().Clone(), aString );
                        if (pNewPrinter->IsKnown())
                            pDocShell->SetPrinter( pNewPrinter, SFX_PRINTER_PRINTER );
                        else
                            delete pNewPrinter;
                    }
                    else
                        throw lang::IllegalArgumentException();
                }
                else
                    throw uno::RuntimeException();
            }
            else if ( aPropertyName.compareToAscii( SC_UNO_PRINTERSETUP ) == 0 )
            {
                uno::Sequence<sal_Int8> aSequence;
                if ( aValue >>= aSequence )
                {
                    sal_uInt32 nSize = aSequence.getLength();
                    SvMemoryStream aStream (aSequence.getArray(), nSize, STREAM_READ );
                    aStream.Seek ( STREAM_SEEK_TO_BEGIN );
                    SfxItemSet* pSet = new SfxItemSet( *pDoc->GetPool(),
                            SID_PRINTER_NOTFOUND_WARN, SID_PRINTER_NOTFOUND_WARN,
                            SID_PRINTER_CHANGESTODOC,  SID_PRINTER_CHANGESTODOC,
                            SID_SCPRINTOPTIONS,        SID_SCPRINTOPTIONS,
                            NULL );
                    pDocShell->SetPrinter( SfxPrinter::Create( aStream, pSet ) );
                }
            }
            else if ( aPropertyName.compareToAscii( SC_UNO_APPLYDOCINF ) == 0 )
                pDocShell->GetDocInfo().SetUseUserData( ScUnoHelpFunctions::GetBoolFromAny( aValue ) );
            else if ( aPropertyName.compareToAscii( SC_UNO_FORBIDDEN ) == 0 )
            {
                //	read-only - should not be set
            }
            else if ( aPropertyName.compareToAscii( SC_UNO_CHARCOMP ) == 0 )
            {
                // Int16 contains CharacterCompressionType values
                sal_Int16 nUno = ScUnoHelpFunctions::GetInt16FromAny( aValue );
                pDoc->SetAsianCompression( (BYTE) nUno );
                bUpdateHeights = sal_True;
            }
            else if ( aPropertyName.compareToAscii( SC_UNO_ASIANKERN ) == 0 )
            {
                pDoc->SetAsianKerning( ScUnoHelpFunctions::GetBoolFromAny( aValue ) );
                bUpdateHeights = sal_True;
            }
            else if ( aPropertyName.compareToAscii( SCSAVEVERSION ) == 0)
                pDocShell->GetDocInfo().SetSaveVersionOnClose( ScUnoHelpFunctions::GetBoolFromAny( aValue ) );
            else if ( aPropertyName.compareToAscii( SC_UNO_UPDTEMPL ) == 0 )
                pDocShell->GetDocInfo().SetQueryLoadTemplate( ScUnoHelpFunctions::GetBoolFromAny( aValue ) );
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
                //	update automatic row heights and repaint
                USHORT nTabCount = pDoc->GetTableCount();
                for (USHORT nTab=0; nTab<nTabCount; nTab++)
                    if ( !pDocShell->AdjustRowHeight( 0, MAXROW, nTab ) )
                        pDocShell->PostPaint( 0,0,nTab, MAXCOL,MAXROW,nTab, PAINT_GRID );
                pDocShell->SetDocumentModified();
            }
        }
        else
            throw uno::RuntimeException();
    }
    else
        throw uno::RuntimeException();
}

uno::Any SAL_CALL ScDocumentConfiguration::getPropertyValue( const ::rtl::OUString& aPropertyName )
                throw(beans::UnknownPropertyException, lang::WrappedTargetException,
                        uno::RuntimeException)
{
    ScUnoGuard aGuard;
    uno::Any aRet;

    if(pDocShell)
    {
        ScDocument* pDoc = pDocShell->GetDocument();
        if (pDoc)
        {
            const ScViewOptions& aViewOpt = pDoc->GetViewOptions();
            if ( aPropertyName.compareToAscii( SC_UNO_SHOWZERO ) == 0 )
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
                SfxPrinter *pPrinter = pDoc->GetPrinter ();
                if (pPrinter)
                    aRet <<= ::rtl::OUString ( pPrinter->GetName());
                else
                    throw uno::RuntimeException();
            }
            else if ( aPropertyName.compareToAscii( SC_UNO_PRINTERSETUP ) == 0 )
            {
                SfxPrinter *pPrinter = pDocShell->GetPrinter();
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
            }
            else if ( aPropertyName.compareToAscii( SC_UNO_APPLYDOCINF ) == 0 )
                ScUnoHelpFunctions::SetBoolInAny( aRet, pDocShell->GetDocInfo().IsUseUserData() );
            else if ( aPropertyName.compareToAscii( SC_UNO_FORBIDDEN ) == 0 )
            {
                uno::Reference<i18n::XForbiddenCharacters> xForbidden = new ScForbiddenCharsObj( pDocShell );
                aRet <<= xForbidden;
            }
            else if ( aPropertyName.compareToAscii( SC_UNO_CHARCOMP ) == 0 )
                aRet <<= static_cast<sal_Int16> ( pDoc->GetAsianCompression() );
            else if ( aPropertyName.compareToAscii( SC_UNO_ASIANKERN ) == 0 )
                ScUnoHelpFunctions::SetBoolInAny( aRet, pDoc->GetAsianKerning() );
            else if ( aPropertyName.compareToAscii( SCSAVEVERSION ) == 0)
                ScUnoHelpFunctions::SetBoolInAny( aRet, pDocShell->GetDocInfo().IsSaveVersionOnClose() );
            else if ( aPropertyName.compareToAscii( SC_UNO_UPDTEMPL ) == 0 )
                ScUnoHelpFunctions::SetBoolInAny( aRet, pDocShell->GetDocInfo().IsQueryLoadTemplate());
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

::rtl::OUString SAL_CALL ScDocumentConfiguration::getImplementationName() throw(uno::RuntimeException)
{
    return ::rtl::OUString::createFromAscii( "ScDocumentConfiguration" );
}

sal_Bool SAL_CALL ScDocumentConfiguration::supportsService( const ::rtl::OUString& rServiceName )
                                                    throw(uno::RuntimeException)
{
    String aServiceStr( rServiceName );
    return aServiceStr.EqualsAscii( SCCOMPSCPREADSHEETSETTINGS_SERVICE ) ||
           aServiceStr.EqualsAscii( SCDOCUMENTSETTINGS_SERVICE );
}

uno::Sequence< ::rtl::OUString> SAL_CALL ScDocumentConfiguration::getSupportedServiceNames()
                                                    throw(uno::RuntimeException)
{
    uno::Sequence< ::rtl::OUString> aRet(2);
    ::rtl::OUString* pArray = aRet.getArray();
    pArray[0] = ::rtl::OUString::createFromAscii( SCCOMPSCPREADSHEETSETTINGS_SERVICE );
    pArray[1] = ::rtl::OUString::createFromAscii( SCDOCUMENTSETTINGS_SERVICE );
    return aRet;
}

//-------------------------------------------------------------------------

}
