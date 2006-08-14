/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: fldmgr.hxx,v $
 *
 *  $Revision: 1.15 $
 *
 *  last change: $Author: hr $ $Date: 2006-08-14 17:41:26 $
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
#ifndef _FLDMGR_HXX
#define _FLDMGR_HXX
#ifndef _SVSTDARR_HXX
#define _SVSTDARR_STRINGSDTOR
#include <svtools/svstdarr.hxx>
#endif

#ifndef _STRING_HXX //autogen
#include <tools/string.hxx>
#endif

#ifndef INCLUDED_SWDLLAPI_H
#include "swdllapi.h"
#endif

#ifndef _SWTYPES_HXX
#include "swtypes.hxx"
#endif

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

class SwWrtShell;
class SwField;
class SwFieldType;
class SbModule;
class SvxMacroItem;
class SvNumberFormatter;
class Window;
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
    Window* pParent; // parent dialog used for SwWrtShell::StartInputFldDlg()

    SwInsertFld_Data(USHORT nType, USHORT nSub, const String& rPar1, const String& rPar2,
                    ULONG nFmtId, SwWrtShell* pShell = NULL, sal_Unicode cSep = ' ', BOOL bIsAutoLanguage = TRUE) :
        nTypeId(nType),
        nSubType(nSub),
        sPar1(rPar1),
        sPar2(rPar2),
        nFormatId(nFmtId),
        pSh(pShell),
        cSeparator(cSep),
        bIsAutomaticLanguage(bIsAutoLanguage),
        pParent(0) {}

    SwInsertFld_Data() :
        pSh(0),
        cSeparator(' '),
        bIsAutomaticLanguage(TRUE){}

};

class SW_DLLPUBLIC SwFldMgr
{
private:
    SwField*            pCurFld;
    SbModule*           pModule;
    const SvxMacroItem* pMacroItem;
    SwWrtShell*         pWrtShell; // darf auch NULL sein!
    String          aCurPar1;
    String          aCurPar2;
    String          sCurFrame;

    String          sMacroPath;
    String          sMacroName;

    ULONG           nCurFmt;
    BOOL            bEvalExp;

    SW_DLLPRIVATE USHORT            GetCurrLanguage() const;

    com::sun::star::uno::Reference<com::sun::star::container::XNameAccess> xDBContext;
    com::sun::star::uno::Reference<com::sun::star::text::XNumberingTypeInfo> xNumberingInfo;
    SW_DLLPRIVATE com::sun::star::uno::Reference<com::sun::star::text::XNumberingTypeInfo> GetNumberingInfo()const;

public:
    SwFldMgr(SwWrtShell* pSh = 0);
    ~SwFldMgr();

    void                SetWrtShell( SwWrtShell* pShell )
                        {   pWrtShell = pShell;     }

    // Feld einfuegen ueber TypeId (TYP_ ...)
    BOOL            InsertFld(  const SwInsertFld_Data& rData );

    // Direkt das aktuelle Feld aendern
    void            UpdateCurFld(ULONG nFormat,
                                 const String& rPar1,
                                 const String& rPar2,
                                 SwField * _pField = 0); // #111840#

    inline const String& GetCurFldPar1() const;
    inline const String& GetCurFldPar2() const;
    inline const ULONG   GetCurFldFmt() const;

    // Ein Feld ermitteln
    SwField*        GetCurFld();

    void            InsertFldType(SwFieldType& rType);

    BOOL            ChooseMacro(const String &rSelMacro = aEmptyStr);
    void            SetMacroPath(const String& rPath);
    inline const String& GetMacroPath() const         { return (sMacroPath); }
    inline const String& GetMacroName() const         { return (sMacroName); }
    inline void     SetMacroModule(SbModule* pMod)    { pModule = pMod; }

    // Vorheriger Naechster gleichen Typ
    BOOL GoNextPrev( BOOL bNext = TRUE, SwFieldType* pTyp = 0 );
    BOOL GoNext( SwFieldType* pTyp = 0 )    { return GoNextPrev( TRUE, pTyp ); }
    BOOL GoPrev( SwFieldType* pTyp = 0 )    { return GoNextPrev( FALSE, pTyp ); }

    // Erfragen von Werten aus Datenbankfeldern (BASIC )
//  String          GetDataBaseFieldValue(const String &rDBName, const String &rFieldName, SwWrtShell* pSh);
    BOOL            IsDBNumeric(const String& rDBName, const String& rTblQryName,
                                        BOOL bIsTable, const String& rFldName);

    // RefMark mit Namen organisieren
    BOOL            CanInsertRefMark( const String& rStr );


    // Zugriff ueber ResId auf Feldtypen
    USHORT          GetFldTypeCount(USHORT nResId = USHRT_MAX) const;
    SwFieldType*    GetFldType(USHORT nResId, USHORT nId = 0) const;
    SwFieldType*    GetFldType(USHORT nResId, const String& rName) const;

    void            RemoveFldType(USHORT nResId, const String& rName);

    // Zugriff ueber TypeId aus dem Dialog
    // Ids fuer einen Bereich von Feldern
    const SwFldGroupRgn& GetGroupRange(BOOL bHtmlMode, USHORT nGrpId) const;
    USHORT          GetGroup(BOOL bHtmlMode, USHORT nTypeId, USHORT nSubType = 0) const;

    // TypeId des aktuellen Feldes
    USHORT          GetCurTypeId() const;

    // TypeId fuer einen konkrete Pos in der Liste
    static USHORT   GetTypeId(USHORT nPos);
    // Name des Typen in der Liste der Felder
    static const String&  GetTypeStr(USHORT nPos);

    // Pos in der Liste der Felder
    static USHORT   GetPos(USHORT nTypeId);

    // Untertypen zu einem Typ
    BOOL            GetSubTypes(USHORT nId, SvStringsDtor& rToFill);

    // Formate zu einem Typ
    USHORT          GetFormatCount(USHORT nTypeId, BOOL bIsText, BOOL bHtmlMode = FALSE) const;
    String          GetFormatStr(USHORT nTypeId, ULONG nFormatId) const;
    USHORT          GetFormatId(USHORT nTypeId, ULONG nFormatId) const;
    ULONG           GetDefaultFormat(USHORT nTypeId, BOOL bIsText, SvNumberFormatter* pFormatter, double* pVal = 0L);

    // Evaluierung der ExpressionFelder ausschalten fuer das Einfuegen
    // vieler Expressionfelder (siehe Etiketten)
    //
    inline void     SetEvalExpFlds(BOOL bEval);
    void            EvalExpFlds(SwWrtShell* pSh = NULL);
};

inline void SwFldMgr::SetEvalExpFlds(BOOL bEval)
    { bEvalExp = bEval; }

inline const String& SwFldMgr::GetCurFldPar1() const
    { return aCurPar1; }

inline const String& SwFldMgr::GetCurFldPar2() const
    { return aCurPar2; }

inline const ULONG   SwFldMgr::GetCurFldFmt() const
    { return nCurFmt; }


#endif

