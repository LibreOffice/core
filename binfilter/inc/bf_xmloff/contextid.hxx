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
#ifndef _XMLOFF_CONTEXTID_HXX_
#define _XMLOFF_CONTEXTID_HXX_
namespace binfilter {

/** These defines determine the unique ids for XML style-context-id's
    used in the SvXMLAutoStylePoolP.
 */

#define XML_SC_CTF_START	0x00001000
#define XML_SD_CTF_START	0x00002000
#define XML_TEXT_CTF_START	0x00003000
#define XML_SCH_CTF_START	0x00004000
#define XML_PM_CTF_START	0x00005000		// page master
#define XML_FORM_CTF_START	0x00006000

#define CTF_SD_CONTROL_SHAPE_DATA_STYLE		( XML_SD_CTF_START +  1 )
#define CTF_SD_NUMBERINGRULES_NAME			( XML_SD_CTF_START +  2 )
#define CTF_SD_SHAPE_PARA_ADJUST			( XML_SD_CTF_START +  3 )

#define CTF_FORMS_DATA_STYLE				( XML_FORM_CTF_START +  0 )

}//end of namespace binfilter
#endif	// _XMLOFF_CONTEXTID_HXX_
