/*************************************************************************
 *
 *  $RCSfile: sdpropls.hxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 17:07:03 $
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

#ifndef _SDPROPLS_HXX
#define _SDPROPLS_HXX

#ifndef _XMLOFF_PROPMAPPINGTYPES_HXX
#include <maptype.hxx>
#endif

#ifndef _XMLOFF_XMLTYPES_HXX
#include <xmltypes.hxx>
#endif

#ifndef _XMLOFF_PROPERTYHANDLERFACTORY_HXX
#include <prhdlfac.hxx>
#endif

//////////////////////////////////////////////////////////////////////////////
// entry list for graphic properties

extern const XMLPropertyMapEntry aXMLSDProperties[];

//////////////////////////////////////////////////////////////////////////////
// entry list for presentation page properties

extern const XMLPropertyMapEntry aXMLSDPresPageProps[];

//////////////////////////////////////////////////////////////////////////////
// types of own properties

#define XML_SD_TYPE_STROKE                          (XML_SD_TYPES_START +  0)
#define XML_SD_TYPE_PRESPAGE_TYPE                   (XML_SD_TYPES_START +  1)
#define XML_SD_TYPE_PRESPAGE_STYLE                  (XML_SD_TYPES_START +  2)
#define XML_SD_TYPE_PRESPAGE_SPEED                  (XML_SD_TYPES_START +  3)
#define XML_SD_TYPE_PRESPAGE_DURATION               (XML_SD_TYPES_START +  4)
// still missing:
//#define   XML_SD_TYPE_PRESPAGE_VISIBILITY             (XML_SD_TYPES_START +  5)
//#define   XML_SD_TYPE_PRESPAGE_SOUND                  (XML_SD_TYPES_START +  6)
#define XML_SD_TYPE_DASHARRAY                       (XML_SD_TYPES_START + 7 )
#define XML_SD_TYPE_MARKER                          (XML_SD_TYPES_START + 8 )
#define XML_SD_TYPE_OPACITY                         (XML_SD_TYPES_START + 9 )
#define XML_SD_TYPE_LINEJOIN                        (XML_SD_TYPES_START + 10 )
#define XML_SD_TYPE_FILLSTYLE                       (XML_SD_TYPES_START + 11 )
#define XML_SD_TYPE_GRADIENT                        (XML_SD_TYPES_START + 12 )
#define XML_SD_TYPE_GRADIENT_STEPCOUNT              (XML_SD_TYPES_START + 13 )
#define XML_SD_TYPE_SHADOW                          (XML_SD_TYPES_START + 14 )
#define XML_SD_TYPE_COLORMODE                       (XML_SD_TYPES_START + 15 )


//////////////////////////////////////////////////////////////////////////////
// factory for own graphic properties

class XMLSdPropHdlFactory : public XMLPropertyHandlerFactory
{
public:
    virtual ~XMLSdPropHdlFactory();
    virtual const XMLPropertyHandler* GetPropertyHandler( sal_Int32 nType ) const;
};


#endif  //  _SDPROPLS_HXX
