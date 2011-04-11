/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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
#ifndef DBAUI_QUERY_TABLEWINDOW_HXX
#define DBAUI_QUERY_TABLEWINDOW_HXX

#include "TableWindow.hxx"
#include "QTableWindowData.hxx"
#include "TableFieldDescription.hxx"
#include <tools/rtti.hxx>

namespace dbaui
{
    //==================================================================
    class OQueryTableWindow : public OTableWindow
    {
        sal_Int32           m_nAliasNum;
        ::rtl::OUString     m_strInitialAlias;
    public:
        OQueryTableWindow( Window* pParent, const TTableWindowData::value_type& pTabWinData, sal_Unicode* pszInitialAlias = NULL );
        virtual ~OQueryTableWindow();

        ::rtl::OUString GetAliasName() const
        {
            return static_cast<OQueryTableWindowData*>(GetData().get())->GetAliasName();
        }
        void            SetAliasName(const ::rtl::OUString& strNewAlias)
        {
            static_cast<OQueryTableWindowData*>(GetData().get())->SetAliasName(strNewAlias);
        }

        // spaeter Constructor, die Basisklasse ERZEUGT beim ersten Aufruf die Listbox
        virtual sal_Bool    Init();

        inline sal_Int32        GetAliasNum() const { return m_nAliasNum; }

        sal_Bool            ExistsField(const ::rtl::OUString& strFieldName, OTableFieldDescRef& rInfo);
        sal_Bool            ExistsAVisitedConn() const;

        virtual ::rtl::OUString     GetName() const { return GetWinName(); }

    protected:
        virtual void    KeyInput( const KeyEvent& rEvt );

        virtual void    OnEntryDoubleClicked(SvLBoxEntry* pEntry);
            // wird aus dem DoubleClickHdl der ListBox heraus aufgerufen
        /** delete the user data with the equal type as created within createUserData
            @param  _pUserData
                The user data store in the listbox entries. Created with a call to createUserData.
                _pUserData may be <NULL/>.
        */
        virtual void deleteUserData(void*& _pUserData);

        /** creates user information that will be append at the ListBoxentry
            @param  _xColumn
                The corresponding column, can be <NULL/>.
            @param  _bPrimaryKey
                <TRUE/> when the column belongs to the primary key
            @return
                the user data which will be append at the listbox entry, may be <NULL/>
        */
        virtual void* createUserData(const ::com::sun::star::uno::Reference<
                                    ::com::sun::star::beans::XPropertySet>& _xColumn,
                                    bool _bPrimaryKey);
    };
}
#endif // DBAUI_QUERY_TABLEWINDOW_HXX


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
