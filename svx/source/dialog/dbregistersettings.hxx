/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: dbregistersettings.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 20:56:53 $
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

#ifndef SVX_DBREGISTERSETTING_HXX
#define SVX_DBREGISTERSETTING_HXX

#ifndef _COMPHELPER_STLTYPES_HXX_
#include <comphelper/stl_types.hxx>
#endif

#ifndef _SFXPOOLITEM_HXX
#include <svtools/poolitem.hxx>
#endif

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

