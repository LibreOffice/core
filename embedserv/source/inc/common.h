/*************************************************************************
 *
 *  $RCSfile: common.h,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: mav $ $Date: 2003-03-25 08:24:23 $
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

#ifndef _COMMON_H_
#define _COMMON_H_

#include "stdafx.h"
#include <initguid.h>

#ifndef _RTL_USTRING_HXX_
#include <rtl/ustring.hxx>
#endif

#ifndef _OSL_INTERLOCK_H_
#include <osl/interlck.h>
#endif

namespace com { namespace sun { namespace star {
    namespace lang {
        class XMultiServiceFactory;
    }
    namespace frame {
        class XModel;
    }
    namespace beans {
        struct PropertyValue;
    }
    namespace io {
        class XInputStream;
        class XOutputStream;
    }
} } }

const sal_Int32 nConstBufferSize = 32000;

#define SUPPORTED_FACTORIES_NUM 5

// {30A2652A-DDF7-45e7-ACA6-3EAB26FC8A4E}
DEFINE_GUID(OID_WriterTextServer,   0x30a2652a, 0xddf7, 0x45e7, 0xac, 0xa6, 0x3e, 0xab, 0x26, 0xfc, 0x8a, 0x4e );

// {43FEEA23-228B-4726-AFFE-214598F47289}
// DEFINE_GUID(OID_WriterWebServer, 0x43feea23, 0x228b, 0x4726, 0xaf, 0xfe, 0x21, 0x45, 0x98, 0xf4, 0x72, 0x89 );

// {1386E9B2-8706-438b-BF51-5CA7864770C5}
// DEFINE_GUID(OID_WriterGlobalServer,  0x1386e9b2, 0x8706, 0x438b, 0xbf, 0x51, 0x5c, 0xa7, 0x86, 0x47, 0x70, 0xc5 );

// {7B342DC4-139A-4a46-8A93-DB0827CCEE9C}
DEFINE_GUID(OID_CalcServer,         0x7b342dc4, 0x139a, 0x4a46, 0x8a, 0x93, 0xdb, 0x8, 0x27, 0xcc, 0xee, 0x9c );

// {41662FC2-0D57-4aff-AB27-AD2E12E7C273}
DEFINE_GUID(OID_DrawingServer,      0x41662fc2, 0xd57, 0x4aff, 0xab, 0x27, 0xad, 0x2e, 0x12, 0xe7, 0xc2, 0x73 );

// {E5A0B632-DFBA-4549-9346-E414DA06E6F8}
DEFINE_GUID(OID_PresentationServer, 0xe5a0b632, 0xdfba, 0x4549, 0x93, 0x46, 0xe4, 0x14, 0xda, 0x6, 0xe6, 0xf8 );

// {D0484DE6-AAEE-468a-991F-8D4B0737B57A}
DEFINE_GUID(OID_MathServer,         0xd0484de6, 0xaaee, 0x468a, 0x99, 0x1f, 0x8d, 0x4b, 0x7, 0x37, 0xb5, 0x7a );


#endif

