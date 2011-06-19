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
#ifndef _CONNECTIVITY_SQLINTERNALNODE_HXX
#define _CONNECTIVITY_SQLINTERNALNODE_HXX

#include "connectivity/dbtoolsdllapi.hxx"
#include <connectivity/sqlnode.hxx>

namespace connectivity
{
    //==========================================================================
    //= OSQLInternalNode
    //==========================================================================
    /** special node for avoiding memory leaks
    */
    class OSQLInternalNode : public OSQLParseNode
    {
    public:
        OSQLInternalNode(const sal_Char* pNewValue,
                         SQLNodeType eNodeType,
                         sal_uInt32 nNodeID = 0);
        OSQLInternalNode(const ::rtl::OString& _rNewValue,
                         SQLNodeType eNodeType,
                         sal_uInt32 nNodeID = 0);
        OSQLInternalNode(const sal_Unicode* pNewValue,
                         SQLNodeType eNodeType,
                         sal_uInt32 nNodeID = 0);
        OSQLInternalNode(const ::rtl::OUString& _rNewValue,
                         SQLNodeType eNodeType,
                         sal_uInt32 nNodeID = 0);

        virtual ~OSQLInternalNode();
    };
}

#endif  //_CONNECTIVITY_SQLINTERNALNODE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
