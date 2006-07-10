/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: adtabdlg.hxx,v $
 *
 *  $Revision: 1.8 $
 *
 *  last change: $Author: obo $ $Date: 2006-07-10 15:30:01 $
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
        virtual BOOL Close();

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
