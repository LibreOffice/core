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

#ifndef _SVDOSMEV_HXX
#define _SVDOSMEV_HXX

#include <tools/bigint.hxx>

#include <vcl/field.hxx>
class Date;
class Time;
class DateTime;
namespace binfilter {

////////////////////////////////////////////////////////////////////////////////////////////////////

class SdrObject;
class SdrObjSmartData;
class ImpSdrExprTokenList;
class ImpSdrExprToken;
class SdrObjList;
class SdrSmartGeometricList;
class SdrSmartGeometric;
class SdrSmartGeometricLine;
class SdrSmartDraftLineList;
class SdrSmartDraftLine;
class SdrSmartGluePointList;
class SdrSmartGluePoint;
class SdrSmartDragPointList;
class SdrSmartDragPoint;

// D:\PU\EvalExpr.Pas

enum SdrExprErrKind {
    SDREXPRERR_NONE,            // kein Fehler
    SDREXPRERR_DIVZERO,         // Division durch 0
    SDREXPRERR_CIRCREF,         // Zirkulare Referenz
    SDREXPRERR_VALUEEXPECTED,   // Wert erwartet
    SDREXPRERR_VAREXPECTED,     // Feld oder Speichervariable erwartet
    SDREXPRERR_VALUETOOBIG,     // Zahlenwert zu gross
    SDREXPRERR_UNEXPECTEDCHAR,  // unerwartetes Zeichen
    SDREXPRERR_UNEXPECTEDEOLN,  // unerwartetes Zeilenende
    SDREXPRERR_UNKNOWNIDENT,    // Unbekannte Funktion oder Variable
    SDREXPRERR_BRKOPEXPECTED,   // ( erwartet    Die 6 Klammern
    SDREXPRERR_BRKCLEXPECTED,   // ) erwartet    muessen genau
    SDREXPRERR_SQBRKOPEXPECTED, // [ erwartet    in dieser
    SDREXPRERR_SQBRKCLEXPECTED, // ] erwartet    Reihenfolge und
    SDREXPRERR_SPBRKOPEXPECTED, // { erwartet    Zusammenhaengend
    SDREXPRERR_SPBRKCLEXPECTED, // } erwartet    stehen bleiben!
    SDREXPRERR_PARAMEXPECTED,   // ; und Parameter erwartet
    SDREXPRERR_ELSEEXPECTED,    // : erwartet
    SDREXPRERR_MEMBEREXPECTED,  // . oder Member erwartet
    SDREXPRERR_FALSEMEMBER,     // Falscher Klassen-Member
    SDREXPRERR_BASEMUSTBEINT,   // Nur ganzzahlige Basen erlaubt
    SDREXPRERR_BASEMUSTBEPOS,   // Nur positive Basen erlaubt
    SDREXPRERR_DUPLICATEBASE,   // Nur eine Basis je Wert erlaubt
    SDREXPRERR_BASECLEXPECTED,  // ] erwartet wegen Zahlenbasis
    SDREXPRERR_INVALIDFUNCARG,  // Ungültiges Funktionsargument
    SDREXPRERR_INVALIDDIGIT,    // Unerlaubte Ziffer
    SDREXPRERR_COMPLEXVALUE,    // x^y: negative Basis, gebrochener Exponent oder Wurzel(-val)
    SDREXPRERR_DOUBLEDECPOINT   // Zu viele Dezimalpunkte
};

enum SdrExprValueKind {
    SDREXPRVALUE_SIMPLE,     // Normale Zahl
    SDREXPRVALUE_COMPLEX,    // Komplexe Zahl
    SDREXPRVALUE_UNITS,      // eine Entfernung
    SDREXPRVALUE_ANGLE,      // ein Winkel in 1/100deg
    // ab hier nur temporaere Typen fuer Evaluate
    SDREXPRVALUE_POINT,      // ein Punkt (Units)
    SDREXPRVALUE_DATE,       // ein Datum
    SDREXPRVALUE_TIME,       // eine Uhrzeit
    SDREXPRVALUE_DATETIME    // ein Datum (Y) mit Uhrzeit (X)
};

// todo: FieldPointer fuer Zuweisungen
class SdrExprValue {
friend class ImpSdrSmartObjEvaluator;
//public:
protected:
    BigInt    nX,nY; //,nX2,nY2;
    double    fX,fY; //,fX2,fY2;
    ULONG     nMSecsUsed;
    FASTBOOL  bFloat; // Switch zwischen BigInt und Float (fuer alle 4 Werte)
    FASTBOOL  bUIMap;
    FASTBOOL  bDirty;
    FASTBOOL  bHasValue;
    FASTBOOL  bLocked;
    FASTBOOL  bIsField;     // RTTI-Sparversion
    FASTBOOL  bIsFieldPtr;  // Ist Zeiger auf ein Feld oder 2 Felder (X/Y)
    SdrExprValueKind eKind;
    FieldUnit eUIMap;
    MapUnit   eCoreMap;
    USHORT    nUnitPower;
private:
    virtual void Undirty(const SdrObject* pObj);
    void CTor();
    FASTBOOL HasUnit() const                    { return nUnitPower!=0; }
    FASTBOOL IsSameUnit(const SdrExprValue& rVal) const;
public:
    SdrExprValue()                              { CTor(); }
    SdrExprValue(double fVal)                   { CTor(); SetValue(fVal);         }
    SdrExprValue(long nVal)                     { CTor(); SetValue(nVal);         }
    SdrExprValue(const BigInt& nVal)            { CTor(); SetValue(nVal);         }
    SdrExprValue(double fVal, FieldUnit eU)     { CTor(); SetUnits(fVal,eU);      }
    SdrExprValue(long nVal, FieldUnit eU)       { CTor(); SetUnits(nVal,eU);      }
    SdrExprValue(const BigInt& n, FieldUnit eU) { CTor(); SetUnits(n,eU);         }
    SdrExprValue(double fVal, MapUnit eU)       { CTor(); SetUnits(fVal,eU);      }
    SdrExprValue(long nVal, MapUnit eU)         { CTor(); SetUnits(nVal,eU);      }
    SdrExprValue(const BigInt& n, MapUnit eU)   { CTor(); SetUnits(n,eU);         }
    SdrExprValue(const Point& P, MapUnit eU)    { CTor(); SetUnits(P,eU);         }
    SdrExprValue(FASTBOOL bVal)                 { CTor(); SetBool(bVal);          }
    SdrExprValue(const Time& rTime)             { CTor(); SetTime(rTime);         }
    SdrExprValue(const Date& rDate)             { CTor(); SetDate(rDate);         }
    SdrExprValue(const DateTime& rDateTime)     { CTor(); SetDateTime(rDateTime); }
    virtual ~SdrExprValue();

