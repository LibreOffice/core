/*************************************************************************
 *
 *  $RCSfile: sdgrffilter.cxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: ka $ $Date: 2001-02-13 12:08:24 $
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

#include <tools/errinf.hxx>
#include <tools/urlobj.hxx>
#include <vcl/msgbox.hxx>
#include <sfx2/docfile.hxx>
#include <sfx2/docfilt.hxx>
#include <svx/impgrf.hxx>
#include <svx/svdograf.hxx>

#ifndef MAC
#ifndef SVX_LIGHT
#include "../../ui/inc/strings.hrc"
#include "../../ui/inc/graphpro.hxx"
#endif //!SVX_LIGHT
#else  //MAC
#ifndef SVX_LIGHT
#include "strings.hrc"
#include "graphpro.hxx"
#endif //!SVX_LIGHT
#endif //!MAC


#include "sdpage.hxx"
#include "drawdoc.hxx"
#include "sdresid.hxx"
#include "sdgrffilter.hxx"

// ---------------
// - SdPPTFilter -
// ---------------

SdGRFFilter::SdGRFFilter( SfxMedium& rMedium, SdDrawDocShell& rDocShell, sal_Bool bShowProgress ) :
    SdFilter( rMedium, rDocShell, bShowProgress )
{
}

// -----------------------------------------------------------------------------

SdGRFFilter::~SdGRFFilter()
{
}

// -----------------------------------------------------------------------------

void SdGRFFilter::HandleGraphicFilterError( USHORT nFilterError, ULONG nStreamError )
{
    USHORT nId;

    switch( nFilterError )
    {
        case GRFILTER_OPENERROR:
            nId = STR_IMPORT_GRFILTER_OPENERROR;
            break;
        case GRFILTER_IOERROR:
            nId = STR_IMPORT_GRFILTER_IOERROR;
            break;
        case GRFILTER_FORMATERROR:
            nId = STR_IMPORT_GRFILTER_FORMATERROR;
            break;
        case GRFILTER_VERSIONERROR:
            nId = STR_IMPORT_GRFILTER_VERSIONERROR;
            break;
        case GRFILTER_FILTERERROR:
            nId = STR_IMPORT_GRFILTER_FILTERERROR;
            break;
        case GRFILTER_TOOBIG:
            nId = STR_IMPORT_GRFILTER_TOOBIG;
            break;
        default:
            nId = STR_IMPORT_GRFILTER_FILTERERROR;
            break;
    }

    if( ERRCODE_NONE != nStreamError )
        ErrorHandler::HandleError( nStreamError );
    else if( STR_IMPORT_GRFILTER_IOERROR == nId )
        ErrorHandler::HandleError( ERRCODE_IO_GENERAL );
    else
    {
        ErrorBox aErrorBox( NULL, WB_OK, String( SdResId( nId ) ) );
        aErrorBox.Execute();
    }
}

// -----------------------------------------------------------------------------

sal_Bool SdGRFFilter::Import()
{
    Graphic         aGraphic;
    const String    aName( mrMedium.GetFilter()->GetName() );
    const String    aFileName( mrMedium.GetURLObject().GetMainURL() );
    GraphicFilter*  pGraphicFilter = GetGrfFilter();
    const USHORT    nFilter = pGraphicFilter->GetImportFormatNumber( aName );
    sal_Bool        bRet = sal_False;

    // ggf. Filterdialog ausfuehren
    if ( !pGraphicFilter->HasImportDialog( nFilter ) || pGraphicFilter->DoImportDialog( NULL, nFilter ) )
    {
        SvStream*       pIStm = mrMedium.GetInStream();
        FilterProgress* pFilterProgress = mbShowProgress ? new FilterProgress( pGraphicFilter, &(SfxObjectShell&) mrDocShell ) : NULL;
        USHORT          nReturn = pIStm ? pGraphicFilter->ImportGraphic( aGraphic, aFileName, *pIStm, nFilter ) : 1;

        if( nReturn )
            HandleGraphicFilterError( nReturn, pGraphicFilter->GetLastError().nStreamError );
        else
        {
            if( mrDocument.GetPageCount() == 0L )
                mrDocument.CreateFirstPages();

            SdPage*     pPage = mrDocument.GetSdPage( 0, PK_STANDARD );
            Point       aPos;
            Size        aPagSize( pPage->GetSize() );
            Size        aGrfSize( OutputDevice::LogicToLogic( aGraphic.GetPrefSize(),
                                  aGraphic.GetPrefMapMode(), MAP_100TH_MM ) );

            aPagSize.Width() -= pPage->GetLftBorder() + pPage->GetRgtBorder();
            aPagSize.Height() -= pPage->GetUppBorder() + pPage->GetLwrBorder();

            // scale to fit page
            if ( ( aGrfSize.Height() > aPagSize.Height() ) || (aGrfSize.Width() > aPagSize.Width() ) &&
                 aGrfSize.Height() && aPagSize.Height() )
            {
                double fGrfWH = (double) aGrfSize.Width() / aGrfSize.Height();
                double fWinWH = (double) aPagSize.Width() / aPagSize.Height();

                // Grafik an Pagesize anpassen (skaliert)
                if( fGrfWH < fWinWH )
                {
                    aGrfSize.Width() = (long) ( aPagSize.Height() * fGrfWH );
                    aGrfSize.Height() = aPagSize.Height();
                }
                else if( fGrfWH > 0.F )
                {
                    aGrfSize.Width() = aPagSize.Width();
                    aGrfSize.Height()= (long) ( aPagSize.Width() / fGrfWH );
                }
            }

            // Ausgaberechteck fuer Grafik setzen
            aPos.X() = ( ( aPagSize.Width() - aGrfSize.Width() ) >> 1 ) + pPage->GetLftBorder();
            aPos.Y() = ( ( aPagSize.Height() - aGrfSize.Height() ) >> 1 )  + pPage->GetUppBorder();

            pPage->InsertObject( new SdrGrafObj( aGraphic, Rectangle( aPos, aGrfSize ) ) );
            bRet = sal_True;
        }

        delete pFilterProgress;
    }

    return bRet;
}
