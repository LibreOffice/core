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
        * \param _bEnable when sal_True enables it, otherwise disable it.
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

        sal_Bool SaveModified();

        TTableWindowData::value_type getReferencingTable()  const;

        /** getContainer returns the container interface
            @return the interface of the container
        */
        IRelationControlInterface* getContainer() const { return m_pParentDialog; }
    };
}
#endif // DBAUI_RELATIONCONTROL_HXX
