/*************************************************************************
 *
 *  $RCSfile: AppDetailPageHelper.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: hr $ $Date: 2004-08-02 15:28:58 $
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
 *  Contributor(s): Ocke Janssen
 *
 *
 ************************************************************************/
#ifndef DBAUI_APPDETAILPAGEHELPER_HXX
#define DBAUI_APPDETAILPAGEHELPER_HXX

#include <vector>

#ifndef _RTL_USTRING_HXX_
#include <rtl/ustring.hxx>
#endif
#ifndef _COM_SUN_STAR_SDBC_XDATABASEMETADATA_HPP_
#include <com/sun/star/sdbc/XDatabaseMetaData.hpp>
#endif
#ifndef _COM_SUN_STAR_UCB_XCONTENT_HPP_
#include <com/sun/star/ucb/XContent.hpp>
#endif
#ifndef DBAUI_APPELEMENTTYPE_HXX
#include "AppElementType.hxx"
#endif
#ifndef _SVTREEBOX_HXX
#include <svtools/svtreebx.hxx>
#endif
#ifndef SVTOOLS_DOCUMENTINFOPREVIEW_HXX
#include <svtools/DocumentInfoPreview.hxx>
#endif
#ifndef _SV_FIXED_HXX
#include <vcl/fixed.hxx>
#endif
#ifndef _SV_TOOLBOX_HXX
#include <vcl/toolbox.hxx>
#endif
#ifndef _SV_CVTGRF_HXX
#include <vcl/cvtgrf.hxx>
#endif
#ifndef _SV_GRAPH_HXX
#include <vcl/graph.hxx>
#endif
#ifndef _GRFMGR_HXX
#include <goodies/grfmgr.hxx>
#endif
#include <memory>

namespace com{ namespace sun { namespace star { namespace awt   { class XWindow; } } } };
namespace com{ namespace sun { namespace star { namespace frame { class XFrame; } } } };
namespace com{ namespace sun { namespace star { namespace io    { class XPersist; } } } };

#define CONTROL_COUNT   4

namespace dbaui
{
    class OAppBorderWindow;
    class DBTreeListBox;

    class OPreviewWindow : public Window
    {
        GraphicObject       m_aGraphicObj;
        Rectangle           m_aPreviewRect;

        /** gets the graphic cnter rect
            @param  rGraphic
                the graphic
            @param  rResultRect
                the resulting rectangle

            @return
                <TRUE/> when successfull
        */
        BOOL ImplGetGraphicCenterRect( const Graphic& rGraphic, Rectangle& rResultRect ) const;
    public:
        OPreviewWindow(Window* _pParent) : Window(_pParent){}

        // window overloads
        virtual void Paint(const Rectangle& rRect);

        void setGraphic(const Graphic& _rGraphic ) { m_aGraphicObj.SetGraphic(_rGraphic); }
    };
    //==================================================================
    // A helper class for the controls in the detail page.
    // Combines general functionality.
    //==================================================================
    class OAppDetailPageHelper : public Window
    {
        DBTreeListBox*      m_pLists[CONTROL_COUNT];
        OAppBorderWindow*   m_pBorderWin;
        FixedLine           m_aFL;
        ToolBox             m_aTBPreview;
        Window              m_aBorder;
        OPreviewWindow      m_aPreview;
        ::svtools::ODocumentInfoPreview
                            m_aDocumentInfo;
        Window*             m_pTablePreview;
        Timer               m_aPreviewTimer;
        ::std::auto_ptr<PopupMenu> m_aMenu;
        PreviewMode         m_ePreviewMode;
        ::com::sun::star::uno::Reference < ::com::sun::star::frame::XFrame >
                            m_xFrame;
        ::com::sun::star::uno::Reference < ::com::sun::star::io::XPersist >
                            m_xDocInfo;
        ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindow >
                            m_xWindow;

        /// returns the index of the visible control
        int getVisibleControlIndex() const;

        /** sorts the entries in the tree list box.
            @param  _nPos
                Which list should be sorted.
            @param  _eSortMode
                How should be sorted.
        */
        void sort(int _nPos,SvSortMode _eSortMode );

