/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: QTableWindow.hxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: obo $ $Date: 2006-07-10 15:40:58 $
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
#ifndef DBAUI_QUERY_TABLEWINDOW_HXX
#define DBAUI_QUERY_TABLEWINDOW_HXX

#ifndef DBAUI_TABLEWINDOW_HXX
#include "TableWindow.hxx"
#endif
#ifndef DBAUI_QUERY_TABLEWINDOWDATA_HXX
#include "QTableWindowData.hxx"
#endif
#ifndef DBAUI_TABLEFIELDDESC_HXX
#include "TableFieldDescription.hxx"
#endif
#ifndef _RTTI_HXX
#include <tools/rtti.hxx>
#endif

namespace dbaui
{
    //==================================================================
    class OQueryTableWindow : public OTableWindow
    {
        sal_Int32           m_nAliasNum;
        ::rtl::OUString     m_strInitialAlias;
    public:
        TYPEINFO();
        OQueryTableWindow( Window* pParent, OQueryTableWindowData* pTabWinData, sal_Unicode* pszInitialAlias = NULL );
        virtual ~OQueryTableWindow();

        ::rtl::OUString GetAliasName() const
        {
            return static_cast<OQueryTableWindowData*>(GetData())->GetAliasName();
        }
        void            SetAliasName(const ::rtl::OUString& strNewAlias)
        {
            static_cast<OQueryTableWindowData*>(GetData())->SetAliasName(strNewAlias);
        }

        // spaeter Constructor, die Basisklasse ERZEUGT beim ersten Aufruf die Listbox
        virtual BOOL    Init();

        sal_Int32       GetAliasNum() const { return m_nAliasNum; }

        BOOL            ExistsField(const ::rtl::OUString& strFieldName, OTableFieldDescRef& rInfo);
        BOOL            ExistsAVisitedConn() const;

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

        virtual bool    allowQueries() const;
        virtual void    onNoColumns_throw();
    };
}
#endif // DBAUI_QUERY_TABLEWINDOW_HXX


