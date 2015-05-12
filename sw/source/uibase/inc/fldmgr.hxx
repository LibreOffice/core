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
#ifndef INCLUDED_SW_SOURCE_UIBASE_INC_FLDMGR_HXX
#define INCLUDED_SW_SOURCE_UIBASE_INC_FLDMGR_HXX

#include "swdllapi.h"
#include "swtypes.hxx"
#include <com/sun/star/uno/Reference.h>
#include <com/sun/star/uno/Any.h>
#include <vector>
#include <vcl/vclptr.hxx>

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
class SwPaM;
class SbModule;
class SvxMacroItem;
class SvNumberFormatter;
namespace vcl { class Window; }

// the groups of fields
enum SwFieldGroups
{
    GRP_DOC,
    GRP_FKT,
    GRP_REF,
    GRP_REG,
    GRP_DB,
    GRP_VAR
};

struct SwFieldGroupRgn
{
    sal_uInt16 nStart;
    sal_uInt16 nEnd;
};

// the field manager handles the insertation of fields
// with command strings
struct SwInsertField_Data
{
    sal_uInt16 m_nTypeId;
    sal_uInt16 m_nSubType;
    const OUString m_sPar1;
    const OUString m_sPar2;
    sal_uLong m_nFormatId;
    SwWrtShell* m_pSh;
    sal_Unicode m_cSeparator;
    bool m_bIsAutomaticLanguage;
    ::com::sun::star::uno::Any m_aDBDataSource;
    ::com::sun::star::uno::Any m_aDBConnection;
    ::com::sun::star::uno::Any m_aDBColumn;
    VclPtr<vcl::Window> m_pParent; // parent dialog used for SwWrtShell::StartInputFieldDlg()

    SwInsertField_Data(sal_uInt16 nType, sal_uInt16 nSub, const OUString& rPar1, const OUString& rPar2,
                    sal_uLong nFormatId, SwWrtShell* pShell = NULL, sal_Unicode cSep = ' ', bool bIsAutoLanguage = true) :
        m_nTypeId(nType),
        m_nSubType(nSub),
        m_sPar1(rPar1),
        m_sPar2(rPar2),
        m_nFormatId(nFormatId),
        m_pSh(pShell),
        m_cSeparator(cSep),
        m_bIsAutomaticLanguage(bIsAutoLanguage),
        m_pParent(0) {}

    SwInsertField_Data() :
        m_pSh(0),
        m_cSeparator(' '),
        m_bIsAutomaticLanguage(true){}
};

class SW_DLLPUBLIC SwFieldMgr
{
private:
    SwField*            pCurField;
    SbModule*           pModule;
    const SvxMacroItem* pMacroItem;
    SwWrtShell*         pWrtShell; // can be ZERO too!
    OUString          aCurPar1;
    OUString          aCurPar2;
    OUString          sCurFrame;

    OUString          sMacroPath;
    OUString          sMacroName;

    sal_uLong           nCurFormat;
    bool            bEvalExp;

    SAL_DLLPRIVATE sal_uInt16            GetCurrLanguage() const;

    com::sun::star::uno::Reference<com::sun::star::container::XNameAccess> xDBContext;
    com::sun::star::uno::Reference<com::sun::star::text::XNumberingTypeInfo> xNumberingInfo;
    SAL_DLLPRIVATE com::sun::star::uno::Reference<com::sun::star::text::XNumberingTypeInfo> GetNumberingInfo()const;

public:
    SwFieldMgr(SwWrtShell* pSh = 0);
    ~SwFieldMgr();

    void                SetWrtShell( SwWrtShell* pShell )
                        {   pWrtShell = pShell;     }

     // insert field using TypeID (TYP_ ...)
    bool InsertField( const SwInsertField_Data& rData );

    // change the current field directly
    void            UpdateCurField(sal_uLong nFormat,
                                 const OUString& rPar1,
                                 const OUString& rPar2,
                                 SwField * _pField = 0); // #111840#

    OUString        GetCurFieldPar1() const { return aCurPar1; }
    OUString        GetCurFieldPar2() const { return aCurPar2; }
    inline sal_uLong   GetCurFieldFormat() const;

    // determine a field
    SwField*        GetCurField();

    void            InsertFieldType(SwFieldType& rType);

    bool            ChooseMacro(const OUString &rSelMacro = OUString());
    void            SetMacroPath(const OUString& rPath);
    inline OUString GetMacroPath() const         { return sMacroPath; }
    inline OUString GetMacroName() const         { return sMacroName; }
    inline void     SetMacroModule(SbModule* pMod)    { pModule = pMod; }

    // previous and next of the same type
    bool GoNextPrev( bool bNext = true, SwFieldType* pTyp = 0 );
    bool GoNext( SwFieldType* pTyp = 0 )    { return GoNextPrev( true, pTyp ); }
    bool GoPrev( SwFieldType* pTyp = 0 )    { return GoNextPrev( false, pTyp ); }

    // query values from database fields (BASIC )
//  String          GetDataBaseFieldValue(const String &rDBName, const String &rFieldName, SwWrtShell* pSh);
    bool            IsDBNumeric(const OUString& rDBName, const OUString& rTableQryName,
                                    bool bIsTable, const OUString& rFieldName);

    // organise RefMark with names
    bool            CanInsertRefMark( const OUString& rStr );

    // access to field types via ResId
    sal_uInt16          GetFieldTypeCount(sal_uInt16 nResId = USHRT_MAX) const;
    SwFieldType*    GetFieldType(sal_uInt16 nResId, sal_uInt16 nId = 0) const;
    SwFieldType*    GetFieldType(sal_uInt16 nResId, const OUString& rName) const;

    void            RemoveFieldType(sal_uInt16 nResId, const OUString& rName);

    // access via TypeId from the dialog
    // Ids for a range of fields
    static const SwFieldGroupRgn& GetGroupRange(bool bHtmlMode, sal_uInt16 nGrpId);
    static sal_uInt16           GetGroup(bool bHtmlMode, sal_uInt16 nTypeId, sal_uInt16 nSubType = 0);

    // the current field's TypeId
    sal_uInt16          GetCurTypeId() const;

    // TypeId for a concrete position in the list
    static sal_uInt16   GetTypeId(sal_uInt16 nPos);
    // name of the type in the list of fields
    static OUString     GetTypeStr(sal_uInt16 nPos);

    // Pos in the list of fields
    static sal_uInt16   GetPos(sal_uInt16 nTypeId);

    // subtypes to a type
    bool            GetSubTypes(sal_uInt16 nId, std::vector<OUString>& rToFill);

    // format to a type
    sal_uInt16          GetFormatCount(sal_uInt16 nTypeId, bool bIsText, bool bHtmlMode = false) const;
    OUString            GetFormatStr(sal_uInt16 nTypeId, sal_uLong nFormatId) const;
    sal_uInt16          GetFormatId(sal_uInt16 nTypeId, sal_uLong nFormatId) const;
    sal_uLong           GetDefaultFormat(sal_uInt16 nTypeId, bool bIsText, SvNumberFormatter* pFormatter, double* pVal = 0L);

    // turn off evaluation of expression fields for insertation
    // of many expressino fields (see labels)

    inline void     SetEvalExpFields(bool bEval);
    void            EvalExpFields(SwWrtShell* pSh = NULL);
};

inline void SwFieldMgr::SetEvalExpFields(bool bEval)
    { bEvalExp = bEval; }

inline sal_uLong SwFieldMgr::GetCurFieldFormat() const
    { return nCurFormat; }

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
