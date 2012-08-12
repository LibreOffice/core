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
#ifndef DBAUI_TABLECONNECTION_HXX
#define DBAUI_TABLECONNECTION_HXX

#include <vector>
#include <tools/debug.hxx>
#include <vcl/window.hxx>
#include <tools/rtti.hxx>
#include <com/sun/star/uno/Reference.h>
#include "TableConnectionData.hxx"

class Point;
class Rectangle;

namespace dbaui
{
    class OTableConnectionData;
    class OTableWindow;
    class OJoinTableView;
    class OConnectionLine;

    DBG_NAMEEX(OTableConnection)
    class OTableConnection : public Window
    {
        ::std::vector<OConnectionLine*> m_vConnLine;
        TTableConnectionData::value_type
                                        m_pData;
        OJoinTableView*                 m_pParent;

        sal_Bool                            m_bSelected;

        void Init();
        /** loops through the vector and deletes all lines */
        void clearLineData();

    protected:
        OConnectionLine* CreateConnLine( const OConnectionLine& rConnLine );

    public:
        OTableConnection( OJoinTableView* pContainer, const TTableConnectionData::value_type& pTabConnData );
        OTableConnection( const OTableConnection& rConn );
        /** destructor

            @attention Normally a pointer to OTableConnectionData is given but
                       here, however, one has to create an instance (with
                       OTableConnectionDate::NewInstance) which is never deleted
                       (same like in other cases). Thus, the caller is
                       responsible to check and save the data for deleting it
                       eventually.
         */
        virtual ~OTableConnection();

        OTableConnection& operator=( const OTableConnection& rConn );


        void        Select();
        void        Deselect();
        sal_Bool        IsSelected() const { return m_bSelected; }
        sal_Bool        CheckHit( const Point& rMousePos ) const;
        bool        InvalidateConnection();
        void        UpdateLineList();

        OTableWindow* GetSourceWin() const;
        OTableWindow* GetDestWin() const;

        bool RecalcLines();
        /** isTableConnection

            @param  _pTable the table where we should check if we belongs to it
            @return true when the source or the destination window are equal
        */
        bool isTableConnection(const OTableWindow* _pTable)
        {
            return (_pTable == GetSourceWin() || _pTable == GetDestWin());
        }

        Rectangle   GetBoundingRect() const;

        inline TTableConnectionData::value_type GetData() const { return m_pData; }
        const ::std::vector<OConnectionLine*>*  GetConnLineList() const { return &m_vConnLine; }
        inline OJoinTableView*                  GetParent() const { return m_pParent; }
        virtual void                    Draw( const Rectangle& rRect );
        using Window::Draw;
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible > CreateAccessible();
    };
}
#endif // DBAUI_TABLECONNECTION_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
