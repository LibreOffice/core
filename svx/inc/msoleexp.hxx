/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: msoleexp.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 18:06:12 $
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
#ifndef _MSOLEEXP_HXX
#define _MSOLEEXP_HXX

#ifndef _COM_SUN_STAR_UNO_REFERENCE_H_
#include <com/sun/star/uno/Reference.h>
#endif

#ifndef INCLUDED_SVXDLLAPI_H
#include "svx/svxdllapi.h"
#endif

// for the CreateSdrOLEFromStorage we need the information, how we handle
// convert able OLE-Objects - this ist stored in
#define OLE_STARMATH_2_MATHTYPE             0x0001
#define OLE_STARWRITER_2_WINWORD            0x0002
#define OLE_STARCALC_2_EXCEL                0x0004
#define OLE_STARIMPRESS_2_POWERPOINT        0x0008

class SotStorage;

#include <svtools/embedhlp.hxx>

class SVX_DLLPUBLIC SvxMSExportOLEObjects
{
    UINT32 nConvertFlags;
public:
    SvxMSExportOLEObjects( UINT32 nCnvrtFlgs ) : nConvertFlags(nCnvrtFlgs) {}

    void SetFlags( UINT32 n )       { nConvertFlags = n; }
    UINT32 GetFlags() const         { return nConvertFlags; }

    void ExportOLEObject( svt::EmbeddedObjectRef& rObj, SotStorage& rDestStg );
    void ExportOLEObject( const com::sun::star::uno::Reference < com::sun::star::embed::XEmbeddedObject>& rObj, SotStorage& rDestStg );
};



#endif

