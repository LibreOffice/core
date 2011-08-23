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
#ifndef _INSDLG_HXX
#define _INSDLG_HXX

#include <tools/globname.hxx>
#include <bf_svtools/ownlist.hxx>
#include <bf_so3/so2ref.hxx>

#include "bf_so3/so3dllapi.h"

class SvAppletObject;

#ifndef SV_DECL_SVBASELINK2_DEFINED
#define SV_DECL_SVBASELINK2_DEFINED
SV_DECL_REF(SvBaseLink2)
#endif

namespace binfilter
{
#ifndef SO2_DECL_SVINPLACEOBJECT_DEFINED
#define SO2_DECL_SVINPLACEOBJECT_DEFINED
SO2_DECL_REF(SvInPlaceObject)
#endif

/********************** SvObjectServer ***********************************
*************************************************************************/
class SO3_DLLPUBLIC SvObjectServer
{
private:
    SvGlobalName    aClassName;
    String          aHumanName;

public:
    SvObjectServer( const SvGlobalName & rClassP, const String & rHumanP ) :
        aClassName( rClassP ),
        aHumanName( rHumanP ) {}

    const SvGlobalName &    GetClassName() const { return aClassName; }
    const String &          GetHumanName() const { return aHumanName; }
};

class SO3_DLLPRIVATE SvObjectServerList
{
    PRV_SV_DECL_OWNER_LIST(SvObjectServerList,SvObjectServer)
    SO3_DLLPRIVATE const SvObjectServer *  Get( const SvGlobalName & ) const;
    SO3_DLLPRIVATE void                    Remove( const SvGlobalName & );

    SO3_DLLPRIVATE void					FillInsertObjects();
};

}   // namespace so3

#endif // _INSDLG_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
