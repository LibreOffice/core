/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: WW8Picture.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: hbrinkm $ $Date: 2006-11-09 15:56:00 $
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

#include <resources.hxx>
#include <WW8DocumentImpl.hxx>

namespace doctok
{

doctok::Reference<Properties>::Pointer_t
WW8PICF::get_DffRecord()
{
    doctok::Reference<Properties>::Pointer_t pRet(new DffBlock(this, get_cbHeader(), getCount() - get_cbHeader(), 0));
    return pRet;
}

doctok::Reference<Properties>::Pointer_t
WW8FSPA::get_shape()
{
    return getDocument()->getShape(get_spid());
}
}
