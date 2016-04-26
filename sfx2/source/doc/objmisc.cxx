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
#include <config_folders.h>

#include <tools/inetmsg.hxx>
#include <tools/diagnose_ex.h>
#include <svl/eitem.hxx>
#include <svl/stritem.hxx>
#include <svl/intitem.hxx>
#include <svtools/svparser.hxx>
#include <osl/mutex.hxx>
#include <cppuhelper/exc_hlp.hxx>

#include <com/sun/star/container/XChild.hpp>
#include <com/sun/star/document/XDocumentPropertiesSupplier.hpp>
#include <com/sun/star/document/XDocumentProperties.hpp>
#include <com/sun/star/document/UpdateDocMode.hpp>
#include <com/sun/star/document/MacroExecMode.hpp>
#include <com/sun/star/document/XScriptInvocationContext.hpp>
#include <com/sun/star/embed/EmbedStates.hpp>
#include <com/sun/star/embed/XEmbedPersist.hpp>
#include <com/sun/star/logging/DocumentIOLogRing.hpp>
#include <com/sun/star/script/XTypeConverter.hpp>
#include <com/sun/star/script/FinishEngineEvent.hpp>
#include <com/sun/star/script/InterruptReason.hpp>
#include <com/sun/star/script/XEngineListener.hpp>
#include <com/sun/star/script/XDebugging.hpp>
#include <com/sun/star/script/XInvocation.hpp>
#include <com/sun/star/script/ContextInformation.hpp>
#include <com/sun/star/script/FinishReason.hpp>
#include <com/sun/star/script/XEngine.hpp>
#include <com/sun/star/script/InterruptEngineEvent.hpp>
#include <com/sun/star/script/XLibraryAccess.hpp>
#include <com/sun/star/script/provider/theMasterScriptProviderFactory.hpp>
#include <com/sun/star/script/provider/XScript.hpp>
#include <com/sun/star/script/provider/XScriptProvider.hpp>
#include <com/sun/star/script/provider/XScriptProviderSupplier.hpp>
#include <com/sun/star/ucb/SimpleFileAccess.hpp>
#include <com/sun/star/util/XModifiable.hpp>

#include <toolkit/helper/vclunohelper.hxx>

#include <com/sun/star/uno/Reference.h>
#include <com/sun/star/uno/Any.h>
#include <com/sun/star/ucb/XContent.hpp>
#include <com/sun/star/task/ErrorCodeRequest.hpp>
#include <unotools/securityoptions.hxx>

#include <comphelper/processfactory.hxx>
#include <comphelper/string.hxx>

#include <com/sun/star/security/DocumentDigitalSignatures.hpp>
#include <com/sun/star/task/DocumentMacroConfirmationRequest.hpp>
#include <com/sun/star/task/InteractionClassification.hpp>
#include <com/sun/star/frame/XModel.hpp>

#include <basic/sbuno.hxx>
#include <basic/sbstar.hxx>
#include <basic/basmgr.hxx>
#include <vcl/msgbox.hxx>
#include <basic/sbx.hxx>
#include <svtools/sfxecode.hxx>
#include <svtools/ehdl.hxx>

#include <unotools/pathoptions.hxx>
#include <unotools/ucbhelper.hxx>
#include <tools/inetmime.hxx>
#include <tools/urlobj.hxx>
#include <svl/inettype.hxx>
#include <svl/sharecontrolfile.hxx>
#include <osl/file.hxx>
#include <rtl/bootstrap.hxx>
#include <vcl/svapp.hxx>
#include <framework/interaction.hxx>
#include <framework/documentundoguard.hxx>
#include <comphelper/interaction.hxx>
#include <comphelper/storagehelper.hxx>
#include <comphelper/documentconstants.hxx>

#include <sfx2/signaturestate.hxx>
#include <sfx2/app.hxx>
#include "appdata.hxx"
#include <sfx2/request.hxx>
#include <sfx2/bindings.hxx>
#include <sfx2/sfxresid.hxx>
#include <sfx2/docfile.hxx>
#include <sfx2/docfilt.hxx>
#include <sfx2/objsh.hxx>
#include "objshimp.hxx"
#include <sfx2/event.hxx>
#include <sfx2/sfx.hrc>
#include <sfx2/dispatch.hxx>
#include <sfx2/viewfrm.hxx>
#include <sfx2/viewsh.hxx>
#include <sfx2/ctrlitem.hxx>
#include "arrdecl.hxx"
#include <sfx2/module.hxx>
#include <sfx2/docfac.hxx>
#include "helper.hxx"
#include "doc.hrc"
#include "workwin.hxx"
#include "helpid.hrc"
#include "../appl/app.hrc"
#include <sfx2/sfxdlg.hxx>
#include "appbaslib.hxx"
#include <openflag.hxx>
#include <objstor.hxx>
#include <appopen.hxx>

#include <memory>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::ucb;
using namespace ::com::sun::star::document;
using namespace ::com::sun::star::frame;
using namespace ::com::sun::star::script;
using namespace ::com::sun::star::script::provider;
using namespace ::com::sun::star::container;

// class SfxHeaderAttributes_Impl ----------------------------------------

class SfxHeaderAttributes_Impl : public SvKeyValueIterator
{
private:
    SfxObjectShell* pDoc;
    SvKeyValueIteratorRef xIter;
    bool bAlert;

public:
    explicit SfxHeaderAttributes_Impl( SfxObjectShell* pSh ) :
        SvKeyValueIterator(), pDoc( pSh ),
        xIter( pSh->GetMedium()->GetHeaderAttributes_Impl() ),
        bAlert( false ) {}

    virtual bool GetFirst( SvKeyValue& rKV ) override { return xIter->GetFirst( rKV ); }
    virtual bool GetNext( SvKeyValue& rKV ) override { return xIter->GetNext( rKV ); }
    virtual void Append( const SvKeyValue& rKV ) override;

    void ClearForSourceView() { xIter = new SvKeyValueIterator; bAlert = false; }
    void SetAttributes();
    void SetAttribute( const SvKeyValue& rKV );
};


sal_uInt16 const aTitleMap_Impl[3][2] =
{
                                //  local               remote
    /*  SFX_TITLE_CAPTION   */  {   SFX_TITLE_FILENAME, SFX_TITLE_TITLE },
    /*  SFX_TITLE_PICKLIST  */  {   32,                 SFX_TITLE_FULLNAME },
    /*  SFX_TITLE_HISTORY   */  {   32,                 SFX_TITLE_FULLNAME }
};


void SfxObjectShell::AbortImport()
{
    pImp->bIsAbortingImport = true;
}


bool SfxObjectShell::IsAbortingImport() const
{
    return pImp->bIsAbortingImport;
}


uno::Reference<document::XDocumentProperties>
SfxObjectShell::getDocProperties()
{
    uno::Reference<document::XDocumentPropertiesSupplier> xDPS(
        GetModel(), uno::UNO_QUERY_THROW);
    uno::Reference<document::XDocumentProperties> xDocProps(
        xDPS->getDocumentProperties());
    DBG_ASSERT(xDocProps.is(),
        "SfxObjectShell: model has no DocumentProperties");
    return xDocProps;
}


void SfxObjectShell::DoFlushDocInfo()
{
}


// Note: the only thing that calls this is the modification event handler
// that is installed at the XDocumentProperties
void SfxObjectShell::FlushDocInfo()
{
    if ( IsLoading() )
        return;

    SetModified();
    uno::Reference<document::XDocumentProperties> xDocProps(getDocProperties());
    DoFlushDocInfo(); // call template method
    OUString url(xDocProps->getAutoloadURL());
    sal_Int32 delay(xDocProps->getAutoloadSecs());
    SetAutoLoad( INetURLObject(url), delay * 1000,
                 (delay > 0) || !url.isEmpty() );
}


void SfxObjectShell::SetError( sal_uInt32 lErr, const OUString& aLogMessage )
{
    if(pImp->lErr==ERRCODE_NONE)
    {
        pImp->lErr=lErr;

        if( lErr != ERRCODE_NONE && !aLogMessage.isEmpty() )
            AddLog( aLogMessage );
    }
}


sal_uInt32 SfxObjectShell::GetError() const
{
    return ERRCODE_TOERROR(GetErrorCode());
}


sal_uInt32 SfxObjectShell::GetErrorCode() const
{
    sal_uInt32 lError=pImp->lErr;
    if(!lError && GetMedium())
        lError=GetMedium()->GetErrorCode();
    return lError;
}


void SfxObjectShell::ResetError()
{
    if( pImp->lErr != ERRCODE_NONE )
        AddLog( OSL_LOG_PREFIX "Resetting Error." );

    pImp->lErr=0;
    SfxMedium * pMed = GetMedium();
    if( pMed )
        pMed->ResetError();
}


bool SfxObjectShell::IsTemplate() const
{
    return pImp->bIsTemplate;
}


