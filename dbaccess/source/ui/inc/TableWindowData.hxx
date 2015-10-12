/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */
#ifndef INCLUDED_DBACCESS_SOURCE_UI_INC_TABLEWINDOWDATA_HXX
#define INCLUDED_DBACCESS_SOURCE_UI_INC_TABLEWINDOWDATA_HXX

#include <tools/gen.hxx>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/container/XIndexAccess.hpp>
#include <com/sun/star/sdbc/XConnection.hpp>
#include <unotools/eventlisteneradapter.hxx>
#include <memory>
#include <vector>

namespace dbaui
{
    class OTableWindowData : public ::utl::OEventListenerAdapter
    {
        mutable ::osl::Mutex    m_aMutex;

        void listen();
    protected:
        // the columns of the table
        css::uno::Reference< css::beans::XPropertySet >       m_xTable; // can either be a table or a query
        css::uno::Reference< css::container::XIndexAccess>    m_xKeys;
        css::uno::Reference< css::container::XNameAccess >    m_xColumns;

        OUString        m_aTableName;
        OUString        m_aWinName;
        OUString        m_sComposedName;
        Point           m_aPosition;
        Size            m_aSize;
        bool            m_bShowAll;
        bool            m_bIsQuery;
        bool            m_bIsValid;

    public:
        explicit OTableWindowData(  const css::uno::Reference< css::beans::XPropertySet>& _xTable
                                   ,const OUString& _rComposedName
                                   ,const OUString& strTableName
                                   ,const OUString& rWinName = OUString() );
        virtual ~OTableWindowData();

        /** late constructor
        *
        * \param _xConnection
        * \param _bAllowQueries when true, queries are allowed
        * \return false if the table was unaccessible otherwise true
        */
        bool init(const css::uno::Reference< css::sdbc::XConnection  >& _xConnection
                 ,bool _bAllowQueries);

        inline OUString GetComposedName()    const { return m_sComposedName; }
        inline OUString GetTableName()       const { return m_aTableName; }
        inline OUString GetWinName()         const { return m_aWinName; }
        inline Point GetPosition()                  const { return m_aPosition; }
        inline Size GetSize()                       const { return m_aSize; }
        inline bool IsShowAll()                     const { return m_bShowAll; }
        inline bool isQuery()                       const { return m_bIsQuery; }
        inline bool isValid()                       const { return m_bIsValid; } // it is either a table or query but it is known
        bool HasPosition()  const;
        bool HasSize()      const;

        inline void SetWinName( const OUString& rWinName )       { m_aWinName = rWinName; }
        inline void SetPosition( const Point& rPos )                    { m_aPosition=rPos; }
        inline void SetSize( const Size& rSize )                        { m_aSize = rSize; }
        inline void ShowAll( bool bAll )                                { m_bShowAll = bAll; }

        inline css::uno::Reference< css::beans::XPropertySet> getTable() const { ::osl::MutexGuard aGuard( m_aMutex  ); return m_xTable; }
        inline css::uno::Reference< css::container::XIndexAccess> getKeys() const { ::osl::MutexGuard aGuard( m_aMutex  ); return m_xKeys; }
        inline css::uno::Reference< css::container::XNameAccess > getColumns() const { ::osl::MutexGuard aGuard( m_aMutex  ); return m_xColumns; }

        // OEventListenerAdapter
        virtual void _disposing( const css::lang::EventObject& _rSource ) override;
    };

    typedef ::std::vector< std::shared_ptr<OTableWindowData> >      TTableWindowData;
}
#endif // INCLUDED_DBACCESS_SOURCE_UI_INC_TABLEWINDOWDATA_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