        /** fills the names in the listbox
            @param  _xContainer
                This can either be the queries, forms or report names.
            @param  _rList
                The tree list box to fill
            @param  _pParent
                The parent of the entries to be inserted.
        */
        void fillNames(  const ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess >& _xContainer
                        ,DBTreeListBox& _rList
                        ,USHORT _nImageId
                        ,SvLBoxEntry* _pParent = NULL);
        /** sets the detail page
            @param  _pWindow
                The control which should be visible.
        */
        void setDetailPage(Window* _pWindow);

        /** sets all HandleCallbacks
            @param  _pTreeView
                The newly created DBTreeListBox
            @param  _nCollapsedBitmap
                The collapsed bitmap resource id
            @param  _nCollapsedBitmap_HI
                The collapsed bitmap resource id for high contrast.
            @return
                The new tree.
        */
        DBTreeListBox* createTree(DBTreeListBox* _pTreeView,USHORT _nCollapsedBitmap,USHORT _nCollapsedBitmap_HI);

        /** creates the tree and sets all HandleCallbacks
            @param  _nHelpId
                The help id of the control
            @param  _nCollapsedBitmap
                The collapsed bitmap resource id
            @param  _nCollapsedBitmap_HI
                The collapsed bitmap resource id for high contrast.
            @return
                The new tree.
        */
        DBTreeListBox* createSimpleTree(ULONG _nHelpId, USHORT _nCollapsedBitmap,USHORT _nCollapsedBitmap_HI);

        DECL_LINK( OnEntryDoubleClick,          SvTreeListBox* );
        DECL_LINK( OnDeSelectHdl,               SvTreeListBox* );

        DECL_LINK( OnEntrySelectHdl,            SvLBoxEntry* );

        DECL_LINK( OnCutEntry,                  SvLBoxEntry* );
        DECL_LINK( OnCopyEntry,                 SvLBoxEntry* );
        DECL_LINK( OnPasteEntry,                SvLBoxEntry* );
        DECL_LINK( OnDeleteEntry,               SvLBoxEntry* );

        DECL_LINK(PreviewChangeHdl, void*);
        // click a TB slot
        DECL_LINK(OnToolBoxSelected, ToolBox*);
        DECL_LINK(OnToolBoxClicked, ToolBox*);

        inline OAppBorderWindow* getBorderWin() const { return m_pBorderWin; }

    public:
        OAppDetailPageHelper(Window* _pParent,OAppBorderWindow* _pBorderWin);
        virtual ~OAppDetailPageHelper();

        // window overloads
        virtual void Resize();
        virtual void KeyInput( const KeyEvent& rKEvt );

        sal_Bool isCutAllowed();
        sal_Bool isCopyAllowed();
        sal_Bool isPasteAllowed();
        void copy();
        void cut();
        void paste();

        /** creates the tables page
            @param  _xConnection
                The connection to get the table names
        */
        void createTablesPage(const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection>& _xConnection);

        /** creates the page for the specific type.
            @param  _eType
                The type which should be created. E_TABLE isn't allowed.
            @param  _xContainer
                The container of the elements to be inserted.
        */
        void createPage(ElementType _eType,const ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess >& _xContainer);

        /** returns the current visible tree list box
        */
        inline DBTreeListBox* getCurrentView() const
        {
            ElementType eType = getElementType();
            return (eType != E_NONE ) ? m_pLists[static_cast<sal_Int32>(eType)] : NULL;
        }

        /// select all entries in the visible control
        void selectAll();

        /// returns <TRUE/> if it sorts ascending
        sal_Bool isSortUp() const;

        /// sorts all entries ascending
        void sortDown();

        /// sorts all entries descending
        void sortUp();

        /** returns the element names which are selected
            @param  _rNames
                The list will be filled.
            @param  _xMetaData
                Will be used when the table list should be filled.
        */
        void getSelectionElementNames(::std::vector< ::rtl::OUString>& _rNames
            ,const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XDatabaseMetaData>& _xMetaData = NULL) const;

