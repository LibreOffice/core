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



#ifndef DBAUI_QUERYCONTAINERWINDOW_HXX
#define DBAUI_QUERYCONTAINERWINDOW_HXX

#ifndef _SV_WINDOW_HXX
#include <vcl/window.hxx>
#endif
#ifndef _SV_SPLIT_HXX
#include <vcl/split.hxx>
#endif
#ifndef DBAUI_DATAVIEW_HXX
#include "dataview.hxx"
#endif
#ifndef _COM_SUN_STAR_FRAME_XFRAME_HPP_
#include <com/sun/star/frame/XFrame.hpp>
#endif
#ifndef DBAUI_QUERYVIEWSWITCH_HXX
#include "QueryViewSwitch.hxx"
#endif
#ifndef _SV_DOCKWIN_HXX
#include <vcl/dockwin.hxx>
#endif

class FixedLine;
//.........................................................................
namespace dbaui
{
//.........................................................................

    //=====================================================================
    //= OBeamer
    //=====================================================================
    // tempoaray class until the beamer is implemented
    class OBeamer : public DockingWindow
    {
    public:
        OBeamer(Window* _pParent) : DockingWindow(_pParent,0){}
    };

    //=====================================================================
    //= OQueryContainerWindow
    //=====================================================================
    class OQueryContainerWindow : public ODataView
    {
        OQueryViewSwitch*   m_pViewSwitch;
        OBeamer*            m_pBeamer;
        Splitter*           m_pSplitter;
        ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame > m_xBeamer;

        DECL_LINK( SplitHdl, void* );
    public:
        OQueryContainerWindow(Window* pParent, OQueryController& _rController,const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >&);
        ~OQueryContainerWindow();

        virtual void Construct();

        virtual long        PreNotify( NotifyEvent& rNEvt );

        // show the beamer
        void    showPreview(const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame >& _xFrame);
            // called when the beamer has been disposed
        void    disposingPreview();

        ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame >
                getPreviewFrame() const { return m_xBeamer; }

        OQueryDesignView*   getDesignView() { return m_pViewSwitch->getDesignView(); }

        sal_Bool isCutAllowed()     { return m_pViewSwitch->isCutAllowed(); }
        sal_Bool isPasteAllowed()   { return m_pViewSwitch->isPasteAllowed(); }
        sal_Bool isCopyAllowed()    { return m_pViewSwitch->isCopyAllowed(); }
        void copy()                 { m_pViewSwitch->copy(); }
        void cut()                  { m_pViewSwitch->cut(); }
        void paste()                { m_pViewSwitch->paste(); }

        void clear()                                                        { m_pViewSwitch->clear(); }
        sal_Bool isSlotEnabled( sal_Int32 _nSlotId )                        { return m_pViewSwitch->isSlotEnabled( _nSlotId ); }
        void     setSlotEnabled( sal_Int32 _nSlotId, sal_Bool _bEnable )    { m_pViewSwitch->setSlotEnabled( _nSlotId, _bEnable ); }
        void     setNoneVisbleRow(sal_Int32 _nRows)                         { m_pViewSwitch->setNoneVisbleRow( _nRows); }

        void setReadOnly( sal_Bool _bReadOnly )                             { m_pViewSwitch->setReadOnly( _bReadOnly ); }

        sal_Bool checkStatement()                                           { return m_pViewSwitch->checkStatement( ); }
        ::rtl::OUString getStatement()                                      { return m_pViewSwitch->getStatement( ); }
        void setStatement( const ::rtl::OUString& _rsStatement )            { m_pViewSwitch->setStatement( _rsStatement ); }

        void    initialize()                                                { m_pViewSwitch->initialize(); }
        void    SaveUIConfig()                                              { m_pViewSwitch->SaveUIConfig(); }
        bool    reset( ::dbtools::SQLExceptionInfo* _pErrorInfo )           { return m_pViewSwitch->reset( _pErrorInfo ); }

        bool    switchView( ::dbtools::SQLExceptionInfo* _pErrorInfo );
        void    forceInitialView();

        virtual void GetFocus();

    protected:
        // re-arrange the controls belonging to the document itself
        virtual void resizeAll( const Rectangle& _rPlayground );

        // arrange dericed classes controls in the rectangle given
        virtual void resizeDocumentView(Rectangle& _rPlayground);
    };
    // end of temp classes

//.........................................................................
}   // namespace dbaui
//.........................................................................

#endif // DBAUI_QUERYCONTAINERWINDOW_HXX

