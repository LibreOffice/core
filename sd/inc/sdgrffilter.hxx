/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: sdgrffilter.hxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 02:59:56 $
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

#ifndef _SD_SDGRFFILTER_HXX
#define _SD_SDGRFFILTER_HXX

#include <tools/errinf.hxx>
#include "sdfilter.hxx"

// ---------------
// - SdCGMFilter -
// ---------------

class SdGRFFilter : public SdFilter
{
public:
    SdGRFFilter (
        SfxMedium& rMedium,
        ::sd::DrawDocShell& rDocShell,
        sal_Bool bShowProgress );
    virtual ~SdGRFFilter (void);

    sal_Bool        Import();
    sal_Bool        Export();

    static void             HandleGraphicFilterError( USHORT nFilterError, ULONG nStreamError = ERRCODE_NONE );

private:

    static GDIMetaFile      ImplRemoveClipRegionActions( const GDIMetaFile& rMtf );
    static BitmapEx         ImplGetBitmapFromMetaFile( const GDIMetaFile& rMtf, BOOL bTransparent, const Size* pSizePixel = NULL );

    bool mbHideSpell;

};

#endif // _SD_SDGRFFILTER_HXX
