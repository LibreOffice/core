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

#ifndef _XMLOFF_ANIMEXP_HXX
#define _XMLOFF_ANIMEXP_HXX

#ifndef _COM_SUN_STAR_DRAWING_XSHAPE_HPP_ 
#include <com/sun/star/drawing/XShape.hpp>
#endif

#ifndef _UNIVERSALL_REFERENCE_HXX
#include <bf_xmloff/uniref.hxx>
#endif
namespace binfilter {

class AnimExpImpl;
class SvXMLExport;
class XMLShapeExport;

class XMLAnimationsExporter : public UniRefBase
{
    AnimExpImpl*	mpImpl;	

public:
    XMLAnimationsExporter( XMLShapeExport* pShapeExp );
    virtual ~XMLAnimationsExporter();

    void prepare( ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShape > xShape );
    void collect( ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShape > xShape );
    void exportAnimations( SvXMLExport& rExport );
};

}//end of namespace binfilter
#endif	//  _XMLOFF_ANIMEXP_HXX

