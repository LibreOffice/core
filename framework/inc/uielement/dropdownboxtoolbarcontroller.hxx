/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: dropdownboxtoolbarcontroller.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: ihi $ $Date: 2006-08-01 09:36:44 $
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

#ifndef __FRAMEWORK_UIELEMENT_DROPDOWNBOXTOOLBARCONTROLLER_HXX_
#define __FRAMEWORK_UIELEMENT_DROPDOWNBOXTOOLBARCONTROLLER_HXX_

#include <com/sun/star/beans/NamedValue.hpp>
#include <com/sun/star/frame/XDispatch.hpp>
#include <com/sun/star/frame/ControlCommand.hpp>

//_________________________________________________________________________________________________________________
//  includes of other projects
//_________________________________________________________________________________________________________________

#include <uielement/complextoolbarcontroller.hxx>
#include <vcl/toolbox.hxx>
#include <vcl/lstbox.hxx>

namespace framework
{

class ToolBar;
class ListBoxControl;

class IListBoxListener
{
    public:
        virtual void Select() = 0;
        virtual void DoubleClick() = 0;
        virtual void GetFocus() = 0;
        virtual void LoseFocus() = 0;
        virtual long PreNotify( NotifyEvent& rNEvt ) = 0;
};

class DropdownToolbarController : public IListBoxListener,
                                  public ComplexToolbarController

{
    public:
        DropdownToolbarController( const com::sun::star::uno::Reference< com::sun::star::lang::XMultiServiceFactory >& rServiceManager,
                                   const com::sun::star::uno::Reference< com::sun::star::frame::XFrame >& rFrame,
                                   ToolBar*     pToolBar,
                                   USHORT       nID,
                                   sal_Int32    nWidth,
                                   const rtl::OUString& aCommand );
        virtual ~DropdownToolbarController();

        // XComponent
        virtual void SAL_CALL dispose() throw ( ::com::sun::star::uno::RuntimeException );

        // XToolbarController
        virtual void SAL_CALL execute( sal_Int16 KeyModifier ) throw (::com::sun::star::uno::RuntimeException);

        // IComboBoxListener
        virtual void Select();
        virtual void DoubleClick();
        virtual void GetFocus();
        virtual void LoseFocus();
        virtual long PreNotify( NotifyEvent& rNEvt );

    protected:
        virtual void executeControlCommand( const ::com::sun::star::frame::ControlCommand& rControlCommand );

    private:
        ListBoxControl*    m_pListBoxControl;
};

}

#endif // __FRAMEWORK_UIELEMENT_DROPDOWNBOXTOOLBARCONTROLLER_HXX_
