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
#include <comphelper/fileurl.hxx>
#include <vcl/svapp.hxx>
#include <vcl/weld.hxx>
#include <svl/style.hxx>

#include <svl/intitem.hxx>
#include <svl/ctloptions.hxx>
#include <comphelper/processfactory.hxx>
#include <unotools/securityoptions.hxx>
#include <tools/datetime.hxx>
#include <comphelper/diagnose_ex.hxx>
#include <tools/helpers.hxx>
#include <rtl/uri.hxx>

#include <unotools/useroptions.hxx>
#include <vcl/virdev.hxx>
#include <vcl/settings.hxx>
#include <vcl/gdimtf.hxx>

#include <sfx2/app.hxx>
#include <sfx2/dinfdlg.hxx>
#include <sfx2/sfxresid.hxx>
#include <appdata.hxx>
#include <sfx2/docfac.hxx>
#include <sfx2/viewsh.hxx>
#include <sfx2/objsh.hxx>
#include <objshimp.hxx>
#include <sfx2/printer.hxx>
#include <sfx2/viewfrm.hxx>
#include <sfx2/doctempl.hxx>
#include <sfx2/sfxsids.hrc>
#include <sfx2/strings.hrc>
#include <sfx2/docfile.hxx>
#include <sfx2/docfilt.hxx>
#include <sfx2/IDocumentModelAccessor.hxx>
#include <memory>
#include <helpids.h>

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
SfxObjectShell::GetPreviewMetaFile( bool bFullContent, bool bOutputForScreen ) const
{
    auto xFile = std::make_shared<GDIMetaFile>();
    ScopedVclPtrInstance< VirtualDevice > pDevice;
    pDevice->EnableOutput( false );
    if(!CreatePreview_Impl(bFullContent, bOutputForScreen, pDevice, xFile.get()))
        return std::shared_ptr<GDIMetaFile>();
    return xFile;
}

Bitmap SfxObjectShell::GetPreviewBitmap() const
{
    SfxCloseVetoLock lock(this);
    ScopedVclPtrInstance< VirtualDevice > pDevice(DeviceFormat::WITH_ALPHA);
    pDevice->SetAntialiasing(AntialiasingFlags::Enable | pDevice->GetAntialiasing());
    if(!CreatePreview_Impl(/*bFullContent*/false, false, pDevice, nullptr))
        return Bitmap();
    Size size = pDevice->GetOutputSizePixel();
    Bitmap aBitmap( pDevice->GetBitmap( Point(), size) );
    // Scale down the image to the desired size from the 4*size from CreatePreview_Impl().
    size = Size( size.Width() / 4, size.Height() / 4 );
    aBitmap.Scale(size, BmpScaleFlag::BestQuality);
    if (!aBitmap.IsEmpty())
        aBitmap.Convert(BmpConversion::N24Bit);
    return aBitmap;
}

