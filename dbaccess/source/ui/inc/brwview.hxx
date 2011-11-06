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



#ifndef _SBX_BRWVIEW_HXX
#define _SBX_BRWVIEW_HXX

#ifndef _SV_WINDOW_HXX //autogen
#include <vcl/window.hxx>
#endif

#ifndef _TOOLS_RESID_HXX //autogen wg. ResId
#include <tools/resid.hxx>
#endif

#ifndef _COM_SUN_STAR_AWT_POSSIZE_HPP_
#include <com/sun/star/awt/PosSize.hpp>
#endif
#ifndef DBAUI_DATAVIEW_HXX
#include "dataview.hxx"
#endif
#ifndef _UNOTOOLS_EVENTLISTENERADAPTER_HXX_
#include <unotools/eventlisteneradapter.hxx>
#endif


namespace com { namespace sun { namespace star { namespace awt {
    class XControl;
    class XControlContainer;
    class XControlModel;
}}}}

// =========================================================================
class ResMgr;
class Splitter;

namespace dbaui
{
    class DBTreeView;
    class SbaGridControl;

    class UnoDataBrowserView : public ODataView, public ::utl::OEventListenerAdapter
    {
    protected:
        ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControl >                 m_xGrid;            // our grid's UNO representation
        ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControlContainer >        m_xMe;              // our own UNO representation
        DBTreeView*             m_pTreeView;
        Splitter*               m_pSplitter;
        mutable SbaGridControl* m_pVclControl;  // our grid's VCL representation
        Window*                 m_pStatus;

        DECL_LINK( SplitHdl, void* );
    // attribute access
    public:
        ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControl >             getGridControl() const  { return m_xGrid; }
        SbaGridControl*         getVclControl() const;

    public:
        UnoDataBrowserView( Window* pParent,
                            IController& _rController,
                            const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& );
        virtual ~UnoDataBrowserView();

        /// late construction
        virtual void Construct(const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControlModel >& xModel);

        /** as columns may be hidden there is a difference between a columns model pos and its view pos
            so we you may use these translation function
        */
        sal_uInt16 View2ModelPos(sal_uInt16 nPos) const;
        /// for the same reason the view column count isn't the same as the model column count

        void setSplitter(Splitter* _pSplitter);
        void setTreeView(DBTreeView* _pTreeView);

        void    showStatus( const String& _rStatus );
        void    hideStatus();

        ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControlContainer >     getContainer() { return m_xMe; }

    protected:
        virtual long PreNotify( NotifyEvent& rNEvt );
        virtual void GetFocus();
        virtual void resizeDocumentView(Rectangle& rRect);
        virtual void _disposing( const ::com::sun::star::lang::EventObject& _rSource );

    private:
        using ODataView::Construct;
    };

    class BrowserViewStatusDisplay
    {
    protected:
        UnoDataBrowserView*     m_pView;

    public:
        BrowserViewStatusDisplay( UnoDataBrowserView* _pView, const String& _rStatus );
        ~BrowserViewStatusDisplay( );
    };
}
#endif // _SBX_BRWVIEW_HXX