void SfxObjectShell::EnableSetModified( bool bEnable )
{
#ifdef DBG_UTIL
    if ( bEnable == pImp->m_bEnableSetModified )
        SAL_INFO( "sfx", "SFX_PERSIST: EnableSetModified 2x called with the same value" );
#endif
    pImp->m_bEnableSetModified = bEnable;
}


bool SfxObjectShell::IsEnableSetModified() const
{
    return pImp->m_bEnableSetModified && !IsReadOnly();
}


bool SfxObjectShell::IsModified()
{
    if ( pImp->m_bIsModified )
        return true;

    if ( !pImp->m_xDocStorage.is() || IsReadOnly() )
    {
        // if the document still has no storage and is not set to be modified explicitly it is not modified
        // a readonly document is also not modified

        return false;
    }

    if (pImp->mpObjectContainer)
    {
        uno::Sequence < OUString > aNames = GetEmbeddedObjectContainer().GetObjectNames();
        for ( sal_Int32 n=0; n<aNames.getLength(); n++ )
        {
            uno::Reference < embed::XEmbeddedObject > xObj = GetEmbeddedObjectContainer().GetEmbeddedObject( aNames[n] );
            OSL_ENSURE( xObj.is(), "An empty entry in the embedded objects list!\n" );
            if ( xObj.is() )
            {
                try
                {
                    sal_Int32 nState = xObj->getCurrentState();
                    if ( nState != embed::EmbedStates::LOADED )
                    {
                        uno::Reference< util::XModifiable > xModifiable( xObj->getComponent(), uno::UNO_QUERY );
                        if ( xModifiable.is() && xModifiable->isModified() )
                            return true;
                    }
                }
                catch( uno::Exception& )
                {}
            }
        }
    }

    return false;
}


void SfxObjectShell::SetModified( bool bModifiedP )
{
#ifdef DBG_UTIL
    if ( !bModifiedP && !IsEnableSetModified() )
        SAL_INFO( "sfx", "SFX_PERSIST: SetModified( sal_False ), although IsEnableSetModified() == sal_False" );
#endif

    if( !IsEnableSetModified() )
        return;

    if( pImp->m_bIsModified != bModifiedP )
    {
        pImp->m_bIsModified = bModifiedP;
        ModifyChanged();
    }
}


void SfxObjectShell::ModifyChanged()
{
    if ( pImp->bClosing )
        // SetModified dispose of the models!
        return;


    SfxViewFrame* pViewFrame = SfxViewFrame::Current();
    if ( pViewFrame )
        pViewFrame->GetBindings().Invalidate( SID_SAVEDOCS );

    Invalidate( SID_SIGNATURE );
    Invalidate( SID_MACRO_SIGNATURE );
    Broadcast( SfxSimpleHint( SFX_HINT_TITLECHANGED ) );    // xmlsec05, signed state might change in title...

    SfxGetpApp()->NotifyEvent( SfxEventHint( SFX_EVENT_MODIFYCHANGED, GlobalEventConfig::GetEventName(GlobalEventId::MODIFYCHANGED), this ) );
}


bool SfxObjectShell::IsReadOnlyUI() const

/*  [Description]

    Returns sal_True if the document for the UI is treated as r/o. This is
    regardless of the actual r/o, which can be checked with <IsReadOnly()>.
*/

{
    return pImp->bReadOnlyUI;
}


bool SfxObjectShell::IsReadOnlyMedium() const

/*  [Description]

    Returns sal_True when the medium is r/o, for instance when opened as r/o.
*/

{
    if ( !pMedium )
        return true;
    return pMedium->IsReadOnly();
}

bool SfxObjectShell::IsOriginallyReadOnlyMedium() const
{
    return pMedium == nullptr || pMedium->IsOriginallyReadOnly();
}


void SfxObjectShell::SetReadOnlyUI( bool bReadOnly )

/*  [Description]

    Turns the document in an r/o and r/w state respectively without reloading
    it and without changing the open mode of the medium.
*/

{
    if ( bReadOnly != pImp->bReadOnlyUI )
    {
        pImp->bReadOnlyUI = bReadOnly;
        Broadcast( SfxSimpleHint(SFX_HINT_MODECHANGED) );
    }
}


void SfxObjectShell::SetReadOnly()
{
    // Let the document be completely readonly, means that the
    // medium open mode is adjusted accordingly, and the write lock
    // on the file is removed.

     if ( pMedium && !IsReadOnlyMedium() )
    {
        bool bWasROUI = IsReadOnly();

        pMedium->UnlockFile( false );

        // the storage-based mediums are already based on the temporary file
        // so UnlockFile has already closed the locking stream
        if ( !pMedium->HasStorage_Impl() && IsLoadingFinished() )
            pMedium->CloseInStream();

        pMedium->SetOpenMode( SFX_STREAM_READONLY, true );
        pMedium->GetItemSet()->Put( SfxBoolItem( SID_DOC_READONLY, true ) );

        if ( !bWasROUI )
            Broadcast( SfxSimpleHint(SFX_HINT_MODECHANGED) );
    }
}


bool SfxObjectShell::IsReadOnly() const
{
    return pImp->bReadOnlyUI || pMedium == nullptr;
}


bool SfxObjectShell::IsInModalMode() const
{
    return pImp->bModalMode || pImp->bRunningMacro;
}

bool SfxObjectShell::AcceptStateUpdate() const
{
    return !IsInModalMode();
}


void SfxObjectShell::SetMacroMode_Impl( bool bModal )
{
    if ( !pImp->bRunningMacro != !bModal )
    {
        pImp->bRunningMacro = bModal;
        Broadcast( SfxSimpleHint( SFX_HINT_MODECHANGED ) );
    }
}


void SfxObjectShell::SetModalMode_Impl( bool bModal )
{
    // Broadcast only if modified, or otherwise it will possibly go into
    // an endless loop
    if ( !pImp->bModalMode != !bModal )
    {
        // Central count
        sal_uInt16 &rDocModalCount = SfxGetpApp()->Get_Impl()->nDocModalMode;
        if ( bModal )
            ++rDocModalCount;
        else
            --rDocModalCount;

        // Switch
        pImp->bModalMode = bModal;
        Broadcast( SfxSimpleHint( SFX_HINT_MODECHANGED ) );
    }
}

#if HAVE_FEATURE_MULTIUSER_ENVIRONMENT

bool SfxObjectShell::SwitchToShared( bool bShared, bool bSave )
{
    bool bResult = true;

    if ( bShared != IsDocShared() )
    {
        OUString aOrigURL = GetMedium()->GetURLObject().GetMainURL( INetURLObject::NO_DECODE );

        if ( aOrigURL.isEmpty() && bSave )
        {
            // this is a new document, let it be stored before switching to the shared mode;
            // the storing should be done without shared flag, since it is possible that the
            // target location does not allow to create sharing control file;
            // the shared flag will be set later after creation of sharing control file
            SfxViewFrame* pViewFrame = SfxViewFrame::GetFirst( this );

            if ( pViewFrame )
            {
                // TODO/LATER: currently the application guards against the reentrance problem
                const SfxPoolItem* pItem = pViewFrame->GetBindings().ExecuteSynchron( HasName() ? SID_SAVEDOC : SID_SAVEASDOC );
                const SfxBoolItem* pResult = dynamic_cast<const SfxBoolItem*>( pItem  );
                bResult = ( pResult && pResult->GetValue() );
                if ( bResult )
                    aOrigURL = GetMedium()->GetURLObject().GetMainURL( INetURLObject::NO_DECODE );
            }
        }

        bool bOldValue = HasSharedXMLFlagSet();
        SetSharedXMLFlag( bShared );

        bool bRemoveEntryOnError = false;
        if ( bResult && bShared )
        {
            try
            {
                ::svt::ShareControlFile aControlFile( aOrigURL );
                aControlFile.InsertOwnEntry();
                bRemoveEntryOnError = true;
            }
            catch( uno::Exception& )
            {
                bResult = false;
            }
        }

        if ( bResult && bSave )
        {
            SfxViewFrame* pViewFrame = SfxViewFrame::GetFirst( this );

            if ( pViewFrame )
            {
                // TODO/LATER: currently the application guards against the reentrance problem
                SetModified(); // the modified flag has to be set to let the document be stored with the shared flag
                const SfxPoolItem* pItem = pViewFrame->GetBindings().ExecuteSynchron( HasName() ? SID_SAVEDOC : SID_SAVEASDOC );
                const SfxBoolItem* pResult = dynamic_cast<const SfxBoolItem*>( pItem  );
                bResult = ( pResult && pResult->GetValue() );
            }
        }

        if ( bResult )
        {
            // TODO/LATER: Is it possible that the following calls fail?
            if ( bShared )
            {
                pImp->m_aSharedFileURL = aOrigURL;
                GetMedium()->SwitchDocumentToTempFile();
            }
            else
            {
                OUString aTempFileURL = pMedium->GetURLObject().GetMainURL( INetURLObject::NO_DECODE );
                GetMedium()->SwitchDocumentToFile( GetSharedFileURL() );
                (pImp->m_aSharedFileURL).clear();

                // now remove the temporary file the document was based on
                ::utl::UCBContentHelper::Kill( aTempFileURL );

                try
                {
                    // aOrigURL can not be used since it contains an old value
                    ::svt::ShareControlFile aControlFile( GetMedium()->GetURLObject().GetMainURL( INetURLObject::NO_DECODE ) );
                    aControlFile.RemoveFile();
                }
                catch( uno::Exception& )
                {
                }
            }
        }
        else
        {
            // the saving has failed!
            if ( bRemoveEntryOnError )
            {
                try
                {
                    ::svt::ShareControlFile aControlFile( aOrigURL );
                    aControlFile.RemoveEntry();
                }
                catch( uno::Exception& )
                {}
            }

            SetSharedXMLFlag( bOldValue );
        }
    }
    else
        bResult = false; // the second switch to the same mode

    if ( bResult )
        SetTitle( "" );

    return bResult;
}


