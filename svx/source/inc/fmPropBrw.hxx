/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: fmPropBrw.hxx,v $
 *
 *  $Revision: 1.13 $
 *
 *  last change: $Author: hr $ $Date: 2007-08-03 10:20:42 $
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
#ifndef SVX_FMPROPBRW_HXX
#define SVX_FMPROPBRW_HXX

/** === begin UNO includes === **/
#ifndef _COM_SUN_STAR_LANG_XMULTISERVICEFACTORY_HPP_
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_XCONTROLCONTAINER_HPP_
#include <com/sun/star/awt/XControlContainer.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_XFRAME_HPP_
#include <com/sun/star/frame/XFrame.hpp>
#endif
#ifndef _COM_SUN_STAR_INSPECTION_XOBJECTINSPECTORMODEL_HPP_
#include <com/sun/star/inspection/XObjectInspectorModel.hpp>
#endif
#include <com/sun/star/uno/XComponentContext.hpp>
/** === end UNO includes === **/
#ifndef _BASEDLGS_HXX
#include <sfx2/basedlgs.hxx>
#endif
#ifndef _SFXCTRLITEM_HXX
#include <sfx2/ctrlitem.hxx>
#endif
#ifndef _SFX_CHILDWIN_HXX
#include <sfx2/childwin.hxx>
#endif
#ifndef _SVX_FMTOOLS_HXX
#include "fmtools.hxx"
#endif

//========================================================================
class FmPropBrwMgr : public SfxChildWindow
{
public:
    FmPropBrwMgr(Window *pParent, sal_uInt16 nId, SfxBindings *pBindings, SfxChildWinInfo *pInfo);
    SFX_DECL_CHILDWINDOW(FmPropBrwMgr);
};

class FmPropControl;
class SfxBindings;
class FmFormShell;
//========================================================================
class FmPropBrw : public SfxFloatingWindow, public SfxControllerItem
{
    sal_Bool        m_bInitialStateChange;
    ::rtl::OUString m_sLastActivePage;
    ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >
                        m_xInspectorContext;
    ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >
                    m_xORB;
    ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame >
                    m_xMeAsFrame;
    ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >
                    m_xLastKnownDocument;
    ::com::sun::star::uno::Reference< ::com::sun::star::inspection::XObjectInspectorModel >
                    m_xInspectorModel;
    ::com::sun::star::uno::Reference< ::com::sun::star::frame::XController >
                    m_xBrowserController;
    ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindow >
                    m_xBrowserComponentWindow;
    ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindow >
                    m_xFrameContainerWindow;

protected:
    virtual void StateChanged(sal_uInt16 nSID, SfxItemState eState, const SfxPoolItem* pState);
    virtual void FillInfo( SfxChildWinInfo& rInfo ) const;
    virtual sal_Bool Close();

    DECL_LINK( OnAsyncGetFocus, void* );

    void implSetNewSelection( const InterfaceBag& _rSelection );
    void implDetachController();
    bool implIsReadOnlyModel() const;
    ::rtl::OUString getCurrentPage() const;

public:
    FmPropBrw(
        const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& _xORB,
        SfxBindings* pBindings,
        SfxChildWindow* pMgr,
        Window* pParent,
        const SfxChildWinInfo* _pInfo
    );
    virtual ~FmPropBrw();

    using SfxFloatingWindow::StateChanged;

protected:
    virtual void        Resize();

private:
    /** creates the PropertyBrowser (aka ObjectInspector) and plugs it into our frame

        This method ensures that a new component is created every time the XModel which
        we're working for changed. This is necessary since this model is part of the
        ComponentContext we use to create the ObjectInspector.
    */
    void    impl_ensurePropertyBrowser_nothrow( FmFormShell* _pFormShell );

    /** creates a property browser

        After this method returns, m_xBrowserController and m_xBrowserComponentWindow are
        not <NULL/>.

    @precond
        we don't have an ObjectInspector, yet, i.e. m_xBrowserController and m_xBrowserComponentWindow
        are <NULL/>.
    */
    void    impl_createPropertyBrowser_throw( FmFormShell* _pFormShell );
};
#endif //SVX_FMPROPBRW_HXX
