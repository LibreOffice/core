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


#include <tools/debug.hxx>
#include <tools/urlobj.hxx>
#include <com/sun/star/uno/Reference.h>
#include <com/sun/star/lang/IllegalArgumentException.hpp>
#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/ui/dialogs/CommonFilePickerElementIds.hpp>
#include <com/sun/star/ui/dialogs/ExecutableDialogResults.hpp>
#include <com/sun/star/ui/dialogs/ExtendedFilePickerElementIds.hpp>
#include <com/sun/star/ui/dialogs/FilePreviewImageFormats.hpp>
#include <com/sun/star/ui/dialogs/ListboxControlActions.hpp>
#include <com/sun/star/ui/dialogs/TemplateDescription.hpp>
#include <com/sun/star/ui/dialogs/XFilePickerControlAccess.hpp>
#include <com/sun/star/ui/dialogs/XFilePicker.hpp>
#include <com/sun/star/ui/dialogs/XFilePickerListener.hpp>
#include <com/sun/star/ui/dialogs/XFilePickerNotifier.hpp>
#include <com/sun/star/ui/dialogs/XFilePreview.hpp>
#include <com/sun/star/ui/dialogs/XFilterManager.hpp>
#include <o3tl/any.hxx>
#include <svl/urihelper.hxx>
#include <vcl/transfer.hxx>
#include <sot/formats.hxx>
#include <vcl/button.hxx>
#include <vcl/svapp.hxx>
#include <vcl/weld.hxx>
#include <sfx2/filedlghelper.hxx>
#include <sfx2/docfile.hxx>
#include <unotools/pathoptions.hxx>
#include <sfx2/opengrf.hxx>
#include <sfx2/strings.hrc>
#include <sfx2/sfxresid.hxx>
#include <osl/diagnose.h>


using namespace ::com::sun::star;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::ui::dialogs;
using namespace ::com::sun::star::uno;
using namespace ::cppu;

static const char* SvxOpenGrfErr2ResId( ErrCode err )
{
    if (err == ERRCODE_GRFILTER_OPENERROR)
        return RID_SVXSTR_GRFILTER_OPENERROR;
    else if (err == ERRCODE_GRFILTER_IOERROR)
        return RID_SVXSTR_GRFILTER_IOERROR;
    else if (err == ERRCODE_GRFILTER_VERSIONERROR)
        return RID_SVXSTR_GRFILTER_VERSIONERROR;
    else if (err == ERRCODE_GRFILTER_FILTERERROR)
        return RID_SVXSTR_GRFILTER_FILTERERROR;
    else
        return RID_SVXSTR_GRFILTER_FORMATERROR;
}

struct SvxOpenGrf_Impl
{
    SvxOpenGrf_Impl(weld::Window* pPreferredParent,
                    sal_Int16 nDialogType);

    sfx2::FileDialogHelper                  aFileDlg;
    OUString sDetectedFilter;
    uno::Reference < XFilePickerControlAccess > xCtrlAcc;
};


SvxOpenGrf_Impl::SvxOpenGrf_Impl(weld::Window* pPreferredParent,
                                 sal_Int16 nDialogType)
    : aFileDlg(nDialogType, FileDialogFlags::Graphic, pPreferredParent)
{
    uno::Reference < XFilePicker3 > xFP = aFileDlg.GetFilePicker();
    xCtrlAcc.set(xFP, UNO_QUERY);
}


SvxOpenGraphicDialog::SvxOpenGraphicDialog(const OUString& rTitle, weld::Window* pPreferredParent)
    : mpImpl(new SvxOpenGrf_Impl(pPreferredParent, ui::dialogs::TemplateDescription::FILEOPEN_LINK_PREVIEW))
{
    mpImpl->aFileDlg.SetTitle(rTitle);
}

SvxOpenGraphicDialog::SvxOpenGraphicDialog(const OUString& rTitle, weld::Window* pPreferredParent,
                                           sal_Int16 nDialogType)
    : mpImpl(new SvxOpenGrf_Impl(pPreferredParent, nDialogType))
{
    mpImpl->aFileDlg.SetTitle(rTitle);
}

SvxOpenGraphicDialog::~SvxOpenGraphicDialog()
{
}

