/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: xiformula.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: obo $ $Date: 2006-07-10 13:59:26 $
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

#ifndef SC_XIFORMULA_HXX
#define SC_XIFORMULA_HXX

#ifndef SC_XLFORMULA_HXX
#include "xlformula.hxx"
#endif
#ifndef SC_XIROOT_HXX
#include "xiroot.hxx"
#endif

// Formula compiler ===========================================================

class ScRangeList;
class XclImpFmlaCompImpl;

/** The formula compiler to create Calc token arrays from Excel token arrays. */
class XclImpFormulaCompiler : protected XclImpRoot
{
public:
    explicit            XclImpFormulaCompiler( const XclImpRoot& rRoot );
    virtual             ~XclImpFormulaCompiler();

    /** Creates a range list from the passed Excel token array.
        @param rStrm  Stream pointing to additional formula data (e.g. constant array data). */
    void                CreateRangeList(
                            ScRangeList& rScRanges, XclFormulaType eType,
                            const XclTokenArray& rXclTokArr, XclImpStream& rStrm );

private:
    typedef ScfRef< XclImpFmlaCompImpl > XclImpFmlaCompImplRef;
    XclImpFmlaCompImplRef mxImpl;
};

// ============================================================================

#endif

