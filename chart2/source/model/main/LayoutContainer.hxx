/*************************************************************************
 *
 *  $RCSfile: LayoutContainer.hxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: bm $ $Date: 2003-10-06 09:58:31 $
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
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2003 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/
#ifndef CHART_LAYOUTCONTAINER_HXX
#define CHART_LAYOUTCONTAINER_HXX

#ifndef _CPPUHELPER_IMPLBASE2_HXX_
#include <cppuhelper/implbase2.hxx>
#endif

#ifndef _COM_SUN_STAR_LANG_XSERVICEINFO_HPP_
#include <com/sun/star/lang/XServiceInfo.hpp>
#endif
#ifndef _DRAFTS_COM_SUN_STAR_LAYOUT_XLAYOUTCONTAINER_HPP_
#include <drafts/com/sun/star/layout/XLayoutContainer.hpp>
#endif

#include "ServiceMacros.hxx"

#include <vector>
#include <map>

namespace chart
{

class LayoutContainer : public
    ::cppu::WeakImplHelper2<
        ::com::sun::star::lang::XServiceInfo,
        ::drafts::com::sun::star::layout::XLayoutContainer >
{
public:
    LayoutContainer();
    virtual ~LayoutContainer();

    /// XServiceInfo declarations
    APPHELPER_XSERVICEINFO_DECL()

protected:
    // ____ XLayoutContainer ____
    virtual void SAL_CALL addConstrainedElementByIdentifier( const ::rtl::OUString& aIdentifier, const ::drafts::com::sun::star::layout::Constraint& Constraint )
        throw (::drafts::com::sun::star::layout::IllegalConstraintException,
               ::com::sun::star::lang::IllegalArgumentException,
               ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL addElementByIdentifier( const ::rtl::OUString& aIdentifier )
        throw (::com::sun::star::lang::IllegalArgumentException,
               ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL removeElementByIdentifier( const ::rtl::OUString& aIdentifier )
        throw (::com::sun::star::container::NoSuchElementException,
               ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setConstraintByIdentifier( const ::rtl::OUString& aIdentifier, const ::drafts::com::sun::star::layout::Constraint& Constraint )
        throw (::com::sun::star::container::NoSuchElementException,
               ::com::sun::star::uno::RuntimeException);
    virtual ::drafts::com::sun::star::layout::Constraint SAL_CALL getConstraintByIdentifier( const ::rtl::OUString& aIdentifier )
        throw (::com::sun::star::container::NoSuchElementException,
               ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence<
        ::rtl::OUString > SAL_CALL getElementIdentifiers()
        throw (::com::sun::star::uno::RuntimeException);

private:
    typedef ::std::vector< ::rtl::OUString > tLayoutElements;

    typedef ::std::map<
        ::rtl::OUString,
        ::drafts::com::sun::star::layout::Constraint > tConstraintsMap;

    tLayoutElements           m_aLayoutElements;
    tConstraintsMap           m_aConstraints;
};

} //  namespace chart

// CHART_LAYOUTCONTAINER_HXX
#endif
