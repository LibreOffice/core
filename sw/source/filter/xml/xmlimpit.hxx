/*************************************************************************
 *
 *  $RCSfile: xmlimpit.hxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: mib $ $Date: 2001-07-04 14:14:49 $
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

#ifndef _XMLIMPIT_HXX
#define _XMLIMPIT_HXX

#include <limits.h>

#ifndef _SOLAR_H
#include <tools/solar.h>
#endif
#ifndef _REF_HXX
#include <tools/ref.hxx>
#endif


#ifndef _COM_SUN_STAR_XML_SAX_SAXPARSEEXCEPTION_HPP_
#include <com/sun/star/xml/sax/SAXParseException.hpp>
#endif
#ifndef _COM_SUN_STAR_XML_SAX_XEXTENDEDDOCUMENTHANDLER_HPP_
#include <com/sun/star/xml/sax/XExtendedDocumentHandler.hpp>
#endif
#ifndef _COM_SUN_STAR_XML_SAX_SAXEXCEPTION_HPP_
#include <com/sun/star/xml/sax/SAXException.hpp>
#endif
#ifndef _COM_SUN_STAR_XML_SAX_XDOCUMENTHANDLER_HPP_
#include <com/sun/star/xml/sax/XDocumentHandler.hpp>
#endif
#ifndef _COM_SUN_STAR_XML_SAX_XATTRIBUTELIST_HPP_
#include <com/sun/star/xml/sax/XAttributeList.hpp>
#endif
#ifndef _COM_SUN_STAR_XML_SAX_XLOCATOR_HPP_
#include <com/sun/star/xml/sax/XLocator.hpp>
#endif

#ifndef _XMLITMAP_HXX
#include "xmlitmap.hxx"
#endif

namespace rtl { class OUString; }

class SvXMLUnitConverter;
class SfxPoolItem;
class SfxItemSet;
class SvXMLAttributeList;
class SvXMLNamespaceMap;
struct SvXMLItemMapEntry;

class SvXMLImportItemMapper
{
protected:
    SvXMLItemMapEntriesRef mrMapEntries;
    USHORT nUnknownWhich;

public:
    SvXMLImportItemMapper( SvXMLItemMapEntriesRef rMapEntries ,
                           USHORT nUnknWhich=USHRT_MAX );
    virtual ~SvXMLImportItemMapper();

    /** fills the given itemset with the attributes in the given list */
    void importXML( SfxItemSet& rSet,
                    ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XAttributeList > xAttrList,
                    const SvXMLUnitConverter& rUnitConverter,
                    const SvXMLNamespaceMap& rNamespaceMap ) const;

    /** this method is called for every item that has the
        MID_FLAG_SPECIAL_ITEM_IMPORT flag set */
    virtual BOOL handleSpecialItem( const SvXMLItemMapEntry& rEntry,
                                    SfxPoolItem& rItem,
                                    SfxItemSet& rSet,
                                    const ::rtl::OUString& rValue,
                                    const SvXMLUnitConverter& rUnitConverter,
                                    const SvXMLNamespaceMap& rNamespaceMap ) const;

    /** this method is called for every item that has the
        MID_FLAG_NO_ITEM_IMPORT flag set */
    virtual BOOL handleNoItem( const SvXMLItemMapEntry& rEntry,
                               SfxItemSet& rSet,
                               const ::rtl::OUString& rValue,
                               const SvXMLUnitConverter& rUnitConverter,
                               const SvXMLNamespaceMap& rNamespaceMap ) const;

    /** This method is called when all attributes have benn processed. It
      * may be used to remove items that are incomplete */
    virtual void finished( SfxItemSet& rSet ) const;

    inline void setMapEntries( SvXMLItemMapEntriesRef rMapEntries );
    inline SvXMLItemMapEntriesRef getMapEntries() const;
};

inline void
SvXMLImportItemMapper::setMapEntries( SvXMLItemMapEntriesRef rMapEntries )
{
    mrMapEntries = rMapEntries;
}

inline SvXMLItemMapEntriesRef
SvXMLImportItemMapper::getMapEntries() const
{
    return mrMapEntries;
}


#endif  //  _XMLIMPIT_HXX
