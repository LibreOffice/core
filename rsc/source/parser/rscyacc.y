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

%{
#if defined __GNUC__
#pragma GCC system_header
#elif defined __SUNPRO_CC
#pragma disable_warn
#endif
%}

/* Compilerstack */

%union {
	Atom			varid;
	struct {
		Atom			hashid;
		sal_Int32			nValue;
	}				constname;
	RscTop *		pClass;
	RSCHEADER		header;
	struct {
		CLASS_DATA	pData;
		RscTop *	pClass;
	}				instance;
	sal_Int32			value;
	sal_uInt16			ushort;
	short			exp_short;
	char *			string;
	sal_Bool			svbool;
	REF_ENUM		copyref;
	RscDefine	  * defineele;
	CharSet 		charset;
	RscExpType		macrostruct;
}

/* Token */
%token <value>			NUMBER
%token <string> 		SYMBOL
%token <defineele>		RSCDEFINE
%token <string> 		STRING
%token <string> 		INCLUDE_STRING
%token <character>		CHARACTER
%token <svbool> 		BOOLEAN

%token LINE
%token AUTO_ID
%token NOT
%token XSCALE
%token YSCALE
%token RGB
%token GEOMETRY
%token POSITION
%token DIMENSION
%token INZOOMOUTPUTSIZE
%token FLOATINGPOS
%token DEFINE
%token INCLUDE
%token MACROTARGET
%token DEFAULT


%token <pClass> 		CLASSNAME
%token <varid>			VARNAME
%token <constname>		CONSTNAME
%token CLASS
%token EXTENDABLE
%token WRITEIFSET


%type  <macrostruct>	macro_expression
%type  <macrostruct>	id_expression
%type  <value>			long_expression
%type  <string> 		string_multiline

%type  <pClass> 		type
%type  <pClass> 		type_base
%type  <header> 		class_header_body
%type  <header> 		class_header
%type  <header> 		var_header_class
%type  <copyref>		copy_ref
%type  <ushort> 		type_flags


%left '|'
%left '&'
%left LEFTSHIFT RIGHTSHIFT
%left '+' '-'
%left '*' '/'
%left UNARYMINUS
%left UNARYPLUS
%left ','
%left '(' ')'


/*	Grammatik  */

%start resource_definitions

%%

/********************** D E F I N I T I O N S ****************************/
resource_definitions
  :
  | resource_definitions resource_definition
  | MACROTARGET macro_expression
	{
		RscExpType		aExpType;
		sal_Int32			lValue;

		aExpType.cType = RSCEXP_NOTHING;
		pExp = new RscExpression( aExpType, '+', $2 );
		if( !pExp->Evaluate( &lValue ) )
			pTC->pEH->Error( ERR_ZERODIVISION, NULL, RscId() );
	}
  ;

resource_definition
  : line_number
  | '#' DEFINE SYMBOL macro_expression
	{
		sal_Bool		bError = sal_False;

		if( $4.IsNumber() ){
			if( !pTC->aFileTab.NewDef( pFI->GetFileIndex(),
									   ByteString( $3 ),
									   $4.GetLong(), LIST_APPEND ) )
				bError = sal_True;
		}
		else if( $4.IsDefinition() ){
			RscExpType		aExpType;
			RscExpression * pExpr;

			aExpType.cType = RSCEXP_NOTHING;
			aExpType.SetLong( 0 );
			aExpType.cType = RSCEXP_LONG;
			pExpr = new RscExpression( aExpType, '+', $4 );

			if( !pTC->aFileTab.NewDef( pFI->GetFileIndex(),
									   ByteString( $3 ), pExpr, LIST_APPEND ) )
				bError = sal_True;
		}
		else if( $4.IsExpression() ){
			if( !pTC->aFileTab.NewDef( pFI->GetFileIndex(),
									   ByteString( $3 ), $4.aExp.pExp,
									   LIST_APPEND ) )
				bError = sal_True;
		}

		if( bError ){
			pTC->pEH->Error( ERR_DECLAREDEFINE, NULL, RscId(), $3 );
		}
	}
  | '#' DEFINE RSCDEFINE macro_expression
	{
		pTC->pEH->Error( ERR_DOUBLEDEFINE, NULL, RscId(), $3->GetName().GetBuffer() );
	}
  | '#' INCLUDE STRING
	{
	}
  | '#' INCLUDE INCLUDE_STRING
	{
	}
  | class_definition ';'
	{
#ifdef D40
			void * pMem;
			pMem = rtl_allocateMemory( 20000 );
			rtl_freeMemory( pMem );
#endif
	}
  | new_class_definition_header '{' new_class_definition_body '}' ';'
  | new_class_definition_header ';'
  ;

