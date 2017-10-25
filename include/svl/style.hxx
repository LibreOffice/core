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

#ifndef INCLUDED_SVL_STYLE_HXX
#define INCLUDED_SVL_STYLE_HXX

#include <com/sun/star/style/XStyle.hpp>
#include <com/sun/star/lang/XUnoTunnel.hpp>

#include <rtl/ref.hxx>
#include <comphelper/weak.hxx>
#include <cppuhelper/implbase.hxx>
#include <svl/svldllapi.h>
#include <svl/hint.hxx>
#include <svl/lstner.hxx>
#include <svl/poolitem.hxx>
#include <svl/SfxBroadcaster.hxx>
#include <svl/stylesheetuser.hxx>

#include <memory>

// This is used as a flags enum in sw/, but only there,
// so I don't pull in o3tl::typed_flags here
enum class SfxStyleFamily {
    None    = 0x00,
    Char    = 0x01,
    Para    = 0x02,
    Frame   = 0x04,
    Page    = 0x08,
    Pseudo  = 0x10,
    Table   = 0x20,
    Cell    = 0x40,
    All     = 0x7fff
};

#define SFXSTYLEBIT_AUTO        0x0000 ///< automatic: flags from application
#define SFXSTYLEBIT_HIDDEN      0x0200 ///< hidden styles (search mask)
#define SFXSTYLEBIT_HIERARCHY   0x1000 ///< hierarchical view - just for dialog
#define SFXSTYLEBIT_READONLY    0x2000 ///< readonly styles (search mask)
#define SFXSTYLEBIT_USED        0x4000 ///< used styles (search mask)
#define SFXSTYLEBIT_USERDEF     0x8000 ///< user defined styles (search mask)
#define SFXSTYLEBIT_ALL_VISIBLE 0xFDFF ///< all styles
#define SFXSTYLEBIT_ALL         0xFFFF ///< all styles

class SfxItemSet;
class SfxItemPool;
class SfxStyleSheetBasePool;
class SvStream;

namespace svl { class IndexedStyleSheets; }
/*
Everyone changing instances of SfxStyleSheetBasePool or SfxStyleSheetBase
must broadcast this using <SfxStyleSheetBasePool::GetBroadcaster()> broadcasts.
The class <SfxStyleSheetHint> is used for this, it contains an Action-Id and a
pointer to the <SfxStyleSheetBase>. The actions are:

#define SfxHintId::StyleSheetCreated      // style is created
#define SfxHintId::StyleSheetModified     // style is modified
#define SfxHintId::StyleSheetChanged      // style is replaced
#define SfxHintId::StyleSheetErased       // style is deleted

The following methods already broadcast themself

SfxSimpleHint(SfxHintId::Dying) from:
   SfxStyleSheetBasePool::~SfxStyleSheetBasePool()

SfxStyleSheetHint( SfxHintId::StyleSheetCreated, *p ) from:
   SfxStyleSheetBasePool::Make( const String& rName,
   SfxStyleFamily eFam, sal_uInt16 mask)

SfxStyleSheetHint( SfxHintId::StyleSheetChanged, *pNew ) from:
   SfxStyleSheetBasePool::Add( SfxStyleSheetBase& rSheet )

SfxStyleSheetHint( SfxHintId::StyleSheetErased, *p ) from:
   SfxStyleSheetBasePool::Erase( SfxStyleSheetBase* p )
   SfxStyleSheetBasePool::Clear()
*/

class SVL_DLLPUBLIC SfxStyleSheetBase : public comphelper::OWeakTypeObject
{
private:
    friend class SfxStyleSheetBasePool;

protected:
    SfxStyleSheetBasePool*  pPool;          // related pool
    SfxStyleFamily          nFamily;

    OUString                aName, aParent, aFollow;
    OUString                maDisplayName;
    OUString                aHelpFile;      // name of the help file
    SfxItemSet*             pSet;           // ItemSet
    sal_uInt16              nMask;          // Flags

    sal_uLong               nHelpId;        // help ID

    bool                    bMySet;         // sal_True: delete Set in dtor
    bool                    bHidden;

    SfxStyleSheetBase( const OUString&, SfxStyleSheetBasePool*, SfxStyleFamily eFam, sal_uInt16 mask );
    SfxStyleSheetBase( const SfxStyleSheetBase& );
    virtual ~SfxStyleSheetBase() override;
    virtual void Load( SvStream&, sal_uInt16 );

public:

    // returns the internal name of this style
    const OUString& GetName() const;

