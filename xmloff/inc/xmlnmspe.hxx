/*************************************************************************
 *
 *  $RCSfile: xmlnmspe.hxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 17:07:02 $
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

#ifndef _XMLOFF_XMLNMSPE_HXX
#define _XMLOFF_XMLNMSPE_HXX

#ifndef _SAL_TYPES_H_
#include <sal/types.h>
#endif

#define XML_NAMESPACE( prefix, key ) \
const sal_uInt16 XML_NAMESPACE_##prefix         = key; \
const sal_uInt16 XML_NAMESPACE_##prefix##_IDX   = key;

#define XML_OLD_NAMESPACE( prefix, index ) \
const sal_uInt16 XML_OLD_NAMESPACE_##prefix##_IDX = \
    (_XML_OLD_NAMESPACE_BASE+index);

// current namespaces
// These namespaces have the same index in the namespace table as prefix used.
// If a namespace is added, _XML_OLD_NAMESPACE_BASE has to be adjusted!
XML_NAMESPACE( OFFICE,          0U )
XML_NAMESPACE( STYLE,           1U )
XML_NAMESPACE( TEXT ,           2U )
XML_NAMESPACE( TABLE,           3U )
XML_NAMESPACE( DRAW ,           4U )
XML_NAMESPACE( FO   ,           5U )
XML_NAMESPACE( XLINK,           6U )
XML_NAMESPACE( DC   ,           7U )
XML_NAMESPACE( META ,           8U )
XML_NAMESPACE( NUMBER,          9U )
XML_NAMESPACE( PRESENTATION,    10U )
XML_NAMESPACE( SVG,             11U )
XML_NAMESPACE( CHART,           12U )

#define _XML_OLD_NAMESPACE_BASE 13U

// namespaces used in the technical preview (SO 5.2)
XML_OLD_NAMESPACE( FO,      0U )
XML_OLD_NAMESPACE( XLINK,   1U )

XML_OLD_NAMESPACE( OFFICE,  2U )
XML_OLD_NAMESPACE( STYLE,   3U )
XML_OLD_NAMESPACE( TEXT,    4U )
XML_OLD_NAMESPACE( TABLE,   5U )
XML_OLD_NAMESPACE( META,    6U )



#endif  //  _XMLOFF_XMLNMSPE_HXX
