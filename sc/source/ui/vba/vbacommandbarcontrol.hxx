/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile$
 *
 *  $Revision$
 *
 *  last change: $Author$ $Date$
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
#ifndef SC_VBA_COMMANDBARCONTROL_HXX
#define SC_VBA_COMMANDBARCONTROL_HXX

#include <ooo/vba/XCommandBarControl.hpp>

#include "vbahelperinterface.hxx"
#include "vbacommandbarcontrols.hxx"

typedef InheritedHelperInterfaceImpl1< ov::XCommandBarControl > CommandBarControl_BASE;

class ScVbaCommandBarControl : public CommandBarControl_BASE
{
private:
    rtl::OUString       m_sName;
    rtl::OUString       m_sBarName;
    rtl::OUString       m_sCommand;
    sal_Int32           m_nType;
    sal_Int32           m_nPosition;
    sal_Bool            m_bTemporary;
    sal_Bool            m_bIsMenu;
    ScVbaCommandBarControls*        m_pCommandBarControls;
    css::uno::Reference< ov::XHelperInterface >               m_xParentHardRef;
    css::uno::Reference< css::ui::XUIConfigurationManager >         m_xUICfgManager;
    css::uno::Reference< css::ui::XUIConfigurationPersistence >     m_xUICfgPers;
    css::uno::Reference< css::container::XIndexContainer >          m_xBarSettings;
    css::uno::Reference< css::container::XIndexContainer >          m_xCurrentSettings;
    css::beans::PropertyValues                                      m_aPropertyValues;

    void initObjects() throw (css::uno::RuntimeException);
    void createNewMenuBarControl();
    void createNewToolBarControl();
public:
    ScVbaCommandBarControl( const css::uno::Reference< ov::XHelperInterface > xParent, const css::uno::Reference< css::uno::XComponentContext > xContext, rtl::OUString sName ) throw (css::uno::RuntimeException);
    ScVbaCommandBarControl( const css::uno::Reference< ov::XHelperInterface > xParent, const css::uno::Reference< css::uno::XComponentContext > xContext, rtl::OUString sName, rtl::OUString sCommand, sal_Int32 nPosition, sal_Bool bTemporary ) throw (css::uno::RuntimeException);
    sal_Int32 GetPosition() { return m_nPosition; };
    css::uno::Reference< css::container::XIndexContainer > GetCurrentSettings() { return m_xCurrentSettings; };
    css::beans::PropertyValues GetPropertyValues() { return m_aPropertyValues; };
    void SetPropertyValues( css::beans::PropertyValues aPropertyValues ) { m_aPropertyValues = aPropertyValues; };

    // Attributes
    virtual ::rtl::OUString SAL_CALL getCaption() throw (css::uno::RuntimeException);
    virtual void SAL_CALL setCaption( const ::rtl::OUString& _caption ) throw (css::uno::RuntimeException);
    virtual ::rtl::OUString SAL_CALL getOnAction() throw (css::uno::RuntimeException);
    virtual void SAL_CALL setOnAction( const ::rtl::OUString& _onaction ) throw (css::uno::RuntimeException);
    virtual ::sal_Bool SAL_CALL getVisible() throw (css::uno::RuntimeException);
    virtual void SAL_CALL setVisible( ::sal_Bool _visible ) throw (css::uno::RuntimeException);

    // Methods
    virtual void SAL_CALL Delete(  ) throw (css::script::BasicErrorException, css::uno::RuntimeException);
    virtual css::uno::Any SAL_CALL Controls( const css::uno::Any& aIndex ) throw (css::script::BasicErrorException, css::uno::RuntimeException);

    // XHelperInterface
    virtual rtl::OUString& getServiceImplName();
    virtual css::uno::Sequence<rtl::OUString> getServiceNames();
};
#endif//SC_VBA_COMMANDBARCONTROL_HXX
