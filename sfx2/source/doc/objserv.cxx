/*************************************************************************
 *
 *  $RCSfile: objserv.cxx,v $
 *
 *  $Revision: 1.58 $
 *
 *  last change: $Author: rt $ $Date: 2003-09-19 08:01:31 $
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
#include <so3/svstor.hxx>

#ifndef  _COM_SUN_STAR_UI_DIALOGS_EXTENDEDFILEPICKERELEMENTIDS_HPP_
#include <com/sun/star/ui/dialogs/ExtendedFilePickerElementIds.hpp>
#endif

#ifndef  _COM_SUN_STAR_UI_DIALOGS_XFILEPICKERCONTROLACCESS_HPP_
#include <com/sun/star/ui/dialogs/XFilePickerControlAccess.hpp>
#endif

#ifndef  _COM_SUN_STAR_UI_DIALOGS_COMMONFILEPICKERELEMENTIDS_HPP_
#include <com/sun/star/ui/dialogs/CommonFilePickerElementIds.hpp>
#endif

#ifndef _COM_SUN_STAR_UI_DIALOGS_XCONTROLACCESS_HPP_
#include <com/sun/star/ui/dialogs/XControlAccess.hpp>
#endif

#ifndef _COM_SUN_STAR_LANG_XMULTISERVICEFACTORY_HPP_
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#endif

#ifndef _COM_SUN_STAR_BEANS_XPROPERTYACCESS_HPP_
#include <com/sun/star/beans/XPropertyAccess.hpp>
#endif

#ifndef _COM_SUN_STAR_BEANS_PROPERTYVALUE_HPP_
#include <com/sun/star/beans/PropertyValue.hpp>
#endif

#ifndef _COM_SUN_STAR_CONTAINER_XNAMEACCESS_HPP_
#include <com/sun/star/container/XNameAccess.hpp>
#endif

#ifndef _COM_SUN_STAR_DOCUMENT_XEXPORTER_HPP_
#include <com/sun/star/document/XExporter.hpp>
#endif

#ifndef _COM_SUN_STAR_TASK_XINTERACTIONHANDLER_HPP_
#include <com/sun/star/task/XInteractionHandler.hpp>
#endif

#ifndef _UNOTOOLS_PROCESSFACTORY_HXX_
#include <comphelper/processfactory.hxx>
#endif

#ifndef _URLOBJ_HXX //autogen
#include <tools/urlobj.hxx>
#endif
#if SUPD<613//MUSTINI
    #ifndef _SFX_INIMGR_HXX //autogen
    #include <inimgr.hxx>
    #endif
#endif
#ifndef _SFX_WHITER_HXX //autogen
#include <svtools/whiter.hxx>
#endif
#if SUPD<613//MUSTINI
    #ifndef _SFXINIMGR_HXX //autogen
    #include <svtools/iniman.hxx>
    #endif
#endif
#ifndef _MSGBOX_HXX //autogen
#include <vcl/msgbox.hxx>
#endif
#ifndef _SFXINTITEM_HXX //autogen
#include <svtools/intitem.hxx>
#endif
#ifndef _SFXENUMITEM_HXX //autogen
#include <svtools/eitem.hxx>
#endif
#include <vcl/wrkwin.hxx>
#ifndef _SFXECODE_HXX
#include <svtools/sfxecode.hxx>
#endif
#ifndef _EHDL_HXX
#include <svtools/ehdl.hxx>
#endif

#include <svtools/sbx.hxx>
#include <svtools/pathoptions.hxx>
#include <svtools/useroptions.hxx>
#include <svtools/asynclink.hxx>
#include <svtools/saveopt.hxx>

#pragma hdrstop

#include "sfxresid.hxx"
#include "event.hxx"
#include "request.hxx"
#include "printer.hxx"
#include "viewsh.hxx"
#include "doctdlg.hxx"
#include "docfilt.hxx"
#include "docfile.hxx"
#include "docinf.hxx"
#include "dispatch.hxx"
#include "dinfdlg.hxx"
#include "objitem.hxx"
#include "objsh.hxx"
#include "objshimp.hxx"
#include "sfxtypes.hxx"
#include "interno.hxx"
#include "module.hxx"
#include "topfrm.hxx"
#include "versdlg.hxx"
#include "doc.hrc"
#include "docfac.hxx"
#include "fcontnr.hxx"
#include "filedlghelper.hxx"
#include "sfxhelp.hxx"

#ifndef _SFX_HELPID_HRC
#include "helpid.hrc"
#endif

using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::ui::dialogs;
using namespace ::com::sun::star::awt;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::document;
using namespace ::com::sun::star::task;

//====================================================================

class SfxSaveAsContext_Impl
{
    String&     _rNewNameVar;
    String      _aNewName;

public:
                SfxSaveAsContext_Impl( String &rNewNameVar,
                                       const String &rNewName )
                :   _rNewNameVar( rNewNameVar ),
                    _aNewName( rNewName )
                { rNewNameVar = rNewName; }
                ~SfxSaveAsContext_Impl()
                { _rNewNameVar.Erase(); }
};

//====================================================================

BOOL ShallSetBaseURL_Impl( SfxMedium &rMed );

#define SfxObjectShell
#include "sfxslots.hxx"

svtools::AsynchronLink* pPendingCloser = 0;

//=========================================================================



SFX_IMPL_INTERFACE(SfxObjectShell,SfxShell,SfxResId(0))
{
}

long SfxObjectShellClose_Impl( void* pObj, void* pArg )
{
    SfxObjectShell *pObjSh = (SfxObjectShell*) pArg;
    if ( pObjSh->Get_Impl()->bHiddenLockedByAPI )
    {
        pObjSh->Get_Impl()->bHiddenLockedByAPI = FALSE;
        pObjSh->OwnerLock(FALSE);
    }
    else if ( !pObjSh->Get_Impl()->bClosing )
        // GCC stuerzt ab, wenn schon im dtor, also vorher Flag abfragen
        pObjSh->DoClose();
    return 0;
}

//=========================================================================

void SfxObjectShell::PrintExec_Impl(SfxRequest &rReq)
{
    SfxViewFrame *pFrame = SfxViewFrame::GetFirst(this);
    if ( pFrame )
    {
        rReq.SetSlot( SID_PRINTDOC );
        pFrame->GetViewShell()->ExecuteSlot(rReq);
    }
}

//--------------------------------------------------------------------

void SfxObjectShell::PrintState_Impl(SfxItemSet &rSet)
{
    FASTBOOL bPrinting = FALSE;
    SfxViewFrame *pFrame = SfxViewFrame::GetFirst(this, TYPE(SfxTopViewFrame));
    if ( pFrame )
    {
        SfxPrinter *pPrinter = pFrame->GetViewShell()->GetPrinter();
        bPrinting = pPrinter && pPrinter->IsPrinting();
    }
    rSet.Put( SfxBoolItem( SID_PRINTOUT, bPrinting ) );
}

//--------------------------------------------------------------------

sal_Bool SfxObjectShell::APISaveAs_Impl
(
    const String& aFileName,
    SfxItemSet*   aParams
)
{
    BOOL bOk = sal_False;

    {DBG_CHKTHIS(SfxObjectShell, 0);}

    pImp->bSetStandardName=FALSE;
    if ( GetMedium() )
    {
        SFX_ITEMSET_ARG( aParams, pSaveToItem, SfxBoolItem, SID_SAVETO, sal_False );
        sal_Bool bSaveTo = pSaveToItem && pSaveToItem->GetValue();

        String aFilterName;
        SFX_ITEMSET_ARG( aParams, pFilterNameItem, SfxStringItem, SID_FILTER_NAME, sal_False );
        if( pFilterNameItem )
            aFilterName = pFilterNameItem->GetValue();

        // in case no filter defined use default one
        if( !aFilterName.Len() )
        {
            sal_uInt16 nActFilt = 0;
            for( const SfxFilter* pFilt = GetFactory().GetFilter( 0 );
                 pFilt && ( !pFilt->CanExport()
                  || !bSaveTo && !pFilt->CanImport() // SaveAs case
                  || pFilt->IsInternal() );
                  pFilt = GetFactory().GetFilter( ++nActFilt ) );

            DBG_ASSERT( pFilt, "No default filter!\n" );

            if( pFilt )
                aFilterName = pFilt->GetFilterName();

            aParams->Put(SfxStringItem( SID_FILTER_NAME, aFilterName));
        }


        {
            SfxObjectShellRef xLock( this ); // ???

            // since saving a document modified its DocumentInfo, the current DocumentInfo must be saved on "SaveTo", because
            // it must be restored after saving
            SfxDocumentInfo aSavedInfo;
            sal_Bool bCopyTo =  bSaveTo || GetCreateMode() == SFX_CREATE_MODE_EMBEDDED;
            if ( bCopyTo )
                aSavedInfo = GetDocInfo();

            bOk = CommonSaveAs_Impl( aFileName, aFilterName, aParams );

            if ( bCopyTo )
            {
                // restore DocumentInfo if only a copy was created
                SfxDocumentInfo &rDocInfo = GetDocInfo();
                rDocInfo = aSavedInfo;
            }
        }

        // Picklisten-Eintrag verhindern
        GetMedium()->SetUpdatePickList( FALSE );
    }

    return bOk;
}

//-------------------------------------------------------------------------

sal_Bool SfxObjectShell::GUISaveAs_Impl(sal_Bool bUrl, SfxRequest *pRequest)
{
    INetURLObject aURL;

    SFX_REQUEST_ARG( (*pRequest), pSaveToItem, SfxBoolItem, SID_SAVETO, sal_False );

    sal_Bool bSaveTo = pSaveToItem ? pSaveToItem->GetValue() : sal_False;
    sal_Bool bIsPDFExport = (( pRequest->GetSlot() == SID_EXPORTDOCASPDF ) ||
                            ( pRequest->GetSlot() == SID_DIRECTEXPORTDOCASPDF ));
    sal_Bool bIsExport = ( pRequest->GetSlot() == SID_EXPORTDOC ) || bIsPDFExport;
    sal_Bool bSuppressFilterOptionsDialog = sal_False;

    // Parameter to return if user cancelled a optional configuration dialog and
    // there for cancelled the whole save procedure.
    DBG_ASSERT( !bIsExport || bSaveTo, "Export mode should use SaveTo mechanics!\n" );

    const SfxFilter* pFilt = NULL;
    if ( pRequest->GetSlot() == SID_EXPORTDOCASPDF ||
         pRequest->GetSlot() == SID_DIRECTEXPORTDOCASPDF )
    {
        // Preselect PDF-Filter for EXPORT
        pFilt = GetFactory().GetFilterContainer()->GetFilter4Extension( String::CreateFromAscii( ".pdf" ), SFX_FILTER_EXPORT );
    }
    else
    {
        sal_uInt16 nActFilt = 0;
        for( pFilt = GetFactory().GetFilter( 0 );
            pFilt && ( !pFilt->CanExport()
                    || bIsExport && pFilt->CanImport() // Export case ( only for GUI )
                    || !bSaveTo && !pFilt->CanImport() // SaveAs case
                    || pFilt->IsInternal() );
            pFilt = GetFactory().GetFilter( ++nActFilt ) );
    }

    DBG_ASSERT( pFilt, "Kein Filter zum Speichern" );
    if ( !pFilt )
        return sal_False;

    String aFilterName;
    if( pFilt )
        aFilterName = pFilt->GetFilterName();

    SfxItemSet *pParams = new SfxAllItemSet( SFX_APP()->GetPool() );
    SFX_REQUEST_ARG( (*pRequest), pFileNameItem, SfxStringItem, SID_FILE_NAME, sal_False );
    if ( pRequest->GetArgs() )
        pParams->Put( *pRequest->GetArgs() );

    SfxItemSet* pMedSet = pMedium->GetItemSet();
    SFX_ITEMSET_ARG( pMedSet, pOptionsItem, SfxStringItem, SID_FILE_FILTEROPTIONS, sal_False );
    if ( pOptionsItem && pParams->GetItemState(SID_FILE_FILTEROPTIONS) != SFX_ITEM_SET )
        pParams->Put( *pOptionsItem );

    SFX_ITEMSET_ARG( pMedSet, pDataItem, SfxUnoAnyItem, SID_FILTER_DATA, sal_False );
    if ( pDataItem && pParams->GetItemState(SID_FILTER_DATA) != SFX_ITEM_SET )
        pParams->Put( *pDataItem );

    sal_Bool bDialogUsed = sal_False;
    sal_Bool bUseFilterOptions = sal_False;

    Reference< XMultiServiceFactory > xServiceManager = ::comphelper::getProcessServiceFactory();
    Reference< XNameAccess > xFilterCFG;
    if( xServiceManager.is() )
    {
        xFilterCFG = Reference< XNameAccess >(
            xServiceManager->createInstance( ::rtl::OUString::createFromAscii( "com.sun.star.document.FilterFactory" ) ),
            UNO_QUERY );
    }

    if ( !pFileNameItem )
    {
        // we need to show the file dialog
        bDialogUsed = sal_True;
        if(! bUrl )
        {
            // check if we have a filter which allows for filter options, so we need a corresponding checkbox in the dialog
            sal_Bool bAllowOptions = sal_False;
            const SfxFilter* pFilter;
            SfxFilterFlags nMust = SFX_FILTER_EXPORT | ( bSaveTo ? 0 : SFX_FILTER_IMPORT );
            SfxFilterFlags nDont = SFX_FILTER_INTERNAL | SFX_FILTER_NOTINFILEDLG | ( bIsExport ? SFX_FILTER_IMPORT : 0 );

            SfxFilterMatcher aMatcher( GetFactory().GetFactoryName() );
            SfxFilterMatcherIter aIter( &aMatcher, nMust, nDont );

            if( !bIsExport )
            {
                // in case of Export, filter options dialog is used if available
                for ( pFilter = aIter.First(); pFilter && !bAllowOptions; pFilter = aIter.Next() )
                {
                    if( xFilterCFG.is() )
                    {
                        try {
                            Sequence < PropertyValue > aProps;
                               Any aAny = xFilterCFG->getByName( pFilter->GetName() );
                               if ( aAny >>= aProps )
                               {
                                   ::rtl::OUString aServiceName;
                                   sal_Int32 nPropertyCount = aProps.getLength();
                                   for( sal_Int32 nProperty=0; nProperty < nPropertyCount; ++nProperty )
                                       if( aProps[nProperty].Name.equals( ::rtl::OUString::createFromAscii("UIComponent")) )
                                       {
                                        ::rtl::OUString aServiceName;
                                           aProps[nProperty].Value >>= aServiceName;
                                        if( aServiceName.getLength() )
                                            bAllowOptions = sal_True;
                                    }
                            }
                        }
                        catch( Exception& )
                        {
                        }
                    }
                }
            }

            // get the filename by dialog ...
            // create the file dialog
            sal_Int16  aDialogMode = bAllowOptions ?
                                        ::sfx2::FILESAVE_AUTOEXTENSION_PASSWORD_FILTEROPTIONS :
                                        ::sfx2::FILESAVE_AUTOEXTENSION_PASSWORD;
            sal_uInt32 aDialogFlags = 0;

            if( bIsExport )
            {
                aDialogMode  = ::sfx2::FILESAVE_AUTOEXTENSION_SELECTION;
                aDialogFlags = SFXWB_EXPORT;
            }

            sfx2::FileDialogHelper* pFileDlg = NULL;

            if ( bIsExport || bIsPDFExport )
            {
                // This is the normal dialog
                SfxObjectFactory& rFact = GetFactory();
                pFileDlg = new sfx2::FileDialogHelper( aDialogMode, aDialogFlags, rFact.GetFactoryName(), nMust, nDont );

                if( strcmp( rFact.GetShortName(), "sdraw" ) != 0 )
                    pFileDlg->SetContext( sfx2::FileDialogHelper::SD_EXPORT );
                else if( strcmp( rFact.GetShortName(), "simpress" ) != 0 )
                    pFileDlg->SetContext( sfx2::FileDialogHelper::SI_EXPORT );

                pFileDlg->CreateMatcher( rFact.GetFactoryName() );
                Reference< ::com::sun::star::ui::dialogs::XFilePicker > xFilePicker = pFileDlg->GetFilePicker();
                Reference< ::com::sun::star::ui::dialogs::XFilePickerControlAccess > xControlAccess =
                    Reference< ::com::sun::star::ui::dialogs::XFilePickerControlAccess >( xFilePicker, UNO_QUERY );

                if ( xControlAccess.is() )
                {
                    String aResStr = String( SfxResId( STR_EXPORTBUTTON ));
                    ::rtl::OUString aCtrlText = aResStr;
                    xControlAccess->setLabel( ::com::sun::star::ui::dialogs::CommonFilePickerElementIds::PUSHBUTTON_OK, aCtrlText );
                    aResStr = String( SfxResId( STR_LABEL_FILEFORMAT ));
                    aCtrlText = aResStr;
                    xControlAccess->setLabel( ::com::sun::star::ui::dialogs::CommonFilePickerElementIds::LISTBOX_FILTER_LABEL, aCtrlText );
                }

                if ( bIsPDFExport && pFilt )
                    pFileDlg->SetCurrentFilter( pFilt->GetUIName() );
            }
            else
            {
                // This is the normal dialog
                pFileDlg = new sfx2::FileDialogHelper( aDialogMode, aDialogFlags, GetFactory().GetFactoryName(), nMust, nDont );
                pFileDlg->CreateMatcher( GetFactory().GetFactoryName() );
            }

            SFX_ITEMSET_ARG( pMedSet, pRepairItem, SfxBoolItem, SID_REPAIRPACKAGE, sal_False );
            if ( HasName() && !( pRepairItem && pRepairItem->GetValue() ) )
            {
                String aLastName = QueryTitle( SFX_TITLE_QUERY_SAVE_NAME_PROPOSAL );
                const SfxFilter* pMedFilter = GetMedium()->GetFilter();
                if( pImp->bSetStandardName && !IsTemplate()
                    || !pMedFilter
                    || !pMedFilter->CanExport()
                    || bIsExport && pMedFilter->CanImport() // Export case ( only for GUI )
                     || !bSaveTo && !pMedFilter->CanImport() // SaveAs case
                    /*!!!|| pMedFilter->GetVersion() != SOFFICE_FILEFORMAT_CURRENT*/ )
                {
                    if( aLastName.Len() )
                    {
                        String aPath( aLastName );
                        bool bWasAbsolute = sal_False;
                        INetURLObject aObj( SvtPathOptions().GetWorkPath() );
                        aObj.setFinalSlash();
                        aObj = INetURLObject( aObj.RelToAbs( aPath, bWasAbsolute ) );
                        aObj.SetExtension( pFilt->GetDefaultExtension().Copy(2) );
                        pFileDlg->SetDisplayDirectory( aObj.GetMainURL( INetURLObject::NO_DECODE ) );
                    }

                    pFileDlg->SetCurrentFilter( pFilt->GetUIName() );
                }
                else
                {
                    if( aLastName.Len() )
                        pFileDlg->SetDisplayDirectory( aLastName );
                    pFileDlg->SetCurrentFilter( pMedFilter->GetUIName() );
                }
            }
            else
            {
                pFileDlg->SetDisplayDirectory( SvtPathOptions().GetWorkPath() );
            }

            SFX_ITEMSET_ARG( GetMedium()->GetItemSet(), pPassItem, SfxStringItem, SID_PASSWORD, FALSE );
            if ( pPassItem != NULL )
                pParams->Put( SfxStringItem( SID_PASSWORD, ::rtl::OUString() ) );

            Reference < ::com::sun::star::view::XSelectionSupplier > xSel( GetModel()->getCurrentController(), UNO_QUERY );
            if ( xSel.is() && xSel->getSelection().hasValue() )
                pParams->Put( SfxBoolItem( SID_SELECTION, TRUE ) );

            if ( pFileDlg->Execute( pParams, aFilterName ) != ERRCODE_NONE )
            {
                SetError(ERRCODE_IO_ABORT);
                delete pFileDlg;
                return sal_False;
            }

            // get the path from the dialog
            aURL.SetURL( pFileDlg->GetPath() );

            // gibt es schon ein Doc mit dem Namen?
            if ( aURL.GetProtocol() != INET_PROT_NOT_VALID )
            {
                SfxObjectShell* pDoc = 0;
                for ( SfxObjectShell* pTmp = SfxObjectShell::GetFirst();
                      pTmp && !pDoc;
                      pTmp = SfxObjectShell::GetNext(*pTmp) )
                {
                    if( ( pTmp != this ) && pTmp->GetMedium() )
                    {
                        INetURLObject aCompare( pTmp->GetMedium()->GetName() );
                        if ( aCompare == aURL )
                            pDoc = pTmp;
                    }
                }
                if ( pDoc )
                {
                    // dann Fehlermeldeung: "schon offen"
                    SetError(ERRCODE_SFX_ALREADYOPEN);
                    delete pFileDlg;
                    return sal_False;
                }
            }

            // old filter options should be cleared in case different filter is used
            SFX_ITEMSET_ARG( pMedSet, pOldFilterNameItem, SfxStringItem, SID_FILTER_NAME, sal_False );
            if ( !pOldFilterNameItem || pOldFilterNameItem->GetValue().CompareTo( aFilterName ) != COMPARE_EQUAL )
            {
                pParams->ClearItem( SID_FILTER_DATA );
                pParams->ClearItem( SID_FILE_FILTEROPTIONS );
            }

            // --**-- pParams->Put( *pDlg->GetItemSet() );
            Reference< XFilePickerControlAccess > xExtFileDlg( pFileDlg->GetFilePicker(), UNO_QUERY );
            if ( xExtFileDlg.is() )
            {
                try
                {
                    if( xFilterCFG.is() )
                    {
                        try {
                            Sequence < PropertyValue > aProps;
                            Any aAny = xFilterCFG->getByName( aFilterName );
                               if ( aAny >>= aProps )
                               {
                                   ::rtl::OUString aServiceName;
                                   sal_Int32 nPropertyCount = aProps.getLength();
                                   for( sal_Int32 nProperty=0; nProperty < nPropertyCount; ++nProperty )
                                       if( aProps[nProperty].Name.equals( ::rtl::OUString::createFromAscii("UIComponent")) )
                                       {
                                        ::rtl::OUString aServiceName;
                                           aProps[nProperty].Value >>= aServiceName;
                                        if( aServiceName.getLength() )
                                            bUseFilterOptions = sal_True;
                                    }
                            }
                        }
                        catch( Exception& )
                        {
                        }
                    }

                    if ( !bIsExport && bUseFilterOptions )
                    {
                        // for exporters: always show dialog if format uses options
                        // for save: show dialog if format uses options and no options given or if forced by user
                        Any aValue = xExtFileDlg->getValue( ExtendedFilePickerElementIds::CHECKBOX_FILTEROPTIONS, 0 );
                        aValue >>= bUseFilterOptions;
                        if ( !bUseFilterOptions )
                            bUseFilterOptions = pParams->GetItemState( SID_FILTER_DATA ) != SFX_ITEM_SET &&
                                                pParams->GetItemState( SID_FILE_FILTEROPTIONS ) != SFX_ITEM_SET;
                    }

                    //pParams->Put( SfxBoolItem( SID_USE_FILTEROPTIONS, bUseFilterOptions ) );
                }
                catch( IllegalArgumentException ){}
            }

            delete pFileDlg;
        }
        else
        {
            SfxUrlDialog aDlg( 0 );
            if( aDlg.Execute() == RET_OK )
                aURL.SetURL( aDlg.GetUrl() );
            else
            {
                SetError(ERRCODE_IO_ABORT);
                return sal_False;
            }
        }

        // merge in results of the dialog execution
        pParams->Put( SfxStringItem( SID_FILE_NAME, aURL.GetMainURL( INetURLObject::NO_DECODE )) );
        pParams->Put( SfxStringItem( SID_FILTER_NAME, aFilterName) );
        // Request mit Dateiname und Filter vervollst"andigen
        pRequest->AppendItem(SfxStringItem( SID_FILE_NAME, aURL.GetMainURL( INetURLObject::NO_DECODE )) );
        pRequest->AppendItem(SfxStringItem( SID_FILTER_NAME, aFilterName));
        const SfxPoolItem* pItem=0;
        pRequest->GetArgs()->GetItemState( SID_FILE_NAME, sal_False, &pItem );
        pFileNameItem = PTR_CAST( SfxStringItem, pItem );
    }

    // neuen Namen an der Object-Shell merken
    SfxSaveAsContext_Impl aSaveAsCtx( pImp->aNewName, aURL.GetMainURL( INetURLObject::NO_DECODE ) );

    // now we can get the filename from the SfxRequest
    DBG_ASSERT( pRequest->GetArgs() != 0, "fehlerhafte Parameter");

    if ( !pFileNameItem && bSaveTo )
    {
        bDialogUsed = sal_True;

        // get the filename by dialog ...
        // create the file dialog
        sfx2::FileDialogHelper aFileDlg( ::sfx2::FILESAVE_AUTOEXTENSION_PASSWORD,
                                         0L, GetFactory().GetFactoryName() );

        SfxItemSet* pTempSet = NULL;
        if ( aFileDlg.Execute( pParams, aFilterName ) != ERRCODE_NONE )
        {
            SetError(ERRCODE_IO_ABORT);
            return sal_False;
        }

        // get the path from the dialog
        aURL.SetURL( aFileDlg.GetPath() );

        // merge in results of the dialog execution
        if( pTempSet )
            pParams->Put( *pTempSet );

        // old filter options should be cleared in case different filter is used
        SFX_ITEMSET_ARG( pMedSet, pOldFilterNameItem, SfxStringItem, SID_FILTER_NAME, sal_False );
        if ( !pOldFilterNameItem || pOldFilterNameItem->GetValue().CompareTo( aFilterName ) != COMPARE_EQUAL )
        {
            pParams->ClearItem( SID_FILTER_DATA );
            pParams->ClearItem( SID_FILE_FILTEROPTIONS );
        }

        pParams->Put( SfxStringItem( SID_FILE_NAME, aURL.GetMainURL( INetURLObject::NO_DECODE )) );
        pParams->Put( SfxStringItem( SID_FILTER_NAME, aFilterName) );
        pRequest->AppendItem(SfxStringItem( SID_FILE_NAME, aURL.GetMainURL( INetURLObject::NO_DECODE )) );
        pRequest->AppendItem(SfxStringItem( SID_FILTER_NAME, aFilterName));
    }
    else if ( pFileNameItem )
    {
        aURL.SetURL(((const SfxStringItem *)pFileNameItem)->GetValue() );
        DBG_ASSERT( aURL.GetProtocol() != INET_PROT_NOT_VALID, "Illegal URL!" );

        const SfxPoolItem* pFilterNameItem=0;
        const SfxItemState eState = pRequest->GetArgs()->GetItemState(SID_FILTER_NAME, sal_True, &pFilterNameItem);
        if ( SFX_ITEM_SET == eState )
        {
            DBG_ASSERT(pFilterNameItem->IsA( TYPE(SfxStringItem) ), "Fehler Parameter");
            aFilterName = ((const SfxStringItem *)pFilterNameItem)->GetValue();
        }
    }
    else
    {
        SetError( ERRCODE_IO_INVALIDPARAMETER );
        return sal_False;
    }

    SfxErrorContext aEc(ERRCTX_SFX_SAVEASDOC,GetTitle());

    // check if a "SaveTo" is wanted, no "SaveAs"
    sal_Bool bCopyTo = GetCreateMode() == SFX_CREATE_MODE_EMBEDDED || bSaveTo;

    // because saving a document modified its DocumentInfo, the current DocumentInfo must be saved on "SaveTo", because
    // it must be restored after saving
    SfxDocumentInfo aSavedInfo;
    if ( bCopyTo )
        aSavedInfo = GetDocInfo();

    // if it is defenitly SaveAs then update doc info
    SfxBoolResetter aDocInfoReset( pImp->bDoNotTouchDocInfo );
    SfxMedium *pActMed = GetMedium();
    const INetURLObject aActName(pActMed->GetName());

    // Don't show filter options dialog
    if ( pRequest->GetSlot() == SID_DIRECTEXPORTDOCASPDF )
        bSuppressFilterOptionsDialog = sal_True;

    if( !bSuppressFilterOptionsDialog &&
        ( bSaveTo || bUseFilterOptions ))
    {
        // call filter dialog
        if( xFilterCFG.is() )
        {
            try {
                   Sequence < PropertyValue > aProps;
                   Any aAny = xFilterCFG->getByName( aFilterName );
                   if ( aAny >>= aProps )
                   {
                       ::rtl::OUString aServiceName;
                       sal_Int32 nPropertyCount = aProps.getLength();
                       for( sal_Int32 nProperty=0; nProperty < nPropertyCount; ++nProperty )
                           if( aProps[nProperty].Name.equals( ::rtl::OUString::createFromAscii("UIComponent")) )
                           {
                            ::rtl::OUString aServiceName;
                               aProps[nProperty].Value >>= aServiceName;
                            if( aServiceName.getLength() )
                            {
                                Reference< XExecutableDialog > xFilterDialog( xServiceManager->createInstance( aServiceName ), UNO_QUERY );
                                Reference< XPropertyAccess > xFilterProperties( xFilterDialog, UNO_QUERY );

                                if( xFilterDialog.is() && xFilterProperties.is() )
                                {
                                    bDialogUsed = sal_True;

                                    Reference< XExporter > xExporter( xFilterDialog, UNO_QUERY );
                                    if( xExporter.is() )
                                        xExporter->setSourceDocument( Reference< XComponent >( GetModel(), UNO_QUERY ) );

                                    Sequence< PropertyValue > aPropsForDialog;
                                    TransformItems( pRequest->GetSlot(), *pParams, aPropsForDialog, NULL );
                                    xFilterProperties->setPropertyValues( aPropsForDialog );

                                    if( xFilterDialog->execute() )
                                    {
                                        SfxAllItemSet aNewParams( GetPool() );
                                        TransformParameters( pRequest->GetSlot(),
                                                             xFilterProperties->getPropertyValues(),
                                                             aNewParams,
                                                             NULL );
                                        pParams->Put( aNewParams );
                                    }
                                    else
                                    {
                                        SetError(ERRCODE_IO_ABORT);
                                        return sal_False; // cancel
                                    }
                                }
                            }

                            break;
                        }
                }
            }
            catch( NoSuchElementException& )
            {
                // the filter name is unknown
                   SetError( ERRCODE_IO_INVALIDPARAMETER );
                return sal_False;
            }
            catch( Exception& )
            {
            }
        }
    }

    if ( aURL != aActName )
    {
        // this is defenitly not a Save
        pImp->bIsSaving = sal_False; // here it's already clear

        // ggf. DocInfo Dialog
        if( bCopyTo && IsEnableSetModified() )
        {
            EnableSetModified( sal_False );
            UpdateDocInfoForSave();
            EnableSetModified( sal_True );
        }
        else
            UpdateDocInfoForSave();

        if (  eCreateMode == SFX_CREATE_MODE_STANDARD && 0 == ( pImp->eFlags & SFXOBJECTSHELL_NODOCINFO ) )
        {
            SvtSaveOptions aOptions;
            if ( aOptions.IsDocInfoSave() )
            {
                DocInfoDlg_Impl( GetDocInfo() );
                pImp->bDoNotTouchDocInfo = sal_True;
            }
        }
    }

    sal_Bool bOk = CommonSaveAs_Impl( aURL, aFilterName, pParams );

    if ( bCopyTo )
    {
        // restore DocumentInfo if only a copy was created
        SfxDocumentInfo &rDocInfo = GetDocInfo();
        rDocInfo = aSavedInfo;
    }

    if( bOk )
    {
        if  (   bDialogUsed
            &&  pFilt->IsOwnFormat()
            &&  pFilt->UsesStorage()
            &&  pFilt->GetVersion() >= SOFFICE_FILEFORMAT_60
            )
        {
            SfxViewFrame* pDocViewFrame = SfxViewFrame::GetFirst( this );
            SfxFrame* pDocFrame = pDocViewFrame ? pDocViewFrame->GetFrame() : NULL;
            if ( pDocFrame )
                SfxHelp::OpenHelpAgent( pDocFrame, HID_DID_SAVE_PACKED_XML );
        }

        return sal_True;
    }
    else
        return sal_False;
}

