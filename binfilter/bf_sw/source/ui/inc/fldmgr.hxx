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
#ifndef _FLDMGR_HXX
#define _FLDMGR_HXX

#ifndef _SVSTDARR_HXX
#define _SVSTDARR_STRINGSDTOR
#include <bf_svtools/svstdarr.hxx>
#endif

#ifndef _STRING_HXX //autogen
#include <tools/string.hxx>
#endif

#include "swtypes.hxx"

#ifndef _COM_SUN_STAR_UNO_REFERENCE_H_
#include <com/sun/star/uno/Reference.h>
#endif
#ifndef _COM_SUN_STAR_UNO_ANY_H_
#include <com/sun/star/uno/Any.h>
#endif
namespace com{namespace sun{namespace star{
    namespace container{
        class XNameAccess;
    }
    namespace text{
        class XNumberingTypeInfo;
    }
}}}
class ListBox; 

namespace binfilter {

class SvxMacroItem; 
class SvNumberFormatter; 
class SwWrtShell;
class SwField;
class SwFieldType;
class SwFmtRefMark;
class SbModule;

/*--------------------------------------------------------------------
    Beschreibung: Shell setzen auf der der SwFldMgr arbeitet
 --------------------------------------------------------------------*/

void SetFldMgrShell(SwWrtShell* pSh);

/*--------------------------------------------------------------------
    Beschreibung: Die Gruppen von Feldern
 --------------------------------------------------------------------*/

enum SwFldGroups
{
    GRP_DOC,
    GRP_FKT,
    GRP_REF,
    GRP_REG,
    GRP_DB,
    GRP_VAR
};

struct SwFldGroupRgn
{
    USHORT nStart;
    USHORT nEnd;
};

/*--------------------------------------------------------------------
    Beschreibung:  Der FeldManager handelt das Einfuegen von Felder
                    ueber Command-Strings
 --------------------------------------------------------------------*/
struct SwInsertFld_Data
{
    USHORT nTypeId;
    USHORT nSubType;
    const String sPar1;
    const String sPar2;
    ULONG nFormatId;
    SwWrtShell* pSh;
    sal_Unicode cSeparator;
    BOOL bIsAutomaticLanguage;
    ::com::sun::star::uno::Any aDBDataSource;
    ::com::sun::star::uno::Any aDBConnection;
    ::com::sun::star::uno::Any aDBColumn;

    SwInsertFld_Data(USHORT nType, USHORT nSub, const String& rPar1, const String& rPar2,
                    ULONG nFmtId, SwWrtShell* pShell = NULL, sal_Unicode cSep = ' ', BOOL bIsAutoLanguage = TRUE) :
        nTypeId(nType),
        nSubType(nSub),
        sPar1(rPar1),
        sPar2(rPar2),
        nFormatId(nFmtId),
        pSh(pShell),
        cSeparator(cSep),
        bIsAutomaticLanguage(bIsAutoLanguage) {}

    SwInsertFld_Data() :
        pSh(0),
        cSeparator(' '),
        bIsAutomaticLanguage(TRUE){}

};
class SwFldMgr
{
private:
    SvStringsDtor	aSubLst;

    SwField*			pCurFld;
    SbModule*			pModule;
    const SvxMacroItem*	pMacroItem;
    SwWrtShell* 		pWrtShell; // darf auch NULL sein!
    String			aCurPar1;
    String			aCurPar2;
    String 			sCurFrame;

    String          sMacroPath;

    ULONG			nCurFmt;
    BOOL			bEvalExp;

    USHORT			GetCurrLanguage() const;

    ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess> xDBContext;
    ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess> GetDBContext();
    ::com::sun::star::uno::Reference< ::com::sun::star::text::XNumberingTypeInfo> xNumberingInfo;
    ::com::sun::star::uno::Reference< ::com::sun::star::text::XNumberingTypeInfo> GetNumberingInfo()const;
public:
    SwFldMgr(SwWrtShell* pSh = 0);
    ~SwFldMgr();

    // Feld einfuegen ueber TypeId (TYP_ ...)
    BOOL            InsertFld(  const SwInsertFld_Data& rData );

    BOOL 			InsertURL(  const String& rName,
                                const String& rVal,
                                const String& rFrame,
                                const SvxMacroItem* pItem = 0);

    // Direkt das aktuelle Feld aendern
    void 			UpdateCurFld(ULONG nFormat,
                                 const String& rPar1,
                                 const String& rPar2);

