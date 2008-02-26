/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: docinf.hxx,v $
 *
 *  $Revision: 1.8 $
 *
 *  last change: $Author: obo $ $Date: 2008-02-26 14:57:32 $
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
#ifndef _SFXDOCINF_HXX
#define _SFXDOCINF_HXX

#ifndef _SAL_CONFIG_H_
#include "sal/config.h"
#endif

#ifndef INCLUDED_SFX2_DLLAPI_H
#include "sfx2/dllapi.h"
#endif

#ifndef _SAL_TYPES_H_
#include "sal/types.h"
#endif
#ifndef _COM_SUN_STAR_UNO_REFERENCE_HXX_
#include <com/sun/star/uno/Reference.hxx>
#endif


class GDIMetaFile;
class SotStorage;

namespace com { namespace sun { namespace star {
    namespace document {
        class XDocumentProperties;
    }
} } }


namespace sfx2 {

/** load document meta-data from OLE stream

    @param  i_xDocProps     Document meta-data
    @param  i_pStorage      OLE Storage
 */
sal_uInt32 SFX2_DLLPUBLIC LoadOlePropertySet(
    ::com::sun::star::uno::Reference<
        ::com::sun::star::document::XDocumentProperties> i_xDocProps,
    SotStorage* i_pStorage );

/** save document meta-data to OLE stream

    @param  i_xDocProps     Document meta-data
    @param  i_pStorage      OLE Storage
    @param  i_pThumb        Thumbnail: preview bitmap
    @param  i_pGuid         Blob: Guid blob ("_PID_GUID")
    @param  i_pHyperlinks   Blob: Hyperlink blob ("_PID_HLINKS")
 */
bool SFX2_DLLPUBLIC SaveOlePropertySet(
    ::com::sun::star::uno::Reference<
        ::com::sun::star::document::XDocumentProperties> i_xDocProps,
    SotStorage* i_pStorage,
    const ::com::sun::star::uno::Sequence<sal_uInt8> * i_pThumb = 0,
    const ::com::sun::star::uno::Sequence<sal_uInt8> * i_pGuid = 0,
    const ::com::sun::star::uno::Sequence<sal_uInt8> * i_pHyperlinks = 0);


::com::sun::star::uno::Sequence<sal_uInt8> SFX2_DLLPUBLIC convertMetaFile(
    GDIMetaFile* i_pThumb);

} // namespace sfx2

#endif

