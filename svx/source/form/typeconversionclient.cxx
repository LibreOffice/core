/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: typeconversionclient.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: obo $ $Date: 2006-09-17 05:14:20 $
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
#include "precompiled_svx.hxx"

#ifndef SVX_TYPECONVERSION_CLIENT_HXX
#include "typeconversionclient.hxx"
#endif

//........................................................................
namespace svxform
{
//........................................................................

    //====================================================================
    //= OTypeConversionClient
    //====================================================================
    //--------------------------------------------------------------------
    OTypeConversionClient::OTypeConversionClient()
    {
    }

    //--------------------------------------------------------------------
    //add by BerryJia for fixing Bug97420 Time:2002-9-12-11:00(PRC time)
    void OTypeConversionClient::create() const
    {
        if (!getFactory().is())
            ODbtoolsClient::create();
        if (getFactory().is())
             m_xTypeConversion = getFactory()->getTypeConversionHelper();
    }

//........................................................................
}   // namespace svxform
//........................................................................


