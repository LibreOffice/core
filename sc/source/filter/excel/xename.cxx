/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: xename.cxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: rt $ $Date: 2005-10-21 11:57:01 $
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

#ifndef SC_XENAME_HXX
#include "xename.hxx"
#endif

#include <map>

#ifndef __GLOBSTR_HRC_
#include "globstr.hrc"
#endif
#ifndef SC_DOCUMENT_HXX
#include "document.hxx"
#endif
#ifndef SC_RANGENAM_HXX
#include "rangenam.hxx"
#endif
#ifndef SC_DBCOLECT_HXX
#include "dbcolect.hxx"
#endif

#ifndef SC_XEHELPER_HXX
#include "xehelper.hxx"
#endif
#ifndef SC_XELINK_HXX
#include "xelink.hxx"
#endif

// for filter manager
#include "excrecds.hxx"

// ============================================================================
// *** Helper classes ***
// ============================================================================

/** Represents an internal defined name, supports writing it to a NAME record. */
class XclExpName : public XclExpRecord, protected XclExpRoot
{
public:
    /** Creates a standard defined name. */
    explicit            XclExpName( const XclExpRoot& rRoot, const String& rName );
    /** Creates a built-in defined name. */
    explicit            XclExpName( const XclExpRoot& rRoot, sal_Unicode cBuiltIn );

    /** Sets a token array containing the definition of this name. */
    void                SetTokenArray( XclExpTokenArrayRef xTokArr );
    /** Changes this defined name to be local on the specified Calc sheet. */
    void                SetLocalTab( SCTAB nScTab );
    /** Hides or unhides the defined name. */
    void                SetHidden( bool bHidden = true );
    /** Changes this name to be the call to a VB macro function or procedure.
        @param bVBasic  true = Visual Basic macro, false = Sheet macro.
        @param bFunc  true = Macro function; false = Macro procedure. */
    void                SetMacroCall( bool bVBasic, bool bFunc );

    /** Returns the original name (title) of this defined name. */
    inline const String& GetOrigName() const { return maOrigName; }
    /** Returns the Excel built-in name index of this defined name.
        @return  The built-in name index or EXC_BUILTIN_UNKNOWN for user-defined names. */
    inline sal_Unicode  GetBuiltInName() const { return mcBuiltIn; }

    /** Returns the token array for this defined name. */
    inline XclExpTokenArrayRef GetTokenArray() const { return mxTokArr; }

    /** Returns true, if this is a document-global defined name. */
    inline bool         IsGlobal() const { return mnXclTab == EXC_NAME_GLOBAL; }
    /** Returns the Calc sheet of a local defined name. */
    inline SCTAB        GetScTab() const { return mnScTab; }

    /** Returns true, if this defined name is volatile. */
    bool                IsVolatile() const;
    /** Returns true, if this defined name is hidden. */
    bool                IsHidden() const;
    /** Returns true, if this defined name describes a macro call.
        @param bFunc  true = Macro function; false = Macro procedure. */
    bool                IsMacroCall( bool bVBasic, bool bFunc ) const;

    /** Writes the entire NAME record to the passed stream. */
    virtual void        Save( XclExpStream& rStrm );

private:
    /** Writes the body of the NAME record to the passed stream. */
    virtual void        WriteBody( XclExpStream& rStrm );

private:
    String              maOrigName;     /// The original user-defined name.
    XclExpStringRef     mxName;         /// The name as Excel string object.
    XclExpTokenArrayRef mxTokArr;       /// The definition of the defined name.
    sal_Unicode         mcBuiltIn;      /// The built-in index for built-in names.
    SCTAB               mnScTab;        /// The Calc sheet index for local names.
    sal_uInt16          mnFlags;        /// Additional flags for this defined name.
    sal_uInt16          mnExtSheet;     /// The 1-based index to a global EXTERNSHEET record.
    sal_uInt16          mnXclTab;       /// The 1-based Excel sheet index for local names.
};

