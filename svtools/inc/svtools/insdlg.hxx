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
#ifndef _SVTOOLS_INSDLG_HXX
#define _SVTOOLS_INSDLG_HXX

#include "svtools/svtdllapi.h"
#include <com/sun/star/uno/Reference.h>
#include <com/sun/star/embed/XEmbeddedObject.hpp>
#include <com/sun/star/embed/XStorage.hpp>
#include <tools/globname.hxx>
#include <sot/formats.hxx>

#include <svl/ownlist.hxx>
#include <svtools/transfer.hxx>

class SvObjectServer
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

class SVT_DLLPUBLIC SvObjectServerList
{
    PRV_SV_DECL_OWNER_LIST(SvObjectServerList,SvObjectServer)
    const SvObjectServer *  Get( const String & rHumanName ) const;
    const SvObjectServer *  Get( const SvGlobalName & ) const;
    void                    Remove( const SvGlobalName & );
    void                    FillInsertObjects();
};

class SVT_DLLPUBLIC SvPasteObjectHelper
{
public:
    static String GetSotFormatUIName( SotFormatStringId nId );
    static sal_Bool GetEmbeddedName(const TransferableDataHelper& rData, String& _rName, String& _rSource, SotFormatStringId& _nFormat);
};

#endif // _SVTOOLS_INSDLG_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
