/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: togglebuttontoolbarcontroller.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: hr $ $Date: 2006-05-08 15:17:18 $
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

#ifndef __FRAMEWORK_UIELEMENT_TOGGLEBUTTONTOOLBARCONTROLLER_HXX_
#define __FRAMEWORK_UIELEMENT_TOGGLEBUTTONTOOLBARCONTROLLER_HXX_

#include <com/sun/star/beans/NamedValue.hpp>
#include <com/sun/star/frame/XDispatch.hpp>
#include <com/sun/star/frame/ControlCommand.hpp>

//_________________________________________________________________________________________________________________
//  includes of other projects
//_________________________________________________________________________________________________________________

#include <uielement/complextoolbarcontroller.hxx>
#include <vcl/toolbox.hxx>
#include <vcl/image.hxx>

namespace framework
{

class ToolBar;
class ToggleButtonToolbarController : public ComplexToolbarController

{
    public:
        enum Style
        {
            STYLE_TOGGLEBUTTON,
            STYLE_DROPDOWNBUTTON,
            STYLE_TOGGLE_DROPDOWNBUTTON
        };

        ToggleButtonToolbarController( const com::sun::star::uno::Reference< com::sun::star::lang::XMultiServiceFactory >& rServiceManager,
                                       const com::sun::star::uno::Reference< com::sun::star::frame::XFrame >& rFrame,
                                       ToolBar*             pToolBar,
                                       USHORT               nID,
                                       Style                eStyle,
                                       const rtl::OUString& aCommand );
        virtual ~ToggleButtonToolbarController();

        // XComponent
        virtual void SAL_CALL dispose() throw ( ::com::sun::star::uno::RuntimeException );

        // XToolbarController
        virtual void SAL_CALL execute( sal_Int16 KeyModifier ) throw (::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindow > SAL_CALL createPopupWindow() throw (::com::sun::star::uno::RuntimeException);

    protected:
        virtual void executeControlCommand( const ::com::sun::star::frame::ControlCommand& rControlCommand );

    private:
        DECL_LINK( MenuSelectHdl, Menu *);

        Style                        m_eStyle;
        rtl::OUString                m_aCurrentSelection;
        std::vector< rtl::OUString > m_aDropdownMenuList;
};

}

#endif // __FRAMEWORK_UIELEMENT_TOGGLEBUTTONTOOLBARCONTROLLER_HXX_
