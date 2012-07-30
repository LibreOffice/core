/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

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

    /** @return true if at least one atim with the given nRecType is found */
    inline bool hasChildAtom( sal_uInt16 nRecType ) const;

    /** @return the first child atom with nRecType or NULL */
    inline const Atom* findFirstChildAtom( sal_uInt16 nRecType ) const;

    /** @return the next child atom after pLast with nRecType or NULL */
    const Atom* findNextChildAtom( sal_uInt16 nRecType, const Atom* pLast ) const;

    /** @return the first child atom or NULL */
    inline const Atom* findFirstChildAtom() const;

    /** @return the next child atom after pLast or NULL */
    inline const Atom* findNextChildAtom( const Atom* pLast ) const;

    /** @return true if this atom is a container */
    inline bool isContainer() const;

    /** seeks to the contents of this atom */
    inline bool seekToContent() const;

    /** @return the record type */
    inline sal_uInt16 getType() const;

    /** @return the record instance */
    inline sal_uInt16 getInstance() const;

    /** @return the record length */
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