new_class_definition_header
  : CLASS SYMBOL id_expression ':' CLASSNAME
	{
		sal_Int32	lType;

		$3.Evaluate( &lType );

		// Klasse anlegen
		Atom nId = pHS->getID( $2 );
		pCurClass = new RscClass( nId, lType, $5 );
		nCurMask = 1;
		pTC->aNmTb.Put( nId, CLASSNAME, pCurClass );
		pTC->GetRoot()->Insert( pCurClass );
	}
  | CLASS CLASSNAME id_expression ':' CLASSNAME
	{
		pCurClass = $2;
		nCurMask = 1;
	}
  ;

new_class_definition_body
  :
  | property_definition ';' new_class_definition_body
  ;

property_definition
  : type_flags type SYMBOL
  {
	// Variable anlegen
	Atom nId = pTC->aNmTb.Put( $3, VARNAME );
	pCurClass->SetVariable( nId, $2, NULL, $1, nCurMask );
	nCurMask <<= 1;
  }
  | type_flags type VARNAME
  {
	pCurClass->SetVariable( $3, $2, NULL, $1, nCurMask );
	nCurMask <<= 1;
  }
  ;

type_flags
  : type_flags EXTENDABLE
  {
	$$ = $1 | VAR_EXTENDABLE;
  }
  | type_flags WRITEIFSET
  {
	$$ = $1 | VAR_SVDYNAMIC;
  }
  |
  {
	$$ = 0;
  }
  ;

type
  : type_base
  {
	$$ = $1;
  }
  | type_base '[' ']'
  {
	if( $1 )
	{
		ByteString aTypeName = pHS->getString( $1->GetId() );
		aTypeName += "[]";
		$$ = pTC->SearchType( pHS->getID( aTypeName.GetBuffer(), true ) );
		if( !$$ )
		{
			RscCont * pCont;
			pCont = new RscCont( pHS->getID( aTypeName.GetBuffer() ), RSC_NOTYPE );
			pCont->SetTypeClass( $1 );
			pTC->InsertType( pCont );
			$$ = pCont;
		}
	}
	else
		$$ = NULL;
  }
  ;

type_base
  : CLASSNAME
  {
	$$ = $1;
  }
  | SYMBOL
  {
	RscTop * pType = pTC->SearchType( pHS->getID( $1, true ) );
	if( !pType )
		pTC->pEH->Error( ERR_NOTYPE, pCurClass, RscId() );
	$$ = pType;
  }
  ;

class_definition
  : class_header class_body
  {
	if( TYPE_REF == $1.nTyp )
		pTC->pEH->Error( ERR_REFNOTALLOWED, S.Top().pClass,
						 RscId( $1.nName1 ) );
	S.Pop();
  }
  | class_header
  {
	ERRTYPE aError;
	RscId aRscId( $1.nName1 );

	if( TYPE_NOTHING == $1.nTyp && aRscId.IsId() )
		aError = S.Top().pClass->SetRef( S.Top(), aRscId );
	else if( TYPE_COPY == $1.nTyp )
		aError = ERR_COPYNOTALLOWED;
	if( aError.IsError() || aError.IsWarning() )
		pTC->pEH->Error( aError, S.Top().pClass, aRscId );
	S.Pop();
  }
  ;

class_header
  : class_header_body
	{
		if( !DoClassHeader( &$1, sal_False ) )
			return( ERR_ERROR );
		$$ = $1;
	}
  ;

copy_ref
  : '<'
	{
		$$ = TYPE_COPY;
	}
  | ','
	{
		$$ = TYPE_REF;
	}
  ;

