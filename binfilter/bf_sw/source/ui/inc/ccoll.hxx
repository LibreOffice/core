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
#ifndef _CCOLL_HXX
#define _CCOLL_HXX

#include <bf_svtools/poolitem.hxx>
#include <tools/string.hxx>
#include <tools/resary.hxx>
#include "cmdid.h"

namespace binfilter {

class SwWrtShell;
class SwFmt;
class SwCollCondition;


struct CollName
{
    ULONG nCnd;
    ULONG nSubCond;
};

#define COND_COMMAND_COUNT 28

struct CommandStruct
{
    ULONG nCnd;
    ULONG nSubCond;
};

class SwCondCollItem : public SfxPoolItem
{
    static CommandStruct		aCmds[COND_COMMAND_COUNT];

    String 						sStyles[COND_COMMAND_COUNT];

public:
    SwCondCollItem(USHORT nWhich = FN_COND_COLL);
    ~SwCondCollItem();

                                TYPEINFO();

    virtual SfxPoolItem*    	Clone( SfxItemPool *pPool = 0 ) const;
    virtual int             	operator==( const SfxPoolItem& ) const;

    static const CommandStruct*	GetCmds();

    const String&				GetStyle(USHORT nPos) const;
    void						SetStyle( const String* pStyle, USHORT nPos);

};

} //namespace binfilter
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
