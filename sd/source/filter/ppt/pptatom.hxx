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

#ifndef _PPTATOM_HXX_
#define _PPTATOM_HXX_

#include <svx/msdffdef.hxx>
#include <filter/msfilter/msdffimp.hxx>

class SvStream;

namespace ppt
{

class Atom
{
public:
    ~Atom();

    /** imports this atom and its child atoms */
    static Atom* import( const DffRecordHeader& rRootRecordHeader, SvStream& rStCtrl );

    inline const DffRecordHeader& getHeader() const;

    /** returns true if at least one atim with the given nRecType is found */
    inline bool hasChildAtom( sal_uInt16 nRecType ) const;

    /** returns the first child atom with nRecType or NULL */
    inline const Atom* findFirstChildAtom( sal_uInt16 nRecType ) const;

    /** returns the next child atom after pLast with nRecType or NULL */
    const Atom* findNextChildAtom( sal_uInt16 nRecType, const Atom* pLast ) const;

    /** returns the first child atom or NULL */
    inline const Atom* findFirstChildAtom() const;

    /** returns the next child atom after pLast or NULL */
    inline const Atom* findNextChildAtom( const Atom* pLast ) const;

    /** returns true if this atom is a container */
    inline bool isContainer() const;

    /** seeks to the contents of this atom */
    inline bool seekToContent() const;

    /** returns the record type */
    inline sal_uInt16 getType() const;

    /** returns the record instance */
    inline sal_uInt16 getInstance() const;

    /** returns the record length */
    inline sal_uInt32 getLength() const;

private:
    Atom( const DffRecordHeader& rRecordHeader, SvStream& rStCtrl );

    SvStream& mrStream;
    DffRecordHeader maRecordHeader;
    Atom* mpFirstChild;
    Atom* mpNextAtom;
};

inline bool Atom::hasChildAtom( sal_uInt16 nRecType ) const
{
    return findFirstChildAtom( nRecType ) != NULL;
}

inline const Atom* Atom::findFirstChildAtom( sal_uInt16 nRecType ) const
{
    return findNextChildAtom( nRecType, NULL );
}

inline const DffRecordHeader& Atom::getHeader() const
{
    return maRecordHeader;
}

inline const Atom* Atom::findFirstChildAtom() const
{
    return mpFirstChild;
}

inline const Atom* Atom::findNextChildAtom( const Atom* pLast ) const
{
    return pLast ? pLast->mpNextAtom : pLast;
}

inline bool Atom::isContainer() const
{
    return (bool)maRecordHeader.IsContainer();
}

inline bool Atom::seekToContent() const
{
    maRecordHeader.SeekToContent( mrStream );
    return mrStream.GetError() == 0;
}

inline sal_uInt16 Atom::getType() const
{
    return maRecordHeader.nRecType;
}

inline sal_uInt16 Atom::getInstance() const
{
    return maRecordHeader.nRecInstance;
}

inline sal_uInt32 Atom::getLength() const
{
    return maRecordHeader.nRecLen;
}

} // namespace ppt

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
