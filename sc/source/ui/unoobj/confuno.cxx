/*************************************************************************
 *
 *  $RCSfile: confuno.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: sab $ $Date: 2001-03-29 10:53:56 $
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

#ifdef PCH
#include "ui_pch.hxx"
#endif

#pragma hdrstop

#include "confuno.hxx"
#include "unonames.hxx"
#include "unoguard.hxx"
#include "scdll.hxx"
#include "docsh.hxx"
#include "miscuno.hxx"
#include "viewopti.hxx"

using namespace com::sun::star;

#define SCDOCUMENTCONFIGURATION_SERVICE "com.sun.star.sheet.DocumentConfiguration"

const SfxItemPropertyMap* lcl_GetConfigPropertyMap()
{
    static SfxItemPropertyMap aConfigPropertyMap_Impl[] =
    {
        {MAP_CHAR_LEN(SC_UNO_SHOWZERO),     0,  &getBooleanCppuType(),              0},
        {MAP_CHAR_LEN(SC_UNO_SHOWNOTES),    0,  &getBooleanCppuType(),              0},
        {MAP_CHAR_LEN(SC_UNO_SHOWGRID),     0,  &getBooleanCppuType(),              0},
        {MAP_CHAR_LEN(SC_UNO_GRIDCOLOR),    0,  &getCppuType((sal_Int32*)0),        0},
        {MAP_CHAR_LEN(SC_UNO_SHOWPAGEBR),   0,  &getBooleanCppuType(),              0},
        {MAP_CHAR_LEN(SC_UNONAME_LINKUPD),  0,  &getCppuType((sal_Int16*)0),        0},
        {MAP_CHAR_LEN(SC_UNO_COLROWHDR),    0,  &getBooleanCppuType(),              0},
        {MAP_CHAR_LEN(SC_UNO_SHEETTABS),    0,  &getBooleanCppuType(),              0},
        {MAP_CHAR_LEN(SC_UNO_OUTLSYMB),     0,  &getBooleanCppuType(),              0},
        {MAP_CHAR_LEN(SC_UNO_SNAPTORASTER), 0,  &getBooleanCppuType(),              0},
        {MAP_CHAR_LEN(SC_UNO_RASTERVIS),    0,  &getBooleanCppuType(),              0},
        {MAP_CHAR_LEN(SC_UNO_RASTERRESX),   0,  &getCppuType((sal_Int32*)0),        0},
        {MAP_CHAR_LEN(SC_UNO_RASTERRESY),   0,  &getCppuType((sal_Int32*)0),        0},
        {MAP_CHAR_LEN(SC_UNO_RASTERSUBX),   0,  &getCppuType((sal_Int32*)0),        0},
        {MAP_CHAR_LEN(SC_UNO_RASTERSUBY),   0,  &getCppuType((sal_Int32*)0),        0},
        {MAP_CHAR_LEN(SC_UNO_RASTERSYNC),   0,  &getBooleanCppuType(),              0},
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

ScDocumentConfiguration::ScDocumentConfiguration()
    : aPropSet ( lcl_GetConfigPropertyMap() )
{
}

ScDocumentConfiguration::~ScDocumentConfiguration()
{
    if (pDocShell)
        pDocShell->GetDocument()->RemoveUnoObject(*this);
}

void ScDocumentConfiguration::Notify( SfxBroadcaster& rBC, const SfxHint& rHint )
{
    //  Referenz-Update interessiert hier nicht

    if ( rHint.ISA( SfxSimpleHint ) &&
            ((const SfxSimpleHint&)rHint).GetId() == SFX_HINT_DYING )
    {
        pDocShell = NULL;       // ungueltig geworden
    }
}

uno::Reference<uno::XInterface> SAL_CALL ScDocumentConfiguration_CreateInstance(
                        const uno::Reference<lang::XMultiServiceFactory>& rSMgr )
{
    ScUnoGuard aGuard;
    SC_DLL()->Load();       // load module
    static uno::Reference<uno::XInterface> xInst = (cppu::OWeakObject*)new ScDocumentConfiguration();
    return xInst;
}

rtl::OUString ScDocumentConfiguration::getImplementationName_Static()
{
    return rtl::OUString::createFromAscii( "stardiv.StarCalc.ScDocumentConfiguration" );
}

uno::Sequence<rtl::OUString> ScDocumentConfiguration::getSupportedServiceNames_Static()
{
    uno::Sequence<rtl::OUString> aRet(1);
    rtl::OUString* pArray = aRet.getArray();
    pArray[0] = rtl::OUString::createFromAscii( SCDOCUMENTCONFIGURATION_SERVICE );
    return aRet;
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
                        const rtl::OUString& aPropertyName, const uno::Any& aValue )
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
            }
            pDoc->SetViewOptions(aViewOpt);
        }
    }
}

uno::Any SAL_CALL ScDocumentConfiguration::getPropertyValue( const rtl::OUString& aPropertyName )
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
    }

    return aRet;
}

SC_IMPL_DUMMY_PROPERTY_LISTENER( ScDocumentConfiguration )
SC_SIMPLE_SERVICE_INFO( ScDocumentConfiguration, "ScDocumentConfiguration", SCDOCUMENTCONFIGURATION_SERVICE )

//-------------------------------------------------------------------------