class_header_body
  : CLASSNAME id_expression copy_ref CLASSNAME id_expression
	{
		$$.pClass = $1;
		$$.nName1 = $2;
		$$.nTyp = $3;
		$$.pRefClass = $4;
		$$.nName2 = $5;
	}
  | CLASSNAME id_expression copy_ref id_expression
	{
		$$.pClass = $1;
		$$.nName1 = $2;
		$$.nTyp = $3;
		$$.pRefClass = NULL;
		$$.nName2 = $4;
	}
  | CLASSNAME id_expression
	{
		$$.pClass = $1;
		$$.nName1 = $2;
		$$.nTyp = TYPE_NOTHING;
		$$.pRefClass = NULL;
		$$.nName2.cType = RSCEXP_NOTHING;
	}
  | CLASSNAME copy_ref id_expression
	{
		$$.pClass = $1;
		$$.nName1.cType = RSCEXP_NOTHING;
		$$.nTyp = $2;
		$$.pRefClass = NULL;
		$$.nName2 = $3;
	}
  | CLASSNAME copy_ref CLASSNAME id_expression
	{
		$$.pClass = $1;
		$$.nName1.cType = RSCEXP_NOTHING;
		$$.nTyp = $2;
		$$.pRefClass = $3;
		$$.nName2 = $4;
	}
  | CLASSNAME
	{
		$$.pClass = $1;
		$$.nName1.cType = RSCEXP_NOTHING;
		$$.nTyp = TYPE_NOTHING;
		$$.nName2.cType = RSCEXP_NOTHING;
	}
  ;

class_body
  : '{' var_definitions '}'
  | '{' '}'
  | string_multiline
	{
		SetString( S.Top(), "TEXT", $1 );
	}
  ;

var_definitions
  : var_definition
  | var_definitions var_definition
  ;

xy_mapmode
  : CONSTNAME
	{
		SetConst( S.Top(), "_XYMAPMODE", $1.hashid, $1.nValue );
	}
  |
  ;

wh_mapmode
  : CONSTNAME
	{
		SetConst( S.Top(), "_WHMAPMODE", $1.hashid, $1.nValue );
	}
  |
  ;

xywh_mapmode
  : CONSTNAME
	{
		SetConst( S.Top(), "_XYMAPMODE", $1.hashid, $1.nValue );
		SetConst( S.Top(), "_WHMAPMODE", $1.hashid, $1.nValue );
	}
  |
  ;

