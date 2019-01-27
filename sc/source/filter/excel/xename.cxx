/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include <xename.hxx>

#include <map>

#include <document.hxx>
#include <rangenam.hxx>
#include <tokenarray.hxx>
#include <xehelper.hxx>
#include <xelink.hxx>
#include <excrecds.hxx>
#include <xlname.hxx>
#include <xeformula.hxx>
#include <xestring.hxx>
#include <xltools.hxx>

#include <formula/grammar.hxx>
#include <oox/export/utils.hxx>
#include <oox/token/tokens.hxx>

using namespace ::oox;

// *** Helper classes ***

/** Represents an internal defined name, supports writing it to a NAME record. */
class XclExpName : public XclExpRecord, protected XclExpRoot
{
public:
    /** Creates a standard defined name. */
    explicit            XclExpName( const XclExpRoot& rRoot, const OUString& rName );
    /** Creates a built-in defined name. */
    explicit            XclExpName( const XclExpRoot& rRoot, sal_Unicode cBuiltIn );

    /** Sets a token array containing the definition of this name. */
    void                SetTokenArray( const XclTokenArrayRef& xTokArr );
    /** Changes this defined name to be local on the specified Calc sheet. */
    void                SetLocalTab( SCTAB nScTab );
    /** Hides or unhides the defined name. */
    void                SetHidden( bool bHidden = true );
    /** Changes this name to be the call to a VB macro function or procedure.
        @param bVBasic  true = Visual Basic macro, false = Sheet macro.
        @param bFunc  true = Macro function; false = Macro procedure. */
    void                SetMacroCall( bool bVBasic, bool bFunc );

    /** Sets the name's symbol value
        @param sValue   the name's symbolic value */
    void                SetSymbol( const OUString& rValue );

    /** Returns the original name (title) of this defined name. */
    const OUString& GetOrigName() const { return maOrigName; }
    /** Returns the Excel built-in name index of this defined name.
        @return  The built-in name index or EXC_BUILTIN_UNKNOWN for user-defined names. */
    sal_Unicode  GetBuiltInName() const { return mcBuiltIn; }

    /** Returns the token array for this defined name. */
    const XclTokenArrayRef& GetTokenArray() const { return mxTokArr; }

    /** Returns true, if this is a document-global defined name. */
    bool         IsGlobal() const { return mnXclTab == EXC_NAME_GLOBAL; }
    /** Returns the Calc sheet of a local defined name. */
    SCTAB        GetScTab() const { return mnScTab; }

    /** Returns true, if this defined name is volatile. */
    bool                IsVolatile() const;
    /** Returns true, if this defined name describes a macro call.
        @param bFunc  true = Macro function; false = Macro procedure. */
    bool                IsMacroCall( bool bVBasic, bool bFunc ) const;

    /** Writes the entire NAME record to the passed stream. */
    virtual void        Save( XclExpStream& rStrm ) override;

    virtual void        SaveXml( XclExpXmlStream& rStrm ) override;

private:
    /** Writes the body of the NAME record to the passed stream. */
    virtual void        WriteBody( XclExpStream& rStrm ) override;

private:
    OUString            maOrigName;     /// The original user-defined name.
    OUString            msSymbol;       /// The value of the symbol
    XclExpStringRef     mxName;         /// The name as Excel string object.
    XclTokenArrayRef    mxTokArr;       /// The definition of the defined name.
    sal_Unicode         mcBuiltIn;      /// The built-in index for built-in names.
    SCTAB               mnScTab;        /// The Calc sheet index for local names.
    sal_uInt16          mnFlags;        /// Additional flags for this defined name.
    sal_uInt16          mnExtSheet;     /// The 1-based index to a global EXTERNSHEET record.
    sal_uInt16          mnXclTab;       /// The 1-based Excel sheet index for local names.
};

/** Implementation class of the name manager. */
class XclExpNameManagerImpl : protected XclExpRoot
{
public:
    explicit            XclExpNameManagerImpl( const XclExpRoot& rRoot );

