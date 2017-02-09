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


#include <com/sun/star/uno/Reference.hxx>

#include <com/sun/star/document/DocumentProperties.hpp>
#include <com/sun/star/document/XDocumentProperties.hpp>
#include <com/sun/star/document/UpdateDocMode.hpp>
#include <com/sun/star/frame/XLayoutManager.hpp>
#include <com/sun/star/embed/ElementModes.hpp>
#include <comphelper/fileurl.hxx>
#include <vcl/msgbox.hxx>
#include <svl/style.hxx>
#include <vcl/wrkwin.hxx>

#include <svl/stritem.hxx>
#include <svl/intitem.hxx>
#include <svl/rectitem.hxx>
#include <svl/eitem.hxx>
#include <svl/urihelper.hxx>
#include <svl/ctloptions.hxx>
#include <comphelper/storagehelper.hxx>
#include <comphelper/processfactory.hxx>
#include <unotools/securityoptions.hxx>
#include <svtools/sfxecode.hxx>
#include <svtools/ehdl.hxx>
#include <tools/datetime.hxx>
#include <rtl/uri.hxx>
#include <math.h>

#include <unotools/saveopt.hxx>
#include <unotools/useroptions.hxx>
#include <vcl/virdev.hxx>
#include <vcl/oldprintadaptor.hxx>
#include <vcl/settings.hxx>

#include <sfx2/app.hxx>
#include <sfx2/sfxresid.hxx>
#include "appdata.hxx"
#include <sfx2/dinfdlg.hxx>
#include <sfx2/fcontnr.hxx>
#include <sfx2/docfac.hxx>
#include <sfx2/viewsh.hxx>
#include <sfx2/objsh.hxx>
#include "objshimp.hxx"
#include <sfx2/evntconf.hxx>
#include <sfx2/sfxhelp.hxx>
#include <sfx2/dispatch.hxx>
#include <sfx2/printer.hxx>
#include <basic/basmgr.hxx>
#include <svtools/svtools.hrc>
#include <sfx2/viewfrm.hxx>
#include <sfx2/doctempl.hxx>
#include "doc.hrc"
#include <sfx2/sfxbasemodel.hxx>
#include <sfx2/docfile.hxx>
#include <sfx2/request.hxx>
#include "openflag.hxx"
#include "querytemplate.hxx"
#include <memory>

#include <LibreOfficeKit/LibreOfficeKitTypes.h>

#include <typeinfo>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;


static
bool operator> (const util::DateTime& i_rLeft, const util::DateTime& i_rRight)
{
    if ( i_rLeft.Year != i_rRight.Year )
        return i_rLeft.Year > i_rRight.Year;

    if ( i_rLeft.Month != i_rRight.Month )
        return i_rLeft.Month > i_rRight.Month;

    if ( i_rLeft.Day != i_rRight.Day )
        return i_rLeft.Day > i_rRight.Day;

    if ( i_rLeft.Hours != i_rRight.Hours )
        return i_rLeft.Hours > i_rRight.Hours;

    if ( i_rLeft.Minutes != i_rRight.Minutes )
        return i_rLeft.Minutes > i_rRight.Minutes;

    if ( i_rLeft.Seconds != i_rRight.Seconds )
        return i_rLeft.Seconds > i_rRight.Seconds;

    if ( i_rLeft.NanoSeconds != i_rRight.NanoSeconds )
        return i_rLeft.NanoSeconds > i_rRight.NanoSeconds;

    return false;
}

std::shared_ptr<GDIMetaFile>
SfxObjectShell::GetPreviewMetaFile( bool bFullContent ) const
{
    return CreatePreviewMetaFile_Impl( bFullContent );
}

