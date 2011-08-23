/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 * 
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#ifndef SC_VALIDAT_HXX
#define SC_VALIDAT_HXX

#include "conditio.hxx"
namespace binfilter {

class ScPatternAttr;
class ScTokenArray;

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
//	Eintrag fuer Gueltigkeit (es gibt nur eine Bedingung)
//

class ScValidationData : public ScConditionEntry
{
    sal_uInt32			nKey;				// Index in Attributen

    ScValidationMode	eDataMode;
    BOOL				bShowInput;
    BOOL				bShowError;
    ScValidErrorStyle	eErrorStyle;
    String				aInputTitle;
    String				aInputMessage;
    String				aErrorTitle;
    String				aErrorMessage;

    BOOL				bIsUsed;			// temporaer beim Speichern


public:
            ScValidationData( ScValidationMode eMode, ScConditionMode eOper,
                                const String& rExpr1, const String& rExpr2,
                                ScDocument* pDocument, const ScAddress& rPos,
                                BOOL bCompileEnglish = FALSE, BOOL bCompileXML = FALSE );
            ScValidationData( const ScValidationData& r );
            ScValidationData( ScDocument* pDocument, const ScValidationData& r );
            ScValidationData( SvStream& rStream, ScMultipleReadHeader& rHdr,
                                ScDocument* pDocument );
            ~ScValidationData();

    void			Store(SvStream& rStream, ScMultipleWriteHeader& rHdr) const;

    ScValidationData* Clone() const		// echte Kopie
                    { return new ScValidationData( GetDocument(), *this ); }
    ScValidationData* Clone(ScDocument* pNew) const
                    { return new ScValidationData( pNew, *this ); }

    void			ResetInput();
    void			ResetError();
    void			SetInput( const String& rTitle, const String& rMsg );
    void			SetError( const String& rTitle, const String& rMsg,
                                ScValidErrorStyle eStyle );

    BOOL			GetInput( String& rTitle, String& rMsg ) const
                        { rTitle = aInputTitle; rMsg = aInputMessage; return bShowInput; }
    BOOL			GetErrMsg( String& rTitle, String& rMsg, ScValidErrorStyle& rStyle ) const;

    BOOL			HasErrMsg() const		{ return bShowError; }

    ScValidationMode GetDataMode() const	{ return eDataMode; }


    BOOL			IsDataValid( ScBaseCell* pCell, const ScAddress& rPos ) const;

                    // TRUE -> Abbruch

    BOOL			IsEmpty() const;
    sal_uInt32		GetKey() const			{ return nKey; }
    void			SetKey(sal_uInt32 nNew)		{ nKey = nNew; }	// nur wenn nicht eingefuegt!

    void			SetUsed(BOOL bSet)		{ bIsUsed = bSet; }
    BOOL			IsUsed() const			{ return bIsUsed; }

    BOOL			EqualEntries( const ScValidationData& r ) const;	// fuer Undo

    //	sortiert (per PTRARR) nach Index
    //	operator== nur fuer die Sortierung
    BOOL operator ==( const ScValidationData& r ) const	{ return nKey == r.nKey; }
    BOOL operator < ( const ScValidationData& r ) const	{ return nKey <  r.nKey; }
};

//
//	Liste der Bedingungen:
//

typedef ScValidationData* ScValidationDataPtr;

SV_DECL_PTRARR_SORT(ScValidationEntries_Impl, ScValidationDataPtr,
                        SC_COND_GROW, SC_COND_GROW)//STRIP008 ;

class ScValidationDataList : public ScValidationEntries_Impl
{
public:
        ScValidationDataList() {}
        ScValidationDataList(const ScValidationDataList& rList);
        ~ScValidationDataList() {}

    void	InsertNew( ScValidationData* pNew )
                { if (!Insert(pNew)) delete pNew; }

    ScValidationData* GetData( sal_uInt32 nKey );

    void	Load( SvStream& rStream, ScDocument* pDocument );
    void	Store( SvStream& rStream ) const;
    void	ResetUsed();

    void	CompileXML();

};

} //namespace binfilter
#endif


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
