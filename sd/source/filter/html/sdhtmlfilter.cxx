/*************************************************************************
 *
 *  $RCSfile: sdhtmlfilter.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: cl $ $Date: 2002-06-25 15:32:39 $
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

#include <unotools/localfilehelper.hxx>
#include <tools/errinf.hxx>
#include <tools/urlobj.hxx>
#include <vcl/msgbox.hxx>
#include <vcl/metaact.hxx>
#include <vcl/virdev.hxx>
#include <svtools/FilterConfigItem.hxx>
#include <sfx2/docfile.hxx>
#include <sfx2/docfilt.hxx>
#include <svx/impgrf.hxx>
#include <svx/svdograf.hxx>
#include <svx/svdpagv.hxx>
#include <svx/xoutbmp.hxx>

#include "sdpage.hxx"
#include "drawdoc.hxx"
#include "sdresid.hxx"
#include "sdattr.hxx"
#include "htmlex.hxx"
#include "sdhtmlfilter.hxx"

// ---------------
// - SdPPTFilter -
// ---------------

SdHTMLFilter::SdHTMLFilter( SfxMedium& rMedium, SdDrawDocShell& rDocShell, sal_Bool bShowProgress ) :
    SdFilter( rMedium, rDocShell, bShowProgress )
{
}

// -----------------------------------------------------------------------------

SdHTMLFilter::~SdHTMLFilter()
{
}

// -----------------------------------------------------------------------------

sal_Bool SdHTMLFilter::Export()
{
    SfxItemSet      aArgs( mrDocument.GetPool(), ATTR_PUBLISH_START, ATTR_PUBLISH_END );
    SdPublishingDlg aDlg( mrDocShell.GetWindow(), mrDocument.GetDocumentType() );

    if( aDlg.Execute() )
    {
//      String aTmp;
//      ::utl::LocalFileHelper::ConvertSystemPathToURL( mrMedium.GetName(), aTmp );
//      const INetURLObject aURL( aTmp );

        mrMedium.Close();
        mrMedium.Commit();
        aDlg.FillItemSet( aArgs );
        aArgs.Put( SfxStringItem( ATTR_PUBLISH_LOCATION, mrMedium.GetName() ) );
        delete( new HtmlExport( &mrDocument, &mrDocShell, &aArgs ) );
    }

    return true;
}
