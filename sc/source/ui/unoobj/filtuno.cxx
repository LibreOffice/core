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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sc.hxx"



#include <com/sun/star/ui/dialogs/ExecutableDialogResults.hpp>
#include <tools/urlobj.hxx>
#include <vcl/msgbox.hxx>
#include <vcl/svapp.hxx>
#include <unotools/ucbstreamhelper.hxx>

#include "editutil.hxx"
#include "filtuno.hxx"
#include "miscuno.hxx"
#include "scdll.hxx"
#include "imoptdlg.hxx"
#include "asciiopt.hxx"
#include "docsh.hxx"
#include "globstr.hrc"


#include "sc.hrc"
#include "scabstdlg.hxx"
#include "i18npool/lang.h"

#include <memory>

using namespace ::com::sun::star;
using ::rtl::OUStringBuffer;

//------------------------------------------------------------------------

#define SCFILTEROPTIONSOBJ_SERVICE      "com.sun.star.ui.dialogs.FilterOptionsDialog"
#define SCFILTEROPTIONSOBJ_IMPLNAME     "com.sun.star.comp.Calc.FilterOptionsDialog"

SC_SIMPLE_SERVICE_INFO( ScFilterOptionsObj, SCFILTEROPTIONSOBJ_IMPLNAME, SCFILTEROPTIONSOBJ_SERVICE )

#define SC_UNONAME_FILENAME         "URL"
#define SC_UNONAME_FILTERNAME       "FilterName"
#define SC_UNONAME_FILTEROPTIONS    "FilterOptions"
#define SC_UNONAME_INPUTSTREAM      "InputStream"

//------------------------------------------------------------------------

ScFilterOptionsObj::ScFilterOptionsObj() :
    bExport( false )
{
}

ScFilterOptionsObj::~ScFilterOptionsObj()
{
}

// stuff for exService_...

uno::Reference<uno::XInterface> SAL_CALL ScFilterOptionsObj_CreateInstance(
                        const uno::Reference<lang::XMultiServiceFactory>& )
{
    SolarMutexGuard aGuard;
    ScDLL::Init();
    return (::cppu::OWeakObject*) new ScFilterOptionsObj;
}

rtl::OUString ScFilterOptionsObj::getImplementationName_Static()
{
    return rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( SCFILTEROPTIONSOBJ_IMPLNAME ));
}

uno::Sequence<rtl::OUString> ScFilterOptionsObj::getSupportedServiceNames_Static()
{
    uno::Sequence<rtl::OUString> aRet(1);
    rtl::OUString* pArray = aRet.getArray();
    pArray[0] = rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( SCFILTEROPTIONSOBJ_SERVICE ));
    return aRet;
}

// XPropertyAccess

uno::Sequence<beans::PropertyValue> SAL_CALL ScFilterOptionsObj::getPropertyValues() throw(uno::RuntimeException)
{
    uno::Sequence<beans::PropertyValue> aRet(1);
    beans::PropertyValue* pArray = aRet.getArray();

    pArray[0].Name = rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( SC_UNONAME_FILTEROPTIONS ));
    pArray[0].Value <<= aFilterOptions;

    return aRet;
}

void SAL_CALL ScFilterOptionsObj::setPropertyValues( const uno::Sequence<beans::PropertyValue>& aProps )
                    throw(beans::UnknownPropertyException, beans::PropertyVetoException,
                            lang::IllegalArgumentException, lang::WrappedTargetException, uno::RuntimeException)
{
    const beans::PropertyValue* pPropArray = aProps.getConstArray();
    long nPropCount = aProps.getLength();
    for (long i = 0; i < nPropCount; i++)
    {
        const beans::PropertyValue& rProp = pPropArray[i];
        String aPropName(rProp.Name);

        if ( aPropName.EqualsAscii( SC_UNONAME_FILENAME ) )
            rProp.Value >>= aFileName;
        else if ( aPropName.EqualsAscii( SC_UNONAME_FILTERNAME ) )
            rProp.Value >>= aFilterName;
        else if ( aPropName.EqualsAscii( SC_UNONAME_FILTEROPTIONS ) )
            rProp.Value >>= aFilterOptions;
        else if ( aPropName.EqualsAscii( SC_UNONAME_INPUTSTREAM ) )
            rProp.Value >>= xInputStream;
    }
}

// XExecutableDialog

void SAL_CALL ScFilterOptionsObj::setTitle( const ::rtl::OUString& /* aTitle */ ) throw(uno::RuntimeException)
{
    // not used
}

