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

#ifndef _SBXBASE_HXX
#define _SBXBASE_HXX

#include <i18npool/lang.h>
#include "svl/svarray.hxx"
#include <basic/sbxdef.hxx>

class SbxFactory;
class SbxVariable;
class SbxBasicFormater;

SV_DECL_PTRARR_DEL(SbxFacs,SbxFactory*,5,5)

// AppData-Struktur for SBX:
struct SbxAppData
{
    SbxError            eSbxError;  // Error code
    SbxFacs             aFacs;      // Factories
    SbxBasicFormater    *pBasicFormater;    // Pointer to Format()-Command helper class

    LanguageType        eBasicFormaterLangType;
        // It might be useful to store this class 'global' because some string reosurces are saved here

    SbxAppData() : eSbxError( SbxERR_OK ), aFacs(), pBasicFormater( NULL ) {}
    ~SbxAppData();
};

SbxAppData* GetSbxData_Impl();

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