ErrCode SvxOpenGraphicDialog::Execute()
{
    ErrCode nImpRet;
    bool    bQuitLoop(false);

    while( !bQuitLoop &&
           mpImpl->aFileDlg.Execute() == ERRCODE_NONE )
    {
        if( !GetPath().isEmpty() )
        {
            GraphicFilter& rFilter = GraphicFilter::GetGraphicFilter();
            INetURLObject aObj( GetPath() );

            // check whether we can load the graphic
            OUString  aCurFilter( GetCurrentFilter() );
            sal_uInt16  nFormatNum = rFilter.GetImportFormatNumber( aCurFilter );
            sal_uInt16  nRetFormat = 0;
            sal_uInt16  nFound = USHRT_MAX;

            // non-local?
            if ( INetProtocol::File != aObj.GetProtocol() )
            {
                SfxMedium aMed( aObj.GetMainURL( INetURLObject::DecodeMechanism::NONE ), StreamMode::READ );
                aMed.Download();
                SvStream* pStream = aMed.GetInStream();

                if( pStream )
                    nImpRet = rFilter.CanImportGraphic( aObj.GetMainURL( INetURLObject::DecodeMechanism::NONE ), *pStream, nFormatNum, &nRetFormat );
                else
                    nImpRet = rFilter.CanImportGraphic( aObj, nFormatNum, &nRetFormat );

                if ( ERRCODE_NONE != nImpRet )
                {
                    if ( !pStream )
                        nImpRet = rFilter.CanImportGraphic( aObj, GRFILTER_FORMAT_DONTKNOW, &nRetFormat );
                    else
                        nImpRet = rFilter.CanImportGraphic( aObj.GetMainURL( INetURLObject::DecodeMechanism::NONE ), *pStream,
                                                             GRFILTER_FORMAT_DONTKNOW, &nRetFormat );
                }
            }
            else
            {
                if( (nImpRet=rFilter.CanImportGraphic( aObj, nFormatNum, &nRetFormat )) != ERRCODE_NONE )
                    nImpRet = rFilter.CanImportGraphic( aObj, GRFILTER_FORMAT_DONTKNOW, &nRetFormat );
            }

            if ( ERRCODE_NONE == nImpRet )
                nFound = nRetFormat;

            // could not load?
            if ( nFound == USHRT_MAX )
            {
                std::unique_ptr<weld::MessageDialog> xWarn(Application::CreateMessageDialog(nullptr,
                                                           VclMessageType::Warning, VclButtonsType::NONE,
                                                           SfxResId(SvxOpenGrfErr2ResId(nImpRet))));
                xWarn->add_button(Button::GetStandardText(StandardButtonType::Retry), RET_RETRY);
                xWarn->add_button(Button::GetStandardText(StandardButtonType::Cancel), RET_CANCEL);
                bQuitLoop = xWarn->run() != RET_RETRY;
            }
            else
            {
                if( rFilter.GetImportFormatCount() )
                {
                    // store detected appropriate filter
                    OUString aFormatName(rFilter.GetImportFormatName(nFound));
                    SetDetectedFilter(aFormatName);
                }
                else
                {
                    SetDetectedFilter(mpImpl->aFileDlg.GetCurrentFilter());
                }

                return nImpRet;
            }
        }
    }

    // cancel
    return ErrCode(sal_uInt32(-1));
}


void SvxOpenGraphicDialog::SetPath( const OUString& rPath, bool bLinkState )
{
    mpImpl->aFileDlg.SetDisplayDirectory(rPath);
    AsLink(bLinkState);
}


void SvxOpenGraphicDialog::EnableLink( bool state )
{
    if( !mpImpl->xCtrlAcc.is() )
        return;

    try
    {
        mpImpl->xCtrlAcc->enableControl( ExtendedFilePickerElementIds::CHECKBOX_LINK, state );
    }
    catch(const IllegalArgumentException&)
    {
#ifdef DBG_UTIL
        OSL_FAIL( "Cannot enable \"link\" checkbox" );
#endif
    }
}


void SvxOpenGraphicDialog::AsLink(bool bState)
{
    if( !mpImpl->xCtrlAcc.is() )
        return;

    try
    {
        mpImpl->xCtrlAcc->setValue( ExtendedFilePickerElementIds::CHECKBOX_LINK, 0, Any(bState) );
    }
    catch(const IllegalArgumentException&)
    {
#ifdef DBG_UTIL
        OSL_FAIL( "Cannot check \"link\" checkbox" );
#endif
    }
}


bool SvxOpenGraphicDialog::IsAsLink() const
{
    try
    {
        if( mpImpl->xCtrlAcc.is() )
        {
            Any aVal = mpImpl->xCtrlAcc->getValue( ExtendedFilePickerElementIds::CHECKBOX_LINK, 0 );
            DBG_ASSERT(aVal.hasValue(), "Value CBX_INSERT_AS_LINK not found");
            return aVal.hasValue() && *o3tl::doAccess<bool>(aVal);
        }
    }
    catch(const IllegalArgumentException&)
    {
#ifdef DBG_UTIL
        OSL_FAIL( "Cannot access \"link\" checkbox" );
#endif
    }

    return false;
}

ErrCode SvxOpenGraphicDialog::GetGraphic(Graphic& rGraphic) const
{
    return mpImpl->aFileDlg.GetGraphic(rGraphic);
}

OUString SvxOpenGraphicDialog::GetPath() const
{
    return mpImpl->aFileDlg.GetPath();
}

OUString SvxOpenGraphicDialog::GetCurrentFilter() const
{
    return mpImpl->aFileDlg.GetCurrentFilter();
}

OUString const & SvxOpenGraphicDialog::GetDetectedFilter() const
{
    return mpImpl->sDetectedFilter;
}

void SvxOpenGraphicDialog::SetCurrentFilter(const OUString& rStr)
{
    mpImpl->aFileDlg.SetCurrentFilter(rStr);
}

void SvxOpenGraphicDialog::SetDetectedFilter(const OUString& rStr)
{
    mpImpl->sDetectedFilter = rStr;
}

Reference<ui::dialogs::XFilePickerControlAccess> const & SvxOpenGraphicDialog::GetFilePickerControlAccess()
{
    return mpImpl->xCtrlAcc;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
