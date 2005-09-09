/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: controlpropertymap.hxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 14:05:04 $
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

#ifndef _XMLOFF_FORMS_CONTROLPROPERTYMAP_HXX_
#define _XMLOFF_FORMS_CONTROLPROPERTYMAP_HXX_

#ifndef _XMLOFF_PROPERTYSETMAPPER_HXX
#include "xmlprmap.hxx"
#endif
#ifndef _XMLOFF_XMLEXPPR_HXX
#include "xmlexppr.hxx"
#endif

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
            SvXMLAttributeList&                         _rAttrList,
            const XMLPropertyState&                     _rProperty,
            const SvXMLUnitConverter&                   _rUnitConverter,
            const SvXMLNamespaceMap&                    _rNamespaceMap,
            const ::std::vector< XMLPropertyState >*    _pProperties,
            sal_uInt32                                  _nIdx
        ) const;
    };

//.........................................................................
}   // namespace xmloff
//.........................................................................

#endif // _XMLOFF_FORMS_CONTROLPROPERTYMAP_HXX_


