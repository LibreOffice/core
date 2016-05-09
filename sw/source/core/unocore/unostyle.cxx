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

#include <svx/svxids.hrc>
#include <hintids.hxx>
#include <osl/mutex.hxx>
#include <vcl/svapp.hxx>
#include <svl/smplhint.hxx>
#include <svtools/ctrltool.hxx>
#include <svl/style.hxx>
#include <svl/itemiter.hxx>
#include <svx/pageitem.hxx>
#include <editeng/sizeitem.hxx>
#include <editeng/ulspitem.hxx>
#include <editeng/lrspitem.hxx>
#include <editeng/boxitem.hxx>
#include <editeng/shaditem.hxx>
#include <editeng/brushitem.hxx>
#include <editeng/flstitem.hxx>
#include <editeng/paperinf.hxx>
#include <pagedesc.hxx>
#include <doc.hxx>
#include <IDocumentUndoRedo.hxx>
#include <IDocumentDeviceAccess.hxx>
#include <IDocumentStylePoolAccess.hxx>
#include <docary.hxx>
#include <charfmt.hxx>
#include <cmdid.h>
#include <unostyle.hxx>
#include <unosett.hxx>
#include <docsh.hxx>
#include <swstyle.h>
#include <paratr.hxx>
#include <unoprnms.hxx>
#include <shellio.hxx>
#include <docstyle.hxx>
#include <unotextbodyhf.hxx>
#include <fmthdft.hxx>
#include <fmtpdsc.hxx>
#include <poolfmt.hrc>
#include <poolfmt.hxx>
#include "unoevent.hxx"
#include <fmtruby.hxx>
#include <SwStyleNameMapper.hxx>
#include <sfx2/printer.hxx>
#include <com/sun/star/style/ParagraphStyleCategory.hpp>
#include <com/sun/star/style/XStyleFamiliesSupplier.hpp>
#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <com/sun/star/beans/NamedValue.hpp>
#include <com/sun/star/drawing/BitmapMode.hpp>
#include <istyleaccess.hxx>
#include <GetMetricVal.hxx>
#include <fmtfsize.hxx>
#include <numrule.hxx>

#include <comphelper/servicehelper.hxx>
#include <cppuhelper/supportsservice.hxx>
#include <comphelper/sequence.hxx>

//UUUU
#include <svx/unobrushitemhelper.hxx>
#include <editeng/unoipset.hxx>
#include <editeng/memberids.hrc>
#include <svx/unoshape.hxx>
#include <svx/xflbstit.hxx>
#include <svx/xflbmtit.hxx>
#include <swunohelper.hxx>
#include <svx/xbtmpit.hxx>

#include "ccoll.hxx"
#include "unocore.hrc"

#include <cassert>
#include <memory>
#include <set>
#include <limits>

namespace
{
    struct StyleFamilyEntry
    {
        using GetCountOrName_t = std::function<sal_Int32 (const SwDoc&, OUString*, sal_Int32)>;
        using CreateStyle_t = std::function<uno::Reference<css::style::XStyle>(SfxStyleSheetBasePool*, SwDocShell*, const OUString&)>;
        using TranslateIndex_t = std::function<sal_uInt16(const sal_uInt16)>;
        SfxStyleFamily m_eFamily;
        sal_uInt16 m_nPropMapType;
        uno::Reference<beans::XPropertySetInfo> m_xPSInfo;
        SwGetPoolIdFromName m_aPoolId;
        OUString m_sName;
        sal_uInt32 m_nResId;
        GetCountOrName_t m_fGetCountOrName;
        CreateStyle_t m_fCreateStyle;
        TranslateIndex_t m_fTranslateIndex;
        StyleFamilyEntry(SfxStyleFamily eFamily, sal_uInt16 nPropMapType, SwGetPoolIdFromName aPoolId, OUString const& sName, sal_uInt32 nResId, GetCountOrName_t fGetCountOrName, CreateStyle_t fCreateStyle, TranslateIndex_t fTranslateIndex)
                : m_eFamily(eFamily)
                , m_nPropMapType(nPropMapType)
                , m_xPSInfo(aSwMapProvider.GetPropertySet(nPropMapType)->getPropertySetInfo())
                , m_aPoolId(aPoolId)
                , m_sName(sName)
                , m_nResId(nResId)
                , m_fGetCountOrName(fGetCountOrName)
                , m_fCreateStyle(fCreateStyle)
                , m_fTranslateIndex(fTranslateIndex)
            { }
    };
    static const std::vector<StyleFamilyEntry>* our_pStyleFamilyEntries;
    // these should really be constexprs, but MSVC still is apparently too stupid for them
    #define nPoolChrNormalRange (RES_POOLCHR_NORMAL_END - RES_POOLCHR_NORMAL_BEGIN)
    #define nPoolChrHtmlRange   (RES_POOLCHR_HTML_END   - RES_POOLCHR_HTML_BEGIN)
    #define nPoolCollTextRange     ( RES_POOLCOLL_TEXT_END  - RES_POOLCOLL_TEXT_BEGIN)
    #define nPoolCollListsRange    ( RES_POOLCOLL_LISTS_END    - RES_POOLCOLL_LISTS_BEGIN)
    #define nPoolCollExtraRange    ( RES_POOLCOLL_EXTRA_END    - RES_POOLCOLL_EXTRA_BEGIN)
    #define nPoolCollRegisterRange ( RES_POOLCOLL_REGISTER_END - RES_POOLCOLL_REGISTER_BEGIN)
    #define nPoolCollDocRange      ( RES_POOLCOLL_DOC_END      - RES_POOLCOLL_DOC_BEGIN)
    #define nPoolCollHtmlRange     ( RES_POOLCOLL_HTML_END     - RES_POOLCOLL_HTML_BEGIN)
    #define nPoolFrameRange ( RES_POOLFRM_END - RES_POOLFRM_BEGIN)
    #define nPoolPageRange  ( RES_POOLPAGE_END - RES_POOLPAGE_BEGIN)
    #define nPoolNumRange   ( RES_POOLNUMRULE_END - RES_POOLNUMRULE_BEGIN)
    #define nPoolCollListsStackedStart    ( nPoolCollTextRange)
    #define nPoolCollExtraStackedStart    ( nPoolCollListsStackedStart    + nPoolCollListsRange)
    #define nPoolCollRegisterStackedStart ( nPoolCollExtraStackedStart    + nPoolCollExtraRange)
    #define nPoolCollDocStackedStart      ( nPoolCollRegisterStackedStart + nPoolCollRegisterRange)
    #define nPoolCollHtmlStackedStart     ( nPoolCollDocStackedStart      + nPoolCollDocRange)
    using paragraphstyle_t = std::remove_const<decltype(style::ParagraphStyleCategory::TEXT)>::type;
    using swstylebits_t = sal_uInt16;
    using collectionbits_t = sal_uInt16;
    struct ParagraphStyleCategoryEntry
    {
        paragraphstyle_t m_eCategory;
        swstylebits_t m_nSwStyleBits;
        collectionbits_t m_nCollectionBits;
        ParagraphStyleCategoryEntry(paragraphstyle_t eCategory, swstylebits_t nSwStyleBits, collectionbits_t nCollectionBits)
                : m_eCategory(eCategory)
                , m_nSwStyleBits(nSwStyleBits)
                , m_nCollectionBits(nCollectionBits)
            { }
    };
    static const std::vector<ParagraphStyleCategoryEntry>* our_pParagraphStyleCategoryEntries;
}
static const std::vector<StyleFamilyEntry>* lcl_GetStyleFamilyEntries();

using namespace ::com::sun::star;

