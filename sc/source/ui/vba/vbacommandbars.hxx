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
#ifndef SC_VBA_COMMANDBARS_HXX
#define SC_VBA_COMMANDBARS_HXX

#include <ooo/vba/XCommandBar.hpp>
#include <ooo/vba/XCommandBars.hpp>
#include <com/sun/star/container/XNameAccess.hpp>

#include <cppuhelper/implbase1.hxx>

#include "vbahelperinterface.hxx"
#include "vbacollectionimpl.hxx"

typedef CollTestImplHelper< ov::XCommandBars > CommandBars_BASE;

class ScVbaCommandBars : public CommandBars_BASE
{
private:
    css::uno::Reference< css::container::XNameAccess > m_xNameAccess;
    rtl::OUString m_sModuleName;
    void retrieveObjects() throw( css::uno::RuntimeException );
public:
    ScVbaCommandBars( const css::uno::Reference< ov::XHelperInterface > xParent, const css::uno::Reference< css::uno::XComponentContext > xContext, const css::uno::Reference< css::container::XIndexAccess > xIndexAccess );

    sal_Bool checkToolBarExist( rtl::OUString sToolBarName );
    rtl::OUString GetModuleName(){ return m_sModuleName; };
    css::uno::Reference< css::container::XNameAccess > GetWindows()
    {
        retrieveObjects();
        return m_xNameAccess;
    };
    // XCommandBars
    virtual css::uno::Reference< ov::XCommandBar > SAL_CALL Add( const css::uno::Any& Name, const css::uno::Any& Position, const css::uno::Any& MenuBar, const css::uno::Any& Temporary ) throw (css::script::BasicErrorException, css::uno::RuntimeException);
    // XEnumerationAccess
    virtual css::uno::Type SAL_CALL getElementType() throw (css::uno::RuntimeException);
    virtual css::uno::Reference< css::container::XEnumeration > SAL_CALL createEnumeration() throw (css::uno::RuntimeException);
    virtual css::uno::Any createCollectionObject( const css::uno::Any& aSource );

    virtual sal_Int32 SAL_CALL getCount() throw(css::uno::RuntimeException);
    virtual css::uno::Any SAL_CALL Item( const css::uno::Any& aIndex, const css::uno::Any& /*aIndex2*/ ) throw( css::uno::RuntimeException);
    // XHelperInterface
    virtual rtl::OUString& getServiceImplName();
    virtual css::uno::Sequence<rtl::OUString> getServiceNames();
};

#endif//SC_VBA_COMMANDBARS_HXX
