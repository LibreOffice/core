/*************************************************************************
 *
 *  $RCSfile: validat.hxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: hjs $ $Date: 2003-08-19 11:34:11 $
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

#ifndef SC_VALIDAT_HXX
#define SC_VALIDAT_HXX

#ifndef SC_CONDITIO_HXX
#include "conditio.hxx"
#endif
#ifndef _COM_SUN_STAR_SHEET_TABLEVALIDATIONVISIBILITY_HPP_
#include <com/sun/star/sheet/TableValidationVisibility.hpp>
#endif

namespace ValidListType = ::com::sun::star::sheet::TableValidationVisibility;

class ScPatternAttr;
class ScTokenArray;
class TypedStrCollection;

enum ScValidationMode
{
    SC_VALID_ANY,
    SC_VALID_WHOLE,
    SC_VALID_DECIMAL,
    SC_VALID_DATE,
    SC_VALID_TIME,
    SC_VALID_TEXTLEN,
    SC_VALID_LIST,
    SC_VALID_CUSTOM
};

enum ScValidErrorStyle
{
    SC_VALERR_STOP,
    SC_VALERR_WARNING,
    SC_VALERR_INFO,
    SC_VALERR_MACRO
};

//
//  Eintrag fuer Gueltigkeit (es gibt nur eine Bedingung)
//

class ScValidationData : public ScConditionEntry
{
    ULONG               nKey;               // Index in Attributen

    ScValidationMode    eDataMode;
    BOOL                bShowInput;
    BOOL                bShowError;
    ScValidErrorStyle   eErrorStyle;
    sal_Int16           mnListType;         // selection list type: none, unsorted, sorted.
    String              aInputTitle;
    String              aInputMessage;
    String              aErrorTitle;
    String              aErrorMessage;

    BOOL                bIsUsed;            // temporaer beim Speichern

    BOOL            DoMacro( const ScAddress& rPos, const String& rInput,
                                ScFormulaCell* pCell, Window* pParent ) const;

public:
            ScValidationData( ScValidationMode eMode, ScConditionMode eOper,
                                const String& rExpr1, const String& rExpr2,
                                ScDocument* pDocument, const ScAddress& rPos,
                                BOOL bCompileEnglish = FALSE, BOOL bCompileXML = FALSE );
            ScValidationData( ScValidationMode eMode, ScConditionMode eOper,
                                const ScTokenArray* pArr1, const ScTokenArray* pArr2,
                                ScDocument* pDocument, const ScAddress& rPos );
            ScValidationData( const ScValidationData& r );
            ScValidationData( ScDocument* pDocument, const ScValidationData& r );
            ScValidationData( SvStream& rStream, ScMultipleReadHeader& rHdr,
                                ScDocument* pDocument );
            ~ScValidationData();

    void            Store(SvStream& rStream, ScMultipleWriteHeader& rHdr) const;

    ScValidationData* Clone() const     // echte Kopie
                    { return new ScValidationData( GetDocument(), *this ); }
    ScValidationData* Clone(ScDocument* pNew) const
                    { return new ScValidationData( pNew, *this ); }

    void            ResetInput();
    void            ResetError();
    void            SetInput( const String& rTitle, const String& rMsg );
    void            SetError( const String& rTitle, const String& rMsg,
                                ScValidErrorStyle eStyle );

    BOOL            GetInput( String& rTitle, String& rMsg ) const
                        { rTitle = aInputTitle; rMsg = aInputMessage; return bShowInput; }
    BOOL            GetErrMsg( String& rTitle, String& rMsg, ScValidErrorStyle& rStyle ) const;

    BOOL            HasErrMsg() const       { return bShowError; }

    ScValidationMode GetDataMode() const    { return eDataMode; }

    inline sal_Int16 GetListType() const                { return mnListType; }
    inline void     SetListType( sal_Int16 nListType )  { mnListType = nListType; }

    /** Returns true, if the validation cell will show a selection list.
        @descr  Use this instead of GetListType() which returns the raw property
        regardless of the validation type. */
    bool            HasSelectionList() const;
    /** Tries to fill the passed collection with list validation entries.
        @descr  Fills the list only, if this is a list validation and IsShowList() is enabled.
        @param rStrings  (out-param) The string list to fill with list validation entires.
        @return  true = rStrings has been filled with at least one entry. */
    bool            FillSelectionList( TypedStrCollection& rStrings, const ScAddress rPos ) const;

                    //  mit String: bei Eingabe, mit Zelle: fuer Detektiv / RC_FORCED
    BOOL            IsDataValid( const String& rTest, const ScPatternAttr& rPattern,
                                    const ScAddress& rPos ) const;
    BOOL            IsDataValid( ScBaseCell* pCell, const ScAddress& rPos ) const;

                    // TRUE -> Abbruch
    BOOL            DoError( Window* pParent, const String& rInput, const ScAddress& rPos ) const;
    void            DoCalcError( ScFormulaCell* pCell ) const;

    BOOL            IsEmpty() const;
    ULONG           GetKey() const          { return nKey; }
    void            SetKey(ULONG nNew)      { nKey = nNew; }    // nur wenn nicht eingefuegt!

    void            SetUsed(BOOL bSet)      { bIsUsed = bSet; }
    BOOL            IsUsed() const          { return bIsUsed; }

    BOOL            EqualEntries( const ScValidationData& r ) const;    // fuer Undo

    //  sortiert (per PTRARR) nach Index
    //  operator== nur fuer die Sortierung
    BOOL operator ==( const ScValidationData& r ) const { return nKey == r.nKey; }
    BOOL operator < ( const ScValidationData& r ) const { return nKey <  r.nKey; }

private:
    /** Tries to get a cell range from a list validation formula.
        @descr  The formula may contain a cell reference, a defined name or a database range.
        @param rRange  (out-param) The resulting cell range.
        @param rBaseAddr  Base address for relative references.
        @return  true = Cell range found, rRange is valid. */
    bool            GetRangeFromFormula( ScRange& rRange, const ScAddress& rBaseAddr, ScTokenArray& rTokArr, int nRecCount = 0 ) const;

    /** Tests, if pCell is equal to what the passed token array represents. */
    bool            IsEqualToTokenArray( ScBaseCell* pCell, const ScAddress& rPos, const ScTokenArray& rTokArr ) const;

    /** Tests, if contents of pCell occur in cell range referenced by own formula, or in a string list. */
    bool            IsListValid( ScBaseCell* pCell, const ScAddress& rPos ) const;
};

//
//  Liste der Bedingungen:
//

typedef ScValidationData* ScValidationDataPtr;

SV_DECL_PTRARR_SORT(ScValidationEntries_Impl, ScValidationDataPtr,
                        SC_COND_GROW, SC_COND_GROW);

class ScValidationDataList : public ScValidationEntries_Impl
{
public:
        ScValidationDataList() {}
        ScValidationDataList(const ScValidationDataList& rList);
        ScValidationDataList(ScDocument* pNewDoc, const ScValidationDataList& rList);
        ~ScValidationDataList() {}

    void    InsertNew( ScValidationData* pNew )
                { if (!Insert(pNew)) delete pNew; }

    ScValidationData* GetData( ULONG nKey );

    void    Load( SvStream& rStream, ScDocument* pDocument );
    void    Store( SvStream& rStream ) const;
    void    ResetUsed();

    void    CompileXML();
    void    UpdateReference( UpdateRefMode eUpdateRefMode,
                                const ScRange& rRange, short nDx, short nDy, short nDz );

    BOOL    operator==( const ScValidationDataList& r ) const;      // fuer Ref-Undo
};

#endif

