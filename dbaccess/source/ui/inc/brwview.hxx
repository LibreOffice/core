/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: brwview.hxx,v $
 *
 *  $Revision: 1.12 $
 *
 *  last change: $Author: hr $ $Date: 2006-06-20 03:16:20 $
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
    class DBTreeListModel;
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
                            IController* _pController,
                            const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& );
        virtual ~UnoDataBrowserView();

        /// late construction
        virtual void Construct(const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControlModel >& xModel);

        /** as columns may be hidden there is a difference between a columns model pos and its view pos
            so we you may use these translation function
        */
        sal_uInt16 Model2ViewPos(sal_uInt16 nPos) const;
        sal_uInt16 View2ModelPos(sal_uInt16 nPos) const;
        /// for the same reason the view column count isn't the same as the model column count
        sal_uInt16 ViewColumnCount() const;

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