//--------------------------------------------------------------------

void SfxObjectShell::ExecFile_Impl(SfxRequest &rReq)
{
    {DBG_CHKTHIS(SfxObjectShell, 0);}

    pImp->bSetStandardName=FALSE;
    USHORT nId = rReq.GetSlot();
    if ( !GetMedium() && nId != SID_CLOSEDOC )
    {
        rReq.Ignore();
        return;
    }

    if( nId == SID_SAVEDOC || nId  == SID_UPDATEDOC )
    {
        // Embedded?
        SfxInPlaceObject *pObj=GetInPlaceObject();
        if( pObj && (
            pObj->GetProtocol().IsEmbed() ||
            GetCreateMode() == SFX_CREATE_MODE_EMBEDDED ))
        {
            BOOL bRet = pObj->GetClient()->SaveObject();
            rReq.SetReturnValue( SfxBoolItem(0, bRet) );
            rReq.Done();
            return;
        }

        SFX_REQUEST_ARG( rReq, pVersionItem, SfxStringItem, SID_DOCINFO_COMMENTS, FALSE);
        if ( !IsModified() && !pVersionItem )
        {
            rReq.SetReturnValue( SfxBoolItem(0, FALSE) );
            rReq.Done();
            return;
        }
    }

    SFX_REQUEST_ARG( rReq, pFileNameItem, SfxStringItem, SID_FILE_NAME, FALSE);
    SFX_REQUEST_ARG( rReq, pFilterItem, SfxStringItem, SID_FILTER_NAME, FALSE);

    const SfxFilter *pCurFilter = GetMedium()->GetFilter();
    const SfxFilter *pDefFilter = GetFactory().GetFilter(0);

    if ( nId == SID_SAVEDOC && pCurFilter && !pCurFilter->CanExport() && pDefFilter && pDefFilter->IsInternal() )
        nId = SID_SAVEASDOC;

    // in case of saving an interaction handler can be required for authentication
    if ( nId == SID_SAVEDOC || nId == SID_SAVEASDOC || nId == SID_SAVEASURL || nId == SID_EXPORTDOC )
    {
        Reference< XInteractionHandler > xInteract;
        SFX_REQUEST_ARG( rReq, pxInteractionItem, SfxUnoAnyItem, SID_INTERACTIONHANDLER, sal_False );

        DBG_ASSERT( !pxInteractionItem || ( ( pxInteractionItem->GetValue() >>= xInteract ) && xInteract.is() ),
                    "Broken InteractionHandler!\n" );

        if ( !pxInteractionItem )
        {
            Reference< XMultiServiceFactory > xServiceManager = ::comphelper::getProcessServiceFactory();
            if( xServiceManager.is() )
            {
                xInteract = Reference< XInteractionHandler >(
                            xServiceManager->createInstance( DEFINE_CONST_UNICODE("com.sun.star.task.InteractionHandler") ),
                            UNO_QUERY );

                rReq.AppendItem( SfxUnoAnyItem( SID_INTERACTIONHANDLER, makeAny( xInteract ) ) );
            }
        }
    }

    // interaktiv speichern via (nicht-Default) Filter?
    if ( !pFilterItem && GetMedium()->GetFilter() && HasName() && (nId == SID_SAVEDOC || nId == SID_UPDATEDOC) )
    {
        // aktuellen und Default-Filter besorgen
        // Filter kann nicht exportieren und Default-Filter ist verf"ugbar?
        if ( !pCurFilter->CanExport() && !pDefFilter->IsInternal() )
        {
            // fragen, ob im default-Format gespeichert werden soll
            String aWarn(SfxResId(STR_QUERY_MUSTOWNFORMAT));
            aWarn = SearchAndReplace( aWarn, DEFINE_CONST_UNICODE( "$(FORMAT)" ),
                        GetMedium()->GetFilter()->GetUIName() );
            aWarn = SearchAndReplace( aWarn, DEFINE_CONST_UNICODE( "$(OWNFORMAT)" ),
                        GetFactory().GetFilter(0)->GetUIName() );
            QueryBox aWarnBox(0,WB_OK_CANCEL|WB_DEF_OK,aWarn);
            if ( aWarnBox.Execute() == RET_OK )
            {
                // ja: Save-As in eigenem Foramt
                rReq.SetSlot(nId = SID_SAVEASDOC);
                pImp->bSetStandardName=TRUE;
            }
            else
            {
                // nein: Abbruch
                rReq.Ignore();
                return;
            }
        }
        else
        {
            // fremdes Format mit m"oglichem Verlust (aber nicht per API) wenn noch nicht gewarnt und anschließend im
            // alien format gespeichert wurde
            if ( !( pCurFilter->IsOwnFormat() && pCurFilter->GetVersion() == SOFFICE_FILEFORMAT_CURRENT || ( pCurFilter->GetFilterFlags() & SFX_FILTER_SILENTEXPORT ) )
                 && ( !pImp->bDidWarnFormat || !pImp->bDidDangerousSave ) )
            {
                // Default-Format verf"ugbar?
                if ( !pDefFilter->IsInternal() && pCurFilter != pDefFilter )
                {
                    // fragen, ob im default-Format gespeichert werden soll
                    String aWarn(SfxResId(STR_QUERY_SAVEOWNFORMAT));
                    aWarn = SearchAndReplace( aWarn, DEFINE_CONST_UNICODE( "$(FORMAT)" ),
                                GetMedium()->GetFilter()->GetUIName());
                    aWarn = SearchAndReplace( aWarn, DEFINE_CONST_UNICODE( "$(OWNFORMAT)" ),
                                GetFactory().GetFilter(0)->GetUIName());

                    SfxViewFrame *pFrame = SfxObjectShell::Current() == this ?
                        SfxViewFrame::Current() : SfxViewFrame::GetFirst( this );
                    while ( pFrame && (pFrame->GetFrameType() & SFXFRAME_SERVER ) )
                        pFrame = SfxViewFrame::GetNext( *pFrame, this );

                    if ( pFrame )
                    {
                        SfxFrame* pTop = pFrame->GetTopFrame();
                        SFX_APP()->SetViewFrame( pTop->GetCurrentViewFrame() );
                        pFrame->GetFrame()->Appear();

                        QueryBox aWarnBox(&pFrame->GetWindow(),WB_YES_NO_CANCEL|WB_DEF_YES,aWarn);
                        switch(aWarnBox.Execute())
                        {
                              case RET_YES:
                            {
                                // ja: in Save-As umsetzen
                                rReq.SetSlot(nId = SID_SAVEASDOC);
                                SFX_ITEMSET_ARG( GetMedium()->GetItemSet(), pPassItem, SfxStringItem, SID_PASSWORD, FALSE );
                                if ( pPassItem )
                                    rReq.AppendItem( *pPassItem );
                                pImp->bSetStandardName = TRUE;
                                break;
                            }

                              case RET_CANCEL:
                                // nein: Abbruch
                                rReq.Ignore();
                                return;
                        }

                        pImp->bDidWarnFormat=TRUE;
                    }
                }
            }
        }
    }

    // Speichern eines namenslosen oder readonly Dokumentes
    BOOL bMediumRO = IsReadOnlyMedium();

    if ( ( nId == SID_SAVEDOC || nId == SID_UPDATEDOC ) && ( !HasName() || bMediumRO ) )
    {
        if ( pFileNameItem )
        {
            // FALSE zur"uckliefern
            rReq.SetReturnValue( SfxBoolItem( 0, FALSE ) );
            rReq.Done();
            return;
        }
        else
        {
            // in SaveAs umwandlen
            rReq.SetSlot(nId = SID_SAVEASDOC);
            if ( SFX_APP()->IsPlugin() && !HasName() )
            {
                SFX_REQUEST_ARG( rReq, pWarnItem, SfxBoolItem, SID_FAIL_ON_WARNING, FALSE);
                if ( pWarnItem && pWarnItem->GetValue() == TRUE )
                {
                    // saving done from PrepareClose without UI
                    INetURLObject aObj( SvtPathOptions().GetWorkPath() );
                    aObj.insertName( GetTitle(), false, INetURLObject::LAST_SEGMENT, true, INetURLObject::ENCODE_ALL );
                    const SfxFilter* pFilter = GetFactory().GetFilter(0);
                    String aExtension( pFilter->GetDefaultExtension().Copy(2) );
                    aObj.setExtension( aExtension, INetURLObject::LAST_SEGMENT, true, INetURLObject::ENCODE_ALL );
                    rReq.AppendItem( SfxStringItem( SID_FILE_NAME, aObj.GetMainURL( INetURLObject::NO_DECODE ) ) );
                    rReq.AppendItem( SfxBoolItem( SID_RENAME, TRUE ) );
                }
            }
        }
    }

    switch(nId)
    {
        // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

        case SID_SAVE_VERSION_ON_CLOSE:
        {
            BOOL bSet = GetDocInfo().IsSaveVersionOnClose();
            SFX_REQUEST_ARG( rReq, pItem, SfxBoolItem, nId, FALSE);
            if ( pItem )
                bSet = pItem->GetValue();
            GetDocInfo().SetSaveVersionOnClose( bSet );
            SetModified( TRUE );
            if ( !pItem )
                rReq.AppendItem( SfxBoolItem( nId, bSet ) );
            rReq.Done();
            return;
            break;
        }
        case SID_VERSION:
        {
            SfxViewFrame* pFrame = GetFrame();
            if ( !pFrame )
                pFrame = SfxViewFrame::GetFirst( this );
            if ( !pFrame )
                return;

            if ( pFrame->GetFrame()->GetParentFrame() )
            {
                pFrame->GetTopViewFrame()->GetObjectShell()->ExecuteSlot( rReq );
                return;
            }

            if ( !IsOwnStorageFormat_Impl( *GetMedium() ) )
                return;

            SfxVersionDialog *pDlg = new SfxVersionDialog( pFrame, NULL );
            pDlg->Execute();
            delete pDlg;
            return;
            break;
        }

        case SID_LOAD_LIBRARY:
        case SID_UNLOAD_LIBRARY:
        case SID_REMOVE_LIBRARY:
        case SID_ADD_LIBRARY:
        {
            // Diese Funktionen sind nur f"ur Aufrufe aus dem Basic gedacht
            SfxApplication *pApp = SFX_APP();
            if ( pApp->IsInBasicCall() )
                pApp->BasicLibExec_Impl( rReq, GetBasicManager() );
            return;
            break;
        }

        case SID_SAVEDOC:
        {
            //!! detaillierte Auswertung eines Fehlercodes
            SfxObjectShellRef xLock( this );
            SfxErrorContext aEc(ERRCTX_SFX_SAVEDOC,GetTitle());
            SFX_APP()->NotifyEvent(SfxEventHint(SFX_EVENT_SAVEDOC,this));

            BOOL bOk = Save_Impl( rReq.GetArgs() );

            ULONG lErr=GetErrorCode();
            if( !lErr && !bOk )
                lErr=ERRCODE_IO_GENERAL;

            if ( lErr && bOk )
            {
                SFX_REQUEST_ARG( rReq, pWarnItem, SfxBoolItem, SID_FAIL_ON_WARNING, FALSE);
                if ( pWarnItem && pWarnItem->GetValue() )
                    bOk = FALSE;
            }

            if( !ErrorHandler::HandleError( lErr ) )
                SFX_APP()->NotifyEvent( SfxEventHint( SFX_EVENT_SAVEFINISHED, this ) );
            ResetError();

            rReq.SetReturnValue( SfxBoolItem(0, bOk) );
            if ( bOk )
                SFX_APP()->NotifyEvent(SfxEventHint(SFX_EVENT_SAVEDOCDONE,this));
            break;
        }

        // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
        case SID_UPDATEDOC:
        {
            return;
        }

        // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
        case SID_DOCINFO:
        {
            SFX_REQUEST_ARG(rReq, pDocInfItem, SfxDocumentInfoItem, SID_DOCINFO, FALSE);

            // keine Parameter vorhanden?
            if ( !pDocInfItem )
            {
                // Dialog ausf"uhren
                SfxDocumentInfo *pOldInfo = new SfxDocumentInfo;
                if ( pImp->pDocInfo )
                    // r/o-flag korrigieren falls es zu frueh gesetzt wurde
                    pImp->pDocInfo->SetReadOnly( IsReadOnly() );
                *pOldInfo = GetDocInfo();
                DocInfoDlg_Impl( GetDocInfo() );

                // ge"andert?
                if( !(*pOldInfo == GetDocInfo()) )
                {
                    // Dokument gilt als ver"andert
                    FlushDocInfo();

                    // ggf. Recorden
                    if ( !rReq.IsRecording() )
                        rReq.AppendItem( SfxDocumentInfoItem( GetTitle(), GetDocInfo() ) );
                    rReq.Done();
                }
                else
                    rReq.Ignore();

                delete pOldInfo;
            }
            else
            {
                // DocInfo aus Parameter anwenden
                GetDocInfo() = (*pDocInfItem)();
                FlushDocInfo();
            }

            return;
        }

        // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

        case SID_EXPORTDOC:
        case SID_EXPORTDOCASPDF:
        case SID_DIRECTEXPORTDOCASPDF:
            rReq.AppendItem( SfxBoolItem( SID_SAVETO, sal_True ) );
            // another part is pretty the same as for SID_SAVEASDOC
        case SID_SAVEASURL:
        case SID_SAVEASDOC:
        {
            //!! detaillierte Auswertung eines Fehlercodes
            SfxObjectShellRef xLock( this );

            // Bei Calls "uber StarOne OverWrite-Status checken
            SFX_REQUEST_ARG( rReq, pOverwriteItem, SfxBoolItem, SID_OVERWRITE, FALSE );
            if ( pOverwriteItem )
            {
                // because there is no "exist" function, the overwrite handling is done in the SfxMedium
                SFX_REQUEST_ARG( rReq, pItem, SfxStringItem, SID_FILE_NAME, FALSE );
                if ( !pItem )
                    // In diesem Falle mu\s ein Dateiname mitkommen
                    SetError( ERRCODE_IO_INVALIDPARAMETER );
            }

            BOOL bOk = GUISaveAs_Impl(nId == SID_SAVEASURL, &rReq);
            ULONG lErr=GetErrorCode();
            if ( !lErr && !bOk )
                lErr=ERRCODE_IO_GENERAL;

            if ( lErr && bOk )
            {
                SFX_REQUEST_ARG( rReq, pWarnItem, SfxBoolItem, SID_FAIL_ON_WARNING, FALSE );
                if ( pWarnItem && pWarnItem->GetValue() )
                    bOk = FALSE;
            }

            if ( lErr!=ERRCODE_IO_ABORT )
            {
                SfxErrorContext aEc(ERRCTX_SFX_SAVEASDOC,GetTitle());
                ErrorHandler::HandleError(lErr);
            }

            if ( nId == SID_EXPORTDOCASPDF )
            {
                // This function is used by the SendMail function that needs information if a export
                // file was written or not. This could be due to cancellation of the export
                // or due to an error. So IO abort must be handled like an error!
                bOk = ( lErr != ERRCODE_IO_ABORT ) & bOk;
            }

            rReq.SetReturnValue( SfxBoolItem(0, bOk) );

            ResetError();

            Invalidate();
            break;
        }

        // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

        case SID_CLOSEDOC:
        {
            SfxViewFrame *pFrame = GetFrame();
            if ( pFrame && pFrame->GetFrame()->GetParentFrame() )
            {
                // Wenn SID_CLOSEDOC "uber Menue etc. ausgef"uhrt wird, das
                // aktuelle Dokument aber in einem Frame liegt, soll eigentlich
                // das FrameSetDocument geclosed werden
                pFrame->GetTopViewFrame()->GetObjectShell()->ExecuteSlot( rReq );
                rReq.Done();
                return;
            }

            BOOL bInFrameSet = FALSE;
            USHORT nFrames=0;
            pFrame = SfxViewFrame::GetFirst( this );
            while ( pFrame )
            {
                if ( pFrame->GetFrame()->GetParentFrame() )
                {
                    // Auf dieses Dokument existiert noch eine Sicht, die
                    // in einem FrameSet liegt; diese darf nat"urlich nicht
                    // geclosed werden
                    bInFrameSet = TRUE;
                }
                else
                    nFrames++;

                pFrame = SfxViewFrame::GetNext( *pFrame, this );
            }

            if ( bInFrameSet )
            {
                // Alle Sichten, die nicht in einem FrameSet liegen, closen
                pFrame = SfxViewFrame::GetFirst( this );
                while ( pFrame )
                {
                    if ( !pFrame->GetFrame()->GetParentFrame() )
                        pFrame->GetFrame()->DoClose();
                    pFrame = SfxViewFrame::GetNext( *pFrame, this );
                }
            }

            // Parameter auswerten
            SFX_REQUEST_ARG(rReq, pSaveItem, SfxBoolItem, SID_CLOSEDOC_SAVE, FALSE);
            SFX_REQUEST_ARG(rReq, pNameItem, SfxStringItem, SID_CLOSEDOC_FILENAME, FALSE);
            if ( pSaveItem )
            {
                if ( pSaveItem->GetValue() )
                {
                    if ( !pNameItem )
                    {
                        SbxBase::SetError( SbxERR_WRONG_ARGS );
                        rReq.Ignore();
                        return;
                    }
                    SfxAllItemSet aArgs( GetPool() );
                    SfxStringItem aTmpItem( SID_FILE_NAME, pNameItem->GetValue() );
                    aArgs.Put( aTmpItem, aTmpItem.Which() );
                    SfxRequest aSaveAsReq( SID_SAVEASDOC, SFX_CALLMODE_API, aArgs );
                    ExecFile_Impl( aSaveAsReq );
                    if ( !aSaveAsReq.IsDone() )
                    {
                        rReq.Ignore();
                        return;
                    }
                }
                else
                    SetModified(FALSE);
            }

            // Benutzer bricht ab?
            if ( !PrepareClose( 2 ) )
            {
                rReq.SetReturnValue( SfxBoolItem(0, FALSE) );
                rReq.Done();
                return;
            }

            SetModified( FALSE );
            ULONG lErr = GetErrorCode();
            ErrorHandler::HandleError(lErr);

            rReq.SetReturnValue( SfxBoolItem(0, TRUE) );
            rReq.Done();
            rReq.ReleaseArgs(); // da der Pool in Close zerst"ort wird

            if ( SfxApplication::IsPlugin() )
            {
                for ( SfxViewFrame* pFrame = SfxViewFrame::GetFirst( this ); pFrame; pFrame = SfxViewFrame::GetNext( *pFrame, this ) )
                {
                    String aName = String::CreateFromAscii("vnd.sun.star.cmd:close");
                    SfxStringItem aNameItem( SID_FILE_NAME, aName );
                    SfxStringItem aReferer( SID_REFERER, DEFINE_CONST_UNICODE( "private/user" ) );
                    SfxFrameItem aFrame( SID_DOCFRAME, pFrame->GetFrame() );
                    SFX_APP()->GetAppDispatcher_Impl()->Execute( SID_OPENDOC, SFX_CALLMODE_SLOT, &aNameItem, &aReferer, 0L );
                    return;
                }
            }
/*
            com::sun::star::uno::Reference < ::com::sun::star::frame::XFramesSupplier >
                    xDesktop( ::comphelper::getProcessServiceFactory()->createInstance( ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.frame.Desktop")) ),
                    com::sun::star::uno::UNO_QUERY );
            com::sun::star::uno::Reference < ::com::sun::star::container::XIndexAccess > xList ( xDesktop->getFrames(), ::com::sun::star::uno::UNO_QUERY );
            sal_Int32 nCount = xList->getCount();
            if ( nCount == nFrames )
            {
                SfxViewFrame* pFrame = SfxViewFrame::GetFirst( this );
                SfxViewFrame* pLastFrame = SfxViewFrame::Current();
                if ( pLastFrame->GetObjectShell() != this )
                    pLastFrame = pFrame;

                SfxViewFrame* pNextFrame = pFrame;
                while ( pNextFrame )
                {
                    pNextFrame = SfxViewFrame::GetNext( *pFrame, this );
                    if ( pFrame != pLastFrame )
                        pFrame->GetFrame()->DoClose();
                    pFrame = pNextFrame;
                }

                pLastFrame->GetFrame()->CloseDocument_Impl();
            }
            else
 */
                DoClose();
            return;
        }

        // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
        case SID_DOCTEMPLATE:
        {
            // speichern als Dokumentvorlagen
            SfxDocumentTemplateDlg *pDlg = 0;
            SfxErrorContext aEc(ERRCTX_SFX_DOCTEMPLATE,GetTitle());
            SfxDocumentTemplates *pTemplates =  new SfxDocumentTemplates;

            // Find the template filter with the highest version number
            const SfxFilter* pFilter=NULL;
            const SfxObjectFactory& rFactory = GetFactory();
            USHORT  nFilterCount = rFactory.GetFilterCount();
            ULONG   nVersion = 0;
            int n;
            for( n=0; n<nFilterCount; n++)
            {
                const SfxFilter* pTemp = rFactory.GetFilter( n );
                if( pTemp && pTemp->IsOwnFormat() &&
                    pTemp->IsOwnTemplateFormat() &&
                    ( pTemp->GetVersion() > nVersion ) )
                {
                    pFilter = pTemp;
                    nVersion = pTemp->GetVersion();
                }
            }

            DBG_ASSERT( pFilter, "Template Filter nicht gefunden" );
            if( !pFilter )
                pFilter = rFactory.GetFilter(0);

            if ( !rReq.GetArgs() )
            {
                pDlg = new SfxDocumentTemplateDlg(0, pTemplates);
                if ( RET_OK == pDlg->Execute() && pDlg->GetTemplateName().Len())
                {
                    String aTargetURL = pTemplates->GetTemplatePath(
                            pDlg->GetRegion(),
                            pDlg->GetTemplateName());

                    if ( aTargetURL.Len() )
                    {
                        INetURLObject aTargetObj( aTargetURL );
                        String aTplExtension( pFilter->GetDefaultExtension().Copy(2) );
                        aTargetObj.setExtension( aTplExtension );
                        aTargetURL = aTargetObj.GetMainURL( INetURLObject::NO_DECODE );
                    }

                    rReq.AppendItem( SfxStringItem( SID_FILE_NAME, aTargetURL ) );

                    rReq.AppendItem(SfxStringItem(
                        SID_TEMPLATE_NAME, pDlg->GetTemplateName()));
                    rReq.AppendItem(SfxUInt16Item(
                        SID_TEMPLATE_REGION, pDlg->GetRegion()));
                }
                else
                {
                    delete pDlg;
                    rReq.Ignore();
                    return;
                }
            }

            // Region und Name aus Parameter holen
            SFX_REQUEST_ARG(rReq, pRegionItem, SfxStringItem, SID_TEMPLATE_REGIONNAME, FALSE);
            SFX_REQUEST_ARG(rReq, pNameItem, SfxStringItem, SID_TEMPLATE_NAME, FALSE);
            SFX_REQUEST_ARG(rReq, pRegionNrItem, SfxUInt16Item, SID_TEMPLATE_REGION, FALSE);
            if ( (!pRegionItem && !pRegionNrItem ) || !pNameItem )
            {
                DBG_ASSERT( rReq.IsAPI(), "non-API call without Arguments" );
                SbxBase::SetError( SbxERR_WRONG_ARGS );
                rReq.Ignore();
                return;
            }
            String aTemplateName = pNameItem->GetValue();

            // Region-Nr besorgen
            USHORT nRegion;
            if( pRegionItem )
            {
                // Region-Name finden (eigentlich nicht unbedingt eindeutig)
                nRegion = pTemplates->GetRegionNo( pRegionItem->GetValue() );
                if ( nRegion == USHRT_MAX )
                {
                    SbxBase::SetError( ERRCODE_IO_INVALIDPARAMETER );
                    rReq.Ignore();
                    return;
                }
            }
            if ( pRegionNrItem )
                nRegion = pRegionNrItem->GetValue();

            // kein File-Name angegeben?
            if ( SFX_ITEM_SET != rReq.GetArgs()->GetItemState( SID_FILE_NAME ) )
            {
                // TemplatePath nicht angebgeben => aus Region+Name ermitteln
                // Dateiname zusammenbauen lassen
                String aTemplPath = pTemplates->GetTemplatePath( nRegion, aTemplateName );
                INetURLObject aURLObj( aTemplPath );
                String aExtension( pFilter->GetDefaultExtension().Copy(2) );
                aURLObj.setExtension( aExtension, INetURLObject::LAST_SEGMENT, true, INetURLObject::ENCODE_ALL );

                rReq.AppendItem( SfxStringItem( SID_FILE_NAME, aURLObj.GetMainURL( INetURLObject::NO_DECODE ) ) );
            }

            // Dateiname
            SFX_REQUEST_ARG(rReq, pFileItem, SfxStringItem, SID_FILE_NAME, FALSE);
            const String aFileName(((const SfxStringItem *)pFileItem)->GetValue());

            // Medium zusammenbauen
            SfxItemSet* pSet = new SfxAllItemSet( *rReq.GetArgs() );
            SfxMedium aMedium( aFileName, STREAM_STD_READWRITE, FALSE, pFilter, pSet);

            // als Vorlage speichern
            BOOL bModified = IsModified();
            BOOL bHasTemplateConfig = HasTemplateConfig();
            SetTemplateConfig( FALSE );
            BOOL bOK = FALSE;
            const String aOldURL( INetURLObject::GetBaseURL() );
            if( ShallSetBaseURL_Impl( aMedium ) )
                INetURLObject::SetBaseURL( aMedium.GetBaseURL() );
            else
                INetURLObject::SetBaseURL( String() );

            aMedium.CreateTempFileNoCopy();

            // Because we can't save into a storage directly ( only using tempfile ), we must save the DocInfo first, then
            // we can call SaveTo_Impl and Commit
            if ( pFilter->UsesStorage() && ( pFilter->GetVersion() < SOFFICE_FILEFORMAT_60 ) )
            {
                SfxDocumentInfo *pInfo = new SfxDocumentInfo;
                pInfo->CopyUserData(GetDocInfo());
                pInfo->SetTitle( aTemplateName );
                pInfo->SetChanged( SfxStamp(SvtUserOptions().GetFullName()));
                SvStorageRef aRef = aMedium.GetStorage();
                if ( aRef.Is() )
                {
                    pInfo->SetTime(0L);
                    pInfo->Save(aRef);
                }

                delete pInfo;
            }

            if ( SaveTo_Impl(aMedium,NULL,FALSE) )
            {
                bOK = TRUE;
                pTemplates->NewTemplate( nRegion, aTemplateName, aFileName );
            }

            INetURLObject::SetBaseURL( aOldURL );

            DELETEX(pDlg);

            SetError(aMedium.GetErrorCode());
            ULONG lErr=GetErrorCode();
            if(!lErr && !bOK)
                lErr=ERRCODE_IO_GENERAL;
            ErrorHandler::HandleError(lErr);
            ResetError();
            delete pTemplates;

            if ( IsHandsOff() )
            {
                if ( !DoSaveCompleted( pMedium ) )
                    DBG_ERROR("Case not handled - no way to get a storage!");
            }
            else
                DoSaveCompleted();

            SetTemplateConfig( bHasTemplateConfig );
            SetModified(bModified);
            rReq.SetReturnValue( SfxBoolItem( 0, bOK ) );
            if ( !bOK )
                return;
            break;
        }
    }

    // Picklisten-Eintrag verhindern
    if ( rReq.IsAPI() )
        GetMedium()->SetUpdatePickList( FALSE );
    else if ( rReq.GetArgs() )
    {
        SFX_ITEMSET_GET( *rReq.GetArgs(), pPicklistItem, SfxBoolItem, SID_PICKLIST, FALSE );
        if ( pPicklistItem )
            GetMedium()->SetUpdatePickList( pPicklistItem->GetValue() );
    }

    // Ignore()-Zweige haben schon returnt
    rReq.Done();
}

