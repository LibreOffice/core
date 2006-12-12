/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: pptatom.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: kz $ $Date: 2006-12-12 16:45:39 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#ifndef _PPTATOM_HXX_
#define _PPTATOM_HXX_

#ifndef _MSDFFDEF_HXX
#include <svx/msdffdef.hxx>
#endif

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

    /** returns true if at least one atim with the given nRecType and nRecInstnace is found */
    inline bool hasChildAtom( sal_uInt16 nRecType, sal_uInt16 nRecInstance ) const;

    /** returns the first child atom with nRecType or NULL */
    inline const Atom* findFirstChildAtom( sal_uInt16 nRecType ) const;

    /** returns the next child atom after pLast with nRecType or NULL */
    const Atom* findNextChildAtom( sal_uInt16 nRecType, const Atom* pLast ) const;

    /** returns the first child atom with nRecType and nRecInstance or NULL */
    inline const Atom* findFirstChildAtom( sal_uInt16 nRecType, sal_uInt16 nRecInstance ) const;

    /** returns the next child atom after pLast with nRecType and nRecInstance or NULL */
    const Atom* findNextChildAtom( sal_uInt16 nRecType, sal_uInt16 nRecInstance, const Atom* pLast ) const;

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

inline bool Atom::hasChildAtom( sal_uInt16 nRecType, sal_uInt16 nRecInstance ) const
{
    return findFirstChildAtom( nRecType, nRecInstance ) != NULL;
}

inline const Atom* Atom::findFirstChildAtom( sal_uInt16 nRecType ) const
{
    return findNextChildAtom( nRecType, NULL );
}

inline const DffRecordHeader& Atom::getHeader() const
{
    return maRecordHeader;
}

inline const Atom* Atom::findFirstChildAtom( sal_uInt16 nRecType, sal_uInt16 nRecInstance ) const
{
    return findNextChildAtom( nRecType, nRecInstance, NULL );
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