namespace sw
{
    class XStyleFamily : public cppu::WeakImplHelper
    <
        container::XNameContainer,
        lang::XServiceInfo,
        container::XIndexAccess,
        beans::XPropertySet
    >
    , public SfxListener
    {
        const StyleFamilyEntry& m_rEntry;
        SfxStyleSheetBasePool* m_pBasePool;
        SwDocShell* m_pDocShell;

        SwXStyle* FindStyle(const OUString& rStyleName) const;
        sal_Int32 GetCountOrName(OUString* pString, sal_Int32 nIndex = SAL_MAX_INT32)
            { return m_rEntry.m_fGetCountOrName(*m_pDocShell->GetDoc(), pString, nIndex); };
        static const StyleFamilyEntry& InitEntry(SfxStyleFamily eFamily)
        {
            auto pEntries = lcl_GetStyleFamilyEntries();
            const auto pEntry = std::find_if(pEntries->begin(), pEntries->end(),
                    [eFamily] (const StyleFamilyEntry& e) { return e.m_eFamily == eFamily; });
            assert(pEntry != pEntries->end());
            return *pEntry;
        }
    public:
        XStyleFamily(SwDocShell* pDocShell, const SfxStyleFamily eFamily)
            : m_rEntry(InitEntry(eFamily))
            , m_pBasePool(pDocShell->GetStyleSheetPool())
            , m_pDocShell(pDocShell)
        {
            StartListening(*m_pBasePool);
        }

        virtual ~XStyleFamily() {};

        //XIndexAccess
        virtual sal_Int32 SAL_CALL getCount() throw( uno::RuntimeException, std::exception ) override
        {
            SolarMutexGuard aGuard;
            return GetCountOrName(nullptr);
        };
        virtual uno::Any SAL_CALL getByIndex(sal_Int32 nIndex) throw( lang::IndexOutOfBoundsException, lang::WrappedTargetException, uno::RuntimeException, std::exception ) override;

        //XElementAccess
        virtual uno::Type SAL_CALL SAL_CALL getElementType(  ) throw(uno::RuntimeException, std::exception) override
            { return cppu::UnoType<style::XStyle>::get(); };
        virtual sal_Bool SAL_CALL SAL_CALL hasElements(  ) throw(uno::RuntimeException, std::exception) override
        {
            if(!m_pBasePool)
                throw uno::RuntimeException();
            return true;
        }

        //XNameAccess
        virtual uno::Any SAL_CALL getByName(const OUString& Name) throw( container::NoSuchElementException, lang::WrappedTargetException, uno::RuntimeException, std::exception ) override;
        virtual uno::Sequence< OUString > SAL_CALL getElementNames() throw( uno::RuntimeException, std::exception ) override;
        virtual sal_Bool SAL_CALL hasByName(const OUString& Name) throw( uno::RuntimeException, std::exception ) override;

        //XNameContainer
        virtual void SAL_CALL insertByName(const OUString& Name, const uno::Any& Element) throw( lang::IllegalArgumentException, container::ElementExistException, lang::WrappedTargetException, uno::RuntimeException, std::exception ) override;
        virtual void SAL_CALL replaceByName(const OUString& Name, const uno::Any& Element) throw( lang::IllegalArgumentException, container::NoSuchElementException, lang::WrappedTargetException, uno::RuntimeException, std::exception ) override;
        virtual void SAL_CALL removeByName(const OUString& Name) throw( container::NoSuchElementException, lang::WrappedTargetException, uno::RuntimeException, std::exception ) override;

        //XPropertySet
        virtual uno::Reference< beans::XPropertySetInfo > SAL_CALL getPropertySetInfo(  ) throw (uno::RuntimeException, std::exception) override
            { return {}; };
        virtual void SAL_CALL setPropertyValue( const OUString&, const uno::Any&) throw (beans::UnknownPropertyException, beans::PropertyVetoException, lang::IllegalArgumentException, lang::WrappedTargetException, uno::RuntimeException, std::exception) override
            { SAL_WARN("sw.uno", "###unexpected!"); };
        virtual uno::Any SAL_CALL getPropertyValue( const OUString& PropertyName ) throw (beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL addPropertyChangeListener( const OUString&, const uno::Reference<beans::XPropertyChangeListener>&) throw (beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException, std::exception) override
            { SAL_WARN("sw.uno", "###unexpected!"); };
        virtual void SAL_CALL removePropertyChangeListener( const OUString&, const uno::Reference<beans::XPropertyChangeListener>&) throw (beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException, std::exception) override
            { SAL_WARN("sw.uno", "###unexpected!"); };
        virtual void SAL_CALL addVetoableChangeListener(const OUString&, const uno::Reference<beans::XVetoableChangeListener>&) throw (beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException, std::exception) override
            { SAL_WARN("sw.uno", "###unexpected!"); };
        virtual void SAL_CALL removeVetoableChangeListener(const OUString&, const uno::Reference<beans::XVetoableChangeListener>&) throw (beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException, std::exception) override
            { SAL_WARN("sw.uno", "###unexpected!"); };

        //SfxListener
        virtual void Notify(SfxBroadcaster& rBC, const SfxHint& rHint) override
        {
            const SfxSimpleHint *pHint = dynamic_cast<const SfxSimpleHint*>( &rHint );
            if(pHint && (pHint->GetId() & SFX_HINT_DYING))
            {
                m_pBasePool = nullptr;
                m_pDocShell = nullptr;
                EndListening(rBC);
            }
        }

        //XServiceInfo
        virtual OUString SAL_CALL getImplementationName() throw( uno::RuntimeException, std::exception ) override
            { return {"XStyleFamily"}; };
        virtual sal_Bool SAL_CALL supportsService(const OUString& rServiceName) throw( uno::RuntimeException, std::exception ) override
            { return cppu::supportsService(this, rServiceName); };
        virtual uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() throw( uno::RuntimeException, std::exception ) override
            { return { "com.sun.star.style.StyleFamily" }; }
    };

}

class SwStyleBase_Impl;
class SwXStyle : public cppu::WeakImplHelper
    <
        css::style::XStyle,
        css::beans::XPropertySet,
        css::beans::XMultiPropertySet,
        css::lang::XServiceInfo,
        css::lang::XUnoTunnel,
        css::beans::XPropertyState,
        css::beans::XMultiPropertyStates
    >
    , public SfxListener
    , public SwClient
{
    SwDoc* m_pDoc;
    OUString m_sStyleName;
    const StyleFamilyEntry& m_rEntry;
    bool m_bIsDescriptor;
    bool m_bIsConditional;
    OUString m_sParentStyleName;

protected:
    SfxStyleSheetBasePool* m_pBasePool;
    std::unique_ptr<SwStyleProperties_Impl> m_pPropertiesImpl;
    css::uno::Reference<css::container::XNameAccess> m_xStyleFamily;
    css::uno::Reference<css::beans::XPropertySet> m_xStyleData;

    template<sal_uInt16>
    void SetPropertyValue(const SfxItemPropertySimpleEntry&, const SfxItemPropertySet&, const uno::Any&, SwStyleBase_Impl&);
    void SetPropertyValues_Impl( const css::uno::Sequence< OUString >& aPropertyNames, const css::uno::Sequence< css::uno::Any >& aValues );
    SfxStyleSheetBase* GetStyleSheetBase();
    void PrepareStyleBase(SwStyleBase_Impl& rBase);
    template<sal_uInt16>
    uno::Any GetStyleProperty(const SfxItemPropertySimpleEntry& rEntry, const SfxItemPropertySet& rPropSet, SwStyleBase_Impl& rBase);
    uno::Any GetStyleProperty_Impl(const SfxItemPropertySimpleEntry& rEntry, const SfxItemPropertySet& rPropSet, SwStyleBase_Impl& rBase);
    uno::Any GetPropertyValue_Impl(const SfxItemPropertySet* pPropSet, SwStyleBase_Impl& rBase, const OUString& rPropertyName);

   virtual void Modify( const SfxPoolItem* pOld, const SfxPoolItem *pNew) override;
public:
    SwXStyle(SwDoc* pDoc, SfxStyleFamily eFam = SfxStyleFamily::Para, bool bConditional = false);
    SwXStyle(SfxStyleSheetBasePool* pPool, SfxStyleFamily eFamily, SwDoc* pDoc, const OUString& rStyleName);
    virtual ~SwXStyle();


    static const css::uno::Sequence< sal_Int8 > & getUnoTunnelId();

    //XUnoTunnel
    virtual sal_Int64 SAL_CALL getSomething( const css::uno::Sequence< sal_Int8 >& aIdentifier ) throw(css::uno::RuntimeException, std::exception) override;

    //XNamed
    virtual OUString SAL_CALL getName() throw( css::uno::RuntimeException, std::exception ) override;
    virtual void SAL_CALL setName(const OUString& Name_) throw( css::uno::RuntimeException, std::exception ) override;

    //XStyle
    virtual sal_Bool SAL_CALL isUserDefined() throw( css::uno::RuntimeException, std::exception ) override;
    virtual sal_Bool SAL_CALL isInUse() throw( css::uno::RuntimeException, std::exception ) override;
    virtual OUString SAL_CALL getParentStyle() throw( css::uno::RuntimeException, std::exception ) override;
    virtual void SAL_CALL setParentStyle(const OUString& aParentStyle) throw( css::container::NoSuchElementException, css::uno::RuntimeException, std::exception ) override;

    //XPropertySet
    virtual css::uno::Reference< css::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo(  ) throw(css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setPropertyValue( const OUString& aPropertyName, const css::uno::Any& aValue ) throw(css::beans::UnknownPropertyException, css::beans::PropertyVetoException, css::lang::IllegalArgumentException, css::lang::WrappedTargetException, css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Any SAL_CALL getPropertyValue( const OUString& PropertyName ) throw(css::beans::UnknownPropertyException, css::lang::WrappedTargetException, css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL addPropertyChangeListener( const OUString&, const css::uno::Reference< css::beans::XPropertyChangeListener >& ) throw(css::beans::UnknownPropertyException, css::lang::WrappedTargetException, css::uno::RuntimeException, std::exception) override
            { OSL_FAIL("not implemented"); };
    virtual void SAL_CALL removePropertyChangeListener( const OUString&, const css::uno::Reference< css::beans::XPropertyChangeListener >& ) throw(css::beans::UnknownPropertyException, css::lang::WrappedTargetException, css::uno::RuntimeException, std::exception) override
            { OSL_FAIL("not implemented"); };
    virtual void SAL_CALL addVetoableChangeListener( const OUString&, const css::uno::Reference< css::beans::XVetoableChangeListener >& ) throw(css::beans::UnknownPropertyException, css::lang::WrappedTargetException, css::uno::RuntimeException, std::exception) override
            { OSL_FAIL("not implemented"); };
    virtual void SAL_CALL removeVetoableChangeListener( const OUString&, const css::uno::Reference< css::beans::XVetoableChangeListener >& ) throw(css::beans::UnknownPropertyException, css::lang::WrappedTargetException, css::uno::RuntimeException, std::exception) override
            { OSL_FAIL("not implemented"); };

    //XMultiPropertySet
    virtual void SAL_CALL setPropertyValues( const css::uno::Sequence< OUString >& aPropertyNames, const css::uno::Sequence< css::uno::Any >& aValues ) throw(css::beans::PropertyVetoException, css::lang::IllegalArgumentException, css::lang::WrappedTargetException, css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Sequence< css::uno::Any > SAL_CALL getPropertyValues( const css::uno::Sequence< OUString >& aPropertyNames ) throw(css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL addPropertiesChangeListener( const css::uno::Sequence< OUString >&, const css::uno::Reference< css::beans::XPropertiesChangeListener >& ) throw(css::uno::RuntimeException, std::exception) override
            {};
    virtual void SAL_CALL removePropertiesChangeListener( const css::uno::Reference< css::beans::XPropertiesChangeListener >& ) throw(css::uno::RuntimeException, std::exception) override
            {};
    virtual void SAL_CALL firePropertiesChangeEvent( const css::uno::Sequence< OUString >&, const css::uno::Reference< css::beans::XPropertiesChangeListener >& ) throw(css::uno::RuntimeException, std::exception) override
            {};

    //XPropertyState
    virtual css::beans::PropertyState SAL_CALL getPropertyState( const OUString& PropertyName ) throw(css::beans::UnknownPropertyException, css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Sequence< css::beans::PropertyState > SAL_CALL getPropertyStates( const css::uno::Sequence< OUString >& aPropertyName ) throw(css::beans::UnknownPropertyException, css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setPropertyToDefault( const OUString& PropertyName ) throw(css::beans::UnknownPropertyException, css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Any SAL_CALL getPropertyDefault( const OUString& aPropertyName ) throw(css::beans::UnknownPropertyException, css::lang::WrappedTargetException, css::uno::RuntimeException, std::exception) override;

    //XMultiPropertyStates
    virtual void SAL_CALL setAllPropertiesToDefault(  ) throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setPropertiesToDefault( const css::uno::Sequence< OUString >& aPropertyNames ) throw (css::beans::UnknownPropertyException, css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Sequence< css::uno::Any > SAL_CALL getPropertyDefaults( const css::uno::Sequence< OUString >& aPropertyNames ) throw (css::beans::UnknownPropertyException, css::lang::WrappedTargetException, css::uno::RuntimeException, std::exception) override;

    //XServiceInfo
    virtual OUString SAL_CALL getImplementationName() throw( css::uno::RuntimeException, std::exception ) override
            { return {"SwXStyle"}; };
    virtual sal_Bool SAL_CALL supportsService(const OUString& rServiceName) throw( css::uno::RuntimeException, std::exception ) override
            { return cppu::supportsService(this, rServiceName); };
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() throw( css::uno::RuntimeException, std::exception ) override;

    //SfxListener
    virtual void        Notify( SfxBroadcaster& rBC, const SfxHint& rHint ) override;
    const OUString&     GetStyleName() const { return m_sStyleName;}
    SfxStyleFamily      GetFamily() const {return m_rEntry.m_eFamily;}

    bool                IsDescriptor() const {return m_bIsDescriptor;}
    bool                IsConditional() const { return m_bIsConditional;}
    const OUString&     GetParentStyleName() const { return m_sParentStyleName;}
    void                SetDoc(SwDoc* pDc, SfxStyleSheetBasePool*   pPool)
                            {
                                m_bIsDescriptor = false; m_pDoc = pDc;
                                m_pBasePool = pPool;
                                StartListening(*m_pBasePool);
                            }
    SwDoc*                GetDoc() const { return m_pDoc; }
    void Invalidate();
    void ApplyDescriptorProperties();
    void SetStyleName(const OUString& rSet){ m_sStyleName = rSet;}
    void SetStyleProperty(const SfxItemPropertySimpleEntry& rEntry, const SfxItemPropertySet& rPropSet, const uno::Any& rValue, SwStyleBase_Impl& rBase) throw(beans::PropertyVetoException, lang::IllegalArgumentException, lang::WrappedTargetException, uno::RuntimeException, std::exception);
    void PutItemToSet(const SvxSetItem* pSetItem, const SfxItemPropertySet& rPropSet, const SfxItemPropertySimpleEntry& rEntry, const uno::Any& rVal, SwStyleBase_Impl& rBaseImpl);
};

class SwXFrameStyle
    : public SwXStyle
    , public css::document::XEventsSupplier
    , public sw::ICoreFrameStyle
{
public:
    SwXFrameStyle(SfxStyleSheetBasePool& rPool,
                                SwDoc*  pDoc,
                                const OUString& rStyleName) :
        SwXStyle(&rPool, SfxStyleFamily::Frame, pDoc, rStyleName){}
    explicit SwXFrameStyle(SwDoc *pDoc);
    virtual ~SwXFrameStyle();

    virtual void SAL_CALL acquire(  ) throw() override {SwXStyle::acquire();}
    virtual void SAL_CALL release(  ) throw() override {SwXStyle::release();}

    virtual css::uno::Sequence< css::uno::Type > SAL_CALL getTypes(  ) throw(css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Any SAL_CALL queryInterface( const css::uno::Type& aType ) throw(css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Reference< css::container::XNameReplace > SAL_CALL getEvents(  ) throw(css::uno::RuntimeException, std::exception) override;

    //ICoreStyle
    virtual void SetItem(enum RES_FRMATR eAtr, const SfxPoolItem& rItem) override;
    virtual const SfxPoolItem* GetItem(enum RES_FRMATR eAtr) override;
    virtual css::document::XEventsSupplier& GetEventsSupplier() override
            { return *this; };
};

class SwXPageStyle
    : public SwXStyle
{
protected:
    void SetPropertyValues_Impl( const css::uno::Sequence< OUString >& aPropertyNames, const css::uno::Sequence< css::uno::Any >& aValues );
    css::uno::Sequence< css::uno::Any > GetPropertyValues_Impl( const css::uno::Sequence< OUString >& aPropertyNames );

public:
    SwXPageStyle(SfxStyleSheetBasePool& rPool, SwDocShell* pDocSh, SfxStyleFamily eFam,
                                const OUString& rStyleName);
    explicit SwXPageStyle(SwDocShell* pDocSh);
    virtual ~SwXPageStyle();

    virtual void SAL_CALL setPropertyValue( const OUString& aPropertyName, const css::uno::Any& aValue ) throw(css::beans::UnknownPropertyException, css::beans::PropertyVetoException, css::lang::IllegalArgumentException, css::lang::WrappedTargetException, css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Any SAL_CALL getPropertyValue( const OUString& PropertyName ) throw(css::beans::UnknownPropertyException, css::lang::WrappedTargetException, css::uno::RuntimeException, std::exception) override;

    virtual void SAL_CALL setPropertyValues( const css::uno::Sequence< OUString >& aPropertyNames, const css::uno::Sequence< css::uno::Any >& aValues ) throw(css::beans::PropertyVetoException, css::lang::IllegalArgumentException, css::lang::WrappedTargetException, css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Sequence< css::uno::Any > SAL_CALL getPropertyValues( const css::uno::Sequence< OUString >& aPropertyNames ) throw(css::uno::RuntimeException, std::exception) override;
};


using sw::XStyleFamily;

OUString SwXStyleFamilies::getImplementationName() throw( uno::RuntimeException, std::exception )
    { return {"SwXStyleFamilies"}; }

sal_Bool SwXStyleFamilies::supportsService(const OUString& rServiceName) throw( uno::RuntimeException, std::exception )
{
    return cppu::supportsService(this, rServiceName);
}

uno::Sequence< OUString > SwXStyleFamilies::getSupportedServiceNames() throw( uno::RuntimeException, std::exception )
    { return { "com.sun.star.style.StyleFamilies" }; }

SwXStyleFamilies::SwXStyleFamilies(SwDocShell& rDocShell) :
        SwUnoCollection(rDocShell.GetDoc()),
        m_pDocShell(&rDocShell)
    { }

SwXStyleFamilies::~SwXStyleFamilies()
    { }

uno::Any SAL_CALL SwXStyleFamilies::getByName(const OUString& Name)
    throw(
        container::NoSuchElementException,
        lang::WrappedTargetException,
        uno::RuntimeException, std::exception )
{
    SolarMutexGuard aGuard;
    if(!IsValid())
        throw uno::RuntimeException();
    auto pEntries(lcl_GetStyleFamilyEntries());
    const auto pEntry = std::find_if(pEntries->begin(), pEntries->end(),
        [&Name] (const StyleFamilyEntry& e) { return e.m_sName == Name; });
    if(pEntry == pEntries->end())
        throw container::NoSuchElementException();
    return getByIndex(pEntry-pEntries->begin());
}

uno::Sequence< OUString > SwXStyleFamilies::getElementNames() throw( uno::RuntimeException, std::exception )
{
    auto pEntries(lcl_GetStyleFamilyEntries());
    uno::Sequence<OUString> aNames(pEntries->size());
    std::transform(pEntries->begin(), pEntries->end(),
            aNames.begin(), [] (const StyleFamilyEntry& e) { return e.m_sName; });
    return aNames;
}

sal_Bool SwXStyleFamilies::hasByName(const OUString& Name) throw( uno::RuntimeException, std::exception )
{
    auto pEntries(lcl_GetStyleFamilyEntries());
    const auto pEntry = std::find_if(pEntries->begin(), pEntries->end(),
        [&Name] (const StyleFamilyEntry& e) { return e.m_sName == Name; });
    return pEntry != pEntries->end();
}

sal_Int32 SwXStyleFamilies::getCount() throw( uno::RuntimeException, std::exception )
{
    return lcl_GetStyleFamilyEntries()->size();
}

uno::Any SwXStyleFamilies::getByIndex(sal_Int32 nIndex)
    throw( lang::IndexOutOfBoundsException, lang::WrappedTargetException, uno::RuntimeException, std::exception )
{
    auto pEntries(lcl_GetStyleFamilyEntries());
    SolarMutexGuard aGuard;
    if(nIndex < 0 || nIndex >= static_cast<sal_Int32>(pEntries->size()))
        throw lang::IndexOutOfBoundsException();
    if(!IsValid())
        throw uno::RuntimeException();
    auto eFamily = (*pEntries)[nIndex].m_eFamily;
    assert(eFamily != SfxStyleFamily::All);
    auto& rxFamily = m_vFamilies[eFamily];
    if(!rxFamily.is())
        rxFamily = new XStyleFamily(m_pDocShell, eFamily);
    return uno::makeAny(rxFamily);
}

uno::Type SwXStyleFamilies::getElementType()
    throw( uno::RuntimeException, std::exception )
{
    return cppu::UnoType<container::XNameContainer>::get();
}

sal_Bool SwXStyleFamilies::hasElements() throw( uno::RuntimeException, std::exception )
    { return true; }

void SwXStyleFamilies::loadStylesFromURL(const OUString& rURL,
    const uno::Sequence< beans::PropertyValue >& aOptions)
    throw (io::IOException, uno::RuntimeException,
           std::exception)
{
    SolarMutexGuard aGuard;
    if(!IsValid() || rURL.isEmpty())
        throw uno::RuntimeException();
    SwgReaderOption aOpt;
    aOpt.SetFrameFormats(true);
    aOpt.SetTextFormats(true);
    aOpt.SetPageDescs(true);
    aOpt.SetNumRules(true);
    aOpt.SetMerge(false);
    for(const auto& rProperty: aOptions)
    {
        if(rProperty.Value.getValueType() != cppu::UnoType<bool>::get())
            continue;
        const bool bValue = rProperty.Value.get<bool>();
        if(rProperty.Name == UNO_NAME_OVERWRITE_STYLES)
            aOpt.SetMerge(!bValue);
        else if(rProperty.Name == UNO_NAME_LOAD_NUMBERING_STYLES)
            aOpt.SetNumRules(bValue);
        else if(rProperty.Name == UNO_NAME_LOAD_PAGE_STYLES)
            aOpt.SetPageDescs(bValue);
        else if(rProperty.Name == UNO_NAME_LOAD_FRAME_STYLES)
            aOpt.SetFrameFormats(bValue);
        else if(rProperty.Name == UNO_NAME_LOAD_TEXT_STYLES)
            aOpt.SetTextFormats(bValue);
    }
    const sal_uLong nErr = m_pDocShell->LoadStylesFromFile( rURL, aOpt, true );
    if(nErr)
        throw io::IOException();
}

uno::Sequence< beans::PropertyValue > SwXStyleFamilies::getStyleLoaderOptions()
        throw( uno::RuntimeException, std::exception )
{
    SolarMutexGuard aGuard;
    uno::Sequence< beans::PropertyValue > aSeq(5);
    beans::PropertyValue* pArray = aSeq.getArray();
    const uno::Any aVal(true);
    pArray[0] = beans::PropertyValue(UNO_NAME_LOAD_TEXT_STYLES, -1, aVal, beans::PropertyState_DIRECT_VALUE);
    pArray[1] = beans::PropertyValue(UNO_NAME_LOAD_FRAME_STYLES, -1, aVal, beans::PropertyState_DIRECT_VALUE);
    pArray[2] = beans::PropertyValue(UNO_NAME_LOAD_PAGE_STYLES, -1, aVal, beans::PropertyState_DIRECT_VALUE);
    pArray[3] = beans::PropertyValue(UNO_NAME_LOAD_NUMBERING_STYLES, -1, aVal, beans::PropertyState_DIRECT_VALUE);
    pArray[4] = beans::PropertyValue(UNO_NAME_OVERWRITE_STYLES, -1, aVal, beans::PropertyState_DIRECT_VALUE);
    return aSeq;
}

static bool lcl_GetHeaderFooterItem(
        SfxItemSet const& rSet, OUString const& rPropName, bool const bFooter,
        SvxSetItem const*& o_rpItem)
{
    SfxItemState eState = rSet.GetItemState(
        (bFooter) ? SID_ATTR_PAGE_FOOTERSET : SID_ATTR_PAGE_HEADERSET,
        false, reinterpret_cast<const SfxPoolItem**>(&o_rpItem));
    if (SfxItemState::SET != eState &&
        rPropName == UNO_NAME_FIRST_IS_SHARED)
    {   // fdo#79269 header may not exist, check footer then
        eState = rSet.GetItemState(
            (!bFooter) ? SID_ATTR_PAGE_FOOTERSET : SID_ATTR_PAGE_HEADERSET,
            false, reinterpret_cast<const SfxPoolItem**>(&o_rpItem));
    }
    return SfxItemState::SET == eState;
}

template<enum SfxStyleFamily>
static sal_Int32 lcl_GetCountOrName(const SwDoc&, OUString*, sal_Int32);

template<>
sal_Int32 lcl_GetCountOrName<SfxStyleFamily::Char>(const SwDoc& rDoc, OUString* pString, sal_Int32 nIndex)
{
    const sal_uInt16 nBaseCount = nPoolChrHtmlRange + nPoolChrNormalRange;
    nIndex -= nBaseCount;
    sal_Int32 nCount = 0;
    for(auto pFormat : *rDoc.GetCharFormats())
    {
        if(pFormat->IsDefault() && pFormat != rDoc.GetDfltCharFormat())
            continue;
        if(!IsPoolUserFormat(pFormat->GetPoolFormatId()))
            continue;
        if(nIndex == nCount)
        {
            // the default character format needs to be set to "Default!"
            if(rDoc.GetDfltCharFormat() == pFormat)
                SwStyleNameMapper::FillUIName(RES_POOLCOLL_STANDARD, *pString);
            else
                *pString = pFormat->GetName();
            break;
        }
        ++nCount;
    }
    return nCount + nBaseCount;
}

template<>
sal_Int32 lcl_GetCountOrName<SfxStyleFamily::Para>(const SwDoc& rDoc, OUString* pString, sal_Int32 nIndex)
{
    const sal_uInt16 nBaseCount = nPoolCollHtmlStackedStart + nPoolCollHtmlRange;
    nIndex -= nBaseCount;
    sal_Int32 nCount = 0;
    for(auto pColl : *rDoc.GetTextFormatColls())
    {
        if(pColl->IsDefault())
            continue;
        if(!IsPoolUserFormat(pColl->GetPoolFormatId()))
            continue;
        if(nIndex == nCount)
        {
            *pString = pColl->GetName();
            break;
        }
        ++nCount;
    }
    return nCount + nBaseCount;
}

template<>
sal_Int32 lcl_GetCountOrName<SfxStyleFamily::Frame>(const SwDoc& rDoc, OUString* pString, sal_Int32 nIndex)
{
    nIndex -= nPoolFrameRange;
    sal_Int32 nCount = 0;
    for(const auto pFormat : *rDoc.GetFrameFormats())
    {
        if(pFormat->IsDefault() || pFormat->IsAuto())
            continue;
        if(!IsPoolUserFormat(pFormat->GetPoolFormatId()))
            continue;
        if(nIndex == nCount)
        {
            *pString = pFormat->GetName();
            break;
        }
        ++nCount;
    }
    return nCount + nPoolFrameRange;
}

template<>
sal_Int32 lcl_GetCountOrName<SfxStyleFamily::Page>(const SwDoc& rDoc, OUString* pString, sal_Int32 nIndex)
{
    nIndex -= nPoolPageRange;
    sal_Int32 nCount = 0;
    const size_t nArrLen = rDoc.GetPageDescCnt();
    for(size_t i = 0; i < nArrLen; ++i)
    {
        const SwPageDesc& rDesc = rDoc.GetPageDesc(i);
        if(!IsPoolUserFormat(rDesc.GetPoolFormatId()))
            continue;
        if(nIndex == nCount)
        {
            *pString = rDesc.GetName();
            break;
        }
        ++nCount;
    }
    nCount += nPoolPageRange;
    return nCount;
}

template<>
sal_Int32 lcl_GetCountOrName<SfxStyleFamily::Pseudo>(const SwDoc& rDoc, OUString* pString, sal_Int32 nIndex)
{
    nIndex -= nPoolNumRange;
    sal_Int32 nCount = 0;
    for(const auto pRule : rDoc.GetNumRuleTable())
    {
        if(pRule->IsAutoRule())
            continue;
        if(!IsPoolUserFormat(pRule->GetPoolFormatId()))
            continue;
        if(nIndex == nCount)
        {
            *pString = pRule->GetName();
            break;
        }
        ++nCount;
    }
    return nCount + nPoolNumRange;
}

template<SfxStyleFamily eFamily>
static uno::Reference< css::style::XStyle> lcl_CreateStyle(SfxStyleSheetBasePool* pBasePool, SwDocShell* pDocShell, const OUString& sStyleName)
    { return pBasePool ? new SwXStyle(pBasePool, eFamily, pDocShell->GetDoc(), sStyleName) : new SwXStyle(pDocShell->GetDoc(), eFamily, false); };

template<>
uno::Reference< css::style::XStyle> lcl_CreateStyle<SfxStyleFamily::Para>(SfxStyleSheetBasePool* pBasePool, SwDocShell* pDocShell, const OUString& sStyleName)
    { return pBasePool ? new SwXStyle(pBasePool, SfxStyleFamily::Para, pDocShell->GetDoc(), sStyleName) : new SwXStyle(pDocShell->GetDoc(), SfxStyleFamily::Para, false); };
template<>
uno::Reference< css::style::XStyle> lcl_CreateStyle<SfxStyleFamily::Frame>(SfxStyleSheetBasePool* pBasePool, SwDocShell* pDocShell, const OUString& sStyleName)
    { return pBasePool ? new SwXFrameStyle(*pBasePool, pDocShell->GetDoc(), sStyleName) : new SwXFrameStyle(pDocShell->GetDoc()); };

template<>
uno::Reference< css::style::XStyle> lcl_CreateStyle<SfxStyleFamily::Page>(SfxStyleSheetBasePool* pBasePool, SwDocShell* pDocShell, const OUString& sStyleName)
    { return pBasePool ? new SwXPageStyle(*pBasePool, pDocShell, SfxStyleFamily::Page, sStyleName) : new SwXPageStyle(pDocShell); };

uno::Reference<css::style::XStyle> SwXStyleFamilies::CreateStyle(SfxStyleFamily eFamily, SwDoc& rDoc)
{
    auto pEntries(lcl_GetStyleFamilyEntries());
    const auto pEntry = std::find_if(pEntries->begin(), pEntries->end(),
            [eFamily] (const StyleFamilyEntry& e) { return e.m_eFamily == eFamily; });
    return pEntry == pEntries->end() ? nullptr : pEntry->m_fCreateStyle(nullptr, rDoc.GetDocShell(), "");
}

// FIXME: Ugly special casing that should die.
uno::Reference<css::style::XStyle> SwXStyleFamilies::CreateStyleCondParagraph(SwDoc& rDoc)
    { return new SwXStyle(&rDoc, SfxStyleFamily::Para, true); };

template<enum SfxStyleFamily>
static sal_uInt16 lcl_TranslateIndex(const sal_uInt16 nIndex);

template<>
sal_uInt16 lcl_TranslateIndex<SfxStyleFamily::Char>(const sal_uInt16 nIndex)
{
    static_assert(nPoolChrNormalRange > 0 && nPoolChrHtmlRange > 0, "invalid pool range");
    if(nIndex < nPoolChrNormalRange)
        return nIndex + RES_POOLCHR_NORMAL_BEGIN;
    else if(nIndex < (nPoolChrHtmlRange+nPoolChrNormalRange))
        return nIndex + RES_POOLCHR_HTML_BEGIN - nPoolChrNormalRange;
    throw lang::IndexOutOfBoundsException();
}

template<>
sal_uInt16 lcl_TranslateIndex<SfxStyleFamily::Para>(const sal_uInt16 nIndex)
{
    static_assert(nPoolCollTextRange > 0 && nPoolCollListsRange > 0 && nPoolCollExtraRange > 0 && nPoolCollRegisterRange > 0 && nPoolCollDocRange > 0 && nPoolCollHtmlRange > 0, "weird pool range");
    if(nIndex < nPoolCollListsStackedStart)
        return nIndex + RES_POOLCOLL_TEXT_BEGIN;
    else if(nIndex < nPoolCollExtraStackedStart)
        return nIndex + RES_POOLCOLL_LISTS_BEGIN - nPoolCollListsStackedStart;
    else if(nIndex < nPoolCollRegisterStackedStart)
        return nIndex + RES_POOLCOLL_EXTRA_BEGIN - nPoolCollExtraStackedStart;
    else if(nIndex < nPoolCollDocStackedStart)
        return nIndex + RES_POOLCOLL_REGISTER_BEGIN - nPoolCollRegisterStackedStart;
    else if(nIndex < nPoolCollHtmlStackedStart)
        return nIndex + RES_POOLCOLL_DOC_BEGIN - nPoolCollDocStackedStart;
    else if(nIndex < nPoolCollHtmlStackedStart + nPoolCollTextRange)
        return nIndex + RES_POOLCOLL_HTML_BEGIN - nPoolCollHtmlStackedStart;
    throw lang::IndexOutOfBoundsException();
}

template<sal_uInt16 nRangeBegin, sal_uInt16 nRangeSize>
static sal_uInt16 lcl_TranslateIndexRange(const sal_uInt16 nIndex)
{
    if(nIndex < nRangeSize)
        return nIndex + nRangeBegin;
    throw lang::IndexOutOfBoundsException();
}

uno::Any XStyleFamily::getByIndex(sal_Int32 nIndex)
    throw( lang::IndexOutOfBoundsException, lang::WrappedTargetException, uno::RuntimeException, std::exception )
{
    SolarMutexGuard aGuard;
    if(nIndex < 0)
        throw lang::IndexOutOfBoundsException();
    if(!m_pBasePool)
        throw uno::RuntimeException();
    OUString sStyleName;
    try
    {
        SwStyleNameMapper::FillUIName(m_rEntry.m_fTranslateIndex(nIndex), sStyleName);
    } catch(...) {}
    if (sStyleName.isEmpty())
        GetCountOrName(&sStyleName, nIndex);
    if(sStyleName.isEmpty())
        throw lang::IndexOutOfBoundsException();
    return getByName(sStyleName);
}

uno::Any XStyleFamily::getByName(const OUString& rName)
    throw( container::NoSuchElementException, lang::WrappedTargetException, uno::RuntimeException, std::exception )
{
    SolarMutexGuard aGuard;
    OUString sStyleName;
    SwStyleNameMapper::FillUIName(rName, sStyleName, m_rEntry.m_aPoolId, true);
    if(!m_pBasePool)
        throw uno::RuntimeException();
    m_pBasePool->SetSearchMask(m_rEntry.m_eFamily);
    SfxStyleSheetBase* pBase = m_pBasePool->Find(sStyleName);
    if(!pBase)
        throw container::NoSuchElementException();
    uno::Reference<style::XStyle> xStyle = FindStyle(sStyleName);
    if(!xStyle.is())
        xStyle = m_rEntry.m_fCreateStyle(m_pBasePool, m_pDocShell, m_rEntry.m_eFamily == SfxStyleFamily::Frame ? pBase->GetName() : sStyleName);
    return uno::makeAny(xStyle);
}

uno::Sequence<OUString> XStyleFamily::getElementNames() throw( uno::RuntimeException, std::exception )
{
    SolarMutexGuard aGuard;
    if(!m_pBasePool)
        throw uno::RuntimeException();
    std::vector<OUString> vRet;
    SfxStyleSheetIteratorPtr pIt = m_pBasePool->CreateIterator(m_rEntry.m_eFamily, SFXSTYLEBIT_ALL);
    for (SfxStyleSheetBase* pStyle = pIt->First(); pStyle; pStyle = pIt->Next())
    {
        OUString sName;
        SwStyleNameMapper::FillProgName(pStyle->GetName(), sName, m_rEntry.m_aPoolId, true);
        vRet.push_back(sName);
    }
    return comphelper::containerToSequence(vRet);
}

sal_Bool XStyleFamily::hasByName(const OUString& rName) throw( uno::RuntimeException, std::exception )
{
    SolarMutexGuard aGuard;
    if(!m_pBasePool)
        throw uno::RuntimeException();
    OUString sStyleName;
    SwStyleNameMapper::FillUIName(rName, sStyleName, m_rEntry.m_aPoolId, true);
    m_pBasePool->SetSearchMask(m_rEntry.m_eFamily);
    SfxStyleSheetBase* pBase = m_pBasePool->Find(sStyleName);
    return nullptr != pBase;
}


void XStyleFamily::insertByName(const OUString& rName, const uno::Any& rElement)
        throw( lang::IllegalArgumentException, container::ElementExistException, lang::WrappedTargetException, uno::RuntimeException, std::exception )
{
    SolarMutexGuard aGuard;
    if(!m_pBasePool)
        throw uno::RuntimeException();
    OUString sStyleName;
    SwStyleNameMapper::FillUIName(rName, sStyleName, m_rEntry.m_aPoolId, true);
    m_pBasePool->SetSearchMask(m_rEntry.m_eFamily);
    SfxStyleSheetBase* pBase = m_pBasePool->Find(sStyleName);
    SfxStyleSheetBase* pUINameBase = m_pBasePool->Find( sStyleName );
    if(pBase || pUINameBase)
        throw container::ElementExistException();
    if(rElement.getValueType().getTypeClass() != uno::TypeClass_INTERFACE)
        throw lang::IllegalArgumentException();
    uno::Reference<lang::XUnoTunnel> xStyleTunnel = rElement.get<uno::Reference<lang::XUnoTunnel>>();
    SwXStyle* pNewStyle = nullptr;
    if(xStyleTunnel.is())
    {
        pNewStyle = reinterpret_cast< SwXStyle * >(
                sal::static_int_cast< sal_IntPtr >( xStyleTunnel->getSomething( SwXStyle::getUnoTunnelId()) ));
    }

    if (!pNewStyle || !pNewStyle->IsDescriptor() || pNewStyle->GetFamily() != m_rEntry.m_eFamily)
        throw lang::IllegalArgumentException();

    sal_uInt16 nMask = SFXSTYLEBIT_ALL;
    if(m_rEntry.m_eFamily == SfxStyleFamily::Para && !pNewStyle->IsConditional())
        nMask &= ~SWSTYLEBIT_CONDCOLL;
    m_pBasePool->Make(sStyleName, m_rEntry.m_eFamily, nMask);
    pNewStyle->SetDoc(m_pDocShell->GetDoc(), m_pBasePool);
    pNewStyle->SetStyleName(sStyleName);
    const OUString sParentStyleName(pNewStyle->GetParentStyleName());
    if (!sParentStyleName.isEmpty())
    {
        m_pBasePool->SetSearchMask(m_rEntry.m_eFamily);
        SfxStyleSheetBase* pParentBase = m_pBasePool->Find(sParentStyleName);
        if(pParentBase && pParentBase->GetFamily() == m_rEntry.m_eFamily &&
            &pParentBase->GetPool() == m_pBasePool)
            m_pBasePool->SetParent(m_rEntry.m_eFamily, sStyleName, sParentStyleName);
    }
    // after all, we still need to apply the properties of the descriptor
    pNewStyle->ApplyDescriptorProperties();
}

void XStyleFamily::replaceByName(const OUString& rName, const uno::Any& rElement)
    throw( lang::IllegalArgumentException, container::NoSuchElementException, lang::WrappedTargetException, uno::RuntimeException, std::exception )
{
    SolarMutexGuard aGuard;
    if(!m_pBasePool)
        throw uno::RuntimeException();
    m_pBasePool->SetSearchMask(m_rEntry.m_eFamily);
    SfxStyleSheetBase* pBase = m_pBasePool->Find(rName);
    // replacements only for userdefined styles
    if(!pBase)
        throw container::NoSuchElementException();
    if(!pBase->IsUserDefined())
        throw lang::IllegalArgumentException();
    //if theres an object available to this style then it must be invalidated
    uno::Reference<style::XStyle> xStyle = FindStyle(pBase->GetName());
    if(xStyle.is())
    {
        uno::Reference<lang::XUnoTunnel> xTunnel( xStyle, uno::UNO_QUERY);
        if(xTunnel.is())
        {
            SwXStyle* pStyle = reinterpret_cast< SwXStyle * >(
                    sal::static_int_cast< sal_IntPtr >( xTunnel->getSomething( SwXStyle::getUnoTunnelId()) ));
            pStyle->Invalidate();
        }
    }
    m_pBasePool->Remove(pBase);
    insertByName(rName, rElement);
}

void XStyleFamily::removeByName(const OUString& rName) throw( container::NoSuchElementException, lang::WrappedTargetException, uno::RuntimeException, std::exception )
{
    SolarMutexGuard aGuard;
    if(!m_pBasePool)
        throw uno::RuntimeException();
    m_pBasePool->SetSearchMask(m_rEntry.m_eFamily);
    OUString sName;
    SwStyleNameMapper::FillUIName(rName, sName, m_rEntry.m_aPoolId, true);
    SfxStyleSheetBase* pBase = m_pBasePool->Find( sName );
    if(!pBase)
        throw container::NoSuchElementException();
    m_pBasePool->Remove(pBase);
}

uno::Any SAL_CALL XStyleFamily::getPropertyValue( const OUString& sPropertyName ) throw (beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException, std::exception)
{
    if(sPropertyName != "DisplayName")
        throw beans::UnknownPropertyException( "unknown property: " + sPropertyName, static_cast<OWeakObject *>(this) );
    SolarMutexGuard aGuard;
    return uno::makeAny(SW_RESSTR(m_rEntry.m_nResId));
}


SwXStyle* XStyleFamily::FindStyle(const OUString& rStyleName) const
{
    const size_t nLCount = m_pBasePool->GetSizeOfVector();
    for(size_t i = 0; i < nLCount; ++i)
    {
        SfxListener* pListener = m_pBasePool->GetListener(i);
        SwXStyle* pTempStyle = dynamic_cast<SwXStyle*>(pListener);
        if(pTempStyle && pTempStyle->GetFamily() == m_rEntry.m_eFamily && pTempStyle->GetStyleName() == rStyleName)
            return pTempStyle;
    }
    return nullptr;
}

static const std::vector<StyleFamilyEntry>* lcl_GetStyleFamilyEntries()
{
    if(!our_pStyleFamilyEntries)
    {
        our_pStyleFamilyEntries = new std::vector<StyleFamilyEntry>{
            { SfxStyleFamily::Char,   PROPERTY_MAP_CHAR_STYLE,  nsSwGetPoolIdFromName::GET_POOLID_CHRFMT,   "CharacterStyles", STR_STYLE_FAMILY_CHARACTER, &lcl_GetCountOrName<SfxStyleFamily::Char>,   &lcl_CreateStyle<SfxStyleFamily::Char>,   &lcl_TranslateIndex<SfxStyleFamily::Char>                       },
            { SfxStyleFamily::Para,   PROPERTY_MAP_PARA_STYLE,  nsSwGetPoolIdFromName::GET_POOLID_TXTCOLL,  "ParagraphStyles", STR_STYLE_FAMILY_PARAGRAPH, &lcl_GetCountOrName<SfxStyleFamily::Para>,   &lcl_CreateStyle<SfxStyleFamily::Para>,   &lcl_TranslateIndex<SfxStyleFamily::Para>                       },
            { SfxStyleFamily::Page,   PROPERTY_MAP_PAGE_STYLE,  nsSwGetPoolIdFromName::GET_POOLID_PAGEDESC, "PageStyles",      STR_STYLE_FAMILY_PAGE,      &lcl_GetCountOrName<SfxStyleFamily::Page>,   &lcl_CreateStyle<SfxStyleFamily::Page>,   &lcl_TranslateIndexRange<RES_POOLPAGE_BEGIN,    nPoolPageRange>  },
            { SfxStyleFamily::Frame,  PROPERTY_MAP_FRAME_STYLE, nsSwGetPoolIdFromName::GET_POOLID_FRMFMT,   "FrameStyles",     STR_STYLE_FAMILY_FRAME,     &lcl_GetCountOrName<SfxStyleFamily::Frame>,  &lcl_CreateStyle<SfxStyleFamily::Frame>,  &lcl_TranslateIndexRange<RES_POOLFRM_BEGIN,     nPoolFrameRange> },
            { SfxStyleFamily::Pseudo, PROPERTY_MAP_NUM_STYLE,   nsSwGetPoolIdFromName::GET_POOLID_NUMRULE,  "NumberingStyles", STR_STYLE_FAMILY_NUMBERING, &lcl_GetCountOrName<SfxStyleFamily::Pseudo>, &lcl_CreateStyle<SfxStyleFamily::Pseudo>, &lcl_TranslateIndexRange<RES_POOLNUMRULE_BEGIN, nPoolNumRange>   }
       };
    }
    return our_pStyleFamilyEntries;
}

static const std::vector<ParagraphStyleCategoryEntry>* lcl_GetParagraphStyleCategoryEntries()
{
    if(!our_pParagraphStyleCategoryEntries)
    {
        our_pParagraphStyleCategoryEntries = new std::vector<ParagraphStyleCategoryEntry>{
            { style::ParagraphStyleCategory::TEXT,    SWSTYLEBIT_TEXT,    COLL_TEXT_BITS     },
            { style::ParagraphStyleCategory::CHAPTER, SWSTYLEBIT_CHAPTER, COLL_DOC_BITS      },
            { style::ParagraphStyleCategory::LIST,    SWSTYLEBIT_LIST,    COLL_LISTS_BITS    },
            { style::ParagraphStyleCategory::INDEX,   SWSTYLEBIT_IDX,     COLL_REGISTER_BITS },
            { style::ParagraphStyleCategory::EXTRA,   SWSTYLEBIT_EXTRA,   COLL_EXTRA_BITS    },
            { style::ParagraphStyleCategory::HTML,    SWSTYLEBIT_HTML,    COLL_HTML_BITS     }
        };
    }
    return our_pParagraphStyleCategoryEntries;
}

class SwStyleProperties_Impl
{
    const PropertyEntryVector_t aPropertyEntries;
    std::map<OUString, uno::Any> m_vPropertyValues;
public:
    explicit SwStyleProperties_Impl(const SfxItemPropertyMap& rMap)
        : aPropertyEntries(rMap.getPropertyEntries())
    { }
    ~SwStyleProperties_Impl()
    { }

    bool AllowsKey(const OUString& rName)
    {
        return aPropertyEntries.end() != std::find_if(aPropertyEntries.begin(), aPropertyEntries.end(),
            [rName] (const SfxItemPropertyNamedEntry& rEntry) {return rName == rEntry.sName;} );
    }
    bool SetProperty(const OUString& rName, const uno::Any& rValue)
    {
        if(!AllowsKey(rName))
            return false;
        m_vPropertyValues[rName] = rValue;
        return true;
    }
    bool GetProperty(const OUString& rName, const uno::Any*& pAny)
    {
        if(!AllowsKey(rName))
        {
            pAny = nullptr;
            return false;
        }
        pAny = &m_vPropertyValues[rName];
        return true;
    }
    bool ClearProperty( const OUString& rName )
    {
        if(!AllowsKey(rName))
            return false;
        m_vPropertyValues[rName] = uno::Any();
        return true;
    }
    void ClearAllProperties( )
            { m_vPropertyValues.clear(); }
    void Apply(SwXStyle& rStyle)
    {
        for(const auto& rPropertyPair : m_vPropertyValues)
        {
            if(rPropertyPair.second.hasValue())
                rStyle.setPropertyValue(rPropertyPair.first, rPropertyPair.second);
        }
    }
    static void GetProperty(const OUString &rPropertyName, const uno::Reference < beans::XPropertySet > &rxPropertySet, uno::Any& rAny )
    {
        rAny = rxPropertySet->getPropertyValue( rPropertyName );
    }
};

static SwGetPoolIdFromName lcl_GetSwEnumFromSfxEnum(SfxStyleFamily eFamily)
{
    auto pEntries(lcl_GetStyleFamilyEntries());
    const auto pEntry = std::find_if(pEntries->begin(), pEntries->end(),
            [eFamily] (const StyleFamilyEntry& e) { return e.m_eFamily == eFamily; });
    if(pEntry != pEntries->end())
        return pEntry->m_aPoolId;
    SAL_WARN("sw.uno", "someone asking for all styles in unostyle.cxx!" );
    return nsSwGetPoolIdFromName::GET_POOLID_CHRFMT;
}

namespace
{
    class theSwXStyleUnoTunnelId : public rtl::Static<UnoTunnelIdInit, theSwXStyleUnoTunnelId> {};
    class theCoreParagraphUnoTunnelId : public rtl::Static<UnoTunnelIdInit, theCoreParagraphUnoTunnelId> {};
}

const uno::Sequence<sal_Int8>& SwXStyle::getUnoTunnelId()
{
    return theSwXStyleUnoTunnelId::get().getSeq();
}

const uno::Sequence<sal_Int8>& sw::ICoreParagraphStyle::getUnoTunnelId()
{
    return theCoreParagraphUnoTunnelId::get().getSeq();
}

sal_Int64 SAL_CALL SwXStyle::getSomething(const uno::Sequence<sal_Int8>& rId)
    throw(uno::RuntimeException, std::exception)
{
    if(rId.getLength() != 16)
        return 0;
    if(0 == memcmp(getUnoTunnelId().getConstArray(), rId.getConstArray(), 16))
    {
        return sal::static_int_cast<sal_Int64>(reinterpret_cast<sal_IntPtr>(this));
    }
    else if(0 == memcmp(sw::ICoreParagraphStyle::getUnoTunnelId().getConstArray(), rId.getConstArray(), 16))
    {
        return sal::static_int_cast<sal_Int64>(reinterpret_cast<sal_IntPtr>(dynamic_cast<sw::ICoreParagraphStyle*>(this)));
    }
    return 0;
}


uno::Sequence< OUString > SwXStyle::getSupportedServiceNames() throw( uno::RuntimeException, std::exception )
{
    long nCount = 1;
    if(SfxStyleFamily::Para == m_rEntry.m_eFamily)
    {
        nCount = 5;
        if(m_bIsConditional)
            nCount++;
    }
    else if(SfxStyleFamily::Char == m_rEntry.m_eFamily)
        nCount = 5;
    else if(SfxStyleFamily::Page == m_rEntry.m_eFamily)
        nCount = 3;
    uno::Sequence< OUString > aRet(nCount);
    OUString* pArray = aRet.getArray();
    pArray[0] = "com.sun.star.style.Style";
    switch(m_rEntry.m_eFamily)
    {
        case SfxStyleFamily::Char:
            pArray[1] = "com.sun.star.style.CharacterStyle";
            pArray[2] = "com.sun.star.style.CharacterProperties";
            pArray[3] = "com.sun.star.style.CharacterPropertiesAsian";
            pArray[4] = "com.sun.star.style.CharacterPropertiesComplex";
        break;
        case SfxStyleFamily::Page:
            pArray[1] = "com.sun.star.style.PageStyle";
            pArray[2] = "com.sun.star.style.PageProperties";
        break;
        case SfxStyleFamily::Para:
            pArray[1] = "com.sun.star.style.ParagraphStyle";
            pArray[2] = "com.sun.star.style.ParagraphProperties";
            pArray[3] = "com.sun.star.style.ParagraphPropertiesAsian";
            pArray[4] = "com.sun.star.style.ParagraphPropertiesComplex";
        if(m_bIsConditional)
            pArray[5] = "com.sun.star.style.ConditionalParagraphStyle";
        break;

        default:
            ;
    }
    return aRet;
}

static uno::Reference<beans::XPropertySet> lcl_InitStandardStyle(const SfxStyleFamily eFamily,  uno::Reference<container::XNameAccess>& rxStyleFamily)
{
    using return_t = decltype(lcl_InitStandardStyle(eFamily, rxStyleFamily));
    if(eFamily != SfxStyleFamily::Para && eFamily != SfxStyleFamily::Page)
        return {};
    auto aResult(rxStyleFamily->getByName("Standard"));
    if(!aResult.has<return_t>())
        return {};
    return aResult.get<return_t>();
}

static uno::Reference<container::XNameAccess> lcl_InitStyleFamily(SwDoc* pDoc, const StyleFamilyEntry& rEntry)
{
    using return_t = decltype(lcl_InitStyleFamily(pDoc, rEntry));
    if(rEntry.m_eFamily != SfxStyleFamily::Char
            && rEntry.m_eFamily != SfxStyleFamily::Para
            && rEntry.m_eFamily != SfxStyleFamily::Page)
        return {};
    auto xModel(pDoc->GetDocShell()->GetBaseModel());
    uno::Reference<style::XStyleFamiliesSupplier> xFamilySupplier(xModel, uno::UNO_QUERY);
    auto xFamilies = xFamilySupplier->getStyleFamilies();
    auto aResult(xFamilies->getByName(rEntry.m_sName));
    if(!aResult.has<return_t>())
        return {};
    return aResult.get<return_t>();
}

static bool lcl_InitConditional(SfxStyleSheetBasePool* pBasePool, const SfxStyleFamily eFamily, const OUString& rStyleName)
{
    if(!pBasePool || eFamily != SfxStyleFamily::Para)
        return false;
    pBasePool->SetSearchMask(eFamily);
    SfxStyleSheetBase* pBase = pBasePool->Find(rStyleName);
    SAL_WARN_IF(!pBase, "sw.uno", "where is the style?" );
    if(!pBase)
        return false;
    const sal_uInt16 nId(SwStyleNameMapper::GetPoolIdFromUIName(rStyleName, nsSwGetPoolIdFromName::GET_POOLID_TXTCOLL));
    if(nId != USHRT_MAX)
        return ::IsConditionalByPoolId(nId);
    return RES_CONDTXTFMTCOLL == static_cast<SwDocStyleSheet*>(pBase)->GetCollection()->Which();
}

static const StyleFamilyEntry& lcl_GetStyleEntry(const SfxStyleFamily eFamily)
{
    auto pEntries = lcl_GetStyleFamilyEntries();
    const auto pEntry = std::find_if(pEntries->begin(), pEntries->end(),
            [eFamily] (const StyleFamilyEntry& e) { return e.m_eFamily == eFamily; });
    assert(pEntry != pEntries->end());
    return *pEntry;
}

SwXStyle::SwXStyle(SwDoc* pDoc, SfxStyleFamily eFamily, bool bConditional)
    : m_pDoc(pDoc)
    , m_rEntry(lcl_GetStyleEntry(eFamily))
    , m_bIsDescriptor(true)
    , m_bIsConditional(bConditional)
    , m_pBasePool(nullptr)
    , m_xStyleFamily(lcl_InitStyleFamily(pDoc, m_rEntry))
    , m_xStyleData(lcl_InitStandardStyle(eFamily, m_xStyleFamily))
{
    assert(!m_bIsConditional || m_rEntry.m_eFamily == SfxStyleFamily::Para); // only paragraph styles are conditional
    // Register ourselves as a listener to the document (via the page descriptor)
    pDoc->getIDocumentStylePoolAccess().GetPageDescFromPool(RES_POOLPAGE_STANDARD)->Add(this);
    m_pPropertiesImpl = std::unique_ptr<SwStyleProperties_Impl>(
            new SwStyleProperties_Impl(aSwMapProvider.GetPropertySet(m_bIsConditional ? PROPERTY_MAP_CONDITIONAL_PARA_STYLE :  m_rEntry.m_nPropMapType)->getPropertyMap()));
}

SwXStyle::SwXStyle(SfxStyleSheetBasePool* pPool, SfxStyleFamily eFamily, SwDoc* pDoc, const OUString& rStyleName)
    : m_pDoc(pDoc)
    , m_sStyleName(rStyleName)
    , m_rEntry(lcl_GetStyleEntry(eFamily))
    , m_bIsDescriptor(false)
    , m_bIsConditional(lcl_InitConditional(pPool, eFamily, rStyleName))
    , m_pBasePool(pPool)
    , m_pPropertiesImpl(nullptr)
{ }

SwXStyle::~SwXStyle()
{
    SolarMutexGuard aGuard;
    if(m_pBasePool)
        EndListening(*m_pBasePool);
    m_pPropertiesImpl.reset();
    if(GetRegisteredIn())
        GetRegisteredIn()->Remove( this );
}

void SwXStyle::Modify( const SfxPoolItem* pOld, const SfxPoolItem *pNew)
{
    ClientModify(this, pOld, pNew);
    if(!GetRegisteredIn())
    {
        m_pDoc = nullptr;
        m_xStyleData.clear();
        m_xStyleFamily.clear();
    }
}

OUString SwXStyle::getName() throw( uno::RuntimeException, std::exception )
{
    SolarMutexGuard aGuard;
    if(!m_pBasePool)
        return m_sStyleName;
    m_pBasePool->SetSearchMask(m_rEntry.m_eFamily);
    SfxStyleSheetBase* pBase = m_pBasePool->Find(m_sStyleName);
    SAL_WARN_IF(!pBase, "sw.uno", "where is the style?");
    if(!pBase)
        throw uno::RuntimeException();
    OUString aString;
    SwStyleNameMapper::FillProgName(pBase->GetName(), aString, lcl_GetSwEnumFromSfxEnum ( m_rEntry.m_eFamily ), true);
    return aString;
}

void SwXStyle::setName(const OUString& rName) throw( uno::RuntimeException, std::exception )
{
    SolarMutexGuard aGuard;
    if(!m_pBasePool)
    {
        m_sStyleName = rName;
        return;
    }
    m_pBasePool->SetSearchMask(m_rEntry.m_eFamily);
    SfxStyleSheetBase* pBase = m_pBasePool->Find(m_sStyleName);
    SAL_WARN_IF(!pBase, "sw.uno", "where is the style?");
    if(!pBase || !pBase->IsUserDefined())
        throw uno::RuntimeException();
    rtl::Reference<SwDocStyleSheet> xTmp(new SwDocStyleSheet(*static_cast<SwDocStyleSheet*>(pBase)));
    if(!xTmp->SetName(rName))
        throw uno::RuntimeException();
    m_sStyleName = rName;
}

sal_Bool SwXStyle::isUserDefined() throw( uno::RuntimeException, std::exception )
{
    SolarMutexGuard aGuard;
    if(!m_pBasePool)
        throw uno::RuntimeException();
    m_pBasePool->SetSearchMask(m_rEntry.m_eFamily);
    SfxStyleSheetBase* pBase = m_pBasePool->Find(m_sStyleName);
    //if it is not found it must be non user defined
    return pBase && pBase->IsUserDefined();
}

sal_Bool SwXStyle::isInUse() throw( uno::RuntimeException, std::exception )
{
    SolarMutexGuard aGuard;
    if(!m_pBasePool)
        throw uno::RuntimeException();
    m_pBasePool->SetSearchMask(m_rEntry.m_eFamily, SFXSTYLEBIT_USED);
    SfxStyleSheetBase* pBase = m_pBasePool->Find(m_sStyleName);
    return pBase && pBase->IsUsed();
}

OUString SwXStyle::getParentStyle() throw( uno::RuntimeException, std::exception )
{
    SolarMutexGuard aGuard;
    if(!m_pBasePool)
    {
        if(!m_bIsDescriptor)
            throw uno::RuntimeException();
        return m_sParentStyleName;
    }
    m_pBasePool->SetSearchMask(m_rEntry.m_eFamily);
    SfxStyleSheetBase* pBase = m_pBasePool->Find(m_sStyleName);
    OUString aString;
    if(pBase)
        aString = pBase->GetParent();
    SwStyleNameMapper::FillProgName(aString, aString, lcl_GetSwEnumFromSfxEnum(m_rEntry.m_eFamily), true);
    return aString;
}

void SwXStyle::setParentStyle(const OUString& rParentStyle) throw( container::NoSuchElementException, uno::RuntimeException, std::exception )
{
    SolarMutexGuard aGuard;
    OUString sParentStyle;
    SwStyleNameMapper::FillUIName(rParentStyle, sParentStyle, lcl_GetSwEnumFromSfxEnum ( m_rEntry.m_eFamily ), true );
    if(!m_pBasePool)
    {
        if(!m_bIsDescriptor)
            throw uno::RuntimeException();
        m_sParentStyleName = sParentStyle;
        try
        {
            const auto aAny = m_xStyleFamily->getByName(sParentStyle);
            m_xStyleData = aAny.get<decltype(m_xStyleData)>();
        }
        catch(...)
        { }
        return;
    }
    m_pBasePool->SetSearchMask(m_rEntry.m_eFamily);
    SfxStyleSheetBase* pBase = m_pBasePool->Find(m_sStyleName);
    if(!pBase)
        throw uno::RuntimeException();
    rtl::Reference<SwDocStyleSheet> xBase(new SwDocStyleSheet(*static_cast<SwDocStyleSheet*>(pBase)));
    //make it a 'real' style - necessary for pooled styles
    xBase->GetItemSet();
    if(xBase->GetParent() != sParentStyle)
    {
        if(!xBase->SetParent(sParentStyle))
            throw uno::RuntimeException();
    }
}

uno::Reference<beans::XPropertySetInfo> SwXStyle::getPropertySetInfo() throw( uno::RuntimeException, std::exception )
{
    if(m_bIsConditional)
    {
        assert(m_rEntry.m_eFamily == SfxStyleFamily::Para);
        static uno::Reference<beans::XPropertySetInfo> xCondParaRef;
        xCondParaRef = aSwMapProvider.GetPropertySet(PROPERTY_MAP_CONDITIONAL_PARA_STYLE)->getPropertySetInfo();
        return xCondParaRef;
    }
    return m_rEntry.m_xPSInfo;
}

void SwXStyle::ApplyDescriptorProperties()
{
    m_bIsDescriptor = false;
    m_xStyleData.clear();
    m_xStyleFamily.clear();
    m_pPropertiesImpl->Apply(*this);
}

class SwStyleBase_Impl
{
private:
    SwDoc& m_rDoc;
    const SwPageDesc* m_pOldPageDesc;
    rtl::Reference<SwDocStyleSheet> m_xNewBase;
    SfxItemSet* m_pItemSet;
    std::unique_ptr<SfxItemSet> m_pMyItemSet;
    OUString m_rStyleName;
    const SwAttrSet* m_pParentStyle;
public:
    SwStyleBase_Impl(SwDoc& rSwDoc, const OUString& rName, const SwAttrSet* pParentStyle)
        : m_rDoc(rSwDoc)
        , m_pOldPageDesc(nullptr)
        , m_pItemSet(nullptr)
        , m_pMyItemSet(nullptr)
        , m_rStyleName(rName)
        , m_pParentStyle(pParentStyle)
    { }

    rtl::Reference<SwDocStyleSheet>& getNewBase()
    {
        return m_xNewBase;
    }

    void setNewBase(SwDocStyleSheet* pNew)
    {
        m_xNewBase = pNew;
    }

    bool HasItemSet() const
    {
        return m_xNewBase.is();
    }

    SfxItemSet& GetItemSet()
    {
        assert(m_xNewBase.is());
        if(!m_pItemSet)
        {
            m_pMyItemSet.reset(new SfxItemSet(m_xNewBase->GetItemSet()));
            m_pItemSet = m_pMyItemSet.get();

            //UUUU set parent style to have the correct XFillStyle setting as XFILL_NONE
            if(!m_pItemSet->GetParent() && m_pParentStyle)
                m_pItemSet->SetParent(m_pParentStyle);
        }
        return *m_pItemSet;
    }

    const SwPageDesc* GetOldPageDesc();

    // still a hack, but a bit more explicit and with a proper scope
    struct ItemSetOverrider
    {
        SwStyleBase_Impl& m_rStyleBase;
        SfxItemSet* m_pOldSet;
        ItemSetOverrider(SwStyleBase_Impl& rStyleBase, SfxItemSet* pTemp)
                : m_rStyleBase(rStyleBase)
                , m_pOldSet(m_rStyleBase.m_pItemSet)
        { m_rStyleBase.m_pItemSet = pTemp; }
        ~ItemSetOverrider()
        { m_rStyleBase.m_pItemSet = m_pOldSet; };
    };
};

const SwPageDesc* SwStyleBase_Impl::GetOldPageDesc()
{
    if(!m_pOldPageDesc)
    {
        SwPageDesc *pd = m_rDoc.FindPageDesc(m_rStyleName);
        if(pd)
            m_pOldPageDesc = pd;

        if(!m_pOldPageDesc)
        {
            for(sal_uInt16 i = RC_POOLPAGEDESC_BEGIN; i <= STR_POOLPAGE_LANDSCAPE; ++i)
            {
                if(SW_RESSTR(i) == m_rStyleName)
                {
                    m_pOldPageDesc = m_rDoc.getIDocumentStylePoolAccess().GetPageDescFromPool( static_cast<sal_uInt16>(RES_POOLPAGE_BEGIN + i - RC_POOLPAGEDESC_BEGIN));
                    break;
                }
            }
        }
    }
    return m_pOldPageDesc;
}



sal_uInt8 lcl_TranslateMetric(const SfxItemPropertySimpleEntry& rEntry, SwDoc* pDoc, uno::Any& o_aValue)
{
    // check for needed metric translation
    if(!(rEntry.nMemberId & SFX_METRIC_ITEM))
        return rEntry.nMemberId;
    // exception: If these ItemTypes are used, do not convert when these are negative
    // since this means they are intended as percent values
    if((XATTR_FILLBMP_SIZEX == rEntry.nWID || XATTR_FILLBMP_SIZEY == rEntry.nWID)
            && o_aValue.has<sal_Int32>()
            && o_aValue.get<sal_Int32>() < 0)
        return rEntry.nMemberId;
    if(!pDoc)
        return rEntry.nMemberId & (~SFX_METRIC_ITEM);

    const SfxItemPool& rPool = pDoc->GetAttrPool();
    const SfxMapUnit eMapUnit(rPool.GetMetric(rEntry.nWID));
    if(eMapUnit != SFX_MAPUNIT_100TH_MM)
        SvxUnoConvertFromMM(eMapUnit, o_aValue);
    return rEntry.nMemberId & (~SFX_METRIC_ITEM);
}
template<>
void SwXStyle::SetPropertyValue<HINT_BEGIN>(const SfxItemPropertySimpleEntry& rEntry, const SfxItemPropertySet& rPropSet, const uno::Any& rValue, SwStyleBase_Impl& o_rStyleBase)
{
    // default ItemSet handling
    SfxItemSet& rStyleSet = o_rStyleBase.GetItemSet();
    SfxItemSet aSet(*rStyleSet.GetPool(), rEntry.nWID, rEntry.nWID);
    aSet.SetParent(&rStyleSet);
    rPropSet.setPropertyValue(rEntry, rValue, aSet);
    rStyleSet.Put(aSet);
}
template<>
void SwXStyle::SetPropertyValue<FN_UNO_HIDDEN>(const SfxItemPropertySimpleEntry& rEntry, const SfxItemPropertySet& rPropSet, const uno::Any& rValue, SwStyleBase_Impl& o_rStyleBase)
{
    bool bHidden = false;
    if(rValue >>= bHidden)
    {
        //make it a 'real' style - necessary for pooled styles
        o_rStyleBase.getNewBase()->GetItemSet();
        o_rStyleBase.getNewBase()->SetHidden(bHidden);
    }
    SetPropertyValue<HINT_BEGIN>(rEntry, rPropSet, rValue, o_rStyleBase);
}
template<>
void SwXStyle::SetPropertyValue<FN_UNO_STYLE_INTEROP_GRAB_BAG>(const SfxItemPropertySimpleEntry& rEntry, const SfxItemPropertySet& rPropSet, const uno::Any& rValue, SwStyleBase_Impl& o_rStyleBase)
{
    o_rStyleBase.getNewBase()->GetItemSet();
    o_rStyleBase.getNewBase()->SetGrabBagItem(rValue);
    SetPropertyValue<HINT_BEGIN>(rEntry, rPropSet, rValue, o_rStyleBase);
}
template<>
void SwXStyle::SetPropertyValue<XATTR_FILLGRADIENT>(const SfxItemPropertySimpleEntry& rEntry, const SfxItemPropertySet& rPropSet, const uno::Any& rValue, SwStyleBase_Impl& o_rStyleBase)
{
    uno::Any aValue(rValue);
    const auto nMemberId(lcl_TranslateMetric(rEntry, m_pDoc, aValue));
    if(MID_NAME == nMemberId)
    {
        // add set commands for FillName items
        SfxItemSet& rStyleSet = o_rStyleBase.GetItemSet();
        if(!aValue.has<OUString>())
            throw lang::IllegalArgumentException();
        SvxShape::SetFillAttribute(rEntry.nWID, aValue.get<OUString>(), rStyleSet);
    }
    else if(MID_GRAFURL == nMemberId)
    {
        if(XATTR_FILLBITMAP == rEntry.nWID)
        {
            // Bitmap also has the MID_GRAFURL mode where a Bitmap URL is used
            const Graphic aNullGraphic;
            SfxItemSet& rStyleSet = o_rStyleBase.GetItemSet();
            XFillBitmapItem aXFillBitmapItem(rStyleSet.GetPool(), aNullGraphic);
            aXFillBitmapItem.PutValue(aValue, nMemberId);
            rStyleSet.Put(aXFillBitmapItem);
        }
    }
    else
        SetPropertyValue<HINT_BEGIN>(rEntry, rPropSet, aValue, o_rStyleBase);
}
template<>
void SwXStyle::SetPropertyValue<RES_BACKGROUND>(const SfxItemPropertySimpleEntry& rEntry, const SfxItemPropertySet&, const uno::Any& rValue, SwStyleBase_Impl& o_rStyleBase)
{
    SfxItemSet& rStyleSet = o_rStyleBase.GetItemSet();
    const SvxBrushItem aOriginalBrushItem(getSvxBrushItemFromSourceSet(rStyleSet, RES_BACKGROUND, true, m_pDoc->IsInXMLImport()));
    SvxBrushItem aChangedBrushItem(aOriginalBrushItem);

    uno::Any aValue(rValue);
    const auto nMemberId(lcl_TranslateMetric(rEntry, m_pDoc, aValue));
    aChangedBrushItem.PutValue(aValue, nMemberId);

    // 0xff is already the default - but if BackTransparent is set
    // to true, it must be applied in the item set on ODF import
    // to potentially override parent style, which is unknown yet
    if(aChangedBrushItem == aOriginalBrushItem && (MID_GRAPHIC_TRANSPARENT != nMemberId || !aValue.has<bool>() || !aValue.get<bool>()))
        return;

    setSvxBrushItemAsFillAttributesToTargetSet(aChangedBrushItem, rStyleSet);
}
template<>
void SwXStyle::SetPropertyValue<OWN_ATTR_FILLBMP_MODE>(const SfxItemPropertySimpleEntry&, const SfxItemPropertySet&, const uno::Any& rValue, SwStyleBase_Impl& o_rStyleBase)
{
    drawing::BitmapMode eMode;
    if(!(rValue >>= eMode))
    {
        if(!rValue.has<sal_Int32>())
            throw lang::IllegalArgumentException();
        eMode = ((drawing::BitmapMode)rValue.get<sal_Int32>());
    }
    SfxItemSet& rStyleSet = o_rStyleBase.GetItemSet();
    rStyleSet.Put(XFillBmpStretchItem(drawing::BitmapMode_STRETCH == eMode));
    rStyleSet.Put(XFillBmpTileItem(drawing::BitmapMode_REPEAT == eMode));
}
template<>
void SwXStyle::SetPropertyValue<RES_PAPER_BIN>(const SfxItemPropertySimpleEntry& rEntry, const SfxItemPropertySet& rPropSet, const uno::Any& rValue, SwStyleBase_Impl& o_rStyleBase)
{
    if(!rValue.has<OUString>())
        throw lang::IllegalArgumentException();
    SfxPrinter* pPrinter = m_pDoc->getIDocumentDeviceAccess().getPrinter(true);
    OUString sValue(rValue.get<OUString>());
    using printeridx_t = decltype(pPrinter->GetPaperBinCount());
    printeridx_t nBin = std::numeric_limits<printeridx_t>::max();
    if(sValue == "[From printer settings]")
        nBin = std::numeric_limits<printeridx_t>::max()-1;
    else if(pPrinter)
    {
        for(sal_uInt16 i=0, nEnd = pPrinter->GetPaperBinCount(); i < nEnd; ++i)
        {
            if (sValue == pPrinter->GetPaperBinName(i))
            {
                nBin = i;
                break;
            }
        }
    }
    if(nBin == std::numeric_limits<printeridx_t>::max())
        throw lang::IllegalArgumentException();
    SfxItemSet& rStyleSet = o_rStyleBase.GetItemSet();
    SfxItemSet aSet(*rStyleSet.GetPool(), rEntry.nWID, rEntry.nWID);
    aSet.SetParent(&rStyleSet);
    rPropSet.setPropertyValue(rEntry, uno::makeAny(static_cast<sal_Int8>(nBin == std::numeric_limits<printeridx_t>::max()-1 ? -1 : nBin)), aSet);
    rStyleSet.Put(aSet);
}
template<>
void SwXStyle::SetPropertyValue<FN_UNO_NUM_RULES>(const SfxItemPropertySimpleEntry&, const SfxItemPropertySet&, const uno::Any& rValue, SwStyleBase_Impl& o_rStyleBase)
{
    if(!rValue.has<uno::Reference<container::XIndexReplace>>() || !rValue.has<uno::Reference<lang::XUnoTunnel>>())
        throw lang::IllegalArgumentException();
    auto xNumberTunnel(rValue.get<uno::Reference<lang::XUnoTunnel>>());
    SwXNumberingRules* pSwXRules = reinterpret_cast<SwXNumberingRules*>(sal::static_int_cast<sal_IntPtr>(xNumberTunnel->getSomething(SwXNumberingRules::getUnoTunnelId())));
    if(!pSwXRules)
        return;
    SwNumRule aSetRule(*pSwXRules->GetNumRule());
    for(sal_uInt16 i = 0; i < MAXLEVEL; ++i)
    {
        const SwNumFormat* pFormat = aSetRule.GetNumFormat(i);
        if(!pFormat)
            continue;
        SwNumFormat aFormat(*pFormat);
        const auto& rCharName(pSwXRules->GetNewCharStyleNames()[i]);
        if(!rCharName.isEmpty()
               && !SwXNumberingRules::isInvalidStyle(rCharName)
               && (!pFormat->GetCharFormat() || pFormat->GetCharFormat()->GetName() != rCharName))
        {
            auto pCharFormatIt(std::find_if(m_pDoc->GetCharFormats()->begin(), m_pDoc->GetCharFormats()->end(),
                    [&rCharName] (SwCharFormat* pF) { return pF->GetName() == rCharName; }));
            if(pCharFormatIt != m_pDoc->GetCharFormats()->end())
                aFormat.SetCharFormat(*pCharFormatIt);
            else if(m_pBasePool)
            {
                auto pBase(static_cast<SfxStyleSheetBasePool*>(m_pBasePool)->Find(rCharName, SfxStyleFamily::Char));
                if(!pBase)
                    pBase = &m_pBasePool->Make(rCharName, SfxStyleFamily::Char);
                aFormat.SetCharFormat(static_cast<SwDocStyleSheet*>(pBase)->GetCharFormat());
            }
            else
                aFormat.SetCharFormat(nullptr);
        }
        // same for fonts:
        const auto& rBulletName(pSwXRules->GetBulletFontNames()[i]);
        if(!rBulletName.isEmpty()
                && !SwXNumberingRules::isInvalidStyle(rBulletName)
                && (!pFormat->GetBulletFont() || pFormat->GetBulletFont()->GetFamilyName() != rBulletName))
        {
            const auto pFontListItem(static_cast<const SvxFontListItem*>(m_pDoc->GetDocShell()->GetItem(SID_ATTR_CHAR_FONTLIST)));
            const auto pList(pFontListItem->GetFontList());
            FontMetric aFontInfo(pList->Get(rBulletName, WEIGHT_NORMAL, ITALIC_NONE));
            vcl::Font aFont(aFontInfo);
            aFormat.SetBulletFont(&aFont);
        }
        aSetRule.Set(i, &aFormat);
    }
    o_rStyleBase.getNewBase()->SetNumRule(aSetRule);
}
template<>
void SwXStyle::SetPropertyValue<RES_PARATR_OUTLINELEVEL>(const SfxItemPropertySimpleEntry&, const SfxItemPropertySet&, const uno::Any& rValue, SwStyleBase_Impl& o_rStyleBase)
{
    if(!rValue.has<sal_Int16>())
        return;
    const auto nLevel(rValue.get<sal_Int16>());
    if(0 <= nLevel && nLevel <= MAXLEVEL)
        o_rStyleBase.getNewBase()->GetCollection()->SetAttrOutlineLevel(nLevel);
}
template<>
void SwXStyle::SetPropertyValue<FN_UNO_FOLLOW_STYLE>(const SfxItemPropertySimpleEntry&, const SfxItemPropertySet&, const uno::Any& rValue, SwStyleBase_Impl& o_rStyleBase)
{
    if(!rValue.has<OUString>())
        return;
    const auto sValue(rValue.get<OUString>());
    OUString aString;
    SwStyleNameMapper::FillUIName(sValue, aString, m_rEntry.m_aPoolId, true);
    o_rStyleBase.getNewBase()->SetFollow(aString);
}
template<>
void SwXStyle::SetPropertyValue<RES_PAGEDESC>(const SfxItemPropertySimpleEntry& rEntry, const SfxItemPropertySet& rPropSet, const uno::Any& rValue, SwStyleBase_Impl& o_rStyleBase)
{
    if(MID_PAGEDESC_PAGEDESCNAME != rEntry.nMemberId)
    {
        SetPropertyValue<HINT_BEGIN>(rEntry, rPropSet, rValue, o_rStyleBase);
        return;
    }
    if(!rValue.has<OUString>())
        throw lang::IllegalArgumentException();
    // special handling for RES_PAGEDESC
    SfxItemSet& rStyleSet = o_rStyleBase.GetItemSet();
    std::unique_ptr<SwFormatPageDesc> pNewDesc;
    const SfxPoolItem* pItem;
    if(SfxItemState::SET == rStyleSet.GetItemState(RES_PAGEDESC, true, &pItem))
        pNewDesc.reset(new SwFormatPageDesc(*static_cast<const SwFormatPageDesc*>(pItem)));
    else
        pNewDesc.reset(new SwFormatPageDesc);
    const auto sValue(rValue.get<OUString>());
    OUString sDescName;
    SwStyleNameMapper::FillUIName(sValue, sDescName, nsSwGetPoolIdFromName::GET_POOLID_PAGEDESC, true);
    if(pNewDesc->GetPageDesc() && pNewDesc->GetPageDesc()->GetName() == sDescName)
        return;
    if(sDescName.isEmpty())
    {
        rStyleSet.ClearItem(RES_BREAK);
        rStyleSet.Put(SwFormatPageDesc());
    }
    else
    {
        SwPageDesc* pPageDesc(SwPageDesc::GetByName(*m_pDoc, sDescName));
        if(!pPageDesc)
            throw lang::IllegalArgumentException();
        pNewDesc->RegisterToPageDesc(*pPageDesc);
        rStyleSet.Put(*pNewDesc);
    }
}
template<>
void SwXStyle::SetPropertyValue<RES_TEXT_VERT_ADJUST>(const SfxItemPropertySimpleEntry&, const SfxItemPropertySet&, const uno::Any& rValue, SwStyleBase_Impl& o_rStyleBase)
{
    if(!m_pDoc || !rValue.has<drawing::TextVerticalAdjust>() || !o_rStyleBase.GetOldPageDesc())
        return;
    SwPageDesc* pPageDesc = m_pDoc->FindPageDesc(o_rStyleBase.GetOldPageDesc()->GetName());
    if(pPageDesc)
        pPageDesc->SetVerticalAdjustment(rValue.get<drawing::TextVerticalAdjust>());
}
template<>
void SwXStyle::SetPropertyValue<FN_UNO_IS_AUTO_UPDATE>(const SfxItemPropertySimpleEntry&, const SfxItemPropertySet&, const uno::Any& rValue, SwStyleBase_Impl& o_rStyleBase)
{
    if(!rValue.has<bool>())
        throw lang::IllegalArgumentException();
    const bool bAuto(rValue.get<bool>());
    if(SfxStyleFamily::Para == m_rEntry.m_eFamily)
        o_rStyleBase.getNewBase()->GetCollection()->SetAutoUpdateFormat(bAuto);
    else if(SfxStyleFamily::Frame == m_rEntry.m_eFamily)
        o_rStyleBase.getNewBase()->GetFrameFormat()->SetAutoUpdateFormat(bAuto);
}
template<>
void SwXStyle::SetPropertyValue<FN_UNO_PARA_STYLE_CONDITIONS>(const SfxItemPropertySimpleEntry&, const SfxItemPropertySet&, const uno::Any& rValue, SwStyleBase_Impl& o_rStyleBase)
{
    static_assert(COND_COMMAND_COUNT == 28, "invalid size of command count?");
    using expectedarg_t = uno::Sequence<beans::NamedValue>;
    if(!rValue.has<expectedarg_t>() || !m_pBasePool)
        throw lang::IllegalArgumentException();
    SwCondCollItem aCondItem;
    for(auto& rNamedValue : rValue.get<expectedarg_t>())
    {
        if(!rNamedValue.Value.has<OUString>())
            throw lang::IllegalArgumentException();

        const OUString sValue(rNamedValue.Value.get<OUString>());
        // get UI style name from programmatic style name
        OUString aStyleName;
        SwStyleNameMapper::FillUIName(sValue, aStyleName, lcl_GetSwEnumFromSfxEnum(m_rEntry.m_eFamily), true);

        // check for correct context and style name
        const auto nIdx(GetCommandContextIndex(rNamedValue.Name));
        if (nIdx == -1)
            throw lang::IllegalArgumentException();
        m_pBasePool->SetSearchMask(SfxStyleFamily::Para);
        bool bStyleFound = false;
        for(auto pBase = m_pBasePool->First(); pBase; pBase = m_pBasePool->Next())
        {
            bStyleFound = pBase->GetName() == aStyleName;
            if (bStyleFound)
                break;
        }
        if (!bStyleFound)
            throw lang::IllegalArgumentException();
        aCondItem.SetStyle(&aStyleName, nIdx);
    }
    o_rStyleBase.GetItemSet().Put(aCondItem);
}
template<>
void SwXStyle::SetPropertyValue<FN_UNO_CATEGORY>(const SfxItemPropertySimpleEntry&, const SfxItemPropertySet&, const uno::Any& rValue, SwStyleBase_Impl& o_rStyleBase)
{
    if(!o_rStyleBase.getNewBase()->IsUserDefined() || !rValue.has<paragraphstyle_t>())
        throw lang::IllegalArgumentException();
    static std::unique_ptr<std::map<paragraphstyle_t, swstylebits_t>> pUnoToCore;
    if(!pUnoToCore)
    {
        pUnoToCore.reset(new std::map<paragraphstyle_t, swstylebits_t>());
        auto pEntries = lcl_GetParagraphStyleCategoryEntries();
        std::transform(pEntries->begin(), pEntries->end(), std::inserter(*pUnoToCore, pUnoToCore->end()),
            [] (const ParagraphStyleCategoryEntry& rEntry) { return std::pair<paragraphstyle_t, swstylebits_t>(rEntry.m_eCategory, rEntry.m_nSwStyleBits); });
    }
    const auto pUnoToCoreIt(pUnoToCore->find(rValue.get<paragraphstyle_t>()));
    if(pUnoToCoreIt == pUnoToCore->end())
        throw lang::IllegalArgumentException();
    o_rStyleBase.getNewBase()->SetMask( pUnoToCoreIt->second|SFXSTYLEBIT_USERDEF );
}
template<>
void SwXStyle::SetPropertyValue<SID_SWREGISTER_COLLECTION>(const SfxItemPropertySimpleEntry&, const SfxItemPropertySet&, const uno::Any& rValue, SwStyleBase_Impl& o_rStyleBase)
{
    OUString sName;
    rValue >>= sName;
    SwRegisterItem aReg(!sName.isEmpty());
    aReg.SetWhich(SID_SWREGISTER_MODE);
    o_rStyleBase.GetItemSet().Put(aReg);
    OUString aString;
    SwStyleNameMapper::FillUIName(sName, aString, nsSwGetPoolIdFromName::GET_POOLID_TXTCOLL, true);
    o_rStyleBase.GetItemSet().Put(SfxStringItem(SID_SWREGISTER_COLLECTION, aString ) );
}
template<>
void SwXStyle::SetPropertyValue<RES_TXTATR_CJK_RUBY>(const SfxItemPropertySimpleEntry& rEntry, const SfxItemPropertySet& rPropSet, const uno::Any& rValue, SwStyleBase_Impl& o_rStyleBase)
{
    if(MID_RUBY_CHARSTYLE != rEntry.nMemberId)
        return;
    if(!rValue.has<OUString>())
        throw lang::IllegalArgumentException();
    const auto sValue(rValue.get<OUString>());
    SfxItemSet& rStyleSet(o_rStyleBase.GetItemSet());
    std::unique_ptr<SwFormatRuby> pRuby;
    const SfxPoolItem* pItem;
    if(SfxItemState::SET == rStyleSet.GetItemState(RES_TXTATR_CJK_RUBY, true, &pItem))
        pRuby.reset(new SwFormatRuby(*static_cast<const SwFormatRuby*>(pItem)));
    else
        pRuby.reset(new SwFormatRuby(OUString()));
    OUString sStyle;
    SwStyleNameMapper::FillUIName(sValue, sStyle, nsSwGetPoolIdFromName::GET_POOLID_CHRFMT, true);
    pRuby->SetCharFormatName(sValue);
    pRuby->SetCharFormatId(0);
    if(!sValue.isEmpty())
    {
        const sal_uInt16 nId(SwStyleNameMapper::GetPoolIdFromUIName(sValue, nsSwGetPoolIdFromName::GET_POOLID_CHRFMT));
        pRuby->SetCharFormatId(nId);
    }
    rStyleSet.Put(*pRuby);
    SetPropertyValue<HINT_BEGIN>(rEntry, rPropSet, rValue, o_rStyleBase);
}
template<>
void SwXStyle::SetPropertyValue<RES_PARATR_DROP>(const SfxItemPropertySimpleEntry& rEntry, const SfxItemPropertySet&, const uno::Any& rValue, SwStyleBase_Impl& o_rStyleBase)
{
    if(MID_DROPCAP_CHAR_STYLE_NAME != rEntry.nMemberId)
        return;
    if(!rValue.has<OUString>())
        throw lang::IllegalArgumentException();
    SfxItemSet& rStyleSet(o_rStyleBase.GetItemSet());
    std::unique_ptr<SwFormatDrop> pDrop;
    const SfxPoolItem* pItem;
    if(SfxItemState::SET == rStyleSet.GetItemState(RES_PARATR_DROP, true, &pItem))
        pDrop.reset(new SwFormatDrop(*static_cast<const SwFormatDrop*>(pItem)));
    else
        pDrop.reset(new SwFormatDrop);
    const auto sValue(rValue.get<OUString>());
    OUString sStyle;
    SwStyleNameMapper::FillUIName(sValue, sStyle, nsSwGetPoolIdFromName::GET_POOLID_CHRFMT, true);
    auto pStyle(static_cast<SwDocStyleSheet*>(m_pDoc->GetDocShell()->GetStyleSheetPool()->Find(sStyle, SfxStyleFamily::Char)));
    if(!pStyle)
        throw lang::IllegalArgumentException();
    pDrop->SetCharFormat(pStyle->GetCharFormat());
    rStyleSet.Put(*pDrop);
}
template<>
void SwXStyle::SetPropertyValue<RES_PARATR_NUMRULE>(const SfxItemPropertySimpleEntry& rEntry, const SfxItemPropertySet& rPropSet, const uno::Any& rValue, SwStyleBase_Impl& o_rStyleBase)
{
    uno::Any aValue(rValue);
    lcl_TranslateMetric(rEntry, m_pDoc, aValue);
    SetPropertyValue<HINT_BEGIN>(rEntry, rPropSet, aValue, o_rStyleBase);
    // --> OD 2006-10-18 #i70223#
    if(SfxStyleFamily::Para == m_rEntry.m_eFamily &&
            o_rStyleBase.getNewBase().is() && o_rStyleBase.getNewBase()->GetCollection() &&
            //rBase.getNewBase()->GetCollection()->GetOutlineLevel() < MAXLEVEL /* assigned to list level of outline style */) //#outline level,removed by zhaojianwei
            o_rStyleBase.getNewBase()->GetCollection()->IsAssignedToListLevelOfOutlineStyle())       ////<-end,add by zhaojianwei
    {
        OUString sNewNumberingRuleName;
        aValue >>= sNewNumberingRuleName;
        if(sNewNumberingRuleName.isEmpty() || sNewNumberingRuleName != m_pDoc->GetOutlineNumRule()->GetName())
            o_rStyleBase.getNewBase()->GetCollection()->DeleteAssignmentToListLevelOfOutlineStyle();
    }
}

void SwXStyle::SetStyleProperty(const SfxItemPropertySimpleEntry& rEntry, const SfxItemPropertySet& rPropSet, const uno::Any& rValue, SwStyleBase_Impl& rBase) throw(beans::PropertyVetoException, lang::IllegalArgumentException, lang::WrappedTargetException, uno::RuntimeException, std::exception)
{
    using propertytype_t = decltype(rEntry.nWID);
    using coresetter_t = std::function<void(SwXStyle&, const SfxItemPropertySimpleEntry&, const SfxItemPropertySet&, const uno::Any&, SwStyleBase_Impl&)>;
    static std::unique_ptr<std::map<propertytype_t, coresetter_t>> pUnoToCore;
    if(!pUnoToCore)
    {
        pUnoToCore.reset(new std::map<propertytype_t, coresetter_t> {
            // these explicit std::mem_fn() calls shouldn't be needed, but apparently MSVC is currently too stupid for C++11 again
            { FN_UNO_HIDDEN,                 std::mem_fn(&SwXStyle::SetPropertyValue<FN_UNO_HIDDEN>)                 },
            { FN_UNO_STYLE_INTEROP_GRAB_BAG, std::mem_fn(&SwXStyle::SetPropertyValue<FN_UNO_STYLE_INTEROP_GRAB_BAG>) },
            { XATTR_FILLGRADIENT,            std::mem_fn(&SwXStyle::SetPropertyValue<XATTR_FILLGRADIENT>)            },
            { XATTR_FILLHATCH,               std::mem_fn(&SwXStyle::SetPropertyValue<XATTR_FILLGRADIENT>)            },
            { XATTR_FILLBITMAP,              std::mem_fn(&SwXStyle::SetPropertyValue<XATTR_FILLGRADIENT>)            },
            { XATTR_FILLFLOATTRANSPARENCE,   std::mem_fn(&SwXStyle::SetPropertyValue<XATTR_FILLGRADIENT>)            },
            { RES_BACKGROUND,                std::mem_fn(&SwXStyle::SetPropertyValue<RES_BACKGROUND>)                },
            { OWN_ATTR_FILLBMP_MODE,         std::mem_fn(&SwXStyle::SetPropertyValue<OWN_ATTR_FILLBMP_MODE>)         },
            { RES_PAPER_BIN,                 std::mem_fn(&SwXStyle::SetPropertyValue<RES_PAPER_BIN>)                 },
            { FN_UNO_NUM_RULES,              std::mem_fn(&SwXStyle::SetPropertyValue<FN_UNO_NUM_RULES>)              },
            { RES_PARATR_OUTLINELEVEL,       std::mem_fn(&SwXStyle::SetPropertyValue<RES_PARATR_OUTLINELEVEL>)       },
            { FN_UNO_FOLLOW_STYLE,           std::mem_fn(&SwXStyle::SetPropertyValue<FN_UNO_FOLLOW_STYLE>)           },
            { RES_PAGEDESC,                  std::mem_fn(&SwXStyle::SetPropertyValue<RES_PAGEDESC>)                  },
            { RES_TEXT_VERT_ADJUST,          std::mem_fn(&SwXStyle::SetPropertyValue<RES_TEXT_VERT_ADJUST>)          },
            { FN_UNO_IS_AUTO_UPDATE,         std::mem_fn(&SwXStyle::SetPropertyValue<FN_UNO_IS_AUTO_UPDATE>)         },
            { FN_UNO_PARA_STYLE_CONDITIONS,  std::mem_fn(&SwXStyle::SetPropertyValue<FN_UNO_PARA_STYLE_CONDITIONS>)  },
            { FN_UNO_CATEGORY,               std::mem_fn(&SwXStyle::SetPropertyValue<FN_UNO_CATEGORY>)               },
            { SID_SWREGISTER_COLLECTION,     std::mem_fn(&SwXStyle::SetPropertyValue<SID_SWREGISTER_COLLECTION>)     },
            { RES_TXTATR_CJK_RUBY,           std::mem_fn(&SwXStyle::SetPropertyValue<RES_TXTATR_CJK_RUBY>)           },
            { RES_PARATR_DROP,               std::mem_fn(&SwXStyle::SetPropertyValue<RES_PARATR_DROP>)               },
            { RES_PARATR_DROP,               std::mem_fn(&SwXStyle::SetPropertyValue<RES_PARATR_DROP>)               },
            { RES_PARATR_NUMRULE,            std::mem_fn(&SwXStyle::SetPropertyValue<RES_PARATR_NUMRULE>)            }
        });
    }
    const auto pUnoToCoreIt(pUnoToCore->find(rEntry.nWID));
    if(pUnoToCoreIt != pUnoToCore->end())
        pUnoToCoreIt->second(*this, rEntry, rPropSet, rValue, rBase);
    else
    {
        //UUUU adapted switch logic to a more readable state; removed goto's and made
        // execution of standard setting of proerty in ItemSet dependent of this variable
        uno::Any aValue(rValue);
        lcl_TranslateMetric(rEntry, m_pDoc, aValue);
        SetPropertyValue<HINT_BEGIN>(rEntry, rPropSet, aValue, rBase);
    }
}

void SwXStyle::SetPropertyValues_Impl(const uno::Sequence<OUString>& rPropertyNames, const uno::Sequence<uno::Any>& rValues)
{
    if(!m_pDoc)
        throw uno::RuntimeException();
    const SfxItemPropertySet* pPropSet = aSwMapProvider.GetPropertySet(m_rEntry.m_nPropMapType);
    const SfxItemPropertyMap &rMap = pPropSet->getPropertyMap();
    if(rPropertyNames.getLength() != rValues.getLength())
        throw lang::IllegalArgumentException();

    SwStyleBase_Impl aBaseImpl(*m_pDoc, m_sStyleName, &GetDoc()->GetDfltTextFormatColl()->GetAttrSet()); //UUUU add pDfltTextFormatColl as parent
    if(m_pBasePool)
    {
        const sal_uInt16 nSaveMask = m_pBasePool->GetSearchMask();
        m_pBasePool->SetSearchMask(m_rEntry.m_eFamily);
        SfxStyleSheetBase* pBase = m_pBasePool->Find(m_sStyleName);
        m_pBasePool->SetSearchMask(m_rEntry.m_eFamily, nSaveMask);
        SAL_WARN_IF(!pBase, "sw.uno", "where is the style?");
        if(!pBase)
            throw uno::RuntimeException();
        aBaseImpl.setNewBase(new SwDocStyleSheet(*static_cast<SwDocStyleSheet*>(pBase)));
    }
    if(!aBaseImpl.getNewBase().is() && !m_bIsDescriptor)
        throw uno::RuntimeException();

    const OUString* pNames = rPropertyNames.getConstArray();
    const uno::Any* pValues = rValues.getConstArray();
    for(sal_Int32 nProp = 0; nProp < rPropertyNames.getLength(); ++nProp)
    {
        const SfxItemPropertySimpleEntry* pEntry = rMap.getByName(pNames[nProp]);
        if(!pEntry || (!m_bIsConditional && pNames[nProp] == UNO_NAME_PARA_STYLE_CONDITIONS))
            throw beans::UnknownPropertyException("Unknown property: " + pNames[nProp], static_cast<cppu::OWeakObject*>(this));
        if(pEntry->nFlags & beans::PropertyAttribute::READONLY)
            throw beans::PropertyVetoException ("Property is read-only: " + pNames[nProp], static_cast<cppu::OWeakObject*>(this));
        if(aBaseImpl.getNewBase().is())
            SetStyleProperty(*pEntry, *pPropSet, pValues[nProp], aBaseImpl);
        else if(!m_pPropertiesImpl->SetProperty(pNames[nProp], pValues[nProp]))
            throw lang::IllegalArgumentException();
    }

    if(aBaseImpl.HasItemSet())
        aBaseImpl.getNewBase()->SetItemSet(aBaseImpl.GetItemSet());
}

void SwXStyle::setPropertyValues(const uno::Sequence<OUString>& rPropertyNames, const uno::Sequence<uno::Any>& rValues)
        throw(beans::PropertyVetoException, lang::IllegalArgumentException, lang::WrappedTargetException, uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    // workaround for bad designed API
    try
    {
        SetPropertyValues_Impl( rPropertyNames, rValues );
    }
    catch (const beans::UnknownPropertyException &rException)
    {
        // wrap the original (here not allowed) exception in
        // a lang::WrappedTargetException that gets thrown instead.
        lang::WrappedTargetException aWExc;
        aWExc.TargetException <<= rException;
        throw aWExc;
    }
}

SfxStyleSheetBase* SwXStyle::GetStyleSheetBase()
{
    if(!m_pBasePool)
        return nullptr;
    const sal_uInt16 nSaveMask = m_pBasePool->GetSearchMask();
    m_pBasePool->SetSearchMask(m_rEntry.m_eFamily);
    SfxStyleSheetBase* pBase = m_pBasePool->Find(m_sStyleName);
    m_pBasePool->SetSearchMask(m_rEntry.m_eFamily, nSaveMask );
    return pBase;
}
void SwXStyle::PrepareStyleBase(SwStyleBase_Impl& rBase)
{
    SfxStyleSheetBase* pBase(GetStyleSheetBase());
    if(!pBase)
        throw uno::RuntimeException();
    if(!rBase.getNewBase().is())
        rBase.setNewBase(new SwDocStyleSheet(*static_cast<SwDocStyleSheet*>(pBase)));
}

template<>
uno::Any SwXStyle::GetStyleProperty<HINT_BEGIN>(const SfxItemPropertySimpleEntry& rEntry, const SfxItemPropertySet& rPropSet, SwStyleBase_Impl& rBase);
template<>
uno::Any SwXStyle::GetStyleProperty<FN_UNO_IS_PHYSICAL>(const SfxItemPropertySimpleEntry&, const SfxItemPropertySet&, SwStyleBase_Impl&)
{
    SfxStyleSheetBase* pBase(GetStyleSheetBase());
    if(!pBase)
        return uno::makeAny(false);
    bool bPhys = static_cast<SwDocStyleSheet*>(pBase)->IsPhysical();
    // The standard character format is not existing physically
    if( bPhys && SfxStyleFamily::Char == GetFamily() &&
        static_cast<SwDocStyleSheet*>(pBase)->GetCharFormat() &&
        static_cast<SwDocStyleSheet*>(pBase)->GetCharFormat()->IsDefault() )
        bPhys = false;
    return uno::makeAny<bool>(bPhys);
}
template<>
uno::Any SwXStyle::GetStyleProperty<FN_UNO_HIDDEN>(const SfxItemPropertySimpleEntry&, const SfxItemPropertySet&, SwStyleBase_Impl&)
{
    SfxStyleSheetBase* pBase(GetStyleSheetBase());
    if(!pBase)
        return uno::makeAny(false);
    rtl::Reference<SwDocStyleSheet> xBase(new SwDocStyleSheet(*static_cast<SwDocStyleSheet*>(pBase)));
    return uno::makeAny(xBase->IsHidden());
}
template<>
uno::Any SwXStyle::GetStyleProperty<FN_UNO_STYLE_INTEROP_GRAB_BAG>(const SfxItemPropertySimpleEntry&, const SfxItemPropertySet&, SwStyleBase_Impl&)
{
    SfxStyleSheetBase* pBase(GetStyleSheetBase());
    if(!pBase)
        return uno::Any();
    uno::Any aRet;
    rtl::Reference<SwDocStyleSheet> xBase(new SwDocStyleSheet(*static_cast<SwDocStyleSheet*>(pBase)));
    xBase->GetGrabBagItem(aRet);
    return aRet;
}
template<>
uno::Any SwXStyle::GetStyleProperty<RES_PAPER_BIN>(const SfxItemPropertySimpleEntry& rEntry, const SfxItemPropertySet& rPropSet, SwStyleBase_Impl& rBase)
{
    PrepareStyleBase(rBase);
    SfxItemSet& rSet = rBase.GetItemSet();
    uno::Any aValue;
    rPropSet.getPropertyValue(rEntry, rSet, aValue);
    sal_Int8 nBin(aValue.get<sal_Int8>());
    if(nBin == -1)
        return uno::makeAny<OUString>("[From printer settings]");
    SfxPrinter* pPrinter = GetDoc()->getIDocumentDeviceAccess().getPrinter(false);
    if(!pPrinter)
        return uno::Any();
    return uno::makeAny(pPrinter->GetPaperBinName(nBin));
}
template<>
uno::Any SwXStyle::GetStyleProperty<FN_UNO_NUM_RULES>(const SfxItemPropertySimpleEntry&, const SfxItemPropertySet&, SwStyleBase_Impl& rBase)
{
    PrepareStyleBase(rBase);
    const SwNumRule* pRule = rBase.getNewBase()->GetNumRule();
    assert(pRule && "Where is the NumRule?");
    uno::Reference<container::XIndexReplace> xRules(new SwXNumberingRules(*pRule, GetDoc()));
    return uno::makeAny<uno::Reference<container::XIndexReplace>>(xRules);
}
template<>
uno::Any SwXStyle::GetStyleProperty<RES_PARATR_OUTLINELEVEL>(const SfxItemPropertySimpleEntry&, const SfxItemPropertySet&, SwStyleBase_Impl& rBase)
{
    PrepareStyleBase(rBase);
    SAL_WARN_IF(SfxStyleFamily::Para == GetFamily(), "sw.uno", "only paras");
    return uno::makeAny<sal_Int16>(rBase.getNewBase()->GetCollection()->GetAttrOutlineLevel());
}
template<>
uno::Any SwXStyle::GetStyleProperty<FN_UNO_FOLLOW_STYLE>(const SfxItemPropertySimpleEntry&, const SfxItemPropertySet&, SwStyleBase_Impl& rBase)
{
    PrepareStyleBase(rBase);
    OUString aString;
    SwStyleNameMapper::FillProgName(rBase.getNewBase()->GetFollow(), aString, lcl_GetSwEnumFromSfxEnum(GetFamily()), true);
    return uno::makeAny(aString);
}
template<>
uno::Any SwXStyle::GetStyleProperty<RES_PAGEDESC>(const SfxItemPropertySimpleEntry& rEntry, const SfxItemPropertySet& rPropSet, SwStyleBase_Impl& rBase)
{
    PrepareStyleBase(rBase);
    const sal_uInt8 nMemberId(rEntry.nMemberId & (~SFX_METRIC_ITEM));
    if(MID_PAGEDESC_PAGEDESCNAME != nMemberId)
        return GetStyleProperty<HINT_BEGIN>(rEntry, rPropSet, rBase);
    // special handling for RES_PAGEDESC
    const SfxPoolItem* pItem;
    if(SfxItemState::SET != rBase.GetItemSet().GetItemState(RES_PAGEDESC, true, &pItem))
        return uno::Any();
    const SwPageDesc* pDesc = static_cast<const SwFormatPageDesc*>(pItem)->GetPageDesc();
    if(!pDesc)
        return uno::Any();
    OUString aString;
    SwStyleNameMapper::FillProgName(pDesc->GetName(), aString, nsSwGetPoolIdFromName::GET_POOLID_PAGEDESC, true);
    return uno::makeAny(aString);
}
template<>
uno::Any SwXStyle::GetStyleProperty<FN_UNO_IS_AUTO_UPDATE>(const SfxItemPropertySimpleEntry&, const SfxItemPropertySet&, SwStyleBase_Impl& rBase)
{
    PrepareStyleBase(rBase);
    switch(GetFamily())
    {
        case SfxStyleFamily::Para : return uno::makeAny<bool>(rBase.getNewBase()->GetCollection()->IsAutoUpdateFormat());
        case SfxStyleFamily::Frame: return uno::makeAny<bool>(rBase.getNewBase()->GetFrameFormat()->IsAutoUpdateFormat());
        default: return uno::Any();
    }
}
template<>
uno::Any SwXStyle::GetStyleProperty<FN_UNO_DISPLAY_NAME>(const SfxItemPropertySimpleEntry&, const SfxItemPropertySet&, SwStyleBase_Impl& rBase)
{
    PrepareStyleBase(rBase);
    return uno::makeAny(rBase.getNewBase()->GetDisplayName());
}
template<>
uno::Any SwXStyle::GetStyleProperty<FN_UNO_PARA_STYLE_CONDITIONS>(const SfxItemPropertySimpleEntry&, const SfxItemPropertySet&, SwStyleBase_Impl& rBase)
{
    PrepareStyleBase(rBase);
    static_assert(COND_COMMAND_COUNT == 28, "invalid size of command count?");
    uno::Sequence<beans::NamedValue> aSeq(COND_COMMAND_COUNT);
    sal_uInt16 nIndex = 0;
    for(auto& rNV : aSeq)
    {
        rNV.Name = GetCommandContextByIndex(nIndex++);
        rNV.Value = uno::makeAny(OUString());
    }
    SwFormat* pFormat = static_cast<SwDocStyleSheet*>(GetStyleSheetBase())->GetCollection();
    if(pFormat && RES_CONDTXTFMTCOLL == pFormat->Which())
    {
        const CommandStruct* pCmds = SwCondCollItem::GetCmds();
        beans::NamedValue* pSeq = aSeq.getArray();
        for(sal_uInt16 n = 0;  n < COND_COMMAND_COUNT; ++n)
        {
            const SwCollCondition* pCond = static_cast<SwConditionTextFormatColl*>(pFormat)->HasCondition(SwCollCondition(nullptr, pCmds[n].nCnd, pCmds[n].nSubCond));
            if(!pCond || !pCond->GetTextFormatColl())
                continue;
            // get programmatic style name from UI style name
            OUString aStyleName = pCond->GetTextFormatColl()->GetName();
            SwStyleNameMapper::FillProgName(aStyleName, aStyleName, lcl_GetSwEnumFromSfxEnum(GetFamily()), true);
            pSeq[n].Value = uno::makeAny(aStyleName);
        }
    }
    return uno::makeAny(aSeq);
}
template<>
uno::Any SwXStyle::GetStyleProperty<FN_UNO_CATEGORY>(const SfxItemPropertySimpleEntry&, const SfxItemPropertySet&, SwStyleBase_Impl& rBase)
{
    PrepareStyleBase(rBase);
    static std::unique_ptr<std::map<collectionbits_t, paragraphstyle_t>> pUnoToCore;
    if(!pUnoToCore)
    {
        pUnoToCore.reset(new std::map<collectionbits_t, paragraphstyle_t>());
        auto pEntries = lcl_GetParagraphStyleCategoryEntries();
        std::transform(pEntries->begin(), pEntries->end(), std::inserter(*pUnoToCore, pUnoToCore->end()),
            [] (const ParagraphStyleCategoryEntry& rEntry) { return std::pair<collectionbits_t, paragraphstyle_t>(rEntry.m_nCollectionBits, rEntry.m_eCategory); });
    }
    const sal_uInt16 nPoolId = rBase.getNewBase()->GetCollection()->GetPoolFormatId();
    const auto pUnoToCoreIt(pUnoToCore->find(COLL_GET_RANGE_BITS & nPoolId));
    if(pUnoToCoreIt == pUnoToCore->end())
        return uno::makeAny<sal_Int16>(-1);
    return uno::makeAny(pUnoToCoreIt->second);
}
template<>
uno::Any SwXStyle::GetStyleProperty<SID_SWREGISTER_COLLECTION>(const SfxItemPropertySimpleEntry&, const SfxItemPropertySet&, SwStyleBase_Impl& rBase)
{
    PrepareStyleBase(rBase);
    const SwPageDesc *pPageDesc = rBase.getNewBase()->GetPageDesc();
    if(!pPageDesc)
        return uno::makeAny(OUString());
    const SwTextFormatColl* pCol = pPageDesc->GetRegisterFormatColl();
    if(!pCol)
        return uno::makeAny(OUString());
    OUString aName;
    SwStyleNameMapper::FillProgName(pCol->GetName(), aName, nsSwGetPoolIdFromName::GET_POOLID_TXTCOLL, true);
    return uno::makeAny(aName);
}
template<>
uno::Any SwXStyle::GetStyleProperty<RES_BACKGROUND>(const SfxItemPropertySimpleEntry& rEntry, const SfxItemPropertySet&, SwStyleBase_Impl& rBase)
{
    PrepareStyleBase(rBase);
    //UUUU
    const SfxItemSet& rSet = rBase.GetItemSet();
    const SvxBrushItem aOriginalBrushItem(getSvxBrushItemFromSourceSet(rSet, RES_BACKGROUND));
    const sal_uInt8 nMemberId(rEntry.nMemberId & (~SFX_METRIC_ITEM));
    uno::Any aResult;
    if(!aOriginalBrushItem.QueryValue(aResult, nMemberId))
        SAL_WARN("sw.uno", "error getting attribute from RES_BACKGROUND.");
    return aResult;
}
template<>
uno::Any SwXStyle::GetStyleProperty<OWN_ATTR_FILLBMP_MODE>(const SfxItemPropertySimpleEntry&, const SfxItemPropertySet&, SwStyleBase_Impl& rBase)
{
    PrepareStyleBase(rBase);
    //UUUU
    const SfxItemSet& rSet = rBase.GetItemSet();
    const XFillBmpTileItem* pTileItem = dynamic_cast<const XFillBmpTileItem*>(&rSet.Get(XATTR_FILLBMP_TILE));
    if(pTileItem && pTileItem->GetValue())
        return uno::makeAny(drawing::BitmapMode_REPEAT);
    const XFillBmpStretchItem* pStretchItem = dynamic_cast<const XFillBmpStretchItem*>(&rSet.Get(XATTR_FILLBMP_STRETCH));
    if(pStretchItem && pStretchItem->GetValue())
        return uno::makeAny(drawing::BitmapMode_STRETCH);
    return uno::makeAny(drawing::BitmapMode_NO_REPEAT);
}
template<>
uno::Any SwXStyle::GetStyleProperty<HINT_BEGIN>(const SfxItemPropertySimpleEntry& rEntry, const SfxItemPropertySet& rPropSet, SwStyleBase_Impl& rBase)
{
    PrepareStyleBase(rBase);
    SfxItemSet& rSet = rBase.GetItemSet();
    uno::Any aResult;
    rPropSet.getPropertyValue(rEntry, rSet, aResult);
    //UUUU
    // since the sfx uint16 item now exports a sal_Int32, we may have to fix this here
    if(rEntry.aType == cppu::UnoType<sal_Int16>::get() && aResult.getValueType() == cppu::UnoType<sal_Int32>::get())
        aResult = uno::makeAny<sal_Int16>(aResult.get<sal_Int32>());
    //UUUU check for needed metric translation
    if(rEntry.nMemberId & SFX_METRIC_ITEM && GetDoc())
    {
        const SfxItemPool& rPool = GetDoc()->GetAttrPool();
        const SfxMapUnit eMapUnit(rPool.GetMetric(rEntry.nWID));
        bool bAllowedConvert(true);
        // exception: If these ItemTypes are used, do not convert when these are negative
        // since this means they are intended as percent values
        if(XATTR_FILLBMP_SIZEX == rEntry.nWID || XATTR_FILLBMP_SIZEY == rEntry.nWID)
            bAllowedConvert = !aResult.has<sal_Int32>() || aResult.get<sal_Int32>() > 0;
        if(eMapUnit != SFX_MAPUNIT_100TH_MM && bAllowedConvert)
            SvxUnoConvertToMM(eMapUnit, aResult);
    }
    return aResult;
}

uno::Any SwXStyle::GetStyleProperty_Impl(const SfxItemPropertySimpleEntry& rEntry, const SfxItemPropertySet& rPropSet, SwStyleBase_Impl& rBase)
{
    using propertytype_t = decltype(rEntry.nWID);
    using coresetter_t = std::function<uno::Any(SwXStyle&, const SfxItemPropertySimpleEntry&, const SfxItemPropertySet&, SwStyleBase_Impl&)>;
    static std::unique_ptr<std::map<propertytype_t, coresetter_t>> pUnoToCore;
    if(!pUnoToCore)
    {
        pUnoToCore.reset(new std::map<propertytype_t, coresetter_t> {
            // these explicit std::mem_fn() calls shouldn't be needed, but apparently MSVC is currently too stupid for C++11 again
            { FN_UNO_IS_PHYSICAL,            std::mem_fn(&SwXStyle::GetStyleProperty<FN_UNO_IS_PHYSICAL>)            },
            { FN_UNO_HIDDEN,                 std::mem_fn(&SwXStyle::GetStyleProperty<FN_UNO_HIDDEN>)                 },
            { FN_UNO_STYLE_INTEROP_GRAB_BAG, std::mem_fn(&SwXStyle::GetStyleProperty<FN_UNO_STYLE_INTEROP_GRAB_BAG>) },
            { RES_PAPER_BIN,                 std::mem_fn(&SwXStyle::GetStyleProperty<RES_PAPER_BIN>)                 },
            { FN_UNO_NUM_RULES,              std::mem_fn(&SwXStyle::GetStyleProperty<FN_UNO_NUM_RULES>)              },
            { RES_PARATR_OUTLINELEVEL,       std::mem_fn(&SwXStyle::GetStyleProperty<RES_PARATR_OUTLINELEVEL>)       },
            { FN_UNO_FOLLOW_STYLE,           std::mem_fn(&SwXStyle::GetStyleProperty<FN_UNO_FOLLOW_STYLE>)           },
            { RES_PAGEDESC,                  std::mem_fn(&SwXStyle::GetStyleProperty<RES_PAGEDESC>)                  },
            { FN_UNO_IS_AUTO_UPDATE,         std::mem_fn(&SwXStyle::GetStyleProperty<FN_UNO_IS_AUTO_UPDATE>)         },
            { FN_UNO_DISPLAY_NAME,           std::mem_fn(&SwXStyle::GetStyleProperty<FN_UNO_DISPLAY_NAME>)           },
            { FN_UNO_PARA_STYLE_CONDITIONS,  std::mem_fn(&SwXStyle::GetStyleProperty<FN_UNO_PARA_STYLE_CONDITIONS>)  },
            { FN_UNO_CATEGORY,               std::mem_fn(&SwXStyle::GetStyleProperty<FN_UNO_CATEGORY>)               },
            { SID_SWREGISTER_COLLECTION,     std::mem_fn(&SwXStyle::GetStyleProperty<SID_SWREGISTER_COLLECTION>)     },
            { RES_BACKGROUND,                std::mem_fn(&SwXStyle::GetStyleProperty<RES_BACKGROUND>)                },
            { OWN_ATTR_FILLBMP_MODE,         std::mem_fn(&SwXStyle::GetStyleProperty<OWN_ATTR_FILLBMP_MODE>)         }
        });
    }
    const auto pUnoToCoreIt(pUnoToCore->find(rEntry.nWID));
    if(pUnoToCoreIt != pUnoToCore->end())
        return pUnoToCoreIt->second(*this, rEntry, rPropSet, rBase);
    return GetStyleProperty<HINT_BEGIN>(rEntry, rPropSet, rBase);
}

uno::Any SwXStyle::GetPropertyValue_Impl(const SfxItemPropertySet* pPropSet, SwStyleBase_Impl& rBase, const OUString& rPropertyName)
{
    const SfxItemPropertyMap& rMap = pPropSet->getPropertyMap();
    const SfxItemPropertySimpleEntry* pEntry = rMap.getByName(rPropertyName);
    if(!pEntry || (!m_bIsConditional && rPropertyName == UNO_NAME_PARA_STYLE_CONDITIONS))
        throw beans::UnknownPropertyException("Unknown property: " + rPropertyName, static_cast<cppu::OWeakObject*>(this));
    if(m_pBasePool)
        return GetStyleProperty_Impl(*pEntry, *pPropSet, rBase);
    const uno::Any* pAny = nullptr;
    m_pPropertiesImpl->GetProperty(rPropertyName, pAny);
    if(pAny->hasValue())
        return *pAny;
    uno::Any aValue;
    switch(m_rEntry.m_eFamily)
    {
        case SfxStyleFamily::Pseudo:
            throw uno::RuntimeException("No default value for: " + rPropertyName);
        break;
        case SfxStyleFamily::Para:
        case SfxStyleFamily::Page:
            SwStyleProperties_Impl::GetProperty(rPropertyName, m_xStyleData, aValue);
        break;
        case SfxStyleFamily::Char:
        case SfxStyleFamily::Frame:
        {
            if(pEntry->nWID < POOLATTR_BEGIN || pEntry->nWID >= RES_UNKNOWNATR_END)
                throw uno::RuntimeException("No default value for: " + rPropertyName);
            SwFormat* pFormat;
            if(m_rEntry.m_eFamily == SfxStyleFamily::Char)
                pFormat = m_pDoc->GetDfltCharFormat();
            else
                pFormat = m_pDoc->GetDfltFrameFormat();
            const SwAttrPool* pPool = pFormat->GetAttrSet().GetPool();
            const SfxPoolItem& rItem = pPool->GetDefaultItem(pEntry->nWID);
            rItem.QueryValue(aValue, pEntry->nMemberId);
        }
        break;
        default:
        ;
    }
    return aValue;
}

uno::Any SwXStyle::getPropertyValue(const OUString& rPropertyName)
    throw(beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    if(!m_pDoc)
        throw uno::RuntimeException();
    if(!m_pBasePool && !m_bIsDescriptor)
        throw uno::RuntimeException();
    sal_Int8 nPropSetId = m_bIsConditional ? PROPERTY_MAP_CONDITIONAL_PARA_STYLE : m_rEntry.m_nPropMapType;
    const SfxItemPropertySet* pPropSet = aSwMapProvider.GetPropertySet(nPropSetId);
    SwStyleBase_Impl aBase(*m_pDoc, m_sStyleName, &m_pDoc->GetDfltTextFormatColl()->GetAttrSet()); //UUUU add pDfltTextFormatColl as parent
    return GetPropertyValue_Impl(pPropSet, aBase, rPropertyName);
}

uno::Sequence<uno::Any> SwXStyle::getPropertyValues(const uno::Sequence<OUString>& rPropertyNames)
    throw(uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    if(!m_pDoc)
        throw uno::RuntimeException();
    if(!m_pBasePool && !m_bIsDescriptor)
        throw uno::RuntimeException();
    sal_Int8 nPropSetId = m_bIsConditional ? PROPERTY_MAP_CONDITIONAL_PARA_STYLE : m_rEntry.m_nPropMapType;
    const SfxItemPropertySet* pPropSet = aSwMapProvider.GetPropertySet(nPropSetId);
    SwStyleBase_Impl aBase(*m_pDoc, m_sStyleName, &m_pDoc->GetDfltTextFormatColl()->GetAttrSet()); //UUUU add pDfltTextFormatColl as parent
    uno::Sequence<uno::Any> aValues(rPropertyNames.getLength());
    // workaround for bad designed API
    try
    {
        for(sal_Int32 nProp = 0; nProp < rPropertyNames.getLength(); ++nProp)
            aValues[nProp] = GetPropertyValue_Impl(pPropSet, aBase, rPropertyNames[nProp]);
    }
    catch(beans::UnknownPropertyException&)
    {
        throw uno::RuntimeException("Unknown property exception caught", static_cast<cppu::OWeakObject*>(this));
    }
    catch(lang::WrappedTargetException&)
    {
        throw uno::RuntimeException("WrappedTargetException caught", static_cast<cppu::OWeakObject*>(this));
    }
    return aValues;
}

void SwXStyle::setPropertyValue(const OUString& rPropertyName, const uno::Any& rValue)
        throw(beans::UnknownPropertyException, beans::PropertyVetoException, lang::IllegalArgumentException, lang::WrappedTargetException, uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    const uno::Sequence<OUString> aProperties(&rPropertyName, 1);
    const uno::Sequence<uno::Any> aValues(&rValue, 1);
    SetPropertyValues_Impl(aProperties, aValues);
}

beans::PropertyState SwXStyle::getPropertyState(const OUString& rPropertyName)
        throw(beans::UnknownPropertyException, uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    uno::Sequence<OUString> aNames{rPropertyName};
    uno::Sequence<beans::PropertyState> aStates = getPropertyStates(aNames);
    return aStates.getConstArray()[0];
}

// allow to retarget the SfxItemSet working on, default correctly. Only
// use pSourceSet below this point (except in header/footer processing)
const SfxItemSet* lcl_GetItemsetForProperty(const SfxItemSet& rSet, SfxStyleFamily eFamily, const OUString& rPropertyName)
{
    if(eFamily != SfxStyleFamily::Page)
        return &rSet;
    const bool isFooter = rPropertyName.startsWith("Footer");
    if(!isFooter && !rPropertyName.startsWith("Header") && rPropertyName != UNO_NAME_FIRST_IS_SHARED)
        return &rSet;
    const SvxSetItem* pSetItem;
    if(!lcl_GetHeaderFooterItem(rSet, rPropertyName, isFooter, pSetItem))
        return nullptr;
    return &pSetItem->GetItemSet();
}
uno::Sequence<beans::PropertyState> SwXStyle::getPropertyStates(const uno::Sequence<OUString>& rPropertyNames)
        throw(beans::UnknownPropertyException, uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    uno::Sequence<beans::PropertyState> aRet(rPropertyNames.getLength());
    beans::PropertyState* pStates = aRet.getArray();

    if(!m_pBasePool)
        throw uno::RuntimeException();
    m_pBasePool->SetSearchMask(m_rEntry.m_eFamily);
    SfxStyleSheetBase* pBase = m_pBasePool->Find(m_sStyleName);

    SAL_WARN_IF(!pBase, "sw.uno", "where is the style?");
    if(!pBase)
        throw uno::RuntimeException();

    const OUString* pNames = rPropertyNames.getConstArray();
    rtl::Reference<SwDocStyleSheet> xStyle(new SwDocStyleSheet(*static_cast<SwDocStyleSheet*>(pBase)));
    sal_Int8 nPropSetId = m_bIsConditional ? PROPERTY_MAP_CONDITIONAL_PARA_STYLE : m_rEntry.m_nPropMapType;

    const SfxItemPropertySet* pPropSet = aSwMapProvider.GetPropertySet(nPropSetId);
    const SfxItemPropertyMap& rMap = pPropSet->getPropertyMap();
    const SfxItemSet& rSet = xStyle->GetItemSet();

    for(sal_Int32 i = 0; i < rPropertyNames.getLength(); ++i)
    {
        const OUString sPropName = pNames[i];
        const SfxItemPropertySimpleEntry* pEntry = rMap.getByName(sPropName);

        if(!pEntry)
            throw beans::UnknownPropertyException("Unknown property: " + sPropName, static_cast<cppu::OWeakObject*>(this));

        if(FN_UNO_NUM_RULES == pEntry->nWID || FN_UNO_FOLLOW_STYLE == pEntry->nWID)
        {
            // handle NumRules first, done
            pStates[i] = beans::PropertyState_DIRECT_VALUE;
            continue;
        }
        const SfxItemSet* pSourceSet = lcl_GetItemsetForProperty(rSet, m_rEntry.m_eFamily, sPropName);
        if(!pSourceSet)
        {
            // if no SetItem, value is ambigous and we are done
            pStates[i] = beans::PropertyState_AMBIGUOUS_VALUE;
            continue;
        }
        switch(pEntry->nWID)
        {
            case OWN_ATTR_FILLBMP_MODE:
            {
                //UUUU
                if(SfxItemState::SET == pSourceSet->GetItemState(XATTR_FILLBMP_STRETCH, false)
                    || SfxItemState::SET == pSourceSet->GetItemState(XATTR_FILLBMP_TILE, false))
                {
                    pStates[i] = beans::PropertyState_DIRECT_VALUE;
                }
                else
                {
                    pStates[i] = beans::PropertyState_AMBIGUOUS_VALUE;
                }
            }
            break;
            case RES_BACKGROUND:
            {
                //UUUU for FlyFrames we need to mark the used properties from type RES_BACKGROUND
                // as beans::PropertyState_DIRECT_VALUE to let users of this property call
                // getPropertyValue where the member properties will be mapped from the
                // fill attributes to the according SvxBrushItem entries
                if (SWUnoHelper::needToMapFillItemsToSvxBrushItemTypes(*pSourceSet, pEntry->nMemberId))
                {
                    pStates[i] = beans::PropertyState_DIRECT_VALUE;
                }
                else
                {
                    pStates[i] = beans::PropertyState_DEFAULT_VALUE;
                }
            }
            break;
            default:
            {
                pStates[i] = pPropSet->getPropertyState(*pEntry, *pSourceSet);

                if(SfxStyleFamily::Page == m_rEntry.m_eFamily && SID_ATTR_PAGE_SIZE == pEntry->nWID && beans::PropertyState_DIRECT_VALUE == pStates[i])
                {
                    const SvxSizeItem& rSize = static_cast <const SvxSizeItem&>( rSet.Get(SID_ATTR_PAGE_SIZE));
                    sal_uInt8 nMemberId = pEntry->nMemberId & 0x7f;

                    if((LONG_MAX == rSize.GetSize().Width() && (MID_SIZE_WIDTH == nMemberId || MID_SIZE_SIZE == nMemberId)) ||
                        (LONG_MAX == rSize.GetSize().Height() && MID_SIZE_HEIGHT == nMemberId))
                    {
                        pStates[i] = beans::PropertyState_DEFAULT_VALUE;
                    }
                }
            }
        }
    }
    return aRet;
}

void SwXStyle::setPropertyToDefault(const OUString& rPropertyName)
        throw(beans::UnknownPropertyException, uno::RuntimeException, std::exception)
{
    const uno::Sequence<OUString> aSequence(&rPropertyName, 1);
    setPropertiesToDefault(aSequence);
}

SwFormat* lcl_GetFormatForStyle(SwDoc* pDoc, const rtl::Reference<SwDocStyleSheet>& xStyle, const SfxStyleFamily eFamily)
{
    if(!xStyle.is())
        return nullptr;
    switch(eFamily)
    {
        case SfxStyleFamily::Char: return xStyle->GetCharFormat();
        case SfxStyleFamily::Para: return xStyle->GetCollection();
        case SfxStyleFamily::Frame: return xStyle->GetFrameFormat();
        case SfxStyleFamily::Page:
        {
            SwPageDesc* pDesc(pDoc->FindPageDesc(xStyle->GetPageDesc()->GetName()));
            if(pDesc)
                return &pDesc->GetMaster();
        }
        break;
        default: ;
    }
    return nullptr;
}

void SAL_CALL SwXStyle::setPropertiesToDefault(const uno::Sequence<OUString>& aPropertyNames)
        throw (beans::UnknownPropertyException, uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    const rtl::Reference<SwDocStyleSheet> xStyle(new SwDocStyleSheet(*static_cast<SwDocStyleSheet*>(GetStyleSheetBase())));
    SwFormat* pTargetFormat = lcl_GetFormatForStyle(m_pDoc, xStyle, m_rEntry.m_eFamily);
    if(!pTargetFormat)
    {
        if(!m_bIsDescriptor)
            return;
        for(const auto& rName : aPropertyNames)
            m_pPropertiesImpl->ClearProperty(rName);
        return;
    }
    const sal_Int8 nPropSetId = m_bIsConditional ? PROPERTY_MAP_CONDITIONAL_PARA_STYLE : m_rEntry.m_nPropMapType;
    const SfxItemPropertySet* pPropSet = aSwMapProvider.GetPropertySet(nPropSetId);
    const SfxItemPropertyMap &rMap = pPropSet->getPropertyMap();
    for(const auto& rName : aPropertyNames)
    {
        const SfxItemPropertySimpleEntry* pEntry = rMap.getByName(rName);
        if(!pEntry)
            throw beans::UnknownPropertyException("Unknown property: " + rName, static_cast<cppu::OWeakObject*>(this));
        if(pEntry->nWID == FN_UNO_FOLLOW_STYLE || pEntry->nWID == FN_UNO_NUM_RULES)
            throw uno::RuntimeException("Cannot reset: " + rName, static_cast<cppu::OWeakObject*>(this));
        if(pEntry->nFlags & beans::PropertyAttribute::READONLY)
            throw uno::RuntimeException("setPropertiesToDefault: property is read-only: " + rName, static_cast<cppu::OWeakObject*>(this));
        if(pEntry->nWID == RES_PARATR_OUTLINELEVEL)
        {
            static_cast<SwTextFormatColl*>(pTargetFormat)->DeleteAssignmentToListLevelOfOutlineStyle();
            continue;
        }
        pTargetFormat->ResetFormatAttr(pEntry->nWID);
        if(OWN_ATTR_FILLBMP_MODE == pEntry->nWID)
        {
            //UUUU
            SwDoc* pDoc = pTargetFormat->GetDoc();
            SfxItemSet aSet(pDoc->GetAttrPool(), XATTR_FILL_FIRST, XATTR_FILL_LAST);
            aSet.SetParent(&pTargetFormat->GetAttrSet());

            aSet.ClearItem(XATTR_FILLBMP_STRETCH);
            aSet.ClearItem(XATTR_FILLBMP_TILE);

            pTargetFormat->SetFormatAttr(aSet);
        }
    }
}

void SAL_CALL SwXStyle::setAllPropertiesToDefault()
    throw(uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    if(!m_pBasePool)
    {
        if(!m_bIsDescriptor)
            throw uno::RuntimeException();
        m_pPropertiesImpl->ClearAllProperties();
        return;
    }
    const rtl::Reference<SwDocStyleSheet> xStyle(new SwDocStyleSheet(*static_cast<SwDocStyleSheet*>(GetStyleSheetBase())));
    if(!xStyle.is())
        throw uno::RuntimeException();
    if(SfxStyleFamily::Page == m_rEntry.m_eFamily)
    {
        size_t nPgDscPos(0);
        SwPageDesc* pDesc = m_pDoc->FindPageDesc(xStyle->GetPageDesc()->GetName(), &nPgDscPos);
        SwFormat* pPageFormat(nullptr);
        if(pDesc)
        {
            pPageFormat = &pDesc->GetMaster();
            pDesc->SetUseOn(nsUseOnPage::PD_ALL);
        }
        else
            pPageFormat = lcl_GetFormatForStyle(m_pDoc, xStyle, m_rEntry.m_eFamily);
        SwPageDesc& rPageDesc = m_pDoc->GetPageDesc(nPgDscPos);
        rPageDesc.ResetAllMasterAttr();

        SvxLRSpaceItem aLR(RES_LR_SPACE);
        sal_Int32 nSize = GetMetricVal(CM_1) * 2;
        aLR.SetLeft(nSize);
        aLR.SetLeft(nSize);
        SvxULSpaceItem aUL(RES_UL_SPACE);
        aUL.SetUpper(static_cast<sal_uInt16>(nSize));
        aUL.SetLower(static_cast<sal_uInt16>(nSize));
        pPageFormat->SetFormatAttr(aLR);
        pPageFormat->SetFormatAttr(aUL);
        SwPageDesc* pStdPgDsc = m_pDoc->getIDocumentStylePoolAccess().GetPageDescFromPool(RES_POOLPAGE_STANDARD);
        SwFormatFrameSize aFrameSz(ATT_FIX_SIZE);

        if(RES_POOLPAGE_STANDARD == rPageDesc.GetPoolFormatId())
        {
            if(m_pDoc->getIDocumentDeviceAccess().getPrinter(false))
            {
                const Size aPhysSize( SvxPaperInfo::GetPaperSize(
                    static_cast<Printer*>(m_pDoc->getIDocumentDeviceAccess().getPrinter(false))));
                aFrameSz.SetSize(aPhysSize);
            }
            else
            {
                aFrameSz.SetSize(SvxPaperInfo::GetDefaultPaperSize());
            }

        }
        else
        {
            aFrameSz = pStdPgDsc->GetMaster().GetFrameSize();
        }

        if(pStdPgDsc->GetLandscape())
        {
            SwTwips nTmp = aFrameSz.GetHeight();
            aFrameSz.SetHeight(aFrameSz.GetWidth());
            aFrameSz.SetWidth(nTmp);
        }

        pPageFormat->SetFormatAttr(aFrameSz);
        m_pDoc->ChgPageDesc(nPgDscPos, m_pDoc->GetPageDesc(nPgDscPos));
        return;
    }
    if(SfxStyleFamily::Para == m_rEntry.m_eFamily)
    {
        if(xStyle->GetCollection())
            xStyle->GetCollection()->DeleteAssignmentToListLevelOfOutlineStyle();
    }
    SwFormat* const pTargetFormat = lcl_GetFormatForStyle(m_pDoc, xStyle, m_rEntry.m_eFamily);
    if(!pTargetFormat)
        return;
    pTargetFormat->ResetAllFormatAttr();
}

uno::Sequence<uno::Any> SAL_CALL SwXStyle::getPropertyDefaults(const uno::Sequence<OUString>& aPropertyNames)
        throw (beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    sal_Int32 nCount = aPropertyNames.getLength();
    uno::Sequence<uno::Any> aRet(nCount);
    if(!nCount)
        return aRet;
    SfxStyleSheetBase* pBase = GetStyleSheetBase();
    if(!pBase)
        throw uno::RuntimeException();
    rtl::Reference<SwDocStyleSheet> xStyle(new SwDocStyleSheet(*static_cast<SwDocStyleSheet*>(pBase)));
    const sal_Int8 nPropSetId = m_bIsConditional ? PROPERTY_MAP_CONDITIONAL_PARA_STYLE : m_rEntry.m_nPropMapType;
    const SfxItemPropertySet* pPropSet = aSwMapProvider.GetPropertySet(nPropSetId);
    const SfxItemPropertyMap& rMap = pPropSet->getPropertyMap();

    const SfxItemSet &rSet = xStyle->GetItemSet(), *pParentSet = rSet.GetParent();
    for(sal_Int32 i = 0; i < nCount; ++i)
    {
        const SfxItemPropertySimpleEntry* pEntry = rMap.getByName(aPropertyNames[i]);

        if(!pEntry)
            throw beans::UnknownPropertyException("Unknown property: " + aPropertyNames[i], static_cast < cppu::OWeakObject * >(this));
        // these cannot be in an item set, especially not the
        // parent set, so the default value is void
        if (pEntry->nWID >= RES_UNKNOWNATR_END)
            continue;

        if(pParentSet)
        {
            aSwMapProvider.GetPropertySet(nPropSetId)->getPropertyValue(aPropertyNames[i], *pParentSet, aRet[i]);
        }
        else if(pEntry->nWID != rSet.GetPool()->GetSlotId(pEntry->nWID))
        {
            const SfxPoolItem& rItem = rSet.GetPool()->GetDefaultItem(pEntry->nWID);
            rItem.QueryValue(aRet[i], pEntry->nMemberId);
        }
    }
    return aRet;
}

uno::Any SwXStyle::getPropertyDefault(const OUString& rPropertyName)
    throw(beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException, std::exception)
{
    const uno::Sequence<OUString> aSequence(&rPropertyName, 1);
    return getPropertyDefaults(aSequence)[0];
}

void SwXStyle::Notify(SfxBroadcaster& rBC, const SfxHint& rHint)
{
    const SfxSimpleHint* pHint = dynamic_cast<const SfxSimpleHint*>(&rHint);
    if(!pHint)
        return;
    if((pHint->GetId() & SFX_HINT_DYING) || (pHint->GetId() & SfxStyleSheetHintId::ERASED))
    {
        m_pBasePool = nullptr;
        EndListening(rBC);
    }
    else if(pHint->GetId() & (SfxStyleSheetHintId::CHANGED))
    {
        static_cast<SfxStyleSheetBasePool&>(rBC).SetSearchMask(m_rEntry.m_eFamily);
        SfxStyleSheetBase* pOwnBase = static_cast<SfxStyleSheetBasePool&>(rBC).Find(m_sStyleName);
        if(!pOwnBase)
        {
            EndListening(rBC);
            Invalidate();
        }
    }
}

void SwXStyle::Invalidate()
{
    m_sStyleName.clear();
    m_pBasePool = nullptr;
    m_pDoc = nullptr;
    m_xStyleData.clear();
    m_xStyleFamily.clear();
}

SwXPageStyle::SwXPageStyle(SfxStyleSheetBasePool& rPool, SwDocShell* pDocSh, SfxStyleFamily eFam, const OUString& rStyleName)
    : SwXStyle(&rPool, eFam, pDocSh->GetDoc(), rStyleName)
{ }

SwXPageStyle::SwXPageStyle(SwDocShell* pDocSh)
    : SwXStyle(pDocSh->GetDoc(), SfxStyleFamily::Page)
{ }

SwXPageStyle::~SwXPageStyle()
{ }

void SwXStyle::PutItemToSet(const SvxSetItem* pSetItem, const SfxItemPropertySet& rPropSet, const SfxItemPropertySimpleEntry& rEntry, const uno::Any& rVal, SwStyleBase_Impl& rBaseImpl)
{
    // create a new SvxSetItem and get it's ItemSet as new target
    const std::unique_ptr<SvxSetItem> pNewSetItem(static_cast<SvxSetItem*>(pSetItem->Clone()));
    SfxItemSet& rSetSet = pNewSetItem->GetItemSet();

    // set parent to ItemSet to ensure XFILL_NONE as XFillStyleItem
    rSetSet.SetParent(&m_pDoc->GetDfltFrameFormat()->GetAttrSet());

    // replace the used SfxItemSet at the SwStyleBase_Impl temporarily and use the
    // default method to set the property
    {
        SwStyleBase_Impl::ItemSetOverrider o(rBaseImpl, &rSetSet);
        SetStyleProperty(rEntry, rPropSet, rVal, rBaseImpl);
    }

    // reset paret at ItemSet from SetItem
    rSetSet.SetParent(nullptr);

    // set the new SvxSetItem at the real target and delete it
    rBaseImpl.GetItemSet().Put(*pNewSetItem);
}

void SwXPageStyle::SetPropertyValues_Impl(const uno::Sequence<OUString>& rPropertyNames, const uno::Sequence<uno::Any>& rValues)
{
    if(!GetDoc())
        throw uno::RuntimeException();

    if(rPropertyNames.getLength() != rValues.getLength())
        throw lang::IllegalArgumentException();

    const SfxItemPropertySet* pPropSet = aSwMapProvider.GetPropertySet(PROPERTY_MAP_PAGE_STYLE);
    const SfxItemPropertyMap& rMap = pPropSet->getPropertyMap();
    SwStyleBase_Impl aBaseImpl(*GetDoc(), GetStyleName(), &GetDoc()->GetDfltFrameFormat()->GetAttrSet()); //UUUU add pDfltFrameFormat as parent
    if(!m_pBasePool)
    {
        if(!IsDescriptor())
            throw uno::RuntimeException();
        for(sal_Int32 nProp = 0; nProp < rPropertyNames.getLength(); ++nProp)
            if(!m_pPropertiesImpl->SetProperty(rPropertyNames[nProp], rValues[nProp]))
                throw lang::IllegalArgumentException();
        return;
    }
    SfxStyleSheetBase* pBase = GetStyleSheetBase();
    if(!pBase)
        throw uno::RuntimeException();
    aBaseImpl.setNewBase(new SwDocStyleSheet(*static_cast<SwDocStyleSheet*>(pBase)));
    for(sal_Int32 nProp = 0; nProp < rPropertyNames.getLength(); ++nProp)
    {
        const OUString& rPropName = rPropertyNames[nProp];
        const SfxItemPropertySimpleEntry* pEntry = rMap.getByName(rPropName);

        if(!pEntry)
            throw beans::UnknownPropertyException("Unknown property: " + rPropName, static_cast<cppu::OWeakObject*>(this));
        if(pEntry->nFlags & beans::PropertyAttribute::READONLY)
            throw beans::PropertyVetoException("Property is read-only: " + rPropName, static_cast<cppu::OWeakObject*>(this));

        const bool bHeader(rPropName.startsWith("Header"));
        const bool bFooter(rPropName.startsWith("Footer"));
        const bool bFirstIsShared(rPropName == UNO_NAME_FIRST_IS_SHARED);
        if(bHeader || bFooter || bFirstIsShared)
        {
            switch(pEntry->nWID)
            {
                case SID_ATTR_PAGE_ON:
                case RES_BACKGROUND:
                case RES_BOX:
                case RES_LR_SPACE:
                case RES_SHADOW:
                case RES_UL_SPACE:
                case SID_ATTR_PAGE_DYNAMIC:
                case SID_ATTR_PAGE_SHARED:
                case SID_ATTR_PAGE_SHARED_FIRST:
                case SID_ATTR_PAGE_SIZE:
                case RES_HEADER_FOOTER_EAT_SPACING:
                {
                    // it is a Header/Footer entry, access the SvxSetItem containing it's information
                    const SvxSetItem* pSetItem = nullptr;
                    if (lcl_GetHeaderFooterItem(aBaseImpl.GetItemSet(), rPropName, bFooter, pSetItem))
                    {
                        PutItemToSet(pSetItem, *pPropSet, *pEntry, rValues[nProp], aBaseImpl);

                        if (pEntry->nWID == SID_ATTR_PAGE_SHARED_FIRST)
                        {
                            // Need to add this to the other as well
                            if (SfxItemState::SET == aBaseImpl.GetItemSet().GetItemState(
                                        bFooter ? SID_ATTR_PAGE_HEADERSET : SID_ATTR_PAGE_FOOTERSET,
                                        false, reinterpret_cast<const SfxPoolItem**>(&pSetItem)))
                            {
                                PutItemToSet(pSetItem, *pPropSet, *pEntry, rValues[nProp], aBaseImpl);
                            }
                        }
                    }
                    else if(pEntry->nWID == SID_ATTR_PAGE_ON && rValues[nProp].get<bool>())
                    {
                        // Header/footer gets switched on, create defaults and the needed SfxSetItem
                        SfxItemSet aTempSet(*aBaseImpl.GetItemSet().GetPool(),
                            RES_FRMATR_BEGIN,RES_FRMATR_END - 1,            // [82

                            //UUUU FillAttribute support
                            XATTR_FILL_FIRST, XATTR_FILL_LAST,              // [1014

                            SID_ATTR_BORDER_INNER,SID_ATTR_BORDER_INNER,    // [10023
                            SID_ATTR_PAGE_SIZE,SID_ATTR_PAGE_SIZE,          // [10051
                            SID_ATTR_PAGE_ON,SID_ATTR_PAGE_SHARED,          // [10060
                            SID_ATTR_PAGE_SHARED_FIRST,SID_ATTR_PAGE_SHARED_FIRST,
                            0);

                        //UUUU set correct parent to get the XFILL_NONE FillStyle as needed
                        aTempSet.SetParent(&GetDoc()->GetDfltFrameFormat()->GetAttrSet());

                        aTempSet.Put(SfxBoolItem(SID_ATTR_PAGE_ON, true));
                        aTempSet.Put(SvxSizeItem(SID_ATTR_PAGE_SIZE, Size(MM50, MM50)));
                        aTempSet.Put(SvxLRSpaceItem(RES_LR_SPACE));
                        aTempSet.Put(SvxULSpaceItem(RES_UL_SPACE));
                        aTempSet.Put(SfxBoolItem(SID_ATTR_PAGE_SHARED, true));
                        aTempSet.Put(SfxBoolItem(SID_ATTR_PAGE_SHARED_FIRST, true));
                        aTempSet.Put(SfxBoolItem(SID_ATTR_PAGE_DYNAMIC, true));

                        SvxSetItem aNewSetItem(bFooter ? SID_ATTR_PAGE_FOOTERSET : SID_ATTR_PAGE_HEADERSET, aTempSet);
                        aBaseImpl.GetItemSet().Put(aNewSetItem);
                    }
                }
                continue;
                case XATTR_FILLBMP_SIZELOG:
                case XATTR_FILLBMP_TILEOFFSETX:
                case XATTR_FILLBMP_TILEOFFSETY:
                case XATTR_FILLBMP_POSOFFSETX:
                case XATTR_FILLBMP_POSOFFSETY:
                case XATTR_FILLBMP_POS:
                case XATTR_FILLBMP_SIZEX:
                case XATTR_FILLBMP_SIZEY:
                case XATTR_FILLBMP_STRETCH:
                case XATTR_FILLBMP_TILE:
                case OWN_ATTR_FILLBMP_MODE:
                case XATTR_FILLCOLOR:
                case XATTR_FILLBACKGROUND:
                case XATTR_FILLBITMAP:
                case XATTR_GRADIENTSTEPCOUNT:
                case XATTR_FILLGRADIENT:
                case XATTR_FILLHATCH:
                case XATTR_FILLSTYLE:
                case XATTR_FILLTRANSPARENCE:
                case XATTR_FILLFLOATTRANSPARENCE:
                case XATTR_SECONDARYFILLCOLOR:
                if(bFirstIsShared) // only special handling for headers/footers here
                    break;
                {
                    const SvxSetItem* pSetItem = nullptr;

                    if(SfxItemState::SET == aBaseImpl.GetItemSet().GetItemState(bFooter ? SID_ATTR_PAGE_FOOTERSET : SID_ATTR_PAGE_HEADERSET, false, reinterpret_cast<const SfxPoolItem**>(&pSetItem)))
                    {
                        // create a new SvxSetItem and get it's ItemSet as new target
                        std::unique_ptr<SvxSetItem> pNewSetItem(static_cast<SvxSetItem*>(pSetItem->Clone()));
                        SfxItemSet& rSetSet = pNewSetItem->GetItemSet();

                        // set parent to ItemSet to ensure XFILL_NONE as XFillStyleItem
                        rSetSet.SetParent(&GetDoc()->GetDfltFrameFormat()->GetAttrSet());

                        // replace the used SfxItemSet at the SwStyleBase_Impl temporarily and use the
                        // default method to set the property
                        {
                            SwStyleBase_Impl::ItemSetOverrider o(aBaseImpl, &rSetSet);
                            SetStyleProperty(*pEntry, *pPropSet, rValues[nProp], aBaseImpl);
                        }

                        // reset paret at ItemSet from SetItem
                        rSetSet.SetParent(nullptr);

                        // set the new SvxSetItem at the real target and delete it
                        aBaseImpl.GetItemSet().Put(*pNewSetItem);
                    }
                }
                continue;
                default: ;
            }
        }
        switch(pEntry->nWID)
        {
            case SID_ATTR_PAGE_DYNAMIC:
            case SID_ATTR_PAGE_SHARED:
            case SID_ATTR_PAGE_SHARED_FIRST:
            case SID_ATTR_PAGE_ON:
            case RES_HEADER_FOOTER_EAT_SPACING:
                // these slots are exclusive to Header/Footer, thus this is an error
                throw beans::UnknownPropertyException("Unknown property: " + rPropName, static_cast<cppu::OWeakObject*>(this));
            case FN_UNO_HEADER:
            case FN_UNO_HEADER_LEFT:
            case FN_UNO_HEADER_RIGHT:
            case FN_UNO_HEADER_FIRST:
            case FN_UNO_FOOTER:
            case FN_UNO_FOOTER_LEFT:
            case FN_UNO_FOOTER_RIGHT:
            case FN_UNO_FOOTER_FIRST:
                throw lang::IllegalArgumentException();
            case FN_PARAM_FTN_INFO:
            {
                const SfxPoolItem& rItem = aBaseImpl.GetItemSet().Get(FN_PARAM_FTN_INFO);
                const std::unique_ptr<SfxPoolItem> pNewFootnoteItem(rItem.Clone());
                if(!pNewFootnoteItem->PutValue(rValues[nProp], pEntry->nMemberId))
                    throw lang::IllegalArgumentException();
                aBaseImpl.GetItemSet().Put(*pNewFootnoteItem);
                break;
            }
            default:
            {
                SetStyleProperty(*pEntry, *pPropSet, rValues[nProp], aBaseImpl);
                break;
            }
        }
    }

    if(aBaseImpl.HasItemSet())
    {
        ::sw::UndoGuard const undoGuard(GetDoc()->GetIDocumentUndoRedo());

        if (undoGuard.UndoWasEnabled())
        {
            // Fix i64460: as long as Undo of page styles with header/footer causes trouble...
            GetDoc()->GetIDocumentUndoRedo().DelAllUndoObj();
        }

        aBaseImpl.getNewBase()->SetItemSet(aBaseImpl.GetItemSet());
    }
}

void SwXPageStyle::setPropertyValues(const uno::Sequence<OUString>& rPropertyNames, const uno::Sequence<uno::Any>& rValues)
        throw(beans::PropertyVetoException, lang::IllegalArgumentException, lang::WrappedTargetException, uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;

    // workaround for bad designed API
    try
    {
        SetPropertyValues_Impl(rPropertyNames, rValues);
    }
    catch (const beans::UnknownPropertyException &rException)
    {
        // wrap the original (here not allowed) exception in
        // a lang::WrappedTargetException that gets thrown instead.
        lang::WrappedTargetException aWExc;
        aWExc.TargetException <<= rException;
        throw aWExc;
    }
}

static uno::Reference<text::XText> lcl_makeHeaderFooter(const sal_uInt16 nRes, const bool bHeader, SwFrameFormat const*const pFrameFormat)
{
    if (!pFrameFormat)
        return nullptr;
    const SfxItemSet& rSet = pFrameFormat->GetAttrSet();
    const SfxPoolItem* pItem;
    if(SfxItemState::SET != rSet.GetItemState(nRes, true, &pItem))
        return nullptr;
    SwFrameFormat* const pHeadFootFormat = (bHeader)
        ? static_cast<SwFormatHeader*>(const_cast<SfxPoolItem*>(pItem))->GetHeaderFormat()
        : static_cast<SwFormatFooter*>(const_cast<SfxPoolItem*>(pItem))->GetFooterFormat();
    if(!pHeadFootFormat)
        return nullptr;
    return SwXHeadFootText::CreateXHeadFootText(*pHeadFootFormat, bHeader);
}

uno::Sequence<uno::Any> SwXPageStyle::GetPropertyValues_Impl(const uno::Sequence<OUString>& rPropertyNames)
{
    if(!GetDoc())
        throw uno::RuntimeException();

    sal_Int32 nLength = rPropertyNames.getLength();
    uno::Sequence<uno::Any> aRet (nLength);
    if(!m_pBasePool)
    {
        if(!IsDescriptor())
            throw uno::RuntimeException();
        for(sal_Int32 nProp = 0; nProp < rPropertyNames.getLength(); ++nProp)
        {
            const uno::Any* pAny = nullptr;
            m_pPropertiesImpl->GetProperty(rPropertyNames[nProp], pAny);
            if (!pAny->hasValue())
                SwStyleProperties_Impl::GetProperty(rPropertyNames[nProp], m_xStyleData, aRet[nProp]);
            else
                aRet[nProp] = *pAny;
        }
        return aRet;
    }
    const SfxItemPropertySet* pPropSet = aSwMapProvider.GetPropertySet(PROPERTY_MAP_PAGE_STYLE);
    const SfxItemPropertyMap& rMap = pPropSet->getPropertyMap();
    SwStyleBase_Impl aBase(*GetDoc(), GetStyleName(), &GetDoc()->GetDfltFrameFormat()->GetAttrSet()); //UUUU add pDfltFrameFormat as parent
    SfxStyleSheetBase* pBase = GetStyleSheetBase();
    if(!pBase)
        throw uno::RuntimeException();
    for(sal_Int32 nProp = 0; nProp < nLength; ++nProp)
    {
        const OUString& rPropName = rPropertyNames[nProp];
        const SfxItemPropertySimpleEntry* pEntry = rMap.getByName(rPropName);

        if (!pEntry)
            throw beans::UnknownPropertyException("Unknown property: " + rPropName, static_cast < cppu::OWeakObject * > ( this ) );
        const bool bHeader(rPropName.startsWith("Header"));
        const bool bFooter(rPropName.startsWith("Footer"));
        const bool bFirstIsShared(rPropName == UNO_NAME_FIRST_IS_SHARED);
        if(bHeader || bFooter || bFirstIsShared)
        {
            switch(pEntry->nWID)
            {
                case SID_ATTR_PAGE_ON:
                case RES_BACKGROUND:
                case RES_BOX:
                case RES_LR_SPACE:
                case RES_SHADOW:
                case RES_UL_SPACE:
                case SID_ATTR_PAGE_DYNAMIC:
                case SID_ATTR_PAGE_SHARED:
                case SID_ATTR_PAGE_SHARED_FIRST:
                case SID_ATTR_PAGE_SIZE:
                case RES_HEADER_FOOTER_EAT_SPACING:
                {
                    // slot is a Header/Footer slot
                    rtl::Reference< SwDocStyleSheet > xStyle( new SwDocStyleSheet( *static_cast<SwDocStyleSheet*>(pBase) ) );
                    const SfxItemSet& rSet = xStyle->GetItemSet();
                    const SvxSetItem* pSetItem;

                    if (lcl_GetHeaderFooterItem(rSet, rPropName, bFooter, pSetItem))
                    {
                        // get from SfxItemSet of the corresponding SfxSetItem
                        const SfxItemSet& rSetSet = pSetItem->GetItemSet();
                        {
                            SwStyleBase_Impl::ItemSetOverrider o(aBase, &const_cast< SfxItemSet& >(rSetSet));
                            aRet[nProp] = GetStyleProperty_Impl(*pEntry, *pPropSet, aBase);
                        }
                    }
                    else if(pEntry->nWID == SID_ATTR_PAGE_ON)
                    {
                        // header/footer is not available, thus off. Default is <false>, though
                        aRet[nProp] <<= false;
                    }
                }
                continue;
                case XATTR_FILLBMP_SIZELOG:
                case XATTR_FILLBMP_TILEOFFSETX:
                case XATTR_FILLBMP_TILEOFFSETY:
                case XATTR_FILLBMP_POSOFFSETX:
                case XATTR_FILLBMP_POSOFFSETY:
                case XATTR_FILLBMP_POS:
                case XATTR_FILLBMP_SIZEX:
                case XATTR_FILLBMP_SIZEY:
                case XATTR_FILLBMP_STRETCH:
                case XATTR_FILLBMP_TILE:
                case OWN_ATTR_FILLBMP_MODE:
                case XATTR_FILLCOLOR:
                case XATTR_FILLBACKGROUND:
                case XATTR_FILLBITMAP:
                case XATTR_GRADIENTSTEPCOUNT:
                case XATTR_FILLGRADIENT:
                case XATTR_FILLHATCH:
                case XATTR_FILLSTYLE:
                case XATTR_FILLTRANSPARENCE:
                case XATTR_FILLFLOATTRANSPARENCE:
                case XATTR_SECONDARYFILLCOLOR:
                if(bFirstIsShared) // only special handling for headers/footers here
                    break;
                {
                    rtl::Reference< SwDocStyleSheet > xStyle( new SwDocStyleSheet( *static_cast<SwDocStyleSheet*>(pBase) ) );
                    const SfxItemSet& rSet = xStyle->GetItemSet();
                    const SvxSetItem* pSetItem;
                    if(SfxItemState::SET == rSet.GetItemState(bFooter ? SID_ATTR_PAGE_FOOTERSET : SID_ATTR_PAGE_HEADERSET, false, reinterpret_cast<const SfxPoolItem**>(&pSetItem)))
                    {
                        // set at SfxItemSet of the corresponding SfxSetItem
                        const SfxItemSet& rSetSet = pSetItem->GetItemSet();
                        {
                            SwStyleBase_Impl::ItemSetOverrider o(aBase, &const_cast<SfxItemSet&>(rSetSet));
                            aRet[nProp] = GetStyleProperty_Impl(*pEntry, *pPropSet, aBase);
                        }
                    }
                }
                continue;
                default: ;
            }
        }
        switch(pEntry->nWID)
        {
            // these slots are exclusive to Header/Footer, thus this is an error
            case SID_ATTR_PAGE_DYNAMIC:
            case SID_ATTR_PAGE_SHARED:
            case SID_ATTR_PAGE_SHARED_FIRST:
            case SID_ATTR_PAGE_ON:
            case RES_HEADER_FOOTER_EAT_SPACING:
                throw beans::UnknownPropertyException(OUString ( RTL_CONSTASCII_USTRINGPARAM ( "Unknown property: " ) ) + rPropName, static_cast < cppu::OWeakObject * > ( this ) );
            case FN_UNO_HEADER:
            case FN_UNO_HEADER_LEFT:
            case FN_UNO_HEADER_FIRST:
            case FN_UNO_HEADER_RIGHT:
            case FN_UNO_FOOTER:
            case FN_UNO_FOOTER_LEFT:
            case FN_UNO_FOOTER_FIRST:
            case FN_UNO_FOOTER_RIGHT:
            {
                bool bLeft(false);
                bool bFirst(false);
                sal_uInt16 nRes = 0;
                switch(pEntry->nWID)
                {
                    case FN_UNO_HEADER:       nRes = RES_HEADER; break;
                    case FN_UNO_HEADER_LEFT:  nRes = RES_HEADER; bLeft = true;  break;
                    case FN_UNO_HEADER_FIRST: nRes = RES_HEADER; bFirst = true; break;
                    case FN_UNO_HEADER_RIGHT: nRes = RES_HEADER; break;
                    case FN_UNO_FOOTER:       nRes = RES_FOOTER; break;
                    case FN_UNO_FOOTER_LEFT:  nRes = RES_FOOTER; bLeft = true;  break;
                    case FN_UNO_FOOTER_FIRST: nRes = RES_FOOTER; bFirst = true; break;
                    case FN_UNO_FOOTER_RIGHT: nRes = RES_FOOTER; break;
                    default: ;
                }

                const SwPageDesc* pDesc = aBase.GetOldPageDesc();
                assert(pDesc);
                const SwFrameFormat* pFrameFormat = nullptr;
                bool bShare = (nRes == RES_HEADER && pDesc->IsHeaderShared()) || (nRes == RES_FOOTER && pDesc->IsFooterShared());
                bool bShareFirst = pDesc->IsFirstShared();
                // TextLeft returns the left content if there is one,
                // Text and TextRight return the master content.
                // TextRight does the same as Text and is for
                // compatibility only.
                if(bLeft && !bShare)
                    pFrameFormat = &pDesc->GetLeft();
                else if(bFirst && !bShareFirst)
                {
                    pFrameFormat = &pDesc->GetFirstMaster();
                    // no need to make GetFirstLeft() accessible
                    // since it is always shared
                }
                else
                    pFrameFormat = &pDesc->GetMaster();
                const uno::Reference<text::XText> xRet = lcl_makeHeaderFooter(nRes, nRes == RES_HEADER, pFrameFormat);
                if (xRet.is())
                    aRet[nProp] = uno::makeAny(xRet);
            }
            break;
            case FN_PARAM_FTN_INFO:
            {
                rtl::Reference<SwDocStyleSheet> xStyle(new SwDocStyleSheet(*static_cast<SwDocStyleSheet*>(pBase)));
                const SfxItemSet& rSet = xStyle->GetItemSet();
                const SfxPoolItem& rItem = rSet.Get(FN_PARAM_FTN_INFO);
                const sal_uInt8 nMemberId(pEntry->nMemberId & (~SFX_METRIC_ITEM));
                rItem.QueryValue(aRet[nProp], nMemberId);
            }
            break;
            default:
                aRet[nProp] = GetStyleProperty_Impl(*pEntry, *pPropSet, aBase);
        }
    }
    return aRet;
}

uno::Sequence<uno::Any> SwXPageStyle::getPropertyValues(const uno::Sequence<OUString>& rPropertyNames)
        throw(uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    uno::Sequence<uno::Any> aValues;

    // workaround for bad designed API
    try
    {
        aValues = GetPropertyValues_Impl(rPropertyNames);
    }
    catch(beans::UnknownPropertyException &)
    {
        throw uno::RuntimeException("Unknown property exception caught", static_cast<cppu::OWeakObject*>(this));
    }
    catch(lang::WrappedTargetException &)
    {
        throw uno::RuntimeException("WrappedTargetException caught", static_cast<cppu::OWeakObject*>(this));
    }

    return aValues;
}

uno::Any SwXPageStyle::getPropertyValue(const OUString& rPropertyName)
        throw(beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    const uno::Sequence<OUString> aProperties(&rPropertyName, 1);
    return GetPropertyValues_Impl(aProperties)[0];
}

void SwXPageStyle::setPropertyValue(const OUString& rPropertyName, const uno::Any& rValue)
        throw(beans::UnknownPropertyException, beans::PropertyVetoException, lang::IllegalArgumentException, lang::WrappedTargetException, uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    const uno::Sequence<OUString> aProperties(&rPropertyName, 1);
    const uno::Sequence<uno::Any> aValues(&rValue, 1);
    SetPropertyValues_Impl(aProperties, aValues);
}

SwXFrameStyle::SwXFrameStyle(SwDoc *pDoc)
    : SwXStyle(pDoc, SfxStyleFamily::Frame, false)
{ }

SwXFrameStyle::~SwXFrameStyle()
{ }

void SwXFrameStyle::SetItem(enum RES_FRMATR eAtr, const SfxPoolItem& rItem)
{
    SfxStyleSheetBase* pBase = GetStyleSheetBase();
    if(!pBase)
        return;
    rtl::Reference<SwDocStyleSheet> xStyle(new SwDocStyleSheet(*static_cast<SwDocStyleSheet*>(pBase)));
    SfxItemSet& rStyleSet = xStyle->GetItemSet();
    SfxItemSet aSet(*rStyleSet.GetPool(), eAtr, eAtr);
    aSet.Put(rItem);
    xStyle->SetItemSet(aSet);
}

const SfxPoolItem* SwXFrameStyle::GetItem(enum RES_FRMATR eAtr)
{
    SfxStyleSheetBase* pBase = GetStyleSheetBase();
    if(!pBase)
        return nullptr;
    rtl::Reference<SwDocStyleSheet> xStyle(new SwDocStyleSheet(*static_cast<SwDocStyleSheet*>(pBase)));
    return &xStyle->GetItemSet().Get(eAtr);
}

uno::Sequence<uno::Type> SwXFrameStyle::getTypes()
        throw(uno::RuntimeException, std::exception)
{
    uno::Sequence<uno::Type> aTypes = SwXStyle::getTypes();
    sal_Int32 nLen = aTypes.getLength();
    aTypes.realloc(nLen + 1);
    aTypes[nLen] = cppu::UnoType<XEventsSupplier>::get();
    return aTypes;
}

uno::Any SwXFrameStyle::queryInterface(const uno::Type& rType)
        throw(uno::RuntimeException, std::exception)
{
    if(rType == cppu::UnoType<XEventsSupplier>::get())
        return uno::makeAny(uno::Reference<XEventsSupplier>(this));
    return SwXStyle::queryInterface(rType);
}

uno::Reference<container::XNameReplace> SwXFrameStyle::getEvents()
        throw(uno::RuntimeException, std::exception)
{
    return new SwFrameStyleEventDescriptor(*this);
}

// Already implemented autostyle families: 3
#define AUTOSTYLE_FAMILY_COUNT 3
const IStyleAccess::SwAutoStyleFamily aAutoStyleByIndex[] =
{
    IStyleAccess::AUTO_STYLE_CHAR,
    IStyleAccess::AUTO_STYLE_RUBY,
    IStyleAccess::AUTO_STYLE_PARA
};

class SwAutoStylesEnumImpl
{
    std::vector<SfxItemSet_Pointer_t> mAutoStyles;
    std::vector<SfxItemSet_Pointer_t>::iterator aIter;
    SwDoc* pDoc;
    IStyleAccess::SwAutoStyleFamily eFamily;
public:
    SwAutoStylesEnumImpl( SwDoc* pInitDoc, IStyleAccess::SwAutoStyleFamily eFam );
    bool hasMoreElements() { return aIter != mAutoStyles.end(); }
    SfxItemSet_Pointer_t nextElement() { return *(aIter++); }
    IStyleAccess::SwAutoStyleFamily getFamily() const { return eFamily; }
    SwDoc* getDoc() const { return pDoc; }
};

SwXAutoStyles::SwXAutoStyles(SwDocShell& rDocShell) :
    SwUnoCollection(rDocShell.GetDoc()), m_pDocShell( &rDocShell )
{
}

SwXAutoStyles::~SwXAutoStyles()
{
}

sal_Int32 SwXAutoStyles::getCount() throw( uno::RuntimeException, std::exception )
{
    return AUTOSTYLE_FAMILY_COUNT;
}

uno::Any SwXAutoStyles::getByIndex(sal_Int32 nIndex)
        throw( lang::IndexOutOfBoundsException, lang::WrappedTargetException,
                uno::RuntimeException, std::exception )
{
    SolarMutexGuard aGuard;
    uno::Any aRet;
    if(nIndex < 0 || nIndex >= AUTOSTYLE_FAMILY_COUNT)
        throw lang::IndexOutOfBoundsException();
    if(IsValid())
    {
        uno::Reference< style::XAutoStyleFamily >  aRef;
        IStyleAccess::SwAutoStyleFamily nType = aAutoStyleByIndex[nIndex];
        switch( nType )
        {
            case IStyleAccess::AUTO_STYLE_CHAR:
            {
                if(!m_xAutoCharStyles.is())
                    m_xAutoCharStyles = new SwXAutoStyleFamily(m_pDocShell, nType);
                aRef = m_xAutoCharStyles;
            }
            break;
            case IStyleAccess::AUTO_STYLE_RUBY:
            {
                if(!m_xAutoRubyStyles.is())
                    m_xAutoRubyStyles = new SwXAutoStyleFamily(m_pDocShell, nType );
                aRef = m_xAutoRubyStyles;
            }
            break;
            case IStyleAccess::AUTO_STYLE_PARA:
            {
                if(!m_xAutoParaStyles.is())
                    m_xAutoParaStyles = new SwXAutoStyleFamily(m_pDocShell, nType );
                aRef = m_xAutoParaStyles;
            }
            break;

            default:
                ;
        }
        aRet.setValue(&aRef, cppu::UnoType<style::XAutoStyleFamily>::get());
    }
    else
        throw uno::RuntimeException();
    return aRet;
}

uno::Type SwXAutoStyles::getElementType(  ) throw(uno::RuntimeException, std::exception)
{
    return cppu::UnoType<style::XAutoStyleFamily>::get();
}

sal_Bool SwXAutoStyles::hasElements(  ) throw(uno::RuntimeException, std::exception)
{
    return true;
}

uno::Any SwXAutoStyles::getByName(const OUString& Name)
        throw( container::NoSuchElementException, lang::WrappedTargetException, uno::RuntimeException, std::exception )
{
    uno::Any aRet;
    if(Name == "CharacterStyles")
        aRet = getByIndex(0);
    else if(Name == "RubyStyles")
        aRet = getByIndex(1);
    else if(Name == "ParagraphStyles")
        aRet = getByIndex(2);
    else
        throw container::NoSuchElementException();
    return aRet;
}

uno::Sequence< OUString > SwXAutoStyles::getElementNames()
            throw( uno::RuntimeException, std::exception )
{
    uno::Sequence< OUString > aNames(AUTOSTYLE_FAMILY_COUNT);
    OUString* pNames = aNames.getArray();
    pNames[0] = "CharacterStyles";
    pNames[1] = "RubyStyles";
    pNames[2] = "ParagraphStyles";
    return aNames;
}

sal_Bool SwXAutoStyles::hasByName(const OUString& Name)
            throw( uno::RuntimeException, std::exception )
{
    if( Name == "CharacterStyles" ||
        Name == "RubyStyles" ||
        Name == "ParagraphStyles" )
        return true;
    else
        return false;
}

SwXAutoStyleFamily::SwXAutoStyleFamily(SwDocShell* pDocSh, IStyleAccess::SwAutoStyleFamily nFamily) :
    m_pDocShell( pDocSh ), m_eFamily(nFamily)
{
    // Register ourselves as a listener to the document (via the page descriptor)
    pDocSh->GetDoc()->getIDocumentStylePoolAccess().GetPageDescFromPool(RES_POOLPAGE_STANDARD)->Add(this);
}

SwXAutoStyleFamily::~SwXAutoStyleFamily()
{
}

void SwXAutoStyleFamily::Modify( const SfxPoolItem* pOld, const SfxPoolItem *pNew)
{
    ClientModify(this, pOld, pNew);
    if(!GetRegisteredIn())
        m_pDocShell = nullptr;
}

uno::Reference< style::XAutoStyle > SwXAutoStyleFamily::insertStyle(
    const uno::Sequence< beans::PropertyValue >& Values )
        throw (uno::RuntimeException, std::exception)
{
    if (!m_pDocShell)
    {
        throw uno::RuntimeException();
    }

    const sal_uInt16* pRange = nullptr;
    const SfxItemPropertySet* pPropSet = nullptr;
    switch( m_eFamily )
    {
        case IStyleAccess::AUTO_STYLE_CHAR:
        {
            pRange = aCharAutoFormatSetRange;
            pPropSet = aSwMapProvider.GetPropertySet(PROPERTY_MAP_CHAR_AUTO_STYLE);
            break;
        }
        case IStyleAccess::AUTO_STYLE_RUBY:
        {
            pRange = nullptr;//aTextNodeSetRange;
            pPropSet = aSwMapProvider.GetPropertySet(PROPERTY_MAP_RUBY_AUTO_STYLE);
            break;
        }
        case IStyleAccess::AUTO_STYLE_PARA:
        {
            pRange = aTextNodeSetRange; //UUUU checked, already added support for [XATTR_FILL_FIRST, XATTR_FILL_LAST]
            pPropSet = aSwMapProvider.GetPropertySet(PROPERTY_MAP_PARA_AUTO_STYLE);
            break;
        }
        default: ;
    }

    if( !pPropSet)
        throw uno::RuntimeException();

    SwAttrSet aSet( m_pDocShell->GetDoc()->GetAttrPool(), pRange );
    const beans::PropertyValue* pSeq = Values.getConstArray();
    sal_Int32 nLen = Values.getLength();
    const bool bTakeCareOfDrawingLayerFillStyle(IStyleAccess::AUTO_STYLE_PARA == m_eFamily);

    if(!bTakeCareOfDrawingLayerFillStyle)
    {
        for( sal_Int32 i = 0; i < nLen; ++i )
        {
            try
            {
                pPropSet->setPropertyValue( pSeq[i].Name, pSeq[i].Value, aSet );
            }
            catch (const beans::UnknownPropertyException &)
            {
                OSL_FAIL( "Unknown property" );
            }
            catch (const lang::IllegalArgumentException &)
            {
                OSL_FAIL( "Illegal argument" );
            }
        }
    }
    else
    {
        //UUUU set parent to ItemSet to ensure XFILL_NONE as XFillStyleItem
        // to make cases in RES_BACKGROUND work correct; target *is* a style
        // where this is the case
        aSet.SetParent(&m_pDocShell->GetDoc()->GetDfltTextFormatColl()->GetAttrSet());

        //UUUU here the used DrawingLayer FillStyles are imported when family is
        // equal to IStyleAccess::AUTO_STYLE_PARA, thus we will need to serve the
        // used slots functionality here to do this correctly
        const SfxItemPropertyMap& rMap = pPropSet->getPropertyMap();

        for( sal_Int32 i = 0; i < nLen; ++i )
        {
            const OUString& rPropName = pSeq[i].Name;
            uno::Any aValue(pSeq[i].Value);
            const SfxItemPropertySimpleEntry* pEntry = rMap.getByName(rPropName);

            if (!pEntry)
            {
                SAL_WARN("sw.core", "SwXAutoStyleFamily::insertStyle: Unknown property: " << rPropName);
                continue;
            }

            const sal_uInt8 nMemberId(pEntry->nMemberId & (~SFX_METRIC_ITEM));
            bool bDone(false);

            // check for needed metric translation
            if(pEntry->nMemberId & SFX_METRIC_ITEM)
            {
                bool bDoIt(true);

                if(XATTR_FILLBMP_SIZEX == pEntry->nWID || XATTR_FILLBMP_SIZEY == pEntry->nWID)
                {
                    // exception: If these ItemTypes are used, do not convert when these are negative
                    // since this means they are intended as percent values
                    sal_Int32 nValue = 0;

                    if(aValue >>= nValue)
                    {
                        bDoIt = nValue > 0;
                    }
                }

                if(bDoIt)
                {
                    const SfxItemPool& rPool = m_pDocShell->GetDoc()->GetAttrPool();
                    const SfxMapUnit eMapUnit(rPool.GetMetric(pEntry->nWID));

                    if(eMapUnit != SFX_MAPUNIT_100TH_MM)
                    {
                        SvxUnoConvertFromMM(eMapUnit, aValue);
                    }
                }
            }

            switch(pEntry->nWID)
            {
                case XATTR_FILLGRADIENT:
                case XATTR_FILLHATCH:
                case XATTR_FILLBITMAP:
                case XATTR_FILLFLOATTRANSPARENCE:
                // not yet needed; activate when LineStyle support may be added
                // case XATTR_LINESTART:
                // case XATTR_LINEEND:
                // case XATTR_LINEDASH:
                {
                    if(MID_NAME == nMemberId)
                    {
                        //UUUU add set commands for FillName items
                        OUString aTempName;

                        if(!(aValue >>= aTempName))
                        {
                            throw lang::IllegalArgumentException();
                        }

                        SvxShape::SetFillAttribute(pEntry->nWID, aTempName, aSet);
                        bDone = true;
                    }
                    else if(MID_GRAFURL == nMemberId)
                    {
                        if(XATTR_FILLBITMAP == pEntry->nWID)
                        {
                            //UUUU Bitmap also has the MID_GRAFURL mode where a Bitmap URL is used
                            const Graphic aNullGraphic;
                            XFillBitmapItem aXFillBitmapItem(aSet.GetPool(), aNullGraphic);

                            aXFillBitmapItem.PutValue(aValue, nMemberId);
                            aSet.Put(aXFillBitmapItem);
                            bDone = true;
                        }
                    }

                    break;
                }
                case RES_BACKGROUND:
                {
                    //UUUU
                    const SvxBrushItem aOriginalBrushItem(getSvxBrushItemFromSourceSet(aSet, RES_BACKGROUND, true, m_pDocShell->GetDoc()->IsInXMLImport()));
                    SvxBrushItem aChangedBrushItem(aOriginalBrushItem);

                    aChangedBrushItem.PutValue(aValue, nMemberId);

                    if(!(aChangedBrushItem == aOriginalBrushItem))
                    {
                        setSvxBrushItemAsFillAttributesToTargetSet(aChangedBrushItem, aSet);
                    }

                    bDone = true;
                    break;
                }
                case OWN_ATTR_FILLBMP_MODE:
                {
                    //UUUU
                    drawing::BitmapMode eMode;

                    if(!(aValue >>= eMode))
                    {
                        sal_Int32 nMode = 0;

                        if(!(aValue >>= nMode))
                        {
                            throw lang::IllegalArgumentException();
                        }

                        eMode = (drawing::BitmapMode)nMode;
                    }

                    aSet.Put(XFillBmpStretchItem(drawing::BitmapMode_STRETCH == eMode));
                    aSet.Put(XFillBmpTileItem(drawing::BitmapMode_REPEAT == eMode));

                    bDone = true;
                    break;
                }
                default: break;
            }

            if(!bDone)
            {
                try
                {
                    pPropSet->setPropertyValue( rPropName, aValue, aSet );
                }
                catch (const beans::UnknownPropertyException &)
                {
                    OSL_FAIL( "Unknown property" );
                }
                catch (const lang::IllegalArgumentException &)
                {
                    OSL_FAIL( "Illegal argument" );
                }
            }
         }

        //UUUU clear parent again
        aSet.SetParent(nullptr);
    }

    //UUUU need to ensure uniqueness of evtl. added NameOrIndex items
    // currently in principle only needed when bTakeCareOfDrawingLayerFillStyle,
    // but does not hurt and is easily forgotten later eventually, so keep it
    // as common case
    m_pDocShell->GetDoc()->CheckForUniqueItemForLineFillNameOrIndex(aSet);

    // AutomaticStyle creation
    SfxItemSet_Pointer_t pSet = m_pDocShell->GetDoc()->GetIStyleAccess().cacheAutomaticStyle( aSet, m_eFamily );
    uno::Reference<style::XAutoStyle> xRet = new SwXAutoStyle(m_pDocShell->GetDoc(), pSet, m_eFamily);

    return xRet;
}

uno::Reference< container::XEnumeration > SwXAutoStyleFamily::createEnumeration(  )
        throw (uno::RuntimeException, std::exception)
{
    if( !m_pDocShell )
        throw uno::RuntimeException();
    return uno::Reference< container::XEnumeration >
        (new SwXAutoStylesEnumerator( m_pDocShell->GetDoc(), m_eFamily ));
}

uno::Type SwXAutoStyleFamily::getElementType(  ) throw(uno::RuntimeException, std::exception)
{
    return cppu::UnoType<style::XAutoStyle>::get();
}

sal_Bool SwXAutoStyleFamily::hasElements(  ) throw(uno::RuntimeException, std::exception)
{
    return false;
}

SwAutoStylesEnumImpl::SwAutoStylesEnumImpl( SwDoc* pInitDoc, IStyleAccess::SwAutoStyleFamily eFam )
: pDoc( pInitDoc ), eFamily( eFam )
{
    // special case for ruby auto styles:
    if ( IStyleAccess::AUTO_STYLE_RUBY == eFam )
    {
        std::set< std::pair< sal_uInt16, sal_uInt16 > > aRubyMap;
        SwAttrPool& rAttrPool = pDoc->GetAttrPool();
        sal_uInt32 nCount = rAttrPool.GetItemCount2( RES_TXTATR_CJK_RUBY );

        for ( sal_uInt32 nI = 0; nI < nCount; ++nI )
        {
            const SwFormatRuby* pItem = static_cast<const SwFormatRuby*>(rAttrPool.GetItem2( RES_TXTATR_CJK_RUBY, nI ));
            if ( pItem && pItem->GetTextRuby() )
            {
                std::pair< sal_uInt16, sal_uInt16 > aPair( pItem->GetPosition(), pItem->GetAdjustment() );
                if ( aRubyMap.find( aPair ) == aRubyMap.end() )
                {
                    aRubyMap.insert( aPair );
                    SfxItemSet_Pointer_t pItemSet( new SfxItemSet( rAttrPool, RES_TXTATR_CJK_RUBY, RES_TXTATR_CJK_RUBY ) );
                    pItemSet->Put( *pItem );
                    mAutoStyles.push_back( pItemSet );
                }
            }
        }
    }
    else
    {
        pDoc->GetIStyleAccess().getAllStyles( mAutoStyles, eFamily );
    }

    aIter = mAutoStyles.begin();
}

SwXAutoStylesEnumerator::SwXAutoStylesEnumerator( SwDoc* pDoc, IStyleAccess::SwAutoStyleFamily eFam )
: m_pImpl( new SwAutoStylesEnumImpl( pDoc, eFam ) )
{
    // Register ourselves as a listener to the document (via the page descriptor)
    pDoc->getIDocumentStylePoolAccess().GetPageDescFromPool(RES_POOLPAGE_STANDARD)->Add(this);
}

SwXAutoStylesEnumerator::~SwXAutoStylesEnumerator()
{
    delete m_pImpl;
}

void SwXAutoStylesEnumerator::Modify( const SfxPoolItem* pOld, const SfxPoolItem *pNew)
{
    ClientModify(this, pOld, pNew);
    if(!GetRegisteredIn())
    {
        delete m_pImpl;
        m_pImpl = nullptr;
    }
}

sal_Bool SwXAutoStylesEnumerator::hasMoreElements(  )
    throw (uno::RuntimeException, std::exception)
{
    if( !m_pImpl )
        throw uno::RuntimeException();
    return m_pImpl->hasMoreElements();
}

uno::Any SwXAutoStylesEnumerator::nextElement(  )
    throw (container::NoSuchElementException, lang::WrappedTargetException, uno::RuntimeException, std::exception)
{
    if( !m_pImpl )
        throw uno::RuntimeException();
    uno::Any aRet;
    if( m_pImpl->hasMoreElements() )
    {
        SfxItemSet_Pointer_t pNextSet = m_pImpl->nextElement();
        uno::Reference< style::XAutoStyle > xAutoStyle = new SwXAutoStyle(m_pImpl->getDoc(),
                                                        pNextSet, m_pImpl->getFamily());
        aRet.setValue(&xAutoStyle, cppu::UnoType<style::XAutoStyle>::get());
    }
    return aRet;
}

//UUUU SwXAutoStyle with the family IStyleAccess::AUTO_STYLE_PARA (or
// PROPERTY_MAP_PARA_AUTO_STYLE) now uses DrawingLayer FillStyles to allow
// unified paragraph background fill, thus the UNO API implementation has to
// support the needed slots for these. This seems to be used only for reading
// (no setPropertyValue implementation here), so maybe specialized for saving
// the Writer Doc to ODF

SwXAutoStyle::SwXAutoStyle(
    SwDoc* pDoc,
    SfxItemSet_Pointer_t pInitSet,
    IStyleAccess::SwAutoStyleFamily eFam)
:   mpSet(pInitSet),
    meFamily(eFam),
    mrDoc(*pDoc)
{
    // Register ourselves as a listener to the document (via the page descriptor)
    mrDoc.getIDocumentStylePoolAccess().GetPageDescFromPool(RES_POOLPAGE_STANDARD)->Add(this);
}

SwXAutoStyle::~SwXAutoStyle()
{
}

void SwXAutoStyle::Modify( const SfxPoolItem* pOld, const SfxPoolItem *pNew)
{
    ClientModify(this, pOld, pNew);
    if(!GetRegisteredIn())
    {
        mpSet.reset();
    }
}

uno::Reference< beans::XPropertySetInfo > SwXAutoStyle::getPropertySetInfo(  )
                throw (uno::RuntimeException, std::exception)
{
    uno::Reference< beans::XPropertySetInfo >  xRet;
    switch( meFamily )
    {
        case IStyleAccess::AUTO_STYLE_CHAR:
        {
            static uno::Reference< beans::XPropertySetInfo >  xCharRef;
            if(!xCharRef.is())
            {
                xCharRef = aSwMapProvider.GetPropertySet(PROPERTY_MAP_CHAR_AUTO_STYLE)->getPropertySetInfo();
            }
            xRet = xCharRef;
        }
        break;
        case IStyleAccess::AUTO_STYLE_RUBY:
        {
            static uno::Reference< beans::XPropertySetInfo >  xRubyRef;
            if(!xRubyRef.is())
            {
                const sal_uInt16 nMapId = PROPERTY_MAP_RUBY_AUTO_STYLE;
                xRubyRef = aSwMapProvider.GetPropertySet(nMapId)->getPropertySetInfo();
            }
            xRet = xRubyRef;
        }
        break;
        case IStyleAccess::AUTO_STYLE_PARA:
        {
            static uno::Reference< beans::XPropertySetInfo >  xParaRef;
            if(!xParaRef.is())
            {
                const sal_uInt16 nMapId = PROPERTY_MAP_PARA_AUTO_STYLE;
                xParaRef = aSwMapProvider.GetPropertySet(nMapId)->getPropertySetInfo();
            }
            xRet = xParaRef;
        }
        break;

        default:
            ;
    }

    return xRet;
}

void SwXAutoStyle::setPropertyValue( const OUString& /*rPropertyName*/, const uno::Any& /*rValue*/ )
     throw( beans::UnknownPropertyException,
            beans::PropertyVetoException,
            lang::IllegalArgumentException,
            lang::WrappedTargetException,
            uno::RuntimeException, std::exception)
{
}

uno::Any SwXAutoStyle::getPropertyValue( const OUString& rPropertyName )
    throw( beans::UnknownPropertyException,
           lang::WrappedTargetException,
           uno::RuntimeException, std::exception )
{
    SolarMutexGuard aGuard;
    const uno::Sequence<OUString> aProperties(&rPropertyName, 1);
    return GetPropertyValues_Impl(aProperties).getConstArray()[0];
}

void SwXAutoStyle::addPropertyChangeListener( const OUString& /*aPropertyName*/,
                                              const uno::Reference< beans::XPropertyChangeListener >& /*xListener*/ )
    throw( beans::UnknownPropertyException,
           lang::WrappedTargetException,
           uno::RuntimeException, std::exception )
{
}

void SwXAutoStyle::removePropertyChangeListener( const OUString& /*aPropertyName*/,
                                                 const uno::Reference< beans::XPropertyChangeListener >& /*aListener*/ )
    throw( beans::UnknownPropertyException,
           lang::WrappedTargetException,
           uno::RuntimeException, std::exception )
{
}

void SwXAutoStyle::addVetoableChangeListener( const OUString& /*PropertyName*/,
                                              const uno::Reference< beans::XVetoableChangeListener >& /*aListener*/ )
    throw( beans::UnknownPropertyException,
           lang::WrappedTargetException,
           uno::RuntimeException, std::exception )
{
}

void SwXAutoStyle::removeVetoableChangeListener( const OUString& /*PropertyName*/,
                                                 const uno::Reference< beans::XVetoableChangeListener >& /*aListener*/ )
    throw( beans::UnknownPropertyException,
           lang::WrappedTargetException,
           uno::RuntimeException, std::exception )
{
}

void SwXAutoStyle::setPropertyValues(
        const uno::Sequence< OUString >& /*aPropertyNames*/,
        const uno::Sequence< uno::Any >& /*aValues*/ )
            throw (beans::PropertyVetoException, lang::IllegalArgumentException,
                lang::WrappedTargetException, uno::RuntimeException, std::exception)
{
}

uno::Sequence< uno::Any > SwXAutoStyle::GetPropertyValues_Impl(
        const uno::Sequence< OUString > & rPropertyNames )
    throw( beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException )
{
    if( !mpSet.get() )
    {
        throw uno::RuntimeException();
    }

    // query_item
    sal_Int8 nPropSetId = PROPERTY_MAP_CHAR_AUTO_STYLE;
    switch(meFamily)
    {
        case IStyleAccess::AUTO_STYLE_CHAR  : nPropSetId = PROPERTY_MAP_CHAR_AUTO_STYLE;  break;
        case IStyleAccess::AUTO_STYLE_RUBY  : nPropSetId = PROPERTY_MAP_RUBY_AUTO_STYLE;  break;
        case IStyleAccess::AUTO_STYLE_PARA  : nPropSetId = PROPERTY_MAP_PARA_AUTO_STYLE;  break;
        default: ;
    }

    const SfxItemPropertySet* pPropSet = aSwMapProvider.GetPropertySet(nPropSetId);
    const SfxItemPropertyMap& rMap = pPropSet->getPropertyMap();
    const OUString* pNames = rPropertyNames.getConstArray();

    const sal_Int32 nLen(rPropertyNames.getLength());
    uno::Sequence< uno::Any > aRet( nLen );
    uno::Any* pValues = aRet.getArray();
    const bool bTakeCareOfDrawingLayerFillStyle(IStyleAccess::AUTO_STYLE_PARA == meFamily);

    for( sal_Int32 i = 0; i < nLen; ++i )
    {
        const OUString sPropName = pNames[i];
        const SfxItemPropertySimpleEntry* pEntry = rMap.getByName(sPropName);
        if(!pEntry)
        {
            throw beans::UnknownPropertyException("Unknown property: " + sPropName, static_cast < cppu::OWeakObject * > ( this ) );
        }

        uno::Any aTarget;
        bool bDone(false);

        if ( RES_TXTATR_AUTOFMT == pEntry->nWID || RES_AUTO_STYLE == pEntry->nWID )
        {
            OUString sName(StylePool::nameOf( mpSet ));
            aTarget <<= sName;
            bDone = true;
        }
        else if(bTakeCareOfDrawingLayerFillStyle)
        {
            //UUUU add support for DrawingLayer FillStyle slots
            switch(pEntry->nWID)
            {
                case RES_BACKGROUND:
                {
                    const SvxBrushItem aOriginalBrushItem(getSvxBrushItemFromSourceSet(*mpSet, RES_BACKGROUND));
                    const sal_uInt8 nMemberId(pEntry->nMemberId & (~SFX_METRIC_ITEM));

                    if(!aOriginalBrushItem.QueryValue(aTarget, nMemberId))
                    {
                        OSL_ENSURE(false, "Error getting attribute from RES_BACKGROUND (!)");
                    }

                    bDone = true;
                    break;
                }
                case OWN_ATTR_FILLBMP_MODE:
                {
                    const XFillBmpStretchItem* pStretchItem = dynamic_cast< const XFillBmpStretchItem* >(&mpSet->Get(XATTR_FILLBMP_STRETCH));
                    const XFillBmpTileItem* pTileItem = dynamic_cast< const XFillBmpTileItem* >(&mpSet->Get(XATTR_FILLBMP_TILE));

                    if( pTileItem && pTileItem->GetValue() )
                    {
                        aTarget <<= drawing::BitmapMode_REPEAT;
                    }
                    else if( pStretchItem && pStretchItem->GetValue() )
                    {
                        aTarget <<= drawing::BitmapMode_STRETCH;
                    }
                    else
                    {
                        aTarget <<= drawing::BitmapMode_NO_REPEAT;
                    }

                    bDone = true;
                    break;
                }
            }
        }

        if(!bDone)
        {
            pPropSet->getPropertyValue( *pEntry, *mpSet, aTarget );
        }

        if(bTakeCareOfDrawingLayerFillStyle)
        {
            if(pEntry->aType == cppu::UnoType<sal_Int16>::get() && pEntry->aType != aTarget.getValueType())
            {
                // since the sfx uint16 item now exports a sal_Int32, we may have to fix this here
                sal_Int32 nValue = 0;
                aTarget >>= nValue;
                aTarget <<= (sal_Int16)nValue;
            }

            // check for needed metric translation
            if(pEntry->nMemberId & SFX_METRIC_ITEM)
            {
                bool bDoIt(true);

                if(XATTR_FILLBMP_SIZEX == pEntry->nWID || XATTR_FILLBMP_SIZEY == pEntry->nWID)
                {
                    // exception: If these ItemTypes are used, do not convert when these are negative
                    // since this means they are intended as percent values
                    sal_Int32 nValue = 0;

                    if(aTarget >>= nValue)
                    {
                        bDoIt = nValue > 0;
                    }
                }

                if(bDoIt)
                {
                    const SfxItemPool& rPool = mrDoc.GetAttrPool();
                    const SfxMapUnit eMapUnit(rPool.GetMetric(pEntry->nWID));

                    if(eMapUnit != SFX_MAPUNIT_100TH_MM)
                    {
                        SvxUnoConvertToMM(eMapUnit, aTarget);
                    }
                }
            }
        }

        // add value
        pValues[i] = aTarget;
    }

    return aRet;
}

uno::Sequence< uno::Any > SwXAutoStyle::getPropertyValues (
        const uno::Sequence< OUString >& rPropertyNames )
            throw (uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    uno::Sequence< uno::Any > aValues;

    // workaround for bad designed API
    try
    {
        aValues = GetPropertyValues_Impl( rPropertyNames );
    }
    catch (beans::UnknownPropertyException &)
    {
        throw uno::RuntimeException("Unknown property exception caught", static_cast < cppu::OWeakObject * > ( this ) );
    }
    catch (lang::WrappedTargetException &)
    {
        throw uno::RuntimeException("WrappedTargetException caught", static_cast < cppu::OWeakObject * > ( this ) );
    }

    return aValues;
}

void SwXAutoStyle::addPropertiesChangeListener(
        const uno::Sequence< OUString >& /*aPropertyNames*/,
        const uno::Reference< beans::XPropertiesChangeListener >& /*xListener*/ )
            throw (uno::RuntimeException, std::exception)
{
}

void SwXAutoStyle::removePropertiesChangeListener(
        const uno::Reference< beans::XPropertiesChangeListener >& /*xListener*/ )
            throw (uno::RuntimeException, std::exception)
{
}

void SwXAutoStyle::firePropertiesChangeEvent(
        const uno::Sequence< OUString >& /*aPropertyNames*/,
        const uno::Reference< beans::XPropertiesChangeListener >& /*xListener*/ )
            throw (uno::RuntimeException, std::exception)
{
}

beans::PropertyState SwXAutoStyle::getPropertyState( const OUString& rPropertyName )
    throw( beans::UnknownPropertyException,
           uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;

    uno::Sequence< OUString > aNames { rPropertyName };
    uno::Sequence< beans::PropertyState > aStates = getPropertyStates(aNames);
    return aStates.getConstArray()[0];
}

void SwXAutoStyle::setPropertyToDefault( const OUString& /*PropertyName*/ )
    throw( beans::UnknownPropertyException,
           uno::RuntimeException, std::exception )
{
}

uno::Any SwXAutoStyle::getPropertyDefault( const OUString& rPropertyName )
    throw( beans::UnknownPropertyException,
           lang::WrappedTargetException,
           uno::RuntimeException, std::exception)
{
    const uno::Sequence < OUString > aSequence ( &rPropertyName, 1 );
    return getPropertyDefaults ( aSequence ).getConstArray()[0];
}

uno::Sequence< beans::PropertyState > SwXAutoStyle::getPropertyStates(
        const uno::Sequence< OUString >& rPropertyNames )
            throw (beans::UnknownPropertyException, uno::RuntimeException, std::exception)
{
    if (!mpSet.get())
    {
        throw uno::RuntimeException();
    }

    SolarMutexGuard aGuard;
    uno::Sequence< beans::PropertyState > aRet(rPropertyNames.getLength());
    beans::PropertyState* pStates = aRet.getArray();
    const OUString* pNames = rPropertyNames.getConstArray();

    sal_Int8 nPropSetId = PROPERTY_MAP_CHAR_AUTO_STYLE;
    switch(meFamily)
    {
        case IStyleAccess::AUTO_STYLE_CHAR  : nPropSetId = PROPERTY_MAP_CHAR_AUTO_STYLE;  break;
        case IStyleAccess::AUTO_STYLE_RUBY  : nPropSetId = PROPERTY_MAP_RUBY_AUTO_STYLE;  break;
        case IStyleAccess::AUTO_STYLE_PARA  : nPropSetId = PROPERTY_MAP_PARA_AUTO_STYLE;  break;
        default: ;
    }

    const SfxItemPropertySet* pPropSet = aSwMapProvider.GetPropertySet(nPropSetId);
    const SfxItemPropertyMap& rMap = pPropSet->getPropertyMap();
    const bool bTakeCareOfDrawingLayerFillStyle(IStyleAccess::AUTO_STYLE_PARA == meFamily);

    for(sal_Int32 i = 0; i < rPropertyNames.getLength(); i++)
    {
        const OUString sPropName = pNames[i];
        const SfxItemPropertySimpleEntry* pEntry = rMap.getByName(sPropName);
        if(!pEntry)
        {
            throw beans::UnknownPropertyException("Unknown property: " + sPropName, static_cast < cppu::OWeakObject * > ( this ) );
        }

        bool bDone(false);

        if(bTakeCareOfDrawingLayerFillStyle)
        {
            //UUUU DrawingLayer PropertyStyle support
            switch(pEntry->nWID)
            {
                case OWN_ATTR_FILLBMP_MODE:
                {
                    if(SfxItemState::SET == mpSet->GetItemState(XATTR_FILLBMP_STRETCH, false)
                        || SfxItemState::SET == mpSet->GetItemState(XATTR_FILLBMP_TILE, false))
                    {
                        pStates[i] = beans::PropertyState_DIRECT_VALUE;
                    }
                    else
                    {
                        pStates[i] = beans::PropertyState_AMBIGUOUS_VALUE;
                    }

                    bDone = true;
                    break;
                }
                case RES_BACKGROUND:
                {
                    if (SWUnoHelper::needToMapFillItemsToSvxBrushItemTypes(*mpSet,
                            pEntry->nMemberId))
                    {
                        pStates[i] = beans::PropertyState_DIRECT_VALUE;
                    }
                    else
                    {
                        pStates[i] = beans::PropertyState_DEFAULT_VALUE;
                    }
                    bDone = true;

                    break;
                }
            }
        }

        if(!bDone)
        {
            pStates[i] = pPropSet->getPropertyState(*pEntry, *mpSet );
        }
    }

    return aRet;
}

void SwXAutoStyle::setAllPropertiesToDefault(  )
            throw (uno::RuntimeException, std::exception)
{
}

void SwXAutoStyle::setPropertiesToDefault(
        const uno::Sequence< OUString >& /*rPropertyNames*/ )
            throw (beans::UnknownPropertyException, uno::RuntimeException, std::exception)
{
}

uno::Sequence< uno::Any > SwXAutoStyle::getPropertyDefaults(
        const uno::Sequence< OUString >& /*aPropertyNames*/ )
            throw (beans::UnknownPropertyException, lang::WrappedTargetException,
                    uno::RuntimeException, std::exception)
{
    uno::Sequence< uno::Any > aRet(0);
    return aRet;
}

uno::Sequence< beans::PropertyValue > SwXAutoStyle::getProperties() throw (uno::RuntimeException, std::exception)
{
    if( !mpSet.get() )
        throw uno::RuntimeException();
    SolarMutexGuard aGuard;
    std::vector< beans::PropertyValue > aPropertyVector;

    sal_Int8 nPropSetId = 0;
    switch(meFamily)
    {
        case IStyleAccess::AUTO_STYLE_CHAR  : nPropSetId = PROPERTY_MAP_CHAR_AUTO_STYLE;  break;
        case IStyleAccess::AUTO_STYLE_RUBY  : nPropSetId = PROPERTY_MAP_RUBY_AUTO_STYLE;  break;
        case IStyleAccess::AUTO_STYLE_PARA  : nPropSetId = PROPERTY_MAP_PARA_AUTO_STYLE;  break;
        default: ;
    }

    const SfxItemPropertySet* pPropSet = aSwMapProvider.GetPropertySet(nPropSetId);
    const SfxItemPropertyMap &rMap = pPropSet->getPropertyMap();
    PropertyEntryVector_t aPropVector = rMap.getPropertyEntries();

    SfxItemSet& rSet = *mpSet.get();
    SfxItemIter aIter(rSet);
    const SfxPoolItem* pItem = aIter.FirstItem();

    while ( pItem )
    {
        const sal_uInt16 nWID = pItem->Which();

        // TODO: Optimize - and fix! the old iteration filled each WhichId
        // only once but there are more properties than WhichIds
        PropertyEntryVector_t::const_iterator aIt = aPropVector.begin();
        while( aIt != aPropVector.end() )
        {
            if ( aIt->nWID == nWID )
            {
                beans::PropertyValue aPropertyValue;
                aPropertyValue.Name = aIt->sName;
                pItem->QueryValue( aPropertyValue.Value, aIt->nMemberId );
                aPropertyVector.push_back( aPropertyValue );
            }
            ++aIt;
        }
        pItem = aIter.NextItem();
    }

    const sal_Int32 nCount = aPropertyVector.size();
    uno::Sequence< beans::PropertyValue > aRet( nCount );
    beans::PropertyValue* pProps = aRet.getArray();

    for ( int i = 0; i < nCount; ++i, pProps++ )
    {
        *pProps = aPropertyVector[i];
    }

    return aRet;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
