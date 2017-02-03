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

#include "workbookfragment.hxx"

#include <oox/core/filterbase.hxx>
#include <oox/core/xmlfilterbase.hxx>
#include <oox/drawingml/themefragmenthandler.hxx>
#include <oox/helper/attributelist.hxx>
#include <oox/helper/progressbar.hxx>
#include <oox/helper/propertyset.hxx>
#include <oox/ole/olestorage.hxx>
#include <oox/token/namespaces.hxx>
#include <oox/token/tokens.hxx>

#include "chartsheetfragment.hxx"
#include "connectionsfragment.hxx"
#include "externallinkbuffer.hxx"
#include "externallinkfragment.hxx"
#include "formulabuffer.hxx"
#include "pivotcachebuffer.hxx"
#include "sharedstringsbuffer.hxx"
#include "sharedstringsfragment.hxx"
#include "revisionfragment.hxx"
#include "stylesfragment.hxx"
#include "tablebuffer.hxx"
#include "themebuffer.hxx"
#include "viewsettings.hxx"
#include "workbooksettings.hxx"
#include "worksheetbuffer.hxx"
#include "worksheethelper.hxx"
#include "worksheetfragment.hxx"
#include "sheetdatacontext.hxx"
#include "extlstcontext.hxx"
#include "officecfg/Office/Common.hxx"

#include "document.hxx"
#include "docsh.hxx"
#include "calcconfig.hxx"
#include "globstr.hrc"

#include <vcl/svapp.hxx>
#include <vcl/timer.hxx>
#include <vcl/msgbox.hxx>

#include <oox/core/fastparser.hxx>
#include <salhelper/thread.hxx>
#include <comphelper/threadpool.hxx>
#include <osl/conditn.hxx>

#include <algorithm>
#include <queue>
#include <thread>
#include <memory>

#include <oox/ole/vbaproject.hxx>

#include <comphelper/processfactory.hxx>
#include <officecfg/Office/Calc.hxx>

