/*************************************************************************
 *
 *  $RCSfile: sdhtmlfilter.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: cl $ $Date: 2002-07-16 08:13:01 $
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

#ifndef _OSL_FILE_HXX_
#include <osl/file.hxx>
#endif

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
    SdPublishingDlg aDlg( mrDocShell.GetWindow(), mrDocument.GetDocumentType() );

    mrMedium.Close();
    mrMedium.Commit();

    SfxItemSet *pSet = mrMedium.GetItemSet();

    ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue > aParams;

    const SfxPoolItem* pItem;
    if ( pSet->GetItemState( SID_FILTER_DATA, sal_False, &pItem ) == SFX_ITEM_SET )
        ((SfxUnoAnyItem*)pItem)->GetValue() >>= aParams;

    delete( new HtmlExport( mrMedium.GetName(), aParams, &mrDocument, &mrDocShell ) );

    return true;
}