//--------------------------------------------------------------------

void SfxObjectShell::GetState_Impl(SfxItemSet &rSet)
{
    DBG_CHKTHIS(SfxObjectShell, 0);
    SfxWhichIter aIter( rSet );
    SfxInPlaceObject *pObj=GetInPlaceObject();
    for ( USHORT nWhich = aIter.FirstWhich(); nWhich; nWhich = aIter.NextWhich() )
    {
        switch ( nWhich )
        {
            case SID_SAVE_VERSION_ON_CLOSE:
            {
                rSet.Put( SfxBoolItem( nWhich, GetDocInfo().IsSaveVersionOnClose() ) );
                break;
            }

            case SID_DOCTEMPLATE :
            {
                if ( !GetFactory().GetTemplateFilter() )
                    rSet.DisableItem( nWhich );
                break;
            }

            case SID_VERSION:
                {
                    SfxObjectShell *pDoc = this;
                    SfxViewFrame* pFrame = GetFrame();
                    if ( !pFrame )
                        pFrame = SfxViewFrame::GetFirst( this );
                    if ( pFrame  )
                    {
                        if ( pFrame->GetFrame()->GetParentFrame() )
                        {
                            pFrame = pFrame->GetTopViewFrame();
                            pDoc = pFrame->GetObjectShell();
                        }
                    }

                    if ( !pFrame || !pDoc->HasName() ||
                        !IsOwnStorageFormat_Impl( *pDoc->GetMedium() ) ||
                        pDoc->GetMedium()->GetStorage()->GetVersion() < SOFFICE_FILEFORMAT_50 )
                        rSet.DisableItem( nWhich );
                    break;
                }
            case SID_SAVEDOC:
            case SID_UPDATEDOC:
                if (pObj && (
                    pObj->GetProtocol().IsEmbed() ||
                    GetCreateMode() == SFX_CREATE_MODE_EMBEDDED ))
                {
                    String aEntry (SfxResId(STR_UPDATEDOC));
                    aEntry += ' ';
                    aEntry += GetInPlaceObject()->GetDocumentName();
                    rSet.Put(SfxStringItem(nWhich, aEntry));
                }
                else
                {
                    BOOL bMediumRO = IsReadOnlyMedium();
                    if ( !bMediumRO && GetMedium() && IsModified() )
                        rSet.Put(SfxStringItem(
                            nWhich, String(SfxResId(STR_SAVEDOC))));
                    else
                        rSet.DisableItem(nWhich);
                }
                break;

            case SID_DOCINFO:
                if ( 0 != ( pImp->eFlags & SFXOBJECTSHELL_NODOCINFO ) )
                    rSet.DisableItem( nWhich );
                break;

            case SID_CLOSEDOC:
            {
                SfxObjectShell *pDoc = this;
                SfxViewFrame *pFrame = GetFrame();
                if ( pFrame && pFrame->GetFrame()->GetParentFrame() )
                {
                    // Wenn SID_CLOSEDOC "uber Menue etc. ausgef"uhrt wird, das
                    // aktuelle Dokument aber in einem Frame liegt, soll eigentlich
                    // das FrameSetDocument geclosed werden
                    pDoc = pFrame->GetTopViewFrame()->GetObjectShell();
                }

                if ( pDoc->GetFlags() & SFXOBJECTSHELL_DONTCLOSE )
                    rSet.DisableItem(nWhich);
                else if ( pObj && (
                    pObj->GetProtocol().IsEmbed() ||
                    GetCreateMode() == SFX_CREATE_MODE_EMBEDDED ))
                {
                    String aEntry (SfxResId(STR_CLOSEDOC_ANDRETURN));
                    aEntry += pObj->GetDocumentName();
                    rSet.Put( SfxStringItem(nWhich, aEntry) );
                }
                else
                    rSet.Put(SfxStringItem(nWhich, String(SfxResId(STR_CLOSEDOC))));
                break;
            }

            case SID_SAVEASDOC:
            {
                if( ( pImp->nLoadedFlags & SFX_LOADED_MAINDOCUMENT ) != SFX_LOADED_MAINDOCUMENT )
                {
                    rSet.DisableItem( nWhich );
                    break;
                }

                const SfxFilter* pCombinedFilters = NULL;
                SfxFilterContainer* pFilterContainer = GetFactory().GetFilterContainer();

                if ( pFilterContainer )
                {
                    SfxFilterFlags    nMust    = SFX_FILTER_IMPORT | SFX_FILTER_EXPORT;
                    SfxFilterFlags    nDont    = SFX_FILTER_NOTINSTALLED;

                    pCombinedFilters = pFilterContainer->GetAnyFilter( nMust, nDont );
                }

                if ( !pCombinedFilters || !GetMedium() )
                    rSet.DisableItem( nWhich );
                else if ( pObj && (
                    pObj->GetProtocol().IsEmbed() ||
                    GetCreateMode() == SFX_CREATE_MODE_EMBEDDED ))
                    rSet.Put( SfxStringItem( nWhich, String( SfxResId( STR_SAVECOPYDOC ) ) ) );
                else
                    rSet.Put( SfxStringItem( nWhich, String( SfxResId( STR_SAVEASDOC ) ) ) );

                break;
            }

            case SID_EXPORTDOCASPDF:
            case SID_DIRECTEXPORTDOCASPDF:
            {
                SfxFilterContainer* pFilterContainer = GetFactory().GetFilterContainer();
                if ( pFilterContainer )
                {
                    String aPDFExtension = String::CreateFromAscii( ".pdf" );
                    const SfxFilter* pFilter = pFilterContainer->GetFilter4Extension( aPDFExtension, SFX_FILTER_EXPORT );
                    if ( pFilter != NULL )
                        break;
                }

                rSet.DisableItem( nWhich );
                break;
            }

            case SID_DOC_MODIFIED:
            {
                rSet.Put( SfxStringItem( SID_DOC_MODIFIED, IsModified() ? '*' : ' ' ) );
                break;
            }

            case SID_MODIFIED:
            {
                rSet.Put( SfxBoolItem( SID_MODIFIED, IsModified() ) );
                break;
            }

            case SID_DOCINFO_TITLE:
            {
                rSet.Put( SfxStringItem(
                    SID_DOCINFO_TITLE, GetDocInfo().GetTitle() ) );
                break;
            }
            case SID_FILE_NAME:
            {
                if( GetMedium() && HasName() )
                    rSet.Put( SfxStringItem(
                        SID_FILE_NAME, GetMedium()->GetName() ) );
                break;
            }
        }
    }
}