    const SdrExprValue& ForceUndirty(const SdrObject* pObj) const { if (bDirty) ((SdrExprValue*)this)->Undirty(pObj); return *this; }
    void             SetDirty()                             { bDirty=TRUE; }
    FASTBOOL         IsDirty() const                        { return bDirty; }
    FASTBOOL         IsField() const                        { return bIsField; }
    FASTBOOL         IsFieldPtr() const                     { return bIsFieldPtr; }

    FASTBOOL         IsLocked(const SdrObject* pObj) const  { ForceUndirty(pObj); return bLocked; }
    ULONG            GetMSecsUsed(const SdrObject* pO) const{ ForceUndirty(pO  ); return nMSecsUsed; }
    FASTBOOL         HasValue(const SdrObject* pObj) const  { ForceUndirty(pObj); return bHasValue; }
    SdrExprValueKind GetExprKind(const SdrObject* pObj) const{ForceUndirty(pObj); return eKind; }
    FASTBOOL         IsLong(const SdrObject* pObj) const    { ForceUndirty(pObj); return !bFloat /*&& !nX.IsLong()*/; }
    FASTBOOL         IsBigInt(const SdrObject* pObj) const  { ForceUndirty(pObj); return !bFloat /*&& nX.IsLong()*/; }
    FASTBOOL         IsFloat(const SdrObject* pObj) const   { ForceUndirty(pObj); return bFloat; }
    double           GetFloat(const SdrObject* pObj) const  { ForceUndirty(pObj); return fX; }
    const BigInt&    GetBigInt(const SdrObject* pObj) const { ForceUndirty(pObj); return nX; }
    long             GetLong(const SdrObject* pObj) const   { ForceUndirty(pObj); return long(nX); }
    FASTBOOL         GetBool(const SdrObject* pObj) const   { ForceUndirty(pObj); return !nX.IsZero(); }
    MapUnit          GetCoreMap(const SdrObject* pObj) const{ ForceUndirty(pObj); return eCoreMap; }
    FieldUnit        GetUIMap(const SdrObject* pObj) const  { ForceUndirty(pObj); return eUIMap; }
    FASTBOOL         QuickIsLocked() const                  { return bLocked; }
    ULONG            QuickGetMSecsUsed() const              { return nMSecsUsed; }
    FASTBOOL         QuickHasValue() const                  { return bHasValue; }
    SdrExprValueKind QuickGetExprKind() const               { return eKind; }
    FASTBOOL         QuickIsLong() const                    { return !bFloat /*&& !nX.IsLong()*/; }
    FASTBOOL         QuickIsBigInt() const                  { return !bFloat /*&& nX.IsLong()*/; }
    FASTBOOL         QuickIsFloat() const                   { return bFloat; }
    double           QuickGetFloat() const                  { return fX; }
    const BigInt&    QuickGetBigInt() const                 { return nX; }
    long             QuickGetLong() const                   { return long(nX); }
    FASTBOOL         QuickGetBool() const                   { return !nX.IsZero(); }
    MapUnit          QuickGetCoreMap() const                { return eCoreMap; }
    FieldUnit        QuickGetUIMap() const                  { return eUIMap; }

