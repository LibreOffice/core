/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/


#ifndef DBAUI_QYDLGTAB_HXX
#define DBAUI_QYDLGTAB_HXX

#ifndef _DIALOG_HXX //autogen
#include <vcl/dialog.hxx>
#endif

#ifndef _BUTTON_HXX //autogen
#include <vcl/button.hxx>
#endif

#ifndef _FIXED_HXX //autogen
#include <vcl/fixed.hxx>
#endif

#ifndef _COM_SUN_STAR_SDBC_XCONNECTION_HPP_
#include <com/sun/star/sdbc/XConnection.hpp>
#endif

#ifndef _LSTBOX_HXX //autogen
#include <vcl/lstbox.hxx>
#endif
#ifndef _SV_DIALOG_HXX
#include <vcl/dialog.hxx>
#endif
#ifndef _STRING_HXX //autogen
#include <tools/string.hxx>
#endif

#ifndef _DBAUI_TABLETREE_HXX_
#include "tabletree.hxx"
#endif

#include <memory>

namespace dbaui
{
    //========================================================================
    /** unifies the access to a list of table/query objects
    */
    class TableObjectListFacade
    {
    public:
        virtual void    updateTableObjectList( bool _bAllowViews ) = 0;
        virtual String  getSelectedName( String& _out_rAliasName ) const = 0;
        virtual bool    isLeafSelected() const = 0;

        virtual ~TableObjectListFacade();
    };

    //========================================================================
    class IAddTableDialogContext
    {
    public:
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection >
                        getConnection() const = 0;
        virtual bool    allowViews() const = 0;
        virtual bool    allowQueries() const = 0;
        virtual bool    allowAddition() const = 0;
        virtual void    addTableWindow( const String& _rQualifiedTableName, const String& _rAliasName ) = 0;
        virtual void    onWindowClosing( const Window* _pWindow ) = 0;
    };

    //========================================================================
    class OAddTableDlg : public ModelessDialog
    {
        RadioButton         m_aCaseTables;
        RadioButton         m_aCaseQueries;

        OTableTreeListBox   m_aTableList;
        SvTreeListBox       m_aQueryList;
        ::std::auto_ptr< TableObjectListFacade >
                            m_pCurrentList;

        PushButton          aAddButton;
        CancelButton        aCloseButton;
        HelpButton          aHelpButton;

        IAddTableDialogContext&
                            m_rContext;

        DECL_LINK( AddClickHdl, Button* );
        DECL_LINK( CloseClickHdl, Button* );
        DECL_LINK( TableListDoubleClickHdl, void* );
        DECL_LINK( TableListSelectHdl, void* );
        DECL_LINK( OnTypeSelected, void* );

    public:
        OAddTableDlg(
            Window* _pParent,
            IAddTableDialogContext& _rContext );
        virtual ~OAddTableDlg();

        void DetermineAddTable() { aAddButton.Enable( impl_isAddAllowed() ); }
        void Update();

        static  String  getDialogTitleForContext(
            IAddTableDialogContext& _rContext );

    private:
        virtual sal_Bool Close();

        bool impl_isAddAllowed();
        void impl_addTable();

        enum ObjectList
        {
            Tables,
            Queries
        };
        void impl_switchTo( ObjectList _eList );
    };
}
#endif // DBAUI_QYDLGTAB_HXX