    /** Creates NAME records for built-in and user defined names. */
    void                Initialize();

    /** Inserts the Calc name with the passed index and returns the Excel NAME index. */
    sal_uInt16          InsertName( SCTAB nTab, sal_uInt16 nScNameIdx, SCTAB nCurrTab );

    /** Inserts a new built-in defined name. */
    sal_uInt16          InsertBuiltInName( sal_Unicode cBuiltIn, const XclTokenArrayRef& xTokArr, SCTAB nScTab, const ScRangeList& aRangeList );
    sal_uInt16          InsertBuiltInName( sal_Unicode cBuiltIn, const XclTokenArrayRef& xTokArr, const ScRange& aRange );
    /** Inserts a new defined name. Sets another unused name, if rName already exists. */
    sal_uInt16          InsertUniqueName( const OUString& rName, const XclTokenArrayRef& xTokArr, SCTAB nScTab );
    /** Returns index of an existing name, or creates a name without definition. */
    sal_uInt16          InsertRawName( const OUString& rName );
    /** Searches or inserts a defined name describing a macro name.
        @param bVBasic  true = Visual Basic macro; false = Sheet macro.
        @param bFunc  true = Macro function; false = Macro procedure. */
    sal_uInt16          InsertMacroCall( const OUString& rMacroName, bool bVBasic, bool bFunc, bool bHidden );

    /** Returns the NAME record at the specified position or 0 on error. */
    const XclExpName*   GetName( sal_uInt16 nNameIdx ) const;

    /** Writes the entire list of NAME records.
        @descr  In BIFF7 and lower, writes the entire global link table, which
            consists of an EXTERNCOUNT record, several EXTERNSHEET records, and
            the list of NAME records. */
    void                Save( XclExpStream& rStrm );

    void                SaveXml( XclExpXmlStream& rStrm );

private:
    typedef XclExpRecordList< XclExpName >      XclExpNameList;
    typedef XclExpNameList::RecordRefType       XclExpNameRef;

    typedef ::std::map< ::std::pair<SCTAB, OUString>, sal_uInt16> NamedExpMap;

private:
    /**
     * @param nTab 0-based table index, or SCTAB_GLOBAL for global names.
     * @param nScIdx calc's name index.
     *
     * @return excel's name index.
     */
    sal_uInt16          FindNamedExp( SCTAB nTab, OUString sName );

    /** Returns the index of an existing built-in NAME record with the passed definition, otherwise 0. */
    sal_uInt16          FindBuiltInNameIdx( const OUString& rName,
                            const XclTokenArray& rTokArr ) const;
    /** Returns an unused name for the passed name. */
    OUString            GetUnusedName( const OUString& rName ) const;

    /** Appends a new NAME record to the record list.
        @return  The 1-based NAME record index used elsewhere in the Excel file. */
    sal_uInt16          Append( const XclExpNameRef& xName );
    /** Creates a new NAME record for the passed user-defined name.
        @return  The 1-based NAME record index used elsewhere in the Excel file. */
    sal_uInt16          CreateName( SCTAB nTab, const ScRangeData& rRangeData );

    /** Creates NAME records for all built-in names in the document. */
    void                CreateBuiltInNames();
    /** Creates NAME records for all user-defined names in the document. */
    void                CreateUserNames();

private:
    /**
     * Maps Calc's named range to Excel's NAME records.  Global names use
     * -1 as their table index, whereas sheet-local names have 0-based table
     *  index.
     */
    NamedExpMap         maNamedExpMap;
    XclExpNameList      maNameList;         /// List of NAME records.
    size_t              mnFirstUserIdx;     /// List index of first user-defined NAME record.
};

// *** Implementation ***

