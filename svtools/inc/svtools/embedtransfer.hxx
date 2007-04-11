/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: embedtransfer.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2007-04-11 19:17:38 $
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

#ifndef _EMBEDTRANSFER_HXX
#define _EMBEDTRANSFER_HXX

#ifndef INCLUDED_SVTDLLAPI_H
#include "svtools/svtdllapi.h"
#endif

#ifndef _COM_SUN_STAR_EMBED_XEMBEDDEDOBJECT_HPP_
#include <com/sun/star/embed/XEmbeddedObject.hpp>
#endif

#ifndef _TRANSFER_HXX
#include <svtools/transfer.hxx>
#endif

class Graphic;
class SVT_DLLPUBLIC SvEmbedTransferHelper : public TransferableHelper
{
private:

    ::com::sun::star::uno::Reference< ::com::sun::star::embed::XEmbeddedObject > m_xObj;
    Graphic* m_pGraphic;
    sal_Int64 m_nAspect;

protected:

    virtual void        AddSupportedFormats();
    virtual sal_Bool    GetData( const ::com::sun::star::datatransfer::DataFlavor& rFlavor );
    virtual void        ObjectReleased();

public:
    // object, replacement image, and the aspect
    SvEmbedTransferHelper( const ::com::sun::star::uno::Reference< ::com::sun::star::embed::XEmbeddedObject >& xObj,
                            Graphic* pGraphic,
                            sal_Int64 nAspect );
    ~SvEmbedTransferHelper();

    static void         FillTransferableObjectDescriptor( TransferableObjectDescriptor& rDesc,
                            const ::com::sun::star::uno::Reference< ::com::sun::star::embed::XEmbeddedObject >& xObj,
                            Graphic* pGraphic,
                            sal_Int64 nAspect );
};

#endif