// ----------------------------------------------------------------------------

class ScRangeData;
class ScDBData;

/** Implementation class of the name manager. */
class XclExpNameManagerImpl : protected XclExpRoot
{
public:
    explicit            XclExpNameManagerImpl( const XclExpRoot& rRoot );

    /** Creates NAME records for built-in and user defined names. */
    void                Initialize();

    /** Inserts the Calc name with the passed index and returns the Excel NAME index. */
    sal_uInt16          InsertName( USHORT nScNameIdx );
    /** Inserts the Calc database range with the passed index and returns the Excel NAME index. */
    sal_uInt16          InsertDBRange( USHORT nScDBRangeIdx );

    /** Inserts a new built-in defined name. */
    sal_uInt16          InsertBuiltInName( sal_Unicode cBuiltIn, XclExpTokenArrayRef xTokArr, SCTAB nScTab );
    /** Inserts a new defined name. Sets another unused name, if rName already exists. */
    sal_uInt16          InsertUniqueName( const String& rName, XclExpTokenArrayRef xTokArr, SCTAB nScTab );
    /** Returns index of an existing name, or creates a name without definition. */
    sal_uInt16          InsertRawName( const String& rName );
    /** Searches or inserts a defined name describing a macro name.
        @param bVBasic  true = Visual Basic macro; false = Sheet macro.
        @param bFunc  true = Macro function; false = Macro procedure. */
    sal_uInt16          InsertMacroCall( const String& rMacroName, bool bVBasic, bool bFunc, bool bHidden );

    /** Returns the NAME record at the specified position or 0 on error. */
    const XclExpName*   GetName( sal_uInt16 nNameIdx ) const;

    /** Writes the entire list of NAME records.
        @descr  In BIFF7 and lower, writes the entire global link table, which
            consists of an EXTERNCOUNT record, several EXTERNSHEET records, and
            the list of NAME records. */
    void                Save( XclExpStream& rStrm );

private:
    typedef XclExpRecordList< XclExpName >      XclExpNameList;
    typedef XclExpNameList::RecordRefType       XclExpNameRef;
    typedef ::std::map< USHORT, sal_uInt16 >    XclExpIndexMap;

private:
    /** Finds the index of a NAME record from the passed Calc index in the specified map. */
    sal_uInt16          FindNameIdx( const XclExpIndexMap& rMap, USHORT nScIdx ) const;
    /** Returns the index of an existing built-in NAME record with the passed definition, otherwise 0. */
    sal_uInt16          FindBuiltInNameIdx( const String& rName,
                            const XclExpTokenArray& rTokArr, bool bDBRange ) const;
    /** Returns an unused name for the passed name. */
    String              GetUnusedName( const String& rName ) const;

    /** Appends a new NAME record to the record list.
        @return  The 1-based NAME record index used elsewhere in the Excel file. */
    sal_uInt16          Append( XclExpNameRef xName );
    /** Creates a new NAME record for the passed user-defined name.
        @return  The 1-based NAME record index used elsewhere in the Excel file. */
    sal_uInt16          CreateName( const ScRangeData& rRangeData );
    /** Creates a new NAME record for the passed database range.
        @return  The 1-based NAME record index used elsewhere in the Excel file. */
    sal_uInt16          CreateName( const ScDBData& rDBData );

    /** Creates NAME records for all built-in names in the document. */
    void                CreateBuiltInNames();
    /** Creates NAME records for all user-defined names in the document. */
    void                CreateUserNames();
    /** Creates NAME records for all database ranges in the document. */
    void                CreateDatabaseNames();

private:
    XclExpNameList      maNameList;         /// List of NAME records.
    XclExpIndexMap      maNameMap;          /// Maps Calc defined names to Excel NAME records.
    XclExpIndexMap      maDBRangeMap;       /// Maps Calc database ranges to Excel NAME records.
    String              maUnnamedDBName;    /// Name of the hidden unnamed database range.
    size_t              mnFirstUserIdx;     /// List index of first user-defined NAME record.
};

