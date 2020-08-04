/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#pragma once

#include <basic/sbx.hxx>
#include <rtl/ustring.hxx>
#include "filefmt.hxx"
#include <o3tl/typed_flags_set.hxx>

// This class reads in the image that's been produced by the compiler
// and manages the access to the single elements.

enum class SbiImageFlags
{
    NONE          = 0,
    EXPLICIT      = 0x0001,  // OPTION EXPLICIT is active
    COMPARETEXT   = 0x0002,  // OPTION COMPARE TEXT is active
    INITCODE      = 0x0004,  // Init-Code does exist
    CLASSMODULE   = 0x0008,  // OPTION ClassModule is active
};
namespace o3tl
{
    template<> struct typed_flags<SbiImageFlags> : is_typed_flags<SbiImageFlags, 0xf> {};
}

class SbiImage {
    friend class SbiCodeGen;            // compiler classes, that the private-

    SbxArrayRef    rTypes;          // User defined types
    SbxArrayRef    rEnums;          // Enum types
    std::vector<sal_uInt32>  mvStringOffsets; // StringId-Offsets
    std::unique_ptr<sal_Unicode[]> pStrings;        // StringPool
    std::unique_ptr<char[]>        pCode;           // Code-Image
    std::unique_ptr<char[]>        pLegacyPCode;        // Code-Image
    bool           bError;
    SbiImageFlags  nFlags;
    sal_uInt32     nStringSize;
    sal_uInt32     nCodeSize;
    sal_uInt16     nLegacyCodeSize;
    sal_uInt16     nDimBase;        // OPTION BASE value
    rtl_TextEncoding eCharSet;
                                    // temporary management-variable:
    short          nStringIdx;
    sal_uInt32     nStringOff;      // current Pos in the stringbuffer
                                    // routines for the compiler:
    void MakeStrings( short );      // establish StringPool
    void AddString( const OUString& );
    void AddCode( std::unique_ptr<char[]>, sal_uInt32 );
    void AddType(SbxObject const *);
    void AddEnum(SbxObject *);

public:
    OUString aName;          // macro name
    OUString aOUSource;      // source code
    OUString aComment;
    bool   bInit;
    bool   bFirstInit;

    SbiImage();
   ~SbiImage();
    void Clear();
    bool Load( SvStream&, sal_uInt32& nVer );
                            // nVer is set to version
                            // of image
    bool Save( SvStream&, sal_uInt32 = B_CURVERSION );
    bool IsError() const            { return bError;    }

    const char* GetCode() const     { return pCode.get();     }
    sal_uInt32  GetCodeSize() const { return nCodeSize; }
    sal_uInt16  GetBase() const     { return nDimBase;  }
    OUString    GetString( short nId ) const;
    const SbxObject* FindType (const OUString& aTypeName) const;

    const SbxArrayRef& GetEnums() const { return rEnums; }

    void        SetFlag( SbiImageFlags n ) { nFlags |= n;      }
    bool        IsFlag( SbiImageFlags n ) const { return bool(nFlags & n); }
    sal_uInt16  CalcLegacyOffset( sal_Int32 nOffset );
    sal_uInt32  CalcNewOffset( sal_Int16 nOffset );
    void        ReleaseLegacyBuffer();
    bool        ExceedsLegacyLimits();
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
