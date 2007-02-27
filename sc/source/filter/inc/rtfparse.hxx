/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: rtfparse.hxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: vg $ $Date: 2007-02-27 12:35:49 $
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

#ifndef SC_RTFPARSE_HXX
#define SC_RTFPARSE_HXX

#include "eeparser.hxx"

#ifdef SC_RTFPARSE_CXX

struct ScRTFCellDefault
{
    SfxItemSet          aItemSet;
    SCCOL               nCol;
    USHORT              nTwips;         // rechter Rand der Zelle
    SCCOL               nColOverlap;    // MergeCell wenn >1, merged cells wenn 0

                        ScRTFCellDefault( SfxItemPool* pPool ) :
                            aItemSet( *pPool ), nColOverlap(1) {}
};

DECLARE_LIST( ScRTFDefaultList, ScRTFCellDefault* )
// Remove: (const unsigned short &) not sufficiently different from (unsigned short)
// deswegen ULONG, typedef bringt's auch nicht :-(
SV_DECL_VARARR_SORT( ScRTFColTwips, ULONG, 16, 4)

#else       // SC_RTFPARSE_CXX

struct ScRTFCellDefault;
class ScRTFDefaultList;
class ScRTFColTwips;

#endif      // SC_RTFPARSE_CXX


class EditEngine;

class ScRTFParser : public ScEEParser
{
private:
    ScRTFDefaultList*   pDefaultList;
    ScRTFColTwips*      pColTwips;
    ScRTFCellDefault*   pInsDefault;
    ScRTFCellDefault*   pActDefault;
    ScRTFCellDefault*   pDefMerge;
    ULONG               nStartAdjust;
    USHORT              nLastWidth;
    BOOL                bNewDef;

    DECL_LINK( RTFImportHdl, ImportInfo* );
    inline void         NextRow();
    void                EntryEnd( ScEEParseEntry*, const ESelection& );
    void                ProcToken( ImportInfo* );
    void                ColAdjust();
    BOOL                SeekTwips( USHORT nTwips, SCCOL* pCol );
    void                NewCellRow( ImportInfo* );

public:
                        ScRTFParser( EditEngine* );
    virtual             ~ScRTFParser();
    virtual ULONG       Read( SvStream&, const String& rBaseURL );
};


#endif
