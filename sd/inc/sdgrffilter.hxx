/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: sdgrffilter.hxx,v $
 *
 *  $Revision: 1.8 $
 *
 *  last change: $Author: ihi $ $Date: 2006-11-14 14:20:18 $
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

#ifndef _COM_SUN_STAR_DRAWING_XSHAPE_HPP_
#include <com/sun/star/drawing/XShape.hpp>
#endif

#include <tools/errinf.hxx>
#include "sdfilter.hxx"

class Graphic;

// ---------------
// - SdCGMFilter -
// ---------------

class SdGRFFilter : public SdFilter
{
    bool mbHideSpell;

public:
    SdGRFFilter ( SfxMedium& rMedium, ::sd::DrawDocShell& rDocShell );

    virtual ~SdGRFFilter (void);

    sal_Bool        Import();
    sal_Bool        Export();

    static void             SaveGraphic( const ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShape >& xShape );
    static void             HandleGraphicFilterError( USHORT nFilterError, ULONG nStreamError = ERRCODE_NONE );
};

#endif // _SD_SDGRFFILTER_HXX