    long             QuickGetUnits(const BigInt& rVal, MapUnit eU) const;
    long             QuickGetUnits(double nVal, MapUnit eU) const;
    double           QuickGetFloatNoUnits(const BigInt& rVal) const;
    double           QuickGetFloatNoUnits(double nVal) const;

    long             GetUnits(MapUnit eU, const SdrObject* pObj) const  { ForceUndirty(pObj); return QuickGetUnits(eU); }
    long             QuickGetUnits(MapUnit eU) const;
    long             QuickGetLongNoUnits() const { return (long)QuickGetFloatNoUnits(); }
    double           QuickGetFloatNoUnits() const;

    Time             QuickGetTime() const;
    Date             QuickGetDate() const;
    DateTime         QuickGetDateTime() const;

    FASTBOOL         QuickIsDate() const       { return eKind==SDREXPRVALUE_DATE; }
    FASTBOOL         QuickIsTime() const       { return eKind==SDREXPRVALUE_TIME; }
    FASTBOOL         QuickIsDateTime() const   { return eKind==SDREXPRVALUE_DATETIME; }
    FASTBOOL         QuickIsDateOrTime() const { return eKind==SDREXPRVALUE_DATE || eKind==SDREXPRVALUE_TIME || eKind==SDREXPRVALUE_DATETIME; }
    BigInt           QuickGetDateTimeMSecs() const;

    void SetExprKind(SdrExprValueKind eNew)  { eKind=eNew; }

    void Negate();

    void SetValue(double fVal);
    void SetValue(long nVal);
    void SetValue(const BigInt& nVal);

    void SetUnits(double fVal, FieldUnit eU);
    void SetUnits(long nVal, FieldUnit eU);
    void SetUnits(const BigInt& n, FieldUnit eU);

    void SetUnits(double fVal, MapUnit eU);
    void SetUnits(long nVal, MapUnit eU);
    void SetUnits(const BigInt& n, MapUnit eU);
    void SetUnits(const Point& P, MapUnit eU);