void SfxObjectShell::FreeSharedFile()
{
    if ( pMedium )
        FreeSharedFile( pMedium->GetURLObject().GetMainURL( INetURLObject::NO_DECODE ) );
}


void SfxObjectShell::FreeSharedFile( const OUString& aTempFileURL )
{
    SetSharedXMLFlag( false );

    if ( IsDocShared() && !aTempFileURL.isEmpty()
      && !::utl::UCBContentHelper::EqualURLs( aTempFileURL, GetSharedFileURL() ) )
    {
        if ( pImp->m_bAllowShareControlFileClean )
        {
            try
            {
                ::svt::ShareControlFile aControlFile( GetSharedFileURL() );
                aControlFile.RemoveEntry();
            }
            catch( uno::Exception& )
            {
            }
        }

        // the cleaning is forbidden only once
        pImp->m_bAllowShareControlFileClean = true;

        // now remove the temporary file the document is based currently on
        ::utl::UCBContentHelper::Kill( aTempFileURL );

        (pImp->m_aSharedFileURL).clear();
    }
}


void SfxObjectShell::DoNotCleanShareControlFile()
{
    pImp->m_bAllowShareControlFileClean = false;
}


void SfxObjectShell::SetSharedXMLFlag( bool bFlag ) const
{
    pImp->m_bSharedXMLFlag = bFlag;
}


bool SfxObjectShell::HasSharedXMLFlagSet() const
{
    return pImp->m_bSharedXMLFlag;
}

#endif

bool SfxObjectShell::IsDocShared() const
{
#if HAVE_FEATURE_MULTIUSER_ENVIRONMENT
    return ( !pImp->m_aSharedFileURL.isEmpty() );
#else
    return false;
#endif
}


OUString SfxObjectShell::GetSharedFileURL() const
{
#if HAVE_FEATURE_MULTIUSER_ENVIRONMENT
    return pImp->m_aSharedFileURL;
#else
    return OUString();
#endif
}

Size SfxObjectShell::GetFirstPageSize()
{
    return GetVisArea(ASPECT_THUMBNAIL).GetSize();
}


IndexBitSet& SfxObjectShell::GetNoSet_Impl()
{
    return pImp->aBitSet;
}


// changes the title of the document

void SfxObjectShell::SetTitle
(
    const OUString& rTitle                // the new Document Title
)

/*  [Description]

    With this method, the title of the document can be set.
    This corresponds initially to the full file name. A setting of the
    title does not affect the file name, but it will be shown in the
    Caption-Bars of the MDI-window.
*/

{

    // Nothing to do?
    if ( ( ( HasName() && pImp->aTitle == rTitle )
        || ( !HasName() && GetTitle() == rTitle ) )
      && !IsDocShared() )
        return;

    SfxApplication *pSfxApp = SfxGetpApp();

    // If possible relase the unnamed number.
    if ( pImp->bIsNamedVisible && USHRT_MAX != pImp->nVisualDocumentNumber )
    {
        pSfxApp->ReleaseIndex(pImp->nVisualDocumentNumber);
        pImp->bIsNamedVisible = false;
    }

    // Set Title
    pImp->aTitle = rTitle;

    // Notification
    if ( GetMedium() )
    {
        SfxShell::SetName( GetTitle(SFX_TITLE_APINAME) );
        Broadcast( SfxSimpleHint(SFX_HINT_TITLECHANGED) );
    }
}


#if OSL_DEBUG_LEVEL > 1
OUString X(const OUString &rRet)
{
    if ( rRet.isEmpty() )
        return OUString( "-empty-" );
    return rRet;
}
#else
#define X(ret) ret
#endif


OUString SfxObjectShell::GetTitle
(
    sal_uInt16  nMaxLength      /*  0 (default)
                                the title itself, as it is

                                1 (==SFX_TITLE_FILENAME)
                                provides the logical file name without path
                                (under WNT depending on the system settings
                                without extension)

                                2 (==SFX_TITLE_FULLNAME)
                                provides the logical file names with full path
                                (remote =>:: com:: sun:: star:: util:: URL)

                                3 (==SFX_TITLE_APINAME)
                                provides the logical filename without path
                                and extension

                                4 (==SFX_TITLE_DETECT)
                                provides the complete title, if not set yet
                                it will be created from DocInfo or the name of
                                the medium.

                                5 (==SFX_TITLE_CAPTION)
                                provides the Title just like MB now in the
                                CaptionBar view

                                6 (==SFX_TITLE_PICKLIST)
                                returns the Title, just like MB now would
                                display it in the PickList

                                7 (==SFX_TITLE_HISTORY)
                                returns the Title just like MB now would
                                display it in the History

                                10 bis USHRT_MAX
                                provides the 'nMaxLength' of the logical
                                file name including the path
                                (remote => css::util::URL)
                                */
) const

/*  [Description]

    Returns the title or logical file name of the document, depending on the
    'nMaxLength'.

    If the file name with path is used, the Name shortened by replacing one or
    more directory names with "...", URLs are currently always returned
    in complete form.
*/

{
    SfxMedium *pMed = GetMedium();
    if ( IsLoading() )
        return OUString();

    // Create Title?
    if ( SFX_TITLE_DETECT == nMaxLength && pImp->aTitle.isEmpty() )
    {
        static bool bRecur = false;
        if ( bRecur )
            return OUString("-not available-");
        bRecur = true;

        OUString aTitle;
        SfxObjectShell *pThis = const_cast<SfxObjectShell*>(this);

        if ( pMed )
        {
            const SfxStringItem* pNameItem = SfxItemSet::GetItem<SfxStringItem>(pMed->GetItemSet(), SID_DOCINFO_TITLE, false);
            if ( pNameItem )
                aTitle = pNameItem->GetValue();
        }

        if ( aTitle.isEmpty() )
            aTitle = GetTitle( SFX_TITLE_FILENAME );

        if ( IsTemplate() )
            pThis->SetTitle( aTitle );
        bRecur = false;
        return X(aTitle);
    }
    else if (SFX_TITLE_APINAME == nMaxLength )
        return X(GetAPIName());

    // Special case templates:
    if( IsTemplate() && !pImp->aTitle.isEmpty() &&
         ( nMaxLength == SFX_TITLE_CAPTION || nMaxLength == SFX_TITLE_PICKLIST ) )
        return X(pImp->aTitle);

    // Picklist/Caption is mapped
    if ( pMed && ( nMaxLength == SFX_TITLE_CAPTION || nMaxLength == SFX_TITLE_PICKLIST ) )
    {
        // If a specific title was given at open:
        // important for URLs: use INetProtocol::File for which the set title is not
        // considered. (See below, analysis of aTitleMap_Impl)
        const SfxStringItem* pNameItem = SfxItemSet::GetItem<SfxStringItem>(pMed->GetItemSet(), SID_DOCINFO_TITLE, false);
        if ( pNameItem )
            return X( pNameItem->GetValue() );
    }

    // Still unnamed?
    DBG_ASSERT( !HasName() || pMed, "HasName() but no Medium?!?" );
    if ( !HasName() || !pMed )
    {
        // Title already set?
        if ( !pImp->aTitle.isEmpty() )
            return X(pImp->aTitle);

        // must it be numbered?
        OUString aNoName(SFX2_RESSTR(STR_NONAME));
        if (pImp->bIsNamedVisible)
        {
            // Append number
            aNoName += OUString::number(pImp->nVisualDocumentNumber);
        }

        // Document called "noname" for the time being
        return X(aNoName);
    }

    const INetURLObject aURL( IsDocShared() ? GetSharedFileURL() : OUString( GetMedium()->GetName() ) );
    if ( nMaxLength > SFX_TITLE_CAPTION && nMaxLength <= SFX_TITLE_HISTORY )
    {
        sal_uInt16 nRemote;
        if( !pMed || aURL.GetProtocol() == INetProtocol::File )
            nRemote = 0;
        else
            nRemote = 1;
        nMaxLength = aTitleMap_Impl[nMaxLength-SFX_TITLE_CAPTION][nRemote];
    }

    // Local file?
    if ( aURL.GetProtocol() == INetProtocol::File )
    {
        OUString aName( aURL.HasMark() ? INetURLObject( aURL.GetURLNoMark() ).PathToFileName() : aURL.PathToFileName() );
        if ( nMaxLength == SFX_TITLE_FULLNAME )
            return X( aName );
        else if ( nMaxLength == SFX_TITLE_FILENAME )
            return X( aURL.getName( INetURLObject::LAST_SEGMENT,
                true, INetURLObject::DECODE_WITH_CHARSET ) );
        else if ( pImp->aTitle.isEmpty() )
            pImp->aTitle = aURL.getBase( INetURLObject::LAST_SEGMENT,
                                         true, INetURLObject::DECODE_WITH_CHARSET );
    }
    else
    {
        if ( nMaxLength >= SFX_TITLE_MAXLEN )
        {
            OUString aComplete( aURL.GetMainURL( INetURLObject::NO_DECODE ) );
            if( aComplete.getLength() > nMaxLength )
            {
                OUString aRet( "..." );
                aRet += aComplete.copy( aComplete.getLength() - nMaxLength + 3, nMaxLength - 3 );
                return X( aRet );
            }
            else
                return X( aComplete );
        }
        else if ( nMaxLength == SFX_TITLE_FILENAME )
        {
            OUString aName( aURL.GetBase() );
            aName = INetURLObject::decode( aName, INetURLObject::DECODE_WITH_CHARSET );
            if( aName.isEmpty() )
                aName = aURL.GetURLNoPass();
            return X(aName);
        }
        else if ( nMaxLength == SFX_TITLE_FULLNAME )
            return X(aURL.GetMainURL( INetURLObject::DECODE_TO_IURI ));

        // Generate Title from file name if possible
        if ( pImp->aTitle.isEmpty() )
            pImp->aTitle = aURL.GetBase();

        // workaround for the case when the name can not be retrieved from URL by INetURLObject
        if ( pImp->aTitle.isEmpty() )
            pImp->aTitle = aURL.GetMainURL( INetURLObject::DECODE_WITH_CHARSET );
    }

    // Complete Title
    return X(pImp->aTitle);
}


