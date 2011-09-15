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

#ifndef _SAMPLE_OBJECT_HXX
#define _SAMPLE_OBJECT_HXX

#include <basic/sbxfac.hxx>
#include <basic/sbxvar.hxx>
#include <basic/sbxobj.hxx>

// 1) Properties:
//    Name      R/O
//    Value     a double-value, R/W
// 2) Methods:
//    Display   display a text
//    Square    argument * argument
//    Event     call of a Basic-program
// 3) Sub-objects:
//    a collection named "elements". The access is implemented as
//    property (for the whole object) and as method (for single
//    elements, is passed through).
// This implementation is an example for a table controlled
// version that can contain many elements.
// The collection is located in COLLECTN.*, the collection's
// objects in COLLELEM.*

class SampleObject : public SbxObject
{
using SbxVariable::GetInfo;
    // Definition of a table entry. This is done here because
    // methods and properties can be declared private that way.
#if defined ( ICC ) || defined ( C50 )
public:
#endif
    typedef void( SampleObject::*pMeth )
        ( SbxVariable* pThis, SbxArray* pArgs, sal_Bool bWrite );
#if defined ( ICC )
private:
#endif

    struct Methods {
        const char* pName;      // name of an entry
        SbxDataType eType;      // data type
        pMeth pFunc;
        short nArgs;            // arguments and flags
    };
    static Methods aMethods[];  // method table

    // methods
    void Display( SbxVariable*, SbxArray*, sal_Bool );
    void Event( SbxVariable*, SbxArray*, sal_Bool );
    void Square( SbxVariable*, SbxArray*, sal_Bool );
    void Create( SbxVariable*, SbxArray*, sal_Bool );
    // fill infoblock
    SbxInfo* GetInfo( short nIdx );
    // Broadcaster Notification
    virtual void SFX_NOTIFY( SfxBroadcaster& rBC, const TypeId& rBCType,
                             const SfxHint& rHint, const TypeId& rHintType );
public:
    SampleObject( const String& );

    virtual SbxVariable* Find( const String&, SbxClassType );
};


class SampleObjectFac : public SbxFactory
{
public:
    virtual SbxObject* CreateObject( const String& );
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
