/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: contextid.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2007-04-11 13:26:21 $
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
#ifndef _XMLOFF_CONTEXTID_HXX_
#define _XMLOFF_CONTEXTID_HXX_

/** These defines determine the unique ids for XML style-context-id's
    used in the SvXMLAutoStylePoolP.
 */

#define XML_SC_CTF_START    0x00001000
#define XML_SD_CTF_START    0x00002000
#define XML_TEXT_CTF_START  0x00003000
#define XML_SCH_CTF_START   0x00004000
#define XML_PM_CTF_START    0x00005000      // page master
#define XML_FORM_CTF_START  0x00006000
#define XML_DB_CTF_START    0x00007000


#define CTF_SD_CONTROL_SHAPE_DATA_STYLE     ( XML_SD_CTF_START +  1 )
#define CTF_SD_NUMBERINGRULES_NAME          ( XML_SD_CTF_START +  2 )
#define CTF_SD_SHAPE_PARA_ADJUST            ( XML_SD_CTF_START +  3 )

#define CTF_FORMS_DATA_STYLE                ( XML_FORM_CTF_START +  0 )

#endif  // _XMLOFF_CONTEXTID_HXX_