sal_Int16 SAL_CALL ScFilterOptionsObj::execute() throw(uno::RuntimeException)
{
    sal_Int16 nRet = ui::dialogs::ExecutableDialogResults::CANCEL;

    String aFilterString( aFilterName );

    ScAbstractDialogFactory* pFact = ScAbstractDialogFactory::Create();
    DBG_ASSERT(pFact, "ScAbstractFactory create fail!");

    if ( !bExport && aFilterString == ScDocShell::GetAsciiFilterName() )
    {
        //  ascii import is special...

        INetURLObject aURL( aFileName );
        String aExt(aURL.getExtension());
        String aPrivDatName(aURL.getName());
        sal_Unicode cAsciiDel;
        if (aExt.EqualsIgnoreCaseAscii("CSV"))
            cAsciiDel = ',';
        else
            cAsciiDel = '\t';

        SvStream* pInStream = NULL;
        if ( xInputStream.is() )
            pInStream = utl::UcbStreamHelper::CreateStream( xInputStream );

        AbstractScImportAsciiDlg* pDlg = pFact->CreateScImportAsciiDlg( NULL, aPrivDatName, pInStream, RID_SCDLG_ASCII, cAsciiDel);
        DBG_ASSERT(pDlg, "Dialog create fail!");
        if ( pDlg->Execute() == RET_OK )
        {
            ScAsciiOptions aOptions;
            pDlg->GetOptions( aOptions );
            pDlg->SaveParameters();
            aFilterOptions = aOptions.WriteToString();
            nRet = ui::dialogs::ExecutableDialogResults::OK;
        }
        delete pDlg;
        delete pInStream;
    }
    else if ( aFilterString == ScDocShell::GetWebQueryFilterName() || aFilterString == ScDocShell::GetHtmlFilterName() )
    {
        if (bExport)
            nRet = ui::dialogs::ExecutableDialogResults::OK;    // export HTML without dialog
        else
        {
            // HTML import.
            ::std::auto_ptr<AbstractScTextImportOptionsDlg> pDlg(
                pFact->CreateScTextImportOptionsDlg(NULL, RID_SCDLG_TEXT_IMPORT_OPTIONS));

            if (pDlg->Execute() == RET_OK)
            {
                LanguageType eLang = pDlg->GetLanguageType();
                OUStringBuffer aBuf;

                aBuf.append(String::CreateFromInt32(static_cast<sal_Int32>(eLang)));
                aBuf.append(sal_Unicode(' '));
                aBuf.append(pDlg->IsDateConversionSet() ? sal_Unicode('1') : sal_Unicode('0'));
                aFilterOptions = aBuf.makeStringAndClear();
                nRet = ui::dialogs::ExecutableDialogResults::OK;
            }
        }
    }
    else
    {
        sal_Bool bMultiByte = sal_True;
        sal_Bool bDBEnc     = false;
        sal_Bool bAscii     = false;

        sal_Unicode cStrDel = '"';
        sal_Unicode cAsciiDel = ';';
        rtl_TextEncoding eEncoding = RTL_TEXTENCODING_DONTKNOW;

        String aTitle;

        if ( aFilterString == ScDocShell::GetAsciiFilterName() )
        {
            //  ascii export (import is handled above)

            INetURLObject aURL( aFileName );
            String aExt(aURL.getExtension());
            if (aExt.EqualsIgnoreCaseAscii("CSV"))
                cAsciiDel = ',';
            else
                cAsciiDel = '\t';

            aTitle = ScGlobal::GetRscString( STR_EXPORT_ASCII );
            bAscii = sal_True;
        }
        else if ( aFilterString == ScDocShell::GetLotusFilterName() )
        {
            //  lotus is only imported
            DBG_ASSERT( !bExport, "Filter Options for Lotus Export is not implemented" );

            aTitle = ScGlobal::GetRscString( STR_IMPORT_LOTUS );
            eEncoding = RTL_TEXTENCODING_IBM_437;
        }
        else if ( aFilterString == ScDocShell::GetDBaseFilterName() )
        {
            if ( bExport )
            {
                //  dBase export
                aTitle = ScGlobal::GetRscString( STR_EXPORT_DBF );
            }
            else
            {
                //  dBase import
                aTitle = ScGlobal::GetRscString( STR_IMPORT_DBF );
            }
            // common for dBase import/export
            eEncoding = RTL_TEXTENCODING_IBM_850;
            bDBEnc = sal_True;
        }
        else if ( aFilterString == ScDocShell::GetDifFilterName() )
        {
            if ( bExport )
            {
                //  DIF export
                aTitle = ScGlobal::GetRscString( STR_EXPORT_DIF );
            }
            else
            {
                //  DIF import
                aTitle = ScGlobal::GetRscString( STR_IMPORT_DIF );
            }
            // common for DIF import/export
            eEncoding = RTL_TEXTENCODING_MS_1252;
        }

        ScImportOptions aOptions( cAsciiDel, cStrDel, eEncoding);

        AbstractScImportOptionsDlg* pDlg = pFact->CreateScImportOptionsDlg( NULL, RID_SCDLG_IMPORTOPT,
                                                                            bAscii, &aOptions, &aTitle, bMultiByte, bDBEnc,
                                                                            !bExport);
        DBG_ASSERT(pDlg, "Dialog create fail!");
        if ( pDlg->Execute() == RET_OK )
        {
            pDlg->GetImportOptions( aOptions );
            if ( bAscii )
                aFilterOptions = aOptions.BuildString();
            else
                aFilterOptions = aOptions.aStrFont;
            nRet = ui::dialogs::ExecutableDialogResults::OK;
        }
        delete pDlg;
    }

    xInputStream.clear();   // don't hold the stream longer than necessary

    return nRet;
}

// XImporter

void SAL_CALL ScFilterOptionsObj::setTargetDocument( const uno::Reference<lang::XComponent>& /* xDoc */ )
                            throw(lang::IllegalArgumentException, uno::RuntimeException)
{
    bExport = false;
}

// XExporter

void SAL_CALL ScFilterOptionsObj::setSourceDocument( const uno::Reference<lang::XComponent>& /* xDoc */ )
                            throw(lang::IllegalArgumentException, uno::RuntimeException)
{
    bExport = sal_True;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
