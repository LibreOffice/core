/*************************************************************************
 *
 *  $RCSfile: opengrf.cxx,v $
 *
 *  $Revision: 1.15 $
 *
 *  last change: $Author: hr $ $Date: 2003-03-27 15:01:02 $
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

#include <tools/urlobj.hxx>

#ifndef  _CPPUHELPER_IMPLBASE1_HXX_
#include <cppuhelper/implbase1.hxx>
#endif

#ifndef _COM_SUN_STAR_UNO_REFERENCE_H_
#include <com/sun/star/uno/Reference.h>
#endif

#ifndef  _COM_SUN_STAR_LANG_XINITIALIZATION_HPP_
#include <com/sun/star/lang/XInitialization.hpp>
#endif

#ifndef  _COM_SUN_STAR_UI_DIALOGS_COMMONFILEPICKERELEMENTIDS_HPP_
#include <com/sun/star/ui/dialogs/CommonFilePickerElementIds.hpp>
#endif
#ifndef  _COM_SUN_STAR_UI_DIALOGS_EXECUTABLEDIALOGRESULTS_HPP_
#include <com/sun/star/ui/dialogs/ExecutableDialogResults.hpp>
#endif
#ifndef  _COM_SUN_STAR_UI_DIALOGS_EXTENDEDFILEPICKERELEMENTIDS_HPP_
#include <com/sun/star/ui/dialogs/ExtendedFilePickerElementIds.hpp>
#endif
#ifndef  _COM_SUN_STAR_UI_DIALOGS_FILEPREVIEWIMAGEFORMATS_HPP_
#include <com/sun/star/ui/dialogs/FilePreviewImageFormats.hpp>
#endif
#ifndef  _COM_SUN_STAR_UI_DIALOGS_LISTBOXCONTROLACTIONS_HPP_
#include <com/sun/star/ui/dialogs/ListboxControlActions.hpp>
#endif
#ifndef  _COM_SUN_STAR_UI_DIALOGS_TEMPLATEDESCRIPTION_HPP_
#include <com/sun/star/ui/dialogs/TemplateDescription.hpp>
#endif
#ifndef  _COM_SUN_STAR_UI_DIALOGS_XFILEPICKERCONTROLACCESS_HPP_
#include <com/sun/star/ui/dialogs/XFilePickerControlAccess.hpp>
#endif
#ifndef  _COM_SUN_STAR_UI_DIALOGS_XFILEPICKER_HPP_
#include <com/sun/star/ui/dialogs/XFilePicker.hpp>
#endif
#ifndef  _COM_SUN_STAR_UI_DIALOGS_XFILEPICKERLISTENER_HPP_
#include <com/sun/star/ui/dialogs/XFilePickerListener.hpp>
#endif
#ifndef  _COM_SUN_STAR_UI_DIALOGS_XFILEPICKERNOTIFIER_HPP_
#include <com/sun/star/ui/dialogs/XFilePickerNotifier.hpp>
#endif
#ifndef  _COM_SUN_STAR_UI_DIALOGS_XFILEPREVIEW_HPP_
#include <com/sun/star/ui/dialogs/XFilePreview.hpp>
#endif
#ifndef  _COM_SUN_STAR_UI_DIALOGS_XFILTERMANAGER_HPP_
#include <com/sun/star/ui/dialogs/XFilterManager.hpp>
#endif
#ifndef SVTOOLS_URIHELPER_HXX
#include <svtools/urihelper.hxx>
#endif
#ifndef _UNOTOOLS_UCBSTREAMHELPER_HXX
#include <unotools/ucbstreamhelper.hxx>
#endif
#ifndef _TRANSFER_HXX //autogen
#include <svtools/transfer.hxx>
#endif
#ifndef _SVDOGRAF_HXX //autogen
#include "svdograf.hxx"
#endif
#ifndef _SVSTOR_HXX //autogen
#include <so3/svstor.hxx>
#endif
#ifndef _SOT_FORMATS_HXX //autogen
#include <sot/formats.hxx>
#endif
#ifndef _MSGBOX_HXX //autogen
#include <vcl/msgbox.hxx>
#endif
#ifndef _FILEDLGHELPER_HXX
#include <sfx2/filedlghelper.hxx>
#endif
#ifndef _SFXDOCFILE_HXX
#include <sfx2/docfile.hxx>
#endif
#ifndef INCLUDED_SVTOOLS_PATHOPTIONS_HXX
#include <svtools/pathoptions.hxx>
#endif
#ifndef _SVX_DIALMGR_HXX
#include "dialmgr.hxx"
#endif

#ifndef _SVX_OPENGRF_HXX
#include "opengrf.hxx"
#endif

#include "dialogs.hrc"
#include "impgrf.hrc"


//-----------------------------------------------------------------------------

using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::ui::dialogs;
using namespace ::com::sun::star::uno;
using namespace ::rtl;
using namespace ::cppu;


//-----------------------------------------------------------------------------

USHORT  SvxOpenGrfErr2ResId(    short   err     )
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
    Reference < XFilePickerControlAccess >  xCtrlAcc;
};


SvxOpenGrf_Impl::SvxOpenGrf_Impl() :
    aFileDlg(SFXWB_GRAPHIC)
{
    Reference < XFilePicker > xFP = aFileDlg.GetFilePicker();
    xCtrlAcc = Reference < XFilePickerControlAccess >(xFP, UNO_QUERY);
}


SvxOpenGraphicDialog::SvxOpenGraphicDialog( const String& rTitle ) :
    mpImpl( new SvxOpenGrf_Impl )
{
    mpImpl->aFileDlg.SetTitle(rTitle);
}


SvxOpenGraphicDialog::~SvxOpenGraphicDialog()
{
}


GraphicFilter* GetGrfFilter();

short SvxOpenGraphicDialog::Execute()
{
    USHORT  nImpRet;
    BOOL    bQuitLoop(FALSE);

    while( bQuitLoop == FALSE &&
           mpImpl->aFileDlg.Execute() == ERRCODE_NONE )
    {
        if( GetPath().Len() )
        {
            GraphicFilter*  pFilter = GetGrfFilter();
            INetURLObject aObj( GetPath() );

            // check whether we can load the graphic
            String  aCurFilter( GetCurrentFilter() );
            USHORT  nFormatNum = pFilter->GetImportFormatNumber( aCurFilter );
            USHORT  nRetFormat = 0;
            USHORT  nFound = USHRT_MAX;

            // non-local?
            if ( INET_PROT_FILE != aObj.GetProtocol() )
            {
                SfxMedium aMed( aObj.GetMainURL( INetURLObject::NO_DECODE ), STREAM_READ, TRUE );
                aMed.SetTransferPriority( SFX_TFPRIO_SYNCHRON );
                aMed.DownLoad();
                SvStream* pStream = aMed.GetInStream();

                if( pStream )
                    nImpRet = pFilter->CanImportGraphic( aObj.GetMainURL( INetURLObject::NO_DECODE ), *pStream, nFormatNum, &nRetFormat );
                else
                    nImpRet = pFilter->CanImportGraphic( aObj, nFormatNum, &nRetFormat );

                if ( GRFILTER_OK != nImpRet )
                {
                    if ( !pStream )
                        nImpRet = pFilter->CanImportGraphic( aObj, GRFILTER_FORMAT_DONTKNOW, &nRetFormat );
                    else
                        nImpRet = pFilter->CanImportGraphic( aObj.GetMainURL( INetURLObject::NO_DECODE ), *pStream,
                                                             GRFILTER_FORMAT_DONTKNOW, &nRetFormat );
                }
            }
            else
            {
                if( (nImpRet=pFilter->CanImportGraphic( aObj, nFormatNum, &nRetFormat )) != GRFILTER_OK )
                    nImpRet = pFilter->CanImportGraphic( aObj, GRFILTER_FORMAT_DONTKNOW, &nRetFormat );
            }

            if ( GRFILTER_OK == nImpRet )
                nFound = nRetFormat;

            // could not load?
            if ( nFound == USHRT_MAX )
            {
                WarningBox aWarningBox( NULL, WB_3DLOOK | WB_RETRY_CANCEL, SVX_RESSTR( SvxOpenGrfErr2ResId(nImpRet) ) );
                bQuitLoop = aWarningBox.Execute()==RET_RETRY ? FALSE : TRUE;
            }
            else
            {
                // setup appropriate filter (so next time, it will work)
                if( pFilter->GetImportFormatCount() )
                {
                    String  aFormatName(pFilter->GetImportFormatName(nFound));
                    SetCurrentFilter(aFormatName);
                }

                return nImpRet;
            }
        }
    }

    // cancel
    return -1;
}


void SvxOpenGraphicDialog::SetPath( const String& rPath )
{
    mpImpl->aFileDlg.SetDisplayDirectory(rPath);
}

void SvxOpenGraphicDialog::SetPath( const String& rPath, sal_Bool bLinkState )
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
        catch(IllegalArgumentException)
        {
#ifdef DBG_UTIL
            DBG_ERROR( "Cannot enable \"link\" checkbox" );
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
        catch(IllegalArgumentException)
        {
#ifdef DBG_UTIL
            DBG_ERROR( "Cannot check \"link\" checkbox" );
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
            DBG_ASSERT(aVal.hasValue(), "Value CBX_INSERT_AS_LINK not found")
            return aVal.hasValue() ? *(sal_Bool*) aVal.getValue() : sal_False;
        }
    }
    catch(IllegalArgumentException)
    {
#ifdef DBG_UTIL
        DBG_ERROR( "Cannot access \"link\" checkbox" );
#endif
    }

    return sal_False;
}


int SvxOpenGraphicDialog::GetGraphic(Graphic& rGraphic) const
{
    return mpImpl->aFileDlg.GetGraphic(rGraphic);
}


String SvxOpenGraphicDialog::GetPath() const
{
    return mpImpl->aFileDlg.GetPath();
}


String SvxOpenGraphicDialog::GetCurrentFilter() const
{
    return mpImpl->aFileDlg.GetCurrentFilter();
}


void SvxOpenGraphicDialog::SetCurrentFilter(const String&   rStr)
{
    mpImpl->aFileDlg.SetCurrentFilter(rStr);
}

void SvxOpenGraphicDialog::SetControlHelpIds( const INT16* _pControlId, const INT32* _pHelpId )
{
    mpImpl->aFileDlg.SetControlHelpIds( _pControlId, _pHelpId );
}

void SvxOpenGraphicDialog::SetDialogHelpId( const INT32 _nHelpId )
{
    mpImpl->aFileDlg.SetDialogHelpId( _nHelpId );
}
