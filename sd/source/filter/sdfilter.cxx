/*************************************************************************
 *
 *  $RCSfile: sdfilter.cxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: ka $ $Date: 2001-02-13 12:05:19 $
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

#include <tools/debug.hxx>
#include <osl/file.hxx>
#include <vos/module.hxx>
#include <svtools/pathoptions.hxx>
#include <sfx2/docfile.hxx>
#include <sfx2/viewfrm.hxx>
#include <sfx2/progress.hxx>

#ifndef MAC
#ifndef SVX_LIGHT
#include "../../ui/inc/docshell.hxx"
#include "../../ui/inc/strings.hrc"
#endif //!SVX_LIGHT
#else  //MAC
#ifndef SVX_LIGHT
#include "docshell.hxx"
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

SdFilter::SdFilter( SfxMedium& rMedium, SdDrawDocShell& rDocShell, sal_Bool bShowProgress ) :
    mrMedium( rMedium ),
    mrDocShell( rDocShell ),
    mrDocument( *rDocShell.GetDoc() ),
    mxModel( rDocShell.GetModel() ),
    mpProgress( NULL ),
    mbIsDraw( rDocShell.GetDocumentType() == DOCUMENT_TYPE_DRAW ),
    mbShowProgress( bShowProgress )
{
}

// -----------------------------------------------------------------------------

SdFilter::~SdFilter()
{
    if( !mrDocShell.HasSpecialProgress() )
        delete mpProgress;
}

// -----------------------------------------------------------------------------

::rtl::OUString SdFilter::ImplGetFullLibraryName( const ::rtl::OUString& rLibraryName ) const
{
    USHORT  i,nq,npq;
    String  aLibrary( rLibraryName );
    String  aUPD( String::CreateFromInt32( SOLARUPD ) );
    String  aDllExt( RTL_CONSTASCII_USTRINGPARAM(__DLLEXTENSION));

    aDllExt.Erase( 2 );

    // Fragezeichen suchen
    for( i = 0, nq = 0; aLibrary.GetChar(i)!=0; i++ )
    {
        if( aLibrary.GetChar(i) == '?' )
        {
            if( nq == 0 )
                npq=i;

            nq++;
        }
        else if( nq==aUPD.Len() )
            break;
        else
            nq=0;
    }

    // Fragezeichen durch UPD-Nummer ersetzen
    if( nq == aUPD.Len() )
        aLibrary.Replace( npq, nq, aUPD );

    // Sternchen suchen
    for( i = 0, nq = 0; aLibrary.GetChar(i) != 0; i++ )
    {
        if( aLibrary.GetChar(i) == '*' )
        {
            if( nq == 0 )
                npq=i;

            nq++;
        }
        else if( nq == 2 )
            break;
        else
            nq=0;
    }

    // Sternchen durch Plattform-Kuerzel ersetzen
    if( nq == 2 )
        aLibrary.Replace( npq, nq, aDllExt );

    return aLibrary;
}

// -----------------------------------------------------------------------------

::vos::OModule* SdFilter::OpenLibrary( const ::rtl::OUString& rLibraryName ) const
{
    ::rtl::OUString aDest;
    ::rtl::OUString aNormalizedPath;
    ::vos::OModule* pRet;

    ::osl::FileBase::normalizePath( SvtPathOptions().GetFilterPath(), aDest );
    aDest += ::rtl::OUString( sal_Unicode( '/' ) );
    aDest += ::rtl::OUString( ImplGetFullLibraryName( rLibraryName ) );
    ::osl::FileBase::getSystemPathFromNormalizedPath( aDest, aNormalizedPath );

    if( !( pRet = new ::vos::OModule( aNormalizedPath ) )->isLoaded() )
        delete pRet, pRet = NULL;

    return pRet;
}

// -----------------------------------------------------------------------------

void SdFilter::CreateStatusIndicator()
{
    if( mxModel.is() )
    {
        Reference< XController > xController( mxModel->getCurrentController() );

        if( xController.is() )
        {
            Reference< XFrame > xFrame( xController->getFrame() );

            if( xFrame.is() )
            {
                Reference< XStatusIndicatorSupplier > xSupplier( xFrame, UNO_QUERY );

                if( xSupplier.is() )
                {
                    mxStatusIndicator = xSupplier->getStatusIndicator();
                }
            }
        }
    }
}

// -----------------------------------------------------------------------------

void SdFilter::CreateProgress()
{
    if( mrDocShell.HasSpecialProgress() )
        mpProgress = mrDocShell.GetSpecialProgress();
    else
    {
        mpProgress = new SfxProgress( &mrDocShell, String( SdResId( STR_OPEN_DOCUMENT ) ), 100 );
        mpProgress->SetState( 0, 100 );
    }
}

// -----------------------------------------------------------------------------

sal_Bool SdFilter::Import()
{
    DBG_ERROR( "Not implemented" );
    return sal_False;
}

// -----------------------------------------------------------------------------

sal_Bool SdFilter::Export()
{
    DBG_ERROR( "Not implemented" );
    return sal_False;
}
