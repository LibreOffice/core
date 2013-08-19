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
#ifndef SW_DOCSTYLE_HXX
#define SW_DOCSTYLE_HXX

#include <rtl/ref.hxx>
#include <rtl/ustring.hxx>

#include <svl/style.hxx>
#include <svl/itemset.hxx>
#include "swdllapi.h"

#include <vector>

class SwDoc;
class SwDocStyleSheetPool;
class SwPageDesc;
class SwCharFmt;
class SwTxtFmtColl;
class SwFrmFmt;
class SwNumRule;

/*--------------------------------------------------------------------
    Local helper class.
 --------------------------------------------------------------------*/
class SwPoolFmtList : public std::vector<OUString>
{
public:
    SwPoolFmtList() {}
    void Append( char cChar, const OUString& rStr );
    void Erase();
};

/*--------------------------------------------------------------------
    Temporary StyleSheet.
 --------------------------------------------------------------------*/
class SW_DLLPUBLIC SwDocStyleSheet : public SfxStyleSheetBase
{
    friend class SwDocStyleSheetPool;
    friend class SwStyleSheetIterator;

    SwCharFmt*          pCharFmt;
    SwTxtFmtColl*       pColl;
    SwFrmFmt*           pFrmFmt;
    const SwPageDesc*   pDesc;
    const SwNumRule*    pNumRule;

    SwDoc&              rDoc;
    SfxItemSet          aCoreSet;

    sal_Bool                bPhysical;


    /// Make empty shell a real StyleSheet (Core).
    SW_DLLPRIVATE void              Create();

    /// Fill StyleSheet with data.
    enum FillStyleType {
        FillOnlyName,
        FillAllInfo,
        FillPhysical
    };

    SW_DLLPRIVATE sal_Bool FillStyleSheet( FillStyleType eFType );

protected:
    virtual ~SwDocStyleSheet();

public:
    SwDocStyleSheet( SwDoc&                 rDoc,
                     const OUString&        rName,
                     SwDocStyleSheetPool*   pPool,
                     SfxStyleFamily         eFam,
                     sal_uInt16             nMask);

    SwDocStyleSheet( const SwDocStyleSheet& );

    void                    Reset();

    void                    SetMask(sal_uInt16 nMsk)            { nMask = nMsk;     }
    void                    SetFamily(SfxStyleFamily eFam)  { nFamily = eFam;   }

    sal_Bool                    IsPhysical() const              { return bPhysical; }
    void                    SetPhysical(sal_Bool bPhys);

    virtual void            SetHidden( sal_Bool bHidden );
    virtual sal_Bool        IsHidden( ) const;

    /** add optional parameter <bResetIndentAttrsAtParagraphStyle>, default value sal_False,
     which indicates that the indent attributes at a paragraph style should
     be reset in case that a list style is applied to the paragraph style and
     no indent attributes are applied. */
    void                    SetItemSet( const SfxItemSet& rSet,
                                        const bool bResetIndentAttrsAtParagraphStyle = false );

    virtual SfxItemSet&     GetItemSet();
    /** new method for paragraph styles to merge indent attributes of applied list
     style into the given item set, if the list style indent attributes are applicable. */
    void MergeIndentAttrsOfListStyle( SfxItemSet& rSet );
    virtual const OUString& GetParent() const;
    virtual const OUString& GetFollow() const;

    virtual sal_uLong GetHelpId( OUString& rFile );
    virtual void SetHelpId( const OUString& r, sal_uLong nId );

    /** Preset the members without physical access.
     Used by StyleSheetPool. */
    void                    PresetName(const OUString& rName)  { aName   = rName; }
    void                    PresetNameAndFamily(const OUString& rName);
    void                    PresetParent(const OUString& rName){ aParent = rName; }
    void                    PresetFollow(const OUString& rName){ aFollow = rName; }

    virtual bool            SetName( const OUString& rStr);
    virtual bool            SetParent( const OUString& rStr);
    virtual bool            SetFollow( const OUString& rStr);

