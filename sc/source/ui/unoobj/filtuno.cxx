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
#include <vcl/svapp.hxx>
#include <unotools/ucbstreamhelper.hxx>
#include <connectivity/dbtools.hxx>

#include <editutil.hxx>
#include <filtuno.hxx>
#include <miscuno.hxx>
#include <scdll.hxx>
#include <imoptdlg.hxx>
#include <asciiopt.hxx>
#include <docsh.hxx>
#include <globstr.hrc>
#include <scresid.hxx>

#include <sc.hrc>
#include <scabstdlg.hxx>
#include <i18nlangtag/lang.h>

#include <optutil.hxx>
#include <com/sun/star/uno/Any.hxx>
#include <com/sun/star/uno/Sequence.hxx>
#include <comphelper/namedvaluecollection.hxx>
#include <comphelper/propertysequence.hxx>
#include <memory>

using namespace com::sun::star;
using namespace com::sun::star::uno;
using namespace connectivity::dbase;

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

namespace
{

    enum class charsetSource
    {
        charset_from_file,
        charset_from_user_setting,
        charset_default
    };

    charsetSource load_CharSet(rtl_TextEncoding &nCharSet, bool bExport, SvStream* dbf_Stream)
    {
        if (dbf_Stream && dbfReadCharset(nCharSet, dbf_Stream))
        {
            return charsetSource::charset_from_file;
        }

        Sequence<Any> aValues;
        const Any *pProperties;
        Sequence<OUString> aNames { DBF_CHAR_SET };
        ScLinkConfigItem aItem( OUString::createFromAscii(
                                    bExport?DBF_SEP_PATH_EXPORT:DBF_SEP_PATH_IMPORT ) );

        aValues = aItem.GetProperties( aNames );
        pProperties = aValues.getConstArray();

        if( pProperties[0].hasValue() )
        {
            sal_Int32 nChar = 0;
            pProperties[0] >>= nChar;
            if( nChar >= 0)
            {
                nCharSet = static_cast<rtl_TextEncoding>(nChar);
                return charsetSource::charset_from_user_setting;
            }
        }

        // Default choice
        nCharSet = RTL_TEXTENCODING_IBM_850;
        return charsetSource::charset_default;
    }

    void save_CharSet( rtl_TextEncoding nCharSet, bool bExport )
    {
        Sequence<Any> aValues;
        Any *pProperties;
        Sequence<OUString> aNames { DBF_CHAR_SET };
        ScLinkConfigItem aItem( OUString::createFromAscii(
                                    bExport?DBF_SEP_PATH_EXPORT:DBF_SEP_PATH_IMPORT ) );

        aValues = aItem.GetProperties( aNames );
        pProperties = aValues.getArray();
        pProperties[0] <<= static_cast<sal_Int32>(nCharSet);

        aItem.PutProperties(aNames, aValues);
    }
}

ScFilterOptionsObj::ScFilterOptionsObj() :
    bExport( false )
{
}

ScFilterOptionsObj::~ScFilterOptionsObj()
{
}

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface*
Calc_FilterOptionsDialog_get_implementation(css::uno::XComponentContext*, css::uno::Sequence<css::uno::Any> const &)
{
    SolarMutexGuard aGuard;
    ScDLL::Init();
    return cppu::acquire(new ScFilterOptionsObj);
}

// XPropertyAccess

uno::Sequence<beans::PropertyValue> SAL_CALL ScFilterOptionsObj::getPropertyValues()
{
    return comphelper::InitPropertySequence({
        { SC_UNONAME_FILTEROPTIONS, Any(aFilterOptions) }
    });
}

void SAL_CALL ScFilterOptionsObj::setPropertyValues( const uno::Sequence<beans::PropertyValue>& aProps )
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

void SAL_CALL ScFilterOptionsObj::setTitle( const OUString& /* aTitle */ )
{
    // not used
}

