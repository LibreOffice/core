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


#ifndef _XMLOFF_XMLSECTIONFOOTNOTECONFIGEXPORT_HXX
#define _XMLOFF_XMLSECTIONFOOTNOTECONFIGEXPORT_HXX

#ifndef _SAL_TYPES_H_
#include <sal/types.h>
#endif

#include <vector>
namespace binfilter {

class SvXMLExport;
class XMLPropertySetMapper;
struct XMLPropertyState;
template<class X> class UniReference;

/**
 * Export the footnote-/endnote-configuration element in section styles.
 *
 * Because this class contains only one method, and all information is
 * available during that method call, we simply make it static.  
 */
class XMLSectionFootnoteConfigExport
{

public:
    static void exportXML(
        SvXMLExport& rExport,
        sal_Bool bEndnote,
        const ::std::vector<XMLPropertyState> * pProperties,
        sal_uInt32 nIdx,
        const UniReference<XMLPropertySetMapper> & rMapper);	/// used only for debugging
};

}//end of namespace binfilter
#endif