var_definition
  : line_number
  | var_header var_body ';'
	{
		S.Pop();
	}
  | class_definition ';'
  | var_header_class class_body ';'
	{
		if( TYPE_REF == $1.nTyp )
			pTC->pEH->Error( ERR_REFNOTALLOWED, S.Top().pClass,
							 RscId( $1.nName1 ) );

		if( S.Top().pClass->GetCount( S.Top() ) )
			pTC->pEH->Error( WRN_SUBINMEMBER, S.Top().pClass,
							 RscId(  $1.nName1 ) );

		S.Pop();
	}
  | var_header_class ';'
	{
		ERRTYPE aError;
		RscId aRscId( $1.nName1 );

		if( TYPE_NOTHING == $1.nTyp && aRscId.IsId() )
			aError = S.Top().pClass->SetRef( S.Top(), aRscId );
		else if( TYPE_COPY == $1.nTyp )
			aError = ERR_COPYNOTALLOWED;
		if( S.Top().pClass->GetCount( S.Top() ) )
			aError = WRN_SUBINMEMBER;
		if( aError.IsError() || aError.IsWarning() )
			pTC->pEH->Error( aError, S.Top().pClass, aRscId );

		S.Pop();
	}
  | XSCALE '=' '(' long_expression ',' long_expression ')' ';'
	{
		SetNumber( S.Top(), "_XNUMERATOR", $4 );
		SetNumber( S.Top(), "_XDENOMINATOR", $6 );
	}
  | YSCALE '=' '(' long_expression ',' long_expression ')' ';'
	{
		SetNumber( S.Top(), "_YNUMERATOR", $4 );
		SetNumber( S.Top(), "_YDENOMINATOR", $6 );
	}
  | RGB '=' '(' long_expression ',' long_expression
				',' long_expression ')' ';'
	{
		SetNumber( S.Top(), "RED", $4 );
		SetNumber( S.Top(), "GREEN", $6 );
		SetNumber( S.Top(), "BLUE", $8 );
	}
  | GEOMETRY '=' xywh_mapmode '(' long_expression ',' long_expression ','
						long_expression ',' long_expression ')' ';'
	{
		SetNumber( S.Top(), "_X", $5 );
		SetNumber( S.Top(), "_Y", $7 );
		SetNumber( S.Top(), "_WIDTH", $9 );
		SetNumber( S.Top(), "_HEIGHT", $11 );
	}
  | POSITION '=' xy_mapmode '(' long_expression ',' long_expression
							')' ';'
	{
		SetNumber( S.Top(), "_X", $5 );
		SetNumber( S.Top(), "_Y", $7 );
	}
  | DIMENSION '=' wh_mapmode '(' long_expression ',' long_expression
							 ')' ';'
	{
		SetNumber( S.Top(), "_WIDTH", $5 );
		SetNumber( S.Top(), "_HEIGHT", $7 );
	}
  | INZOOMOUTPUTSIZE '=' CONSTNAME '(' long_expression ',' long_expression
							 ')' ';'
	{
		SetConst( S.Top(), "_ZOOMINMAPMODE", $3.hashid, $3.nValue );
		SetNumber( S.Top(), "_ZOOMINWIDTH", $5 );
		SetNumber( S.Top(), "_ZOOMINHEIGHT", $7 );
	}
  | INZOOMOUTPUTSIZE '=' '(' long_expression ',' long_expression ')' ';'
	{
		SetNumber( S.Top(), "_ZOOMINWIDTH", $4 );
		SetNumber( S.Top(), "_ZOOMINHEIGHT", $6 );
	}
  | FLOATINGPOS '=' CONSTNAME '(' long_expression ',' long_expression
							 ')' ';'
	{
		SetConst( S.Top(),	"_FLOATINGPOSMAPMODE", $3.hashid, $3.nValue );
		SetNumber( S.Top(), "_FLOATINGPOSX", $5 );
		SetNumber( S.Top(), "_FLOATINGPOSY", $7 );
	}
  | FLOATINGPOS '=' '(' long_expression ',' long_expression ')' ';'
	{
		SetNumber( S.Top(), "_FLOATINGPOSX", $4 );
		SetNumber( S.Top(), "_FLOATINGPOSY", $6 );
	}
  ;

var_header_class
  : VARNAME '=' class_header_body
	{
		RSCINST aInst;

		aInst = S.Top().pClass->GetVariable( S.Top(), $1, RSCINST(), sal_False, $3.pClass );

		if( aInst.pData )
			S.Push( aInst );
		else
		{
			pTC->pEH->Error( ERR_NOVARIABLENAME, S.Top().pClass, RscId(),
							 pHS->getString( $1 )  );
			return( ERR_ERROR );
		};

		if( !DoClassHeader( &$3, sal_True ) )
			return( ERR_ERROR );
		$$ = $3;
	}
  | VARNAME '[' CONSTNAME ']' '=' class_header_body
	{
		RSCINST aInst;

		aInst = S.Top().pClass->GetVariable( S.Top(), $1, RSCINST() );

		if( aInst.pData )
		{
			ERRTYPE aError;
			RSCINST aIdxInst;

			aError = aInst.pClass->GetArrayEle( aInst, $3.hashid, NULL, &aIdxInst );
			if( aError.IsError() || aError.IsWarning() )
				 pTC->pEH->Error( aError, S.Top().pClass, RscId() );
			if( aError.IsError() )
				return( ERR_ERROR );
			S.Push( aIdxInst );
		}
		else
		{
			pTC->pEH->Error( ERR_NOVARIABLENAME, S.Top().pClass, RscId(),
							 pHS->getString( $1 )  );
			return( ERR_ERROR );
		};
		if( !DoClassHeader( &$6, sal_True ) )
			return( ERR_ERROR );
		$$ = $6;
	}
  | VARNAME '[' SYMBOL ']' '=' class_header_body
	{
		RSCINST aInst;

		aInst = S.Top().pClass->GetVariable( S.Top(), $1, RSCINST() );

		if( aInst.pData )
		{
            long nNewLang = pTC->AddLanguage( $3 );
			ERRTYPE aError;
			RSCINST aIdxInst;

			aError = aInst.pClass->GetArrayEle( aInst, nNewLang, NULL, &aIdxInst );
			if( aError.IsError() || aError.IsWarning() )
				 pTC->pEH->Error( aError, S.Top().pClass, RscId() );
			if( aError.IsError() )
				return( ERR_ERROR );
			S.Push( aIdxInst );
		}
		else
		{
			pTC->pEH->Error( ERR_NOVARIABLENAME, S.Top().pClass, RscId(),
							 pHS->getString( $1 )  );
			return( ERR_ERROR );
		};
		if( !DoClassHeader( &$6, sal_True ) )
			return( ERR_ERROR );
		$$ = $6;
	}
  ;

