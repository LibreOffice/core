/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/
#ifndef SW_VBA_SYSTEM_HXX
#define SW_VBA_SYSTEM_HXX

#include <ooo/vba/word/XSystem.hpp>
#include <vbahelper/vbahelperinterface.hxx>
#include <cppuhelper/implbase1.hxx>
#include <vbahelper/vbapropvalue.hxx>

typedef InheritedHelperInterfaceImpl1< ooo::vba::word::XSystem > SwVbaSystem_BASE;

class PrivateProfileStringListener : public PropListener
{
private:
    rtl::OUString maFileName;
    ByteString maGroupName;
    ByteString  maKey;
public:
    PrivateProfileStringListener(){};
    virtual ~PrivateProfileStringListener();
    void Initialize( const rtl::OUString& rFileName, const ByteString& rGroupName, const ByteString& rKey );

    //PropListener
    virtual void setValueEvent( const css::uno::Any& value );
    virtual css::uno::Any getValueEvent();
};

class SwVbaSystem : public SwVbaSystem_BASE
{
private:
    PrivateProfileStringListener maPrivateProfileStringListener;

public:
    SwVbaSystem( css::uno::Reference< css::uno::XComponentContext >& m_xContext );
    virtual ~SwVbaSystem();

    // XSystem
    virtual sal_Int32 SAL_CALL getCursor() throw ( css::uno::RuntimeException );
    virtual void SAL_CALL setCursor( sal_Int32 _cursor ) throw ( css::uno::RuntimeException );
    virtual css::uno::Any SAL_CALL PrivateProfileString( const rtl::OUString& rFilename, const rtl::OUString& rSection, const rtl::OUString& rKey ) throw ( css::uno::RuntimeException );

    // XHelperInterface
    virtual rtl::OUString& getServiceImplName();
    virtual css::uno::Sequence<rtl::OUString> getServiceNames();
};
#endif /* SW_VBA_SYSTEM_HXX */