//--------------------------------------------------------------------

void SfxObjectShell::ExecProps_Impl(SfxRequest &rReq)
{
    switch ( rReq.GetSlot() )
    {
        case SID_MODIFIED:
        {
            SetModified( ( (SfxBoolItem&) rReq.GetArgs()->Get(SID_MODIFIED)).GetValue() );
            rReq.Done();
            break;
        }

        case SID_DOCTITLE:
            SetTitle( ( (SfxStringItem&) rReq.GetArgs()->Get(SID_DOCTITLE)).GetValue() );
            rReq.Done();
            break;

        case SID_ON_CREATEDOC:
        case SID_ON_OPENDOC:
        case SID_ON_PREPARECLOSEDOC:
        case SID_ON_CLOSEDOC:
        case SID_ON_SAVEDOC:
        case SID_ON_SAVEASDOC:
        case SID_ON_ACTIVATEDOC:
        case SID_ON_DEACTIVATEDOC:
        case SID_ON_PRINTDOC:
        case SID_ON_SAVEDOCDONE:
        case SID_ON_SAVEASDOCDONE:
            SFX_APP()->EventExec_Impl( rReq, this );
            break;

        // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
        case SID_PLAYMACRO:
        {
            SFX_APP()->PlayMacro_Impl( rReq, GetBasic() );
            break;
        }

        case SID_DOCINFO_AUTHOR :
        {
            String aStr = ( (SfxStringItem&)rReq.GetArgs()->Get(rReq.GetSlot())).GetValue();
            SfxStamp aStamp( GetDocInfo().GetCreated() );
            aStamp.SetName( aStr );
            GetDocInfo().SetCreated( aStamp );
            break;
        }

        case SID_DOCINFO_COMMENTS :
        {
            String aStr = ( (SfxStringItem&)rReq.GetArgs()->Get(rReq.GetSlot())).GetValue();
            GetDocInfo().SetComment( aStr );
            break;
        }

        case SID_DOCINFO_KEYWORDS :
        {
            String aStr = ( (SfxStringItem&)rReq.GetArgs()->Get(rReq.GetSlot())).GetValue();
            GetDocInfo().SetKeywords( aStr );
            break;
        }
    }
}

