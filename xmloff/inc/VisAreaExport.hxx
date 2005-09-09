/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: VisAreaExport.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 12:44:01 $
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

#ifndef _XMLOFF_VISAREAEXPORT_HXX
#define _XMLOFF_VISAREAEXPORT_HXX

#ifndef _VCL_MAPUNIT_HXX
#include <vcl/mapunit.hxx>
#endif

#ifndef _SAL_TYPES_H_
#include <sal/types.h>
#endif

class SvXMLExport;
class Rectangle;

namespace com { namespace sun { namespace star { namespace awt {
    struct Rectangle;
} } } }

class XMLVisAreaExport
{
public:
    // the complete export is done in the constructor
    XMLVisAreaExport(SvXMLExport& rExport, const sal_Char *pName,
            const Rectangle& aRect, const MapUnit aMapUnit);

    XMLVisAreaExport(SvXMLExport& rExport, const sal_Char *pName,
                    const com::sun::star::awt::Rectangle& aRect, const sal_Int16 nMeasureUnit );

    ~XMLVisAreaExport();
};

#endif

