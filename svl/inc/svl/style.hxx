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
mußt broadcast this using <SfxStyleSheetBasePool::GetBroadcaster()> broadcasten.
The class <SfxStyleSheetHint> is used for this, it contains an Action-Id and a
pointer to the <SfxStyleSheetBase>. The actions are:

#define SFX_STYLESHEET_CREATED      // style is created
#define SFX_STYLESHEET_MODIFIED     // style is modified
#define SFX_STYLESHEET_CHANGED      // style is replaced
#define SFX_STYLESHEET_ERASED       // style is deleted

The following methods already broadcast themself

SfxStyleSheetHint(SFX_STYLESHEET_MODIFIED) from:
   SfxStyleSheetBase::SetName( const String& rName )
   SfxStyleSheetBase::SetParent( const String& rName )
   SfxStyleSheetBase::SetFollow( const String& rName )

SfxSimpleHint(SFX_HINT_DYING) from:
   SfxStyleSheetBasePool::~SfxStyleSheetBasePool()

SfxStyleSheetHint( SFX_STYLESHEET_CREATED, *p ) from:
   SfxStyleSheetBasePool::Make( const String& rName,
   SfxStyleFamily eFam, USHORT mask, USHORT nPos)

SfxStyleSheetHint( SFX_STYLESHEET_CHANGED, *pNew ) from:
   SfxStyleSheetBasePool::Add( SfxStyleSheetBase& rSheet )

SfxStyleSheetHint( SFX_STYLESHEET_ERASED, *p ) from:
   SfxStyleSheetBasePool::Erase( SfxStyleSheetBase* p )
   SfxStyleSheetBasePool::Clear()
*/

#define VIRTUAL510 virtual

class SVL_DLLPUBLIC SfxStyleSheetBase : public comphelper::OWeakTypeObject
{
    friend class SfxStyleSheetBasePool;

protected:
    SfxStyleSheetBasePool&  rPool;          // zugehoeriger Pool
    SfxStyleFamily          nFamily;        // Familie

    UniString               aName, aParent, aFollow;
    rtl::OUString           maDisplayName;
    String                  aHelpFile;      // Name der Hilfedatei
    SfxItemSet*             pSet;           // ItemSet
    USHORT                  nMask;          // Flags

    ULONG                   nHelpId;        // Hilfe-ID

    BOOL                    bMySet;         // TRUE: Set loeschen im dtor

    SfxStyleSheetBase(); // do not use!
    SfxStyleSheetBase( const UniString&, SfxStyleSheetBasePool&, SfxStyleFamily eFam, USHORT mask );
    SfxStyleSheetBase( const SfxStyleSheetBase& );
    virtual ~SfxStyleSheetBase();
    virtual void Load( SvStream&, USHORT );
    virtual void Store( SvStream& );

public:
    TYPEINFO();

    // returns the internal name of this style
    virtual const UniString& GetName() const;

    // sets the internal name of this style
    virtual BOOL SetName( const UniString& );

    /** returns the display name of this style, it is used at the user interface.
        If the display name is empty, this method returns the internal name. */
    virtual rtl::OUString GetDisplayName() const;

    // sets the display name of this style
    virtual void SetDisplayName( const rtl::OUString& );

    virtual const UniString& GetParent() const;
    virtual BOOL SetParent( const UniString& );
    virtual const UniString& GetFollow() const;
    virtual BOOL SetFollow( const UniString& );
    virtual BOOL HasFollowSupport() const;      // Default TRUE
    virtual BOOL HasParentSupport() const;      // Default TRUE
    virtual BOOL HasClearParentSupport() const; // Default FALSE
    virtual BOOL IsUsed() const;                // Default TRUE
        // Default aus dem Itemset; entweder dem uebergebenen
        // oder aus dem per GetItemSet() zurueckgelieferten Set
    virtual UniString GetDescription();
    virtual UniString GetDescription( SfxMapUnit eMetric );

    SfxStyleSheetBasePool& GetPool() { return rPool;   }
    SfxStyleFamily GetFamily() const     { return nFamily; }
    USHORT   GetMask() const     { return nMask; }
    void     SetMask( USHORT mask) { nMask = mask; }
    BOOL   IsUserDefined() const
           { return BOOL( ( nMask & SFXSTYLEBIT_USERDEF) != 0 ); }

    virtual ULONG GetHelpId( String& rFile );
    virtual void   SetHelpId( const String& r, ULONG nId );

    virtual SfxItemSet& GetItemSet();
    virtual USHORT GetVersion() const;
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
                          SfxStyleFamily eFam, USHORT n=0xFFFF );
    virtual USHORT GetSearchMask() const;
    virtual SfxStyleFamily GetSearchFamily() const;
    virtual USHORT Count();
    virtual SfxStyleSheetBase *operator[](USHORT nIdx);
    virtual SfxStyleSheetBase* First();
    virtual SfxStyleSheetBase* Next();
    virtual SfxStyleSheetBase* Find(const UniString& rStr);
    virtual ~SfxStyleSheetIterator();

protected:

    SfxStyleSheetBasePool*  pBasePool;
    SfxStyleFamily          nSearchFamily;
    USHORT                  nMask;
    BOOL                    SearchUsed() const { return bSearchUsed; }

private:
    USHORT                  GetPos(){return nAktPosition;}
    SVL_DLLPRIVATE BOOL                     IsTrivialSearch();
    SVL_DLLPRIVATE BOOL                     DoesStyleMatch(SfxStyleSheetBase *pStyle);

    void*                   pImp;
    SfxStyleSheetBase*      pAktStyle;
    USHORT                  nAktPosition;
    BOOL                    bSearchUsed;

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
    SVL_DLLPRIVATE BOOL                         Load1_Impl( SvStream& );
    SVL_DLLPRIVATE SfxStyleSheetIterator&      GetIterator_Impl();
