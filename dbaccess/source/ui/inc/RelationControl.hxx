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