XclExpName::XclExpName( const XclExpRoot& rRoot, const OUString& rName ) :
    XclExpRecord( EXC_ID_NAME ),
    XclExpRoot( rRoot ),
    maOrigName( rName ),
    mxName( XclExpStringHelper::CreateString( rRoot, rName, XclStrFlags::EightBitLength ) ),
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
        OUString aName( XclTools::GetXclBuiltInDefName( EXC_BUILTIN_FILTERDATABASE ) );
        mxName = XclExpStringHelper::CreateString( rRoot, aName, XclStrFlags::EightBitLength );
        maOrigName = XclTools::GetXclBuiltInDefName( cBuiltIn );
    }
    else
    {
        maOrigName =  XclTools::GetBuiltInDefNameXml( cBuiltIn ) ;
        mxName = XclExpStringHelper::CreateString( rRoot, cBuiltIn, XclStrFlags::EightBitLength );
        ::set_flag( mnFlags, EXC_NAME_BUILTIN );
    }
}

void XclExpName::SetTokenArray( const XclTokenArrayRef& xTokArr )
{
    mxTokArr = xTokArr;
}

void XclExpName::SetLocalTab( SCTAB nScTab )
{
    OSL_ENSURE( GetTabInfo().IsExportTab( nScTab ), "XclExpName::SetLocalTab - invalid sheet index" );
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

void XclExpName::SetSymbol( const OUString& rSymbol )
{
    msSymbol = rSymbol;
}

bool XclExpName::IsVolatile() const
{
    return mxTokArr && mxTokArr->IsVolatile();
}

bool XclExpName::IsMacroCall( bool bVBasic, bool bFunc ) const
{
    return
        (::get_flag( mnFlags, EXC_NAME_VB ) == bVBasic) &&
        (::get_flag( mnFlags, EXC_NAME_FUNC ) == bFunc);
}

void XclExpName::Save( XclExpStream& rStrm )
{
    OSL_ENSURE( mxName && (mxName->Len() > 0), "XclExpName::Save - missing name" );
    OSL_ENSURE( !(IsGlobal() && ::get_flag( mnFlags, EXC_NAME_BUILTIN )), "XclExpName::Save - global built-in name" );
    SetRecSize( 11 + mxName->GetSize() + (mxTokArr ? mxTokArr->GetSize() : 2) );
    XclExpRecord::Save( rStrm );
}

void XclExpName::SaveXml( XclExpXmlStream& rStrm )
{
    sax_fastparser::FSHelperPtr& rWorkbook = rStrm.GetCurrentStream();
    rWorkbook->startElement( XML_definedName,
            // OOXTODO: XML_comment, "",
            // OOXTODO: XML_customMenu, "",
            // OOXTODO: XML_description, "",
            XML_function, ToPsz( ::get_flag( mnFlags, EXC_NAME_VB ) ),
            // OOXTODO: XML_functionGroupId, "",
            // OOXTODO: XML_help, "",
            XML_hidden, ToPsz( ::get_flag( mnFlags, EXC_NAME_HIDDEN ) ),
            XML_localSheetId, mnScTab == SCTAB_GLOBAL ? nullptr : OString::number( mnScTab ).getStr(),
            XML_name, XclXmlUtils::ToOString( maOrigName ).getStr(),
            // OOXTODO: XML_publishToServer, "",
            // OOXTODO: XML_shortcutKey, "",
            // OOXTODO: XML_statusBar, "",
            XML_vbProcedure, ToPsz( ::get_flag( mnFlags, EXC_NAME_VB ) ),
            // OOXTODO: XML_workbookParameter, "",
            // OOXTODO: XML_xlm, "",
            FSEND );
    rWorkbook->writeEscaped( msSymbol );
    rWorkbook->endElement( XML_definedName );
}

void XclExpName::WriteBody( XclExpStream& rStrm )
{
    sal_uInt16 nFmlaSize = mxTokArr ? mxTokArr->GetSize() : 0;

    rStrm   << mnFlags                  // flags
            << sal_uInt8( 0 );          // keyboard shortcut
    mxName->WriteLenField( rStrm );     // length of name
    rStrm   << nFmlaSize                // size of token array
            << mnExtSheet               // BIFF5/7: EXTSHEET index, BIFF8: not used
            << mnXclTab                 // 1-based sheet index for local names
            << sal_uInt32( 0 );         // length of menu/descr/help/status text
    mxName->WriteFlagField( rStrm );    // BIFF8 flag field (no-op in <=BIFF7)
    mxName->WriteBuffer( rStrm );       // character array of the name
    if( mxTokArr )
        mxTokArr->WriteArray( rStrm );  // token array without size
}

/** Returns true (needed fixing) if FormulaToken was not absolute and 3D.
    So, regardless of whether the fix was successful or not, true is still returned since a fix was required.*/
static bool lcl_EnsureAbs3DToken( const SCTAB nTab, formula::FormulaToken* pTok, const bool bFix = true )
{
    bool bFixRequired = false;
    if ( !pTok || ( pTok->GetType() != formula::svSingleRef && pTok->GetType() != formula::svDoubleRef ) )
        return bFixRequired;

    ScSingleRefData* pRef1 = pTok->GetSingleRef();
    if ( !pRef1 )
        return bFixRequired;

    ScSingleRefData* pRef2 = nullptr;
    if ( pTok->GetType() == formula::svDoubleRef )
        pRef2 = pTok->GetSingleRef2();

    if ( pRef1->IsTabRel() || !pRef1->IsFlag3D() )
    {
        bFixRequired = true;
        if ( bFix )
        {
            if ( pRef1->IsTabRel() && nTab != SCTAB_GLOBAL )
                pRef1->SetAbsTab( nTab + pRef1->Tab() );  //XLS requirement
            if ( !pRef1->IsTabRel() )
            {
                pRef1->SetFlag3D( true );  //XLSX requirement
                if ( pRef2 && !pRef2->IsTabRel() )
                    pRef2->SetFlag3D( pRef2->Tab() != pRef1->Tab() );
            }
        }
    }

    if ( pRef2 && pRef2->IsTabRel() && !pRef1->IsTabRel() )
    {
        bFixRequired = true;
        if ( bFix && nTab != SCTAB_GLOBAL )
        {
            pRef2->SetAbsTab( nTab + pRef2->Tab() );
            pRef2->SetFlag3D( pRef2->Tab() != pRef1->Tab() );
        }
    }
    return bFixRequired;
}

XclExpNameManagerImpl::XclExpNameManagerImpl( const XclExpRoot& rRoot ) :
    XclExpRoot( rRoot ),
    mnFirstUserIdx( 0 )
{
}

void XclExpNameManagerImpl::Initialize()
{
    CreateBuiltInNames();
    mnFirstUserIdx = maNameList.GetSize();
    CreateUserNames();
}

sal_uInt16 XclExpNameManagerImpl::InsertName( SCTAB nTab, sal_uInt16 nScNameIdx, SCTAB nCurrTab )
{
    sal_uInt16 nNameIdx = 0;
    const ScRangeData* pData = nullptr;
    ScRangeName* pRN = (nTab == SCTAB_GLOBAL) ? GetDoc().GetRangeName() : GetDoc().GetRangeName(nTab);
    if (pRN)
        pData = pRN->findByIndex(nScNameIdx);

    if (pData)
    {
        bool bEmulateGlobalRelativeTable = false;
        const ScTokenArray* pCode = pData->GetCode();
        if ( pCode
            && nTab == SCTAB_GLOBAL
            && (pData->HasType( ScRangeData::Type::AbsPos ) || pData->HasType( ScRangeData::Type::AbsArea )) )
        {
            bEmulateGlobalRelativeTable = lcl_EnsureAbs3DToken( nTab, pCode->FirstToken(), /*bFix=*/false );
        }
        nNameIdx = FindNamedExp( bEmulateGlobalRelativeTable ? nCurrTab : nTab, pData->GetName() );
        if (!nNameIdx)
            nNameIdx = CreateName(nTab, *pData);
    }

    return nNameIdx;
}

sal_uInt16 XclExpNameManagerImpl::InsertBuiltInName( sal_Unicode cBuiltIn, const XclTokenArrayRef& xTokArr, const ScRange& aRange )
{
    XclExpNameRef xName( new XclExpName( GetRoot(), cBuiltIn ) );
    xName->SetTokenArray( xTokArr );
    xName->SetLocalTab( aRange.aStart.Tab() );
    OUString sSymbol(aRange.Format(ScRefFlags::RANGE_ABS_3D, &GetDocRef(), ScAddress::Details( ::formula::FormulaGrammar::CONV_XL_A1)));
    xName->SetSymbol( sSymbol );
    return Append( xName );
}

sal_uInt16 XclExpNameManagerImpl::InsertBuiltInName( sal_Unicode cBuiltIn, const XclTokenArrayRef& xTokArr, SCTAB nScTab, const ScRangeList& rRangeList )
{
    XclExpNameRef xName( new XclExpName( GetRoot(), cBuiltIn ) );
    xName->SetTokenArray( xTokArr );
    xName->SetLocalTab( nScTab );
    OUString sSymbol;
    rRangeList.Format( sSymbol, ScRefFlags::RANGE_ABS_3D, &GetDocRef(), ::formula::FormulaGrammar::CONV_XL_A1 );
    xName->SetSymbol( sSymbol );
    return Append( xName );
}

sal_uInt16 XclExpNameManagerImpl::InsertUniqueName(
        const OUString& rName, const XclTokenArrayRef& xTokArr, SCTAB nScTab )
{
    OSL_ENSURE( !rName.isEmpty(), "XclExpNameManagerImpl::InsertUniqueName - empty name" );
    XclExpNameRef xName( new XclExpName( GetRoot(), GetUnusedName( rName ) ) );
    xName->SetTokenArray( xTokArr );
    xName->SetLocalTab( nScTab );
    return Append( xName );
}

sal_uInt16 XclExpNameManagerImpl::InsertRawName( const OUString& rName )
{
    // empty name? may occur in broken external Calc tokens
    if( rName.isEmpty() )
        return 0;

    // try to find an existing NAME record, regardless of its type
    for( size_t nListIdx = mnFirstUserIdx, nListSize = maNameList.GetSize(); nListIdx < nListSize; ++nListIdx )
    {
        XclExpNameRef xName = maNameList.GetRecord( nListIdx );
        if( xName->IsGlobal() && (xName->GetOrigName() == rName) )
            return static_cast< sal_uInt16 >( nListIdx + 1 );
    }

    // create a new NAME record
    XclExpNameRef xName( new XclExpName( GetRoot(), rName ) );
    return Append( xName );
}

sal_uInt16 XclExpNameManagerImpl::InsertMacroCall( const OUString& rMacroName, bool bVBasic, bool bFunc, bool bHidden )
{
    // empty name? may occur in broken external Calc tokens
    if( rMacroName.isEmpty() )
        return 0;

    // try to find an existing NAME record
    for( size_t nListIdx = mnFirstUserIdx, nListSize = maNameList.GetSize(); nListIdx < nListSize; ++nListIdx )
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
    OSL_ENSURE( maNameList.HasRecord( nNameIdx - 1 ), "XclExpNameManagerImpl::GetName - wrong record index" );
    return maNameList.GetRecord( nNameIdx - 1 ).get();
}

void XclExpNameManagerImpl::Save( XclExpStream& rStrm )
{
    maNameList.Save( rStrm );
}

void XclExpNameManagerImpl::SaveXml( XclExpXmlStream& rStrm )
{
    if( maNameList.IsEmpty() )
        return;
    sax_fastparser::FSHelperPtr& rWorkbook = rStrm.GetCurrentStream();
    rWorkbook->startElement( XML_definedNames, FSEND );
    maNameList.SaveXml( rStrm );
    rWorkbook->endElement( XML_definedNames );
}

// private --------------------------------------------------------------------

sal_uInt16 XclExpNameManagerImpl::FindNamedExp( SCTAB nTab, OUString sName )
{
    NamedExpMap::key_type key = NamedExpMap::key_type(nTab, sName);
    NamedExpMap::const_iterator itr = maNamedExpMap.find(key);
    return (itr == maNamedExpMap.end()) ? 0 : itr->second;
}

sal_uInt16 XclExpNameManagerImpl::FindBuiltInNameIdx(
        const OUString& rName, const XclTokenArray& rTokArr ) const
{
    /*  Get built-in index from the name. Special case: the database range
        'unnamed' will be mapped to Excel's built-in '_FilterDatabase' name. */
    sal_Unicode cBuiltIn = XclTools::GetBuiltInDefNameIndex( rName );

    if( cBuiltIn < EXC_BUILTIN_UNKNOWN )
    {
        // try to find the record in existing built-in NAME record list
        for( size_t nPos = 0; nPos < mnFirstUserIdx; ++nPos )
        {
            XclExpNameRef xName = maNameList.GetRecord( nPos );
            if( xName->GetBuiltInName() == cBuiltIn )
            {
                XclTokenArrayRef xTokArr = xName->GetTokenArray();
                if( xTokArr && (*xTokArr == rTokArr) )
                    return static_cast< sal_uInt16 >( nPos + 1 );
            }
        }
    }
    return 0;
}

OUString XclExpNameManagerImpl::GetUnusedName( const OUString& rName ) const
{
    OUString aNewName( rName );
    sal_Int32 nAppIdx = 0;
    bool bExist = true;
    while( bExist )
    {
        // search the list of user-defined names
        bExist = false;
        for( size_t nPos = mnFirstUserIdx, nSize = maNameList.GetSize(); !bExist && (nPos < nSize); ++nPos )
        {
            XclExpNameRef xName = maNameList.GetRecord( nPos );
            bExist = xName->GetOrigName() == aNewName;
            // name exists -> create a new name "<originalname>_<counter>"
            if( bExist )
                aNewName = rName + "_" + OUString::number( ++nAppIdx );
        }
    }
    return aNewName;
}

sal_uInt16 XclExpNameManagerImpl::Append( const XclExpNameRef& xName )
{
    if( maNameList.GetSize() == 0xFFFF )
        return 0;
    maNameList.AppendRecord( xName );
    return static_cast< sal_uInt16 >( maNameList.GetSize() );  // 1-based
}

sal_uInt16 XclExpNameManagerImpl::CreateName( SCTAB nTab, const ScRangeData& rRangeData )
{
    const OUString& rName = rRangeData.GetName();

    /*  #i38821# recursive names: first insert the (empty) name object,
        otherwise a recursive call of this function from the formula compiler
        with the same defined name will not find it and will create it again. */
    size_t nOldListSize = maNameList.GetSize();
    XclExpNameRef xName( new XclExpName( GetRoot(), rName ) );
    if (nTab != SCTAB_GLOBAL)
        xName->SetLocalTab(nTab);
    sal_uInt16 nNameIdx = Append( xName );
    // store the index of the NAME record in the lookup map
    NamedExpMap::key_type key = NamedExpMap::key_type(nTab, rRangeData.GetName());
    maNamedExpMap[key] = nNameIdx;

    /*  Create the definition formula.
        This may cause recursive creation of other defined names. */
    if( const ScTokenArray* pScTokArr = const_cast< ScRangeData& >( rRangeData ).GetCode() )
    {
        XclTokenArrayRef xTokArr;
        OUString sSymbol;
        // MSO requires named ranges to have absolute sheet references
        if ( rRangeData.HasType( ScRangeData::Type::AbsPos ) || rRangeData.HasType( ScRangeData::Type::AbsArea ) )
        {
            // Don't modify the actual document; use a temporary copy to create the export formulas.
            std::unique_ptr<ScTokenArray> pTokenCopy( pScTokArr->Clone() );
            lcl_EnsureAbs3DToken(nTab, pTokenCopy->FirstToken());

            xTokArr = GetFormulaCompiler().CreateFormula(EXC_FMLATYPE_NAME, *pTokenCopy);
            if ( GetOutput() != EXC_OUTPUT_BINARY )
            {
                ScCompiler aComp(&GetDocRef(), rRangeData.GetPos(), *pTokenCopy,
                                 formula::FormulaGrammar::GRAM_OOXML);
                aComp.CreateStringFromTokenArray( sSymbol );
            }
        }
        else
        {
            xTokArr = GetFormulaCompiler().CreateFormula( EXC_FMLATYPE_NAME, *pScTokArr );
            rRangeData.GetSymbol( sSymbol, ((GetOutput() == EXC_OUTPUT_BINARY) ?
                     formula::FormulaGrammar::GRAM_ENGLISH_XL_A1 : formula::FormulaGrammar::GRAM_OOXML));
        }
        xName->SetTokenArray( xTokArr );
        xName->SetSymbol( sSymbol );

        /*  Try to replace by existing built-in name - complete token array is
            needed for comparison, and due to the recursion problem above this
            cannot be done earlier. If a built-in name is found, the created NAME
            record for this name and all following records in the list must be
            deleted, otherwise they may contain wrong name list indexes. */
        sal_uInt16 nBuiltInIdx = FindBuiltInNameIdx( rName, *xTokArr );
        if( nBuiltInIdx != 0 )
        {
            // delete the new NAME records
            while( maNameList.GetSize() > nOldListSize )
                maNameList.RemoveRecord( maNameList.GetSize() - 1 );
            // use index of the found built-in NAME record
            key = NamedExpMap::key_type(nTab, rRangeData.GetName());
            maNamedExpMap[key] = nNameIdx = nBuiltInIdx;
        }
    }

    return nNameIdx;
}

void XclExpNameManagerImpl::CreateBuiltInNames()
{
    ScDocument& rDoc = GetDoc();
    XclExpTabInfo& rTabInfo = GetTabInfo();

    /*  #i2394# built-in defined names must be sorted by the name of the
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
                for( sal_uInt16 nIdx = 0, nCount = rDoc.GetPrintRangeCount( nScTab ); nIdx < nCount; ++nIdx )
                {
                    const ScRange* pPrintRange = rDoc.GetPrintRange( nScTab, nIdx );
                    if (!pPrintRange)
                        continue;
                    ScRange aRange( *pPrintRange );
                    // Calc document does not care about sheet index in print ranges
                    aRange.aStart.SetTab( nScTab );
                    aRange.aEnd.SetTab( nScTab );
                    aRange.PutInOrder();
                    aRangeList.push_back( aRange );
                }
                // create the NAME record (do not warn if ranges are shrunken)
                GetAddressConverter().ValidateRangeList( aRangeList, false );
                if( !aRangeList.empty() )
                    GetNameManager().InsertBuiltInName( EXC_BUILTIN_PRINTAREA, aRangeList );
            }

            // *** 2) print titles *** ----------------------------------------

            ScRangeList aTitleList;
            // repeated columns
            if( const ScRange* pColRange = rDoc.GetRepeatColRange( nScTab ) )
                aTitleList.push_back( ScRange(
                    pColRange->aStart.Col(), 0, nScTab,
                    pColRange->aEnd.Col(), GetXclMaxPos().Row(), nScTab ) );
            // repeated rows
            if( const ScRange* pRowRange = rDoc.GetRepeatRowRange( nScTab ) )
                aTitleList.push_back( ScRange(
                    0, pRowRange->aStart.Row(), nScTab,
                    GetXclMaxPos().Col(), pRowRange->aEnd.Row(), nScTab ) );
            // create the NAME record
            GetAddressConverter().ValidateRangeList( aTitleList, false );
            if( !aTitleList.empty() )
                GetNameManager().InsertBuiltInName( EXC_BUILTIN_PRINTTITLES, aTitleList );

            // *** 3) filter ranges *** ---------------------------------------

            if( GetBiff() == EXC_BIFF8 )
                GetFilterManager().InitTabFilter( nScTab );
        }
    }
}

void XclExpNameManagerImpl::CreateUserNames()
{
    std::vector<ScRangeData*> vEmulateAsLocalRange;
    const ScRangeName& rNamedRanges = GetNamedRanges();
    for (const auto& rEntry : rNamedRanges)
    {
        // skip definitions of shared formulas
        if (!FindNamedExp(SCTAB_GLOBAL, rEntry.second->GetName()))
        {
            const ScTokenArray* pCode = rEntry.second->GetCode();
            if ( pCode
                && (rEntry.second->HasType( ScRangeData::Type::AbsPos ) || rEntry.second->HasType( ScRangeData::Type::AbsArea ))
                && lcl_EnsureAbs3DToken( SCTAB_GLOBAL, pCode->FirstToken(), /*bFix=*/false ) )
            {
                vEmulateAsLocalRange.emplace_back(rEntry.second.get());
            }
            else
                CreateName(SCTAB_GLOBAL, *rEntry.second);
        }
    }
    //look at sheets containing local range names
    ScRangeName::TabNameCopyMap rLocalNames;
    GetDoc().GetAllTabRangeNames(rLocalNames);
    for (const auto& [rTab, pRangeName] : rLocalNames)
    {
        for (const auto& rEntry : *pRangeName)
        {
            // skip definitions of shared formulas
            if (!FindNamedExp(rTab, rEntry.second->GetName()))
                CreateName(rTab, *rEntry.second);
        }
    }

    // Emulate relative global variables by creating a copy in each local range.
    // Creating AFTER true local range names so that conflicting global names will be ignored.
    for ( SCTAB nTab = 0; nTab < GetDoc().GetTableCount(); ++nTab )
    {
        for ( auto rangeDataItr : vEmulateAsLocalRange )
        {
            if ( !FindNamedExp(nTab, rangeDataItr->GetName()) )
                CreateName(nTab, *rangeDataItr );
        }
    }
}

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

