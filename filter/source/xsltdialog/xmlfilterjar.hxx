/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: xmlfilterjar.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 22:18:48 $
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
#ifndef _XMLFILTERJAR_HXX_
#define _XMLFILTERJAR_HXX_

#ifndef _COM_SUN_STAR_CONTAINER_XHIERARCHICALNAMEACCESS_HPP_
#include <com/sun/star/container/XHierarchicalNameAccess.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XSINGLESERVICEFACTORY_HPP_
#include <com/sun/star/lang/XSingleServiceFactory.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XMULTISERVICEFACTORY_HPP_
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#endif

#include <vector>

class filter_info_impl;

typedef std::vector< filter_info_impl* > XMLFilterVector;

class XMLFilterJarHelper
{
public:
    XMLFilterJarHelper( com::sun::star::uno::Reference< com::sun::star::lang::XMultiServiceFactory >& xMSF );

    bool savePackage( const rtl::OUString& rPackageURL, const XMLFilterVector& rFilters );
    void openPackage( const rtl::OUString& rPackageURL, XMLFilterVector& rFilters );

private:
    void addFile( com::sun::star::uno::Reference< com::sun::star::uno::XInterface > xRootFolder, com::sun::star::uno::Reference< com::sun::star::lang::XSingleServiceFactory > xFactory, const ::rtl::OUString& rSourceFile ) throw( com::sun::star::uno::Exception );

    bool copyFile( com::sun::star::uno::Reference< com::sun::star::container::XHierarchicalNameAccess > xIfc, ::rtl::OUString& rURL, const ::rtl::OUString& rTargetURL );
    bool copyFiles( com::sun::star::uno::Reference< com::sun::star::container::XHierarchicalNameAccess > xIfc, filter_info_impl* pFilter );

    com::sun::star::uno::Reference< com::sun::star::lang::XMultiServiceFactory > mxMSF;

    ::rtl::OUString sVndSunStarPackage;
    ::rtl::OUString sXSLTPath;
    ::rtl::OUString sDTDPath;
    ::rtl::OUString sTemplatePath;
    ::rtl::OUString sSpecialConfigManager;
    ::rtl::OUString sPump;
    ::rtl::OUString sProgPath;
};

#endif