        /** return the qualified name.
            @param  _pEntry
                The entry of a table, or query, form, report to get the qualified name.
                If the entry is <NULL/>, the first selected is chosen.
            @param  _xMetaData
                The meta data are used to create the table name, otherwise this may also be <NULL/>
            @return
                the qualified name
        */
        ::rtl::OUString getQualifiedName(SvLBoxEntry* _pEntry
                                        ,const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XDatabaseMetaData>& _xMetaData) const;

        /// return the element of currently select entry
        ElementType getElementType() const;

        /// returns the count of selected entries
        sal_Int32 getSelectionCount();

        /// returns the count of entries
        sal_Int32 getElementCount();

        /** returns if an entry is a leaf
            @param _pEntry
                The entry to check
            @return
                <TRUE/> if the entry is a leaf, otherwise <FALSE/>
        */
        sal_Bool isLeaf(SvLBoxEntry* _pEntry) const;

        /** returns if one of the selected entries is a leaf
            @return
                <TRUE/> if the entry is a leaf, otherwise <FALSE/>
        */
        sal_Bool isALeafSelected() const;

        SvLBoxEntry* getEntry( const Point& _aPosPixel ) const;

        /// clears the detail pages
        void clearPages();

        /// returns <TRUE/> when a detail page was filled
        sal_Bool isFilled() const;

        /** adds a new object to the detail page.
            @param  _eType
                The type where the entry shold be appended.
            @param  _rName
                The name of the object to be inserted
            @param  _rObject
                The object to add.
            @param  _rxConn
                If we insert a table, the connection must be set.
        */
        SvLBoxEntry*  elementAdded(ElementType eType
                        ,const ::rtl::OUString& _rName
                        ,const ::com::sun::star::uno::Any& _rObject
                        ,const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection >& _rxConn = NULL);

        /** replaces a objects name with a new one
            @param  _eType
                The type where the entry shold be appended.
            @param  _rOldName
                The old name of the object to be replaced
            @param  _rNewName
                The new name of the object to be replaced
            @param  _rxConn
                If we insert a table, the connection must be set.
        */
        void elementReplaced(ElementType eType
                        ,const ::rtl::OUString& _rOldName
                        ,const ::rtl::OUString& _rNewName
                        ,const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection >& _rxConn = NULL);

        /** removes an element from the detail page.
            @param  _rName
                The name of the element to be removed.
            @param  _rxConn
                If we remove a table, the connection must be set.
        */
        void elementRemoved(const ::rtl::OUString& _rName
                            ,const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection >& _rxConn);


        /// returns the preview mode
        PreviewMode getPreviewMode();

        /// <TRUE/> if the preview is enabled
        sal_Bool isPreviewEnabled();

        /// switches the current preview
        void switchPreview();

        /** switches to the given preview mode
            @param  _eMode
                the mode to set for the preview
            @param  _bForce
                Force the preview to be resetted
        */
        void switchPreview(PreviewMode _eMode,BOOL _bForce = FALSE);

        /** shows the Preview of the content when it is enabled.
            @param  _xContent
                The content which must support the "preview" command.
        */
        void showPreview(const ::com::sun::star::uno::Reference< ::com::sun::star::ucb::XContent >& _xContent);

        /** shows the Preview of a table or query
            @param  _sDataSourceName
                the name of the data source
            @param  _xConnection
                the connection which will be shared
            @param  _sName
                the name of table or query
            @param  _bTable
                <TRUE/> if it is a table, otherwise <FALSE/>
            @return void
        */
        void showPreview(   const ::rtl::OUString& _sDataSourceName,
                            const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection>& _xConnection,
                            const ::rtl::OUString& _sName,
                            sal_Bool _bTable);

        /** disable the controls
            @param  _bDisable
                if <TRUE/> then the controls will be disabled otherwise they will be enabled.
        */
        void disableControls(sal_Bool _bDisable);
    };
}
#endif // DBAUI_APPDETAILPAGEHELPER_HXX