// ============================================================================
// *** Implementation ***
// ============================================================================

XclExpName::XclExpName( const XclExpRoot& rRoot, const String& rName ) :
    XclExpRecord( EXC_ID_NAME ),
    XclExpRoot( rRoot ),
    maOrigName( rName ),
    mxName( XclExpStringHelper::CreateString( rRoot, rName, EXC_STR_8BITLENGTH ) ),
    mcBuiltIn( EXC_BUILTIN_UNKNOWN ),
    mnScTab( SCTAB_GLOBAL ),
    mnFlags( EXC_NAME_DEFAULT ),
    mnExtSheet( EXC_NAME_GLOBAL ),
    mnXclTab( EXC_NAME_GLOBAL )
{
}

XclExpName::XclExpName( const XclExpRoot& rRoot, sal_Unicode cBuiltIn ) :
    XclExpRecord( EXC_ID_NAME ),
    XclExpRoot( rRoot ),
    mcBuiltIn( cBuiltIn ),
    mnScTab( SCTAB_GLOBAL ),
    mnFlags( EXC_NAME_DEFAULT ),
    mnExtSheet( EXC_NAME_GLOBAL ),
    mnXclTab( EXC_NAME_GLOBAL )
{
    // filter source range is hidden in Excel
    if( cBuiltIn == EXC_BUILTIN_FILTERDATABASE )
        SetHidden();

    // special case for BIFF5/7 filter source range - name appears as plain text without built-in flag
    if( (GetBiff() <= EXC_BIFF5) && (cBuiltIn == EXC_BUILTIN_FILTERDATABASE) )
    {
        String aName( XclTools::GetXclBuiltInDefName( EXC_BUILTIN_FILTERDATABASE ) );
        mxName = XclExpStringHelper::CreateString( rRoot, aName, EXC_STR_8BITLENGTH );
    }
    else
    {
        mxName = XclExpStringHelper::CreateString( rRoot, cBuiltIn, EXC_STR_8BITLENGTH );
        ::set_flag( mnFlags, EXC_NAME_BUILTIN );
    }
}

void XclExpName::SetTokenArray( XclExpTokenArrayRef xTokArr )
{
    mxTokArr = xTokArr;
}

void XclExpName::SetLocalTab( SCTAB nScTab )
{
    DBG_ASSERT( GetTabInfo().IsExportTab( nScTab ), "XclExpName::SetLocalTab - invalid sheet index" );
    if( GetTabInfo().IsExportTab( nScTab ) )
    {
        mnScTab = nScTab;
        GetGlobalLinkManager().FindExtSheet( mnExtSheet, mnXclTab, nScTab );

        // special handling for NAME record
        switch( GetBiff() )
        {
            case EXC_BIFF5: // EXTERNSHEET index is positive in NAME record
                mnExtSheet = ~mnExtSheet + 1;
            break;
            case EXC_BIFF8: // EXTERNSHEET index not used, but must be created in link table
                mnExtSheet = 0;
            break;
            default:    DBG_ERROR_BIFF();
        }

        // Excel sheet index is 1-based
        ++mnXclTab;
    }
}

void XclExpName::SetHidden( bool bHidden )
{
    ::set_flag( mnFlags, EXC_NAME_HIDDEN, bHidden );
}

void XclExpName::SetMacroCall( bool bVBasic, bool bFunc )
{
    ::set_flag( mnFlags, EXC_NAME_PROC );
    ::set_flag( mnFlags, EXC_NAME_VB, bVBasic );
    ::set_flag( mnFlags, EXC_NAME_FUNC, bFunc );
}

bool XclExpName::IsVolatile() const
{
    return mxTokArr.is() && mxTokArr->IsVolatile();
}

bool XclExpName::IsHidden() const
{
    return ::get_flag( mnFlags, EXC_NAME_HIDDEN );
}

