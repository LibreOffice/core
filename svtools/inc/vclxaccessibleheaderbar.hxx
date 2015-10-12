/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */
#ifndef INCLUDED_SVTOOLS_INC_VCLXACCESSIBLEHEADERBAR_HXX
#define INCLUDED_SVTOOLS_INC_VCLXACCESSIBLEHEADERBAR_HXX

#include <svtools/headbar.hxx>
#include <toolkit/awt/vclxaccessiblecomponent.hxx>
#include <toolkit/awt/vclxwindow.hxx>

class HeaderBar;


//  class VCLXAccessibleHeaderBar


typedef std::vector< ::com::sun::star::uno::WeakReference< ::com::sun::star::accessibility::XAccessible > >
    ListItems;

class VCLXAccessibleHeaderBar : public VCLXAccessibleComponent
{

public:
    VclPtr<HeaderBar>  m_pHeadBar;
    virtual ~VCLXAccessibleHeaderBar();

    virtual void ProcessWindowEvent( const VclWindowEvent& rVclWindowEvent ) override;
    virtual void FillAccessibleStateSet( utl::AccessibleStateSetHelper& rStateSet ) override;

    VCLXAccessibleHeaderBar( VCLXWindow* pVCLXindow );

    // XAccessibleContext
    virtual sal_Int32 SAL_CALL getAccessibleChildCount(  ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible > SAL_CALL getAccessibleChild( sal_Int32 i ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual sal_Int16 SAL_CALL getAccessibleRole(  ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;


    // XServiceInfo
    virtual ::rtl::OUString SAL_CALL getImplementationName() throw (::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames() throw (::com::sun::star::uno::RuntimeException, std::exception) override;

public:
    virtual void SAL_CALL disposing() override;
    ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible > CreateChild(sal_Int32 i);

private:
    ListItems m_aAccessibleChildren;


};

class VCLXHeaderBar :  public VCLXWindow
{
public:
    VCLXHeaderBar(vcl::Window* pHeaderBar);
    virtual ~VCLXHeaderBar();

    virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessibleContext > CreateAccessibleContext() override;

};

#endif // INCLUDED_SVTOOLS_INC_VCLXACCESSIBLEHEADERBAR_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
