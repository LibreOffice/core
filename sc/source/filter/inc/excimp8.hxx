/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: excimp8.hxx,v $
 *
 *  $Revision: 1.69 $
 *
 *  last change: $Author: rt $ $Date: 2008-01-29 15:29:45 $
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

#ifndef _EXCIMP8_HXX
#define _EXCIMP8_HXX

#include <string.h>

#ifndef _IMP_OP_HXX
#include "imp_op.hxx"
#endif
#ifndef _ROOT_HXX
#include "root.hxx"
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

class SotStorage;

class ScBaseCell;
class ScRangeList;
class ScDBData;

class ScfSimpleProgressBar;

class XclImpStream;



class ImportExcel8 : public ImportExcel
{
    protected:
        ExcScenarioList         aScenList;

        BOOL                    bHasBasic;

        void                    Calccount( void );              // 0x0C
        void                    Precision( void );              // 0x0E
        void                    Delta( void );                  // 0x10
        void                    Iteration( void );              // 0x11
        void                    Note( void );                   // 0x1C
        void                    WinProtection(  void );         // 0x19
        void                    Boundsheet( void );             // 0x85
        void                    FilterMode( void );             // 0x9B
        void                    AutoFilterInfo( void );         // 0x9D
        void                    AutoFilter( void );             // 0x9E
        void                    Scenman( void );                // 0xAE
        void                    Scenario( void );               // 0xAF
        void                    ReadBasic( void );              // 0xD3
        void                    Labelsst( void );               // 0xFD

        void                    Hlink( void );                  // 0x01B8
        void                    Codename( BOOL bWBGlobals );    // 0x01BA

        virtual void            EndSheet( void );
        virtual void            PostDocLoad( void );

    public:
                                ImportExcel8( XclImpRootData& rImpData, SvStream& rStrm );
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

    inline bool                 IsActive() const    { return bActive; }
    inline bool                 IsFiltered() const  { return bAutoOrAdvanced; }
    inline SCTAB                Tab() const         { return aParam.nTab; }
    inline SCCOL                StartCol() const    { return aParam.nCol1; }
    inline SCROW                StartRow() const    { return aParam.nRow1; }
    inline SCCOL                EndCol() const      { return aParam.nCol2; }
    inline SCROW                EndRow() const      { return aParam.nRow2; }

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
    using                       List::Insert;

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
    inline void                 IncrementActiveAF() { nAFActiveCount++; }
    inline BOOL                 UseUnNamed() { return nAFActiveCount == 1; }
};

#endif

