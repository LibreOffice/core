/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright IBM Corporation 2010.
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
#ifndef _SVTOOLS_VCLXACCESSIBLEHEADERBAR_HXX_
#define _SVTOOLS_VCLXACCESSIBLEHEADERBAR_HXX_

#include <svtools/headbar.hxx>
#include <toolkit/awt/vclxaccessiblecomponent.hxx>
#include "toolkit/awt/vclxwindow.hxx"

class HeaderBar;

//	----------------------------------------------------
//	class VCLXAccessibleHeaderBar
//	----------------------------------------------------

typedef std::vector< ::com::sun::star::uno::WeakReference< ::com::sun::star::accessibility::XAccessible > >  ListItems;

class VCLXAccessibleHeaderBar : public VCLXAccessibleComponent
{

public:
	HeaderBar*	m_pHeadBar;
	virtual ~VCLXAccessibleHeaderBar();

    virtual void ProcessWindowEvent( const VclWindowEvent& rVclWindowEvent );
    virtual void FillAccessibleStateSet( utl::AccessibleStateSetHelper& rStateSet );

	VCLXAccessibleHeaderBar( VCLXWindow* pVCLXindow );

	// XAccessibleContext
	virtual sal_Int32 SAL_CALL getAccessibleChildCount(  ) throw (::com::sun::star::uno::RuntimeException);
	virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible > SAL_CALL getAccessibleChild( sal_Int32 i )
        throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException);
	virtual sal_Int16 SAL_CALL getAccessibleRole(  ) throw (::com::sun::star::uno::RuntimeException);


	// XServiceInfo
	virtual ::rtl::OUString SAL_CALL getImplementationName() throw (::com::sun::star::uno::RuntimeException);
	virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames() throw (::com::sun::star::uno::RuntimeException);

public:
	virtual void SAL_CALL disposing (void);
	::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible > CreateChild(sal_Int32 i);

private:
	ListItems m_aAccessibleChildren;


};

class VCLXHeaderBar :  public VCLXWindow
{
public:
    VCLXHeaderBar(Window* pHeaderBar);
    virtual ~VCLXHeaderBar();

    virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessibleContext > CreateAccessibleContext();

};

#endif // _SVTOOLS_VCLXACCESSIBLEHEADERBAR_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