std::shared_ptr<GDIMetaFile>
SfxObjectShell::CreatePreviewMetaFile_Impl( bool bFullContent ) const
{
    // DoDraw can only be called when no printing is done, otherwise
    // the printer may be turned off
    SfxViewFrame *pFrame = SfxViewFrame::GetFirst( this );
    if ( pFrame && pFrame->GetViewShell() &&
         pFrame->GetViewShell()->GetPrinter() &&
         pFrame->GetViewShell()->GetPrinter()->IsPrinting() )
         return std::shared_ptr<GDIMetaFile>();

    std::shared_ptr<GDIMetaFile> xFile(new GDIMetaFile);

    ScopedVclPtrInstance< VirtualDevice > pDevice;
    pDevice->EnableOutput( false );

    MapMode aMode( this->GetMapUnit() );
    pDevice->SetMapMode( aMode );
    xFile->SetPrefMapMode( aMode );

    Size aTmpSize;
    sal_Int8 nAspect;
    if ( bFullContent )
    {
        nAspect = ASPECT_CONTENT;
        aTmpSize = GetVisArea( nAspect ).GetSize();
    }
    else
    {
        nAspect = ASPECT_THUMBNAIL;
        aTmpSize = const_cast<SfxObjectShell*>(this)->GetFirstPageSize();
    }

    xFile->SetPrefSize( aTmpSize );
    DBG_ASSERT( aTmpSize.Height() != 0 && aTmpSize.Width() != 0,
        "size of first page is 0, override GetFirstPageSize or set vis-area!" );

    xFile->Record( pDevice );

    LanguageType eLang;
    SvtCTLOptions aCTLOptions;
    if ( SvtCTLOptions::NUMERALS_HINDI == aCTLOptions.GetCTLTextNumerals() )
        eLang = LANGUAGE_ARABIC_SAUDI_ARABIA;
    else if ( SvtCTLOptions::NUMERALS_ARABIC == aCTLOptions.GetCTLTextNumerals() )
        eLang = LANGUAGE_ENGLISH;
    else
        eLang = (LanguageType) Application::GetSettings().GetLanguageTag().getLanguageType();

    pDevice->SetDigitLanguage( eLang );

    const_cast<SfxObjectShell*>(this)->DoDraw( pDevice, Point(0,0), aTmpSize, JobSetup(), nAspect );

    xFile->Stop();

    return xFile;
}


void SfxObjectShell::UpdateDocInfoForSave()
{
    uno::Reference<document::XDocumentProperties> xDocProps(getDocProperties());

    // clear user data if recommend (see 'Tools - Options - Open/StarOffice - Security')
    if ( SvtSecurityOptions().IsOptionSet(
            SvtSecurityOptions::EOption::DocWarnRemovePersonalInfo ) )
    {
        xDocProps->resetUserData( OUString() );
    }
    else if ( IsModified() )
    {
        OUString aUserName = SvtUserOptions().GetFullName();
        if ( !IsUseUserData() )
        {
            // remove all data pointing to the current user
            if (xDocProps->getAuthor().equals(aUserName)) {
                xDocProps->setAuthor( OUString() );
            }
            xDocProps->setModifiedBy( OUString() );
            if (xDocProps->getPrintedBy().equals(aUserName)) {
                xDocProps->setPrintedBy( OUString() );
            }
        }
        else
        {
            // update ModificationAuthor, revision and editing time
            ::DateTime now( ::DateTime::SYSTEM );
            xDocProps->setModificationDate( now.GetUNODateTime() );
            xDocProps->setModifiedBy( aUserName );
            UpdateTime_Impl( xDocProps );
        }
    }
}


static void
lcl_add(util::Duration & rDur, tools::Time const& rTime)
{
    // here we don't care about overflow: rDur is converted back to seconds
    // anyway, and tools::Time cannot store more than ~4000 hours
    rDur.Hours   += rTime.GetHour();
    rDur.Minutes += rTime.GetMin();
    rDur.Seconds += rTime.GetSec();
}

