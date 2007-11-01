/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: TableWindowData.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: hr $ $Date: 2007-11-01 15:19:26 $
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
#ifndef DBAUI_TABLEWINDOWDATA_HXX
#define DBAUI_TABLEWINDOWDATA_HXX

#ifndef _SV_GEN_HXX
#include <tools/gen.hxx>
#endif
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/container/XNameAccess.hpp>
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
        ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess >    m_xColumns;

        ::rtl::OUString m_aTableName;
        ::rtl::OUString m_aWinName;
        ::rtl::OUString m_sComposedName;
        Point           m_aPosition;
        Size            m_aSize;
        sal_Bool        m_bShowAll;
        bool            m_bIsQuery;

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
        inline BOOL IsShowAll()                     const { return m_bShowAll; }
        inline bool isQuery()                       const { return m_bIsQuery; }
        BOOL HasPosition()  const;
        BOOL HasSize()      const;

        inline void SetWinName( const ::rtl::OUString& rWinName )       { m_aWinName = rWinName; }
        inline void SetPosition( const Point& rPos )                    { m_aPosition=rPos; }
        inline void SetSize( const Size& rSize )                        { m_aSize = rSize; }
        inline void ShowAll( BOOL bAll )                                { m_bShowAll = bAll; }

        inline ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet> getTable() const { ::osl::MutexGuard aGuard( m_aMutex  ); return m_xTable; }
        inline ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess > getColumns() const { ::osl::MutexGuard aGuard( m_aMutex  ); return m_xColumns; }

        // OEventListenerAdapter
        virtual void _disposing( const ::com::sun::star::lang::EventObject& _rSource );
    };

    typedef ::std::vector< ::boost::shared_ptr<OTableWindowData> >      TTableWindowData;
}
#endif // DBAUI_TABLEWINDOWDATA_HXX