sal_Int16 SAL_CALL ScFilterOptionsObj::execute()
{
    sal_Int16 nRet = ui::dialogs::ExecutableDialogResults::CANCEL;

    OUString aFilterString( aFilterName );

    ScAbstractDialogFactory* pFact = ScAbstractDialogFactory::Create();

    if ( !bExport && aFilterString == ScDocShell::GetAsciiFilterName() )
    {
        //  ascii import is special...

        INetURLObject aURL( aFileName );
        OUString aPrivDatName(aURL.getName());
        std::unique_ptr<SvStream> pInStream;
        if ( xInputStream.is() )
            pInStream = utl::UcbStreamHelper::CreateStream( xInputStream );

        ScopedVclPtr<AbstractScImportAsciiDlg> pDlg(pFact->CreateScImportAsciiDlg(nullptr, aPrivDatName, pInStream.get(), SC_IMPORTFILE));
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
            ScopedVclPtr<AbstractScTextImportOptionsDlg> pDlg(
                pFact->CreateScTextImportOptionsDlg(Application::GetFrameWeld(xDialogParent)));

            if (pDlg->Execute() == RET_OK)
            {
                LanguageType eLang = pDlg->GetLanguageType();
                OUStringBuffer aBuf;

                aBuf.append(OUString::number(static_cast<sal_uInt16>(eLang)));
                aBuf.append(' ');
                aBuf.append(pDlg->IsDateConversionSet() ? u'1' : u'0');
                aFilterOptions = aBuf.makeStringAndClear();
                nRet = ui::dialogs::ExecutableDialogResults::OK;
            }
        }
    }
    else
    {
        bool bDBEnc     = false;
        bool bAscii     = false;
        bool skipDialog = false;

        sal_Unicode const cStrDel = '"';
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

            aTitle = ScResId( STR_EXPORT_ASCII );
            bAscii = true;
        }
        else if ( aFilterString == ScDocShell::GetLotusFilterName() )
        {
            //  lotus is only imported
            OSL_ENSURE( !bExport, "Filter Options for Lotus Export is not implemented" );

            aTitle = ScResId( STR_IMPORT_LOTUS );
            eEncoding = RTL_TEXTENCODING_IBM_437;
        }
        else if ( aFilterString == ScDocShell::GetDBaseFilterName() )
        {
            if ( bExport )
            {
                //  dBase export
                aTitle = ScResId( STR_EXPORT_DBF );
            }
            else
            {
                //  dBase import
                aTitle = ScResId( STR_IMPORT_DBF );
            }

            std::unique_ptr<SvStream> pInStream;
            if ( xInputStream.is() )
                pInStream = utl::UcbStreamHelper::CreateStream( xInputStream );
            switch(load_CharSet( eEncoding, bExport, pInStream.get()))
            {
                case charsetSource::charset_from_file:
                  skipDialog = true;
                  break;
                case charsetSource::charset_from_user_setting:
                case charsetSource::charset_default:
                   break;
            }
            bDBEnc = true;
            // pInStream goes out of scope, the stream is automatically closed
        }
        else if ( aFilterString == ScDocShell::GetDifFilterName() )
        {
            if ( bExport )
            {
                //  DIF export
                aTitle = ScResId( STR_EXPORT_DIF );
            }
            else
            {
                //  DIF import
                aTitle = ScResId( STR_IMPORT_DIF );
            }
            // common for DIF import/export
            eEncoding = RTL_TEXTENCODING_MS_1252;
        }

        ScImportOptions aOptions( cAsciiDel, cStrDel, eEncoding);
        if(skipDialog)
        {
            // TODO: check we are not missing some of the stuff that ScImportOptionsDlg::GetImportOptions
            // (file sc/source/ui/dbgui/scuiimoptdlg.cxx) does
            // that is, if the dialog sets options that are not selected by the user (!)
            // then we are missing them here.
            // Then we may need to rip them out of the dialog.
            // Or we actually change the dialog to not display if skipDialog==true
            // in that case, add an argument skipDialog to CreateScImportOptionsDlg
            nRet = ui::dialogs::ExecutableDialogResults::OK;
        }
        else
        {
            ScopedVclPtr<AbstractScImportOptionsDlg> pDlg(pFact->CreateScImportOptionsDlg(Application::GetFrameWeld(xDialogParent),
                                                                            bAscii, &aOptions, &aTitle,
                                                                            bDBEnc, !bExport));
            if ( pDlg->Execute() == RET_OK )
            {
                pDlg->SaveImportOptions();
                pDlg->GetImportOptions( aOptions );
                save_CharSet( aOptions.eCharSet, bExport );
                nRet = ui::dialogs::ExecutableDialogResults::OK;
            }
        }
        if (nRet == ui::dialogs::ExecutableDialogResults::OK)
        {
            if ( bAscii )
                aFilterOptions = aOptions.BuildString();
            else
                aFilterOptions = aOptions.aStrFont;
        }
    }

    xInputStream.clear();   // don't hold the stream longer than necessary

    return nRet;
}

// XImporter

void SAL_CALL ScFilterOptionsObj::setTargetDocument( const uno::Reference<lang::XComponent>& /* xDoc */ )
{
    bExport = false;
}

// XExporter

void SAL_CALL ScFilterOptionsObj::setSourceDocument( const uno::Reference<lang::XComponent>& /* xDoc */ )
{
    bExport = true;
}

// XInitialization

void SAL_CALL ScFilterOptionsObj::initialize(const uno::Sequence<uno::Any>& rArguments)
{
    ::comphelper::NamedValueCollection aProperties(rArguments);
    if (aProperties.has("ParentWindow"))
        aProperties.get("ParentWindow") >>= xDialogParent;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
