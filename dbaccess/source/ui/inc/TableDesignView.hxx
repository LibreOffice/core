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

        void ImplInitSettings( sal_Bool bFont, sal_Bool bForeground, sal_Bool bBackground );
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
        OTableController&                   m_rController;
        ChildFocusState                     m_eChildFocus;

        IClipboardTest* getActiveChild() const;
    protected:


        // return the Rectangle where I can paint myself
        virtual void resizeDocumentView(Rectangle& rRect);

    public:
        OTableDesignView(   Window* pParent,
                            const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >&,
                            OTableController& _rController);
        virtual ~OTableDesignView();

        // window overloads
        virtual long            PreNotify( NotifyEvent& rNEvt );
        virtual void            GetFocus();

        OTableEditorCtrl*       GetEditorCtrl() const { return m_pWin ? m_pWin->GetEditorCtrl() : NULL; }
        OTableFieldDescWin*     GetDescWin()    const { return m_pWin ? m_pWin->GetDescWin() : NULL; }
        OTableController&       getController() const { return m_rController; }

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