bool SfxObjectShell::CreatePreview_Impl( bool bFullContent, bool bOutputForScreen, VirtualDevice* pDevice, GDIMetaFile* pFile) const
{
    // DoDraw can only be called when no printing is done, otherwise
    // the printer may be turned off
    SfxViewFrame *pFrame = SfxViewFrame::GetFirst( this );
    if ( pFrame && pFrame->GetViewShell() &&
         pFrame->GetViewShell()->GetPrinter() &&
         pFrame->GetViewShell()->GetPrinter()->IsPrinting() )
         return false;

    MapMode aMode( GetMapUnit() );
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
        aTmpSize = GetFirstPageSize();
    }

    DBG_ASSERT( !aTmpSize.IsEmpty(),
        "size of first page is 0, override GetFirstPageSize or set visible-area!" );

    if(pFile)
    {
        pDevice->SetMapMode( aMode );
        pFile->SetPrefMapMode( aMode );
        pFile->SetPrefSize( aTmpSize );
        pFile->Record( pDevice );
    }
    else
    {
        // Use pixel size, that's also what DoDraw() requires in this case,
        // despite the metafile case (needlessly?) setting mapmode.
        Size aSizePix = pDevice->LogicToPixel( aTmpSize, aMode );
        // Code based on GDIMetaFile::CreateThumbnail().
        sal_uInt32      nMaximumExtent = 512;
        // determine size that has the same aspect ratio as image size and
        // fits into the rectangle determined by nMaximumExtent
        if ( aSizePix.Width() && aSizePix.Height()
          && ( sal::static_int_cast< tools::ULong >(aSizePix.Width()) >
                   nMaximumExtent ||
               sal::static_int_cast< tools::ULong >(aSizePix.Height()) >
                   nMaximumExtent ) )
        {
            double      fWH = static_cast< double >( aSizePix.Width() ) / aSizePix.Height();
            if ( fWH <= 1.0 )
            {
                aSizePix.setWidth(basegfx::fround<tools::Long>(nMaximumExtent * fWH));
                aSizePix.setHeight( nMaximumExtent );
            }
            else
            {
                aSizePix.setWidth( nMaximumExtent );
                aSizePix.setHeight(basegfx::fround<tools::Long>(nMaximumExtent / fWH));
            }
        }
        // do it 4x larger to be able to scale it down & get beautiful antialias
        aTmpSize = Size( aSizePix.Width() * 4, aSizePix.Height() * 4 );
        pDevice->SetOutputSizePixel( aTmpSize );
    }

    LanguageType eLang;
    if ( SvtCTLOptions::NUMERALS_HINDI == SvtCTLOptions::GetCTLTextNumerals() )
        eLang = LANGUAGE_ARABIC_SAUDI_ARABIA;
    else if ( SvtCTLOptions::NUMERALS_ARABIC == SvtCTLOptions::GetCTLTextNumerals() )
        eLang = LANGUAGE_ENGLISH;
    else
        eLang = Application::GetSettings().GetLanguageTag().getLanguageType();

    pDevice->SetDigitLanguage( eLang );

    const_cast<SfxObjectShell*>(this)->DoDraw( pDevice, Point(0,0), aTmpSize, JobSetup(), nAspect, bOutputForScreen );

    if(pFile)
        pFile->Stop();

    return true;
}


