/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: propbrw.hxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: kz $ $Date: 2008-03-06 19:15:10 $
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

#ifndef _BASCTL_PROPBRW_HXX
#define _BASCTL_PROPBRW_HXX

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

#ifndef _COMPHELPER_COMPOSEDPROPS_HXX_
#include <comphelper/composedprops.hxx>
#endif

#ifndef _BASEDLGS_HXX
#include <sfx2/basedlgs.hxx>
#endif

#ifndef _SFXBRDCST_HXX //autogen
#include <svtools/brdcst.hxx>
#endif

#ifndef _SFXLSTNER_HXX //autogen
#include <svtools/lstner.hxx>
#endif

#ifndef _SFX_CHILDWIN_HXX
#include <sfx2/childwin.hxx>
#endif

#ifndef _SVDMARK_HXX //autogen
#include <svx/svdmark.hxx>
#endif

//============================================================================
// PropBrwMgr
//============================================================================

class PropBrwMgr : public SfxChildWindow
{
public:
    PropBrwMgr(Window *pParent, sal_uInt16 nId, SfxBindings *pBindings, SfxChildWinInfo *pInfo);
    SFX_DECL_CHILDWINDOW(PropBrwMgr);
};

//============================================================================
// PropBrw
//============================================================================

class SfxBindings;
class SdrView;

class PropBrw : public SfxFloatingWindow , public SfxListener, public SfxBroadcaster
{
private:
    sal_Bool        m_bInitialStateChange;

    ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >
                    m_xORB;
    ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame >
                    m_xMeAsFrame;
    ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >
                    m_xBrowserController;
    ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindow >
                    m_xBrowserComponentWindow;
    ::com::sun::star::uno::Reference< ::com::sun::star::frame::XModel >
                    m_xContextDocument;

protected:
    SdrView*        pView;
    virtual void Resize();
    virtual void FillInfo( SfxChildWinInfo& rInfo ) const;
    virtual sal_Bool Close();

    DECLARE_STL_VECTOR(::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface>, InterfaceArray);

    ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > >
        CreateMultiSelectionSequence( const SdrMarkList& _rMarkList );
    void implSetNewObjectSequence( const ::com::sun::star::uno::Sequence
        < ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > >& _rObjectSeq );

    void implSetNewObject(
        const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& _rxObject);

    ::rtl::OUString GetHeadlineName(
        const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& _rxObject);

public:
    PropBrw( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& _xORB,
             SfxBindings *pBindings,
             PropBrwMgr* pMgr,
             Window* pParent,
             const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XModel >& _rxContextDocument
    );
    virtual ~PropBrw();
    using Window::Update;
    // note: changing the Context document to an instance other than the one given in the ctor is not supported
    // currently
    void    Update( const SfxViewShell* _pShell );
    SdrView*        GetCurView() const { return pView; }

private:
    void    ImplUpdate( const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XModel >& _rxContextDocument, SdrView* pView );
    void    ImplDestroyController();
    void    ImplReCreateController();
};

#endif // _BASCTL_PROPBRW_HXX