bool XclExpName::IsMacroCall( bool bVBasic, bool bFunc ) const
{
    return
        (::get_flag( mnFlags, EXC_NAME_VB ) == bVBasic) &&
        (::get_flag( mnFlags, EXC_NAME_FUNC ) == bFunc);
}

void XclExpName::Save( XclExpStream& rStrm )
{
    DBG_ASSERT( mxName.is() && (mxName->Len() > 0), "XclExpName::Save - missing name" );
    DBG_ASSERT( !(IsGlobal() && ::get_flag( mnFlags, EXC_NAME_BUILTIN )), "XclExpName::Save - global built-in name" );
    SetRecSize( 11 + mxName->GetSize() + (mxTokArr.is() ? mxTokArr->GetSize() : 2) );
    XclExpRecord::Save( rStrm );
}

void XclExpName::WriteBody( XclExpStream& rStrm )
{
    sal_uInt16 nFmlaSize = mxTokArr.is() ? mxTokArr->GetSize() : 0;

    rStrm   << mnFlags                  // flags
            << sal_uInt8( 0 );          // keyboard shortcut
    mxName->WriteLenField( rStrm );     // length of name
    rStrm   << nFmlaSize                // size of token array
            << mnExtSheet               // BIFF5/7: EXTSHEET index, BIFF8: not used
            << mnXclTab                 // 1-based sheet index for local names
            << sal_uInt32( 0 );         // length of menu/descr/help/status text
    mxName->WriteFlagField( rStrm );    // BIFF8 flag field (no-op in <=BIFF7)
    mxName->WriteBuffer( rStrm );       // character array of the name
    if( mxTokArr.is() )
        mxTokArr->WriteArray( rStrm );  // token array without size
}

// ----------------------------------------------------------------------------

XclExpNameManagerImpl::XclExpNameManagerImpl( const XclExpRoot& rRoot ) :
    XclExpRoot( rRoot ),
    maUnnamedDBName( ScGlobal::GetRscString( STR_DB_NONAME ) ),
    mnFirstUserIdx( 0 )
{
}

void XclExpNameManagerImpl::Initialize()
{
    CreateBuiltInNames();
    mnFirstUserIdx = maNameList.Size();
    CreateUserNames();
    CreateDatabaseNames();
}

sal_uInt16 XclExpNameManagerImpl::InsertName( USHORT nScNameIdx )
{
    sal_uInt16 nNameIdx = FindNameIdx( maNameMap, nScNameIdx );
    if( nNameIdx == 0 )
        if( const ScRangeData* pRangeData = GetNamedRanges().FindIndex( nScNameIdx ) )
            nNameIdx = CreateName( *pRangeData );
    return nNameIdx;
}

sal_uInt16 XclExpNameManagerImpl::InsertDBRange( USHORT nScDBRangeIdx )
{
    sal_uInt16 nNameIdx = FindNameIdx( maDBRangeMap, nScDBRangeIdx );
    if( nNameIdx == 0 )
        if( const ScDBData* pDBData = GetDatabaseRanges().FindIndex( nScDBRangeIdx ) )
            nNameIdx = CreateName( *pDBData );
    return nNameIdx;
}

sal_uInt16 XclExpNameManagerImpl::InsertBuiltInName( sal_Unicode cBuiltIn, XclExpTokenArrayRef xTokArr, SCTAB nScTab )
{
    XclExpNameRef xName( new XclExpName( GetRoot(), cBuiltIn ) );
    xName->SetTokenArray( xTokArr );
    xName->SetLocalTab( nScTab );
    return Append( xName );
}

sal_uInt16 XclExpNameManagerImpl::InsertUniqueName(
        const String& rName, XclExpTokenArrayRef xTokArr, SCTAB nScTab )
{
    DBG_ASSERT( rName.Len(), "XclExpNameManagerImpl::InsertUniqueName - empty name" );
    XclExpNameRef xName( new XclExpName( GetRoot(), GetUnusedName( rName ) ) );
    xName->SetTokenArray( xTokArr );
    xName->SetLocalTab( nScTab );
    return Append( xName );
}