    void SetBool(FASTBOOL bVal);
    void SetTime(const Time& rTime);
    void SetDate(const Date& rDate);
    void SetDateTime(const DateTime& rDateTime);

/*  const SdrObject*             GetObject       () const { return (const SdrObject*            )pPointer; }
    const SdrObjList*            GetObjList      () const { return (const SdrObjList*           )pPointer; }
    const SdrSmartGeometricList* GetGeometricList() const { return (const SdrSmartGeometricList*)pPointer; }
    const SdrSmartGeometric*     GetGeometricArea() const { return (const SdrSmartGeometric*    )pPointer; }
    const SdrSmartGeometricLine* GetGeometricLine() const { return (const SdrSmartGeometricLine*)pPointer; }
    const SdrSmartDraftLineList* GetDraftArea    () const { return (const SdrSmartDraftLineList*)pPointer; }
    const SdrSmartDraftLine*     GetDraftLine    () const { return (const SdrSmartDraftLine*    )pPointer; }
    const SdrSmartGluePointList* GetGlueArea     () const { return (const SdrSmartGluePointList*)pPointer; }
    const SdrSmartGluePoint*     GetGlueLine     () const { return (const SdrSmartGluePoint*    )pPointer; }
    const SdrSmartDragPointList* GetDragArea     () const { return (const SdrSmartDragPointList*)pPointer; }
    const SdrSmartDragPoint*     GetDragLine     () const { return (const SdrSmartDragPoint*    )pPointer; }
    void SetObject       (const SdrObject*             pObj      ) { pPointer=pObj;       eKind=SDREXPRVALUE_OBJECT;        }
    void SetObjList      (const SdrObjList*            pObjList  ) { pPointer=pObjList;   eKind=SDREXPRVALUE_OBJLIST;       }
    void SetGeometricList(const SdrSmartGeometricList* pGeos     ) { pPointer=pGeos;      eKind=SDREXPRVALUE_GEOMETRICS;    }
    void SetGeometricArea(const SdrSmartGeometric*     pGeo      ) { pPointer=pGeo;       eKind=SDREXPRVALUE_GEOMETRIC;     }
    void SetGeometricLine(const SdrSmartGeometricLine* pGeoLine  ) { pPointer=pGeoLine;   eKind=SDREXPRVALUE_GEOLINE;       }
    void SetDraftArea    (const SdrSmartDraftLineList* pDraftArea) { pPointer=pDraftArea; eKind=SDREXPRVALUE_DRAFTAREA;     }
    void SetDraftLine    (const SdrSmartDraftLine*     pDraftLine) { pPointer=pDraftLine; eKind=SDREXPRVALUE_DRAFTAREALINE; }
    void SetGlueArea     (const SdrSmartGluePointList* pGlueArea ) { pPointer=pGlueArea;  eKind=SDREXPRVALUE_GLUEAREA;      }
    void SetGlueLine     (const SdrSmartGluePoint*     pGlueLine ) { pPointer=pGlueLine;  eKind=SDREXPRVALUE_GLUEAREALINE;  }
    void SetDragArea     (const SdrSmartDragPointList* pDragArea ) { pPointer=pDragArea;  eKind=SDREXPRVALUE_DRAGAREA;      }
    void SetDragLine     (const SdrSmartDragPoint*     pDragLine ) { pPointer=pDragLine;  eKind=SDREXPRVALUE_DRAGAREALINE;  }*/

