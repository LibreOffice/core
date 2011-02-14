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
#ifndef DBAUI_TABLEWINDOWDATA_HXX
#define DBAUI_TABLEWINDOWDATA_HXX

#ifndef _SV_GEN_HXX
#include <tools/gen.hxx>
#endif
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/container/XIndexAccess.hpp>
#include <com/sun/star/sdbc/XConnection.hpp>
#include <unotools/eventlisteneradapter.hxx>
#include <boost/shared_ptr.hpp>
#include <vector>

namespace dbaui
{
    class OTableWindowData : public ::utl::OEventListenerAdapter
    {
        mutable ::osl::Mutex    m_aMutex;

        void listen();
    protected:
        // the columns of the table
        ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >       m_xTable; // can either be a table or a query
        ::com::sun::star::uno::Reference< ::com::sun::star::container::XIndexAccess>    m_xKeys;
        ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess >    m_xColumns;

        ::rtl::OUString m_aTableName;
        ::rtl::OUString m_aWinName;
        ::rtl::OUString m_sComposedName;
        Point           m_aPosition;
        Size            m_aSize;
        sal_Bool        m_bShowAll;
        bool            m_bIsQuery;
        bool            m_bIsValid;

    public:
        explicit OTableWindowData(  const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet>& _xTable
                                   ,const ::rtl::OUString& _rComposedName
                                   ,const ::rtl::OUString& strTableName
                                   ,const ::rtl::OUString& rWinName = ::rtl::OUString() );
        virtual ~OTableWindowData();

        /** late constructor
        *
        * \param _xConnection
        * \param _bAllowQueries when true, queries are allowed
        * \return false if the table was unaccessible otherwise true
        */
        bool init(const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection  >& _xConnection
                 ,bool _bAllowQueries);

        inline ::rtl::OUString GetComposedName()    const { return m_sComposedName; }
        inline ::rtl::OUString GetTableName()       const { return m_aTableName; }
        inline ::rtl::OUString GetWinName()         const { return m_aWinName; }
        inline Point GetPosition()                  const { return m_aPosition; }
        inline Size GetSize()                       const { return m_aSize; }
        inline sal_Bool IsShowAll()                     const { return m_bShowAll; }
        inline bool isQuery()                       const { return m_bIsQuery; }
        inline bool isValid()                       const { return m_bIsValid; } // it is either a table or query but it is known
        sal_Bool HasPosition()  const;
        sal_Bool HasSize()      const;

        inline void SetWinName( const ::rtl::OUString& rWinName )       { m_aWinName = rWinName; }
        inline void SetPosition( const Point& rPos )                    { m_aPosition=rPos; }
        inline void SetSize( const Size& rSize )                        { m_aSize = rSize; }
        inline void ShowAll( sal_Bool bAll )                                { m_bShowAll = bAll; }

        inline ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet> getTable() const { ::osl::MutexGuard aGuard( m_aMutex  ); return m_xTable; }
        inline ::com::sun::star::uno::Reference< ::com::sun::star::container::XIndexAccess> getKeys() const { ::osl::MutexGuard aGuard( m_aMutex  ); return m_xKeys; }
        inline ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess > getColumns() const { ::osl::MutexGuard aGuard( m_aMutex  ); return m_xColumns; }

        // OEventListenerAdapter
        virtual void _disposing( const ::com::sun::star::lang::EventObject& _rSource );
    };

    typedef ::std::vector< ::boost::shared_ptr<OTableWindowData> >      TTableWindowData;
}
#endif // DBAUI_TABLEWINDOWDATA_HXX