    // sets the internal name of this style.
    //
    // If the name of a style is changed, then the styles container needs to be
    // reindexed (see IndexedStyleSheets). If you set bReindexNow to false to
    // defer that indexing, then you must call the Reindex manually on the
    // SfxStyleSheetBasePool parent.
    virtual bool SetName(const OUString& rNewName, bool bReindexNow = true);

    /** returns the display name of this style, it is used at the user interface.
        If the display name is empty, this method returns the internal name. */
    OUString const & GetDisplayName() const;

    virtual const OUString& GetParent() const;
    virtual bool SetParent( const OUString& );
    virtual const OUString& GetFollow() const;
    virtual bool SetFollow( const OUString& );
    virtual bool HasFollowSupport() const;      // Default true
    virtual bool HasParentSupport() const;      // Default true
    virtual bool HasClearParentSupport() const; // Default false
    virtual bool IsUsed() const;                // Default true
    virtual OUString GetDescription( MapUnit eMetric );

    SfxStyleSheetBasePool& GetPool() { return *pPool; }
    SfxStyleFamily GetFamily() const     { return nFamily; }
    sal_uInt16   GetMask() const     { return nMask; }
    void     SetMask( sal_uInt16 mask) { nMask = mask; }
    bool     IsUserDefined() const
           { return ( nMask & SFXSTYLEBIT_USERDEF) != 0; }

    virtual bool IsHidden() const { return bHidden; }
    virtual void SetHidden( bool bValue );

    virtual sal_uLong GetHelpId( OUString& rFile );
    virtual void   SetHelpId( const OUString& r, sal_uLong nId );

    virtual SfxItemSet& GetItemSet();
    /// Due to writer's usual lack of sanity this is a separate function for
    /// preview only; it shall not create the style in case it does not exist.
    /// If the style has parents, it is _not_ required that the returned item
    /// set has parents (i.e. use it for display purposes only).
    virtual std::unique_ptr<SfxItemSet> GetItemSetForPreview();
};

/* Class to iterate and search on a SfxStyleSheetBasePool */
class SVL_DLLPUBLIC SfxStyleSheetIterator
{
public:
    /** Constructor.
     * The iterator will only iterate over style sheets which have the family \p eFam
     */
    SfxStyleSheetIterator(SfxStyleSheetBasePool *pBase,
                          SfxStyleFamily eFam, sal_uInt16 n=SFXSTYLEBIT_ALL );
    sal_uInt16 GetSearchMask() const;
    SfxStyleFamily GetSearchFamily() const;
    virtual sal_uInt16 Count();
    virtual SfxStyleSheetBase *operator[](sal_uInt16 nIdx);
    virtual SfxStyleSheetBase* First();
    virtual SfxStyleSheetBase* Next();
    virtual SfxStyleSheetBase* Find(const OUString& rStr);
    virtual ~SfxStyleSheetIterator();

    bool                    SearchUsed() const { return bSearchUsed; }

protected:

    SfxStyleSheetBasePool*  pBasePool;
    SfxStyleFamily          nSearchFamily;
    sal_uInt16              nMask;


private:
    SVL_DLLPRIVATE bool         IsTrivialSearch() const;

    SfxStyleSheetBase*      pAktStyle;
    sal_uInt16              nAktPosition;
    bool                    bSearchUsed;

friend class SfxStyleSheetBasePool;
};

class SfxStyleSheetBasePool_Impl;

class SVL_DLLPUBLIC SfxStyleSheetBasePool: public SfxBroadcaster, public comphelper::OWeakTypeObject
{
friend class SfxStyleSheetIterator;
friend class SfxStyleSheetBase;

    std::unique_ptr<SfxStyleSheetBasePool_Impl> pImpl;

protected:
    SfxStyleSheetIterator&      GetIterator_Impl();

    SfxItemPool&                rPool;
    SfxStyleFamily              nSearchFamily;
    sal_uInt16                  nMask;

    void                        ChangeParent( const OUString&, const OUString&, bool bVirtual = true );
    virtual SfxStyleSheetBase*  Create( const OUString&, SfxStyleFamily, sal_uInt16 );
    virtual SfxStyleSheetBase*  Create( const SfxStyleSheetBase& );

    virtual                     ~SfxStyleSheetBasePool() override;

    void                        StoreStyleSheet(const rtl::Reference< SfxStyleSheetBase >&);

    /** Obtain the indexed style sheets.
     */
    const svl::IndexedStyleSheets&
                                GetIndexedStyleSheets() const;
    rtl::Reference<SfxStyleSheetBase>
                                GetStyleSheetByPositionInIndex(unsigned pos);

public:
                                SfxStyleSheetBasePool( SfxItemPool& );
                                SfxStyleSheetBasePool( const SfxStyleSheetBasePool& );