//--------------------------------------------------------------------

void SfxObjectShell::StateProps_Impl(SfxItemSet &rSet)
{
    SfxWhichIter aIter(rSet);
    for ( USHORT nSID = aIter.FirstWhich(); nSID; nSID = aIter.NextWhich() )
    {
        switch ( nSID )
        {
            case SID_DOCINFO_AUTHOR :
            {
                String aStr = GetDocInfo().GetCreated().GetName();
                rSet.Put( SfxStringItem( nSID, aStr ) );
                break;
            }

            case SID_DOCINFO_COMMENTS :
            {
                String aStr = GetDocInfo().GetComment();
                rSet.Put( SfxStringItem( nSID, aStr ) );
                break;
            }

            case SID_DOCINFO_KEYWORDS :
            {
                String aStr = GetDocInfo().GetKeywords();
                rSet.Put( SfxStringItem( nSID, aStr ) );
                break;
            }

            case SID_DOCPATH:
            {
                DBG_ERROR( "Not supported anymore!" );
                break;
            }

            case SID_DOCFULLNAME:
            {
                rSet.Put( SfxStringItem( SID_DOCFULLNAME, GetTitle(SFX_TITLE_FULLNAME) ) );
                break;
            }

            case SID_DOCTITLE:
            {
                rSet.Put( SfxStringItem( SID_DOCTITLE, GetTitle() ) );
                break;
            }

            case SID_DOC_READONLY:
            {
                rSet.Put( SfxBoolItem( SID_DOC_READONLY, IsReadOnly() ) );
                break;
            }

            case SID_DOC_SAVED:
            {
                rSet.Put( SfxBoolItem( SID_DOC_SAVED, !IsModified() ) );
                break;
            }

            case SID_CLOSING:
            {
                rSet.Put( SfxBoolItem( SID_CLOSING, Get_Impl()->bInCloseEvent ) );
                break;
            }

            case SID_ON_CREATEDOC:
            case SID_ON_OPENDOC:
            case SID_ON_PREPARECLOSEDOC:
            case SID_ON_CLOSEDOC:
            case SID_ON_SAVEDOC:
            case SID_ON_SAVEASDOC:
            case SID_ON_ACTIVATEDOC:
            case SID_ON_DEACTIVATEDOC:
            case SID_ON_PRINTDOC:
            case SID_ON_SAVEDOCDONE:
            case SID_ON_SAVEASDOCDONE:
                SFX_APP()->EventState_Impl( nSID, rSet, this );
                break;

            case SID_DOC_LOADING:
                rSet.Put( SfxBoolItem( nSID, SFX_LOADED_MAINDOCUMENT !=
                            ( pImp->nLoadedFlags & SFX_LOADED_MAINDOCUMENT ) ) );
                break;

            case SID_IMG_LOADING:
                rSet.Put( SfxBoolItem( nSID, SFX_LOADED_IMAGES !=
                            ( pImp->nLoadedFlags & SFX_LOADED_IMAGES ) ) );
                break;
        }
    }
}

