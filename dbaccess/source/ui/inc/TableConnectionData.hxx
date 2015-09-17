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
#ifndef INCLUDED_DBACCESS_SOURCE_UI_INC_TABLECONNECTIONDATA_HXX
#define INCLUDED_DBACCESS_SOURCE_UI_INC_TABLECONNECTIONDATA_HXX

#include "ConnectionLineData.hxx"
#include "TableWindowData.hxx"
#include <vector>
#include <tools/rtti.hxx>
#include <memory>

namespace dbaui
{
    //===========================================//
    // ConnData     ---------->*    ConnLineData //
    //    ^1                            ^1       //
    //    |                             |        //
    //  Conn        ---------->*    ConnLine     //
    //===========================================//

    /** Contains all connection data which exists between two windows */
    class OTableConnectionData
    {
    protected:
        TTableWindowData::value_type m_pReferencingTable;
        TTableWindowData::value_type m_pReferencedTable;
        OUString m_aConnName;

        OConnectionLineDataVec m_vConnLineData;

        void    Init();

        virtual OConnectionLineDataRef CreateLineDataObj();
        virtual OConnectionLineDataRef CreateLineDataObj( const OConnectionLineData& rConnLineData );

        OTableConnectionData& operator=( const OTableConnectionData& rConnData );
    public:
        OTableConnectionData();
        OTableConnectionData( const TTableWindowData::value_type& _pReferencingTable,
                              const TTableWindowData::value_type& _pReferencedTable,
                              const OUString& rConnName = OUString() );
        OTableConnectionData( const OTableConnectionData& rConnData );
        virtual ~OTableConnectionData();

        /// initialise from a source (more comfortable than a virtual assignment operator)
        virtual void CopyFrom(const OTableConnectionData& rSource);

        /** deliver a new instance of my own type

            derived classes have to deliver an instance of their own type

            @note does NOT have to be initialised
         */
        virtual OTableConnectionData* NewInstance() const;

        bool SetConnLine( sal_uInt16 nIndex, const OUString& rSourceFieldName, const OUString& rDestFieldName );
        bool AppendConnLine( const OUString& rSourceFieldName, const OUString& rDestFieldName );
        /** Deletes list of ConnLines
        */
        void ResetConnLines();

        /** moves the empty lines to the back
            removes duplicated empty lines

            caller is responsible for repainting them

            @return index of first changed line, or one-past-the-end if no change
         */
        OConnectionLineDataVec::size_type normalizeLines();

        const OConnectionLineDataVec& GetConnLineDataList() const { return m_vConnLineData; }
        OConnectionLineDataVec& GetConnLineDataList() { return m_vConnLineData; }

        inline TTableWindowData::value_type getReferencingTable() const { return m_pReferencingTable; }
        inline TTableWindowData::value_type getReferencedTable()  const { return m_pReferencedTable;  }

        inline void setReferencingTable(const TTableWindowData::value_type& _pTable) { m_pReferencingTable = _pTable; }
        inline void setReferencedTable(const TTableWindowData::value_type& _pTable)  { m_pReferencedTable  = _pTable; }

        OUString GetConnName() const { return m_aConnName; }

        void SetConnName( const OUString& rConnName ){ m_aConnName = rConnName; }
        /** Update create a new connection

            @return true if successful
        */
        virtual bool Update(){ return true; }
    };

    typedef ::std::vector< ::std::shared_ptr<OTableConnectionData> >  TTableConnectionData;

}
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
