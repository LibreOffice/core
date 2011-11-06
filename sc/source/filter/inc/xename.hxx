/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



#ifndef SC_XENAME_HXX
#define SC_XENAME_HXX

#include "xerecord.hxx"
#include "xlname.hxx"
#include "xlformula.hxx"
#include "xeroot.hxx"

// ============================================================================

class ScRangeList;
class XclExpNameManagerImpl;

/** Manager that stores all internal defined names (NAME records) of the document. */
class XclExpNameManager : public XclExpRecordBase, protected XclExpRoot
{
public:
    explicit            XclExpNameManager( const XclExpRoot& rRoot );
    virtual             ~XclExpNameManager();

    /** Creates NAME records for built-in and user defined names. */
    void                Initialize();

    /** Inserts the Calc name with the passed index and returns the Excel NAME index. */
    sal_uInt16          InsertName( sal_uInt16 nScNameIdx );
    /** Inserts the Calc database range with the passed index and returns the Excel NAME index. */
    sal_uInt16          InsertDBRange( sal_uInt16 nScDBRangeIdx );

//UNUSED2009-05 /** Inserts a new built-in defined name. */
//UNUSED2009-05 sal_uInt16          InsertBuiltInName( sal_Unicode cBuiltIn, XclTokenArrayRef xTokArr, SCTAB nScTab );
    /** Inserts a new built-in defined name, referring to the passed sheet range. */
    sal_uInt16          InsertBuiltInName( sal_Unicode cBuiltIn, const ScRange& rRange );
    /** Inserts a new built-in defined name, referring to the passed sheet range list. */
    sal_uInt16          InsertBuiltInName( sal_Unicode cBuiltIn, const ScRangeList& rRangeList );

    /** Inserts a new defined name. Sets another unused name, if rName already exists. */
    sal_uInt16          InsertUniqueName( const String& rName, XclTokenArrayRef xTokArr, SCTAB nScTab );
    /** Returns index of an existing name, or creates a name without definition. */
    sal_uInt16          InsertRawName( const String& rName );
    /** Searches or inserts a defined name describing a macro name.
        @param bVBasic  true = Visual Basic macro, false = Sheet macro.
        @param bFunc  true = Macro function; false = Macro procedure. */
    sal_uInt16          InsertMacroCall( const String& rMacroName, bool bVBasic, bool bFunc, bool bHidden = false );

    /** Returns the Calc sheet of a local defined name, or SCTAB_GLOBAL for global defined names. */
    const String&       GetOrigName( sal_uInt16 nNameIdx ) const;
    /** Returns the Calc sheet of a local defined name, or SCTAB_GLOBAL for global defined names. */
    SCTAB               GetScTab( sal_uInt16 nNameIdx ) const;
    /** Returns true, if the specified defined name is volatile. */
    bool                IsVolatile( sal_uInt16 nNameIdx ) const;

    /** Writes the entire list of NAME records. */
    virtual void        Save( XclExpStream& rStrm );
    virtual void        SaveXml( XclExpXmlStream& rStrm );

private:
    typedef ScfRef< XclExpNameManagerImpl > XclExpNameMgrImplRef;
    XclExpNameMgrImplRef mxImpl;
};

// ============================================================================

#endif

