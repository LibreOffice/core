/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: generictoolbarcontroller.hxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 00:43:30 $
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

#ifndef __FRAMEWORK_UIELEMENT_GENERICTOOLBARCONTROLLER_HXX_
#define __FRAMEWORK_UIELEMENT_GENERICTOOLBARCONTROLLER_HXX_

//_________________________________________________________________________________________________________________
//  includes of other projects
//_________________________________________________________________________________________________________________

#ifndef _SVTOOLS_TOOLBOXCONTROLLER_HXX
#include <svtools/toolboxcontroller.hxx>
#endif

#ifndef _SV_TOOLBOX_HXX
#include <vcl/toolbox.hxx>
#endif

namespace framework
{

struct ExecuteInfo;
class ToolBar;
class GenericToolbarController : public svt::ToolboxController
{
    public:
        GenericToolbarController( const com::sun::star::uno::Reference< com::sun::star::lang::XMultiServiceFactory >& rServiceManager,
                                  const com::sun::star::uno::Reference< com::sun::star::frame::XFrame >& rFrame,
                                  ToolBar* pToolBar,
                                  USHORT   nID,
                                  const rtl::OUString& aCommand );
        virtual ~GenericToolbarController();

        // XComponent
        virtual void SAL_CALL dispose() throw ( ::com::sun::star::uno::RuntimeException );

        // XToolbarController
        virtual void SAL_CALL execute( sal_Int16 KeyModifier ) throw (::com::sun::star::uno::RuntimeException);

        // XStatusListener
        virtual void SAL_CALL statusChanged( const ::com::sun::star::frame::FeatureStateEvent& Event ) throw ( ::com::sun::star::uno::RuntimeException );

         DECL_STATIC_LINK( GenericToolbarController, ExecuteHdl_Impl, ExecuteInfo* );

    private:
        ToolBar*        m_pToolbar;
        sal_uInt16      m_nID;
        sal_Bool        m_bEnumCommand : 1,
                        m_bMadeInvisible : 1;
        rtl::OUString   m_aEnumCommand;
};

}

#endif // __FRAMEWORK_UIELEMENT_GENERICTOOLBARCONTROLLER_HXX_