var_header
  : VARNAME '='
	{
		RSCINST aInst;

		aInst = S.Top().pClass->GetVariable( S.Top(), $1, RSCINST() );

		if( aInst.pData )
			S.Push( aInst );
		else{
			pTC->pEH->Error( ERR_NOVARIABLENAME, S.Top().pClass, RscId(),
							 pHS->getString( $1 )  );
			return( ERR_ERROR );
		};
	}
  | VARNAME '[' CONSTNAME ']' '='
	{
		RSCINST aInst;

		aInst = S.Top().pClass->GetVariable( S.Top(), $1, RSCINST() );

		if( aInst.pData )
		{
			ERRTYPE aError;
			RSCINST aIdxInst;

			aError = aInst.pClass->GetArrayEle( aInst, $3.hashid, NULL, &aIdxInst );
			if( aError.IsError() || aError.IsWarning() )
				 pTC->pEH->Error( aError, S.Top().pClass, RscId() );
			if( aError.IsError() )
				return( ERR_ERROR );
			S.Push( aIdxInst );
		}
		else{
			pTC->pEH->Error( ERR_NOVARIABLENAME, S.Top().pClass, RscId(),
							 pHS->getString( $1 )  );
			return( ERR_ERROR );
		};
	}
  | VARNAME '[' SYMBOL ']' '='
	{
		RSCINST aInst;

		aInst = S.Top().pClass->GetVariable( S.Top(), $1, RSCINST() );

		if( aInst.pData )
		{
            long nNewLang = pTC->AddLanguage( $3 );
			ERRTYPE aError;
			RSCINST aIdxInst;

			aError = aInst.pClass->GetArrayEle( aInst, nNewLang, NULL, &aIdxInst );
			if( aError.IsError() || aError.IsWarning() )
				 pTC->pEH->Error( aError, S.Top().pClass, RscId() );
			if( aError.IsError() )
				return( ERR_ERROR );
			S.Push( aIdxInst );
		}
		else{
			pTC->pEH->Error( ERR_NOVARIABLENAME, S.Top().pClass, RscId(),
							 pHS->getString( $1 )  );
			return( ERR_ERROR );
		};
	}
  ;
tupel_header0
  :
	{
		RSCINST aInst;

		aInst = S.Top().pClass->GetTupelVar( S.Top(), 0, RSCINST() );
		if( aInst.pData )
			S.Push( aInst );
		else
		{
			pTC->pEH->Error( ERR_NOTUPELNAME, S.Top().pClass, RscId() );
			return( ERR_ERROR );
		};
	}
  ;

tupel_header1
  :
	{
		RSCINST aInst;

		aInst = S.Top().pClass->GetTupelVar( S.Top(), 1, RSCINST() );
		if( aInst.pData )
			S.Push( aInst );
		else
		{
			pTC->pEH->Error( ERR_NOTUPELNAME, S.Top().pClass, RscId() );
			return( ERR_ERROR );
		};
	}
  ;