void SfxObjectShell::UpdateDocInfoForSave()
{
    uno::Reference<document::XDocumentProperties> xDocProps(getDocProperties());

    // clear user data if recommend (see 'Tools - Options - LibreOffice - Security')
    if ( SvtSecurityOptions::IsOptionSet(
            SvtSecurityOptions::EOption::DocWarnRemovePersonalInfo ) && !SvtSecurityOptions::IsOptionSet(
                SvtSecurityOptions::EOption::DocWarnKeepDocUserInfo))
    {
        xDocProps->resetUserData( OUString() );
    }
    else if ( IsModified() )
    {
        const OUString aUserName = SvtUserOptions().GetFullName();
        if ( !IsUseUserData() )
        {
            // remove all data pointing to the current user
            if (xDocProps->getAuthor() == aUserName) {
                xDocProps->setAuthor( OUString() );
            }
            xDocProps->setModifiedBy( OUString() );
            if (xDocProps->getPrintedBy() == aUserName) {
                xDocProps->setPrintedBy( OUString() );
            }
        }
        else
        {
            // update ModificationAuthor, revision and editing time
            ::DateTime now( ::DateTime::SYSTEM );
            xDocProps->setModificationDate( now.GetUNODateTime() );
            xDocProps->setModifiedBy( aUserName );
            if (!SvtSecurityOptions::IsOptionSet(SvtSecurityOptions::EOption::DocWarnRemoveEditingTimeInfo))
                UpdateTime_Impl( xDocProps );
        }
        // reset only editing time to zero if RemoveEditingTimeOnSaving is true
        if (SvtSecurityOptions::IsOptionSet(SvtSecurityOptions::EOption::DocWarnRemoveEditingTimeInfo))
            xDocProps->setEditingDuration(0);
    }
    else
    {
        // reset only editing time to zero if RemoveEditingTimeOnSaving is true
        if (SvtSecurityOptions::IsOptionSet(SvtSecurityOptions::EOption::DocWarnRemoveEditingTimeInfo))
            xDocProps->setEditingDuration(0);
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

    // Initialize some local member! It's necessary for follow operations!
    DateTime     aNow( DateTime::SYSTEM );   // Date and time at current moment

    // Save impossible cases!
    // User has changed time to the past between last editing and now... it's not possible!!!
    DBG_ASSERT( !(aNow.GetDate()<pImpl->nTime.GetDate()), "Timestamp of last change is in the past!?..." );

    // Do the follow only, if user has NOT changed time to the past.
    // Else add a time of 0 to aOldTime... !!!
    if (aNow.GetDate()>=pImpl->nTime.GetDate())
    {
        // Count of days between now and last editing
        sal_Int32 nDays = aNow.GetSecFromDateTime(Date(pImpl->nTime.GetDate()))/86400 ;
        tools::Time nAddTime(tools::Time::EMPTY); // Value to add on aOldTime

        if (nDays==0)
        {
            // If no day between now and last editing - calculate time directly.
            nAddTime    =   static_cast<const tools::Time&>(aNow) - static_cast<const tools::Time&>(pImpl->nTime);
        }
        else if (nDays<=31)
        {
            // If time of working without save greater than 1 month (!)...
            // we add 0 to aOldTime!

            // If 1 or up to 31 days between now and last editing - calculate time indirectly.
            // nAddTime = (24h - nTime) + (nDays * 24h) + aNow
            tools::Time n24Time (24,0,0,0); // Time-value for 24 hours
            --nDays;
            nAddTime.MakeTimeFromNS(nDays * n24Time.GetNSFromTime());
            nAddTime    +=  n24Time-static_cast<const tools::Time&>(pImpl->nTime);
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

std::shared_ptr<SfxDocumentInfoDialog> SfxObjectShell::CreateDocumentInfoDialog(weld::Window* pParent,
                                                                                const SfxItemSet& rSet)
{
    return std::make_shared<SfxDocumentInfoDialog>(pParent, rSet);
}

std::optional<NamedColor> SfxObjectShell::GetRecentColor(sal_uInt16 nSlotId)
{
    auto it = pImpl->m_aRecentColors.find(nSlotId);
    if (it != pImpl->m_aRecentColors.end())
        return it->second;

    return std::nullopt;
}

void SfxObjectShell::SetRecentColor(sal_uInt16 nSlotId, const NamedColor& rColor, bool bBroadcast)
{
    pImpl->m_aRecentColors[nSlotId] = rColor;
    if (bBroadcast)
        Broadcast(SfxHint(SfxHintId::ColorsChanged));
}

std::set<Color> SfxObjectShell::GetDocColors()
{
    std::set<Color> empty;
    return empty;
}

std::shared_ptr<model::ColorSet> SfxObjectShell::GetThemeColors() { return {}; }

std::shared_ptr<sfx::IDocumentModelAccessor> SfxObjectShell::GetDocumentModelAccessor() const
{
    return {};
}

sfx::AccessibilityIssueCollection SfxObjectShell::runAccessibilityCheck()
{
    sfx::AccessibilityIssueCollection aCollection;
    return aCollection;
}

SfxStyleSheetBasePool* SfxObjectShell::GetStyleSheetPool()
{
    return nullptr;
}

namespace {

struct Styles_Impl
{
    SfxStyleSheetBase *pSource;
    SfxStyleSheetBase *pDest;
};

}

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
    DBG_ASSERT(pSourcePool, "Source-DocumentShell without StyleSheetPool");
    SfxStyleSheetBasePool *pMyPool = GetStyleSheetPool();
    DBG_ASSERT(pMyPool, "Dest-DocumentShell without StyleSheetPool");
    auto xIter = pSourcePool->CreateIterator(SfxStyleFamily::All);
    std::unique_ptr<Styles_Impl[]> pFound(new Styles_Impl[xIter->Count()]);
    sal_uInt16 nFound = 0;

    SfxStyleSheetBase *pSource = xIter->First();
    while ( pSource )
    {
        SfxStyleSheetBase *pDest =
            pMyPool->Find( pSource->GetName(), pSource->GetFamily() );
        if ( !pDest )
        {
            pDest = &pMyPool->Make( pSource->GetName(),
                    pSource->GetFamily(), pSource->GetMask());
            // Setting of parents, the next style
        }
        pFound[nFound].pSource = pSource;
        pFound[nFound].pDest = pDest;
        ++nFound;
        pSource = xIter->Next();
    }

    for ( sal_uInt16 i = 0; i < nFound; ++i )
    {
        pFound[i].pDest->GetItemSet().PutExtended(pFound[i].pSource->GetItemSet(), SfxItemState::INVALID, SfxItemState::DEFAULT);
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

namespace
{
    class QueryTemplateBox
    {
    private:
        std::unique_ptr<weld::MessageDialog> m_xQueryBox;
    public:
        QueryTemplateBox(weld::Window* pParent, const OUString& rMessage)
            : m_xQueryBox(Application::CreateMessageDialog(pParent, VclMessageType::Question, VclButtonsType::NONE, rMessage))
        {
            m_xQueryBox->add_button(SfxResId(STR_QRYTEMPL_UPDATE_BTN), RET_YES);
            m_xQueryBox->add_button(SfxResId(STR_QRYTEMPL_KEEP_BTN), RET_NO);
            m_xQueryBox->set_default_response(RET_YES);
            m_xQueryBox->set_help_id(HID_QUERY_LOAD_TEMPLATE);
        }
        short run() { return m_xQueryBox->run(); }
    };
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

    // tdf#113935 - do not remove this line - somehow, it makes the process
    // of switching from viewing a read-only document to opening it in writable
    // mode much faster.
    uno::Reference< embed::XStorage > xDocStor = pFile->GetStorage(false);

    // only for own storage formats
    if ( !pFile->GetFilter() || !pFile->GetFilter()->IsOwnFormat() )
        return;

    const SfxUInt16Item* pUpdateDocItem = pFile->GetItemSet().GetItem(SID_UPDATEDOCMODE, false);
    sal_Int16 bCanUpdateFromTemplate = pUpdateDocItem ? pUpdateDocItem->GetValue() : document::UpdateDocMode::NO_UPDATE;

    // created from template?
    uno::Reference<document::XDocumentProperties> xDocProps(getDocProperties());
    const OUString aTemplName( xDocProps->getTemplateName() );
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
            aTempl.GetFull( u"", aTemplName, aFoundName );
    }

    if ( aFoundName.isEmpty() )
        return;

    // check existence of template storage
    aTemplURL = aFoundName;

    // should the document checked against changes in the template ?
    if ( !IsQueryLoadTemplate() )
        return;

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
    catch (const Exception&)
    {
        TOOLS_INFO_EXCEPTION("sfx.doc", "");
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
                const OUString sMessage( SfxResId(STR_QRYTEMPL_MESSAGE).replaceAll( "$(ARG1)", aTemplName ) );
                QueryTemplateBox aBox(Application::GetFrameWeld(GetDialogParent()), sMessage);
                if (RET_YES == aBox.run())
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

    if ( !bLoad )
        return;

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

bool SfxObjectShell::IsHelpDocument() const
{
    if (!pMedium)
    {
        return false;
    }

    std::shared_ptr<const SfxFilter> pFilter = pMedium->GetFilter();
    return (pFilter && pFilter->GetFilterName() == "writer_web_HTML_help");
}

void SfxObjectShell::ResetFromTemplate( const OUString& rTemplateName, std::u16string_view rFileName )
{
    // only care about resetting this data for LibreOffice formats otherwise
    if ( !IsOwnStorageFormat( *GetMedium())  )
        return;

    uno::Reference<document::XDocumentProperties> xDocProps(getDocProperties());
    xDocProps->setTemplateURL( OUString() );
    xDocProps->setTemplateName( OUString() );
    xDocProps->setTemplateDate( util::DateTime() );
    xDocProps->resetUserData( OUString() );

    // TODO/REFACTOR:
    // Title?

    if( !comphelper::isFileUrl( rFileName ) )
        return;

    OUString aFoundName;
    if( SfxGetpApp()->Get_Impl()->GetDocumentTemplates()->GetFull( u"", rTemplateName, aFoundName ) )
    {
        INetURLObject aObj( rFileName );
        xDocProps->setTemplateURL( aObj.GetMainURL(INetURLObject::DecodeMechanism::ToIUri) );
        xDocProps->setTemplateName( rTemplateName );

        ::DateTime now( ::DateTime::SYSTEM );
        xDocProps->setTemplateDate( now.GetUNODateTime() );

        SetQueryLoadTemplate( true );
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

bool SfxObjectShell::IsModifyPasswordEntered() const
{
    return pImpl->m_bModifyPasswordEntered;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