// Update the processing time
void SfxObjectShell::UpdateTime_Impl(
    const uno::Reference<document::XDocumentProperties> & i_xDocProps)
{
    // Get old time from documentinfo
    const sal_Int32 secs = i_xDocProps->getEditingDuration();
    util::Duration editDuration(false, 0, 0, 0,
            secs/3600, (secs%3600)/60, secs%60, 0);

    // Initialize some local member! Its necessary for follow operations!
    DateTime    aNow( DateTime::SYSTEM );   // Date and time at current moment
    tools::Time        n24Time     (24,0,0,0)  ;   // Time-value for 24 hours - see follow calculation
    sal_uIntPtr     nDays       = 0         ;   // Count of days between now and last editing
    tools::Time        nAddTime    (0)         ;   // Value to add on aOldTime

    // Safe impossible cases!
    // User has changed time to the past between last editing and now ... it's not possible!!!
    DBG_ASSERT( !(aNow.GetDate()<pImpl->nTime.GetDate()), "Timestamp of last change is in the past ?!..." );

    // Do the follow only, if user has NOT changed time to the past.
    // Else add a time of 0 to aOldTime ... !!!
    if (aNow.GetDate()>=pImpl->nTime.GetDate())
    {
        // Get count of days last editing.
        nDays = aNow.GetSecFromDateTime(Date(pImpl->nTime.GetDate()))/86400 ;

        if (nDays==0)
        {
            // If no day between now and last editing - calculate time directly.
            nAddTime    =   (const tools::Time&)aNow - (const tools::Time&)pImpl->nTime ;
        }
        else if (nDays<=31)
        {
            // If time of working without save greater then 1 month (!) ....
            // we add 0 to aOldTime!

            // If 1 or up to 31 days between now and last editing - calculate time indirectly.
            // nAddTime = (24h - nTime) + (nDays * 24h) + aNow
            --nDays;
             nAddTime    =  nDays*n24Time.GetTime() ;
            nAddTime    +=  n24Time-(const tools::Time&)pImpl->nTime        ;
            nAddTime    +=  aNow                    ;
        }

        lcl_add(editDuration, nAddTime);
    }

    pImpl->nTime = aNow;
    try {
        const sal_Int32 newSecs( (editDuration.Hours*3600)
            + (editDuration.Minutes*60) + editDuration.Seconds);
        i_xDocProps->setEditingDuration(newSecs);
        i_xDocProps->setEditingCycles(i_xDocProps->getEditingCycles() + 1);
    }
    catch (const lang::IllegalArgumentException &)
    {
        // ignore overflow
    }
}


VclPtr<SfxDocumentInfoDialog> SfxObjectShell::CreateDocumentInfoDialog
(
    const SfxItemSet&   rSet
)
{
    return VclPtr<SfxDocumentInfoDialog>::Create(nullptr, rSet);
}


std::set<Color> SfxObjectShell::GetDocColors()
{
    std::set<Color> empty;
    return empty;
}

SfxStyleSheetBasePool* SfxObjectShell::GetStyleSheetPool()
{
    return nullptr;
}

struct Styles_Impl
{
    SfxStyleSheetBase *pSource;
    SfxStyleSheetBase *pDest;
};

void SfxObjectShell::LoadStyles
(
    SfxObjectShell &rSource         /*  the document template from which
                                        the styles are to be loaded */
)

/*  [Description]

    This method is called by the SFx if styles are to be loaded from a template.
    Existing styles are in this case overwritten. The document must then be
    re-formatted. Therefore, applications usually override this method
    and call the implementation in the base class.
*/

