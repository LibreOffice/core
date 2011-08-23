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
#ifndef INCLUDED_SVTOOLS_USEROPTIONS_HXX
#define INCLUDED_SVTOOLS_USEROPTIONS_HXX

#include "bf_svtools/svtdllapi.h"

#include <unotools/configitem.hxx>
#include <tools/string.hxx>
#include <osl/mutex.hxx>
#include <bf_svtools/brdcst.hxx>
#include <bf_svtools/lstner.hxx>
#include <bf_svtools/options.hxx>

namespace binfilter
{

// define ----------------------------------------------------------------

#define USER_OPT_CITY				((USHORT)0)
#define USER_OPT_COMPANY			((USHORT)1)
#define USER_OPT_COUNTRY			((USHORT)2)
#define USER_OPT_EMAIL				((USHORT)3)
#define USER_OPT_FAX				((USHORT)4)
#define USER_OPT_FIRSTNAME			((USHORT)5)
#define USER_OPT_LASTNAME			((USHORT)6)
#define USER_OPT_POSITION			((USHORT)7)
#define USER_OPT_STATE				((USHORT)8)
#define USER_OPT_STREET				((USHORT)9)
#define USER_OPT_TELEPHONEHOME		((USHORT)10)
#define USER_OPT_TELEPHONEWORK		((USHORT)11)
#define USER_OPT_TITLE				((USHORT)12)
#define USER_OPT_ID					((USHORT)13)
#define USER_OPT_ZIP				((USHORT)14)
#define USER_OPT_FATHERSNAME        ((USHORT)15)
#define USER_OPT_APARTMENT          ((USHORT)16)

// class SvtUserOptions --------------------------------------------------

class SvtUserOptions_Impl;

class  SvtUserOptions : public Options, public SfxBroadcaster, public SfxListener
{
private:
    SvtUserOptions_Impl*	pImp;

public:
    SvtUserOptions();
    virtual ~SvtUserOptions();

    static ::osl::Mutex&	GetInitMutex();

    virtual void        Notify( SfxBroadcaster& rBC, const SfxHint& rHint );

    // get the address token
    const String&	GetCompany() const;
    const String&	GetFirstName() const;
    const String&	GetLastName() const;
    const String&	GetID() const;
    const String&	GetStreet() const;
    const String&	GetCity() const;
    const String&	GetState() const;
    const String&	GetZip() const;
    const String&	GetCountry() const;
    const String&	GetPosition() const;
    const String&	GetTitle() const;
    const String&	GetTelephoneHome() const;
    const String&	GetTelephoneWork() const;
    const String&	GetFax() const;
    const String&	GetEmail() const;

    const String&	GetFullName() const;
};

}

#endif // #ifndef INCLUDED_SVTOOLS_USEROPTIONS_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
