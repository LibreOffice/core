/*************************************************************************
 *
 *  $RCSfile: RelationDlg.hxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: oj $ $Date: 2001-02-28 10:08:37 $
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
#ifndef DBAUI_RELATIONDIALOG_HXX
#define DBAUI_RELATIONDIALOG_HXX

#ifndef _DIALOG_HXX //autogen
#include <vcl/dialog.hxx>
#endif

#ifndef _GROUP_HXX //autogen
#include <vcl/group.hxx>
#endif

#ifndef _BUTTON_HXX //autogen
#include <vcl/button.hxx>
#endif

#ifndef _FIXED_HXX //autogen
#include <vcl/fixed.hxx>
#endif

#ifndef _EDIT_HXX //autogen
#include <vcl/edit.hxx>
#endif

#ifndef _SV_LSTBOX_HXX //autogen
#include <vcl/lstbox.hxx>
#endif

#ifndef _SVX_DBBROWSE_HXX
#include <svx/dbbrowse.hxx>
#endif // _SVX_DBBROWSE_HXX

#ifndef _SV_MSGBOX_HXX
#include <vcl/msgbox.hxx>
#endif
#ifndef DBAUI_RTABLECONNECTIONDATA_HXX
#include "RTableConnectionData.hxx"
#endif
#ifndef _COM_SUN_STAR_SDBC_XCONNECTION_HPP_
#include <com/sun/star/sdbc/XConnection.hpp>
#endif


namespace dbaui
{
    //========================================================================
    class ORelationDialog;
    class ORelationControl : public DbBrowseBox
    {
        friend class ORelationDialog;

        ULONG                                                                           m_nDeActivateEvent;
        DbListBoxCtrl*                                                                  m_pListCell;
        ORelationTableConnectionData*                                                   m_pConnData;
        long                                                                            m_nDataPos;
        ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet>        m_xSourceDef;
        ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet>        m_xDestDef;

        void SetDef(const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet>& xDest,sal_Int32 _nPos);
    public:
        ORelationControl( ORelationDialog* pParent );
        virtual ~ORelationControl();

        void SetSourceDef(const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet>& _xNewSource);
        void SetDestDef(const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet>& _xNewDest);

    protected:
        virtual void Resize();

        virtual long PreNotify(NotifyEvent& rNEvt );

        virtual BOOL IsTabAllowed(BOOL bForward) const;

        virtual void Init() { DbBrowseBox::Init(); }    // late construction
        virtual void Init(ORelationTableConnectionData* _pConnData);
        virtual void InitController( DbCellControllerRef& rController, long nRow, USHORT nCol );
        virtual DbCellController* GetController( long nRow, USHORT nCol );
        virtual void PaintCell( OutputDevice& rDev, const Rectangle& rRect, USHORT nColId ) const;
        virtual BOOL SeekRow( long nRow );
        virtual BOOL SaveModified();
        virtual String GetCellText( long nRow, USHORT nColId );

        virtual void CellModified();

    private:

        DECL_LINK( AsynchActivate, void* );
        DECL_LINK( AsynchDeactivate, void* );
    };

    class OJoinTableView;
    //========================================================================
    class ORelationDialog : public ModalDialog
    {
        GroupBox    aGB_InvolvedTables;
        ListBox     m_lmbLeftTable,
                    m_lmbRightTable;

        GroupBox    aGB_InvolvedFields;

        GroupBox    aGB_CascUpd;
        RadioButton aRB_NoCascUpd,
                    aRB_CascUpd,
                    aRB_CascUpdNull,
                    aRB_CascUpdDefault;
        GroupBox    aGB_CascDel;
        RadioButton aRB_NoCascDel,
                    aRB_CascDel,
                    aRB_CascDelNull,
                    aRB_CascDelDefault;

        OKButton    aPB_OK;
        CancelButton aPB_CANCEL;
        HelpButton  aPB_HELP;

        ORelationControl*                                                       m_pRC_Tables;
        ORelationTableConnectionData*                                           m_pConnData;
        ORelationTableConnectionData*                                           m_pOrigConnData;
        ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection > m_xConnection;

        String                                                                  m_strCurrentLeft;
        String                                                                  m_strCurrentRight;
        BOOL                                                                    m_bTriedOneUpdate;

    public:
        ORelationDialog(OJoinTableView* pParent,
                        ORelationTableConnectionData* pConnectionData,
                        BOOL bAllowTableSelect = FALSE );
        virtual ~ORelationDialog();

        ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection > getConnection(){ return m_xConnection; }

        void NotifyCellChange();

        virtual short Execute();

    protected:
        void Init(ORelationTableConnectionData* _pConnData);

    private:
        DECL_LINK( OKClickHdl, Button* );
        DECL_LINK( OnTableChanged, ListBox* );
    };
}
#endif // DBAUI_RELATIONDIALOG_HXX


