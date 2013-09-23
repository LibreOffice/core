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
#ifndef _FMTCOL_HXX
#define _FMTCOL_HXX

#include "swdllapi.h"
#include <frmatr.hxx>
#include <swtypes.hxx> // For MAXLEVEL.
#include <boost/ptr_container/ptr_vector.hpp>
#include <tools/string.hxx>

class SwDoc; // For friend.

class SwFmtColl : public SwFmt
{
protected:
    SwFmtColl( SwAttrPool& rPool, const sal_Char* pFmtName,
                const sal_uInt16* pWhichRanges, SwFmtColl* pDerFrom,
                sal_uInt16 nFmtWhich )
          : SwFmt( rPool, pFmtName, pWhichRanges, pDerFrom, nFmtWhich )
    { SetAuto( sal_False ); }

    SwFmtColl( SwAttrPool& rPool, const OUString &rFmtName,
                const sal_uInt16* pWhichRanges, SwFmtColl* pDerFrom,
                sal_uInt16 nFmtWhich )
          : SwFmt( rPool, rFmtName, pWhichRanges, pDerFrom, nFmtWhich )
    { SetAuto( sal_False ); }

private:

    /// For now don't copy and don't assign.
    SwFmtColl(const SwFmtColl & );
    const SwFmtColl &operator=(const SwFmtColl &);
};

/// Represents the style of a paragraph.
class SW_DLLPUBLIC SwTxtFmtColl: public SwFmtColl
{
    friend class SwDoc;

    SwTxtFmtColl(const SwTxtFmtColl & rRef);

    bool mbStayAssignedToListLevelOfOutlineStyle;

protected:

    bool mbAssignedToOutlineStyle;

    SwTxtFmtColl *pNextTxtFmtColl;

    SwTxtFmtColl( SwAttrPool& rPool, const sal_Char* pFmtCollName,
                    SwTxtFmtColl* pDerFrom = 0,
                    sal_uInt16 nFmtWh = RES_TXTFMTCOLL )
        : SwFmtColl( rPool, pFmtCollName, aTxtFmtCollSetRange,
                        pDerFrom, nFmtWh ),
          mbStayAssignedToListLevelOfOutlineStyle( false ),

          mbAssignedToOutlineStyle(false)
    { pNextTxtFmtColl = this; }

    SwTxtFmtColl( SwAttrPool& rPool, const OUString &rFmtCollName,
                    SwTxtFmtColl* pDerFrom = 0,
                    sal_uInt16 nFmtWh = RES_TXTFMTCOLL )
        : SwFmtColl( rPool, rFmtCollName, aTxtFmtCollSetRange,
                        pDerFrom, nFmtWh ),

          mbStayAssignedToListLevelOfOutlineStyle( false ),

          mbAssignedToOutlineStyle(false)
    { pNextTxtFmtColl = this; }

    /// To get UL- / LR- / FontHeight-changes.
   virtual void Modify( const SfxPoolItem*, const SfxPoolItem* );

public:

    TYPEINFO(); ///< Already in base class Client.

    inline void SetNextTxtFmtColl(SwTxtFmtColl& rNext);
    SwTxtFmtColl& GetNextTxtFmtColl() const { return *pNextTxtFmtColl; }

    bool IsAtDocNodeSet() const;

    void SetAttrOutlineLevel( int );
    int  GetAttrOutlineLevel() const;
    int  GetAssignedOutlineStyleLevel() const;
    inline bool IsAssignedToListLevelOfOutlineStyle() const
    {
        return mbAssignedToOutlineStyle;
    }
    void AssignToListLevelOfOutlineStyle(const int nAssignedListLevel);
    void DeleteAssignmentToListLevelOfOutlineStyle();

    /** Override to recognize changes on the <SwNumRuleItem> and register/unregister
     the paragragh style at the corresponding <SwNumRule> instance. */
    virtual bool SetFmtAttr( const SfxPoolItem& rAttr );
    virtual bool SetFmtAttr( const SfxItemSet& rSet );
    virtual bool ResetFmtAttr( sal_uInt16 nWhich1, sal_uInt16 nWhich2 = 0 );

    /// Override <ResetAllFmtAttr()> to stay assigned to list level of outline style.
    virtual sal_uInt16 ResetAllFmtAttr();

    inline bool StayAssignedToListLevelOfOutlineStyle() const
    {
        return mbStayAssignedToListLevelOfOutlineStyle;
    }

    bool AreListLevelIndentsApplicable() const;

/*
 Is the functionality of character styles at paragraph styles needed?
 If so, a second Attset for char-attributes has to be created
 in TextNode and here in TxtCollection in order to make both
 the inheritance of and the access to set attributes function correctly!

    virtual sal_Bool SetDerivedFrom( SwFmtColl* pDerFrom = 0 );

    inline SwCharFmt* GetCharFmt() const;
    inline sal_Bool IsCharFmtSet() const;
    void SetCharFmt(SwCharFmt *);
    void ResetCharFmt();
    inline sal_Bool SwTxtFmtColl::IsCharFmtSet() const
    {
        return aCharDepend.GetRegisteredIn() ? sal_True : sal_False;
    }
    inline SwCharFmt* SwTxtFmtColl::GetCharFmt() const
    {
        return (SwCharFmt*)aCharDepend.GetRegisteredIn();
    }
*/
};