    void operator+=(const SdrExprValue& rVal);
    void operator-=(const SdrExprValue& rVal);
    void operator*=(const SdrExprValue& rVal);
    void operator/=(const SdrExprValue& rVal);
    void operator%=(const SdrExprValue& rVal);
    FASTBOOL operator==(const SdrExprValue& rVal) const;
    FASTBOOL operator!=(const SdrExprValue& rVal) const    { return !operator==(rVal); }
    FASTBOOL operator==(double fVal) const;
    FASTBOOL operator!=(double fVal) const                 { return !operator==(fVal); }
    FASTBOOL operator==(long nVal) const;
    FASTBOOL operator!=(long nVal) const                   { return !operator==(nVal); }
    FASTBOOL operator==(const BigInt& nVal) const;
    FASTBOOL operator!=(const BigInt& nVal) const          { return !operator==(nVal); }
    FASTBOOL operator> (const SdrExprValue& rVal) const;
    FASTBOOL operator>=(const SdrExprValue& rVal) const;
    FASTBOOL operator< (const SdrExprValue& rVal) const    { return !operator>=(rVal); }
    FASTBOOL operator<=(const SdrExprValue& rVal) const    { return !operator>(rVal); }
    FASTBOOL operator> (double fVal) const;
    FASTBOOL operator>=(double fVal) const;
    FASTBOOL operator< (double fVal) const                 { return !operator>=(fVal); }
    FASTBOOL operator<=(double fVal) const                 { return !operator>(fVal); }
    FASTBOOL operator> (long nVal) const;
    FASTBOOL operator>=(long nVal) const;
    FASTBOOL operator< (long nVal) const                   { return !operator>=(nVal); }
    FASTBOOL operator<=(long nVal) const                   { return !operator>(nVal); }
    FASTBOOL operator> (const BigInt& nVal) const;
    FASTBOOL operator>=(const BigInt& nVal) const;
    FASTBOOL operator< (const BigInt& nVal) const          { return !operator>=(nVal); }
    FASTBOOL operator<=(const BigInt& nVal) const          { return !operator>(nVal); }
};

/*class ImpSdrSmartObjEvaluator0 {
    String aStr;
    const SdrObject* pObj;
    const SdrObjSmartData* pSmart;
    SdrExprErrKind eError;
    USHORT nErrPos;
    USHORT nPos;
    char   cAktChar;
    SdrExprValue aResult;
    //FASTBOOL bHasValue;
    //FASTBOOL bLocked;
    FASTBOOL bMetric;
    FASTBOOL bMetricX;
    FASTBOOL bMetricY;
    //double fValue;
    //long   nValue;
    //long   nUnitValue;
    FieldUnit eUIMap;
    MapUnit   eCoreMap;
private:
    void Error(USHORT nEPos, SdrExprErrKind eECode);
    FASTBOOL CharsLeft() const { return nPos<aStr.Len(); }
    void KillSpaces();
    char NextChar();
    SdrExprValue Func();
    SdrExprValue Valu();
    SdrExprValue Wert();
    SdrExprValue Fakt();
    SdrExprValue Term();
    SdrExprValue Expr();
public:
    ImpSdrSmartObjEvaluator0(const SdrObject* pObj_);
    void SetMetric(FASTBOOL bOn)  { bMetric=bOn; bMetricX=FALSE; bMetricY=FALSE; }
    void SetMetricX(FASTBOOL bOn) { bMetric=bOn; bMetricX=TRUE;  bMetricY=FALSE; }
    void SetMetricY(FASTBOOL bOn) { bMetric=bOn; bMetricX=FALSE; bMetricY=TRUE;  }
    FASTBOOL Evaluate(const String& rStr);
//    double   GetFloatValue() const  { return aResult.fValue; }
//    long     GetIntValue() const    { return aResult.nValue; }
//    long     GetUnitValue() const   { return aResult.nUnitValue; }
    FASTBOOL HasValue() const       { return aResult.HasValue(pObj); }
    FASTBOOL IsLocked() const       { return aResult.IsLocked(pObj); }
    FASTBOOL IsError() const        { return eError!=SDREXPRERR_NONE; }
    SdrExprErrKind GetError() const { return eError; }
    USHORT   GetErrorPos() const    { return nErrPos; }
    const SdrExprValue& GetResult() const { return aResult; }
};*/

class ImpSdrSmartObjEvaluator {
friend class ImpZwischenergebnis;
    const ImpSdrExprTokenList* pTokenList;
    const SdrObject* pObj;
    const SdrObjSmartData* pSmart;
    const ImpSdrExprToken* pAktTok;
    const SdrExprValue* pTargetField; // Zur Bestimmung der AktLine, ..., anhand pObj
    SdrExprErrKind eError;
    ULONG          nErrPos;
    ULONG          nAnz;
    ULONG          nPos;
    SdrExprValue   aResult;
    FASTBOOL       bMetric;
    FASTBOOL       bMetricX;
    FASTBOOL       bMetricY;
    FieldUnit      eUIMap;
    MapUnit        eCoreMap;
    // Feldlokation in Geo,Draft,Vars,Glue oder Drag
    FASTBOOL       bFieldLocationSearched;
    FASTBOOL       bFieldLocationFound;
    USHORT         nFieldArea; // eigentlich enum ...
    ULONG          nGeoFieldAreaNum;
    ULONG          nFieldLine;
    ULONG          nFieldColumn;
    ULONG          nMSecsUsed; // Falls nur Date, dann =86400000 (ms/Tag)
    FASTBOOL       bLocked;
private:
    void         SetMSecsUsed(ULONG n);
    void         SearchFieldLocation();
    void         ForceFieldLocation() const { if (!bFieldLocationSearched) ((ImpSdrSmartObjEvaluator*)this)->SearchFieldLocation(); }
    void         Error(ULONG nEPos, SdrExprErrKind eECode);
    FASTBOOL     TokensLeft() const { return nPos<nAnz; }
    const ImpSdrExprToken* NextToken();
    FASTBOOL     BracketOpen();
    FASTBOOL     BracketClose(USHORT eOpen);
    FASTBOOL     NextArg(SdrExprValue*& pArg, FASTBOOL bSkip);
    void         FuncRound(FASTBOOL bSkip, double nRet, SdrExprValue* pRet, SdrExprValue*& pArg);
    void         FuncRandom(FASTBOOL bSkip, FASTBOOL bBracket, double nRet, SdrExprValue* pRet, SdrExprValue*& pArg);
    SdrExprValue* Memb(FASTBOOL bSkip);
    SdrExprValue* Func(FASTBOOL bSkip);
    SdrExprValue* Wert(FASTBOOL bSkip);
    SdrExprValue* Fakt(FASTBOOL bSkip);
    SdrExprValue* Term(FASTBOOL bSkip);
    SdrExprValue* Expr2(FASTBOOL bSkip);
    SdrExprValue* CompTerm(FASTBOOL bSkip);
    SdrExprValue* BoolFakt(FASTBOOL bSkip);
    SdrExprValue* BoolXTerm(FASTBOOL bSkip);
    SdrExprValue* BoolTerm(FASTBOOL bSkip);
    SdrExprValue* Condition(FASTBOOL bSkip);
    SdrExprValue* Expr(FASTBOOL bSkip);
public:
    ImpSdrSmartObjEvaluator(const SdrObject* pObj_);
    void SetMetric(FASTBOOL bOn)  { bMetric=bOn; bMetricX=FALSE; bMetricY=FALSE; }
    void SetMetricX(FASTBOOL bOn) { bMetric=bOn; bMetricX=TRUE;  bMetricY=FALSE; }
    void SetMetricY(FASTBOOL bOn) { bMetric=bOn; bMetricX=FALSE; bMetricY=TRUE;  }
    FASTBOOL Evaluate(const ImpSdrExprTokenList* pTokenList_, const SdrExprValue* pTargetField_);
    FASTBOOL HasValue() const       { return aResult.HasValue(pObj); }
    FASTBOOL IsLocked() const       { return aResult.IsLocked(pObj); }
    FASTBOOL IsError() const        { return eError!=SDREXPRERR_NONE; }
    SdrExprErrKind GetError() const { return eError; }
    static XubString GetErrorStr(SdrExprErrKind eErr);
    XubString GetErrorStr() const    { return GetErrorStr(eError); }
    ULONG    GetErrorPos() const    { return nErrPos; }
    const SdrExprValue& GetResult() const { return aResult; }
    MapUnit GetCoreMap() const      { return eCoreMap; }
    const SdrObject* GetObject() const { return pObj; }
    const SdrObjSmartData* GetSmartData() const { return pSmart; }
    ULONG GetMSecsUsed() const  { return nMSecsUsed; }
};

////////////////////////////////////////////////////////////////////////////////////////////////////

}//end of namespace binfilter
#endif //_SVDOSMEV_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