protected:
    String                      aAppName;
    SfxItemPool&                rPool;
    SfxStyles                   aStyles;
    SfxStyleFamily              nSearchFamily;
    USHORT                      nMask;

    SfxStyleSheetBase&          Add( SfxStyleSheetBase& );
    void                        ChangeParent( const UniString&, const UniString&, BOOL bVirtual = TRUE );
    virtual SfxStyleSheetBase*  Create( const UniString&, SfxStyleFamily, USHORT );
    virtual SfxStyleSheetBase*  Create( const SfxStyleSheetBase& );

                                ~SfxStyleSheetBasePool();

public:
                                SfxStyleSheetBasePool( SfxItemPool& );
                                SfxStyleSheetBasePool( const SfxStyleSheetBasePool& );

    static String               GetStreamName();

    const String&               GetAppName() const { return aAppName;   }

    SfxItemPool&                GetPool();
    const SfxItemPool&          GetPool() const;

    virtual SfxStyleSheetIterator* CreateIterator(SfxStyleFamily, USHORT nMask);
    virtual USHORT              Count();
    virtual SfxStyleSheetBase*  operator[](USHORT nIdx);

    virtual SfxStyleSheetBase&  Make(const UniString&,
                                     SfxStyleFamily eFam,
                                     USHORT nMask = 0xffff ,
                                     USHORT nPos = 0xffff);

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
    virtual SfxStyleSheetBase*  Find( const UniString&, SfxStyleFamily eFam, USHORT n=0xFFFF );

    virtual BOOL                SetParent(SfxStyleFamily eFam,
                                          const UniString &rStyle,
                                          const UniString &rParent);

    SfxStyleSheetBase*          Find(const UniString& rStr)
                                { return Find(rStr, nSearchFamily, nMask); }

    void                        SetSearchMask(SfxStyleFamily eFam, USHORT n=0xFFFF );
    USHORT                      GetSearchMask() const;
    SfxStyleFamily              GetSearchFamily() const  { return nSearchFamily; }

    BOOL                        Load( SvStream& );
    BOOL                        Store( SvStream&, BOOL bUsed = TRUE );
};

//=========================================================================

class SVL_DLLPUBLIC SfxStyleSheet: public SfxStyleSheetBase,
                     public SfxListener, public SfxBroadcaster
{
public:
                        TYPEINFO();

                        SfxStyleSheet( const UniString&, const SfxStyleSheetBasePool&, SfxStyleFamily, USHORT );
                        SfxStyleSheet( const SfxStyleSheet& );

    virtual void        Notify( SfxBroadcaster& rBC, const SfxHint& rHint );
    virtual BOOL        SetParent( const UniString& );

protected:
    SfxStyleSheet(); // do not use!
    virtual             ~SfxStyleSheet();
};

//=========================================================================

class SVL_DLLPUBLIC SfxStyleSheetPool: public SfxStyleSheetBasePool
{
protected:
    using SfxStyleSheetBasePool::Create;
    virtual SfxStyleSheetBase* Create(const UniString&, SfxStyleFamily, USHORT mask);
    virtual SfxStyleSheetBase* Create(const SfxStyleSheet &);

public:
    SfxStyleSheetPool( SfxItemPool const& );

//  virtual BOOL CopyTo(SfxStyleSheetPool &rDest, const String &rSourceName);
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
    USHORT nHint;

public:
    TYPEINFO();

                        SfxStyleSheetPoolHint(USHORT nArgHint) :  nHint(nArgHint){}
    USHORT              GetHint() const
                        { return nHint; }
};

//=========================================================================

class SVL_DLLPUBLIC SfxStyleSheetHint: public SfxHint
{
    SfxStyleSheetBase*  pStyleSh;
    USHORT              nHint;

public:
                        TYPEINFO();

                        SfxStyleSheetHint( USHORT );
                        SfxStyleSheetHint( USHORT, SfxStyleSheetBase& );
    SfxStyleSheetBase*  GetStyleSheet() const
                        { return pStyleSh; }
    USHORT              GetHint() const
                        { return nHint; }
};

class SVL_DLLPUBLIC SfxStyleSheetHintExtended: public SfxStyleSheetHint
{
    String              aName;

public:
                        TYPEINFO();

                        SfxStyleSheetHintExtended(
                            USHORT, const String& rOld );
                        SfxStyleSheetHintExtended(
                            USHORT, const String& rOld,
                            SfxStyleSheetBase& );
    const String&       GetOldName() { return aName; }
};

class SVL_DLLPUBLIC SfxUnoStyleSheet : public ::cppu::ImplInheritanceHelper2< SfxStyleSheet, ::com::sun::star::style::XStyle, ::com::sun::star::lang::XUnoTunnel >
{
public:
    SfxUnoStyleSheet( const UniString& _rName, const SfxStyleSheetBasePool& _rPool, SfxStyleFamily _eFamily, USHORT _nMaske );
    SfxUnoStyleSheet( const SfxStyleSheet& _rSheet );

    static SfxUnoStyleSheet* getUnoStyleSheet( const ::com::sun::star::uno::Reference< ::com::sun::star::style::XStyle >& xStyle );

    // XUnoTunnel
    virtual ::sal_Int64 SAL_CALL getSomething( const ::com::sun::star::uno::Sequence< ::sal_Int8 >& aIdentifier ) throw (::com::sun::star::uno::RuntimeException);

private:
    SfxUnoStyleSheet(); // not implemented

    static const ::com::sun::star::uno::Sequence< ::sal_Int8 >& getIdentifier();
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
