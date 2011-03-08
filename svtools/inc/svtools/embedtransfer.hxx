/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#ifndef _EMBEDTRANSFER_HXX
#define _EMBEDTRANSFER_HXX

#include "svtools/svtdllapi.h"
#include <com/sun/star/embed/XEmbeddedObject.hpp>
#include <svtools/transfer.hxx>

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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
