/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: TableDesignView.hxx,v $
 *
 *  $Revision: 1.11 $
 *
 *  last change: $Author: obo $ $Date: 2006-03-29 12:39:00 $
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
#ifndef DBAUI_TABLEDESIGNVIEW_HXX
#define DBAUI_TABLEDESIGNVIEW_HXX

#ifndef DBAUI_DATAVIEW_HXX
#include "dataview.hxx"
#endif
#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSET_HPP_
#include <com/sun/star/beans/XPropertySet.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_XCONTROLLER_HPP_
#include <com/sun/star/frame/XController.hpp>
#endif
#ifndef _SV_SPLIT_HXX
#include <vcl/split.hxx>
#endif
#ifndef DBACCESS_TABLEDESIGN_ICLIPBOARDTEST_HXX
#include "IClipBoardTest.hxx"
#endif

namespace dbaui
{
    class OTableController;
    class OTableFieldDescWin;
    class OTableEditorCtrl;
    //==================================================================
    class OTableBorderWindow : public Window
    {
        Splitter                            m_aHorzSplitter;
        OTableFieldDescWin*                 m_pFieldDescWin;
        OTableEditorCtrl*                   m_pEditorCtrl;

        void ImplInitSettings( BOOL bFont, BOOL bForeground, BOOL bBackground );
        void ArrangeChilds( long nSplitPos ,Rectangle& rRect);
        DECL_LINK( SplitHdl, Splitter* );
    protected:
        virtual void DataChanged(const DataChangedEvent& rDCEvt);
    public:
        OTableBorderWindow(Window* pParent);
        ~OTableBorderWindow();
        // window overloads
        virtual void Resize();
        virtual void GetFocus();

        OTableEditorCtrl*       GetEditorCtrl() const { return m_pEditorCtrl; }
        OTableFieldDescWin*     GetDescWin()    const { return m_pFieldDescWin; }
    };
    //==================================================================
    class OTableDesignView : public ODataView
                            ,public IClipboardTest
    {
        enum ChildFocusState
        {
            DESCRIPTION,
            EDITOR,
            NONE
        };
    private:
        ::com::sun::star::lang::Locale      m_aLocale;
        OTableBorderWindow*                 m_pWin;
        OTableController*                   m_pController;
        ChildFocusState                     m_eChildFocus;

        IClipboardTest* getActiveChild() const;
    protected:


        // return the Rectangle where I can paint myself
        virtual void resizeDocumentView(Rectangle& rRect);

    public:
        OTableDesignView(   Window* pParent,
                            const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >&,
                            OTableController* _pController);
        virtual ~OTableDesignView();

        // window overloads
        virtual long            PreNotify( NotifyEvent& rNEvt );
        virtual void            GetFocus();

        OTableEditorCtrl*       GetEditorCtrl() const { return m_pWin ? m_pWin->GetEditorCtrl() : NULL; }
        OTableFieldDescWin*     GetDescWin()    const { return m_pWin ? m_pWin->GetDescWin() : NULL; }
        OTableController*       getController() const { return m_pController; }

        ::com::sun::star::lang::Locale      getLocale() const { return m_aLocale;}

        // IClipboardTest
        virtual sal_Bool isCutAllowed();
        virtual sal_Bool isCopyAllowed();
        virtual sal_Bool isPasteAllowed();
        virtual sal_Bool hasChildPathFocus() { return HasChildPathFocus(); }
        virtual void copy();
        virtual void cut();
        virtual void paste();

        // set the view readonly or not
        virtual void setReadOnly(sal_Bool _bReadOnly);

        virtual void initialize();
        void reSync(); // resync window data with realdata

        DECL_LINK( SwitchHdl, Accelerator* );
    };
}
#endif // DBAUI_TABLEDESIGNVIEW_HXX

