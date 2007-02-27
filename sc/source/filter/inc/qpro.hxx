/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: qpro.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: vg $ $Date: 2007-02-27 12:35:10 $
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


#ifndef SC_QPRO_HXX
#define SC_QPRO_HXX

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
#include "qprostyle.hxx"
#include "biff.hxx"

// Stream wrapper class
class ScQProReader : public ScBiffReader
{
    public:
    bool recordsLeft();
    bool IsEndOfFile();
    void SetEof( bool bValue ){ mbEndOfFile = bValue; }
    bool nextRecord();
    sal_uInt16 getId() { return mnId; }
    sal_uInt16 getLength() { return mnLength; }
    void readString( String &rString, sal_uInt16 nLength );
    ScQProReader( SfxMedium &rMedium );
    ~ScQProReader(){ };
    FltError import( ScDocument *pDoc );
    FltError readSheet( SCTAB nTab, ScDocument* pDoc, ScQProStyle *pStyle );
};
#endif