{
    SfxStyleSheetBasePool *pSourcePool = rSource.GetStyleSheetPool();
    DBG_ASSERT(pSourcePool, "Source-DocumentShell ohne StyleSheetPool");
    SfxStyleSheetBasePool *pMyPool = GetStyleSheetPool();
    DBG_ASSERT(pMyPool, "Dest-DocumentShell ohne StyleSheetPool");
    pSourcePool->SetSearchMask(SfxStyleFamily::All);
    std::unique_ptr<Styles_Impl[]> pFound(new Styles_Impl[pSourcePool->Count()]);
    sal_uInt16 nFound = 0;

    SfxStyleSheetBase *pSource = pSourcePool->First();
    while ( pSource )
    {
        SfxStyleSheetBase *pDest =
            pMyPool->Find( pSource->GetName(), pSource->GetFamily() );
        if ( !pDest )
        {
            pDest = &pMyPool->Make( pSource->GetName(),
                    pSource->GetFamily(), pSource->GetMask());
            // Setting of Parents, the next style
        }
        pFound[nFound].pSource = pSource;
        pFound[nFound].pDest = pDest;
        ++nFound;
        pSource = pSourcePool->Next();
    }

    for ( sal_uInt16 i = 0; i < nFound; ++i )
    {
        pFound[i].pDest->GetItemSet().PutExtended(pFound[i].pSource->GetItemSet(), SfxItemState::DONTCARE, SfxItemState::DEFAULT);
        if(pFound[i].pSource->HasParentSupport())
            pFound[i].pDest->SetParent(pFound[i].pSource->GetParent());
        if(pFound[i].pSource->HasFollowSupport())
            pFound[i].pDest->SetFollow(pFound[i].pSource->GetParent());
    }
}

sfx2::StyleManager* SfxObjectShell::GetStyleManager()
{
    return nullptr;
}

void SfxObjectShell::UpdateFromTemplate_Impl(  )

/*  [Description]

    This internal method checks whether the document was created from a
    template, and if this is newer than the document. If this is the case,
    the user is asked if the Templates (StyleSheets) should be updated.
    If this is answered positively, the StyleSheets are updated.
*/

