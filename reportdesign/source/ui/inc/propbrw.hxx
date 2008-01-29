#ifndef _REPORT_PROPBRW_HXX
#define _REPORT_PROPBRW_HXX
/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: propbrw.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2008-01-29 13:49:29 $
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


#ifndef _COM_SUN_STAR_LANG_XMULTISERVICEFACTORY_HPP_
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#endif

#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSET_HPP_
#include <com/sun/star/beans/XPropertySet.hpp>
#endif

#ifndef _COM_SUN_STAR_AWT_XCONTROLCONTAINER_HPP_
#include <com/sun/star/awt/XControlContainer.hpp>
#endif

#ifndef _COM_SUN_STAR_FRAME_XFRAME_HPP_
#include <com/sun/star/frame/XFrame.hpp>
#endif
#ifndef _COM_SUN_STAR_INSPECTION_XOBJECTINSPECTOR_HPP_
#include <com/sun/star/inspection/XObjectInspector.hpp>
#endif
#include <com/sun/star/uno/XComponentContext.hpp>
#ifndef _SFXBRDCST_HXX //autogen
#include <svtools/brdcst.hxx>
#endif

#ifndef _SFXLSTNER_HXX //autogen
#include <svtools/lstner.hxx>
#endif

#ifndef _SV_DOCKWIN_HXX
#include <vcl/dockwin.hxx>
#endif
#ifndef _SVDMARK_HXX //autogen
#include <svx/svdmark.hxx>
#endif
#ifndef _RPTUI_MODULE_HELPER_RPT_HXX_
#include "ModuleHelper.hxx"
#endif

namespace rptui
{

class OSectionView;
class ODesignView;
class OObjectBase;
//============================================================================
// PropBrw
//============================================================================

class PropBrw : public DockingWindow , public SfxListener, public SfxBroadcaster
{
private:
    OModuleClient       m_aModuleClient;
    ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >
                        m_xInspectorContext;
    ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >
                        m_xORB;
    ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame >
                        m_xMeAsFrame;
    ::com::sun::star::uno::Reference< ::com::sun::star::inspection::XObjectInspector >
                        m_xBrowserController;
    ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindow >
                        m_xBrowserComponentWindow;
    ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface>
                        m_xLastSection; /// is the previously displayed section
    ::rtl::OUString     m_sLastActivePage;
    ODesignView*        m_pDesignView;
    OSectionView*       m_pView;
    sal_Bool            m_bInitialStateChange;

    PropBrw(PropBrw&);
    void operator =(PropBrw&);
protected:

    virtual void Resize();
    virtual sal_Bool Close();

    ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface> >
        CreateCompPropSet(const SdrMarkList& rMarkList);

    void implSetNewObject(
        const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface> >& _aObjects = ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface> >());

    ::rtl::OUString GetHeadlineName(
        const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface> >& _aObjects);

    void implDetachController();
    ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface> CreateComponentPair(OObjectBase* _pObj);
    ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface> CreateComponentPair(
             const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface>& _xFormComponent
            ,const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface>& _xReportComponent);
    DECL_LINK( OnAsyncGetFocus, void* );

public:
    PropBrw(const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& _xORB
            ,Window* pParent
            ,ODesignView*  _pDesignView);
    virtual ~PropBrw();

    virtual void LoseFocus();

    void    Update( OSectionView* m_pView );
    void    Update( const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface>& _xReportComponent);
    inline OSectionView*    GetCurView() const { return m_pView; }
    ::rtl::OUString         getCurrentPage() const;
    inline void             setCurrentPage(const ::rtl::OUString& _sLastActivePage) { m_sLastActivePage = _sLastActivePage; }

    ::Size getMinimumSize() const;
private:
    using Window::Update;
};
//==============================================================================
} // rptui
//==============================================================================
#endif // _REPORT_PROPBRW_HXX
