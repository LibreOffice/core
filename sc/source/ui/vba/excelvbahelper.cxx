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

#include "excelvbahelper.hxx"

#include <basic/basmgr.hxx>
#include <comphelper/propertyvalue.hxx>
#include <comphelper/processfactory.hxx>
#include <comphelper/servicehelper.hxx>
#include <osl/file.hxx>
#include <tools/urlobj.hxx>
#include <vbahelper/vbahelper.hxx>
#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <com/sun/star/beans/PropertyExistException.hpp>
#include <com/sun/star/frame/XModel.hpp>
#include <com/sun/star/sheet/XSheetCellRange.hpp>
#include <com/sun/star/sheet/GlobalSheetSettings.hpp>
#include <com/sun/star/sheet/XUnnamedDatabaseRanges.hpp>
#include <com/sun/star/sheet/XSpreadsheet.hpp>
#include <com/sun/star/sheet/XDatabaseRange.hpp>
#include <com/sun/star/system/SystemShellExecute.hpp>
#include <com/sun/star/system/SystemShellExecuteFlags.hpp>
#include <com/sun/star/util/XCloneable.hpp>

#include <document.hxx>
#include <docuno.hxx>
#include <tabvwsh.hxx>
#include <transobj.hxx>
#include <cellsuno.hxx>
#include <gridwin.hxx>

#include <com/sun/star/script/vba/VBAEventId.hpp>
#include <com/sun/star/script/vba/XVBACompatibility.hpp>
#include <com/sun/star/script/vba/XVBAEventProcessor.hpp>
#include <com/sun/star/script/vba/XVBAModuleInfo.hpp>
#include <com/sun/star/script/ModuleInfo.hpp>
#include <com/sun/star/script/ModuleType.hpp>

using namespace ::com::sun::star;
using namespace ::ooo::vba;

namespace ooo::vba::excel {

uno::Reference< sheet::XUnnamedDatabaseRanges >
GetUnnamedDataBaseRanges( const ScDocShell* pShell )
{
    uno::Reference< frame::XModel > xModel;
    if ( pShell )
        xModel.set( pShell->GetModel(), uno::UNO_SET_THROW );
    uno::Reference< beans::XPropertySet > xModelProps( xModel, uno::UNO_QUERY_THROW );
    uno::Reference< sheet::XUnnamedDatabaseRanges > xUnnamedDBRanges( xModelProps->getPropertyValue(u"UnnamedDatabaseRanges"_ustr), uno::UNO_QUERY_THROW );
    return xUnnamedDBRanges;
}

// returns the XDatabaseRange for the autofilter on sheet (nSheet)
// also populates sName with the name of range
uno::Reference< sheet::XDatabaseRange >
GetAutoFiltRange( const ScDocShell* pShell, sal_Int16 nSheet )
{
    uno::Reference< sheet::XUnnamedDatabaseRanges > xUnnamedDBRanges( GetUnnamedDataBaseRanges( pShell ), uno::UNO_SET_THROW );
    uno::Reference< sheet::XDatabaseRange > xDataBaseRange;
    if (xUnnamedDBRanges->hasByTable( nSheet ) )
    {
        uno::Reference< sheet::XDatabaseRange > xDBRange( xUnnamedDBRanges->getByTable( nSheet ) , uno::UNO_QUERY_THROW );
        bool bHasAuto = false;
        uno::Reference< beans::XPropertySet > xProps( xDBRange, uno::UNO_QUERY_THROW );
        xProps->getPropertyValue(u"AutoFilter"_ustr) >>= bHasAuto;
        if ( bHasAuto )
        {
            xDataBaseRange=xDBRange;
        }
    }
    return xDataBaseRange;
}

ScDocShell* GetDocShellFromRange( const uno::Reference< uno::XInterface >& xRange )
{
    ScCellRangesBase* pScCellRangesBase = dynamic_cast<ScCellRangesBase*>( xRange.get() );
    if ( !pScCellRangesBase )
    {
        throw uno::RuntimeException(u"Failed to access underlying doc shell uno range object"_ustr );
    }
    return pScCellRangesBase->GetDocShell();
}

uno::Reference< XHelperInterface >
getUnoSheetModuleObj( const uno::Reference< table::XCellRange >& xRange )
{
    uno::Reference< sheet::XSheetCellRange > xSheetRange( xRange, uno::UNO_QUERY_THROW );
    uno::Reference< sheet::XSpreadsheet > xSheet( xSheetRange->getSpreadsheet(), uno::UNO_SET_THROW );
    return getUnoSheetModuleObj( xSheet );
}

void implSetZoom( const uno::Reference< frame::XModel >& xModel, sal_Int16 nZoom, std::vector< SCTAB >& nTabs )
{
    ScTabViewShell* pViewSh = excel::getBestViewShell( xModel );
    Fraction aFract( nZoom, 100 );
    pViewSh->GetViewData().SetZoom( aFract, aFract, nTabs );
    pViewSh->RefreshZoom();
}

namespace {

class PasteCellsWarningReseter
{
private:
    bool bInitialWarningState;
    /// @throws uno::RuntimeException
    static uno::Reference< sheet::XGlobalSheetSettings > const & getGlobalSheetSettings()
    {
        static uno::Reference< sheet::XGlobalSheetSettings > xProps = sheet::GlobalSheetSettings::create( comphelper::getProcessComponentContext() );
        return xProps;
    }