{
    // Storage-medium?
    SfxMedium *pFile = GetMedium();
    DBG_ASSERT( pFile, "cannot UpdateFromTemplate without medium" );
    if ( !pFile )
        return;

    if ( !comphelper::isFileUrl( pFile->GetName() ) )
        // update only for documents loaded from the local file system
        return;

    // only for own storage formats
    uno::Reference< embed::XStorage > xDocStor = pFile->GetStorage();
    if ( !pFile->GetFilter() || !pFile->GetFilter()->IsOwnFormat() )
        return;

    const SfxUInt16Item* pUpdateDocItem = SfxItemSet::GetItem<SfxUInt16Item>(pFile->GetItemSet(), SID_UPDATEDOCMODE, false);
    sal_Int16 bCanUpdateFromTemplate = pUpdateDocItem ? pUpdateDocItem->GetValue() : document::UpdateDocMode::NO_UPDATE;

    // created from template?
    uno::Reference<document::XDocumentProperties> xDocProps(getDocProperties());
    OUString aTemplName( xDocProps->getTemplateName() );
    OUString aTemplURL( xDocProps->getTemplateURL() );
    OUString aFoundName;

    if ( !aTemplName.isEmpty() || (!aTemplURL.isEmpty() && !IsReadOnly()) )
    {
        // try to locate template, first using filename this must be done
        // because writer global document uses this "great" idea to manage
        // the templates of all parts in the master document but it is NOT
        // an error if the template filename points not to a valid file
        SfxDocumentTemplates aTempl;
        if (!aTemplURL.isEmpty())
        {
            try {
                aFoundName = ::rtl::Uri::convertRelToAbs(GetMedium()->GetName(),
                            aTemplURL);
            } catch (::rtl::MalformedUriException const&) {
                assert(false); // don't think that's supposed to happen?
            }
        }

        if( aFoundName.isEmpty() && !aTemplName.isEmpty() )
            // if the template filename did not lead to success,
            // try to get a file name for the logical template name
            aTempl.GetFull( OUString(), aTemplName, aFoundName );
    }

    if ( !aFoundName.isEmpty() )
    {
        // check existence of template storage
        aTemplURL = aFoundName;

        // should the document checked against changes in the template ?
        if ( IsQueryLoadTemplate() )
        {
            bool bLoad = false;

            // load document properties of template
            bool bOK = false;
            util::DateTime aTemplDate;
            try
            {
                Reference<document::XDocumentProperties> const
                    xTemplateDocProps( document::DocumentProperties::create(
                            ::comphelper::getProcessComponentContext()));
                xTemplateDocProps->loadFromMedium(aTemplURL,
                        Sequence<beans::PropertyValue>());
                aTemplDate = xTemplateDocProps->getModificationDate();
                bOK = true;
            }
            catch (const Exception& e)
            {
                SAL_INFO("sfx.doc", "caught exception" << e.Message);
            }

            // if modify date was read successfully
            if ( bOK )
            {
                // compare modify data of template with the last check date of the document
                const util::DateTime aInfoDate( xDocProps->getTemplateDate() );
                if ( aTemplDate > aInfoDate )
                {
                    // ask user
                    if( bCanUpdateFromTemplate == document::UpdateDocMode::QUIET_UPDATE
                    || bCanUpdateFromTemplate == document::UpdateDocMode::FULL_UPDATE )
                        bLoad = true;
                    else if ( bCanUpdateFromTemplate == document::UpdateDocMode::ACCORDING_TO_CONFIG )
                    {
                        OUString sMessage( SfxResId(STR_QRYTEMPL_MESSAGE).toString() );
                        sMessage = sMessage.replaceAll( "$(ARG1)", aTemplName );
                        ScopedVclPtrInstance< sfx2::QueryTemplateBox > aBox(GetDialogParent(), sMessage);
                        if ( RET_YES == aBox->Execute() )
                            bLoad = true;
                    }

                    if( !bLoad )
                    {
                        // user refuses, so don't ask again for this document
                        SetQueryLoadTemplate(false);
                        SetModified();
                    }
                }
            }

            if ( bLoad )
            {
                // styles should be updated, create document in organizer mode to read in the styles
                //TODO: testen!
                SfxObjectShellLock xTemplDoc = CreateObjectByFactoryName( GetFactory().GetFactoryName(), SfxObjectCreateMode::ORGANIZER );
                xTemplDoc->DoInitNew();

                // TODO/MBA: do we need a BaseURL? Then LoadFrom must be extended!
                //xTemplDoc->SetBaseURL( aFoundName );

                // TODO/LATER: make sure that we don't use binary templates!
                SfxMedium aMedium( aFoundName, StreamMode::STD_READ );
                if ( xTemplDoc->LoadFrom( aMedium ) )
                {
                    // transfer styles from xTemplDoc to this document
                    // TODO/MBA: make sure that no BaseURL is needed in *this* document
                    LoadStyles(*xTemplDoc);

                    // remember date/time of check
                    xDocProps->setTemplateDate(aTemplDate);
                    // TODO/LATER: new functionality to store document info is required ( didn't work for SO7 XML format )
                }
            }
        }
    }
}

bool SfxObjectShell::IsHelpDocument() const
{
    std::shared_ptr<const SfxFilter> pFilter = GetMedium()->GetFilter();
    return (pFilter && pFilter->GetFilterName() == "writer_web_HTML_help");
}