sal_uInt16 XclExpNameManagerImpl::InsertRawName( const String& rName )
{
    // empty name? may occur in broken external Calc tokens
    if( !rName.Len() )
        return 0;

    // try to find an existing NAME record, regardless of its type
    for( size_t nListIdx = mnFirstUserIdx, nListSize = maNameList.Size(); nListIdx < nListSize; ++nListIdx )
    {
        XclExpNameRef xName = maNameList.GetRecord( nListIdx );
        if( xName->IsGlobal() && (xName->GetOrigName() == rName) )
            return static_cast< sal_uInt16 >( nListIdx + 1 );
    }

    // create a new NAME record
    XclExpNameRef xName( new XclExpName( GetRoot(), rName ) );
    return Append( xName );
}

sal_uInt16 XclExpNameManagerImpl::InsertMacroCall( const String& rMacroName, bool bVBasic, bool bFunc, bool bHidden )
{
    // empty name? may occur in broken external Calc tokens
    if( !rMacroName.Len() )
        return 0;

    // try to find an existing NAME record
    for( size_t nListIdx = mnFirstUserIdx, nListSize = maNameList.Size(); nListIdx < nListSize; ++nListIdx )
    {
        XclExpNameRef xName = maNameList.GetRecord( nListIdx );
        if( xName->IsMacroCall( bVBasic, bFunc ) && (xName->GetOrigName() == rMacroName) )
            return static_cast< sal_uInt16 >( nListIdx + 1 );
    }

    // create a new NAME record
    XclExpNameRef xName( new XclExpName( GetRoot(), rMacroName ) );
    xName->SetMacroCall( bVBasic, bFunc );
    xName->SetHidden( bHidden );

    // for sheet macros, add a #NAME! error
    if( !bVBasic )
        xName->SetTokenArray( GetFormulaCompiler().CreateErrorFormula( EXC_ERR_NAME ) );

    return Append( xName );
}

const XclExpName* XclExpNameManagerImpl::GetName( sal_uInt16 nNameIdx ) const
{
    DBG_ASSERT( maNameList.HasRecord( nNameIdx - 1 ), "XclExpNameManagerImpl::GetName - wrong record index" );
    return maNameList.GetRecord( nNameIdx - 1 ).get();
}

void XclExpNameManagerImpl::Save( XclExpStream& rStrm )
{
    maNameList.Save( rStrm );
}

// private --------------------------------------------------------------------

sal_uInt16 XclExpNameManagerImpl::FindNameIdx( const XclExpIndexMap& rMap, USHORT nScIdx ) const
{
    XclExpIndexMap::const_iterator aIt = rMap.find( nScIdx );
    return (aIt == rMap.end()) ? 0 : aIt->second;
}

sal_uInt16 XclExpNameManagerImpl::FindBuiltInNameIdx(
        const String& rName, const XclExpTokenArray& rTokArr, bool bDBRange ) const
{
    /*  Get built-in index from the name. Special case: the database range
        'unnamed' will be mapped to Excel's built-in '_FilterDatabase' name. */
    sal_Unicode cBuiltIn = (bDBRange && (rName == maUnnamedDBName)) ?
        EXC_BUILTIN_FILTERDATABASE : XclTools::GetBuiltInDefNameIndex( rName );

    if( cBuiltIn < EXC_BUILTIN_UNKNOWN )
    {
        // try to find the record in existing built-in NAME record list
        for( size_t nPos = 0; nPos < mnFirstUserIdx; ++nPos )
        {
            XclExpNameRef xName = maNameList.GetRecord( nPos );
            if( xName->GetBuiltInName() == cBuiltIn )
            {
                XclExpTokenArrayRef xTokArr = xName->GetTokenArray();
                if( xTokArr.is() && (*xTokArr == rTokArr) )
                    return static_cast< sal_uInt16 >( nPos + 1 );
            }
        }
    }
    return 0;
}

