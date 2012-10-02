/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
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
 *  The Initial Developer of the Original Code is: IBM Corporation
 *
 *  Copyright: 2008 by IBM Corporation
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/
#ifndef BENTOID_H
#define BENTOID_H

namespace OpenStormBento
{

#define BEN_OBJID_TOC 1 // TOC object ID
#define BEN_PROPID_TOC_SEED 2 // TOC object starting seed
#define BEN_TYPEID_TOC_TYPE 19 // TOC object property value type
#define BEN_TYPEID_7_BIT_ASCII 21 // 7-bit ASCII
#define BEN_PROPID_GLOBAL_TYPE_NAME 23 // Global type name
#define BEN_PROPID_GLOBAL_PROPERTY_NAME 24 // Global property name
#define BEN_PROPID_OBJ_REFERENCES 31 // Referenced objects from an object

#define BEN_NEW_OBJECT 1 // object + property + type
#define BEN_NEW_PROPERTY 2 // property + type
#define BEN_NEW_TYPE 3 // type
#define BEN_EXPLICIT_GEN 4 // generation

#define BEN_SEGMENT_CODE_START 5
#define BEN_OFFSET4_LEN4 5 // 4-byte offset + 4-byte length
#define BEN_CONT_OFFSET4_LEN4 6 // continued 4-byte offset + 4-byte length
#define BEN_OFFSET8_LEN4 7 // 8-byte offset + 4-byte length
#define BEN_CONT_OFFSET8_LEN4 8 // continued 8-byte offset + 4-byte length
#define BEN_IMMEDIATE0 9 // immediate (length = 0)
#define BEN_IMMEDIATE1 10 // immediate (length = 1)
#define BEN_IMMEDIATE2 11 // immediate (length = 2)
#define BEN_IMMEDIATE3 12 // immediate (length = 3)
#define BEN_IMMEDIATE4 13 // immediate (length = 4)
#define BEN_CONT_IMMEDIATE4 14 // continued immediate (length = 4)
#define BEN_SEGMENT_CODE_END 14

#define BEN_REFERENCE_LIST_ID 15 // references recording object ID
#define BEN_END_OF_BUFFER 24 // end of current buffer, go to next
#define BEN_READ_PAST_END_OF_TOC 50 // Our code
#define BEN_NOOP 0xFF

}// end OpenStormBento namespace
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
