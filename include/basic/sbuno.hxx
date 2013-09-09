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

#ifndef _SB_SBUNO_HXX
#define _SB_SBUNO_HXX

#include <com/sun/star/beans/Property.hpp>
#include <com/sun/star/uno/Any.hxx>
#include <com/sun/star/uno/Type.hxx>
#include <basic/sbxobj.hxx>
#include "basicdllapi.h"

namespace com { namespace sun { namespace star { namespace uno { class Any; }}}}

// Returns a SbxObject that wrapps an Uno Interface
// Implementation in basic/source/classes/sbunoobj.cxx
BASIC_DLLPUBLIC SbxObjectRef GetSbUnoObject( const OUString& aName, const com::sun::star::uno::Any& aUnoObj_ );

// Force creation of all properties for debugging
BASIC_DLLPUBLIC void createAllObjectProperties( SbxObject* pObj );
BASIC_DLLPUBLIC void SetSbUnoObjectDfltPropName( SbxObject* pObj );

BASIC_DLLPUBLIC ::com::sun::star::uno::Any sbxToUnoValue( const SbxValue* pVar );
BASIC_DLLPUBLIC ::com::sun::star::uno::Any sbxToUnoValue( const SbxValue* pVar, const ::com::sun::star::uno::Type& rType, com::sun::star::beans::Property* pUnoProperty = NULL );

BASIC_DLLPUBLIC void unoToSbxValue( SbxVariable* pVar, const ::com::sun::star::uno::Any& aValue );

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
