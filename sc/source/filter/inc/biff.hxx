/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: biff.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: hr $ $Date: 2006-01-27 15:49:52 $
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


#ifndef SC_BASE_HXX
#define SC_BASE_HXX

#include <sal/config.h>
#include "filter.hxx"
#include "document.hxx"
#include "cell.hxx"

#ifndef _STRING_HXX
#include <tools/string.hxx>
#endif

#include <tools/color.hxx>
#include "flttypes.hxx"
#include "ftools.hxx"

// Stream wrapper class
class ScBiffReader
{
    protected:
    sal_uInt16 mnId;
    sal_uInt16 mnLength;
    sal_uInt32 mnOffset;
    SvStream *mpStream;
    bool mbEndOfFile;

    public:
    ScBiffReader( SfxMedium& rMedium );
    ~ScBiffReader();
    bool recordsLeft() { return mpStream && !mpStream->IsEof(); }
    bool IsEndOfFile() { return mbEndOfFile; }
    void SetEof( bool bValue ){ mbEndOfFile = bValue; }
    bool nextRecord();
    sal_uInt16 getId() { return mnId; }
    sal_uInt16 getLength() { return mnLength; }
    SvStream& getStream() { return *mpStream; }
};
#endif