tupel_header2
  :
	{
		RSCINST aInst;

		aInst = S.Top().pClass->GetTupelVar( S.Top(), 2, RSCINST() );
		if( aInst.pData )
			S.Push( aInst );
		else
		{
			pTC->pEH->Error( ERR_NOTUPELNAME, S.Top().pClass, RscId() );
			return( ERR_ERROR );
		};
	}
  ;

tupel_header3
  :
	{
		RSCINST aInst;

		aInst = S.Top().pClass->GetTupelVar( S.Top(), 3, RSCINST() );
		if( !aInst.pData )
		{
			pTC->pEH->Error( ERR_NOTUPELNAME, S.Top().pClass, RscId() );
			return( ERR_ERROR );
		};
		S.Push( aInst );
	}
  ;

tupel_body
  : var_body
	{
		S.Pop();
	}
  ;

var_list_header
  :
	{
		ERRTYPE aError;
		RSCINST aInst;

		aError = S.Top().pClass->GetElement( S.Top(), RscId(),
											NULL, RSCINST(), &aInst );
		if( aError.IsError() || aError.IsWarning() )
			pTC->pEH->Error( aError, S.Top().pClass, RscId() );
		if( aError.IsError() )
		{ // unbedingt Instanz auf den Stack bringen
			aInst = S.Top().pClass->Create( NULL, RSCINST() );
		}
		S.Push( aInst );
	}
  ;

list_body
  : var_bodycomplex
	{
		S.Pop();
	}
  ;

list_header
  :
	{
		sal_uInt32 nCount = S.Top().pClass->GetCount( S.Top() );
		sal_uInt32 i;

		for( i = nCount; i > 0; i-- )
			S.Top().pClass->DeletePos( S.Top(), i -1 );
	}
  ;

list
  : list var_list_header list_body ';'
  | list var_bodysimple ';'
  | list class_definition ';'
  | list line_number
  |
  ;

var_bodysimple
  : macro_expression
	{
		sal_Int32	l;
		ERRTYPE aError;

		if( !$1.Evaluate( &l ) )
			pTC->pEH->Error( ERR_ZERODIVISION, NULL, RscId() );
		else
		{
			aError = S.Top().pClass->SetRef( S.Top(), RscId( $1 ) );
			if( aError.IsError() )
			{
				aError.Clear();
				aError = S.Top().pClass->SetNumber( S.Top(), l );
			}
			if( aError.IsError() )
			{ // Aufwaertskompatible, Tupel probieren
				RSCINST aInst = GetFirstTupelEle( S.Top() );
				if( aInst.pData )
				{
					aError.Clear(); // Fehler zuruecksetzen
					aError = aInst.pClass->SetRef( aInst, RscId( $1 ) );
					if( aError.IsError() )
					{
						aError.Clear();
						aError = aInst.pClass->SetNumber( aInst, l );
					}
				}
			}
		}

		if( $1.IsExpression() )
			delete $1.aExp.pExp;

		if( aError.IsError() || aError.IsWarning() )
			pTC->pEH->Error( aError, S.Top().pClass, RscId() );
	}
  | CONSTNAME
	{
		ERRTYPE aError;
		aError = S.Top().pClass->SetConst( S.Top(), $1.hashid, $1.nValue );
		if( aError.IsError() )
		{ // Aufwaertskompatible, Tupel probieren
			RSCINST aInst = GetFirstTupelEle( S.Top() );
			if( aInst.pData )
			{
				aError.Clear(); // Fehler zuruecksetzen
				aError = aInst.pClass->SetConst( aInst, $1.hashid, $1.nValue );
			}
		}

		if( aError.IsError() || aError.IsWarning() )
			pTC->pEH->Error( aError, S.Top().pClass, RscId() );
	}
  | NOT CONSTNAME
	{
		ERRTYPE aError;
		aError = S.Top().pClass->SetNotConst( S.Top(), $2.hashid );
		if( aError.IsError() )
		{ // Aufwaertskompatible, Tupel probieren
			RSCINST aInst = GetFirstTupelEle( S.Top() );
			if( aInst.pData )
			{
				aError.Clear(); // Fehler zuruecksetzen
				aError = aInst.pClass->SetNotConst( aInst, $2.hashid );
			}
		}

		if( aError.IsError() || aError.IsWarning() )
			pTC->pEH->Error( aError, S.Top().pClass, RscId() );
	}
  | BOOLEAN
	{
		ERRTYPE aError;
		aError = S.Top().pClass->SetBool( S.Top(), $1 );
		if( aError.IsError() )
		{ // Aufwaertskompatible, Tupel probieren
			RSCINST aInst = GetFirstTupelEle( S.Top() );
			if( aInst.pData )
			{
				aError.Clear(); // Fehler zuruecksetzen
				aError = aInst.pClass->SetBool( aInst, $1 );
			}
		}

		if( aError.IsError() || aError.IsWarning() )
			pTC->pEH->Error( aError, S.Top().pClass, RscId() );
	}
  | string_multiline
	{
		ERRTYPE aError;
		aError = S.Top().pClass->SetString( S.Top(), $1 );
		if( aError.IsError() )
		{ // Aufwaertskompatible, Tupel probieren
			RSCINST aInst = GetFirstTupelEle( S.Top() );
			if( aInst.pData )
			{
				aError.Clear(); // Fehler zuruecksetzen
				aError = aInst.pClass->SetString( aInst, $1 );
			}
		}

		if( aError.IsError() || aError.IsWarning() )
			pTC->pEH->Error( aError, S.Top().pClass, RscId() );
	}
  | DEFAULT
  ;

