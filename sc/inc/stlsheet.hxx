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

#ifndef SC_STLSHEET_HXX
#define SC_STLSHEET_HXX

#include <svl/style.hxx>

//------------------------------------------------------------------------

#define SCSTYLEBIT_STANDARD     0x0001

//------------------------------------------------------------------------

class ScStyleSheetPool;

class ScStyleSheet : public SfxStyleSheet
{
friend class ScStyleSheetPool;

public:

    enum    Usage
    {
        UNKNOWN,
        USED,
        NOTUSED
    };

private:
    mutable ScStyleSheet::Usage eUsage;

public:
                        TYPEINFO();

                        ScStyleSheet( const ScStyleSheet& rStyle );

    virtual sal_Bool        SetParent        ( const String& rParentName );
    virtual SfxItemSet& GetItemSet       ();
    virtual sal_Bool        IsUsed           () const;
    virtual sal_Bool        HasFollowSupport () const;
    virtual sal_Bool        HasParentSupport () const;

    virtual const String& GetName() const;
    virtual const String& GetParent() const;
    virtual const String& GetFollow() const;

    virtual sal_Bool SetName( const String& );

            void                SetUsage( ScStyleSheet::Usage eUse ) const
                                    { eUsage = eUse; }
            ScStyleSheet::Usage GetUsage() const
                                    { return eUsage; }

protected:
    virtual             ~ScStyleSheet();

                ScStyleSheet( const String&     rName,
                              ScStyleSheetPool& rPool,
                              SfxStyleFamily    eFamily,
                              sal_uInt16            nMask );

    virtual void Notify( SfxBroadcaster& rBC, const SfxHint& rHint );
};

#endif     // SC_STLSHEET_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
