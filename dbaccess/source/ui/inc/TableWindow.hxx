/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: TableWindow.hxx,v $
 *
 *  $Revision: 1.18 $
 *
 *  last change: $Author: hr $ $Date: 2007-11-01 15:19:13 $
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
#include <vector>
#ifndef _SV_WINDOW_HXX
#include <vcl/window.hxx>
#endif
#ifndef _UNOTOOLS_EVENTLISTENERADAPTER_HXX_
#include <unotools/eventlisteneradapter.hxx>
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
    class OTableWindowAccess;

    class OTableWindow : public Window
    {
        friend class OTableWindowTitle;
        friend class OTableWindowListBox;
    protected:
        // und die Tabelle selber (brauche ich, da ich sie locken will, solange das Fenster lebt)
        FixedImage              m_aTypeImage;
        OTableWindowTitle       m_aTitle;
        OTableWindowListBox*    m_pListBox;
        OTableWindowAccess*     m_pAccessible;

    private:
        TTableWindowData::value_type
                                m_pData;
        sal_Int32               m_nMoveCount;           // how often the arrow keys was pressed
        sal_Int32               m_nMoveIncrement;       // how many pixel we should move
        UINT16                  m_nSizingFlags;
        BOOL                    m_bActive;

        void Draw3DBorder( const Rectangle& rRect );

    protected:
        virtual void    Resize();
        virtual void    Paint( const Rectangle& rRect );
        virtual void    MouseMove( const MouseEvent& rEvt );
        virtual void    MouseButtonDown( const MouseEvent& rEvt );
        virtual void    DataChanged( const DataChangedEvent& rDCEvt );

        virtual OTableWindowListBox*    CreateListBox();
            // wird im ERSTEN Init aufgerufen
        BOOL FillListBox();
            // wird in JEDEM Init aufgerufen

        virtual void OnEntryDoubleClicked(SvLBoxEntry* /*pEntry*/) { }
            // wird aus dem DoubleClickHdl der ListBox heraus aufgerufen

        /** HandleKeyInput triues to handle the KeyEvent. Movement or deletion
            @param  rEvt
                The KEyEvent
            @return
                <TRUE/> when the table could handle the keyevent.
        */
        BOOL            HandleKeyInput( const KeyEvent& rEvt );

        /** delete the user data with the equal type as created within createUserData
            @param  _pUserData
                The user data store in the listbox entries. Created with a call to createUserData.
                _pUserData may be <NULL/>. _pUserData will be set to <NULL/> after call.
        */
        virtual void deleteUserData(void*& _pUserData);

        /** creates user information that will be append at the ListBoxentry
            @param  _xColumn
                The corresponding column, can be <NULL/>.
            @param  _bPrimaryKey
                <TRUE/> when the column belongs to the primary key
            @return
                the user data which will be append at the listbox entry, may be <NULL/>
        */
        virtual void* createUserData(const ::com::sun::star::uno::Reference<
                                    ::com::sun::star::beans::XPropertySet>& _xColumn,
                                    bool _bPrimaryKey);

        /** updates m_aTypeImage
        */
        void    impl_updateImage();

        OTableWindow( Window* pParent, const TTableWindowData::value_type& pTabWinData );

    public:
        virtual ~OTableWindow();

        // spaeter Constructor, siehe auch CreateListbox und FillListbox
        virtual BOOL Init();

        OJoinTableView*             getTableView();
        const OJoinTableView*       getTableView() const;
        OJoinDesignView*            getDesignView();
        void                        SetPosPixel( const Point& rNewPos );
        void                        SetSizePixel( const Size& rNewSize );
        void                        SetPosSizePixel( const Point& rNewPos, const Size& rNewSize );

        String                      getTitle() const;
        void                        SetBoldTitle( BOOL bBold );
        void                        setActive(sal_Bool _bActive = sal_True);

        void                        Remove();
        BOOL                        IsActiveWindow(){ return m_bActive; }

        ::rtl::OUString             GetTableName() const { return m_pData->GetTableName(); }
        ::rtl::OUString             GetWinName() const { return m_pData->GetWinName(); }
        ::rtl::OUString             GetComposedName() const { return m_pData->GetComposedName(); }
        OTableWindowListBox*        GetListBox() const { return m_pListBox; }
        TTableWindowData::value_type GetData() const { return m_pData; }
        OTableWindowTitle*          GetTitleCtrl() { return &m_aTitle; }

        /** returns the name which should be used when displaying join or relations
            @return
                The composed name or the window name.
        */
        virtual ::rtl::OUString     GetName() const = 0;

        inline ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess > GetOriginalColumns() const { return m_pData->getColumns(); }
        inline ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >    GetTable() const { return m_pData->getTable(); }

        UINT16                      GetSizingFlags() const { return m_nSizingFlags; }
        /** set the sizing flag to the direction
            @param  _rPos
                The EndPosition after resizing.
        */
        void                        setSizingFlag(const Point& _rPos);
        /** set the rsizing flag to NONE.
        */
        void                        resetSizingFlag() { m_nSizingFlags = SIZING_NONE; }

        /** returns the new sizing
        */
        Rectangle getSizingRect(const Point& _rPos,const Size& _rOutputSize) const;

        // window override
        virtual void                StateChanged( StateChangedType nStateChange );
        virtual void                GetFocus();
        virtual long                PreNotify( NotifyEvent& rNEvt );
        virtual void                Command(const CommandEvent& rEvt);

        // Accessibility
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible > CreateAccessible();

        // habe ich Connections nach aussen ?
        BOOL ExistsAConn() const;

        void EnumValidFields(::std::vector< ::rtl::OUString>& arrstrFields);

        /** clears the listbox inside. Must be called be the dtor is called.
        */
        void clearListBox();

    protected:
        using Window::SetPosSizePixel;
    };
}
#endif //DBAUI_TABLEWINDOW_HXX