var_bodycomplex
  : '{' list_header list '}'
  | '<' tupel_header0 tupel_body ';' '>'
  | '<' tupel_header0 tupel_body ';' tupel_header1 tupel_body ';' '>'
  | '<' tupel_header0 tupel_body ';' tupel_header1 tupel_body ';'
		tupel_header2 tupel_body ';' '>'
  | '<' tupel_header0 tupel_body ';' tupel_header1 tupel_body ';'
		tupel_header2 tupel_body ';' tupel_header3 tupel_body ';' '>'
  ;

var_body
  : var_bodysimple
  | var_bodycomplex
  ;

/********************** work on yacc stack *******************************/
string_multiline
  : STRING
	{
		$$ = $1;
	}
  | string_multiline STRING
	{
        rtl::OStringBuffer aBuf( 256 );
        aBuf.append( $1 );
        aBuf.append( $2 );
        $$ = (char*)pStringContainer->putString( aBuf.getStr() );
	}
  ;

long_expression
  : macro_expression
	{
		if( !$1.Evaluate( &$$ ) )
			pTC->pEH->Error( ERR_ZERODIVISION, NULL, RscId() );
		if( $1.IsExpression() )
			delete $1.aExp.pExp;
	}
  ;

macro_expression
  : RSCDEFINE
	{
		$$.cType = RSCEXP_DEF;
		$$.aExp.pDef = $1;
	}
  | NUMBER
	{
		$$.cType = RSCEXP_LONG;
		$$.SetLong( $1 );
	}
  | '-' macro_expression %prec UNARYMINUS
	{
		if( $2.IsNumber() ){
			$$.cType = $2.cType;
			$$.SetLong( - $2.GetLong() );
		}
		else{
			RscExpType aLeftExp;

			aLeftExp.cType = RSCEXP_NOTHING;
			$$.cType = RSCEXP_EXP;
			$$.aExp.pExp = new RscExpression( aLeftExp, '-', $2 );
		}
	}
  | '+' macro_expression %prec UNARYPLUS
	{
		$$ = $2;
	}
  | macro_expression '+' macro_expression
	{
		if( $1.IsNumber() && $3.IsNumber() ){
			$$.cType = RSCEXP_LONG;
			$$.SetLong( $1.GetLong() + $3.GetLong() );
		}
		else{
			$$.cType = RSCEXP_EXP;
			$$.aExp.pExp = new RscExpression( $1, '+', $3 );
		}
	}
  | macro_expression '-' macro_expression
	{
		if( $1.IsNumber() && $3.IsNumber() ){
			$$.cType = RSCEXP_LONG;
			$$.SetLong( $1.GetLong() - $3.GetLong() );
		}
		else{
			$$.cType = RSCEXP_EXP;
			$$.aExp.pExp = new RscExpression( $1, '-', $3 );
		}
	}
  | macro_expression '*' macro_expression
	{
		if( $1.IsNumber() && $3.IsNumber() ){
			$$.cType = RSCEXP_LONG;
			$$.SetLong( $1.GetLong() * $3.GetLong() );
		}
		else{
			$$.cType = RSCEXP_EXP;
			$$.aExp.pExp = new RscExpression( $1, '*', $3 );
		}
	}
  | macro_expression '/' macro_expression
	{
		if( $1.IsNumber() && $3.IsNumber() ){
			if( 0 == $3.GetLong() ){
				$$.cType = RSCEXP_EXP;
				$$.aExp.pExp = new RscExpression( $1, '/', $3 );
			}
			else{
				$$.cType = RSCEXP_LONG;
				$$.SetLong( $1.GetLong() / $3.GetLong() );
			}
		}
		else{
			$$.cType = RSCEXP_EXP;
			$$.aExp.pExp = new RscExpression( $1, '/', $3 );
		}
	}
  | macro_expression '&' macro_expression
	{
		if( $1.IsNumber() && $3.IsNumber() ){
			$$.cType = RSCEXP_LONG;
			$$.SetLong( $1.GetLong() & $3.GetLong() );
		}
		else{
			$$.cType = RSCEXP_EXP;
			$$.aExp.pExp = new RscExpression( $1, '&', $3 );
		}
	}
  | macro_expression '|' macro_expression
	{
		if( $1.IsNumber() && $3.IsNumber() ){
			$$.cType = RSCEXP_LONG;
			$$.SetLong( $1.GetLong() | $3.GetLong() );
		}
		else{
			$$.cType = RSCEXP_EXP;
			$$.aExp.pExp = new RscExpression( $1, '|', $3 );
		}
	}
  | '(' macro_expression ')'
	{
		$$ = $2;
	}
  | macro_expression LEFTSHIFT macro_expression
	{
		if( $1.IsNumber() && $3.IsNumber() ){
			$$.cType = RSCEXP_LONG;
			$$.SetLong( $1.GetLong() << $3.GetLong() );
		}
		else{
			$$.cType = RSCEXP_EXP;
			$$.aExp.pExp = new RscExpression( $1, 'l', $3 );
		}
	}
  | macro_expression RIGHTSHIFT macro_expression
	{
		if( $1.IsNumber() && $3.IsNumber() ){
			$$.cType = RSCEXP_LONG;
			$$.SetLong( $1.GetLong() >> $3.GetLong() );
		}
		else{
			$$.cType = RSCEXP_EXP;
			$$.aExp.pExp = new RscExpression( $1, 'r', $3 );
		}
	}
  ;

