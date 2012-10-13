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

#ifndef _SFXSTYLE_HXX
#define _SFXSTYLE_HXX

#include <com/sun/star/style/XStyle.hpp>
#include <com/sun/star/lang/XUnoTunnel.hpp>

#include <rtl/ref.hxx>
#include <vector>
#include <comphelper/weak.hxx>
#include <cppuhelper/implbase2.hxx>
#include "svl/svldllapi.h"
#include <rsc/rscsfx.hxx>
#include <tools/string.hxx>
#include <svl/hint.hxx>
#include <svl/lstner.hxx>
#include <svl/brdcst.hxx>
#include <svl/poolitem.hxx>

#include <svl/style.hrc>

class SfxItemSet;
class SfxItemPool;

class SfxStyleSheetBasePool;
class SvStream;

/*
Everyone changing instances of SfxStyleSheetBasePool or SfxStyleSheetBase
must broadcast this using <SfxStyleSheetBasePool::GetBroadcaster()> broadcasts.
The class <SfxStyleSheetHint> is used for this, it contains an Action-Id and a
pointer to the <SfxStyleSheetBase>. The actions are:

#define SFX_STYLESHEET_CREATED      // style is created
#define SFX_STYLESHEET_MODIFIED     // style is modified
#define SFX_STYLESHEET_CHANGED      // style is replaced
#define SFX_STYLESHEET_ERASED       // style is deleted

The following methods already broadcast themself

SfxSimpleHint(SFX_HINT_DYING) from:
   SfxStyleSheetBasePool::~SfxStyleSheetBasePool()

SfxStyleSheetHint( SFX_STYLESHEET_CREATED, *p ) from:
   SfxStyleSheetBasePool::Make( const String& rName,
   SfxStyleFamily eFam, sal_uInt16 mask, sal_uInt16 nPos)

SfxStyleSheetHint( SFX_STYLESHEET_CHANGED, *pNew ) from:
   SfxStyleSheetBasePool::Add( SfxStyleSheetBase& rSheet )

SfxStyleSheetHint( SFX_STYLESHEET_ERASED, *p ) from:
   SfxStyleSheetBasePool::Erase( SfxStyleSheetBase* p )
   SfxStyleSheetBasePool::Clear()
*/

class SVL_DLLPUBLIC SfxStyleSheetBase : public comphelper::OWeakTypeObject
{
private:
    friend class SfxStyleSheetBasePool;
    SVL_DLLPRIVATE static SfxStyleSheetBasePool& implGetStaticPool();

protected:
    SfxStyleSheetBasePool*  pPool;          // zugehoeriger Pool
    SfxStyleFamily          nFamily;        // Familie

    UniString               aName, aParent, aFollow;
    rtl::OUString           maDisplayName;
    String                  aHelpFile;      // Name der Hilfedatei
    SfxItemSet*             pSet;           // ItemSet
    sal_uInt16              nMask;          // Flags

    sal_uLong               nHelpId;        // Hilfe-ID

    bool                    bMySet;         // sal_True: Set loeschen im dtor

    SfxStyleSheetBase( const UniString&, SfxStyleSheetBasePool*, SfxStyleFamily eFam, sal_uInt16 mask );
    SfxStyleSheetBase( const SfxStyleSheetBase& );
    virtual ~SfxStyleSheetBase();
    virtual void Load( SvStream&, sal_uInt16 );
    virtual void Store( SvStream& );

public:
    TYPEINFO();

    // returns the internal name of this style
    virtual const UniString& GetName() const;

    // sets the internal name of this style
    virtual bool SetName( const UniString& );

    /** returns the display name of this style, it is used at the user interface.
        If the display name is empty, this method returns the internal name. */
    virtual rtl::OUString GetDisplayName() const;

    // sets the display name of this style
    virtual void SetDisplayName( const rtl::OUString& );

    virtual const UniString& GetParent() const;
    virtual bool SetParent( const UniString& );
    virtual const UniString& GetFollow() const;
    virtual bool SetFollow( const UniString& );
    virtual bool HasFollowSupport() const;      // Default true
    virtual bool HasParentSupport() const;      // Default true
    virtual bool HasClearParentSupport() const; // Default false
    virtual bool IsUsed() const;                // Default true
        // Default aus dem Itemset; entweder dem uebergebenen
        // oder aus dem per GetItemSet() zurueckgelieferten Set
    virtual UniString GetDescription();
    virtual UniString GetDescription( SfxMapUnit eMetric );

    SfxStyleSheetBasePool& GetPool() { return *pPool; }
    SfxStyleFamily GetFamily() const     { return nFamily; }
    sal_uInt16   GetMask() const     { return nMask; }
    void     SetMask( sal_uInt16 mask) { nMask = mask; }
    bool     IsUserDefined() const
           { return ( nMask & SFXSTYLEBIT_USERDEF) != 0; }