void SfxObjectShell::ResetFromTemplate( const OUString& rTemplateName, const OUString& rFileName )
{
    // only care about resetting this data for openoffice formats otherwise
    if ( IsOwnStorageFormat( *GetMedium())  )
    {
        uno::Reference<document::XDocumentProperties> xDocProps(getDocProperties());
        xDocProps->setTemplateURL( OUString() );
        xDocProps->setTemplateName( OUString() );
        xDocProps->setTemplateDate( util::DateTime() );
        xDocProps->resetUserData( OUString() );

        // TODO/REFACTOR:
        // Title?

        if( comphelper::isFileUrl( rFileName ) )
        {
            OUString aFoundName;
            if( SfxGetpApp()->Get_Impl()->GetDocumentTemplates()->GetFull( OUString(), rTemplateName, aFoundName ) )
            {
                INetURLObject aObj( rFileName );
                xDocProps->setTemplateURL( aObj.GetMainURL(INetURLObject::DecodeMechanism::ToIUri) );
                xDocProps->setTemplateName( rTemplateName );

                ::DateTime now( ::DateTime::SYSTEM );
                xDocProps->setTemplateDate( now.GetUNODateTime() );

                SetQueryLoadTemplate( true );
            }
        }
    }
}

bool SfxObjectShell::IsQueryLoadTemplate() const
{
    return pImpl->bQueryLoadTemplate;
}

bool SfxObjectShell::IsUseUserData() const
{
    return pImpl->bUseUserData;
}

bool SfxObjectShell::IsUseThumbnailSave() const
{
    return pImpl->bUseThumbnailSave;
}

void SfxObjectShell::SetQueryLoadTemplate( bool bNew )
{
    if ( pImpl->bQueryLoadTemplate != bNew )
        SetModified();
    pImpl->bQueryLoadTemplate = bNew;
}

void SfxObjectShell::SetUseUserData( bool bNew )
{
    if ( pImpl->bUseUserData != bNew )
        SetModified();
    pImpl->bUseUserData = bNew;
}

void SfxObjectShell::SetUseThumbnailSave( bool _bNew )
{
    if ( pImpl->bUseThumbnailSave != _bNew )
        SetModified();
    pImpl->bUseThumbnailSave = _bNew;
}

bool SfxObjectShell::IsLoadReadonly() const
{
    return pImpl->bLoadReadonly;
}

bool SfxObjectShell::IsSaveVersionOnClose() const
{
    return pImpl->bSaveVersionOnClose;
}

void SfxObjectShell::SetLoadReadonly( bool bNew )
{
    if ( pImpl->bLoadReadonly != bNew )
        SetModified();
    pImpl->bLoadReadonly = bNew;
}

void SfxObjectShell::SetSaveVersionOnClose( bool bNew )
{
    if ( pImpl->bSaveVersionOnClose != bNew )
        SetModified();
    pImpl->bSaveVersionOnClose = bNew;
}

sal_uInt32 SfxObjectShell::GetModifyPasswordHash() const
{
    return pImpl->m_nModifyPasswordHash;
}

bool SfxObjectShell::SetModifyPasswordHash( sal_uInt32 nHash )
{
    if ( ( !IsReadOnly() && !IsReadOnlyUI() )
      || !(pImpl->nFlagsInProgress & SfxLoadedFlags::MAINDOCUMENT ) )
    {
        // the hash can be changed only in editable documents,
        // or during loading of document
        pImpl->m_nModifyPasswordHash = nHash;
        return true;
    }

    return false;
}

const uno::Sequence< beans::PropertyValue >& SfxObjectShell::GetModifyPasswordInfo() const
{
    return pImpl->m_aModifyPasswordInfo;
}

bool SfxObjectShell::SetModifyPasswordInfo( const uno::Sequence< beans::PropertyValue >& aInfo )
{
    if ( ( !IsReadOnly() && !IsReadOnlyUI() )
      || !(pImpl->nFlagsInProgress & SfxLoadedFlags::MAINDOCUMENT ) )
    {
        // the hash can be changed only in editable documents,
        // or during loading of document
        pImpl->m_aModifyPasswordInfo = aInfo;
        return true;
    }

    return false;
}

void SfxObjectShell::SetModifyPasswordEntered( bool bEntered )
{
    pImpl->m_bModifyPasswordEntered = bEntered;
}

bool SfxObjectShell::IsModifyPasswordEntered()
{
    return pImpl->m_bModifyPasswordEntered;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
