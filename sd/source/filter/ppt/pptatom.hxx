/*************************************************************************
 *
 *  $RCSfile: pptatom.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2004-11-26 19:51:38 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
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

}; // namespace ppt

#endif
