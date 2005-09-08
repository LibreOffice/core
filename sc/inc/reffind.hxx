/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: reffind.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 17:51:35 $
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

#ifndef SC_REFFIND_HXX
#define SC_REFFIND_HXX

#ifndef _STRING_HXX //autogen
#include <tools/string.hxx>
#endif

class ScDocument;

// -----------------------------------------------------------------------

class ScRefFinder
{
private:
    String      aFormula;
    ScDocument* pDoc;
    xub_StrLen  nFound;
    xub_StrLen  nSelStart;
    xub_StrLen  nSelEnd;

public:
    static const sal_Unicode __FAR_DATA pDelimiters[];

    ScRefFinder( const String& rFormula, ScDocument* pDocument = NULL );
    ~ScRefFinder();

    const String&   GetText() const     { return aFormula; }
    xub_StrLen      GetFound() const    { return nFound; }
    xub_StrLen      GetSelStart() const { return nSelStart; }
    xub_StrLen      GetSelEnd() const   { return nSelEnd; }

    void            ToggleRel( xub_StrLen nStartPos, xub_StrLen nEndPos );
};



#endif