void SfxObjectShell::InvalidateName()

/*  [Description]

    Returns the title of the new document, DocInfo-Title or
    File name. Is required for loading from template or SaveAs.
*/

{
    pImp->aTitle.clear();
    SetName( GetTitle( SFX_TITLE_APINAME ) );

    Broadcast( SfxSimpleHint(SFX_HINT_TITLECHANGED) );
}


void SfxObjectShell::SetNamedVisibility_Impl()
{
    if ( !pImp->bIsNamedVisible )
    {
        pImp->bIsNamedVisible = true;
        if ( !HasName() && USHRT_MAX == pImp->nVisualDocumentNumber && pImp->aTitle.isEmpty() )
        {
            pImp->nVisualDocumentNumber = SfxGetpApp()->GetFreeIndex();
            Broadcast( SfxSimpleHint(SFX_HINT_TITLECHANGED) );
        }
    }

    SetName( GetTitle(SFX_TITLE_APINAME) );
}

void SfxObjectShell::SetNoName()
{
    bHasName = false;
    GetModel()->attachResource( OUString(), GetModel()->getArgs() );
}


SfxProgress* SfxObjectShell::GetProgress() const
{
    return pImp->pProgress;
}


void SfxObjectShell::SetProgress_Impl
(
    SfxProgress *pProgress      /* to started <SfxProgress> or 0,
                                   if the progress is to be reset */
)

/*  [Description]

    Internal method to set or reset the Progress modes for
    SfxObjectShell.
*/

{
    DBG_ASSERT( ( !pImp->pProgress && pProgress ) ||
                ( pImp->pProgress && !pProgress ),
                "Progress activation/deactivation mismatch" );
    pImp->pProgress = pProgress;
}


void SfxObjectShell::PostActivateEvent_Impl( SfxViewFrame* pFrame )
{
    SfxApplication* pSfxApp = SfxGetpApp();
    if ( !pSfxApp->IsDowning() && !IsLoading() && pFrame && !pFrame->GetFrame().IsClosing_Impl() )
    {
        const SfxBoolItem* pHiddenItem = SfxItemSet::GetItem<SfxBoolItem>(pMedium->GetItemSet(), SID_HIDDEN, false);
        if ( !pHiddenItem || !pHiddenItem->GetValue() )
        {
            sal_uInt16 nId = pImp->nEventId;
            pImp->nEventId = 0;
            if ( nId == SFX_EVENT_OPENDOC )
                pSfxApp->NotifyEvent(SfxViewEventHint( nId, GlobalEventConfig::GetEventName(GlobalEventId::OPENDOC), this, pFrame->GetFrame().GetController() ), false);
            else if (nId == SFX_EVENT_CREATEDOC )
                pSfxApp->NotifyEvent(SfxViewEventHint( nId, GlobalEventConfig::GetEventName(GlobalEventId::CREATEDOC), this, pFrame->GetFrame().GetController() ), false);
        }
    }
}


void SfxObjectShell::SetActivateEvent_Impl(sal_uInt16 nId )
{
    if ( GetFactory().GetFlags() & SfxObjectShellFlags::HASOPENDOC )
        pImp->nEventId = nId;
}

void SfxObjectShell::PrepareReload( )
/*  [Description]

    Is called before the Reload and gives the opportunity to clear any caches.
*/
{
}

bool SfxObjectShell::IsAutoLoadLocked() const

/* Returns whether an Autoload is allowed to be executed. Before the
   surrounding FrameSet of the AutoLoad is also taken into account as well.
*/

{
    return !IsReadOnly() || pImp->nAutoLoadLocks > 0;
}


void SfxObjectShell::BreakMacroSign_Impl( bool bBreakMacroSign )
{
    pImp->m_bMacroSignBroken = bBreakMacroSign;
}


void SfxObjectShell::CheckSecurityOnLoading_Impl()
{
    uno::Reference< task::XInteractionHandler > xInteraction;
    if ( GetMedium() )
        xInteraction = GetMedium()->GetInteractionHandler();

    // check if there is a broken signature...
    CheckForBrokenDocSignatures_Impl( xInteraction );

    CheckEncryption_Impl( xInteraction );

    // check macro security
    pImp->aMacroMode.checkMacrosOnLoading( xInteraction );
}


void SfxObjectShell::CheckEncryption_Impl( const uno::Reference< task::XInteractionHandler >& xHandler )
{
    OUString aVersion;
    bool bIsEncrypted = false;
    bool bHasNonEncrypted = false;

    try
    {
        uno::Reference < beans::XPropertySet > xPropSet( GetStorage(), uno::UNO_QUERY_THROW );
        xPropSet->getPropertyValue("Version") >>= aVersion;
        xPropSet->getPropertyValue("HasEncryptedEntries") >>= bIsEncrypted;
        xPropSet->getPropertyValue("HasNonEncryptedEntries") >>= bHasNonEncrypted;
    }
    catch( uno::Exception& )
    {
    }

    if ( aVersion.compareTo( ODFVER_012_TEXT ) >= 0 )
    {
        // this is ODF1.2 or later
        if ( bIsEncrypted && bHasNonEncrypted )
        {
            if ( !pImp->m_bIncomplEncrWarnShown )
            {
                // this is an encrypted document with nonencrypted streams inside, show the warning
                css::task::ErrorCodeRequest aErrorCode;
                aErrorCode.ErrCode = ERRCODE_SFX_INCOMPLETE_ENCRYPTION;

                SfxMedium::CallApproveHandler( xHandler, uno::makeAny( aErrorCode ), false );
                pImp->m_bIncomplEncrWarnShown = true;
            }

            // broken signatures imply no macro execution at all
            pImp->aMacroMode.disallowMacroExecution();
        }
    }
}


void SfxObjectShell::CheckForBrokenDocSignatures_Impl( const uno::Reference< task::XInteractionHandler >& xHandler )
{
    SignatureState nSignatureState = GetDocumentSignatureState();
    bool bSignatureBroken = ( nSignatureState == SignatureState::BROKEN );
    if ( !bSignatureBroken )
        return;

    pImp->showBrokenSignatureWarning( xHandler );

    // broken signatures imply no macro execution at all
    pImp->aMacroMode.disallowMacroExecution();
}


void SfxObjectShell::SetAutoLoad(
    const INetURLObject& rUrl, sal_uInt32 nTime, bool bReload )
{
    if ( pImp->pReloadTimer )
        DELETEZ(pImp->pReloadTimer);
    if ( bReload )
    {
        pImp->pReloadTimer = new AutoReloadTimer_Impl(
                                rUrl.GetMainURL( INetURLObject::DECODE_TO_IURI ),
                                nTime, this );
        pImp->pReloadTimer->Start();
    }
}

