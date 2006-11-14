/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: sdfilter.cxx,v $
 *
 *  $Revision: 1.14 $
 *
 *  last change: $Author: ihi $ $Date: 2006-11-14 14:22:10 $
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

#ifndef _COM_SUN_STAR_TASK_XSTATUSINDICATORFACTORY_HPP_
#include <com/sun/star/task/XStatusIndicatorFactory.hpp>
#endif

#include <tools/debug.hxx>
#include <osl/file.hxx>
#include <vos/module.hxx>
#include <svtools/pathoptions.hxx>
#include <sfx2/docfile.hxx>
#include <sfx2/viewfrm.hxx>
#include <sfx2/progress.hxx>

#ifndef _SFXITEMSET_HXX
#include <svtools/itemset.hxx>
#endif

#ifndef MAC
#ifndef SVX_LIGHT
#ifndef SD_DRAW_DOC_SHELL_HXX
#include "../ui/inc/DrawDocShell.hxx"
#endif
#include "../ui/inc/strings.hrc"
#endif //!SVX_LIGHT
#else  //MAC
#ifndef SVX_LIGHT
#ifndef SD_DRAW_DOC_SHELL_HXX
#include "DrawDocShell.hxx"
#endif
#include "strings.hrc"
#endif //!SVX_LIGHT
#endif //!MAC

#include "sdresid.hxx"
#include "pres.hxx"
#include "drawdoc.hxx"
#include "sdfilter.hxx"

// --------------
// - Namespaces -
// --------------

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::task;
using namespace ::com::sun::star::frame;

// ------------
// - SdFilter -
// ------------

SdFilter::SdFilter( SfxMedium& rMedium, ::sd::DrawDocShell& rDocShell, sal_Bool bShowProgress ) :
    mrMedium( rMedium ),
    mrDocShell( rDocShell ),
    mrDocument( *rDocShell.GetDoc() ),
    mxModel( rDocShell.GetModel() ),
    mbIsDraw( rDocShell.GetDocumentType() == DOCUMENT_TYPE_DRAW ),
    mbShowProgress( bShowProgress )
{
}

// -----------------------------------------------------------------------------

SdFilter::~SdFilter()
{
}

// -----------------------------------------------------------------------------

::rtl::OUString SdFilter::ImplGetFullLibraryName( const ::rtl::OUString& rLibraryName ) const
{
    String aTemp( ::rtl::OUString::createFromAscii( SVLIBRARY( "?" ) ) );
    xub_StrLen nIndex = aTemp.Search( (sal_Unicode)'?' );
    aTemp.Replace( nIndex, 1, rLibraryName );
    ::rtl::OUString aLibraryName( aTemp );
    return aLibraryName;
}

// -----------------------------------------------------------------------------

::vos::OModule* SdFilter::OpenLibrary( const ::rtl::OUString& rLibraryName ) const
{
    ::rtl::OUString aDest;
    ::rtl::OUString aNormalizedPath;
    ::vos::OModule* pRet;

    if ( ::osl::FileBase::getFileURLFromSystemPath( SvtPathOptions().GetModulePath(), aDest ) != ::osl::FileBase::E_None )
        aDest = SvtPathOptions().GetModulePath();
    aDest += ::rtl::OUString( sal_Unicode( '/' ) );
    aDest += ::rtl::OUString( ImplGetFullLibraryName( rLibraryName ) );
    ::osl::FileBase::getSystemPathFromFileURL( aDest, aNormalizedPath );

    if( !( pRet = new ::vos::OModule( aNormalizedPath ) )->isLoaded() )
        delete pRet, pRet = NULL;

    return pRet;
}

// -----------------------------------------------------------------------------

void SdFilter::CreateStatusIndicator()
{
    // The status indicator must be retrieved from the provided medium arguments
    const SfxUnoAnyItem* pStatusBarItem = static_cast<const SfxUnoAnyItem*>(
            mrMedium.GetItemSet()->GetItem(SID_PROGRESS_STATUSBAR_CONTROL) );

    if ( pStatusBarItem )
        pStatusBarItem->GetValue() >>= mxStatusIndicator;

//  try
//  {
//      if (mxModel.is())
//      {
//          Reference< XController > xController( mxModel->getCurrentController());
//          if( xController.is())
//          {
//              Reference< XFrame > xFrame( xController->getFrame());
//              if( xFrame.is())
//              {
//                  Reference< XStatusIndicatorFactory > xFactory( xFrame, UNO_QUERY );
//                  if( xFactory.is())
//                  {
//                      mxStatusIndicator = xFactory->createStatusIndicator();
//                  }
//              }
//          }
//      }
//  }
//  catch( Exception& )
//  {
//  }
}