    virtual sal_uLong GetHelpId( String& rFile );
    virtual void   SetHelpId( const String& r, sal_uLong nId );

    virtual SfxItemSet& GetItemSet();
    virtual sal_uInt16 GetVersion() const;
};

//=========================================================================

typedef std::vector< rtl::Reference< SfxStyleSheetBase > > SfxStyles;

//=========================================================================

class SVL_DLLPUBLIC SfxStyleSheetIterator

/*  [Beschreibung]

    Klasse zum Iterieren und Suchen auf einem SfxStyleSheetBasePool.

*/

{
public:
    SfxStyleSheetIterator(SfxStyleSheetBasePool *pBase,
                          SfxStyleFamily eFam, sal_uInt16 n=0xFFFF );
    virtual sal_uInt16 GetSearchMask() const;
    virtual SfxStyleFamily GetSearchFamily() const;
    virtual sal_uInt16 Count();
    virtual SfxStyleSheetBase *operator[](sal_uInt16 nIdx);
    virtual SfxStyleSheetBase* First();
    virtual SfxStyleSheetBase* Next();
    virtual SfxStyleSheetBase* Find(const rtl::OUString& rStr);
    virtual ~SfxStyleSheetIterator();

protected:

    SfxStyleSheetBasePool*  pBasePool;
    SfxStyleFamily          nSearchFamily;
    sal_uInt16              nMask;

    bool                    SearchUsed() const { return bSearchUsed; }

private:
    sal_uInt16                  GetPos() { return nAktPosition; }
    SVL_DLLPRIVATE bool         IsTrivialSearch();
    SVL_DLLPRIVATE bool         DoesStyleMatch(SfxStyleSheetBase *pStyle);

    SfxStyleSheetBase*      pAktStyle;
    sal_uInt16              nAktPosition;
    bool                    bSearchUsed;

friend class SfxStyleSheetBasePool;
};

//=========================================================================

class SfxStyleSheetBasePool_Impl;

class SVL_DLLPUBLIC SfxStyleSheetBasePool: public SfxBroadcaster, public comphelper::OWeakTypeObject
{
friend class SfxStyleSheetIterator;
friend class SfxStyleSheetBase;

    SfxStyleSheetBasePool_Impl *pImp;

private:
    SVL_DLLPRIVATE SfxStyleSheetIterator&      GetIterator_Impl();
protected:
    String                      aAppName;
    SfxItemPool&                rPool;
    SfxStyles                   aStyles;
    SfxStyleFamily              nSearchFamily;
    sal_uInt16                      nMask;

    SfxStyleSheetBase&          Add( SfxStyleSheetBase& );
    void                        ChangeParent( const UniString&, const UniString&, bool bVirtual = true );
    virtual SfxStyleSheetBase*  Create( const UniString&, SfxStyleFamily, sal_uInt16 );
    virtual SfxStyleSheetBase*  Create( const SfxStyleSheetBase& );

    virtual                     ~SfxStyleSheetBasePool();

public:
                                SfxStyleSheetBasePool( SfxItemPool& );
                                SfxStyleSheetBasePool( const SfxStyleSheetBasePool& );

    const String&               GetAppName() const { return aAppName;   }

    SfxItemPool&                GetPool();
    const SfxItemPool&          GetPool() const;

    virtual SfxStyleSheetIterator* CreateIterator(SfxStyleFamily, sal_uInt16 nMask);
    virtual sal_uInt16              Count();
    virtual SfxStyleSheetBase*  operator[](sal_uInt16 nIdx);

    virtual SfxStyleSheetBase&  Make(const UniString&,
                                     SfxStyleFamily eFam,
                                     sal_uInt16 nMask = 0xffff ,
                                     sal_uInt16 nPos = 0xffff);

    virtual void             Replace(
        SfxStyleSheetBase& rSource, SfxStyleSheetBase& rTarget );

    virtual void                Remove( SfxStyleSheetBase* );
    virtual void                Insert( SfxStyleSheetBase* );

    virtual void                Clear();

    SfxStyleSheetBasePool&      operator=( const SfxStyleSheetBasePool& );
    SfxStyleSheetBasePool&      operator+=( const SfxStyleSheetBasePool& );

    const SfxStyles&            GetStyles();
    virtual SfxStyleSheetBase*  First();
    virtual SfxStyleSheetBase*  Next();
    virtual SfxStyleSheetBase*  Find( const UniString&, SfxStyleFamily eFam, sal_uInt16 n=0xFFFF );

    virtual bool                SetParent(SfxStyleFamily eFam,
                                          const UniString &rStyle,
                                          const UniString &rParent);