class SwGrfFmtColl: public SwFmtColl
{
    friend class SwDoc;
protected:
    SwGrfFmtColl( SwAttrPool& rPool, const sal_Char* pFmtCollName,
                    SwGrfFmtColl* pDerFrom = 0 )
        : SwFmtColl( rPool, pFmtCollName, aGrfFmtCollSetRange,
                    pDerFrom, RES_GRFFMTCOLL )
    {}

    SwGrfFmtColl( SwAttrPool& rPool, const OUString &rFmtCollName,
                    SwGrfFmtColl* pDerFrom = 0 )
        : SwFmtColl( rPool, rFmtCollName, aGrfFmtCollSetRange,
                    pDerFrom, RES_GRFFMTCOLL )
    {}

public:
    TYPEINFO(); ///< Already in base class Client.
};

// FEATURE::CONDCOLL
/// Conditional styles.
enum Master_CollConditions
{
    PARA_IN_LIST        = 0x0001,
    PARA_IN_OUTLINE     = 0x0002,
    PARA_IN_FRAME       = 0x0004,
    PARA_IN_TABLEHEAD   = 0x0008,
    PARA_IN_TABLEBODY   = 0x0010,
    PARA_IN_SECTION     = 0x0020,
    PARA_IN_FOOTENOTE   = 0x0040,
    PARA_IN_FOOTER      = 0x0080,
    PARA_IN_HEADER      = 0x0100,
    PARA_IN_ENDNOTE     = 0x0200,
    // ...
    USRFLD_EXPRESSION   = (int)0x8000
};

class SW_DLLPUBLIC SwCollCondition : public SwClient
{
    sal_uLong nCondition;
    union
    {
        sal_uLong nSubCondition;
        String* pFldExpression;
    } aSubCondition;

public:
    TYPEINFO(); ///< Already in base class Client.

    SwCollCondition( SwTxtFmtColl* pColl, sal_uLong nMasterCond,
                    sal_uLong nSubCond = 0 );
    SwCollCondition( SwTxtFmtColl* pColl, sal_uLong nMasterCond,
                    const String& rSubExp );
    virtual ~SwCollCondition();

    /// @@@ public copy ctor, but no copy assignment?
    SwCollCondition( const SwCollCondition& rCpy );
private:
    /// @@@ public copy ctor, but no copy assignment?
    SwCollCondition & operator= (const SwCollCondition &);
public:

    int operator==( const SwCollCondition& rCmp ) const;
    int operator!=( const SwCollCondition& rCmp ) const
                            { return ! (*this == rCmp); }

    sal_uLong GetCondition() const      { return nCondition; }
    sal_uLong GetSubCondition() const   { return aSubCondition.nSubCondition; }
    const String* GetFldExpression() const
                                    { return aSubCondition.pFldExpression; }

    void SetCondition( sal_uLong nCond, sal_uLong nSubCond );
    SwTxtFmtColl* GetTxtFmtColl() const     { return (SwTxtFmtColl*)GetRegisteredIn(); }
    void RegisterToFormat( SwFmt& );
};

class SwFmtCollConditions : public boost::ptr_vector<SwCollCondition> {};

class SW_DLLPUBLIC SwConditionTxtFmtColl : public SwTxtFmtColl
{
    friend class SwDoc;
protected:
    SwFmtCollConditions aCondColls;

    SwConditionTxtFmtColl( SwAttrPool& rPool, const sal_Char* pFmtCollName,
                            SwTxtFmtColl* pDerFrom = 0 )
        : SwTxtFmtColl( rPool, pFmtCollName, pDerFrom, RES_CONDTXTFMTCOLL )
    {}
    SwConditionTxtFmtColl( SwAttrPool& rPool, const String &rFmtCollName,
                            SwTxtFmtColl* pDerFrom = 0 )
        : SwTxtFmtColl( rPool, rFmtCollName, pDerFrom, RES_CONDTXTFMTCOLL )
    {}

public:
    TYPEINFO(); ///< Already in base class Client.

    virtual ~SwConditionTxtFmtColl();

    const SwCollCondition* HasCondition( const SwCollCondition& rCond ) const;
    const SwFmtCollConditions& GetCondColls() const     { return aCondColls; }
    void InsertCondition( const SwCollCondition& rCond );
    bool RemoveCondition( const SwCollCondition& rCond );

    void SetConditions( const SwFmtCollConditions& );
};

// FEATURE::CONDCOLL
/// Inline implementations.
inline void SwTxtFmtColl::SetNextTxtFmtColl( SwTxtFmtColl& rNext )
{
    pNextTxtFmtColl = &rNext;
}
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