bool SfxObjectShell::IsLoadingFinished() const
{
    return ( pImp->nLoadedFlags == SfxLoadedFlags::ALL );
}

void SfxObjectShell::InitOwnModel_Impl()
{
    if ( !pImp->bModelInitialized )
    {
        const SfxStringItem* pSalvageItem = SfxItemSet::GetItem<SfxStringItem>(pMedium->GetItemSet(), SID_DOC_SALVAGE, false);
        if ( pSalvageItem )
        {
            pImp->aTempName = pMedium->GetPhysicalName();
            pMedium->GetItemSet()->ClearItem( SID_DOC_SALVAGE );
            pMedium->GetItemSet()->ClearItem( SID_FILE_NAME );
            pMedium->GetItemSet()->Put( SfxStringItem( SID_FILE_NAME, pMedium->GetOrigURL() ) );
        }
        else
        {
            pMedium->GetItemSet()->ClearItem( SID_PROGRESS_STATUSBAR_CONTROL );
            pMedium->GetItemSet()->ClearItem( SID_DOCUMENT );
        }

        pMedium->GetItemSet()->ClearItem( SID_REFERER );
        uno::Reference< frame::XModel >  xModel ( GetModel(), uno::UNO_QUERY );
        if ( xModel.is() )
        {
            OUString aURL = GetMedium()->GetOrigURL();
            SfxItemSet *pSet = GetMedium()->GetItemSet();
            if ( !GetMedium()->IsReadOnly() )
                pSet->ClearItem( SID_INPUTSTREAM );
            uno::Sequence< beans::PropertyValue > aArgs;
            TransformItems( SID_OPENDOC, *pSet, aArgs );
            xModel->attachResource( aURL, aArgs );
            impl_addToModelCollection(xModel);
        }

        pImp->bModelInitialized = true;
    }
}

void SfxObjectShell::FinishedLoading( SfxLoadedFlags nFlags )
{
    bool bSetModifiedTRUE = false;
    const SfxStringItem* pSalvageItem = SfxItemSet::GetItem<SfxStringItem>(pMedium->GetItemSet(), SID_DOC_SALVAGE, false);
    if( ( nFlags & SfxLoadedFlags::MAINDOCUMENT ) && !(pImp->nLoadedFlags & SfxLoadedFlags::MAINDOCUMENT )
        && !(pImp->nFlagsInProgress & SfxLoadedFlags::MAINDOCUMENT ))
    {
        pImp->nFlagsInProgress |= SfxLoadedFlags::MAINDOCUMENT;
        static_cast<SfxHeaderAttributes_Impl*>(GetHeaderAttributes())->SetAttributes();
        pImp->bImportDone = true;
        if( !IsAbortingImport() )
            PositionView_Impl();

        if ( ( GetModifyPasswordHash() || GetModifyPasswordInfo().getLength() ) && !IsModifyPasswordEntered() )
            SetReadOnly();

        // Salvage
        if ( pSalvageItem )
            bSetModifiedTRUE = true;

        if ( !IsEnableSetModified() )
            EnableSetModified();

        if( !bSetModifiedTRUE && IsEnableSetModified() )
            SetModified( false );

        CheckSecurityOnLoading_Impl();

        bHasName = true; // the document is loaded, so the name should already available
        GetTitle( SFX_TITLE_DETECT );
        InitOwnModel_Impl();
        pImp->nFlagsInProgress &= ~SfxLoadedFlags::MAINDOCUMENT;
    }

    if( ( nFlags & SfxLoadedFlags::IMAGES ) && !(pImp->nLoadedFlags & SfxLoadedFlags::IMAGES )
        && !(pImp->nFlagsInProgress & SfxLoadedFlags::IMAGES ))
    {
        pImp->nFlagsInProgress |= SfxLoadedFlags::IMAGES;
        uno::Reference<document::XDocumentProperties> xDocProps(
            getDocProperties());
        OUString url(xDocProps->getAutoloadURL());
        sal_Int32 delay(xDocProps->getAutoloadSecs());
        SetAutoLoad( INetURLObject(url), delay * 1000,
                     (delay > 0) || !url.isEmpty() );
        if( !bSetModifiedTRUE && IsEnableSetModified() )
            SetModified( false );
        Invalidate( SID_SAVEASDOC );
        pImp->nFlagsInProgress &= ~SfxLoadedFlags::IMAGES;
    }

    pImp->nLoadedFlags |= nFlags;

    if ( pImp->nFlagsInProgress == SfxLoadedFlags::NONE )
    {
        // in case of reentrance calls the first called FinishedLoading() call on the stack
        // should do the notification, in result the notification is done when all the FinishedLoading() calls are finished

        if ( bSetModifiedTRUE )
            SetModified();
        else
            SetModified( false );

        if ( (pImp->nLoadedFlags & SfxLoadedFlags::MAINDOCUMENT ) && (pImp->nLoadedFlags & SfxLoadedFlags::IMAGES ) )
        {
            const SfxBoolItem* pTemplateItem = SfxItemSet::GetItem<SfxBoolItem>(pMedium->GetItemSet(), SID_TEMPLATE, false);
            bool bTemplate = pTemplateItem && pTemplateItem->GetValue();

            // closing the streams on loading should be under control of SFX!
            DBG_ASSERT( pMedium->IsOpen(), "Don't close the medium when loading documents!" );

            if ( bTemplate )
            {
                TemplateDisconnectionAfterLoad();
            }
            else
            {
                // if a readonly medium has storage then it's stream is already based on temporary file
                if( !(pMedium->GetOpenMode() & StreamMode::WRITE) && !pMedium->HasStorage_Impl() )
                    // don't lock file opened read only
                    pMedium->CloseInStream();
            }
        }

        SetInitialized_Impl( false );

        // Title is not available until loading has finished
        Broadcast( SfxSimpleHint( SFX_HINT_TITLECHANGED ) );
        if ( pImp->nEventId )
            PostActivateEvent_Impl(SfxViewFrame::GetFirst(this));
    }
}


void SfxObjectShell::TemplateDisconnectionAfterLoad()
{
    // document is created from a template
    //TODO/LATER: should the templates always be XML docs!

    SfxMedium* pTmpMedium = pMedium;
    if ( pTmpMedium )
    {
        OUString aName( pTmpMedium->GetName() );
        const SfxStringItem* pTemplNamItem = SfxItemSet::GetItem<SfxStringItem>(pTmpMedium->GetItemSet(), SID_TEMPLATE_NAME, false);
        OUString aTemplateName;
        if ( pTemplNamItem )
            aTemplateName = pTemplNamItem->GetValue();
        else
        {
            // !TODO/LATER: what's this?!
            // Interactive ( DClick, Contextmenu ) no long name is included
            aTemplateName = getDocProperties()->getTitle();
            if ( aTemplateName.isEmpty() )
            {
                INetURLObject aURL( aName );
                aURL.CutExtension();
                aTemplateName = aURL.getName( INetURLObject::LAST_SEGMENT, true, INetURLObject::DECODE_WITH_CHARSET );
            }
        }

        // set medium to noname
        pTmpMedium->SetName( OUString(), true );
        pTmpMedium->Init_Impl();

        // drop resource
        SetNoName();
        InvalidateName();

        if( IsPackageStorageFormat_Impl( *pTmpMedium ) )
        {
            // untitled document must be based on temporary storage
            // the medium should not dispose the storage in this case
            uno::Reference < embed::XStorage > xTmpStor = ::comphelper::OStorageHelper::GetTemporaryStorage();
            GetStorage()->copyToStorage( xTmpStor );

            // the medium should disconnect from the original location
            // the storage should not be disposed since the document is still
            // based on it, but in DoSaveCompleted it will be disposed
            pTmpMedium->CanDisposeStorage_Impl( false );
            pTmpMedium->Close();

            // setting the new storage the medium will be based on
            pTmpMedium->SetStorage_Impl( xTmpStor );

            pMedium = nullptr;
            bool ok = DoSaveCompleted( pTmpMedium );
            assert(pMedium != nullptr);
            if( ok )
            {
                const SfxStringItem* pSalvageItem = SfxItemSet::GetItem<SfxStringItem>(pMedium->GetItemSet(), SID_DOC_SALVAGE, false);
                bool bSalvage = pSalvageItem != nullptr;

                if ( !bSalvage )
                {
                    // some further initializations for templates
                    SetTemplate_Impl( aName, aTemplateName, this );
                }

                // the medium should not dispose the storage, DoSaveCompleted() has let it to do so
                pTmpMedium->CanDisposeStorage_Impl( false );
            }
            else
            {
                SetError( ERRCODE_IO_GENERAL, OSL_LOG_PREFIX );
            }
        }
        else
        {
            // some further initializations for templates
            SetTemplate_Impl( aName, aTemplateName, this );
            pTmpMedium->CreateTempFile();
        }

        // templates are never readonly
        pTmpMedium->GetItemSet()->ClearItem( SID_DOC_READONLY );
        pTmpMedium->SetOpenMode( SFX_STREAM_READWRITE, true );

        // notifications about possible changes in readonly state and document info
        Broadcast( SfxSimpleHint(SFX_HINT_MODECHANGED) );

        // created untitled document can't be modified
        SetModified( false );
    }
}


