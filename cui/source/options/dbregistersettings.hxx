/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: dbregistersettings.hxx,v $
 * $Revision: 1.4 $
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

#ifndef SVX_DBREGISTERSETTING_HXX
#define SVX_DBREGISTERSETTING_HXX

#include <comphelper/stl_types.hxx>
#include <svl/poolitem.hxx>

//........................................................................
namespace svx
{
//........................................................................

    //====================================================================
    //= DatabaseMapItem
    //====================================================================
    DECLARE_STL_USTRINGACCESS_MAP(::rtl::OUString,TNameLocationMap);
    class DatabaseMapItem : public SfxPoolItem
    {
    protected:
        TNameLocationMap    m_aSettings;

    public:
        TYPEINFO();

        DatabaseMapItem( sal_uInt16 _nId, const TNameLocationMap& _rSettings );

        virtual int              operator==( const SfxPoolItem& ) const;
        virtual SfxPoolItem*     Clone( SfxItemPool *pPool = 0 ) const;

        const TNameLocationMap& getSettings() const { return m_aSettings; }
    };

//........................................................................
}   // namespace svx
//........................................................................
#endif // SVX_DBREGISTERSETTING_HXX

