/*************************************************************************
 *
 *  $RCSfile: excimp8.hxx,v $
 *
 *  $Revision: 1.55 $
 *
 *  last change: $Author: obo $ $Date: 2004-08-11 09:46:22 $
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



#ifndef _EXCIMP8_HXX
#define _EXCIMP8_HXX

#ifndef _LIST_HXX
#include <tools/list.hxx>
#endif
#include <string.h>

#ifndef _IMP_OP_HXX
#include "imp_op.hxx"
#endif
#ifndef _ROOT_HXX
#include "root.hxx"
#endif
#ifndef _FLTTOOLS_HXX
#include "flttools.hxx"
#endif
#ifndef _EXCSCEN_HXX
#include "excscen.hxx"
#endif
#ifndef _EXCDEFS_HXX
#include "excdefs.hxx"
#endif

#ifndef SC_FTOOLS_HXX
#include "ftools.hxx"
#endif

class SvStorage;

class ScBaseCell;
class ScRangeList;
class ScDBData;

class ScfSimpleProgressBar;

class XclImpStream;



class ImportExcel8 : public ImportExcel
{
    protected:
        ExcScenarioList         aScenList;

        BOOL                    bObjSection;

        BOOL                    bHasBasic;

        BOOL                    bFirstScl;          // only one Scl-Record has to be read per chart!

        void                    RecString( void );              // 0x07
        void                    Calccount( void );              // 0x0C
        void                    Delta( void );                  // 0x10
        void                    Iteration( void );              // 0x11
        void                    Note( void );                   // 0x1C
        void                    WinProtection(  void );         // 0x19
        void                    Cont( void );                   // 0x3C
        void                    Obj( void );                    // 0x5D
        void                    Boundsheet( void );             // 0x85
        void                    FilterMode( void );             // 0x9B
        void                    AutoFilterInfo( void );         // 0x9D
        void                    AutoFilter( void );             // 0x9E
        void                    Scenman( void );                // 0xAE
        void                    Scenario( void );               // 0xAF
        void                    ReadBasic( void );              // 0xD3
        void                    Cellmerging( void );            // 0xE5     geraten...
        void                    Msodrawinggroup( void );        // 0xEB
        void                    Msodrawing( void );             // 0xEC
        void                    Msodrawingselection( void );    // 0xED
        void                    Labelsst( void );               // 0xFD
        void                    Rstring( void );                // 0xD6
        void                    Label( void );                  // 0x0204

        void                    Txo( void );                    // 0x01B6
        void                    Hlink( void );                  // 0x01B8
        void                    Codename( BOOL bWBGlobals );    // 0x01BA
        void                    Dimensions( void );             // 0x0200

        void                    ChartEof( void );               // 0x000A
        void                    ChartScl( void );               // 0x00A0

        void                    EndSheet( void );
        virtual void            EndAllChartObjects( void );     // -> excobj.cxx
        virtual void            PostDocLoad( void );

        /** Post processes all Escher objects, and inserts them into the document. */
        void                    ApplyEscherObjects();

        virtual FltError        ReadChart8( ScfSimpleProgressBar&, const BOOL bOwnTab );

    public:
                                ImportExcel8(
                                    SfxMedium&  rMedium,
                                    SvStream&   aStream,
                                    XclBiff     eBiff,
                                    ScDocument* pDoc );

        virtual                 ~ImportExcel8( void );

        virtual FltError        Read( void );
};



//___________________________________________________________________
// classes AutoFilterData, AutoFilterBuffer

class XclImpAutoFilterData : private ExcRoot
{
private:
    ScDBData*                   pCurrDBData;
    ScQueryParam                aParam;
    SCSIZE                      nFirstEmpty;
    BOOL                        bActive;
    BOOL                        bHasDropDown;
    BOOL                        bHasConflict;
    BOOL                        bCriteria;
    BOOL                        bAutoOrAdvanced;
    ScRange                     aCriteriaRange;
    String                      aFilterName;

    void                        CreateFromDouble( String& rStr, double fVal );
    void                        SetCellAttribs();
    void                        InsertQueryParam();
    void                        AmendAFName(const BOOL bUseUnNamed);

protected:
public:
                                XclImpAutoFilterData(
                                    RootData* pRoot,
                                    const ScRange& rRange,
                                    const String& rName );

    inline SCTAB                Tab() const         { return aParam.nTab; }
    inline SCCOL                StartCol() const    { return aParam.nCol1; }
    inline SCROW                StartRow() const    { return aParam.nRow1; }
    inline SCCOL                EndCol() const      { return aParam.nCol2; }
    inline SCROW                EndRow() const      { return aParam.nRow2; }
    BOOL                        HasDropDown( SCCOL nCol, SCROW nRow, SCTAB nTab ) const;

    void                        ReadAutoFilter( XclImpStream& rStrm );

    inline void                 Activate()          { bActive = TRUE; }
    void                        SetAdvancedRange( const ScRange* pRange );
    void                        SetExtractPos( const ScAddress& rAddr );
    inline void                 SetAutoOrAdvanced()  { bAutoOrAdvanced = TRUE; }
    void                        Apply( const BOOL bUseUnNamed = FALSE );
    void                        CreateScDBData( const BOOL bUseUnNamed );
    void                        EnableRemoveFilter();
};


class XclImpAutoFilterBuffer : private List
{
private:
    UINT16                      nAFActiveCount;

    inline XclImpAutoFilterData* _First()   { return (XclImpAutoFilterData*) List::First(); }
    inline XclImpAutoFilterData* _Next()    { return (XclImpAutoFilterData*) List::Next(); }

    inline void                 Append( XclImpAutoFilterData* pData )
                                    { List::Insert( pData, LIST_APPEND ); }
protected:
public:
                                XclImpAutoFilterBuffer();
    virtual                     ~XclImpAutoFilterBuffer();

    void                        Insert( RootData* pRoot, const ScRange& rRange,
                                                const String& rName );
    void                        AddAdvancedRange( const ScRange& rRange );
    void                        AddExtractPos( const ScRange& rRange );
    void                        Apply();

    XclImpAutoFilterData*       GetByTab( SCTAB nTab );
    BOOL                        HasDropDown( SCCOL nCol, SCROW nRow, SCTAB nTab );
    inline void                 IncrementActiveAF() { nAFActiveCount++; }
    inline BOOL                 UseUnNamed() { return nAFActiveCount == 1; }
};

#endif

