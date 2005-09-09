/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: SchXMLAutoStylePoolP.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 13:23:56 $
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
#ifndef _SCH_XMLAUTOSTYLEPOOLP_HXX_
#define _SCH_XMLAUTOSTYLEPOOLP_HXX_

#ifndef _XMLOFF_XMLASTPLP_HXX
#include "xmlaustp.hxx"
#endif

class SchXMLExport;

class SchXMLAutoStylePoolP : public SvXMLAutoStylePoolP
{
protected:
    SchXMLExport& mrSchXMLExport;

    virtual void exportStyleAttributes(
        SvXMLAttributeList& rAttrList,
        sal_Int32 nFamily,
        const ::std::vector< XMLPropertyState >& rProperties,
        const SvXMLExportPropertyMapper& rPropExp
        , const SvXMLUnitConverter& rUnitConverter,
        const SvXMLNamespaceMap& rNamespaceMap
        ) const;

public:
    SchXMLAutoStylePoolP( SchXMLExport& rSchXMLExport );
    virtual ~SchXMLAutoStylePoolP();
};

#endif  // _SCH_XMLAUTOSTYLEPOOLP_HXX_