String XclExpNameManagerImpl::GetUnusedName( const String& rName ) const
{
    String aNewName( rName );
    sal_Int32 nAppIdx = 0;
    bool bExist = true;
    while( bExist )
    {
        // search the list of user-defined names
        bExist = false;
        for( size_t nPos = mnFirstUserIdx, nSize = maNameList.Size(); !bExist && (nPos < nSize); ++nPos )
        {
            XclExpNameRef xName = maNameList.GetRecord( nPos );
            bExist = xName->GetOrigName() == aNewName;
            // name exists -> create a new name "<originalname>_<counter>"
            if( bExist )
                aNewName.Assign( rName ).Append( '_' ).Append( String::CreateFromInt32( ++nAppIdx ) );
        }
    }
    return aNewName;
}

sal_uInt16 XclExpNameManagerImpl::Append( XclExpNameRef xName )
{
    if( maNameList.Size() == 0xFFFF )
        return 0;
    maNameList.AppendRecord( xName );
    return static_cast< sal_uInt16 >( maNameList.Size() );  // 1-based
}

sal_uInt16 XclExpNameManagerImpl::CreateName( const ScRangeData& rRangeData )
{
    const String& rName = rRangeData.GetName();

    /*  #i38821# recursive names: first insert the (empty) name object,
        otherwise a recursive call of this function from the formula compiler
        with the same defined name will not find it and will create it again. */
    size_t nOldListSize = maNameList.Size();
    XclExpNameRef xName( new XclExpName( GetRoot(), rName ) );
    sal_uInt16 nNameIdx = Append( xName );
    // store the index of the NAME record in the lookup map
    maNameMap[ rRangeData.GetIndex() ] = nNameIdx;

    /*  Create the definition formula.
        This may cause recursive creation of other defined names. */
    if( const ScTokenArray* pScTokArr = const_cast< ScRangeData& >( rRangeData ).GetCode() )
    {
        XclExpTokenArrayRef xTokArr = GetFormulaCompiler().CreateFormula( EXC_FMLATYPE_NAME, *pScTokArr );
        xName->SetTokenArray( xTokArr );

        /*  Try to replace by existing built-in name - complete token array is
            needed for comparison, and due to the recursion problem above this
            cannot be done earlier. If a built-in name is found, the created NAME
            record for this name and all following records in the list must be
            deleted, otherwise they may contain wrong name list indexes. */
        sal_uInt16 nBuiltInIdx = FindBuiltInNameIdx( rName, *xTokArr, false );
        if( nBuiltInIdx != 0 )
        {
            // delete the new NAME records
            while( maNameList.Size() > nOldListSize )
                maNameList.RemoveRecord( maNameList.Size() - 1 );
            // use index of the found built-in NAME record
            maNameMap[ rRangeData.GetIndex() ] = nNameIdx = nBuiltInIdx;
        }
    }

    return nNameIdx;
}

sal_uInt16 XclExpNameManagerImpl::CreateName( const ScDBData& rDBData )
{
    // get name and source range, and create the definition formula
    const String& rName = rDBData.GetName();
    ScRange aRange;
    rDBData.GetArea( aRange );
    XclExpTokenArrayRef xTokArr = GetFormulaCompiler().CreateFormula( EXC_FMLATYPE_NAME, aRange );

    // try to use an existing built-in name
    sal_uInt16 nNameIdx = FindBuiltInNameIdx( rName, *xTokArr, true );
    if( nNameIdx == 0 )
    {
        // insert a new name into the list
        XclExpNameRef xName( new XclExpName( GetRoot(), GetUnusedName( rName ) ) );
        xName->SetTokenArray( xTokArr );
        nNameIdx = Append( xName );
    }

    // store the index of the NAME record in the lookup map
    maDBRangeMap[ rDBData.GetIndex() ] = nNameIdx;
    return nNameIdx;
}

