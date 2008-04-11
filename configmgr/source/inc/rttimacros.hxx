/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: rttimacros.hxx,v $
 * $Revision: 1.4 $
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

/* PLEASE DON'T DELETE ANY COMMENT LINES, ALSO IT'S UNNECESSARY. */


#ifndef CONFIGMGR_RTTIMACROS_HXX
#define CONFIGMGR_RTTIMACROS_HXX

#define RTTI_BASE(classname)    \
    virtual sal_Bool isA(const sal_Char* _pName) const  \
    {   \
        return  0 == strcmp(_pName, getStaticType());   \
    }   \
    virtual const sal_Char* getType() const { return #classname; }  \
    static const sal_Char* getStaticType() { return #classname; }

#define RTTI(classname, baseclassname)  \
    virtual sal_Bool isA(const sal_Char* _pName) const  \
    {   \
        return  0 == strcmp(_pName, getStaticType())    \
            ||  baseclassname::isA(_pName); \
    }   \
    const sal_Char* getType() const { return #classname; }  \
    static const sal_Char* getStaticType() { return #classname; }

#ifndef ISA
#define ISA(classname) isA(classname::getStaticType())
#endif

#endif
