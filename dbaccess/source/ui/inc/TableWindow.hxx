/*************************************************************************
 *
 *  $RCSfile: TableWindow.hxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: oj $ $Date: 2001-02-28 10:07:01 $
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
#ifndef DBAUI_TABLEWINDOW_HXX
#define DBAUI_TABLEWINDOW_HXX

#ifndef _COM_SUN_STAR_CONTAINER_XNAMEACCESS_HPP_
#include <com/sun/star/container/XNameAccess.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSET_HPP_
#include <com/sun/star/beans/XPropertySet.hpp>
#endif
#ifndef DBAUI_TABLEWINDOWTITLE_HXX
#include "TableWindowTitle.hxx"
#endif
#ifndef _RTTI_HXX
#include <tools/rtti.hxx>
#endif
#ifndef DBAUI_TABLEWINDOWDATA_HXX
#include "TableWindowData.hxx"
#endif
#ifndef _VECTOR_
#include <vector>
#endif
#ifndef _SV_WINDOW_HXX
#include <vcl/window.hxx>
#endif

class SvLBoxEntry;
namespace dbaui
{
    //////////////////////////////////////////////////////////////////////////
    // Flags fuer die Groessenanpassung der SbaJoinTabWins
    const UINT16 SIZING_NONE    = 0x0000;
    const UINT16 SIZING_TOP     = 0x0001;
    const UINT16 SIZING_BOTTOM  = 0x0002;
    const UINT16 SIZING_LEFT    = 0x0004;
    const UINT16 SIZING_RIGHT   = 0x0008;

    class OTableWindowListBox;
    class OJoinDesignView;
    class OJoinTableView;

    class OTableWindow : public Window
    {
        friend class OTableWindowTitle;
        friend class OTableWindowListBox;
    protected:
        // und die Tabelle selber (brauche ich, da ich sie locken will, solange das Fenster lebt)
        OTableWindowTitle       m_aTitle;
        OTableWindowListBox*    m_pListBox;
    private:
        // the columns of the table
        ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet>    m_xTable;
        ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess> m_xColumns;

        OTableWindowData*       m_pData;
        ::rtl::OUString         m_strInitialWinName;
        UINT16                  m_nSizingFlags;
        BOOL                    m_bActive;

        void Draw3DBorder( const Rectangle& rRect );

    protected:
        virtual void    Resize();
        virtual void    Paint( const Rectangle& rRect );
        virtual void    MouseMove( const MouseEvent& rEvt );
        virtual void    MouseButtonDown( const MouseEvent& rEvt );
        virtual void    KeyInput( const KeyEvent& rEvt );

        virtual void    DataChanged( const DataChangedEvent& rDCEvt );

        virtual OTableWindowListBox*    CreateListBox();
            // wird im ERSTEN Init aufgerufen
        virtual BOOL FillListBox();
            // wird in JEDEM Init aufgerufen

        virtual void EmptyListBox();
            // Liste wird geleert, damit haben abgeleitete Klassen die Moeglichkeit, eventuell gesetzte UserData zu loeschen

        virtual void OnEntryDoubleClicked(SvLBoxEntry* pEntry) { }
            // wird aus dem DoubleClickHdl der ListBox heraus aufgerufen

        // die unterliegende Tabellendefinition (un)locken
        //  void LockTable() { if (m_xTableDef.Is()) m_xTableDef->AddUsage(); }
        //  void UnlockTable() { if (m_xTableDef.Is()) m_xTableDef->ReleaseUsage(); }

    public:
        TYPEINFO();
        OTableWindow( Window* pParent, OTableWindowData* pTabWinData);
        virtual ~OTableWindow();

        // spaeter Constructor, siehe auch CreateListbox und FillListbox
        virtual BOOL Init();

        OJoinTableView*             getTableView();
        const OJoinTableView*       getTableView() const;
        OJoinDesignView*            getDesignView();
        void                        SetPosPixel( const Point& rNewPos );
        void                        SetSizePixel( const Size& rNewSize );
        void                        SetPosSizePixel( const Point& rNewPos, const Size& rNewSize );

        void                        SetTitle( const ::rtl::OUString& rTit );
        void                        SetBoldTitle( BOOL bBold );

        void                        Remove();
        BOOL                        IsActive(){ return m_bActive; }

        ::rtl::OUString             GetTableName() const { return m_pData->GetTableName(); }
        ::rtl::OUString             GetWinName() const { return m_pData->GetWinName(); }
        ::rtl::OUString             GetComposedName() const { return m_pData->GetComposedName(); }
        OTableWindowListBox*        GetListBox() const { return m_pListBox; }
        OTableWindowData*           GetData() const { return m_pData; }
        ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess> GetOriginalColumns() const { return m_xColumns; }
        ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet>    GetTable() const { return m_xTable; }

        UINT16                      GetSizingFlags() const { return m_nSizingFlags; }

        // window override
        virtual void                StateChanged( StateChangedType nStateChange );
        virtual void                GetFocus();
        virtual void                LoseFocus();

        // Linien neu zeichnen
        void InvalidateLines();
        // habe ich Connections nach aussen ?
        BOOL ExistsAConn() const;

        virtual void EnumValidFields(::std::vector< ::rtl::OUString>& arrstrFields);
    };
}
#endif //DBAUI_TABLEWINDOW_HXX