void SfxObjectShell::PositionView_Impl()
{
    MarkData_Impl *pMark = Get_Impl()->pMarkData;
    if( pMark )
    {
        SfxViewShell* pSh = pMark->pFrame->GetViewShell();
        if( !pMark->aUserData.isEmpty() )
            pSh->ReadUserData( pMark->aUserData, true );
        else if( !pMark->aMark.isEmpty() )
            pSh->JumpToMark( pMark->aMark );
        DELETEZ( Get_Impl()->pMarkData );
    }
}


bool SfxObjectShell::IsLoading() const
/*  [Description]

    Has FinishedLoading been called?
*/
{
    return !( pImp->nLoadedFlags & SfxLoadedFlags::MAINDOCUMENT );
}


void SfxObjectShell::CancelTransfers()
/*  [Description]

    Here can Transfers get canceled, which were not registered
    by RegisterTransfer.
*/
{
    if( ( pImp->nLoadedFlags & SfxLoadedFlags::ALL ) != SfxLoadedFlags::ALL )
    {
        AbortImport();
        if( IsLoading() )
            FinishedLoading();
    }
}


AutoReloadTimer_Impl::AutoReloadTimer_Impl(
    const OUString& rURL, sal_uInt32 nTime, SfxObjectShell* pSh )
    : aUrl( rURL ), pObjSh( pSh )
{
    SetTimeout( nTime );
}


void AutoReloadTimer_Impl::Invoke()
{
    SfxViewFrame *pFrame = SfxViewFrame::GetFirst( pObjSh );

    if ( pFrame )
    {
        // Not possible/meanigfull at the moment?
        if ( !pObjSh->CanReload_Impl() || pObjSh->IsAutoLoadLocked() || Application::IsUICaptured() )
        {
            // Allow a retry
            Start();
            return;
        }

        SfxAllItemSet aSet( SfxGetpApp()->GetPool() );
        aSet.Put( SfxBoolItem( SID_AUTOLOAD, true ) );
        if ( !aUrl.isEmpty() )
            aSet.Put(  SfxStringItem( SID_FILE_NAME, aUrl ) );
        if (pObjSh->HasName()) {
            aSet.Put(
                SfxStringItem(SID_REFERER, pObjSh->GetMedium()->GetName()));
        }
        SfxRequest aReq( SID_RELOAD, SfxCallMode::SLOT, aSet );
        pObjSh->Get_Impl()->pReloadTimer = nullptr;
        delete this;
        pFrame->ExecReload_Impl( aReq );
        return;
    }

    pObjSh->Get_Impl()->pReloadTimer = nullptr;
    delete this;
}

SfxModule* SfxObjectShell::GetModule() const
{
    return GetFactory().GetModule();
}

ErrCode SfxObjectShell::CallBasic( const OUString& rMacro,
    const OUString& rBasic, SbxArray* pArgs,
    SbxValue* pRet )
{
    SfxApplication* pApp = SfxGetpApp();
    if( pApp->GetName() != rBasic )
    {
        if ( !AdjustMacroMode( OUString() ) )
            return ERRCODE_IO_ACCESSDENIED;
    }

    BasicManager *pMgr = GetBasicManager();
    if( pApp->GetName() == rBasic )
        pMgr = SfxApplication::GetBasicManager();
    ErrCode nRet = SfxApplication::CallBasic( rMacro, pMgr, pArgs, pRet );
    return nRet;
}

namespace
{
    static bool lcl_isScriptAccessAllowed_nothrow( const Reference< XInterface >& _rxScriptContext )
    {
        try
        {
            Reference< XEmbeddedScripts > xScripts( _rxScriptContext, UNO_QUERY );
            if ( !xScripts.is() )
            {
                Reference< XScriptInvocationContext > xContext( _rxScriptContext, UNO_QUERY_THROW );
                xScripts.set( xContext->getScriptContainer(), UNO_SET_THROW );
            }

            return xScripts->getAllowMacroExecution();
        }
        catch( const Exception& )
        {
            DBG_UNHANDLED_EXCEPTION();
        }
        return false;
    }
}

ErrCode SfxObjectShell::CallXScript( const Reference< XInterface >& _rxScriptContext, const OUString& _rScriptURL,
    const Sequence< Any >& aParams, Any& aRet, Sequence< sal_Int16 >& aOutParamIndex, Sequence< Any >& aOutParam, bool bRaiseError, const css::uno::Any* pCaller )
{
    OSL_TRACE( "in CallXScript" );
    ErrCode nErr = ERRCODE_NONE;

    bool bIsDocumentScript = ( _rScriptURL.indexOf( "location=document" ) >= 0 );
        // TODO: we should parse the URL, and check whether there is a parameter with this name.
        // Otherwise, we might find too much.
    if ( bIsDocumentScript && !lcl_isScriptAccessAllowed_nothrow( _rxScriptContext ) )
        return ERRCODE_IO_ACCESSDENIED;

    bool bCaughtException = false;
    Any aException;
    try
    {
        // obtain/create a script provider
        Reference< provider::XScriptProvider > xScriptProvider;
        Reference< provider::XScriptProviderSupplier > xSPS( _rxScriptContext, UNO_QUERY );
        if ( xSPS.is() )
            xScriptProvider.set( xSPS->getScriptProvider() );

        if ( !xScriptProvider.is() )
        {
            Reference< provider::XScriptProviderFactory > xScriptProviderFactory =
                provider::theMasterScriptProviderFactory::get( ::comphelper::getProcessComponentContext() );
            xScriptProvider.set( xScriptProviderFactory->createScriptProvider( makeAny( _rxScriptContext ) ), UNO_SET_THROW );
        }

        // ry to protect the invocation context's undo manager (if present), just in case the script tampers with it
        ::framework::DocumentUndoGuard aUndoGuard( _rxScriptContext.get() );

        // obtain the script, and execute it
        Reference< provider::XScript > xScript( xScriptProvider->getScript( _rScriptURL ), UNO_QUERY_THROW );
        if ( pCaller && pCaller->hasValue() )
        {
            Reference< beans::XPropertySet > xProps( xScript, uno::UNO_QUERY );
            if ( xProps.is() )
            {
                Sequence< uno::Any > aArgs( 1 );
                aArgs[ 0 ] = *pCaller;
                xProps->setPropertyValue("Caller", uno::makeAny( aArgs ) );
            }
        }
        aRet = xScript->invoke( aParams, aOutParamIndex, aOutParam );
    }
    catch ( const uno::Exception& )
    {
        aException = ::cppu::getCaughtException();
        bCaughtException = true;
        nErr = ERRCODE_BASIC_INTERNAL_ERROR;
    }

    if ( bCaughtException && bRaiseError )
    {
        std::unique_ptr< VclAbstractDialog > pScriptErrDlg;
        SfxAbstractDialogFactory* pFact = SfxAbstractDialogFactory::Create();
        if ( pFact )
            pScriptErrDlg.reset( pFact->CreateScriptErrorDialog( nullptr, aException ) );
        OSL_ENSURE( pScriptErrDlg.get(), "SfxObjectShell::CallXScript: no script error dialog!" );

        if ( pScriptErrDlg.get() )
            pScriptErrDlg->Execute();
    }

    OSL_TRACE( "leaving CallXScript" );
    return nErr;
}

// perhaps rename to CallScript once we get rid of the existing CallScript
// and Call, CallBasic, CallStarBasic methods
ErrCode SfxObjectShell::CallXScript( const OUString& rScriptURL,
        const css::uno::Sequence< css::uno::Any >& aParams,
        css::uno::Any& aRet,
        css::uno::Sequence< sal_Int16 >& aOutParamIndex,
        css::uno::Sequence< css::uno::Any >& aOutParam,
        bool bRaiseError,
        const css::uno::Any* pCaller )
{
    return CallXScript( GetModel(), rScriptURL, aParams, aRet, aOutParamIndex, aOutParam, bRaiseError, pCaller );
}

SfxObjectShellFlags SfxObjectShell::GetFlags() const
{
    if( pImp->eFlags == SfxObjectShellFlags::UNDEFINED )
        pImp->eFlags = GetFactory().GetFlags();
    return pImp->eFlags;
}

void SfxHeaderAttributes_Impl::SetAttributes()
{
    bAlert = true;
    SvKeyValue aPair;
    for( bool bCont = xIter->GetFirst( aPair ); bCont;
         bCont = xIter->GetNext( aPair ) )
        SetAttribute( aPair );
}

