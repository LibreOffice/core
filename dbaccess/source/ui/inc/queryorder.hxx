/*************************************************************************
 *
 *  $RCSfile: queryorder.hxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: oj $ $Date: 2000-11-10 16:10:13 $
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
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
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
#ifndef DBAUI_QUERYORDER_HXX
#define DBAUI_QUERYORDER_HXX

#ifndef _DIALOG_HXX //autogen
#include <vcl/dialog.hxx>
#endif

#ifndef _LSTBOX_HXX //autogen
#include <vcl/lstbox.hxx>
#endif

#ifndef _EDIT_HXX //autogen
#include <vcl/edit.hxx>
#endif

#ifndef _FIXED_HXX //autogen
#include <vcl/fixed.hxx>
#endif

#ifndef _GROUP_HXX //autogen
#include <vcl/group.hxx>
#endif

#ifndef _BUTTON_HXX //autogen
#include <vcl/button.hxx>
#endif

#define DOG_ROWS    3

namespace rtl
{
    class OUString;
}
namespace com
{
    namespace sun
    {
        namespace star
        {
            namespace sdb
            {
                class XSQLQueryComposer;
            }
            namespace sdbc
            {
                class XConnection;
            }
            namespace container
            {
                class XNameAccess;
            }
            namespace beans
            {
                struct PropertyValue;
            }
        }
    }
}

//==================================================================
// DlgOrderCrit
//==================================================================
namespace dbaui
{
    class DlgOrderCrit : public ModalDialog
    {
    protected:
        ListBox         aLB_ORDERFIELD1;
        ListBox         aLB_ORDERVALUE1;
        ListBox         aLB_ORDERFIELD2;
        ListBox         aLB_ORDERVALUE2;
        ListBox         aLB_ORDERFIELD3;
        ListBox         aLB_ORDERVALUE3;
        FixedText       aFT_ORDERFIELD;
        FixedText       aFT_ORDERVALUE;
        FixedText       aFT_ORDERAFTER1;
        FixedText       aFT_ORDERAFTER2;
        FixedText       aFT_ORDEROPER;
        FixedText       aFT_ORDERDIR;
        OKButton        aBT_OK;
        CancelButton    aBT_CANCEL;
        HelpButton      aBT_HELP;
        GroupBox        aGB_ORDER;
        String          aSTR_NOENTRY;

        ::com::sun::star::uno::Reference< ::com::sun::star::sdb::XSQLQueryComposer> m_xQueryComposer;
        ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess> m_xColumns;
        ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection>      m_xConnection;


        ListBox*        arrLbFields[DOG_ROWS];
        ListBox*        arrLbValues[DOG_ROWS];

        DECL_LINK( FieldListSelectHdl, ListBox * );
        void            EnableLines();

    public:
        DlgOrderCrit(   Window * pParent,
                        const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection>& _rxConnection,
                        const ::com::sun::star::uno::Reference< ::com::sun::star::sdb::XSQLQueryComposer>& _rxQueryComposer,
                        const ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess>& _rxCols);

                        ~DlgOrderCrit();
        void            BuildOrderPart();

        String          GetOrderList( );
        void            SetOrderList( const String& _rOrderList );
    };
}
#endif // DBAUI_QUERYORDER_HXX


