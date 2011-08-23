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

#ifndef _SCH_STLSHEET_HXX
#define _SCH_STLSHEET_HXX

#include <bf_svtools/style.hxx>
namespace binfilter {




class SchStyleSheet : public SfxStyleSheet
{
protected:
    virtual void Load (SvStream& rIn, USHORT nVersion);
    virtual void Store(SvStream& rOut);

public:
    TYPEINFO();
    SchStyleSheet(const String& rName, SfxStyleSheetBasePool& rPool,
                  SfxStyleFamily eFamily, USHORT nMask);
    virtual ~SchStyleSheet();

    virtual BOOL SetParent (const String& rParentName);
    virtual SfxItemSet& GetItemSet();
};

} //namespace binfilter
#endif	   // _SCH_STLSHEET_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
