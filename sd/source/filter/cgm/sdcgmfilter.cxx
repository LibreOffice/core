/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: sdcgmfilter.cxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: ihi $ $Date: 2006-11-14 14:22:32 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sd.hxx"

#include <vos/module.hxx>
#include <tools/urlobj.hxx>
#include <svtools/itemset.hxx>
#include <sfx2/docfile.hxx>
#include <sfx2/docfilt.hxx>
#include <svx/xflclit.hxx>
#include <svx/xfillit0.hxx>

#include "sdpage.hxx"
#include "drawdoc.hxx"
#include "sdcgmfilter.hxx"

// -----------
// - Defines -
// -----------

#define CGM_IMPORT_CGM      0x00000001
#define CGM_IMPORT_IM       0x00000002

#define CGM_EXPORT_IMPRESS  0x00000100
#define CGM_EXPORT_META     0x00000200
#define CGM_EXPORT_COMMENT  0x00000400

#define CGM_NO_PAD_BYTE     0x00010000
#define CGM_BIG_ENDIAN      0x00020000
#define CGM_LITTLE_ENDIAN   0x00040000

// --------------
// - Namespaces -
// --------------

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::task;
using namespace ::com::sun::star::frame;

// ------------
// - Typedefs -
// ------------

typedef UINT32 ( __LOADONCALLAPI *ImportCGM )( ::rtl::OUString&, Reference< XModel >&, UINT32, Reference< XStatusIndicator >& );
typedef BOOL ( __LOADONCALLAPI *ExportCGM )( ::rtl::OUString&, Reference< XModel >&, Reference< XStatusIndicator >&, void* );

// ---------------
// - SdPPTFilter -
// ---------------

SdCGMFilter::SdCGMFilter( SfxMedium& rMedium, ::sd::DrawDocShell& rDocShell, sal_Bool bShowProgress ) :
    SdFilter( rMedium, rDocShell, bShowProgress )
{
}

// -----------------------------------------------------------------------------

SdCGMFilter::~SdCGMFilter()
{
}

// -----------------------------------------------------------------------------

sal_Bool SdCGMFilter::Import()
{
    ::vos::OModule* pLibrary = OpenLibrary( mrMedium.GetFilter()->GetUserData() );
    sal_Bool        bRet = sal_False;

    if( pLibrary && mxModel.is() )
    {
        ImportCGM       FncImportCGM = ( ImportCGM ) pLibrary->getSymbol( ::rtl::OUString::createFromAscii( "ImportCGM" ) );
        ::rtl::OUString aFileURL( mrMedium.GetURLObject().GetMainURL( INetURLObject::NO_DECODE ) );
        UINT32          nRetValue;

        if( mrDocument.GetPageCount() == 0L )
            mrDocument.CreateFirstPages();

        CreateStatusIndicator();
        nRetValue = FncImportCGM( aFileURL, mxModel, CGM_IMPORT_CGM | CGM_BIG_ENDIAN | CGM_EXPORT_IMPRESS, mxStatusIndicator );

        if( nRetValue )
        {
            bRet = TRUE;

            if( ( nRetValue &~0xff000000 ) != 0xffffff )    // maybe the backgroundcolor is already white
            {                                               // so we must not set a master page
                mrDocument.StopWorkStartupDelay();
                SdrObject* pObj = mrDocument.GetMasterSdPage(0, PK_STANDARD)->GetPresObj(PRESOBJ_BACKGROUND);

                if( pObj )
                {
                    SfxItemSet  aSet( mrDocument.GetPool() );
                    Color       aColor( (BYTE)( nRetValue >> 16 ), (BYTE)( nRetValue >> 8 ), (BYTE)( nRetValue >> 16 ) );

                    aSet.Put( XFillColorItem( String(), aColor ) );
                    aSet.Put( XFillStyleItem( XFILL_SOLID ) );
                    pObj->SetMergedItemSetAndBroadcast( aSet );
                }
            }
        }
    }

    delete pLibrary;

    return bRet;
}

// -----------------------------------------------------------------------------

sal_Bool SdCGMFilter::Export()
{
    ::vos::OModule* pLibrary = OpenLibrary( mrMedium.GetFilter()->GetUserData() );
    sal_Bool        bRet = sal_False;

    if( pLibrary && mxModel.is() )
    {
        ExportCGM FncCGMExport = ( ExportCGM ) pLibrary->getSymbol( ::rtl::OUString::createFromAscii( "ExportCGM" ) );

        if( FncCGMExport )
        {
            ::rtl::OUString aPhysicalName( mrMedium.GetPhysicalName() );

            /* !!!
            if ( pViewShell && pViewShell->GetView() )
                pViewShell->GetView()->SdrEndTextEdit();
            */
            CreateStatusIndicator();
            bRet = FncCGMExport( aPhysicalName, mxModel, mxStatusIndicator, NULL );
        }
    }

    delete pLibrary;

    return bRet;
}
