/*************************************************************************
 *
 *  $RCSfile: AppView.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: hr $ $Date: 2004-08-02 15:30:44 $
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
#ifndef DBAUI_APPVIEW_HXX
#define DBAUI_APPVIEW_HXX

#ifndef DBAUI_DATAVIEW_HXX
#include "dataview.hxx"
#endif
#ifndef _COM_SUN_STAR_FRAME_XCONTROLLER_HPP_
#include <com/sun/star/frame/XController.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XNAMEACCESS_HPP_
#include <com/sun/star/container/XNameAccess.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_XDATABASEMETADATA_HPP_
#include <com/sun/star/sdbc/XDatabaseMetaData.hpp>
#endif
#ifndef _COM_SUN_STAR_UCB_XCONTENT_HPP_
#include <com/sun/star/ucb/XContent.hpp>
#endif
#ifndef _SV_STATUS_HXX
#include <vcl/status.hxx>
#endif
#ifndef _SV_FIXED_HXX
#include <vcl/fixed.hxx>
#endif
#ifndef _UNOTOOLS_EVENTLISTENERADAPTER_HXX_
#include <unotools/eventlisteneradapter.hxx>
#endif
#ifndef DBACCESS_TABLEDESIGN_ICLIPBOARDTEST_HXX
#include "IClipBoardTest.hxx"
#endif
#ifndef DBAUI_APPELEMENTTYPE_HXX
#include "AppElementType.hxx"
#endif

namespace com{ namespace sun { namespace star { namespace beans    { class XPropertySet; } } } };
namespace com{ namespace sun { namespace star { namespace frame    { class XController; } } } };
class Control;
class SvLBoxEntry;

namespace dbaui
{
    class IApplicationElementNotification;
    class IControlActionListener;
    class IContainerFoundListener;
    class IViewChangeListener;
    class OApplicationView;
    class OApplicationDetailView;
    class OApplicationSwapWindow;
    class OTitleWindow;
    //==================================================================
    class OAppBorderWindow : public Window
    {
        OTitleWindow*                       m_pPanel;
        OApplicationDetailView*             m_pDetailView;
        OApplicationView*                   m_pView;

    public:
        OAppBorderWindow(OApplicationView* _pParent);
        virtual ~OAppBorderWindow();

        // window overloads
        virtual void GetFocus();
        virtual void Resize();

        OApplicationView*       getView() const;
        OApplicationSwapWindow* getPanel() const;
        OApplicationDetailView* getDetailView() const;
    };
    //==================================================================
    class OApplicationView : public ODataView
                            ,public IClipboardTest
                            ,public ::utl::OEventListenerAdapter
    {
        enum ChildFocusState
        {
            PANELSWAP,
            DETAIL,
            NONE
        };
    private:
        StatusBar                           m_aStatusBar;
        ::com::sun::star::lang::Locale      m_aLocale;
        ::com::sun::star::uno::Reference<
            ::com::sun::star::frame::XController>
                                            m_xController;
        ::com::sun::star::uno::Reference< ::com::sun::star::lang::XComponent >
                                            m_xObject;
        OAppBorderWindow*                   m_pWin;
        IApplicationElementNotification*    m_pElementNotification;
        IControlActionListener*             m_pActonListener;
        IContainerFoundListener*            m_pContainerListener;
        IViewChangeListener*                m_pViewChangeListener;
        ChildFocusState                     m_eChildFocus;

        IClipboardTest* getActiveChild() const;


    protected:


        // return the Rectangle where I can paint myself
        virtual void resizeDocumentView(Rectangle& rRect);

        // OEventListenerAdapter
        virtual void _disposing( const ::com::sun::star::lang::EventObject& _rSource );

    public:
        OApplicationView(   Window* pParent
                            ,const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >&
                            ,IController* _pIController
                            ,IApplicationElementNotification*   _pController
                            ,IControlActionListener*            _pActonListener
                            ,IContainerFoundListener*           _pContainerListener
                            ,IViewChangeListener*               _pViewChangeListener
                            ,const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XController>& _xController
                            );
        virtual ~OApplicationView();

        // window overloads
        virtual long PreNotify( NotifyEvent& rNEvt );
        virtual void DataChanged( const DataChangedEvent& rDCEvt );
        virtual void GetFocus();

        inline IApplicationElementNotification*         getElementNotification() const { return m_pElementNotification; }
        inline IControlActionListener*                  getActionListener() const { return m_pActonListener; }
        inline IContainerFoundListener*                 getContainerListener() const { return m_pContainerListener; }
        inline IViewChangeListener*                     getViewChangeListener() const { return m_pViewChangeListener; }
        inline const ::com::sun::star::lang::Locale&    getLocale() const { return m_aLocale;}
        inline ::com::sun::star::uno::Reference<
            ::com::sun::star::frame::XController> getController() const { return m_xController; }

        // IClipboardTest
        virtual sal_Bool isCutAllowed();
        virtual sal_Bool isCopyAllowed();
        virtual sal_Bool isPasteAllowed();
        virtual sal_Bool hasChildPathFocus() { return HasChildPathFocus(); }
        virtual void copy();
        virtual void cut();
        virtual void paste();

        /// get the left panel
        inline OApplicationSwapWindow*  getPanel()      const { return m_pWin->getPanel(); }
        /// get the detail page
        inline OApplicationDetailView*  getDetailView() const { return m_pWin->getDetailView(); }

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

        /// returns the count of entries
        sal_Int32 getElementCount();

        /// returns the count of selected entries
        sal_Int32 getSelectionCount();

        /** clears the detail page and the selection on the left side.
            @param  _bTaskAlso
                If <TRUE/> the task window will also be cleared.
        */
        void clearPages(sal_Bool _bTaskAlso = sal_True);

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
        SvLBoxEntry* elementAdded(ElementType _eType
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

        /** clears the selection in the icon choice control and calls the handler
        */
        void clearSelection();


        /** changes the container which should be displayed. The select handler will also be called.
            @param  _eType
                Which container to show.
        */
        void changeContainer(ElementType _eType);

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

        SvLBoxEntry* getEntry( const Point& _aPosPixel ) const;

        void setStatusInformations(const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet>& _xDatasource);

        /** disable the controls
            @param  _bDisable
                if <TRUE/> then the controls will be disabled otherwise they will be enabled.
        */
        void disableControls(sal_Bool _bDisable);

        DECL_LINK( SwitchHdl, Accelerator* );
    };
}
#endif // DBAUI_APPVIEW_HXX

