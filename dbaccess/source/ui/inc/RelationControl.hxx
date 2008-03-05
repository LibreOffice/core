/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: RelationControl.hxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: kz $ $Date: 2008-03-05 17:01:22 $
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
#ifndef DBAUI_RELATIONCONTROL_HXX
#define DBAUI_RELATIONCONTROL_HXX

#ifndef _FIXED_HXX //autogen
#include <vcl/fixed.hxx>
#endif
#ifndef _SV_LSTBOX_HXX //autogen
#include <vcl/lstbox.hxx>
#endif
#ifndef DBAUI_JOINTABLEVIEW_HXX
#include "JoinTableView.hxx"
#endif

namespace dbaui
{
    //========================================================================
    class OTableListBoxControl;
    class OTableConnectionData;
    class ORelationTableConnectionData;
    class IRelationControlInterface;
    class ORelationControl;

    class OTableListBoxControl  : public Window
    {
        FixedLine                               m_aFL_InvolvedTables;
        ListBox                                 m_lmbLeftTable,
                                                m_lmbRightTable;
        FixedLine                               m_aFL_InvolvedFields;

        ORelationControl*                       m_pRC_Tables;
        const OJoinTableView::OTableWindowMap*  m_pTableMap;
        IRelationControlInterface*              m_pParentDialog;
        String                                  m_strCurrentLeft;
        String                                  m_strCurrentRight;
    private:
        DECL_LINK( OnTableChanged, ListBox* );
    public:
        OTableListBoxControl(Window* _pParent,
                             const ResId& _rResId,
                             const OJoinTableView::OTableWindowMap* _pTableMap,
                             IRelationControlInterface* _pParentDialog);
        virtual ~OTableListBoxControl();

        /** fillListBoxes fills the list boxes with the table windows
        */
        void fillListBoxes();

        /** fillAndDisable fill the listboxes only with one entry and then disable them
            @param  _pConnectionData
                    contains the data which should be filled into the listboxes
        */
        void fillAndDisable(const TTableConnectionData::value_type& _pConnectionData);

        /** enables the relation control
        *
        * \param _bEnable when TRUE enables it, otherwise disable it.
        */
        void enableRelation(bool _bEnable);

        /** NotifyCellChange notifies the browse control that the conenction data has changed
        */
        void NotifyCellChange();

        /** Init is a call through to the control inside this one
            @param  _pConnData
                    the connection data which is used to init the control
        */
        void Init(const TTableConnectionData::value_type& _pConnData);
        void lateUIInit(Window* _pTableSeparator = NULL);
        void lateInit();

        BOOL SaveModified();

        TTableWindowData::value_type getReferencingTable()  const;

        /** getContainer returns the container interface
            @return the interface of the container
        */
        IRelationControlInterface* getContainer() const { return m_pParentDialog; }
    };
}
#endif // DBAUI_RELATIONCONTROL_HXX
