/*************************************************************************
 *
 *  $RCSfile: QTableWindow.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: oj $ $Date: 2001-03-23 09:46:40 $
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

        BOOL            ExistsField(const ::rtl::OUString& strFieldName, OTableFieldDesc& rInfo);
        BOOL            ExistsAVisitedConn() const;

    protected:
        virtual void    KeyInput( const KeyEvent& rEvt );

            // wird im ERSTEN Init aufgerufen
        virtual BOOL    FillListBox();
            // wird in JEDEM Init aufgerufen
        virtual void    EmptyListBox();
            // ListBox definiert leeren

        virtual void    OnEntryDoubleClicked(SvLBoxEntry* pEntry);
            // wird aus dem DoubleClickHdl der ListBox heraus aufgerufen
    };
}
#endif // DBAUI_QUERY_TABLEWINDOW_HXX