id_expression
  : id_expression line_number
  | macro_expression
	{  // pExpession auswerten und loeschen
		if( RSCEXP_EXP == $1.cType ){
			sal_Int32	lValue;

			if( !$1.Evaluate( &lValue ) )
				pTC->pEH->Error( ERR_ZERODIVISION, NULL, RscId() );
			delete $1.aExp.pExp;
			$$.cType = RSCEXP_LONG;
			$$.SetLong( lValue );
		}
		else
			$$ = $1;
	}
  ;

DUMMY_NUMBER
  : NUMBER
	{
	}
  |
	{
	}
  ;

line_number
  : '#' LINE NUMBER STRING
	{
		RscFile * pFName;

		pFI->SetLineNo( $3 );
		pFI->SetFileIndex( pTC->aFileTab.NewCodeFile( ByteString( $4 ) ) );
		pFName = pTC->aFileTab.Get( pFI->GetFileIndex() );
		pFName->bLoaded = sal_True;
		pFName->bScanned = sal_True;
	}
  | '#' NUMBER STRING DUMMY_NUMBER
	{
		RscFile * pFName;

		pFI->SetLineNo( $2 );
		pFI->SetFileIndex( pTC->aFileTab.NewCodeFile( ByteString( $3 ) ) );
		pFName = pTC->aFileTab.Get( pFI->GetFileIndex() );
		pFName->bLoaded = sal_True;
		pFName->bScanned = sal_True;
	}
  | '#' NUMBER
	{
		pFI->SetLineNo( $2 );
	}
  ;