void XclExpNameManagerImpl::CreateBuiltInNames()
{
    ScDocument& rDoc = GetDoc();
    XclExpTabInfo& rTabInfo = GetTabInfo();

    /*  #i2394# #100489# built-in defined names must be sorted by the name of the
        containing sheet. Example: SheetA!Print_Range must be stored *before*
        SheetB!Print_Range, regardless of the position of SheetA in the document! */
    for( SCTAB nScTabIdx = 0, nScTabCount = rTabInfo.GetScTabCount(); nScTabIdx < nScTabCount; ++nScTabIdx )
    {
        // find real sheet index from the nScTabIdx counter
        SCTAB nScTab = rTabInfo.GetRealScTab( nScTabIdx );
        // create NAME records for all built-in names of this sheet
        if( rTabInfo.IsExportTab( nScTab ) )
        {
            // *** 1) print ranges *** ----------------------------------------

            if( rDoc.HasPrintRange() )
            {
                ScRangeList aRangeList;
                for( USHORT nIdx = 0, nCount = rDoc.GetPrintRangeCount( nScTab ); nIdx < nCount; ++nIdx )
                {
                    ScRange aRange( *rDoc.GetPrintRange( nScTab, nIdx ) );
                    // Calc document does not care about sheet index in print ranges
                    aRange.aStart.SetTab( nScTab );
                    aRange.aEnd.SetTab( nScTab );
                    aRangeList.Append( aRange );
                }
                GetAddressConverter().ValidateRangeList( aRangeList, true );
                GetNameManager().InsertBuiltInName( EXC_BUILTIN_PRINTAREA, aRangeList );
            }

            // *** 2) print titles *** ----------------------------------------

            ScRangeList aTitleList;
            // repeated columns
            if( const ScRange* pColRange = rDoc.GetRepeatColRange( nScTab ) )
                aTitleList.Append( ScRange(
                    pColRange->aStart.Col(), 0, nScTab,
                    pColRange->aEnd.Col(), GetXclMaxPos().Row(), nScTab ) );
            // repeated rows
            if( const ScRange* pRowRange = rDoc.GetRepeatRowRange( nScTab ) )
                aTitleList.Append( ScRange(
                    0, pRowRange->aStart.Row(), nScTab,
                    GetXclMaxPos().Col(), pRowRange->aEnd.Row(), nScTab ) );
            // create the NAME record
            GetAddressConverter().ValidateRangeList( aTitleList, true );
            GetNameManager().InsertBuiltInName( EXC_BUILTIN_PRINTTITLES, aTitleList );

            // *** 3) filter ranges *** ---------------------------------------

            if( GetBiff() == EXC_BIFF8 )
                GetFilterManager().InitTabFilter( nScTab );
        }
    }
}

void XclExpNameManagerImpl::CreateUserNames()
{
    const ScRangeName& rNamedRanges = GetNamedRanges();
    for( USHORT nNameIdx = 0, nNameCount = rNamedRanges.GetCount(); nNameIdx < nNameCount; ++nNameIdx )
    {
        const ScRangeData* pRangeData = rNamedRanges[ nNameIdx ];
        DBG_ASSERT( rNamedRanges[ nNameIdx ], "XclExpNameManagerImpl::CreateUserNames - missing defined name" );
        // skip definitions of shared formulas
        if( pRangeData && !pRangeData->HasType( RT_SHARED ) && !FindNameIdx( maNameMap, pRangeData->GetIndex() ) )
            CreateName( *pRangeData );
    }
}

