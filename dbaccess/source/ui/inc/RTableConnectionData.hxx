/*************************************************************************
 *
 *  $RCSfile: RTableConnectionData.hxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: oj $ $Date: 2001-02-28 10:07:39 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the License); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an AS IS basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/
#ifndef DBAUI_RTABLECONNECTIONDATA_HXX
#define DBAUI_RTABLECONNECTIONDATA_HXX

#ifndef DBAUI_TABLECONNECTIONDATA_HXX
#include "TableConnectionData.hxx"
#endif
#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSET_HPP_
#include <com/sun/star/beans/XPropertySet.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XNAMEACCESS_HPP_
#include <com/sun/star/container/XNameAccess.hpp>
#endif
#ifndef DBAUI_ENUMTYPES_HXX
#include "QEnumTypes.hxx"
#endif

namespace dbaui
{
    const UINT16 CARDINAL_UNDEFINED = 0x0000;
    const UINT16 CARDINAL_ONE_MANY  = 0x0001;
    const UINT16 CARDINAL_MANY_ONE  = 0x0002;
    const UINT16 CARDINAL_ONE_ONE   = 0x0004;

    class OConnectionLineData;
    //==================================================================
    class ORelationTableConnectionData : public OTableConnectionData
    {
        ::rtl::OUString m_sDatabaseName;
        ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess> m_xTables;
        ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet>    m_xSource;
        ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet>    m_xDest;

        // @see com.sun.star.sdbc.KeyRule
        sal_Int32 m_nUpdateRules;
        sal_Int32 m_nDeleteRules;
        sal_Int32 m_nCardinality;

        BOOL checkPrimaryKey(const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet>& _xTable,EConnectionSide _eEConnectionSide) const;
        BOOL IsSourcePrimKey()  const { return checkPrimaryKey(m_xSource,JTCS_FROM);    }
        BOOL IsDestPrimKey()    const { return checkPrimaryKey(m_xDest,JTCS_TO);        }

    protected:
        virtual OConnectionLineData* CreateLineDataObj();
        virtual OConnectionLineData* CreateLineDataObj( const OConnectionLineData& rConnLineData );

    public:
        ORelationTableConnectionData();
        ORelationTableConnectionData( const ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess>& _xTables);
        ORelationTableConnectionData( const ORelationTableConnectionData& rConnData );
        ORelationTableConnectionData( const ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess>& _xTables,
                                      const ::rtl::OUString& rSourceWinName,
                                      const ::rtl::OUString& rDestWinName,
                                      const ::rtl::OUString& rConnName = ::rtl::OUString() );
        virtual ~ORelationTableConnectionData();

        virtual void CopyFrom(const OTableConnectionData& rSource);
        virtual OTableConnectionData* NewInstance() const { return new ORelationTableConnectionData(); }

        ORelationTableConnectionData& operator=( const ORelationTableConnectionData& rConnData );

        ::rtl::OUString GetDatabaseName() const { return m_sDatabaseName; }

        ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess> getTables() const { return m_xTables;}
        ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet>    getSource() const { return m_xSource;}
        ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet>    getDest() const { return m_xDest; }

        virtual void SetSourceWinName( const String& rSourceWinName );
        virtual void SetDestWinName( const String& rDestWinName );

        void        SetCardinality();
        void        SetUpdateRules( sal_Int32 nAttr ){ m_nUpdateRules = nAttr; }
        void        SetDeleteRules( sal_Int32 nAttr ){ m_nDeleteRules = nAttr; }

        sal_Int32   GetUpdateRules() const { return m_nUpdateRules; }
        sal_Int32   GetDeleteRules() const { return m_nDeleteRules; }
        sal_Int32   GetCardinality() const { return m_nCardinality; }

        BOOL        Update();
        BOOL        IsConnectionPossible();
        void        ChangeOrientation();
        BOOL        DropRelation();
    };
}

#endif // DBAUI_RTABLECONNECTIONDATA_HXX



