/*************************************************************************
 *
 *  $RCSfile: AppDetailView.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: hr $ $Date: 2004-08-02 15:29:17 $
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
#ifndef DBAUI_APPDETAILVIEW_HXX
#define DBAUI_APPDETAILVIEW_HXX

#ifndef _COM_SUN_STAR_FRAME_XCONTROLLER_HPP_
#include <com/sun/star/frame/XController.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XNAMEACCESS_HPP_
#include <com/sun/star/container/XNameAccess.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_XCONNECTION_HPP_
#include <com/sun/star/sdbc/XConnection.hpp>
#endif
#ifndef _COM_SUN_STAR_UCB_XCONTENT_HPP_
#include <com/sun/star/ucb/XContent.hpp>
#endif
#ifndef _SV_SPLIT_HXX
#include <vcl/split.hxx>
#endif
#ifndef _SV_FIXED_HXX
#include <vcl/fixed.hxx>
#endif
#ifndef DBACCESS_TABLEDESIGN_ICLIPBOARDTEST_HXX
#include "IClipBoardTest.hxx"
#endif
#ifndef DBAUI_TITLE_WINDOW_HXX
#include "AppTitleWindow.hxx"
#endif
#ifndef DBAUI_APPELEMENTTYPE_HXX
#include "AppElementType.hxx"
#endif
#ifndef _SVTREEBOX_HXX
#include <svtools/svtreebx.hxx>
#endif
#ifndef DBAUI_VERTSPLITVIEW_HXX
#include "VertSplitView.hxx"
#endif

#include <vector>

class SvLBoxEntry;
namespace dbaui
{
    class OApplicationController;
    class OAppBorderWindow;
    class OApplicationDetailView;
    class OAppDetailPageHelper;
    class OTasksWindow;

    class OCreationList : public SvTreeListBox
    {
        OTasksWindow* m_pTaskWindow;
    public:
        OCreationList(OTasksWindow* _pParent);
        // window overloads
        virtual void MouseMove( const MouseEvent& rMEvt );
        virtual void MouseButtonDown( const MouseEvent& rMEvt );
        virtual void KeyInput( const KeyEvent& rKEvt );
    };

    typedef ::std::pair<USHORT,USHORT>                          TResourcePair;
    typedef ::std::vector< ::std::pair<String, TResourcePair> > TResourceStruct;

    class OTasksWindow : public Window
    {
        ::std::vector< USHORT >             m_aHelpTextIds;
        OCreationList                       m_aCreation;
        FixedText                           m_aDescription;
        FixedText                           m_aHelpText;
        FixedLine                           m_aFL;
        OApplicationDetailView*             m_pDetailView;

        DECL_LINK( OnEntrySelectHdl,        SvTreeListBox* );
    public:
        OTasksWindow(Window* _pParent,OApplicationDetailView* _pDetailView);
        virtual ~OTasksWindow();

        // window overloads
        virtual void Resize();

        OApplicationDetailView* getDetailView() const { return m_pDetailView; }

        /** fills the Creation listbox with the necessary strings and images
            @param  _rList
                The strings and the id of the images and help texts to add.
            @param  _nImageListId
                The resource id for the imagelist which contains the images
        */
        void fillCreationNew(const TResourceStruct& _rList, USHORT _nImageListId);

        void Clear();
        void setHelpText(USHORT _nId);
    };
    //==================================================================
    class OApplicationDetailView : public OSplitterView
                                 , public IClipboardTest
    {
        Splitter                            m_aHorzSplitter;
        OTitleWindow                        m_aTasks;
        OTitleWindow                        m_aContainer;
        OAppBorderWindow*                   m_pBorderWin;       // my parent
        OAppDetailPageHelper*               m_pControlHelper;

        void ImplInitSettings( BOOL bFont, BOOL bForeground, BOOL bBackground );
    protected:
        virtual void DataChanged(const DataChangedEvent& rDCEvt);
    public:
        OApplicationDetailView(OAppBorderWindow* _pParent);
        virtual ~OApplicationDetailView();
        // window overloads
        //  virtual void Resize();
        virtual void GetFocus();

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

        sal_Bool isCutAllowed() ;
        sal_Bool isCopyAllowed()    ;
        sal_Bool isPasteAllowed();
        virtual sal_Bool hasChildPathFocus() { return HasChildPathFocus(); }
        void copy();
        void cut();
        void paste();

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

        /** select all entries in the detail page
        */
        void selectAll();

        /// returns <TRUE/> if it sorts ascending
        sal_Bool isSortUp() const;

        /// sort the entries in the detail page down
        void sortDown();

        /// sort the entries in the detail page up
        void sortUp();

        /// returns <TRUE/> when a detail page was filled
        sal_Bool isFilled() const;

        /// return the element of currently select entry
        ElementType getElementType() const;

        /** clears the detail pages.
            @param  _bTaskAlso
                If <TRUE/> the task window will also be cleared.
        */
        void clearPages(sal_Bool _bTaskAlso = sal_True);

        /// returns the count of entries
        sal_Int32 getElementCount();

        /// returns the count of selected entries
        sal_Int32 getSelectionCount();

        /** returns the element names which are selected
            @param  _rNames
                The list will be filled.
            @param  _xMetaData
                Will be used when the table list should be filled.
        */
        void getSelectionElementNames(::std::vector< ::rtl::OUString>& _rNames
            ,const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XDatabaseMetaData>& _xMetaData = NULL) const;

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
        SvLBoxEntry* elementAdded(ElementType eType
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
        */
        void switchPreview(PreviewMode _eMode);

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

        SvLBoxEntry* getEntry( const Point& _aPoint ) const;

        void onCreationClick( sal_uInt16 _nId);

        /** disable the controls
            @param  _bDisable
                if <TRUE/> then the controls will be disabled otherwise they will be enabled.
        */
        void disableControls(sal_Bool _bDisable);
    };
}
#endif // DBAUI_APPDETAILVIEW_HXX