sal_uInt16 XclExpNameManager::InsertName( SCTAB nTab, sal_uInt16 nScNameIdx, SCTAB nCurrTab )
{
    return mxImpl->InsertName( nTab, nScNameIdx, nCurrTab );
}

sal_uInt16 XclExpNameManager::InsertBuiltInName( sal_Unicode cBuiltIn, const ScRange& rRange )
{
    XclTokenArrayRef xTokArr = GetFormulaCompiler().CreateFormula( EXC_FMLATYPE_NAME, rRange );
    return mxImpl->InsertBuiltInName( cBuiltIn, xTokArr, rRange );
}

sal_uInt16 XclExpNameManager::InsertBuiltInName( sal_Unicode cBuiltIn, const ScRangeList& rRangeList )
{
    sal_uInt16 nNameIdx = 0;
    if( !rRangeList.empty() )
    {
        XclTokenArrayRef xTokArr = GetFormulaCompiler().CreateFormula( EXC_FMLATYPE_NAME, rRangeList );
        nNameIdx = mxImpl->InsertBuiltInName( cBuiltIn, xTokArr, rRangeList.front().aStart.Tab(), rRangeList );
    }
    return nNameIdx;
}

sal_uInt16 XclExpNameManager::InsertUniqueName(
        const OUString& rName, const XclTokenArrayRef& xTokArr, SCTAB nScTab )
{
    return mxImpl->InsertUniqueName( rName, xTokArr, nScTab );
}

sal_uInt16 XclExpNameManager::InsertRawName( const OUString& rName )
{
    return mxImpl->InsertRawName( rName );
}

sal_uInt16 XclExpNameManager::InsertMacroCall( const OUString& rMacroName, bool bVBasic, bool bFunc, bool bHidden )
{
    return mxImpl->InsertMacroCall( rMacroName, bVBasic, bFunc, bHidden );
}

const OUString& XclExpNameManager::GetOrigName( sal_uInt16 nNameIdx ) const
{
    const XclExpName* pName = mxImpl->GetName( nNameIdx );
    return pName ? pName->GetOrigName() : EMPTY_OUSTRING;
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

void XclExpNameManager::SaveXml( XclExpXmlStream& rStrm )
{
    mxImpl->SaveXml( rStrm );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