void XclExpNameManagerImpl::CreateDatabaseNames()
{
    const ScDBCollection& rDBRanges = GetDatabaseRanges();
    for( USHORT nDBIdx = 0, nDBCount = rDBRanges.GetCount(); nDBIdx < nDBCount; ++nDBIdx )
    {
        const ScDBData* pDBData = rDBRanges[ nDBIdx ];
        DBG_ASSERT( pDBData, "XclExpNameManagerImpl::CreateDatabaseNames - missing database range" );
        // skip hidden "unnamed" range
        if( pDBData && (pDBData->GetName() != maUnnamedDBName) && !FindNameIdx( maDBRangeMap, pDBData->GetIndex() ) )
            CreateName( *pDBData );
    }
}

// ----------------------------------------------------------------------------

XclExpNameManager::XclExpNameManager( const XclExpRoot& rRoot ) :
    XclExpRoot( rRoot ),
    mxImpl( new XclExpNameManagerImpl( rRoot ) )
{
}

XclExpNameManager::~XclExpNameManager()
{
}

void XclExpNameManager::Initialize()
{
    mxImpl->Initialize();
}

sal_uInt16 XclExpNameManager::InsertName( USHORT nScNameIdx )
{
    return mxImpl->InsertName( nScNameIdx );
}

sal_uInt16 XclExpNameManager::InsertDBRange( USHORT nScDBRangeIdx )
{
    return mxImpl->InsertDBRange( nScDBRangeIdx );
}

sal_uInt16 XclExpNameManager::InsertBuiltInName( sal_Unicode cBuiltIn, XclExpTokenArrayRef xTokArr, SCTAB nScTab )
{
    return mxImpl->InsertBuiltInName( cBuiltIn, xTokArr, nScTab );
}

sal_uInt16 XclExpNameManager::InsertBuiltInName( sal_Unicode cBuiltIn, const ScRange& rRange )
{
    XclExpTokenArrayRef xTokArr = GetFormulaCompiler().CreateFormula( EXC_FMLATYPE_NAME, rRange );
    return mxImpl->InsertBuiltInName( cBuiltIn, xTokArr, rRange.aStart.Tab() );
}

sal_uInt16 XclExpNameManager::InsertBuiltInName( sal_Unicode cBuiltIn, const ScRangeList& rRangeList )
{
    sal_uInt16 nNameIdx = 0;
    if( rRangeList.Count() )
    {
        XclExpTokenArrayRef xTokArr = GetFormulaCompiler().CreateFormula( EXC_FMLATYPE_NAME, rRangeList );
        nNameIdx = mxImpl->InsertBuiltInName( cBuiltIn, xTokArr, rRangeList.GetObject( 0 )->aStart.Tab() );
    }
    return nNameIdx;
}

sal_uInt16 XclExpNameManager::InsertUniqueName(
        const String& rName, XclExpTokenArrayRef xTokArr, SCTAB nScTab )
{
    return mxImpl->InsertUniqueName( rName, xTokArr, nScTab );
}

sal_uInt16 XclExpNameManager::InsertRawName( const String& rName )
{
    return mxImpl->InsertRawName( rName );
}

sal_uInt16 XclExpNameManager::InsertMacroCall( const String& rMacroName, bool bVBasic, bool bFunc, bool bHidden )
{
    return mxImpl->InsertMacroCall( rMacroName, bVBasic, bFunc, bHidden );
}

const String& XclExpNameManager::GetOrigName( sal_uInt16 nNameIdx ) const
{
    const XclExpName* pName = mxImpl->GetName( nNameIdx );
    return pName ? pName->GetOrigName() : EMPTY_STRING;
}

SCTAB XclExpNameManager::GetScTab( sal_uInt16 nNameIdx ) const
{
    const XclExpName* pName = mxImpl->GetName( nNameIdx );
    return pName ? pName->GetScTab() : SCTAB_GLOBAL;
}

bool XclExpNameManager::IsVolatile( sal_uInt16 nNameIdx ) const
{
    const XclExpName* pName = mxImpl->GetName( nNameIdx );
    return pName && pName->IsVolatile();
}

void XclExpNameManager::Save( XclExpStream& rStrm )
{
    mxImpl->Save( rStrm );
}

// ============================================================================