    SfxItemPool&                GetPool() { return rPool;}
    const SfxItemPool&          GetPool() const { return rPool;}

    virtual std::shared_ptr<SfxStyleSheetIterator> CreateIterator(SfxStyleFamily, sal_uInt16 nMask);
    sal_uInt16              Count();
    SfxStyleSheetBase*  operator[](sal_uInt16 nIdx);

    virtual SfxStyleSheetBase&  Make(const OUString&,
                                     SfxStyleFamily eFam,
                                     sal_uInt16 nMask = SFXSTYLEBIT_ALL);

    virtual void                Remove( SfxStyleSheetBase* );
    void                Insert( SfxStyleSheetBase* );

    void                Clear();

    SfxStyleSheetBasePool&      operator=( const SfxStyleSheetBasePool& );
    SfxStyleSheetBasePool&      operator+=( const SfxStyleSheetBasePool& );

    SfxStyleSheetBase*  First();
    SfxStyleSheetBase*  Next();
    virtual SfxStyleSheetBase*  Find( const OUString&, SfxStyleFamily eFam, sal_uInt16 n=SFXSTYLEBIT_ALL );

    virtual bool                SetParent(SfxStyleFamily eFam,
                                          const OUString &rStyle,
                                          const OUString &rParent);

    SfxStyleSheetBase*          Find(const OUString& rStr)
                                { return Find(rStr, nSearchFamily, nMask); }

    void                        SetSearchMask(SfxStyleFamily eFam, sal_uInt16 n=SFXSTYLEBIT_ALL );
    sal_uInt16                      GetSearchMask() const { return nMask;}
    SfxStyleFamily              GetSearchFamily() const  { return nSearchFamily; }

    void                        Reindex();
    /** Add a style sheet.
     * Not an actual public function. Do not call it from non-subclasses.
     */
    void                        Add( const SfxStyleSheetBase& );
};

class SVL_DLLPUBLIC SfxStyleSheet: public SfxStyleSheetBase,
                     public SfxListener, public SfxBroadcaster, public svl::StyleSheetUser
{
public:

                        SfxStyleSheet( const OUString&, const SfxStyleSheetBasePool&, SfxStyleFamily, sal_uInt16 );
                        SfxStyleSheet( const SfxStyleSheet& );

    virtual void        Notify( SfxBroadcaster& rBC, const SfxHint& rHint ) override;

    virtual bool        isUsedByModel() const override;

    virtual bool        SetParent( const OUString& ) override;

protected:
    virtual             ~SfxStyleSheet() override;
};

class SVL_DLLPUBLIC SfxStyleSheetPool: public SfxStyleSheetBasePool
{
protected:
    using SfxStyleSheetBasePool::Create;
    virtual SfxStyleSheetBase* Create(const OUString&, SfxStyleFamily, sal_uInt16 mask) override;

public:
    SfxStyleSheetPool( SfxItemPool const& );
};


class SVL_DLLPUBLIC SfxStyleSheetPoolHint : public SfxHint
{
public:
                         SfxStyleSheetPoolHint() {}
};


class SVL_DLLPUBLIC SfxStyleSheetHint: public SfxHint
{
    SfxStyleSheetBase*  pStyleSh;
public:
                        SfxStyleSheetHint( SfxHintId, SfxStyleSheetBase& );
    SfxStyleSheetBase*  GetStyleSheet() const
                        { return pStyleSh; }
};

class SVL_DLLPUBLIC SfxStyleSheetModifiedHint: public SfxStyleSheetHint
{
    OUString            aName;

public:
                        SfxStyleSheetModifiedHint( const OUString& rOld,
                                                   SfxStyleSheetBase& );
    const OUString&     GetOldName() const { return aName; }
};

class SVL_DLLPUBLIC SfxUnoStyleSheet : public cppu::ImplInheritanceHelper<SfxStyleSheet, css::style::XStyle, css::lang::XUnoTunnel>
{
public:
    SfxUnoStyleSheet( const OUString& _rName, const SfxStyleSheetBasePool& _rPool, SfxStyleFamily _eFamily, sal_uInt16 _nMaske );

    static SfxUnoStyleSheet* getUnoStyleSheet( const css::uno::Reference< css::style::XStyle >& xStyle );

    // XUnoTunnel
    virtual ::sal_Int64 SAL_CALL getSomething( const css::uno::Sequence< ::sal_Int8 >& aIdentifier ) override;

private:
    static const css::uno::Sequence< ::sal_Int8 >& getIdentifier();
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