namespace oox {
namespace xls {

using namespace ::com::sun::star::io;
using namespace ::com::sun::star::table;
using namespace ::com::sun::star::uno;
using namespace ::oox::core;

using ::oox::drawingml::ThemeFragmentHandler;

namespace {

const double PROGRESS_LENGTH_GLOBALS        = 0.1;      /// 10% of progress bar for globals import.

} // namespace

WorkbookFragment::WorkbookFragment( const WorkbookHelper& rHelper, const OUString& rFragmentPath ) :
    WorkbookFragmentBase( rHelper, rFragmentPath )
{
}

ContextHandlerRef WorkbookFragment::onCreateContext( sal_Int32 nElement, const AttributeList& rAttribs )
{
    switch( getCurrentElement() )
    {
        case XML_ROOT_CONTEXT:
            if( nElement == XLS_TOKEN( workbook ) ) return this;
        break;

        case XLS_TOKEN( workbook ):
            switch( nElement )
            {
                case XLS_TOKEN( sheets ):
                case XLS_TOKEN( bookViews ):
                case XLS_TOKEN( externalReferences ):
                case XLS_TOKEN( definedNames ):
                case XLS_TOKEN( pivotCaches ):          return this;

                case XLS_TOKEN( fileSharing ):          getWorkbookSettings().importFileSharing( rAttribs );    break;
                case XLS_TOKEN( workbookPr ):           getWorkbookSettings().importWorkbookPr( rAttribs );     break;
                case XLS_TOKEN( calcPr ):               getWorkbookSettings().importCalcPr( rAttribs );         break;
                case XLS_TOKEN( oleSize ):              getViewSettings().importOleSize( rAttribs );            break;

                case XLS_TOKEN( extLst ):               return new ExtLstGlobalWorkbookContext( *this );
            }
        break;

        case XLS_TOKEN( sheets ):
            if( nElement == XLS_TOKEN( sheet ) ) getWorksheets().importSheet( rAttribs );
        break;
        case XLS_TOKEN( bookViews ):
            if( nElement == XLS_TOKEN( workbookView ) ) getViewSettings().importWorkbookView( rAttribs );
        break;
        case XLS_TOKEN( externalReferences ):
            if( nElement == XLS_TOKEN( externalReference ) ) importExternalReference( rAttribs );
        break;
        case XLS_TOKEN( definedNames ):
            if( nElement == XLS_TOKEN( definedName ) ) { importDefinedName( rAttribs ); return this; } // collect formula
        break;
        case XLS_TOKEN( pivotCaches ):
            if( nElement == XLS_TOKEN( pivotCache ) ) importPivotCache( rAttribs );
        break;
    }
    return nullptr;
}

void WorkbookFragment::onCharacters( const OUString& rChars )
{
    if( isCurrentElement( XLS_TOKEN( definedName ) ) && mxCurrName.get() )
        mxCurrName->setFormula( rChars );
}

ContextHandlerRef WorkbookFragment::onCreateRecordContext( sal_Int32 nRecId, SequenceInputStream& rStrm )
{
    switch( getCurrentElement() )
    {
        case XML_ROOT_CONTEXT:
            if( nRecId == BIFF12_ID_WORKBOOK ) return this;
        break;

        case BIFF12_ID_WORKBOOK:
            switch( nRecId )
            {
                case BIFF12_ID_SHEETS:
                case BIFF12_ID_BOOKVIEWS:
                case BIFF12_ID_EXTERNALREFS:
                case BIFF12_ID_PIVOTCACHES:     return this;

                case BIFF12_ID_FILESHARING:     getWorkbookSettings().importFileSharing( rStrm );   break;
                case BIFF12_ID_WORKBOOKPR:      getWorkbookSettings().importWorkbookPr( rStrm );    break;
                case BIFF12_ID_CALCPR:          getWorkbookSettings().importCalcPr( rStrm );        break;
                case BIFF12_ID_OLESIZE:         getViewSettings().importOleSize( rStrm );           break;
                case BIFF12_ID_DEFINEDNAME:     getDefinedNames().importDefinedName( rStrm );       break;
            }
        break;

        case BIFF12_ID_SHEETS:
            if( nRecId == BIFF12_ID_SHEET ) getWorksheets().importSheet( rStrm );
        break;
        case BIFF12_ID_BOOKVIEWS:
            if( nRecId == BIFF12_ID_WORKBOOKVIEW ) getViewSettings().importWorkbookView( rStrm );
        break;

        case BIFF12_ID_EXTERNALREFS:
            switch( nRecId )
            {
                case BIFF12_ID_EXTERNALREF:     importExternalRef( rStrm );                         break;
                case BIFF12_ID_EXTERNALSELF:    getExternalLinks().importExternalSelf( rStrm );     break;
                case BIFF12_ID_EXTERNALSAME:    getExternalLinks().importExternalSame( rStrm );     break;
                case BIFF12_ID_EXTERNALADDIN:   getExternalLinks().importExternalAddin( rStrm );    break;
                case BIFF12_ID_EXTERNALSHEETS:  getExternalLinks().importExternalSheets( rStrm );   break;
            }
        break;

        case BIFF12_ID_PIVOTCACHES:
            if( nRecId == BIFF12_ID_PIVOTCACHE ) importPivotCache( rStrm );
    }
    return nullptr;
}

const RecordInfo* WorkbookFragment::getRecordInfos() const
{
    static const RecordInfo spRecInfos[] =
    {
        { BIFF12_ID_BOOKVIEWS,      BIFF12_ID_BOOKVIEWS + 1         },
        { BIFF12_ID_EXTERNALREFS,   BIFF12_ID_EXTERNALREFS + 1      },
        { BIFF12_ID_FUNCTIONGROUPS, BIFF12_ID_FUNCTIONGROUPS + 2    },
        { BIFF12_ID_PIVOTCACHE,     BIFF12_ID_PIVOTCACHE + 1        },
        { BIFF12_ID_PIVOTCACHES,    BIFF12_ID_PIVOTCACHES + 1       },
        { BIFF12_ID_SHEETS,         BIFF12_ID_SHEETS + 1            },
        { BIFF12_ID_WORKBOOK,       BIFF12_ID_WORKBOOK + 1          },
        { -1,                       -1                              }
    };
    return spRecInfos;
}

namespace {

typedef std::pair<WorksheetGlobalsRef, FragmentHandlerRef> SheetFragmentHandler;
typedef std::vector<SheetFragmentHandler> SheetFragmentVector;

class WorkerThread : public comphelper::ThreadTask
{
    sal_Int32 &mrSheetsLeft;
    WorkbookFragment& mrWorkbookHandler;
    rtl::Reference<FragmentHandler> mxHandler;

public:
    WorkerThread( const std::shared_ptr<comphelper::ThreadTaskTag> & pTag,
                  WorkbookFragment& rWorkbookHandler,
                  const rtl::Reference<FragmentHandler>& xHandler,
                  sal_Int32 &rSheetsLeft ) :
        comphelper::ThreadTask( pTag ),
        mrSheetsLeft( rSheetsLeft ),
        mrWorkbookHandler( rWorkbookHandler ),
        mxHandler( xHandler )
    {
    }

