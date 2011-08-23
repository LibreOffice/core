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

#ifndef _SCH_OBJFAC_HXX
#define _SCH_OBJFAC_HXX

#ifndef _SOLAR_H
#include <tools/solar.h>
#endif

#ifndef _LINK_HXX
#include <tools/link.hxx>
#endif
namespace binfilter {


/*************************************************************************
|*
|* Factory fuer Chart-spezifische Objekte
|*
\************************************************************************/

class SdrObjFactory;

class SchObjFactory
{
private:
    BOOL bInserted;

public:
    SchObjFactory();

    void SetInserted(BOOL bIns) { bInserted = bIns; }
    BOOL GetInserted() const { return bInserted; }

    DECL_LINK( MakeObject, SdrObjFactory * );
    DECL_LINK( MakeUserData, SdrObjFactory * );
};

} //namespace binfilter
#endif // _SCH_OBJFAC_HXX


