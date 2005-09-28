/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: xiname.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: hr $ $Date: 2005-09-28 12:00:31 $
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

#ifndef SC_XINAME_HXX
#define SC_XINAME_HXX

#include <map>

#ifndef SC_XLNAME_HXX
#include "xlname.hxx"
#endif
#ifndef SC_XIROOT_HXX
#include "xiroot.hxx"
#endif

//class ScDocument;
//class ScTokenArray;

// ============================================================================

class ScRangeData;

/** Represents a defined name. It may be related to a single sheet or global. */
class XclImpName : protected XclImpRoot
{
public:
    explicit            XclImpName( XclImpStream& rStrm, sal_uInt16 nXclNameIdx );

    inline const String& GetXclName() const { return maXclName; }
    inline const String& GetScName() const { return maScName; }
    inline SCTAB        GetScTab() const { return mnScTab; }
    inline const ScRangeData* GetScRangeData() const { return mpScData; }
    inline bool         IsGlobal() const { return mnScTab == SCTAB_MAX; }
    inline bool         IsVBName() const { return mbVBName; }

private:
    String              maXclName;      /// Original name read from the file.
    String              maScName;       /// Name inserted into the Calc document.
    const ScRangeData*  mpScData;       /// Pointer to Calc defined name (no ownership).
    sal_Unicode         mcBuiltIn;      /// Excel built-in name index.
    SCTAB               mnScTab;        /// Calc sheet index of local names.
    bool                mbVBName;       /// true = Visual Basic procedure.
};

// ----------------------------------------------------------------------------

/** This buffer contains all internal defined names of the document.
    @descr  It manages the position of the names in the document, means if they are
    global or attached to a specific sheet. While inserting the names into the Calc
    document this buffer resolves conflicts caused by equal names from different
    sheets. */
class XclImpNameManager : protected XclImpRoot
{
public:
    explicit            XclImpNameManager( const XclImpRoot& rRoot );

    /** Reads a NAME record and creates an entry in this buffer. */
    void                ReadName( XclImpStream& rStrm );

    /** Tries to find the name used in Calc, based on the original Excel defined name.
        @param nScTab  The sheet index for local names or SCTAB_MAX for global names.
        If no local name is found, tries to find a matching global name.
        @return  Pointer to the defined name or 0 on error. */
    const XclImpName*   FindName( const String& rXclName, SCTAB nScTab = SCTAB_MAX ) const;

    /** Returns the defined name specified by its Excel index.
        @param nXclNameIdx  The index of the internal defined name.
        @return  Pointer to the defined name or 0 on error. */
    const XclImpName*   GetName( sal_uInt16 nXclNameIdx ) const;

private:
    typedef ScfDelList< XclImpName > XclImpNameList;
    XclImpNameList      maNameList;
};

// ============================================================================

#endif

