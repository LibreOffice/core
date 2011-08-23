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

/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil -*- */

#ifndef _MSVBASIC_HXX
#define _MSVBASIC_HXX

#include <tools/solar.h>
#include <tools/debug.hxx>
#include <sot/storage.hxx>
#include <tools/dynary.hxx>
#include <vector>
#include <map>

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

DECLARE_DYNARRAY(StringArray,String *)

// #117718# define internal types to distinguish between
// module types, form, class & normal
// #i37965# DR 2004-12-03: add "Document", used in Excel for macros attached to sheet

// #117718# define map to hold types of module
//
typedef sal_Int32 ModType;
typedef ::std::map< UniString,
    ModType > ModuleTypeHash;

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
    ModType GetModuleType( const UniString& rModuleName );
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