void SfxHeaderAttributes_Impl::SetAttribute( const SvKeyValue& rKV )
{
    OUString aValue = rKV.GetValue();
    if( rKV.GetKey().equalsIgnoreAsciiCase("refresh") && !rKV.GetValue().isEmpty() )
    {
        sal_uInt32 nTime = aValue.getToken(  0, ';' ).toInt32() ;
        OUString aURL = comphelper::string::strip(aValue.getToken( 1, ';' ), ' ');
        uno::Reference<document::XDocumentProperties> xDocProps(
            pDoc->getDocProperties());
        if( aURL.startsWithIgnoreAsciiCase( "url=" ) )
        {
            INetURLObject aObj;
            INetURLObject( pDoc->GetMedium()->GetName() ).GetNewAbsURL( aURL.copy( 4 ), &aObj );
            xDocProps->setAutoloadURL(
                aObj.GetMainURL( INetURLObject::NO_DECODE ) );
        }
        try
        {
            xDocProps->setAutoloadSecs( nTime );
        }
        catch (lang::IllegalArgumentException &)
        {
            // ignore
        }
    }
    else if( rKV.GetKey().equalsIgnoreAsciiCase( "expires" ) )
    {
        DateTime aDateTime( DateTime::EMPTY );
        if( INetMIMEMessage::ParseDateField( rKV.GetValue(), aDateTime ) )
        {
            aDateTime.ConvertToLocalTime();
            pDoc->GetMedium()->SetExpired_Impl( aDateTime );
        }
        else
        {
            pDoc->GetMedium()->SetExpired_Impl( Date( 1, 1, 1970 ) );
        }
    }
}

void SfxHeaderAttributes_Impl::Append( const SvKeyValue& rKV )
{
    xIter->Append( rKV );
    if( bAlert ) SetAttribute( rKV );
}

SvKeyValueIterator* SfxObjectShell::GetHeaderAttributes()
{
    if( !pImp->xHeaderAttributes.Is() )
    {
        DBG_ASSERT( pMedium, "No Medium" );
        pImp->xHeaderAttributes = new SfxHeaderAttributes_Impl( this );
    }
    return static_cast<SvKeyValueIterator*>( &pImp->xHeaderAttributes );
}

void SfxObjectShell::ClearHeaderAttributesForSourceViewHack()
{
    static_cast<SfxHeaderAttributes_Impl*>(GetHeaderAttributes())
        ->ClearForSourceView();
}


void SfxObjectShell::SetHeaderAttributesForSourceViewHack()
{
    static_cast<SfxHeaderAttributes_Impl*>(GetHeaderAttributes())
        ->SetAttributes();
}

bool SfxObjectShell::IsPreview() const
{
    if ( !pMedium )
        return false;

    bool bPreview = false;
    const SfxStringItem* pFlags = SfxItemSet::GetItem<SfxStringItem>(pMedium->GetItemSet(), SID_OPTIONS, false);
    if ( pFlags )
    {
        // Distributed values among individual items
        OUString aFileFlags = pFlags->GetValue();
        aFileFlags = aFileFlags.toAsciiUpperCase();
        if ( -1 != aFileFlags.indexOf( 'B' ) )
            bPreview = true;
    }

    if ( !bPreview )
    {
        const SfxBoolItem* pItem = SfxItemSet::GetItem<SfxBoolItem>(pMedium->GetItemSet(), SID_PREVIEW, false);
        if ( pItem )
            bPreview = pItem->GetValue();
    }

    return bPreview;
}

void SfxObjectShell::SetWaitCursor( bool bSet ) const
{
    for( SfxViewFrame* pFrame = SfxViewFrame::GetFirst( this ); pFrame; pFrame = SfxViewFrame::GetNext( *pFrame, this ) )
    {
        if ( bSet )
            pFrame->GetFrame().GetWindow().EnterWait();
        else
            pFrame->GetFrame().GetWindow().LeaveWait();
    }
}

OUString SfxObjectShell::GetAPIName() const
{
    INetURLObject aURL( IsDocShared() ? GetSharedFileURL() : OUString( GetMedium()->GetName() ) );
    OUString aName( aURL.GetBase() );
    if( aName.isEmpty() )
        aName = aURL.GetURLNoPass();
    if ( aName.isEmpty() )
        aName = GetTitle( SFX_TITLE_DETECT );
    return aName;
}

void SfxObjectShell::Invalidate( sal_uInt16 nId )
{
    for( SfxViewFrame* pFrame = SfxViewFrame::GetFirst( this ); pFrame; pFrame = SfxViewFrame::GetNext( *pFrame, this ) )
        Invalidate_Impl( pFrame->GetBindings(), nId );
}

bool SfxObjectShell::AdjustMacroMode( const OUString& /*rScriptType*/, bool bSuppressUI )
{
    uno::Reference< task::XInteractionHandler > xInteraction;
    if ( pMedium && !bSuppressUI )
        xInteraction = pMedium->GetInteractionHandler();

    CheckForBrokenDocSignatures_Impl( xInteraction );

    CheckEncryption_Impl( xInteraction );

    return pImp->aMacroMode.adjustMacroMode( xInteraction );
}

vcl::Window* SfxObjectShell::GetDialogParent( SfxMedium* pLoadingMedium )
{
    vcl::Window* pWindow = nullptr;
    SfxItemSet* pSet = pLoadingMedium ? pLoadingMedium->GetItemSet() : GetMedium()->GetItemSet();
    const SfxUnoFrameItem* pUnoItem = SfxItemSet::GetItem<SfxUnoFrameItem>(pSet, SID_FILLFRAME, false);
    if ( pUnoItem )
    {
        uno::Reference < frame::XFrame > xFrame( pUnoItem->GetFrame() );
        pWindow = VCLUnoHelper::GetWindow( xFrame->getContainerWindow() );
    }

    if ( !pWindow )
    {
        SfxFrame* pFrame = nullptr;
        const SfxFrameItem* pFrameItem = SfxItemSet::GetItem<SfxFrameItem>(pSet, SID_DOCFRAME, false);
        if( pFrameItem && pFrameItem->GetFrame() )
            // get target frame from ItemSet
            pFrame = pFrameItem->GetFrame();
        else
        {
            // try the current frame
            SfxViewFrame* pView = SfxViewFrame::Current();
            if ( !pView || pView->GetObjectShell() != this )
                // get any visible frame
                pView = SfxViewFrame::GetFirst(this);
            if ( pView )
                pFrame = &pView->GetFrame();
        }

        if ( pFrame )
            // get topmost window
            pWindow = VCLUnoHelper::GetWindow( pFrame->GetFrameInterface()->getContainerWindow() );
    }

    if ( pWindow )
    {
        // this frame may be invisible, show it if it is allowed
        const SfxBoolItem* pHiddenItem = SfxItemSet::GetItem<SfxBoolItem>(pSet, SID_HIDDEN, false);
        if ( !pHiddenItem || !pHiddenItem->GetValue() )
        {
            pWindow->Show();
            pWindow->ToTop();
        }
    }

    return pWindow;
}

void SfxObjectShell::SetCreateMode_Impl( SfxObjectCreateMode nMode )
{
    eCreateMode = nMode;
}

bool SfxObjectShell::IsInPlaceActive()
{
    if ( eCreateMode != SfxObjectCreateMode::EMBEDDED )
        return false;

    SfxViewFrame* pFrame = SfxViewFrame::GetFirst( this );
    return pFrame && pFrame->GetFrame().IsInPlace();
}

bool SfxObjectShell::IsUIActive()
{
    if ( eCreateMode != SfxObjectCreateMode::EMBEDDED )
        return false;

    SfxViewFrame* pFrame = SfxViewFrame::GetFirst( this );
    return pFrame && pFrame->GetFrame().IsInPlace() && pFrame->GetFrame().GetWorkWindow_Impl()->IsVisible_Impl();
}

void SfxObjectShell::InPlaceActivate( bool )
{
}

bool SfxObjectShell::UseInteractionToHandleError(
                    const uno::Reference< task::XInteractionHandler >& xHandler,
                    sal_uInt32 nError )
{
    bool bResult = false;

    if ( xHandler.is() )
    {
        try
        {
            uno::Any aInteraction;
            uno::Sequence< uno::Reference< task::XInteractionContinuation > > lContinuations(2);
            ::comphelper::OInteractionAbort* pAbort = new ::comphelper::OInteractionAbort();
            ::comphelper::OInteractionApprove* pApprove = new ::comphelper::OInteractionApprove();
            lContinuations[0].set( static_cast< task::XInteractionContinuation* >( pAbort ), uno::UNO_QUERY );
            lContinuations[1].set( static_cast< task::XInteractionContinuation* >( pApprove ), uno::UNO_QUERY );

            task::ErrorCodeRequest aErrorCode;
            aErrorCode.ErrCode = nError;
            aInteraction <<= aErrorCode;
            xHandler->handle(::framework::InteractionRequest::CreateRequest (aInteraction,lContinuations));
            bResult = pAbort->wasSelected();
        }
        catch( uno::Exception& )
        {}
    }

    return bResult;
}

