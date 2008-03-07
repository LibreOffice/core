/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: propertystorage.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: kz $ $Date: 2008-03-07 11:24:41 $
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

#ifndef PROPERTYSTORAGE_HXX
#define PROPERTYSTORAGE_HXX

/** === begin UNO includes === **/
#include <com/sun/star/uno/Any.hxx>
/** === end UNO includes === **/

#include <tools/solar.h>

#include <boost/shared_ptr.hpp>
#include <map>

class SfxItemSet;

//........................................................................
namespace dbaui
{
//........................................................................

    //====================================================================
    //= PropertyStorage
    //====================================================================
    class SAL_NO_VTABLE PropertyStorage
    {
    public:
        virtual void getPropertyValue( ::com::sun::star::uno::Any& _out_rValue ) const = 0;
        virtual void setPropertyValue( const ::com::sun::star::uno::Any& _rValue ) = 0;

        virtual ~PropertyStorage();
    };

    typedef ::boost::shared_ptr< PropertyStorage >      PPropertyStorage;
    typedef ::std::map< sal_Int32, PPropertyStorage >   PropertyValues;

    //====================================================================
    //= SetItemPropertyStorage
    //====================================================================
    typedef USHORT  ItemId;

    /** a PropertyStorage implementation which stores the value in an item set
    */
    class SetItemPropertyStorage : public PropertyStorage
    {
    public:
        SetItemPropertyStorage( SfxItemSet& _rItemSet, const ItemId _nItemID )
            :m_rItemSet( _rItemSet )
            ,m_nItemID( _nItemID )
        {
        }

        virtual ~SetItemPropertyStorage()
        {
        }

        virtual void getPropertyValue( ::com::sun::star::uno::Any& _out_rValue ) const;
        virtual void setPropertyValue( const ::com::sun::star::uno::Any& _rValue );

    private:
        SfxItemSet&     m_rItemSet;
        const ItemId    m_nItemID;
    };

//........................................................................
} // namespace dbaui
//........................................................................

#endif // PROPERTYSTORAGE_HXX
