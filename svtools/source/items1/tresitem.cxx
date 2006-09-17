/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: tresitem.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: obo $ $Date: 2006-09-17 15:07:41 $
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
#include "precompiled_svtools.hxx"

#ifndef SVTOOLS_TRESITEM_HXX
#include <tresitem.hxx>
#endif

using namespace com::sun::star;

//============================================================================
//
//  CntTransferResultItem
//
//============================================================================

TYPEINIT1_AUTOFACTORY(CntTransferResultItem, SfxPoolItem)

//============================================================================
// virtual
int CntTransferResultItem::operator ==(SfxPoolItem const & rItem) const
{
    if (CntTransferResultItem * pResultItem = PTR_CAST(CntTransferResultItem,
                                                       &rItem))
        return m_aResult.Source == pResultItem->m_aResult.Source
               && m_aResult.Target == pResultItem->m_aResult.Target
               && m_aResult.Result == pResultItem->m_aResult.Result;
    return false;
}

//============================================================================
// virtual
BOOL CntTransferResultItem::QueryValue(uno::Any & rVal, BYTE) const
{
    rVal <<= m_aResult;
    return true;
}

//============================================================================
// virtual
BOOL CntTransferResultItem::PutValue(uno::Any const & rVal, BYTE)
{
    return rVal >>= m_aResult;
}

//============================================================================
// virtual
SfxPoolItem * CntTransferResultItem::Clone(SfxItemPool *) const
{
    return new CntTransferResultItem(*this);
}

