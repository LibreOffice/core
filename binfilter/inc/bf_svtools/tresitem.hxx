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

#ifndef SVTOOLS_TRESITEM_HXX
#define SVTOOLS_TRESITEM_HXX

#ifndef _COM_SUN_STAR_UCB_TRANSFERRESULT_HPP_
#include <com/sun/star/ucb/TransferResult.hpp>
#endif

#ifndef _SFXPOOLITEM_HXX
#include <bf_svtools/poolitem.hxx>
#endif

namespace binfilter
{

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

}

#endif // SVTOOLS_TRESITEM_HXX