//--------------------------------------------------------------------

void SfxObjectShell::ExecView_Impl(SfxRequest &rReq)
{
    switch ( rReq.GetSlot() )
    {
        case SID_ACTIVATE:
        {
            SfxViewFrame *pFrame =
                    SfxViewFrame::GetFirst( this, TYPE(SfxTopViewFrame), TRUE );
            if ( pFrame )
                pFrame->GetFrame()->Appear();
            rReq.SetReturnValue( SfxObjectItem( 0, pFrame ) );
            rReq.Done();
            break;
        }
        case SID_NEWWINDOWFOREDIT:
        {
            SfxViewFrame* pFrame = SfxViewFrame::Current();
            if( pFrame->GetObjectShell() == this &&
                ( pFrame->GetFrameType() & SFXFRAME_HASTITLE ) )
                pFrame->ExecuteSlot( rReq );
            else
            {
                String aFileName( GetObjectShell()->GetMedium()->GetName() );
                if ( aFileName.Len() )
                {
                    SfxStringItem aName( SID_FILE_NAME, aFileName );
                    SfxBoolItem aCreateView( SID_OPEN_NEW_VIEW, TRUE );
                    SFX_APP()->GetAppDispatcher_Impl()->Execute(
                        SID_OPENDOC, SFX_CALLMODE_ASYNCHRON, &aName,
                        &aCreateView, 0L);
                }
            }
        }
    }
}

//--------------------------------------------------------------------

void SfxObjectShell::StateView_Impl(SfxItemSet &rSet)
{
}