    /// @throws uno::RuntimeException
    static bool getReplaceCellsWarning()
    {
        return getGlobalSheetSettings()->getReplaceCellsWarning();
    }

    /// @throws uno::RuntimeException
    static void setReplaceCellsWarning( bool bState )
    {
        getGlobalSheetSettings()->setReplaceCellsWarning( bState );
    }
public:
    /// @throws uno::RuntimeException
    PasteCellsWarningReseter()
    {
        bInitialWarningState = getReplaceCellsWarning();
        if ( bInitialWarningState )
            setReplaceCellsWarning( false );
    }
    ~PasteCellsWarningReseter()
    {
        if ( bInitialWarningState )
        {
            // don't allow dtor to throw
            try
            {
                setReplaceCellsWarning( true );
            }
            catch ( uno::Exception& /*e*/ ){}
        }
    }
};

}

void
implnPaste( const uno::Reference< frame::XModel>& xModel )
{
    PasteCellsWarningReseter resetWarningBox;
    ScTabViewShell* pViewShell = getBestViewShell( xModel );
    if ( pViewShell )
    {
        pViewShell->PasteFromSystem();
        pViewShell->CellContentChanged();
    }
}

void
implnCopy( const uno::Reference< frame::XModel>& xModel )
{
    ScTabViewShell* pViewShell = getBestViewShell( xModel );
    ScDocShell* pDocShell = getDocShell( xModel );
    if ( !(pViewShell && pDocShell) )
        return;

    pViewShell->CopyToClip(nullptr,false,false,true);

    // mark the copied transfer object so it is used in ScVbaRange::Insert
    uno::Reference<datatransfer::XTransferable2> xTransferable(ScTabViewShell::GetClipData(pViewShell->GetViewData().GetActiveWin()));
    ScTransferObj* pClipObj = ScTransferObj::GetOwnClipboard(xTransferable);
    if (pClipObj)
    {
        pClipObj->SetUseInApi( true );
        pDocShell->SetClipData(xTransferable);
    }
}

void
implnCut( const uno::Reference< frame::XModel>& xModel )
{
    ScTabViewShell* pViewShell =  getBestViewShell( xModel );
    ScDocShell* pDocShell = getDocShell( xModel );
    if ( !(pViewShell && pDocShell) )
        return;

    pViewShell->CutToClip();

    // mark the copied transfer object so it is used in ScVbaRange::Insert
    uno::Reference<datatransfer::XTransferable2> xTransferable(ScTabViewShell::GetClipData(pViewShell->GetViewData().GetActiveWin()));
    ScTransferObj* pClipObj = ScTransferObj::GetOwnClipboard(xTransferable);
    if (pClipObj)
    {
        pClipObj->SetUseInApi( true );
        pDocShell->SetClipData(xTransferable);
    }
}

void implnPasteSpecial( const uno::Reference< frame::XModel>& xModel, InsertDeleteFlags nFlags, ScPasteFunc nFunction, bool bSkipEmpty, bool bTranspose)
{
    PasteCellsWarningReseter resetWarningBox;

    ScTabViewShell* pTabViewShell = getBestViewShell(xModel);
    if (!pTabViewShell)
        return;

    ScDocShell* pDocShell = getDocShell(xModel);
    if (!pDocShell)
        return;

    ScViewData& rView = pTabViewShell->GetViewData();
    vcl::Window* pWin = rView.GetActiveWin();
    if (!pWin)
        return;

    const ScTransferObj* pOwnClip = ScTransferObj::GetOwnClipboard(ScTabViewShell::GetClipData(pWin));
    if (pOwnClip)
    {
        pTabViewShell->PasteFromClip(nFlags, pOwnClip->GetDocument(),
            nFunction, bSkipEmpty, bTranspose, false,
            INS_NONE, InsertDeleteFlags::NONE, true);

        pTabViewShell->CellContentChanged();
    }
}

ScDocShell*
getDocShell( const css::uno::Reference< css::frame::XModel>& xModel )
{
    uno::Reference< uno::XInterface > xIf( xModel, uno::UNO_QUERY_THROW );
    ScModelObj* pModel = comphelper::getFromUnoTunnel< ScModelObj >( xIf );
    ScDocShell* pDocShell = nullptr;
    if ( pModel )
        pDocShell = static_cast<ScDocShell*>(pModel->GetEmbeddedObject());
    return pDocShell;

}

ScTabViewShell*
getBestViewShell( const css::uno::Reference< css::frame::XModel>& xModel )
{
    ScDocShell* pDocShell = getDocShell( xModel );
    if ( pDocShell )
        return pDocShell->GetBestViewShell();
    return nullptr;
}

ScTabViewShell*
getCurrentBestViewShell(  const uno::Reference< uno::XComponentContext >& xContext )
{
    uno::Reference< frame::XModel > xModel = getCurrentExcelDoc( xContext );
    return getBestViewShell( xModel );
}

SfxViewFrame*
getViewFrame( const uno::Reference< frame::XModel >& xModel )
{
    ScTabViewShell* pViewShell = getBestViewShell( xModel );
    if ( pViewShell )
        return &pViewShell->GetViewFrame();
    return nullptr;
}

uno::Reference< XHelperInterface >
getUnoSheetModuleObj( const uno::Reference< sheet::XSpreadsheet >& xSheet )
{
    uno::Reference< beans::XPropertySet > xProps( xSheet, uno::UNO_QUERY_THROW );
    OUString sCodeName;
    xProps->getPropertyValue(u"CodeName"_ustr) >>= sCodeName;
    // #TODO #FIXME ideally we should 'throw' here if we don't get a valid parent, but... it is possible
    // to create a module ( and use 'Option VBASupport 1' ) for a calc document, in this scenario there
    // are *NO* special document module objects ( of course being able to switch between vba/non vba mode at
    // the document in the future could fix this, especially IF the switching of the vba mode takes care to
    // create the special document module objects if they don't exist.
    return getUnoDocModule( sCodeName, GetDocShellFromRange( xSheet ) );
}

uno::Reference< XHelperInterface >
getUnoSheetModuleObj( const uno::Reference< sheet::XSheetCellRangeContainer >& xRanges )
{
    uno::Reference< container::XEnumerationAccess > xEnumAccess( xRanges, uno::UNO_QUERY_THROW );
    uno::Reference< container::XEnumeration > xEnum = xEnumAccess->createEnumeration();
    uno::Reference< table::XCellRange > xRange( xEnum->nextElement(), uno::UNO_QUERY_THROW );
    return getUnoSheetModuleObj( xRange );
}

uno::Reference< XHelperInterface >
getUnoSheetModuleObj( const uno::Reference< table::XCell >& xCell )
{
    uno::Reference< sheet::XSheetCellRange > xSheetRange( xCell, uno::UNO_QUERY_THROW );
    uno::Reference< sheet::XSpreadsheet > xSheet( xSheetRange->getSpreadsheet(), uno::UNO_SET_THROW );
    return getUnoSheetModuleObj( xSheet );
}

uno::Reference< XHelperInterface >
getUnoSheetModuleObj( const uno::Reference< frame::XModel >& xModel, SCTAB nTab )
{
    uno::Reference< sheet::XSpreadsheetDocument > xDoc( xModel, uno::UNO_QUERY_THROW );
    uno::Reference< container::XIndexAccess > xSheets( xDoc->getSheets(), uno::UNO_QUERY_THROW );
    uno::Reference< sheet::XSpreadsheet > xSheet( xSheets->getByIndex( nTab ), uno::UNO_QUERY_THROW );
    return getUnoSheetModuleObj( xSheet );
}

void setUpDocumentModules( const uno::Reference< sheet::XSpreadsheetDocument >& xDoc )
{
    uno::Reference< frame::XModel > xModel( xDoc, uno::UNO_QUERY );
    ScDocShell* pShell = excel::getDocShell( xModel );
    if ( !pShell )
        return;

    OUString aPrjName( u"Standard"_ustr );
    pShell->GetBasicManager()->SetName( aPrjName );

    /*  Set library container to VBA compatibility mode. This will create
        the VBA Globals object and store it in the Basic manager of the
        document. */
    uno::Reference<script::XLibraryContainer> xLibContainer = pShell->GetBasicContainer();
    uno::Reference<script::vba::XVBACompatibility> xVBACompat( xLibContainer, uno::UNO_QUERY_THROW );
    xVBACompat->setVBACompatibilityMode( true );

    if( xLibContainer.is() )
    {
        if( !xLibContainer->hasByName( aPrjName ) )
            xLibContainer->createLibrary( aPrjName );
        uno::Any aLibAny = xLibContainer->getByName( aPrjName );
        uno::Reference< container::XNameContainer > xLib;
        aLibAny >>= xLib;
        if( xLib.is()  )
        {
            uno::Reference< script::vba::XVBAModuleInfo > xVBAModuleInfo( xLib, uno::UNO_QUERY_THROW );
            uno::Reference< container::XNameAccess > xVBACodeNamedObjectAccess( pShell->GetModel()->createInstance(u"ooo.vba.VBAObjectModuleObjectProvider"_ustr), uno::UNO_QUERY_THROW );
            // set up the module info for the workbook and sheets in the newly created
            // spreadsheet
            ScDocument& rDoc = pShell->GetDocument();
            OUString sCodeName = rDoc.GetCodeName();
            if ( sCodeName.isEmpty() )
            {
                sCodeName = "ThisWorkbook";
                rDoc.SetCodeName( sCodeName );
            }

            std::vector< OUString > sDocModuleNames { sCodeName };

            for ( SCTAB index = 0; index < rDoc.GetTableCount(); index++)
            {
                OUString aName;
                rDoc.GetCodeName( index, aName );
                sDocModuleNames.push_back( aName );
            }

            for ( const auto& rName : sDocModuleNames )
            {
                script::ModuleInfo sModuleInfo;

                uno::Any aName= xVBACodeNamedObjectAccess->getByName( rName );
                sModuleInfo.ModuleObject.set( aName, uno::UNO_QUERY );
                sModuleInfo.ModuleType = script::ModuleType::DOCUMENT;
                xVBAModuleInfo->insertModuleInfo( rName, sModuleInfo );
                if( xLib->hasByName( rName ) )
                    xLib->replaceByName( rName, uno::Any( u"Option VBASupport 1\n"_ustr ) );
                else
                    xLib->insertByName( rName, uno::Any( u"Option VBASupport 1\n"_ustr ) );
            }
        }
    }

    /*  Trigger the Workbook_Open event, event processor will register
        itself as listener for specific events. */
    try
    {
        uno::Reference< script::vba::XVBAEventProcessor > xVbaEvents( pShell->GetDocument().GetVbaEventProcessor(), uno::UNO_SET_THROW );
        uno::Sequence< uno::Any > aArgs;
        xVbaEvents->processVbaEvent( script::vba::VBAEventId::WORKBOOK_OPEN, aArgs );
    }
    catch( uno::Exception& )
    {
    }
}

void ExportAsFixedFormatHelper(
    const uno::Reference< frame::XModel >& xModel, const css::uno::Reference< XApplication >& xApplication,
    const css::uno::Any& Type, const css::uno::Any& FileName, const css::uno::Any& Quality,
    const css::uno::Any& IncludeDocProperties, const css::uno::Any& From,
    const css::uno::Any& To, const css::uno::Any& OpenAfterPublish)
{
    OUString sType;
    if ((Type >>= sType) && (sType.equalsIgnoreAsciiCase(u"xlTypeXPS") || sType == "1"))
    {
        /* xlTypePDF    0   "PDF" - Portable Document Format file(.pdf)
           xlTypeXPS    1   "XPS" - XPS Document(.xps) --> not supported in LibreOffice */
        return;
    }

    OUString sFileName;
    FileName >>= sFileName;
    OUString sRelURL;;
    osl::FileBase::getFileURLFromSystemPath(sFileName, sRelURL);
    // detect if there is no path then we need
    // to use the current folder
    INetURLObject aURL(sRelURL);
    OUString sURL;
    sURL = aURL.GetMainURL(INetURLObject::DecodeMechanism::ToIUri);
    if (sURL.isEmpty())
    {
        // need to add cur dir ( of this workbook ) or else the 'Work' dir
        sURL = xModel->getURL();

        if (sURL.isEmpty())
        {
            // not path available from 'this' document
            // need to add the 'document'/work directory then
            OUString sWorkPath = xApplication->getDefaultFilePath();
            OUString sWorkURL;
            osl::FileBase::getFileURLFromSystemPath(sWorkPath, sWorkURL);
            aURL.SetURL(sWorkURL);
        }
        else
        {
            if (!sFileName.isEmpty())
            {
                aURL.SetURL(INetURLObject::GetAbsURL(sURL, sRelURL));
            }
            else
            {
                aURL.SetURL(sURL);
                if (aURL.removeExtension())
                    aURL.setExtension(u"pdf");
            }
        }
        sURL = aURL.GetMainURL(INetURLObject::DecodeMechanism::ToIUri);

    }

    sal_Int32 nTo = 0;
    sal_Int32 nFrom = 0;
    From >>= nFrom;
    To >>= nTo;

    OUString sRange(u"-"_ustr);

    css::uno::Sequence<css::beans::PropertyValue> aFilterData;
    if (nFrom || nTo)
    {
        if (nFrom)
            sRange = OUString::number(nFrom) + sRange;
        if (nTo)
            sRange += OUString::number(nTo);

        aFilterData.realloc(aFilterData.getLength() + 1);
        aFilterData.getArray()[aFilterData.getLength() - 1] = comphelper::makePropertyValue(u"PageRange"_ustr, sRange);
    }

    OUString sQuality;
    if (Quality >>= sQuality)
    {
        if (sQuality.equalsIgnoreAsciiCase(u"xlQualityMinimum") || sQuality == "1")
        {
            aFilterData.realloc(aFilterData.getLength() + 1);
            aFilterData.getArray()[aFilterData.getLength() - 1] = comphelper::makePropertyValue(u"Quality"_ustr, sal_Int32(70));
        }
        else if (sQuality.equalsIgnoreAsciiCase(u"xlQualityStandard") || sQuality == "0")
        {
            aFilterData.realloc(aFilterData.getLength() + 1);
            aFilterData.getArray()[aFilterData.getLength() - 1] = comphelper::makePropertyValue(u"UseLosslessCompression"_ustr, true);
        }
        else
        {
            /* Name               Value Description
               xlQualityMinimum   1     Minimum quality
               xlQualityStandard  0     Standard quality */
        }
    }

    // init set of params for storeToURL() call
    css::uno::Sequence<css::beans::PropertyValue> storeProps{
        comphelper::makePropertyValue(u"FilterData"_ustr, aFilterData),
        comphelper::makePropertyValue(u"FilterName"_ustr, u"calc_pdf_Export"_ustr),
        comphelper::makePropertyValue(u"URL"_ustr, sURL)
    };

    bool bIncludeDocProperties = true;
    if ((IncludeDocProperties >>= bIncludeDocProperties) && !bIncludeDocProperties)
    {
        uno::Reference<document::XDocumentPropertiesSupplier> xDPS(xModel, uno::UNO_QUERY);
        if (xDPS.is())
        {
            uno::Reference<document::XDocumentProperties> xDocProps = xDPS->getDocumentProperties();
            uno::Reference<util::XCloneable> xCloneable(xDocProps, uno::UNO_QUERY_THROW);
            uno::Reference<document::XDocumentProperties> xOldDocProps(xCloneable->createClone(), uno::UNO_QUERY_THROW);

            // reset doc properties to default temporary
            xDocProps->resetUserData(OUString());

            uno::Reference< frame::XStorable > xStor(xModel, uno::UNO_QUERY_THROW);
            try {
                xStor->storeToURL(sURL, storeProps);
            }
            catch (const uno::Exception&)
            {
                SetDocInfoState(xModel, xOldDocProps);
                throw;
            }

            SetDocInfoState(xModel, xOldDocProps);
        }
    }
    else
    {
        uno::Reference< frame::XStorable > xStor(xModel, uno::UNO_QUERY_THROW);
        xStor->storeToURL(sURL, storeProps);
    }

    bool bOpenAfterPublish = false;
    if ((OpenAfterPublish >>= bOpenAfterPublish) && bOpenAfterPublish)
    {
        uno::Reference<css::system::XSystemShellExecute> xSystemShellExecute(css::system::SystemShellExecute::create(::comphelper::getProcessComponentContext()));
        xSystemShellExecute->execute(aURL.GetMainURL(INetURLObject::DecodeMechanism::NONE), u""_ustr, css::system::SystemShellExecuteFlags::URIS_ONLY);
    }
}

void SetDocInfoState(
    const uno::Reference< frame::XModel >& xModel,
    const uno::Reference< css::document::XDocumentProperties>& i_xOldDocProps)
{
    uno::Reference<document::XDocumentPropertiesSupplier> const
        xModelDocPropsSupplier(xModel, uno::UNO_QUERY_THROW);
    uno::Reference<document::XDocumentProperties> const xDocPropsToFill =
        xModelDocPropsSupplier->getDocumentProperties();
    uno::Reference< beans::XPropertySet > const xPropSet(
        i_xOldDocProps->getUserDefinedProperties(), uno::UNO_QUERY_THROW);

    uno::Reference< util::XModifiable > xModifiable(xModel, uno::UNO_QUERY);
    if (!xModifiable.is())
        throw uno::RuntimeException();

    bool bIsModified = xModifiable->isModified();

    try
    {
        uno::Reference< beans::XPropertySet > const xSet(
            xDocPropsToFill->getUserDefinedProperties(), uno::UNO_QUERY);
        uno::Reference< beans::XPropertyContainer > xContainer(xSet, uno::UNO_QUERY);
        uno::Reference< beans::XPropertySetInfo > xSetInfo = xSet->getPropertySetInfo();
        const uno::Sequence< beans::Property > lProps = xSetInfo->getProperties();
        for (const beans::Property& rProp : lProps)
        {
            uno::Any aValue = xPropSet->getPropertyValue(rProp.Name);
            if (rProp.Attributes & css::beans::PropertyAttribute::REMOVABLE)
            {
                try
                {
                    // QUESTION: DefaultValue?!
                    xContainer->addProperty(rProp.Name, rProp.Attributes, aValue);
                }
                catch (beans::PropertyExistException const&) {}
                try
                {
                    // it is possible that the propertysets from XML and binary files differ; we shouldn't break then
                    xSet->setPropertyValue(rProp.Name, aValue);
                }
                catch (const uno::Exception&) {}
            }
        }

        // sigh... have to set these manually I'm afraid...
        xDocPropsToFill->setAuthor(i_xOldDocProps->getAuthor());
        xDocPropsToFill->setGenerator(i_xOldDocProps->getGenerator());
        xDocPropsToFill->setCreationDate(i_xOldDocProps->getCreationDate());
        xDocPropsToFill->setTitle(i_xOldDocProps->getTitle());
        xDocPropsToFill->setSubject(i_xOldDocProps->getSubject());
        xDocPropsToFill->setDescription(i_xOldDocProps->getDescription());
        xDocPropsToFill->setKeywords(i_xOldDocProps->getKeywords());
        xDocPropsToFill->setModifiedBy(i_xOldDocProps->getModifiedBy());
        xDocPropsToFill->setModificationDate(i_xOldDocProps->getModificationDate());
        xDocPropsToFill->setPrintedBy(i_xOldDocProps->getPrintedBy());
        xDocPropsToFill->setPrintDate(i_xOldDocProps->getPrintDate());
        xDocPropsToFill->setAutoloadURL(i_xOldDocProps->getAutoloadURL());
        xDocPropsToFill->setAutoloadSecs(i_xOldDocProps->getAutoloadSecs());
        xDocPropsToFill->setDefaultTarget(i_xOldDocProps->getDefaultTarget());
        xDocPropsToFill->setEditingCycles(i_xOldDocProps->getEditingCycles());
        xDocPropsToFill->setEditingDuration(i_xOldDocProps->getEditingDuration());
    }
    catch (const uno::Exception&) {}

    // set the modified flag back if required
    if (bIsModified != bool(xModifiable->isModified()))
        xModifiable->setModified(bIsModified);
}

SfxItemSet*
ScVbaCellRangeAccess::GetDataSet( ScCellRangesBase* pRangeObj )
{
    return pRangeObj ? pRangeObj->GetCurrentDataSet( true ) : nullptr;
}

} // namespace ooo::vba::excel

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
