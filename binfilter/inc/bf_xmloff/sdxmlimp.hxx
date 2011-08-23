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

#ifndef _SDXMLIMP_HXX
#define _SDXMLIMP_HXX

#ifndef _COM_SUN_STAR_FRAME_XMODEL_HPP_
#include <com/sun/star/frame/XModel.hpp>
#endif

#ifndef _COM_SUN_STAR_CONTAINER_XINDEXCONTAINER_HPP_ 
#include <com/sun/star/container/XIndexContainer.hpp>
#endif


#ifndef _COM_SUN_STAR_XML_SAX_XDOCUMENTHANDLER_HPP_ 
#include <com/sun/star/xml/sax/XDocumentHandler.hpp>
#endif

#ifndef _COM_SUN_STAR_TASK_XSTATUSINDICATOR_HPP_
#include <com/sun/star/task/XStatusIndicator.hpp>
#endif
namespace binfilter {

::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XDocumentHandler >
    CreateSdXMLImport(
        ::com::sun::star::uno::Reference< ::com::sun::star::frame::XModel >& rMod,
        ::com::sun::star::uno::Reference< ::com::sun::star::container::XIndexContainer >& rGrfContainer,
        ::com::sun::star::uno::Reference< ::com::sun::star::task::XStatusIndicator >& rStatusIndicator,
        sal_Bool bLoadDoc, 
        sal_uInt16 nStyleFamMask, 
        sal_Bool bShowProgr, 
        sal_Bool bIsDraw );

}//end of namespace binfilter
#endif	//  _SDXMLIMP_HXX