sal_Int16 SfxObjectShell_Impl::getCurrentMacroExecMode() const
{
    sal_Int16 nImposedExecMode( MacroExecMode::NEVER_EXECUTE );

    const SfxMedium* pMedium( rDocShell.GetMedium() );
    OSL_PRECOND( pMedium, "SfxObjectShell_Impl::getCurrentMacroExecMode: no medium!" );
    if ( pMedium )
    {
        const SfxUInt16Item* pMacroModeItem = SfxItemSet::GetItem<SfxUInt16Item>(pMedium->GetItemSet(), SID_MACROEXECMODE, false);
        if ( pMacroModeItem )
            nImposedExecMode = pMacroModeItem->GetValue();
    }
    return nImposedExecMode;
}

bool SfxObjectShell_Impl::setCurrentMacroExecMode( sal_uInt16 nMacroMode )
{
    const SfxMedium* pMedium( rDocShell.GetMedium() );
    OSL_PRECOND( pMedium, "SfxObjectShell_Impl::getCurrentMacroExecMode: no medium!" );
    if ( pMedium )
    {
        pMedium->GetItemSet()->Put( SfxUInt16Item( SID_MACROEXECMODE, nMacroMode ) );
        return true;
    }

    return false;
}

OUString SfxObjectShell_Impl::getDocumentLocation() const
{
    OUString sLocation;

    const SfxMedium* pMedium( rDocShell.GetMedium() );
    OSL_PRECOND( pMedium, "SfxObjectShell_Impl::getDocumentLocation: no medium!" );
    if ( pMedium )
    {
        sLocation = pMedium->GetName();
        if ( sLocation.isEmpty() )
        {
            // for documents made from a template: get the name of the template
            sLocation = rDocShell.getDocProperties()->getTemplateURL();
        }
    }
    return sLocation;
}

bool SfxObjectShell_Impl::documentStorageHasMacros() const
{
    return ::sfx2::DocumentMacroMode::storageHasMacros( m_xDocStorage );
}

Reference< XEmbeddedScripts > SfxObjectShell_Impl::getEmbeddedDocumentScripts() const
{
    return Reference< XEmbeddedScripts >( rDocShell.GetModel(), UNO_QUERY );
}

SignatureState SfxObjectShell_Impl::getScriptingSignatureState()
{
    SignatureState nSignatureState( rDocShell.GetScriptingSignatureState() );

    if ( nSignatureState != SignatureState::NOSIGNATURES && m_bMacroSignBroken )
    {
        // if there is a macro signature it must be handled as broken
        nSignatureState = SignatureState::BROKEN;
    }

    return nSignatureState;
}

bool SfxObjectShell_Impl::hasTrustedScriptingSignature( bool bAllowUIToAddAuthor )
{
    bool bResult = false;

    try
    {
        OUString aVersion;
        try
        {
            uno::Reference < beans::XPropertySet > xPropSet( rDocShell.GetStorage(), uno::UNO_QUERY_THROW );
            xPropSet->getPropertyValue("Version") >>= aVersion;
        }
        catch( uno::Exception& )
        {
        }

        uno::Reference< security::XDocumentDigitalSignatures > xSigner( security::DocumentDigitalSignatures::createWithVersion(comphelper::getProcessComponentContext(), aVersion) );

        if ( nScriptingSignatureState == SignatureState::UNKNOWN
          || nScriptingSignatureState == SignatureState::OK
          || nScriptingSignatureState == SignatureState::NOTVALIDATED )
        {
            uno::Sequence< security::DocumentSignatureInformation > aInfo = rDocShell.ImplAnalyzeSignature( true, xSigner );

            if ( aInfo.getLength() )
            {
                if ( nScriptingSignatureState == SignatureState::UNKNOWN )
                    nScriptingSignatureState = rDocShell.ImplCheckSignaturesInformation( aInfo );

                if ( nScriptingSignatureState == SignatureState::OK
                  || nScriptingSignatureState == SignatureState::NOTVALIDATED )
                {
                    for ( sal_Int32 nInd = 0; !bResult && nInd < aInfo.getLength(); nInd++ )
                    {
                        bResult = xSigner->isAuthorTrusted( aInfo[nInd].Signer );
                    }

                    if ( !bResult && bAllowUIToAddAuthor )
                    {
                        uno::Reference< task::XInteractionHandler > xInteraction;
                        if ( rDocShell.GetMedium() )
                            xInteraction = rDocShell.GetMedium()->GetInteractionHandler();

                        if ( xInteraction.is() )
                        {
                            task::DocumentMacroConfirmationRequest aRequest;
                            aRequest.DocumentURL = getDocumentLocation();
                            aRequest.DocumentStorage = rDocShell.GetMedium()->GetZipStorageToSign_Impl();
                            aRequest.DocumentSignatureInformation = aInfo;
                            aRequest.DocumentVersion = aVersion;
                            aRequest.Classification = task::InteractionClassification_QUERY;
                            bResult = SfxMedium::CallApproveHandler( xInteraction, uno::makeAny( aRequest ), true );
                        }
                    }
                }
            }
        }
    }
    catch( uno::Exception& )
    {}

    return bResult;
}

void SfxObjectShell_Impl::showBrokenSignatureWarning( const uno::Reference< task::XInteractionHandler >& _rxInteraction ) const
{
    if  ( !bSignatureErrorIsShown )
    {
        SfxObjectShell::UseInteractionToHandleError( _rxInteraction, ERRCODE_SFX_BROKENSIGNATURE );
        const_cast< SfxObjectShell_Impl* >( this )->bSignatureErrorIsShown = true;
    }
}

void SfxObjectShell::AddLog( const OUString& aMessage )
{
    if ( !pImp->m_xLogRing.is() )
    {
        try
        {
            Reference<XComponentContext> xContext( ::comphelper::getProcessComponentContext() );
            pImp->m_xLogRing.set( logging::DocumentIOLogRing::get(xContext) );
        }
        catch( uno::Exception& )
        {}
    }

    if ( pImp->m_xLogRing.is() )
        pImp->m_xLogRing->logString( aMessage );
}

namespace {

void WriteStringInStream( const uno::Reference< io::XOutputStream >& xOutStream, const OUString& aString )
{
    if ( xOutStream.is() )
    {
        OString aStrLog = OUStringToOString( aString, RTL_TEXTENCODING_UTF8 );
        uno::Sequence< sal_Int8 > aLogData( reinterpret_cast<const sal_Int8*>(aStrLog.getStr()), aStrLog.getLength() );
        xOutStream->writeBytes( aLogData );

        aLogData.realloc( 1 );
        aLogData[0] = '\n';
        xOutStream->writeBytes( aLogData );
    }
}

}

void SfxObjectShell::StoreLog()
{
    if ( !pImp->m_xLogRing.is() )
    {
        try
        {
            Reference<XComponentContext> xContext( ::comphelper::getProcessComponentContext() );
            pImp->m_xLogRing.set( logging::DocumentIOLogRing::get(xContext) );
        }
        catch( uno::Exception& )
        {}
    }

    if ( pImp->m_xLogRing.is() )
    {
        OUString aFileURL = ( "${$BRAND_BASE_DIR/" LIBO_ETC_FOLDER "/" SAL_CONFIGFILE("bootstrap") ":UserInstallation}"  );

        ::rtl::Bootstrap::expandMacros( aFileURL );

        OUString aBuildID = ( "${$BRAND_BASE_DIR/" LIBO_ETC_FOLDER "/" SAL_CONFIGFILE("setup") ":buildid}"  );

        ::rtl::Bootstrap::expandMacros( aBuildID );

        if ( !aFileURL.isEmpty() )
        {
            aFileURL += "/user/temp/document_io_logring.txt";
            try
            {
                uno::Reference< uno::XComponentContext > xContext( ::comphelper::getProcessComponentContext() );
                uno::Reference< ucb::XSimpleFileAccess3 > xSimpleFileAccess(ucb::SimpleFileAccess::create(xContext));
                uno::Reference< io::XStream > xStream( xSimpleFileAccess->openFileReadWrite( aFileURL ), uno::UNO_SET_THROW );
                uno::Reference< io::XOutputStream > xOutStream( xStream->getOutputStream(), uno::UNO_SET_THROW );
                uno::Reference< io::XTruncate > xTruncate( xOutStream, uno::UNO_QUERY_THROW );
                xTruncate->truncate();

                if ( !aBuildID.isEmpty() )
                    WriteStringInStream( xOutStream, aBuildID );

                uno::Sequence< OUString > aLogSeq = pImp->m_xLogRing->getCollectedLog();
                for ( sal_Int32 nInd = 0; nInd < aLogSeq.getLength(); nInd++ )
                    WriteStringInStream( xOutStream, aLogSeq[nInd] );
            }
            catch( uno::Exception& )
            {}
        }
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
