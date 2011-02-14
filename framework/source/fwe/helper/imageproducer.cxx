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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_framework.hxx"

#include <framework/imageproducer.hxx>

namespace framework
{

static pfunc_getImage   _pGetImageFunc = NULL;

pfunc_getImage SAL_CALL SetImageProducer( pfunc_getImage pNewGetImageFunc )
{
    pfunc_getImage  pOldFunc = _pGetImageFunc;
    _pGetImageFunc = pNewGetImageFunc;

    return pOldFunc;
}


Image SAL_CALL GetImageFromURL( const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame >& rFrame, const ::rtl::OUString& aURL, sal_Bool bBig, sal_Bool bHiContrast )
{
    if ( _pGetImageFunc )
        return _pGetImageFunc( rFrame, aURL, bBig, bHiContrast );
    else
        return Image();
}

}

