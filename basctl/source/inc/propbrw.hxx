/*************************************************************************
 *
 *  $RCSfile: propbrw.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: tbe $ $Date: 2001-03-23 16:16:45 $
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
 *  Contributor(s): _______________________________________
 *
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
protected:
    ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControlContainer >
                    m_xUnoRepresentation;
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

protected:
    SdrView*        pView;
    virtual void Resize();
    virtual void FillInfo( SfxChildWinInfo& rInfo ) const;
    virtual sal_Bool Close();

    DECLARE_STL_VECTOR(::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet>, PropertySetArray);

    ::comphelper::OComposedPropertySet* CreateCompPropSet(const SdrMarkList& rMarkList);

    void implSetNewObject(
        const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& _rxObject);

    ::rtl::OUString GetHeadlineName(
        const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& _rxObject);

    void implDetachController();

public:
    PropBrw(const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& _xORB,
            SfxBindings *pBindings, SfxChildWindow *pMgr, Window* pParent);
    virtual ~PropBrw();
    void    Update( SdrView* pView );
    SdrView*        GetCurView() const { return pView; }
};

#endif // _BASCTL_PROPBRW_HXX
