/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: tresitem.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2007-04-11 19:41:06 $
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

#ifndef SVTOOLS_TRESITEM_HXX
#define SVTOOLS_TRESITEM_HXX

#ifndef _COM_SUN_STAR_UCB_TRANSFERRESULT_HPP_
#include <com/sun/star/ucb/TransferResult.hpp>
#endif

#ifndef _SFXPOOLITEM_HXX
#include <svtools/poolitem.hxx>
#endif

//============================================================================
class CntTransferResultItem: public SfxPoolItem
{
    com::sun::star::ucb::TransferResult m_aResult;

public:
    TYPEINFO();

    CntTransferResultItem(USHORT which = 0): SfxPoolItem(which) {}

    CntTransferResultItem(USHORT which,
                          com::sun::star::ucb::TransferResult const &
                              rTheResult):
        SfxPoolItem(which), m_aResult(rTheResult) {}

    virtual int operator ==(SfxPoolItem const & rItem) const;

    virtual BOOL QueryValue(com::sun::star::uno::Any & rVal, BYTE = 0) const;

    virtual BOOL PutValue(const com::sun::star::uno::Any & rVal, BYTE = 0);

    virtual SfxPoolItem * Clone(SfxItemPool * = 0) const;

    com::sun::star::ucb::TransferResult const & GetValue() const
    { return m_aResult; }
};

#endif // SVTOOLS_TRESITEM_HXX

