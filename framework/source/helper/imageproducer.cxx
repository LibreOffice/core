/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: imageproducer.cxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: obo $ $Date: 2006-09-16 13:57:27 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_framework.hxx"

#include <helper/imageproducer.hxx>

namespace framework
{

static pfunc_getImage   _pGetImageFunc = NULL;

pfunc_getImage SAL_CALL SetImageProducer( pfunc_getImage pNewGetImageFunc )
{
    pfunc_getImage  pOldFunc = _pGetImageFunc;
    _pGetImageFunc = pNewGetImageFunc;

    return pOldFunc;
}


Image SAL_CALL GetImageFromURL( ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame >& rFrame, const ::rtl::OUString& aURL, BOOL bBig, BOOL bHiContrast )
{
    if ( _pGetImageFunc )
        return _pGetImageFunc( rFrame, aURL, bBig, bHiContrast );
    else
        return Image();
}

}