    virtual bool            HasFollowSupport() const;
    virtual bool            HasParentSupport() const;
    virtual bool            HasClearParentSupport() const;
    virtual OUString        GetDescription();
    virtual OUString        GetDescription(SfxMapUnit eUnit);

    SwCharFmt*              GetCharFmt();
    SwTxtFmtColl*           GetCollection();
    SwFrmFmt*               GetFrmFmt();
    const SwPageDesc*       GetPageDesc();
    const SwNumRule*        GetNumRule();
    void                    SetNumRule(const SwNumRule& rRule);

    virtual bool            IsUsed() const;
};

/*--------------------------------------------------------------------
   Iterator for Pool.
 --------------------------------------------------------------------*/

class SwStyleSheetIterator : public SfxStyleSheetIterator, public SfxListener
{
    rtl::Reference< SwDocStyleSheet > mxIterSheet;
    rtl::Reference< SwDocStyleSheet > mxStyleSheet;
    SwPoolFmtList       aLst;
    sal_uInt16              nLastPos;
    sal_Bool                bFirstCalled;

    void                AppendStyleList(const ::std::vector<OUString>& rLst,
                                        sal_Bool    bUsed,
                                        sal_Bool    bTestHidden,
                                        bool        bOnlyHidden,
                                        sal_uInt16  nSection,
                                        char    cType);

public:
    SwStyleSheetIterator( SwDocStyleSheetPool* pBase,
                          SfxStyleFamily eFam, sal_uInt16 n=SFXSTYLEBIT_ALL );
    virtual ~SwStyleSheetIterator();

    virtual sal_uInt16 Count();
    virtual SfxStyleSheetBase *operator[](sal_uInt16 nIdx);
    virtual SfxStyleSheetBase* First();
    virtual SfxStyleSheetBase* Next();
    virtual SfxStyleSheetBase* Find(const OUString& rStr);

    virtual void Notify( SfxBroadcaster&, const SfxHint& );

    void InvalidateIterator();
};


class SwDocStyleSheetPool : public SfxStyleSheetBasePool
{
    rtl::Reference< SwDocStyleSheet > mxStyleSheet;
    SwDoc&              rDoc;
    sal_Bool                bOrganizer : 1;     ///< Organizer


    virtual SfxStyleSheetBase* Create( const OUString&, SfxStyleFamily, sal_uInt16 nMask);
    virtual SfxStyleSheetBase* Create( const SfxStyleSheetBase& );

    using SfxStyleSheetBasePool::Find;

public:
    SwDocStyleSheetPool( SwDoc&, sal_Bool bOrganizer = sal_False );

    virtual void Replace( SfxStyleSheetBase& rSource,
                          SfxStyleSheetBase& rTarget );
    virtual SfxStyleSheetBase& Make(const OUString&, SfxStyleFamily,
            sal_uInt16 nMask) SAL_OVERRIDE;

    virtual SfxStyleSheetBase* Find( const OUString&, SfxStyleFamily eFam,
                                    sal_uInt16 n=SFXSTYLEBIT_ALL );

    virtual bool SetParent( SfxStyleFamily eFam, const OUString &rStyle,
                            const OUString &rParent );

    virtual void Remove( SfxStyleSheetBase* pStyle);

    void    SetOrganizerMode( sal_Bool bMode )  { bOrganizer = bMode; }
    sal_Bool    IsOrganizerMode() const         { return bOrganizer; }

    virtual SfxStyleSheetIteratorPtr CreateIterator( SfxStyleFamily, sal_uInt16 nMask );

    SwDoc& GetDoc() const { return rDoc; }

    void dispose();

    virtual void SAL_CALL acquire(  ) throw ();
    virtual void SAL_CALL release(  ) throw ();

    void InvalidateIterator();

protected:
    virtual ~SwDocStyleSheetPool();

    /// For not-so-clever compilers.
private:
    SwDocStyleSheetPool( const SwDocStyleSheetPool& );
};


#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