    virtual void doWork() override
    {
        // We hold the solar mutex in all threads except for
        // the small safe section of the inner loop in
        // sheetdatacontext.cxx
        SAL_INFO( "sc.filter",  "start wait on solar\n" );
        SolarMutexGuard aGuard;
        SAL_INFO( "sc.filter",  "got solar\n" );

        std::unique_ptr<oox::core::FastParser> xParser(
                oox::core::XmlFilterBase::createParser() );

        SAL_INFO( "sc.filter",  "start import\n" );
        mrWorkbookHandler.importOoxFragment( mxHandler, *xParser );
        SAL_INFO( "sc.filter",  "end import, release solar\n" );
        mrSheetsLeft--;
        assert( mrSheetsLeft >= 0 );
        if( mrSheetsLeft == 0 )
            Application::EndYield();
    }
};

class ProgressBarTimer : private Timer
{
    // FIXME: really we should unify all sheet loading
    // progress reporting into something pleasant.
    class ProgressWrapper : public ISegmentProgressBar
    {
        double mfPosition;
        ISegmentProgressBarRef mxWrapped;
    public:
        explicit ProgressWrapper(const ISegmentProgressBarRef &xRef)
            : mfPosition(0.0)
            , mxWrapped(xRef)
        {
        }

        // IProgressBar
        virtual double getPosition() const override { return mfPosition; }
        virtual void   setPosition( double fPosition ) override { mfPosition = fPosition; }
        // ISegmentProgressBar
        virtual double getFreeLength() const override { return 0.0; }
        virtual ISegmentProgressBarRef createSegment( double /* fLength */ ) override
        {
            return ISegmentProgressBarRef();
        }
        void UpdateBar()
        {
            mxWrapped->setPosition( mfPosition );
        }
    };
    std::vector< ISegmentProgressBarRef > aSegments;
public:
    ProgressBarTimer() : Timer()
    {
        SetTimeout( 500 );
    }
    virtual ~ProgressBarTimer() override
    {
        aSegments.clear();
    }
    ISegmentProgressBarRef wrapProgress( const ISegmentProgressBarRef &xProgress )
    {
        aSegments.push_back( ISegmentProgressBarRef( new ProgressWrapper( xProgress ) ) );
        return aSegments.back();
    }
    virtual void Invoke() override
    {
        for(std::shared_ptr<ISegmentProgressBar> & pSegment : aSegments)
            static_cast< ProgressWrapper *>( pSegment.get() )->UpdateBar();
    }
};

void importSheetFragments( WorkbookFragment& rWorkbookHandler, SheetFragmentVector& rSheets )
{
    rWorkbookHandler.getDocImport().initForSheets();

    Reference< XComponentContext > xContext = comphelper::getProcessComponentContext();

    // test sequential read in this mode
    comphelper::ThreadPool &rSharedPool = comphelper::ThreadPool::getSharedOptimalPool();
    std::shared_ptr<comphelper::ThreadTaskTag> pTag = comphelper::ThreadPool::createThreadTaskTag();

    sal_Int32 nSheetsLeft = 0;
    ProgressBarTimer aProgressUpdater;
    SheetFragmentVector::iterator it = rSheets.begin(), itEnd = rSheets.end();
    for( ; it != itEnd; ++it )
    {
         // getting at the WorksheetGlobals is rather unpleasant
         IWorksheetProgress *pProgress = WorksheetHelper::getWorksheetInterface( it->first );
         pProgress->setCustomRowProgress(
                     aProgressUpdater.wrapProgress(
                             pProgress->getRowProgress() ) );
         rSharedPool.pushTask( new WorkerThread( pTag, rWorkbookHandler, it->second,
                                           /* ref */ nSheetsLeft ) );
         nSheetsLeft++;
     }

     // coverity[loop_top] - this isn't an infinite loop where nSheetsLeft gets decremented by the above threads
     while( nSheetsLeft > 0)
     {
         // This is a much more controlled re-enterancy hazard than
         // allowing a yield deeper inside the filter code for progress
         // bar updating.
         Application::Yield();
     }
     rSharedPool.waitUntilDone(pTag);

     // threads joined in ThreadPool destructor
}

}

void WorkbookFragment::finalizeImport()
{
    ISegmentProgressBarRef xGlobalSegment = getProgressBar().createSegment( PROGRESS_LENGTH_GLOBALS );

    // read the theme substream
    OUString aThemeFragmentPath = getFragmentPathFromFirstTypeFromOfficeDoc( "theme" );
    if( !aThemeFragmentPath.isEmpty() )
        importOoxFragment( new ThemeFragmentHandler( getFilter(), aThemeFragmentPath, getTheme() ) );
    xGlobalSegment->setPosition( 0.25 );

    // read the styles substream (requires finalized theme buffer)
    OUString aStylesFragmentPath = getFragmentPathFromFirstTypeFromOfficeDoc( "styles" );
    if( !aStylesFragmentPath.isEmpty() )
        importOoxFragment( new StylesFragment( *this, aStylesFragmentPath ) );
    xGlobalSegment->setPosition( 0.5 );

    // read the shared string table substream (requires finalized styles buffer)
    OUString aSstFragmentPath = getFragmentPathFromFirstTypeFromOfficeDoc( "sharedStrings" );
    if( !aSstFragmentPath.isEmpty() )
        if (!importOoxFragment( new SharedStringsFragment( *this, aSstFragmentPath ) ))
            importOoxFragment(new SharedStringsFragment(*this, aSstFragmentPath.replaceFirst("sharedStrings","SharedStrings")));
    xGlobalSegment->setPosition( 0.75 );

    // read the connections substream
    OUString aConnFragmentPath = getFragmentPathFromFirstTypeFromOfficeDoc( "connections" );
    if( !aConnFragmentPath.isEmpty() )
        importOoxFragment( new ConnectionsFragment( *this, aConnFragmentPath ) );
    xGlobalSegment->setPosition( 1.0 );

    /*  Create fragments for all sheets, before importing them. Needed to do
        some preprocessing in the fragment constructors, e.g. loading the table
        fragments for all sheets that are needed before the cell formulas are
        loaded. Additionally, the instances of the WorkbookGlobals structures
        have to be stored for every sheet. */
    SheetFragmentVector aSheetFragments;
    std::vector<WorksheetHelper*> aHelpers;
    WorksheetBuffer& rWorksheets = getWorksheets();
    sal_Int32 nWorksheetCount = rWorksheets.getWorksheetCount();
    for( sal_Int32 nWorksheet = 0; nWorksheet < nWorksheetCount; ++nWorksheet )
    {
        sal_Int16 nCalcSheet = rWorksheets.getCalcSheetIndex( nWorksheet );
        const Relation* pRelation = getRelations().getRelationFromRelId( rWorksheets.getWorksheetRelId( nWorksheet ) );
        if( (nCalcSheet >= 0) && pRelation )
        {
            // get fragment path of the sheet
            OUString aFragmentPath = getFragmentPathFromRelation( *pRelation );
            OSL_ENSURE( !aFragmentPath.isEmpty(), "WorkbookFragment::finalizeImport - cannot access sheet fragment" );
            if( !aFragmentPath.isEmpty() )
            {
                // leave space for formula processing ( calculate the segments as
                // if there is an extra sheet )
                double fSegmentLength = getProgressBar().getFreeLength() / (nWorksheetCount - ( nWorksheet - 1) );
                ISegmentProgressBarRef xSheetSegment = getProgressBar().createSegment( fSegmentLength );

                // get the sheet type according to the relations type
                WorksheetType eSheetType = SHEETTYPE_EMPTYSHEET;
                if( pRelation->maType == CREATE_OFFICEDOC_RELATION_TYPE( "worksheet" ) ||
                        pRelation->maType == CREATE_OFFICEDOC_RELATION_TYPE_STRICT( "worksheet" ))
                    eSheetType = SHEETTYPE_WORKSHEET;
                else if( pRelation->maType == CREATE_OFFICEDOC_RELATION_TYPE( "chartsheet" ) ||
                        pRelation->maType == CREATE_OFFICEDOC_RELATION_TYPE_STRICT( "chartsheet" ))
                    eSheetType = SHEETTYPE_CHARTSHEET;
                else if( (pRelation->maType == CREATE_MSOFFICE_RELATION_TYPE( "xlMacrosheet" )) ||
                         (pRelation->maType == CREATE_MSOFFICE_RELATION_TYPE( "xlIntlMacrosheet" )) )
                    eSheetType = SHEETTYPE_MACROSHEET;
                else if( pRelation->maType == CREATE_OFFICEDOC_RELATION_TYPE( "dialogsheet" ) ||
                        pRelation->maType == CREATE_OFFICEDOC_RELATION_TYPE_STRICT(" dialogsheet" ))
                    eSheetType = SHEETTYPE_DIALOGSHEET;
                OSL_ENSURE( eSheetType != SHEETTYPE_EMPTYSHEET, "WorkbookFragment::finalizeImport - unknown sheet type" );
                if( eSheetType != SHEETTYPE_EMPTYSHEET )
                {
                    // create the WorksheetGlobals object
                    WorksheetGlobalsRef xSheetGlob = WorksheetHelper::constructGlobals( *this, xSheetSegment, eSheetType, nCalcSheet );
                    OSL_ENSURE( xSheetGlob.get(), "WorkbookFragment::finalizeImport - missing sheet in document" );
                    if( xSheetGlob.get() )
                    {
                        // create the sheet fragment handler
                        ::rtl::Reference< WorksheetFragmentBase > xFragment;
                        switch( eSheetType )
                        {
                            case SHEETTYPE_WORKSHEET:
                            case SHEETTYPE_MACROSHEET:
                            case SHEETTYPE_DIALOGSHEET:
                                xFragment.set( new WorksheetFragment( *xSheetGlob, aFragmentPath ) );
                            break;
                            case SHEETTYPE_CHARTSHEET:
                                xFragment.set( new ChartsheetFragment( *xSheetGlob, aFragmentPath ) );
                            break;
                            // coverity[dead_error_begin] - following conditions exist to avoid compiler warning
                            case SHEETTYPE_EMPTYSHEET:
                            case SHEETTYPE_MODULESHEET:
                                break;
                        }

                        // insert the fragment into the map
                        if( xFragment.is() )
                        {
                            aSheetFragments.push_back( SheetFragmentHandler( xSheetGlob, xFragment.get() ) );
                            aHelpers.push_back(xFragment.get());
                        }
                    }
                }
            }
        }
    }

    // setup structure sizes for the number of sheets
    getFormulaBuffer().SetSheetCount( aSheetFragments.size() );

    // create all database ranges and defined names, in that order
    getTables().finalizeImport();
    getTables().applyTableColumns();
    getDefinedNames().finalizeImport();
    // open the VBA project storage
    OUString aVbaFragmentPath = getFragmentPathFromFirstType( CREATE_MSOFFICE_RELATION_TYPE( "vbaProject" ) );
    if( !aVbaFragmentPath.isEmpty() )
    {
        Reference< XInputStream > xInStrm = getBaseFilter().openInputStream( aVbaFragmentPath );
        if( xInStrm.is() )
        {
            StorageRef xPrjStrg( new ::oox::ole::OleStorage( getBaseFilter().getComponentContext(), xInStrm, false ) );
            setVbaProjectStorage( xPrjStrg );
            getBaseFilter().getVbaProject().readVbaModules( *xPrjStrg );
        }
    }

    // load all worksheets
    importSheetFragments(*this, aSheetFragments);

    sal_Int16 nActiveSheet = getViewSettings().getActiveCalcSheet();
    getWorksheets().finalizeImport( nActiveSheet );

    // final conversions, e.g. calculation settings and view settings
    finalizeWorkbookImport();
    //
    //stop preventing establishment of listeners as is done in
    //ScDocShell::AfterXMLLoading() for ods
    getScDocument().SetInsertingFromOtherDoc(false);
    getDocImport().finalize();

    recalcFormulaCells();

    for( std::vector<WorksheetHelper*>::iterator aIt = aHelpers.begin(), aEnd = aHelpers.end(); aIt != aEnd; ++aIt )
    {
        (*aIt)->finalizeDrawingImport();
    }

    for( SheetFragmentVector::iterator aIt = aSheetFragments.begin(), aEnd = aSheetFragments.end(); aIt != aEnd; ++aIt )
    {
        // delete fragment object and WorkbookGlobals object, will free all allocated sheet buffers
        aIt->second.clear();
        aIt->first.reset();
    }

    OUString aRevHeadersPath = getFragmentPathFromFirstType(CREATE_OFFICEDOC_RELATION_TYPE("revisionHeaders"));
    if (!aRevHeadersPath.isEmpty())
    {
        std::unique_ptr<oox::core::FastParser> xParser(oox::core::XmlFilterBase::createParser());
        rtl::Reference<oox::core::FragmentHandler> xFragment(new RevisionHeadersFragment(*this, aRevHeadersPath));
        importOoxFragment(xFragment, *xParser);
    }

    // attach macros to registered objects now that all objects have been created.
    getBaseFilter().getVbaProject().attachMacros();
}

namespace {

ScDocShell& getDocShell(ScDocument& rDoc)
{
    return static_cast<ScDocShell&>(*rDoc.GetDocumentShell());
}

}

void WorkbookFragment::recalcFormulaCells()
{
    // Recalculate formula cells.
    ScDocument& rDoc = getScDocument();
    ScDocShell& rDocSh = getDocShell(rDoc);
    Reference< XComponentContext > xContext = comphelper::getProcessComponentContext();
    ScRecalcOptions nRecalcMode =
        static_cast<ScRecalcOptions>(officecfg::Office::Calc::Formula::Load::OOXMLRecalcMode::get(xContext));
    bool bHardRecalc = false;
    if (nRecalcMode == RECALC_ASK)
    {
        if (rDoc.IsUserInteractionEnabled())
        {
            // Ask the user if full re-calculation is desired.
            ScopedVclPtrInstance<QueryBox> aBox(
                ScDocShell::GetActiveDialogParent(), WinBits(WB_YES_NO | WB_DEF_YES),
                ScGlobal::GetRscString(STR_QUERY_FORMULA_RECALC_ONLOAD_XLS));
            aBox->SetCheckBoxText(ScGlobal::GetRscString(STR_ALWAYS_PERFORM_SELECTED));

            sal_Int32 nRet = aBox->Execute();
            bHardRecalc = nRet == RET_YES;

            if (aBox->GetCheckBoxState())
            {
                // Always perform selected action in the future.
                std::shared_ptr<comphelper::ConfigurationChanges> batch(comphelper::ConfigurationChanges::create());
                officecfg::Office::Calc::Formula::Load::OOXMLRecalcMode::set(sal_Int32(0), batch);
                ScFormulaOptions aOpt = SC_MOD()->GetFormulaOptions();
                aOpt.SetOOXMLRecalcOptions(bHardRecalc ? RECALC_ALWAYS : RECALC_NEVER);
                /* XXX  is this really supposed to set the ScModule options?
                 *      Not the ScDocShell options? */
                SC_MOD()->SetFormulaOptions(aOpt);

                batch->commit();
            }
        }
    }
    else if (nRecalcMode == RECALC_ALWAYS)
        bHardRecalc = true;

    if (bHardRecalc)
        rDocSh.DoHardRecalc(false);
    else
        rDoc.CalcFormulaTree(false, true, false);
}

// private --------------------------------------------------------------------

void WorkbookFragment::importExternalReference( const AttributeList& rAttribs )
{
    if( ExternalLink* pExtLink = getExternalLinks().importExternalReference( rAttribs ).get() )
        importExternalLinkFragment( *pExtLink );
}

void WorkbookFragment::importDefinedName( const AttributeList& rAttribs )
{
    mxCurrName = getDefinedNames().importDefinedName( rAttribs );
}

void WorkbookFragment::importPivotCache( const AttributeList& rAttribs )
{
    sal_Int32 nCacheId = rAttribs.getInteger( XML_cacheId, -1 );
    OUString aRelId = rAttribs.getString( R_TOKEN( id ), OUString() );
    importPivotCacheDefFragment( aRelId, nCacheId );
}

void WorkbookFragment::importExternalRef( SequenceInputStream& rStrm )
{
    if( ExternalLink* pExtLink = getExternalLinks().importExternalRef( rStrm ).get() )
        importExternalLinkFragment( *pExtLink );
}

void WorkbookFragment::importPivotCache( SequenceInputStream& rStrm )
{
    sal_Int32 nCacheId = rStrm.readInt32();
    OUString aRelId = BiffHelper::readString( rStrm );
    importPivotCacheDefFragment( aRelId, nCacheId );
}

void WorkbookFragment::importExternalLinkFragment( ExternalLink& rExtLink )
{
    OUString aFragmentPath = getFragmentPathFromRelId( rExtLink.getRelId() );
    if( !aFragmentPath.isEmpty() )
        importOoxFragment( new ExternalLinkFragment( *this, aFragmentPath, rExtLink ) );
}

void WorkbookFragment::importPivotCacheDefFragment( const OUString& rRelId, sal_Int32 nCacheId )
{
    // pivot caches will be imported on demand, here we just store the fragment path in the buffer
    getPivotCaches().registerPivotCacheFragment( nCacheId, getFragmentPathFromRelId( rRelId ) );
}

} // namespace xls
} // namespace oox

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