    inline const String& GetCurFldPar1() const;
    inline const String& GetCurFldPar2() const;
    inline ULONG   GetCurFldFmt() const;

    // Ein Feld ermitteln
    SwField*		GetCurFld();

    // Zugriff	auf Feldtypen
    void			RemoveDBTypes();
    void			InsertFldType(SwFieldType& rType);

    BOOL			ChooseMacro(const String &rSelMacro = aEmptyStr);
    inline void		SetMacroPath(const String& rPath) { sMacroPath = rPath; }
    inline const String& GetMacroPath() const		  { return (sMacroPath); }
    inline void		SetMacroModule(SbModule* pMod)	  { pModule = pMod; }

    // Vorheriger Naechster gleichen Typ
    BOOL GoNextPrev( BOOL bNext = TRUE, SwFieldType* pTyp = 0 );
    BOOL GoNext( SwFieldType* pTyp = 0 )	{ return GoNextPrev( TRUE, pTyp ); }
    BOOL GoPrev( SwFieldType* pTyp = 0 )	{ return GoNextPrev( FALSE, pTyp ); }

    // Setzen von Werten aus Benutzerfeldern (BASIC)
    //
    BOOL			SetFieldValue(const String &rFieldName,
                                  const String &rValue);

    // Erfragen von Werten aus Datenbankfeldern (BASIC )
//	String			GetDataBaseFieldValue(const String &rDBName, const String &rFieldName, SwWrtShell* pSh);
    BOOL			IsDBNumeric(const String& rDBName, const String& rTblQryName,
                                        BOOL bIsTable, const String& rFldName);

    // RefMark mit Namen organisieren
    BOOL 			CanInsertRefMark( const String& rStr );


    // Zugriff ueber ResId auf Feldtypen
    USHORT 			GetFldTypeCount(USHORT nResId = USHRT_MAX) const;
    SwFieldType* 	GetFldType(USHORT nResId, USHORT nId = 0) const;
    SwFieldType* 	GetFldType(USHORT nResId, const String& rName) const;

    void 			RemoveFldType(USHORT nResId, USHORT nId = 0);
    void 			RemoveFldType(USHORT nResId, const String& rName);

    // Zugriff ueber TypeId aus dem Dialog
    // Ids fuer einen Bereich von Feldern
    const SwFldGroupRgn& GetGroupRange(BOOL bHtmlMode, USHORT nGrpId) const;
    USHORT			GetGroup(BOOL bHtmlMode, USHORT nTypeId, USHORT nSubType = 0) const;

    // TypeId des aktuellen Feldes
    USHORT			GetCurTypeId() const;
    // TypeId fuer einen konkrete Pos in der Liste
    USHORT 			GetTypeId(USHORT nPos) const;
    // Name des Typen in der Liste der Felder
    const String&	GetTypeStr(USHORT nPos) const;

    // Pos in der Liste der Felder
    USHORT			GetPos(USHORT nTypeId) const;

    // Untertypen zu einem Typ
    SvStringsDtor& 	GetSubTypes(USHORT nId);

    BOOL			SetUserSubType(const String& rName, USHORT nSubType);

    // Formate zu einem Typ
    USHORT 			GetFormatCount(USHORT nTypeId, BOOL bIsText, BOOL bHtmlMode = FALSE) const;
    String 			GetFormatStr(USHORT nTypeId, ULONG nFormatId) const;
    USHORT			GetFormatId(USHORT nTypeId, ULONG nFormatId) const;
    ULONG			GetDefaultFormat(USHORT nTypeId, BOOL bIsText, SvNumberFormatter* pFormatter, double* pVal = 0L);

    // Evaluierung der ExpressionFelder ausschalten fuer das Einfuegen
    // vieler Expressionfelder (siehe Etiketten)
    //
    inline void		SetEvalExpFlds(BOOL bEval);
    void			EvalExpFlds(SwWrtShell* pSh = NULL);

    // public, damit sie im fldini initialisiert werden koennen.
    static String 	*pDate, *pTime;

};

inline void SwFldMgr::SetEvalExpFlds(BOOL bEval)
    { bEvalExp = bEval; }

inline const String& SwFldMgr::GetCurFldPar1() const
    { return aCurPar1; }

inline const String& SwFldMgr::GetCurFldPar2() const
    { return aCurPar2; }

inline ULONG   SwFldMgr::GetCurFldFmt() const
    { return nCurFmt; }


} //namespace binfilter
#endif

