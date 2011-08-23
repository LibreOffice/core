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

#ifndef _XMLOFF_FORMS_CONTROLPROPERTYMAP_HXX_
#define _XMLOFF_FORMS_CONTROLPROPERTYMAP_HXX_

#ifndef _XMLOFF_PROPERTYSETMAPPER_HXX 
#include "xmlprmap.hxx"
#endif
#ifndef _XMLOFF_XMLEXPPR_HXX 
#include "xmlexppr.hxx"
#endif
namespace binfilter {

struct XMLPropertyMapEntry;
//.........................................................................
namespace xmloff
{
//.........................................................................

    const XMLPropertyMapEntry* getControlStylePropertyMap( );

    void initializePropertyMaps();

    //=====================================================================
    //= OFormExportPropertyMapper
    //=====================================================================
    class OFormExportPropertyMapper : public SvXMLExportPropertyMapper
    {
    public:
        OFormExportPropertyMapper( const UniReference< XMLPropertySetMapper >& _rMapper );

        void handleSpecialItem(
            SvXMLAttributeList&							_rAttrList,
            const XMLPropertyState&						_rProperty,
            const SvXMLUnitConverter&					_rUnitConverter,
            const SvXMLNamespaceMap&					_rNamespaceMap,
            const ::std::vector< XMLPropertyState >*	_pProperties,
            sal_uInt32									_nIdx
        ) const;
    };

//.........................................................................
}	// namespace xmloff
//.........................................................................

}//end of namespace binfilter
#endif // _XMLOFF_FORMS_CONTROLPROPERTYMAP_HXX_


