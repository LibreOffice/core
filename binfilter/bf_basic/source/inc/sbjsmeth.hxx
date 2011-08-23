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


#ifndef _SB_SBJSMETH_HXX
#define _SB_SBJSMETH_HXX

#ifndef _SB_SBMETH_HXX
#include "sbmeth.hxx"
#endif

namespace binfilter {

// Basic-Modul fuer JavaScript-Sourcen.
// Alle Basic-spezifischen Methoden muessen virtuell ueberladen und deaktiviert
// werden. Die Unterscheidung von normalen Modulen erfolgt uebr RTTI.

class SbJScriptMethod : public SbMethod
{
public:
    SbJScriptMethod( const String&, SbxDataType, SbModule* );
    virtual ~SbJScriptMethod();

    SBX_DECL_PERSIST_NODATA(SBXCR_SBX,SBXID_JSCRIPTMETH,2);
    TYPEINFO();
};

#ifndef __SB_SBJSCRIPTMETHODREF_HXX
#define __SB_SBJSCRIPTMETHODREF_HXX
SV_DECL_IMPL_REF(SbJScriptMethod)
#endif

}

#endif