    SfxStyleSheetBase*          Find(const UniString& rStr)
                                { return Find(rStr, nSearchFamily, nMask); }

    void                        SetSearchMask(SfxStyleFamily eFam, sal_uInt16 n=0xFFFF );
    sal_uInt16                      GetSearchMask() const;
    SfxStyleFamily              GetSearchFamily() const  { return nSearchFamily; }
};

//=========================================================================

class SVL_DLLPUBLIC SfxStyleSheet: public SfxStyleSheetBase,
                     public SfxListener, public SfxBroadcaster
{
public:
                        TYPEINFO();

                        SfxStyleSheet( const UniString&, const SfxStyleSheetBasePool&, SfxStyleFamily, sal_uInt16 );
                        SfxStyleSheet( const SfxStyleSheet& );

    virtual void        Notify( SfxBroadcaster& rBC, const SfxHint& rHint );
    virtual bool        SetParent( const UniString& );

protected:
    SfxStyleSheet() // do not use! needed by MSVC at compile time to satisfy ImplInheritanceHelper2
        : SfxStyleSheetBase(rtl::OUString("dummy"),
            NULL, SFX_STYLE_FAMILY_ALL, 0)
    {
        assert(false);
    }
    virtual             ~SfxStyleSheet();
};

//=========================================================================

class SVL_DLLPUBLIC SfxStyleSheetPool: public SfxStyleSheetBasePool
{
protected:
    using SfxStyleSheetBasePool::Create;
    virtual SfxStyleSheetBase* Create(const UniString&, SfxStyleFamily, sal_uInt16 mask);
    virtual SfxStyleSheetBase* Create(const SfxStyleSheet &);

public:
    SfxStyleSheetPool( SfxItemPool const& );

//  virtual sal_Bool CopyTo(SfxStyleSheetPool &rDest, const String &rSourceName);
};

//=========================================================================

#define SFX_STYLESHEET_CREATED       1  // neu
#define SFX_STYLESHEET_MODIFIED      2  // ver"andert
#define SFX_STYLESHEET_CHANGED       3  // gel"oscht und neu (ausgetauscht)
#define SFX_STYLESHEET_ERASED        4  // gel"oscht
#define SFX_STYLESHEET_INDESTRUCTION 5  // wird gerade entfernt

#define SFX_STYLESHEETPOOL_CHANGES  1  // Aenderungen, die den Zustand
                                       // des Pools anedern, aber nicht
                                       // ueber die STYLESHEET Hints
                                       // verschickt werden sollen.

//========================================================================

class SVL_DLLPUBLIC SfxStyleSheetPoolHint : public SfxHint
{
    sal_uInt16 nHint;

public:
    TYPEINFO();

                        SfxStyleSheetPoolHint(sal_uInt16 nArgHint) :  nHint(nArgHint){}
    sal_uInt16              GetHint() const
                        { return nHint; }
};

//=========================================================================

class SVL_DLLPUBLIC SfxStyleSheetHint: public SfxHint
{
    SfxStyleSheetBase*  pStyleSh;
    sal_uInt16              nHint;

public:
                        TYPEINFO();

                        SfxStyleSheetHint( sal_uInt16, SfxStyleSheetBase& );
    SfxStyleSheetBase*  GetStyleSheet() const
                        { return pStyleSh; }
    sal_uInt16              GetHint() const
                        { return nHint; }
};

class SVL_DLLPUBLIC SfxStyleSheetHintExtended: public SfxStyleSheetHint
{
    String              aName;

public:
                        TYPEINFO();

                        SfxStyleSheetHintExtended(
                            sal_uInt16, const String& rOld,
                            SfxStyleSheetBase& );
    const String&       GetOldName() { return aName; }
};

class SVL_DLLPUBLIC SfxUnoStyleSheet : public ::cppu::ImplInheritanceHelper2< SfxStyleSheet, ::com::sun::star::style::XStyle, ::com::sun::star::lang::XUnoTunnel >
{
public:
    SfxUnoStyleSheet( const UniString& _rName, const SfxStyleSheetBasePool& _rPool, SfxStyleFamily _eFamily, sal_uInt16 _nMaske );

    static SfxUnoStyleSheet* getUnoStyleSheet( const ::com::sun::star::uno::Reference< ::com::sun::star::style::XStyle >& xStyle );

    // XUnoTunnel
    virtual ::sal_Int64 SAL_CALL getSomething( const ::com::sun::star::uno::Sequence< ::sal_Int8 >& aIdentifier ) throw (::com::sun::star::uno::RuntimeException);

private:
    SfxUnoStyleSheet(); // not implemented

    static const ::com::sun::star::uno::Sequence< ::sal_Int8 >& getIdentifier();
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
