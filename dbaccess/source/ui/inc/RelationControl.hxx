/*************************************************************************
 *
 *  $RCSfile: RelationControl.hxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: oj $ $Date: 2002-02-06 07:13:01 $
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
            @param _pConnectionData contains the data which should be filled into the listboxes
        */
        void fillAndDisable(OTableConnectionData* _pConnectionData);

        /** NotifyCellChange notifies the browse control that the conenction data has changed
        */
        void NotifyCellChange();

        /** Init is a call through to the control inside this one
            @param  _pConnData  the connection data which is used to init the control
        */
        void Init(OTableConnectionData* _pConnData);

        BOOL SaveModified();

        String getSourceWinName()   const;
        String getDestWinName()     const;

        /** getContainer returns the container interface
            @return the interface of the container
        */
        IRelationControlInterface* getContainer() const { return m_pParentDialog; }
    };
}
#endif // DBAUI_RELATIONCONTROL_HXX
