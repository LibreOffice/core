/*************************************************************************
 *
 *  $RCSfile: docxforms.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: obo $ $Date: 2004-11-16 10:21:59 $
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
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/


#pragma hdrstop


#ifndef _DOC_HXX
#include <doc.hxx>
#endif

#ifndef _COM_SUN_STAR_UNO_REFERENCE_HXX_
#include <com/sun/star/uno/Reference.hxx>
#endif

#ifndef _COM_SUN_STAR_CONTAINER_XNAMECONTAINER_HPP_
#include <com/sun/star/container/XNameContainer.hpp>
#endif

#ifndef _COM_SUN_STAR_LANG_XMULTISERVICEFACTORY_HPP_
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#endif

#ifndef _COM_SUN_STAR_XFORMS_XMODEL_HPP_
#include <com/sun/star/xforms/XModel.hpp>
#endif

#ifndef _COM_SUN_STAR_XFORMS_XFORMSUIHELPER1_HPP_
#include <com/sun/star/xforms/XFormsUIHelper1.hpp>
#endif

#ifndef _UNOTOOLS_PROCESSFACTORY_HXX_
#include <unotools/processfactory.hxx>
#endif


using com::sun::star::uno::Reference;
using com::sun::star::uno::XInterface;
using com::sun::star::uno::UNO_QUERY;
using com::sun::star::uno::makeAny;
using com::sun::star::container::XNameContainer;
using com::sun::star::xforms::XModel;
using com::sun::star::xforms::XFormsUIHelper1;
using rtl::OUString;


Reference<XNameContainer> SwDoc::getXForms() const
{
    return xXForms;
}

bool SwDoc::isXForms() const
{
    return xXForms.is();
}

Reference<XInterface> lcl_createInstance( const sal_Char* pServiceName )
{
    DBG_ASSERT( pServiceName != NULL, "no service name" );
    return utl::getProcessServiceFactory()->createInstance(
        OUString::createFromAscii( pServiceName  ) );
}

void SwDoc::initXForms( bool bCreateDefaultModel )
{
    DBG_ASSERT( ! isXForms(), "please initialize only once" );

    // create XForms components
    xXForms.set( lcl_createInstance( "com.sun.star.xforms.XForms" ),
                 UNO_QUERY );
    DBG_ASSERT( xXForms.is(), "can't create XForms container" );

    // create default model
    if( bCreateDefaultModel && xXForms.is() )
    {
        OUString sName(RTL_CONSTASCII_USTRINGPARAM("Model 1"));
        Reference<XModel> xModel(
            lcl_createInstance( "com.sun.star.xforms.Model" ),
            UNO_QUERY );
        DBG_ASSERT( xModel.is(), "no model?" );
        if( xModel.is() )
        {
            xModel->setID( sName );
            Reference<XFormsUIHelper1>( xModel, UNO_QUERY )->newInstance(
                OUString(RTL_CONSTASCII_USTRINGPARAM("Instance 1")),
                OUString(), sal_True );
            xModel->initialize();
            xXForms->insertByName( sName, makeAny( xModel ) );
        }
        DBG_ASSERT( xXForms->hasElements(), "can't create XForms model" );
    }

    DBG_ASSERT( isXForms(), "initialization failed" );
}
