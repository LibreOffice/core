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
#include <i18nlangtag/lang.h>

#include <optutil.hxx>
#include <com/sun/star/uno/Any.hxx>
#include <com/sun/star/uno/Sequence.hxx>
#include <boost/scoped_ptr.hpp>

using namespace com::sun::star;
using namespace com::sun::star::uno;

#define SCFILTEROPTIONSOBJ_SERVICE      "com.sun.star.ui.dialogs.FilterOptionsDialog"
#define SCFILTEROPTIONSOBJ_IMPLNAME     "com.sun.star.comp.Calc.FilterOptionsDialog"

SC_SIMPLE_SERVICE_INFO( ScFilterOptionsObj, SCFILTEROPTIONSOBJ_IMPLNAME, SCFILTEROPTIONSOBJ_SERVICE )

#define SC_UNONAME_FILENAME         "URL"
#define SC_UNONAME_FILTERNAME       "FilterName"
#define SC_UNONAME_FILTEROPTIONS    "FilterOptions"
#define SC_UNONAME_INPUTSTREAM      "InputStream"

#define DBF_CHAR_SET                "CharSet"
#define DBF_SEP_PATH_IMPORT         "Office.Calc/Dialogs/DBFImport"
#define DBF_SEP_PATH_EXPORT         "Office.Calc/Dialogs/DBFExport"

static void load_CharSet( rtl_TextEncoding &nCharSet, bool bExport )
{
    Sequence<Any> aValues;
    const Any *pProperties;
    Sequence<OUString> aNames(1);
    OUString* pNames = aNames.getArray();
    ScLinkConfigItem aItem( OUString::createFromAscii(
                                bExport?DBF_SEP_PATH_EXPORT:DBF_SEP_PATH_IMPORT ) );

    pNames[0] = DBF_CHAR_SET;
    aValues = aItem.GetProperties( aNames );
    pProperties = aValues.getConstArray();

    // Default choice
    nCharSet = RTL_TEXTENCODING_IBM_850;

    if( pProperties[0].hasValue() )
    {
        sal_Int32 nChar = 0;
        pProperties[0] >>= nChar;
        if( nChar >= 0)
        {
            nCharSet = (rtl_TextEncoding) nChar;
        }
    }
}

static void save_CharSet( rtl_TextEncoding nCharSet, bool bExport )
{
    Sequence<Any> aValues;
    Any *pProperties;
    Sequence<OUString> aNames(1);
    OUString* pNames = aNames.getArray();
    ScLinkConfigItem aItem( OUString::createFromAscii(
                                bExport?DBF_SEP_PATH_EXPORT:DBF_SEP_PATH_IMPORT ) );

    pNames[0] = DBF_CHAR_SET;
    aValues = aItem.GetProperties( aNames );
    pProperties = aValues.getArray();
    pProperties[0] <<= (sal_Int32) nCharSet;

    aItem.PutProperties(aNames, aValues);
}

ScFilterOptionsObj::ScFilterOptionsObj() :
    bExport( false )
{
}

ScFilterOptionsObj::~ScFilterOptionsObj()
{
}

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface* SAL_CALL
Calc_FilterOptionsDialog_get_implementation(css::uno::XComponentContext*, css::uno::Sequence<css::uno::Any> const &)
{
    SolarMutexGuard aGuard;
    ScDLL::Init();
    return cppu::acquire(new ScFilterOptionsObj);
}

// XPropertyAccess

uno::Sequence<beans::PropertyValue> SAL_CALL ScFilterOptionsObj::getPropertyValues() throw(uno::RuntimeException, std::exception)
{
    uno::Sequence<beans::PropertyValue> aRet(1);
    beans::PropertyValue* pArray = aRet.getArray();

    pArray[0].Name = SC_UNONAME_FILTEROPTIONS;
    pArray[0].Value <<= aFilterOptions;

    return aRet;
}

void SAL_CALL ScFilterOptionsObj::setPropertyValues( const uno::Sequence<beans::PropertyValue>& aProps )
                    throw(beans::UnknownPropertyException, beans::PropertyVetoException,
                            lang::IllegalArgumentException, lang::WrappedTargetException, uno::RuntimeException, std::exception)
{
    const beans::PropertyValue* pPropArray = aProps.getConstArray();
    long nPropCount = aProps.getLength();
    for (long i = 0; i < nPropCount; i++)
    {
        const beans::PropertyValue& rProp = pPropArray[i];
        OUString aPropName(rProp.Name);

        if ( aPropName == SC_UNONAME_FILENAME )
            rProp.Value >>= aFileName;
        else if ( aPropName == SC_UNONAME_FILTERNAME )
            rProp.Value >>= aFilterName;
        else if ( aPropName == SC_UNONAME_FILTEROPTIONS )
            rProp.Value >>= aFilterOptions;
        else if ( aPropName == SC_UNONAME_INPUTSTREAM )
            rProp.Value >>= xInputStream;
    }
}

// XExecutableDialog

