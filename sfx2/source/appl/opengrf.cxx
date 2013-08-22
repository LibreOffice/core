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


#include <tools/urlobj.hxx>
#include <cppuhelper/implbase1.hxx>
#include <com/sun/star/uno/Reference.h>
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
#include <svl/urihelper.hxx>
#include <unotools/ucbstreamhelper.hxx>
#include <svtools/transfer.hxx>
#include <sot/formats.hxx>
#include <vcl/msgbox.hxx>
#include <sfx2/filedlghelper.hxx>
#include <sfx2/docfile.hxx>
#include <unotools/pathoptions.hxx>
#include <sfx2/opengrf.hxx>
#include "app.hrc"
#include "sfx2/sfxresid.hxx"

//-----------------------------------------------------------------------------

using namespace ::com::sun::star;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::ui::dialogs;
using namespace ::com::sun::star::uno;
using namespace ::rtl;
using namespace ::cppu;


//-----------------------------------------------------------------------------

sal_uInt16  SvxOpenGrfErr2ResId(    short   err     )
{
    switch( err )
    {
        case GRFILTER_OPENERROR:
            return RID_SVXSTR_GRFILTER_OPENERROR;
        case GRFILTER_IOERROR:
            return RID_SVXSTR_GRFILTER_IOERROR;
        case GRFILTER_VERSIONERROR:
            return RID_SVXSTR_GRFILTER_VERSIONERROR;
        case GRFILTER_FILTERERROR:
            return RID_SVXSTR_GRFILTER_FILTERERROR;
        case GRFILTER_FORMATERROR:
        default:
            return RID_SVXSTR_GRFILTER_FORMATERROR;
    }
}


struct SvxOpenGrf_Impl
{
    SvxOpenGrf_Impl         ();

    sfx2::FileDialogHelper                  aFileDlg;
    uno::Reference < XFilePickerControlAccess > xCtrlAcc;
};


SvxOpenGrf_Impl::SvxOpenGrf_Impl()
    : aFileDlg(ui::dialogs::TemplateDescription::FILEOPEN_LINK_PREVIEW,
            SFXWB_GRAPHIC)
{
    uno::Reference < XFilePicker > xFP = aFileDlg.GetFilePicker();
    xCtrlAcc = uno::Reference < XFilePickerControlAccess >(xFP, UNO_QUERY);
}


SvxOpenGraphicDialog::SvxOpenGraphicDialog( const OUString& rTitle ) :
    mpImpl( new SvxOpenGrf_Impl )
{
    mpImpl->aFileDlg.SetTitle(rTitle);
}


SvxOpenGraphicDialog::~SvxOpenGraphicDialog()
{
}


short SvxOpenGraphicDialog::Execute()
{
    sal_uInt16  nImpRet;
    sal_Bool    bQuitLoop(sal_False);

    while( bQuitLoop == sal_False &&
           mpImpl->aFileDlg.Execute() == ERRCODE_NONE )
    {
        if( !GetPath().isEmpty() )
        {
            GraphicFilter& rFilter = GraphicFilter::GetGraphicFilter();
            INetURLObject aObj( GetPath() );

            // check whether we can load the graphic
            String  aCurFilter( GetCurrentFilter() );
            sal_uInt16  nFormatNum = rFilter.GetImportFormatNumber( aCurFilter );
            sal_uInt16  nRetFormat = 0;
            sal_uInt16  nFound = USHRT_MAX;

            // non-local?
            if ( INET_PROT_FILE != aObj.GetProtocol() )
            {
                SfxMedium aMed( aObj.GetMainURL( INetURLObject::NO_DECODE ), STREAM_READ );
                aMed.DownLoad();
                SvStream* pStream = aMed.GetInStream();

                if( pStream )
                    nImpRet = rFilter.CanImportGraphic( aObj.GetMainURL( INetURLObject::NO_DECODE ), *pStream, nFormatNum, &nRetFormat );
                else
                    nImpRet = rFilter.CanImportGraphic( aObj, nFormatNum, &nRetFormat );

                if ( GRFILTER_OK != nImpRet )
                {
                    if ( !pStream )
                        nImpRet = rFilter.CanImportGraphic( aObj, GRFILTER_FORMAT_DONTKNOW, &nRetFormat );
                    else
                        nImpRet = rFilter.CanImportGraphic( aObj.GetMainURL( INetURLObject::NO_DECODE ), *pStream,
                                                             GRFILTER_FORMAT_DONTKNOW, &nRetFormat );
                }
            }
            else
            {
                if( (nImpRet=rFilter.CanImportGraphic( aObj, nFormatNum, &nRetFormat )) != GRFILTER_OK )
                    nImpRet = rFilter.CanImportGraphic( aObj, GRFILTER_FORMAT_DONTKNOW, &nRetFormat );
            }

            if ( GRFILTER_OK == nImpRet )
                nFound = nRetFormat;

            // could not load?
            if ( nFound == USHRT_MAX )
            {
                WarningBox aWarningBox( NULL, WB_3DLOOK | WB_RETRY_CANCEL, SfxResId( SvxOpenGrfErr2ResId(nImpRet) ).toString() );
                bQuitLoop = aWarningBox.Execute()==RET_RETRY ? sal_False : sal_True;
            }
            else
            {
                // setup appropriate filter (so next time, it will work)
                if( rFilter.GetImportFormatCount() )
                {
                    String  aFormatName(rFilter.GetImportFormatName(nFound));
                    SetCurrentFilter(aFormatName);
                }

                return nImpRet;
            }
        }
    }

    // cancel
    return -1;
}


void SvxOpenGraphicDialog::SetPath( const OUString& rPath )
{
    mpImpl->aFileDlg.SetDisplayDirectory(rPath);
}

void SvxOpenGraphicDialog::SetPath( const OUString& rPath, sal_Bool bLinkState )
{
    SetPath(rPath);
    AsLink(bLinkState);
}


void SvxOpenGraphicDialog::EnableLink( sal_Bool  state  )
{
    if( mpImpl->xCtrlAcc.is() )
    {
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
}


void SvxOpenGraphicDialog::AsLink(sal_Bool  bState)
{
    if( mpImpl->xCtrlAcc.is() )
    {
        try
        {
            Any aAny; aAny <<= bState;
            mpImpl->xCtrlAcc->setValue( ExtendedFilePickerElementIds::CHECKBOX_LINK, 0, aAny );
        }
        catch(const IllegalArgumentException&)
        {
#ifdef DBG_UTIL
            OSL_FAIL( "Cannot check \"link\" checkbox" );
#endif
        }
    }
}


sal_Bool SvxOpenGraphicDialog::IsAsLink() const
{
    try
    {
        if( mpImpl->xCtrlAcc.is() )
        {
            Any aVal = mpImpl->xCtrlAcc->getValue( ExtendedFilePickerElementIds::CHECKBOX_LINK, 0 );
            DBG_ASSERT(aVal.hasValue(), "Value CBX_INSERT_AS_LINK not found");
            return aVal.hasValue() ? *(sal_Bool*) aVal.getValue() : sal_False;
        }
    }
    catch(const IllegalArgumentException&)
    {
#ifdef DBG_UTIL
        OSL_FAIL( "Cannot access \"link\" checkbox" );
#endif
    }

    return sal_False;
}


int SvxOpenGraphicDialog::GetGraphic(Graphic& rGraphic) const
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


void SvxOpenGraphicDialog::SetCurrentFilter(const OUString& rStr)
{
    mpImpl->aFileDlg.SetCurrentFilter(rStr);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
