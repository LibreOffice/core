/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: sdxmlwrp.hxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 03:02:24 $
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
#ifndef _SDXMLWRP_HXX
#define _SDXMLWRP_HXX

#include <tools/errcode.hxx>
#include "sdfilter.hxx"

// ---------------
// - SdXMLFilter -
// ---------------

enum SdXMLFilterMode
{
    SDXMLMODE_Normal,           // standard load and save of the complete document
    SDXMLMODE_Preview,          // only for import, only the first draw page and its master page is loaded
    SDXMLMODE_Organizer         // only for import, only the styles are loaded
};

class SdXMLFilter : public SdFilter
{
public:
    SdXMLFilter(
        SfxMedium& rMedium,
        ::sd::DrawDocShell& rDocShell,
        sal_Bool bShowProgress,
        SdXMLFilterMode eFilterMode = SDXMLMODE_Normal,
        ULONG nStoreVer = SOFFICE_FILEFORMAT_8 );
    virtual ~SdXMLFilter (void);

    sal_Bool        Import( ErrCode& nError );
    sal_Bool        Export();

private:
    SdXMLFilterMode meFilterMode;
    ULONG mnStoreVer;
};

#endif  // _SDXMLWRP_HXX