void SAL_CALL ScFilterOptionsObj::setTitle( const OUString& /* aTitle */ ) throw(uno::RuntimeException, std::exception)
{
    // not used
}

sal_Int16 SAL_CALL ScFilterOptionsObj::execute() throw(uno::RuntimeException, std::exception)
{
    sal_Int16 nRet = ui::dialogs::ExecutableDialogResults::CANCEL;

    OUString aFilterString( aFilterName );

    ScAbstractDialogFactory* pFact = ScAbstractDialogFactory::Create();
    OSL_ENSURE(pFact, "ScAbstractFactory create fail!");

    if ( !bExport && aFilterString == ScDocShell::GetAsciiFilterName() )
    {
        //  ascii import is special...

        INetURLObject aURL( aFileName );
        OUString aPrivDatName(aURL.getName());
        boost::scoped_ptr<SvStream> pInStream;
        if ( xInputStream.is() )
            pInStream.reset(utl::UcbStreamHelper::CreateStream( xInputStream ));

        boost::scoped_ptr<AbstractScImportAsciiDlg> pDlg(pFact->CreateScImportAsciiDlg( NULL, aPrivDatName, pInStream.get(), SC_IMPORTFILE));
        OSL_ENSURE(pDlg, "Dialog create fail!");
        if ( pDlg->Execute() == RET_OK )
        {
            ScAsciiOptions aOptions;
            pDlg->GetOptions( aOptions );
            pDlg->SaveParameters();
            aFilterOptions = aOptions.WriteToString();
            nRet = ui::dialogs::ExecutableDialogResults::OK;
        }
    }
    else if ( aFilterString == ScDocShell::GetWebQueryFilterName() || aFilterString == ScDocShell::GetHtmlFilterName() )
    {
        if (bExport)
            nRet = ui::dialogs::ExecutableDialogResults::OK;    // export HTML without dialog
        else
        {
            // HTML import.
            boost::scoped_ptr<AbstractScTextImportOptionsDlg> pDlg(
                pFact->CreateScTextImportOptionsDlg(NULL));

            if (pDlg->Execute() == RET_OK)
            {
                LanguageType eLang = pDlg->GetLanguageType();
                OUStringBuffer aBuf;

                aBuf.append(OUString::number(static_cast<sal_Int32>(eLang)));
                aBuf.append(' ');
                aBuf.append(pDlg->IsDateConversionSet() ? sal_Unicode('1') : sal_Unicode('0'));
                aFilterOptions = aBuf.makeStringAndClear();
                nRet = ui::dialogs::ExecutableDialogResults::OK;
            }
        }
    }
    else
    {
        bool bMultiByte = true;
        bool bDBEnc     = false;
        bool bAscii     = false;

        sal_Unicode cStrDel = '"';
        sal_Unicode cAsciiDel = ';';
        rtl_TextEncoding eEncoding = RTL_TEXTENCODING_DONTKNOW;

        OUString aTitle;

        if ( aFilterString == ScDocShell::GetAsciiFilterName() )
        {
            //  ascii export (import is handled above)

            INetURLObject aURL( aFileName );
            OUString aExt(aURL.getExtension());
            if (aExt.equalsIgnoreAsciiCase("CSV"))
                cAsciiDel = ',';
            else
                cAsciiDel = '\t';

            aTitle = ScGlobal::GetRscString( STR_EXPORT_ASCII );
            bAscii = true;
        }
        else if ( aFilterString == ScDocShell::GetLotusFilterName() )
        {
            //  lotus is only imported
            OSL_ENSURE( !bExport, "Filter Options for Lotus Export is not implemented" );

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
            load_CharSet( eEncoding, bExport );
            bDBEnc = true;
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

        boost::scoped_ptr<AbstractScImportOptionsDlg> pDlg(pFact->CreateScImportOptionsDlg(NULL,
                                                                            bAscii, &aOptions, &aTitle, bMultiByte, bDBEnc,
                                                                            !bExport));
        OSL_ENSURE(pDlg, "Dialog create fail!");
        if ( pDlg->Execute() == RET_OK )
        {
            pDlg->GetImportOptions( aOptions );
            save_CharSet( aOptions.eCharSet, bExport );
            if ( bAscii )
                aFilterOptions = aOptions.BuildString();
            else
                aFilterOptions = aOptions.aStrFont;
            nRet = ui::dialogs::ExecutableDialogResults::OK;
        }
    }

    xInputStream.clear();   // don't hold the stream longer than necessary

    return nRet;
}

// XImporter

void SAL_CALL ScFilterOptionsObj::setTargetDocument( const uno::Reference<lang::XComponent>& /* xDoc */ )
                            throw(lang::IllegalArgumentException, uno::RuntimeException, std::exception)
{
    bExport = false;
}

// XExporter

void SAL_CALL ScFilterOptionsObj::setSourceDocument( const uno::Reference<lang::XComponent>& /* xDoc */ )
                            throw(lang::IllegalArgumentException, uno::RuntimeException, std::exception)
{
    bExport = true;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
