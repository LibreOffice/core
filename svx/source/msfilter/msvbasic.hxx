/*************************************************************************
 *
 *  $RCSfile: msvbasic.hxx,v $
 *
 *  $Revision: 1.10 $
 *
 *  last change: $Author: kz $ $Date: 2005-01-14 12:17:29 $
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
 *  Contributor(s): cmc@openoffice.org
 *
 *
 ************************************************************************/

/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil -*- */

#ifndef _MSVBASIC_HXX
#define _MSVBASIC_HXX

#ifdef _SOLAR_H
#include <tools/solar.h>
#endif
#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif
#ifndef _SVSTOR_HXX //autogen
#include <sot/storage.hxx>
#endif
#ifndef _DYNARY_HXX
#include <tools/dynary.hxx>
#endif
#ifndef __SGI_STL_VECTOR
#include <vector>
#endif
#include<map>

/* class VBA:
 * The VBA class provides a set of methods to handle Visual Basic For
 * Applications streams, the constructor is given the root ole2 stream
 * of the document, Open reads the VBA project file and figures out
 * the number of VBA streams, and the offset of the data within them.
 * Decompress decompresses a particular numbered stream, NoStreams returns
 * this number, and StreamName can give you the streams name. Decompress
 * will return a string with the decompressed data. The optional extra
 * argument will be set if not NULL to 1 in the case of a string overflow,
 * if I can figure out how to do that.
 *
 * Otherwise it is possible to inherit from VBA and implement a Output
 * member which gets called with each 4096 output sized block.
 *
 * cmc
 * */

DECLARE_DYNARRAY(StringArray,String *);

// #117718# define internal types to distinguish between
// module types, form, class & normal
// #i37965# DR 2004-12-03: add "Document", used in Excel for macros attached to sheet
enum ModuleType { Unknown = 0, Normal, Class, Form, Document };

// #117718# define map to hold types of module
//

typedef ::std::map< UniString,
    ModuleType > ModuleTypeHash;

class VBA_Impl
{
public:
    VBA_Impl(SvStorage &rIn, bool bCmmntd = true);
    ~VBA_Impl();
    //0 for failure, 1 for success
    bool Open( const String &rToplevel, const String &rSublevel);
    const StringArray & Decompress(sal_uInt16 nIndex, int *pOverflow=0);
    sal_uInt16 GetNoStreams() const { return nOffsets; }
    const String &GetStreamName(sal_uInt16 nIndex) const
    {
        DBG_ASSERT( nIndex < nOffsets, "Index out of range" );
        return pOffsets[ nIndex ].sName;
    }
    //I'm the method that would be made virtual to make this class
    //useful elsewhere
    void Output(int len, const sal_uInt8 *data);
    //
    // #117718# member map of module names to types of module
    ModuleType GetModuleType( const UniString& rModuleName );

    std::vector<String> maReferences;
private:
    struct VBAOffset_Impl
    {
        String sName;
        sal_uInt32 nOffset;
    };

    // #117718# member map of module names to types of module
    ModuleTypeHash mhModHash;
    SvStorageRef xVBA;
    StringArray aVBAStrings;
    String sComment;
    SvStorageRef xStor;
    VBAOffset_Impl *pOffsets;
    sal_uInt16 nOffsets;
    enum Limits {nWINDOWLEN = 4096};
    sal_uInt8 aHistory[nWINDOWLEN];
    rtl_TextEncoding meCharSet;
    bool bCommented;
    bool mbMac;
    int nLines;

    //0 for failure, anything else for success
    int ReadVBAProject(const SvStorageRef &rxVBAStorage);
    int DecompressVBA(int index, SvStorageStreamRef &rxVBAStream);
    sal_uInt8 ReadPString(SvStorageStreamRef &xVBAProject, bool bIsUnicode);
};

#endif

/* vi:set tabstop=4 shiftwidth=4 expandtab: */
