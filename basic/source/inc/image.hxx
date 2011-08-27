/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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

#ifndef _SBIMAGE_HXX
#define _SBIMAGE_HXX

#include "sbintern.hxx"
#include <rtl/ustring.hxx>
#include <filefmt.hxx>

// This class reads in the image that's been produced by the compiler
// and manages the access to the single elements.

struct SbPublicEntry;

class SbiImage {
    friend class SbiCodeGen;            // compiler classes, that the private-

    SbxArrayRef    rTypes;          // User defined types
    SbxArrayRef    rEnums;          // Enum types
    sal_uInt32*        pStringOff;      // StringId-Offsets
    sal_Unicode*   pStrings;        // StringPool
    char*          pCode;           // Code-Image
    char*          pLegacyPCode;        // Code-Image
    sal_Bool           bError;
    sal_uInt16         nFlags;
    short          nStrings;
    sal_uInt32         nStringSize;
    sal_uInt32         nCodeSize;
    sal_uInt16         nLegacyCodeSize;
    sal_uInt16         nDimBase;        // OPTION BASE value
    rtl_TextEncoding eCharSet;
                                    // temporary management-variable:
    short          nStringIdx;
    sal_uInt32         nStringOff;      // current Pos in the stringbuffer
                                    // routines for the compiler:
    void MakeStrings( short );      // establish StringPool
    void AddString( const String& );
    void AddCode( char*, sal_uInt32 );
    void AddType(SbxObject *);
    void AddEnum(SbxObject *);

public:
    String aName;                   // macro name
    ::rtl::OUString aOUSource;      // source code
    String aComment;
    sal_Bool   bInit;
    sal_Bool   bFirstInit;

    SbiImage();
   ~SbiImage();
    void Clear();
    sal_Bool Load( SvStream&, sal_uInt32& nVer );
                            // nVer is set to version
                            // of image
    sal_Bool Load( SvStream& );
    sal_Bool Save( SvStream&, sal_uInt32 = B_CURVERSION );
    sal_Bool IsError()                  { return bError;    }

    const char* GetCode() const     { return pCode;     }
    sal_uInt32      GetCodeSize() const { return nCodeSize; }
    ::rtl::OUString& GetSource32()  { return aOUSource; }
    sal_uInt16      GetBase() const     { return nDimBase;  }
    String      GetString( short nId ) const;
    const SbxObject*  FindType (String aTypeName) const;

    SbxArrayRef GetEnums()          { return rEnums; }

    void        SetFlag( sal_uInt16 n ) { nFlags |= n;      }
    sal_uInt16      GetFlag( sal_uInt16 n ) const { return nFlags & n; }
    sal_uInt16      CalcLegacyOffset( sal_Int32 nOffset );
    sal_uInt32      CalcNewOffset( sal_Int16 nOffset );
    void        ReleaseLegacyBuffer();
    sal_Bool        ExceedsLegacyLimits();

};

#define SBIMG_EXPLICIT      0x0001  // OPTION EXPLICIT is active
#define SBIMG_COMPARETEXT   0x0002  // OPTION COMPARE TEXT is active
#define SBIMG_INITCODE      0x0004  // Init-Code does exist
#define SBIMG_CLASSMODULE   0x0008  // OPTION ClassModule is active

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
