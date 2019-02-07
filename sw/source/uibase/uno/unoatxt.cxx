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

#include <sal/config.h>

#include <com/sun/star/io/IOException.hpp>
#include <com/sun/star/lang/IndexOutOfBoundsException.hpp>
#include <com/sun/star/lang/WrappedTargetRuntimeException.hpp>
#include <osl/diagnose.h>
#include <rtl/character.hxx>
#include <vcl/svapp.hxx>
#include <svtools/unoevent.hxx>
#include <svl/urihelper.hxx>
#include <sfx2/event.hxx>
#include <swtypes.hxx>
#include <glosdoc.hxx>
#include <shellio.hxx>
#include <initui.hxx>
#include <gloslst.hxx>
#include <unoatxt.hxx>
#include <unomap.hxx>
#include <unomid.h>
#include <unotextbodyhf.hxx>
#include <unotextrange.hxx>
#include <TextCursorHelper.hxx>
#include <swevent.hxx>
#include <doc.hxx>
#include <unocrsr.hxx>
#include <IMark.hxx>
#include <IDocumentContentOperations.hxx>
#include <IDocumentRedlineAccess.hxx>
#include <IDocumentFieldsAccess.hxx>
#include <IDocumentState.hxx>
#include <unoprnms.hxx>
#include <docsh.hxx>
#include <swmodule.hxx>
#include <swdll.hxx>
#include <svl/hint.hxx>
#include <svl/macitem.hxx>
#include <editeng/acorrcfg.hxx>
#include <comphelper/servicehelper.hxx>
#include <cppuhelper/exc_hlp.hxx>
#include <cppuhelper/supportsservice.hxx>

#include <memory>

using namespace ::com::sun::star;

SwXAutoTextContainer::SwXAutoTextContainer()
{
    pGlossaries = ::GetGlossaries();

}

SwXAutoTextContainer::~SwXAutoTextContainer()
{

}

sal_Int32 SwXAutoTextContainer::getCount()
{
    OSL_ENSURE(pGlossaries->GetGroupCnt() < static_cast<size_t>(SAL_MAX_INT32),
               "SwXAutoTextContainer::getCount: too many items");
    return static_cast<sal_Int32>(pGlossaries->GetGroupCnt());
}

uno::Any SwXAutoTextContainer::getByIndex(sal_Int32 nIndex)
{
    SolarMutexGuard aGuard;
    const size_t nCount = pGlossaries->GetGroupCnt();
    if ( nIndex < 0 || static_cast<size_t>(nIndex) >= nCount )
        throw lang::IndexOutOfBoundsException();
    return getByName(pGlossaries->GetGroupName( static_cast<size_t>(nIndex) ));
}

uno::Type SwXAutoTextContainer::getElementType()
{
    return cppu::UnoType<text::XAutoTextGroup>::get();

}

sal_Bool SwXAutoTextContainer::hasElements()
{
    // At least standard should always exists!
    return true;
}

uno::Any SwXAutoTextContainer::getByName(const OUString& GroupName)
{
    SolarMutexGuard aGuard;

    uno::Reference< text::XAutoTextGroup > xGroup;
    if ( pGlossaries && hasByName( GroupName ) )    // group name already known?
        // true = create group if not already available
        xGroup = pGlossaries->GetAutoTextGroup( GroupName );

    if ( !xGroup.is() )
        throw container::NoSuchElementException();

    return makeAny( xGroup );
}

uno::Sequence< OUString > SwXAutoTextContainer::getElementNames()
{
    SolarMutexGuard aGuard;
    const size_t nCount = pGlossaries->GetGroupCnt();
    OSL_ENSURE(nCount < static_cast<size_t>(SAL_MAX_INT32),
               "SwXAutoTextContainer::getElementNames: too many groups");

    uno::Sequence< OUString > aGroupNames(static_cast<sal_Int32>(nCount));
    OUString *pArr = aGroupNames.getArray();

    for ( size_t i = 0; i < nCount; ++i )
    {
        // The names will be passed without a path extension.
        pArr[i] = pGlossaries->GetGroupName(i).getToken(0, GLOS_DELIM);
    }
    return aGroupNames;
}
// Finds group names with or without path index.
sal_Bool SwXAutoTextContainer::hasByName(const OUString& Name)
{
    SolarMutexGuard aGuard;
    OUString sGroupName( pGlossaries->GetCompleteGroupName( Name ) );
    if(!sGroupName.isEmpty())
        return true;
    return false;
}

uno::Reference< text::XAutoTextGroup >  SwXAutoTextContainer::insertNewByName(
    const OUString& aGroupName)
{
    SolarMutexGuard aGuard;
    if(hasByName(aGroupName))
        throw container::ElementExistException();
    //check for non-ASCII characters
    if(aGroupName.isEmpty())
    {
        lang::IllegalArgumentException aIllegal;
        aIllegal.Message = "group name must not be empty";
        throw aIllegal;
    }
    for(sal_Int32 nPos = 0; nPos < aGroupName.getLength(); nPos++)
    {
        sal_Unicode cChar = aGroupName[nPos];
        if (rtl::isAsciiAlphanumeric(cChar) ||
            (cChar == '_') ||
            (cChar == 0x20) ||
            (cChar == GLOS_DELIM) )
        {
            continue;
        }
        lang::IllegalArgumentException aIllegal;
        aIllegal.Message = "group name must contain a-z, A-z, '_', ' ' only";
        throw aIllegal;
    }
    OUString sGroup(aGroupName);
    if (sGroup.indexOf(GLOS_DELIM)<0)
    {
        sGroup += OUStringLiteral1(GLOS_DELIM) + "0";
    }
    pGlossaries->NewGroupDoc(sGroup, sGroup.getToken(0, GLOS_DELIM));

    uno::Reference< text::XAutoTextGroup > xGroup = pGlossaries->GetAutoTextGroup( sGroup );
    OSL_ENSURE( xGroup.is(), "SwXAutoTextContainer::insertNewByName: no UNO object created? How this?" );
        // We just inserted the group into the glossaries, so why doesn't it exist?

    return xGroup;
}

void SwXAutoTextContainer::removeByName(const OUString& aGroupName)
{
    SolarMutexGuard aGuard;
    // At first find the name with path extension
    OUString sGroupName = pGlossaries->GetCompleteGroupName( aGroupName );
    if(sGroupName.isEmpty())
        throw container::NoSuchElementException();
    pGlossaries->DelGroupDoc(sGroupName);
}

OUString SwXAutoTextContainer::getImplementationName()
{
    return OUString("SwXAutoTextContainer" );
}

sal_Bool SwXAutoTextContainer::supportsService(const OUString& rServiceName)
{
    return cppu::supportsService(this, rServiceName);
}

uno::Sequence< OUString > SwXAutoTextContainer::getSupportedServiceNames()
{
    OUString sService("com.sun.star.text.AutoTextContainer");
    const uno::Sequence< OUString > aSeq( &sService, 1 );
    return aSeq;
}

namespace
{
    class theSwXAutoTextGroupUnoTunnelId : public rtl::Static< UnoTunnelIdInit, theSwXAutoTextGroupUnoTunnelId > {};
}

const uno::Sequence< sal_Int8 > & SwXAutoTextGroup::getUnoTunnelId()
{
    return theSwXAutoTextGroupUnoTunnelId::get().getSeq();
}

sal_Int64 SAL_CALL SwXAutoTextGroup::getSomething( const uno::Sequence< sal_Int8 >& rId )
{
    if( rId.getLength() == 16
        && 0 == memcmp( getUnoTunnelId().getConstArray(),
                                        rId.getConstArray(), 16 ) )
    {
            return sal::static_int_cast< sal_Int64 >( reinterpret_cast< sal_IntPtr >( this ));
    }
    return 0;
}

SwXAutoTextGroup::SwXAutoTextGroup(const OUString& rName,
            SwGlossaries*   pGlos) :
    pPropSet(aSwMapProvider.GetPropertySet(PROPERTY_MAP_AUTO_TEXT_GROUP)),
    pGlossaries(pGlos),
    sName(rName),
    m_sGroupName(rName)
{
    OSL_ENSURE( -1 != rName.indexOf( GLOS_DELIM ),
        "SwXAutoTextGroup::SwXAutoTextGroup: to be constructed with a complete name only!" );
}

SwXAutoTextGroup::~SwXAutoTextGroup()
{
}

uno::Sequence< OUString > SwXAutoTextGroup::getTitles()
{
    SolarMutexGuard aGuard;
    std::unique_ptr<SwTextBlocks> pGlosGroup(pGlossaries ? pGlossaries->GetGroupDoc(m_sGroupName) : nullptr);
    if (!pGlosGroup || pGlosGroup->GetError())
        throw uno::RuntimeException();
    const sal_uInt16 nCount = pGlosGroup->GetCount();

    uno::Sequence< OUString > aEntryTitles(nCount);
    OUString *pArr = aEntryTitles.getArray();

    for ( sal_uInt16 i = 0; i < nCount; i++ )
        pArr[i] = pGlosGroup->GetLongName(i);
    return aEntryTitles;
}

void SwXAutoTextGroup::renameByName(const OUString& aElementName,
    const OUString& aNewElementName, const OUString& aNewElementTitle)
{
    SolarMutexGuard aGuard;
    // throw exception only if the programmatic name is to be changed into an existing name
    if(aNewElementName != aElementName && hasByName(aNewElementName))
        throw container::ElementExistException();
    std::unique_ptr<SwTextBlocks> pGlosGroup(pGlossaries ? pGlossaries->GetGroupDoc(m_sGroupName) : nullptr);
    if(!pGlosGroup || pGlosGroup->GetError())
        throw uno::RuntimeException();

    const sal_uInt16 nIdx = pGlosGroup->GetIndex( aElementName);
    if(USHRT_MAX == nIdx)
        throw lang::IllegalArgumentException();
    OUString aNewShort(aNewElementName);
    OUString aNewName(aNewElementTitle);
    sal_uInt16 nOldLongIdx = pGlosGroup->GetLongIndex( aNewShort );
    sal_uInt16 nOldIdx = pGlosGroup->GetIndex( aNewName );

    if ((nOldLongIdx == USHRT_MAX || nOldLongIdx == nIdx)
        && (nOldIdx == USHRT_MAX || nOldIdx == nIdx))
    {
        pGlosGroup->Rename( nIdx, &aNewShort, &aNewName );
        if(pGlosGroup->GetError() != ERRCODE_NONE)
            throw io::IOException();
    }

}

static bool lcl_CopySelToDoc( SwDoc* pInsDoc, OTextCursorHelper* pxCursor, SwXTextRange* pxRange)
{
    OSL_ENSURE( pInsDoc, "no InsDoc");

    SwNodes& rNds = pInsDoc->GetNodes();

    SwNodeIndex aIdx( rNds.GetEndOfContent(), -1 );
    SwContentNode * pNd = aIdx.GetNode().GetContentNode();
    SwPosition aPos(aIdx, SwIndex(pNd, pNd ? pNd->Len() : 0));

    bool bRet = false;
    pInsDoc->getIDocumentFieldsAccess().LockExpFields();
    {
        SwDoc *const pDoc(pxCursor ? pxCursor->GetDoc() : &pxRange->GetDoc());
        SwPaM aPam(pDoc->GetNodes());
        SwPaM * pPam(nullptr);
        if(pxCursor)
        {
            pPam = pxCursor->GetPaM();
        }
        else
        {
            if (pxRange->GetPositions(aPam))
            {
                pPam = & aPam;
            }
        }
        if (!pPam) { return false; }
        bRet = pDoc->getIDocumentContentOperations().CopyRange( *pPam, aPos, /*bCopyAll=*/false, /*bCheckPos=*/true ) || bRet;
    }

    pInsDoc->getIDocumentFieldsAccess().UnlockExpFields();
    if( !pInsDoc->getIDocumentFieldsAccess().IsExpFieldsLocked() )
        pInsDoc->getIDocumentFieldsAccess().UpdateExpFields(nullptr, true);

    return bRet;
}

uno::Reference< text::XAutoTextEntry >  SwXAutoTextGroup::insertNewByName(const OUString& aName,
        const OUString& aTitle, const uno::Reference< text::XTextRange > & xTextRange)
{
    SolarMutexGuard aGuard;
    if(hasByName(aName))
        throw container::ElementExistException();
    if(!xTextRange.is())
        throw uno::RuntimeException();

    std::unique_ptr<SwTextBlocks> pGlosGroup;
    if (pGlossaries)
        pGlosGroup = pGlossaries->GetGroupDoc(m_sGroupName);
    const OUString& sShortName(aName);
    const OUString& sLongName(aTitle);
    if (pGlosGroup && !pGlosGroup->GetError())
    {
        uno::Reference<lang::XUnoTunnel> xRangeTunnel( xTextRange, uno::UNO_QUERY);
        SwXTextRange* pxRange = nullptr;
        OTextCursorHelper* pxCursor = nullptr;
        if(xRangeTunnel.is())
        {
            pxRange = reinterpret_cast<SwXTextRange*>(xRangeTunnel->getSomething(
                                    SwXTextRange::getUnoTunnelId()));
            pxCursor = reinterpret_cast<OTextCursorHelper*>(xRangeTunnel->getSomething(
                                    OTextCursorHelper::getUnoTunnelId()));
        }

        OUString sOnlyText;
        OUString* pOnlyText = nullptr;
        bool bNoAttr = !pxCursor && !pxRange;
        if(bNoAttr)
        {
            sOnlyText = xTextRange->getString();
            pOnlyText = &sOnlyText;
        }

        const SvxAutoCorrCfg& rCfg = SvxAutoCorrCfg::Get();

        SwDoc* pGDoc = pGlosGroup->GetDoc();

        // Until there is an option for that, delete base util::URL
        if(rCfg.IsSaveRelFile())
        {
            INetURLObject aTemp(pGlosGroup->GetFileName());
            pGlosGroup->SetBaseURL( aTemp.GetMainURL(INetURLObject::DecodeMechanism::NONE));
        }
        else
            pGlosGroup->SetBaseURL( OUString() );

        sal_uInt16 nRet = USHRT_MAX;
        if( pOnlyText )
            nRet = pGlosGroup->PutText( sShortName, sLongName, *pOnlyText );
        else
        {
            pGlosGroup->ClearDoc();
            if( pGlosGroup->BeginPutDoc( sShortName, sLongName ) )
            {
                pGDoc->getIDocumentRedlineAccess().SetRedlineFlags_intern( RedlineFlags::DeleteRedlines );
                lcl_CopySelToDoc( pGDoc, pxCursor, pxRange );
                pGDoc->getIDocumentRedlineAccess().SetRedlineFlags_intern(RedlineFlags::NONE);
                nRet = pGlosGroup->PutDoc();
            }
        }

        if (nRet == USHRT_MAX)
        {
            throw uno::RuntimeException();
        }
    }
    pGlosGroup.reset();

    uno::Reference< text::XAutoTextEntry > xEntry;

    try
    {
        xEntry = pGlossaries ?
            pGlossaries->GetAutoTextEntry( m_sGroupName, sName, sShortName ) :
            uno::Reference< text::XAutoTextEntry >();
        OSL_ENSURE( xEntry.is(), "SwXAutoTextGroup::insertNewByName: no UNO object created? How this?" );
            // we just inserted the entry into the group, so why doesn't it exist?
    }
    catch (const container::ElementExistException&)
    {
        throw;
    }
    catch (const uno::RuntimeException&)
    {
        throw;
    }
    catch (const uno::Exception&)
    {
        css::uno::Any anyEx = cppu::getCaughtException();
        throw css::lang::WrappedTargetRuntimeException(
               "Error Getting AutoText!",
               static_cast < OWeakObject * > ( this ),
               anyEx );
    }

    return xEntry;
}

void SwXAutoTextGroup::removeByName(const OUString& aEntryName)
{
    SolarMutexGuard aGuard;
    std::unique_ptr<SwTextBlocks> pGlosGroup(pGlossaries ? pGlossaries->GetGroupDoc(m_sGroupName) : nullptr);
    if(!pGlosGroup || pGlosGroup->GetError())
        throw container::NoSuchElementException();

    sal_uInt16 nIdx = pGlosGroup->GetIndex(aEntryName);
    if ( nIdx == USHRT_MAX )
        throw container::NoSuchElementException();

    pGlosGroup->Delete(nIdx);
}

OUString SwXAutoTextGroup::getName()
{
    SolarMutexGuard aGuard;
    return sName;
}

void SwXAutoTextGroup::setName(const OUString& rName)
{
    SolarMutexGuard aGuard;
    if( !pGlossaries )
        throw uno::RuntimeException();

    sal_Int32 nNewDelimPos = rName.lastIndexOf( GLOS_DELIM );
    sal_Int32 nOldDelimPos = sName.lastIndexOf( GLOS_DELIM );

    OUString aNewSuffix;
    if (nNewDelimPos > -1)
        aNewSuffix = rName.copy( nNewDelimPos + 1 );
    OUString aOldSuffix;
    if (nOldDelimPos > -1)
        aOldSuffix = sName.copy( nOldDelimPos + 1 );

    sal_Int32 nNewNumeric = aNewSuffix.toInt32();
    sal_Int32 nOldNumeric = aOldSuffix.toInt32();

    OUString aNewPrefix( (nNewDelimPos > 1) ? rName.copy( 0, nNewDelimPos ) : rName );
    OUString aOldPrefix( (nOldDelimPos > 1) ? sName.copy( 0, nOldDelimPos ) : sName );

    if ( sName == rName ||
       ( nNewNumeric == nOldNumeric && aNewPrefix == aOldPrefix ) )
        return;
    OUString sNewGroup(rName);
    if (sNewGroup.indexOf(GLOS_DELIM)<0)
    {
        sNewGroup += OUStringLiteral1(GLOS_DELIM) + "0";
    }

    //the name must be saved, the group may be invalidated while in RenameGroupDoc()
    SwGlossaries* pTempGlossaries = pGlossaries;

    OUString sPreserveTitle( pGlossaries->GetGroupTitle( sName ) );
    if ( !pGlossaries->RenameGroupDoc( sName, sNewGroup, sPreserveTitle ) )
        throw uno::RuntimeException();
    sName = rName;
    m_sGroupName = sNewGroup;
    pGlossaries = pTempGlossaries;
}

sal_Int32 SwXAutoTextGroup::getCount()
{
    SolarMutexGuard aGuard;
    std::unique_ptr<SwTextBlocks> pGlosGroup(pGlossaries ? pGlossaries->GetGroupDoc(m_sGroupName) : nullptr);
    if (!pGlosGroup || pGlosGroup->GetError())
        throw uno::RuntimeException();
    return static_cast<sal_Int32>(pGlosGroup->GetCount());
}

uno::Any SwXAutoTextGroup::getByIndex(sal_Int32 nIndex)
{
    SolarMutexGuard aGuard;
    std::unique_ptr<SwTextBlocks> pGlosGroup(pGlossaries ? pGlossaries->GetGroupDoc(m_sGroupName) : nullptr);
    if (!pGlosGroup || pGlosGroup->GetError())
        throw uno::RuntimeException();
    const sal_uInt16 nCount = pGlosGroup->GetCount();
    if (nIndex < 0 || nIndex >= static_cast<sal_Int32>(nCount))
        throw lang::IndexOutOfBoundsException();
    return getByName(pGlosGroup->GetShortName(static_cast<sal_uInt16>(nIndex)));
}

uno::Type SwXAutoTextGroup::getElementType()
{
    return cppu::UnoType<text::XAutoTextEntry>::get();

}

sal_Bool SwXAutoTextGroup::hasElements()
{
    SolarMutexGuard aGuard;
    std::unique_ptr<SwTextBlocks> pGlosGroup(pGlossaries ? pGlossaries->GetGroupDoc(m_sGroupName) : nullptr);
    if (!pGlosGroup || pGlosGroup->GetError())
        throw uno::RuntimeException();
    return pGlosGroup->GetCount() > 0;

}

uno::Any SwXAutoTextGroup::getByName(const OUString& _rName)
{
    SolarMutexGuard aGuard;
    uno::Reference< text::XAutoTextEntry > xEntry = pGlossaries->GetAutoTextEntry( m_sGroupName, sName, _rName );
    OSL_ENSURE( xEntry.is(), "SwXAutoTextGroup::getByName: GetAutoTextEntry is fractious!" );
        // we told it to create the object, so why didn't it?
    return makeAny( xEntry );
}

uno::Sequence< OUString > SwXAutoTextGroup::getElementNames()
{
    SolarMutexGuard aGuard;
    std::unique_ptr<SwTextBlocks> pGlosGroup(pGlossaries ? pGlossaries->GetGroupDoc(m_sGroupName) : nullptr);
    if (!pGlosGroup || pGlosGroup->GetError())
        throw uno::RuntimeException();

    const sal_uInt16 nCount = pGlosGroup->GetCount();
    uno::Sequence< OUString > aEntryNames(nCount);
    OUString *pArr = aEntryNames.getArray();

    for ( sal_uInt16 i = 0; i < nCount; i++ )
        pArr[i] = pGlosGroup->GetShortName(i);
    return aEntryNames;
}

sal_Bool SwXAutoTextGroup::hasByName(const OUString& rName)
{
    SolarMutexGuard aGuard;
    bool bRet = false;
    std::unique_ptr<SwTextBlocks> pGlosGroup(pGlossaries ? pGlossaries->GetGroupDoc(m_sGroupName) : nullptr);
    if (!pGlosGroup || pGlosGroup->GetError())
        throw uno::RuntimeException();

    const sal_uInt16 nCount = pGlosGroup->GetCount();
    for( sal_uInt16 i = 0; i < nCount; ++i )
    {
        OUString sCompare(pGlosGroup->GetShortName(i));
        if(sCompare.equalsIgnoreAsciiCase(rName))
        {
            bRet = true;
            break;
        }
    }
    return bRet;
}

uno::Reference< beans::XPropertySetInfo >  SwXAutoTextGroup::getPropertySetInfo()
{
    static uno::Reference< beans::XPropertySetInfo >  xRet = pPropSet->getPropertySetInfo();
    return xRet;
}

void SwXAutoTextGroup::setPropertyValue(
    const OUString& rPropertyName, const uno::Any& aValue)
{
    SolarMutexGuard aGuard;
    const SfxItemPropertySimpleEntry*   pEntry = pPropSet->getPropertyMap().getByName( rPropertyName );

    if(!pEntry)
        throw beans::UnknownPropertyException();

    std::unique_ptr<SwTextBlocks> pGlosGroup(pGlossaries ? pGlossaries->GetGroupDoc(m_sGroupName) : nullptr);
    if(!pGlosGroup || pGlosGroup->GetError())
        throw uno::RuntimeException();
    switch(pEntry->nWID)
    {
        case  WID_GROUP_TITLE:
        {
            OUString sNewTitle;
            aValue >>= sNewTitle;
            if(sNewTitle.isEmpty())
                throw lang::IllegalArgumentException();
            bool bChanged = sNewTitle != pGlosGroup->GetName();
            pGlosGroup->SetName(sNewTitle);
            if(bChanged && HasGlossaryList())
                GetGlossaryList()->ClearGroups();
        }
        break;
    }
}

uno::Any SwXAutoTextGroup::getPropertyValue(const OUString& rPropertyName)
{
    SolarMutexGuard aGuard;
    const SfxItemPropertySimpleEntry*   pEntry = pPropSet->getPropertyMap().getByName( rPropertyName);

    if(!pEntry)
        throw beans::UnknownPropertyException();
    std::unique_ptr<SwTextBlocks> pGlosGroup(pGlossaries ? pGlossaries->GetGroupDoc(m_sGroupName) : nullptr);
    if(!pGlosGroup  || pGlosGroup->GetError())
        throw uno::RuntimeException();

    uno::Any aAny;
    switch(pEntry->nWID)
    {
        case  WID_GROUP_PATH:
            aAny <<= pGlosGroup->GetFileName();
        break;
        case  WID_GROUP_TITLE:
            aAny <<= pGlosGroup->GetName();
        break;
    }
    return aAny;
}

void SwXAutoTextGroup::addPropertyChangeListener(
    const OUString& /*PropertyName*/, const uno::Reference< beans::XPropertyChangeListener > & /*aListener*/)
{
}

void SwXAutoTextGroup::removePropertyChangeListener(
    const OUString& /*PropertyName*/, const uno::Reference< beans::XPropertyChangeListener > & /*aListener*/)
{
}

void SwXAutoTextGroup::addVetoableChangeListener(
    const OUString& /*PropertyName*/, const uno::Reference< beans::XVetoableChangeListener > & /*aListener*/)
{
}

void SwXAutoTextGroup::removeVetoableChangeListener(
    const OUString& /*PropertyName*/, const uno::Reference< beans::XVetoableChangeListener > & /*aListener*/)
{
}

void SwXAutoTextGroup::Invalidate()
{
    pGlossaries = nullptr;
    sName.clear();
    m_sGroupName.clear();
}

OUString SwXAutoTextGroup::getImplementationName()
{
    return OUString("SwXAutoTextGroup");
}

sal_Bool SwXAutoTextGroup::supportsService(const OUString& rServiceName)
{
    return cppu::supportsService(this, rServiceName);
}

uno::Sequence< OUString > SwXAutoTextGroup::getSupportedServiceNames()
{
    uno::Sequence<OUString> aRet { "com.sun.star.text.AutoTextGroup" };
    return aRet;
}

namespace
{
    class theSwXAutoTextEntryUnoTunnelId : public rtl::Static< UnoTunnelIdInit, theSwXAutoTextEntryUnoTunnelId > {};
}

const uno::Sequence< sal_Int8 > & SwXAutoTextEntry::getUnoTunnelId()
{
    return theSwXAutoTextEntryUnoTunnelId::get().getSeq();
}

sal_Int64 SAL_CALL SwXAutoTextEntry::getSomething( const uno::Sequence< sal_Int8 >& rId )
{
    if( rId.getLength() == 16
        && 0 == memcmp( getUnoTunnelId().getConstArray(),
                                        rId.getConstArray(), 16 ) )
    {
            return sal::static_int_cast< sal_Int64 >( reinterpret_cast< sal_IntPtr >( this ));
    }
    return 0;
}

SwXAutoTextEntry::SwXAutoTextEntry(SwGlossaries* pGlss, const OUString& rGroupName,
                                            const OUString& rEntryName) :
    WeakComponentImplHelper(m_aMutex),
    pGlossaries(pGlss),
    sGroupName(rGroupName),
    sEntryName(rEntryName)
{
}

SwXAutoTextEntry::~SwXAutoTextEntry()
{
    SolarMutexGuard aGuard;

    // ensure that any pending modifications are written
    implFlushDocument( true );
}

void SwXAutoTextEntry::implFlushDocument( bool _bCloseDoc )
{
    if ( xDocSh.is() )
    {
        if ( xDocSh->GetDoc()->getIDocumentState().IsModified () )
            xDocSh->Save();

        if ( _bCloseDoc )
        {
            // stop listening at the document
            EndListening( *xDocSh );

            xDocSh->DoClose();
            xDocSh.clear();
        }
    }
}

void SwXAutoTextEntry::Notify( SfxBroadcaster& _rBC, const SfxHint& _rHint )
{
    if ( &_rBC == xDocSh.get() )
    {   // it's our document
        if (const SfxEventHint* pEventHint = dynamic_cast<const SfxEventHint*>(&_rHint))
        {
            if (SfxEventHintId::PrepareCloseDoc == pEventHint->GetEventId())
            {
                implFlushDocument();
                mxBodyText.clear();
                EndListening( *xDocSh );
                xDocSh.clear();
            }
        }
        else
        {
            if ( SfxHintId::Deinitializing == _rHint.GetId() )
            {
                // our document is dying (possibly because we're shutting down, and the document was notified
                // earlier than we are?)
                // stop listening at the docu
                EndListening( *xDocSh );
                // and release our reference
                xDocSh.clear();
            }
        }
    }
}

void SwXAutoTextEntry::GetBodyText ()
{
    SolarMutexGuard aGuard;

    xDocSh = pGlossaries->EditGroupDoc ( sGroupName, sEntryName, false );
    OSL_ENSURE( xDocSh.is(), "SwXAutoTextEntry::GetBodyText: unexpected: no doc returned by EditGroupDoc!" );

    // start listening at the document
    StartListening( *xDocSh );

    mxBodyText = new SwXBodyText ( xDocSh->GetDoc() );
}

void SwXAutoTextEntry::disposing()
{
    SolarMutexGuard g;
    implFlushDocument(true);
}

uno::Reference< text::XTextCursor >  SwXAutoTextEntry::createTextCursor()
{
    SolarMutexGuard aGuard;
    EnsureBodyText();
    return mxBodyText->createTextCursor();
}

uno::Reference< text::XTextCursor >  SwXAutoTextEntry::createTextCursorByRange(
    const uno::Reference< text::XTextRange > & aTextPosition)
{
    SolarMutexGuard aGuard;
    EnsureBodyText();
    return mxBodyText->createTextCursorByRange ( aTextPosition );
}

void SwXAutoTextEntry::insertString(const uno::Reference< text::XTextRange > & xRange, const OUString& aString, sal_Bool bAbsorb)
{
    SolarMutexGuard aGuard;
    EnsureBodyText();
    mxBodyText->insertString ( xRange, aString, bAbsorb );
}

void SwXAutoTextEntry::insertControlCharacter(const uno::Reference< text::XTextRange > & xRange,
    sal_Int16 nControlCharacter, sal_Bool bAbsorb)
{
    SolarMutexGuard aGuard;
    EnsureBodyText();
    mxBodyText->insertControlCharacter ( xRange, nControlCharacter, bAbsorb );
}

void SwXAutoTextEntry::insertTextContent(
    const uno::Reference< text::XTextRange > & xRange,
    const uno::Reference< text::XTextContent > & xContent, sal_Bool bAbsorb)
{
    SolarMutexGuard aGuard;
    EnsureBodyText();
    mxBodyText->insertTextContent ( xRange, xContent, bAbsorb );
}

void SwXAutoTextEntry::removeTextContent(
    const uno::Reference< text::XTextContent > & xContent)
{
    SolarMutexGuard aGuard;
    EnsureBodyText();
    mxBodyText->removeTextContent ( xContent );
}

uno::Reference< text::XText >  SwXAutoTextEntry::getText()
{
    SolarMutexGuard aGuard;
    uno::Reference< text::XText >  xRet =  static_cast<text::XText*>(this);
    return xRet;
}

uno::Reference< text::XTextRange >  SwXAutoTextEntry::getStart()
{
    SolarMutexGuard aGuard;
    EnsureBodyText();
    return mxBodyText->getStart();
}

uno::Reference< text::XTextRange >  SwXAutoTextEntry::getEnd()
{
    SolarMutexGuard aGuard;
    EnsureBodyText();
    return mxBodyText->getEnd();
}

OUString SwXAutoTextEntry::getString()
{
    SolarMutexGuard aGuard;
    EnsureBodyText();
    return mxBodyText->getString();
}

void SwXAutoTextEntry::setString(const OUString& aString)
{
    SolarMutexGuard aGuard;
    EnsureBodyText();
    mxBodyText->setString( aString );
}

void SwXAutoTextEntry::applyTo(const uno::Reference< text::XTextRange > & xTextRange)
{
    SolarMutexGuard aGuard;

    // ensure that any pending modifications are written
    // reason is that we're holding the _copy_ of the auto text, while the real auto text
    // is stored somewhere. And below, we're not working with our copy, but only tell the target
    // TextRange to work with the stored version.
    // #96380# - 2003-03-03 - fs@openoffice.org
    implFlushDocument();
        // TODO: think about if we should pass "true" here
        // The difference would be that when the next modification is made to this instance here, then
        // we would be forced to open the document again, instead of working on our current copy.
        // This means that we would reflect any changes which were done to the AutoText by foreign instances
        // in the meantime

    uno::Reference<lang::XUnoTunnel> xTunnel( xTextRange, uno::UNO_QUERY);
    SwXTextRange* pRange = nullptr;
    OTextCursorHelper* pCursor = nullptr;
    SwXText *pText = nullptr;

    if(xTunnel.is())
    {
        pRange = reinterpret_cast < SwXTextRange* >
                ( xTunnel->getSomething( SwXTextRange::getUnoTunnelId() ) );
        pCursor = reinterpret_cast < OTextCursorHelper*>
                ( xTunnel->getSomething( OTextCursorHelper::getUnoTunnelId() ) );
        pText = reinterpret_cast < SwXText* >
                ( xTunnel->getSomething( SwXText::getUnoTunnelId() ) );
    }

    SwDoc* pDoc = nullptr;
    if (pRange)
        pDoc = &pRange->GetDoc();
    else if ( pCursor )
        pDoc = pCursor->GetDoc();
    else if ( pText && pText->GetDoc() )
    {
        xTunnel.set(pText->getStart(), uno::UNO_QUERY);
        if (xTunnel.is())
        {
            pCursor = reinterpret_cast < OTextCursorHelper* >
                ( xTunnel->getSomething( OTextCursorHelper::getUnoTunnelId() ) );
            if (pCursor)
                pDoc = pText->GetDoc();
        }
    }

    if(!pDoc)
        throw uno::RuntimeException();

    SwPaM InsertPaM(pDoc->GetNodes());
    if (pRange)
    {
        if (!pRange->GetPositions(InsertPaM))
        {
            throw uno::RuntimeException();
        }
    }
    else
    {
        InsertPaM = *pCursor->GetPaM();
    }

    std::unique_ptr<SwTextBlocks> pBlock(pGlossaries->GetGroupDoc(sGroupName));
    const bool bResult = pBlock.get() && !pBlock->GetError()
                    && pDoc->InsertGlossary( *pBlock, sEntryName, InsertPaM);

    if(!bResult)
        throw uno::RuntimeException();
}

OUString SwXAutoTextEntry::getImplementationName()
{
    return OUString("SwXAutoTextEntry");
}

sal_Bool SwXAutoTextEntry::supportsService(const OUString& rServiceName)
{
    return cppu::supportsService(this, rServiceName);
}

uno::Sequence< OUString > SwXAutoTextEntry::getSupportedServiceNames()
{
    uno::Sequence<OUString> aRet { "com.sun.star.text.AutoTextEntry" };
    return aRet;
}

uno::Reference< container::XNameReplace > SwXAutoTextEntry::getEvents()
{
    return new SwAutoTextEventDescriptor( *this );
}

const struct SvEventDescription aAutotextEvents[] =
{
    { SvMacroItemId::SwStartInsGlossary,  "OnInsertStart" },
    { SvMacroItemId::SwEndInsGlossary,    "OnInsertDone" },
    { SvMacroItemId::NONE, nullptr }
};

SwAutoTextEventDescriptor::SwAutoTextEventDescriptor(
    SwXAutoTextEntry& rAutoText ) :
        SvBaseEventDescriptor(aAutotextEvents),
        rAutoTextEntry(rAutoText)
{
}

SwAutoTextEventDescriptor::~SwAutoTextEventDescriptor()
{
}

OUString SwAutoTextEventDescriptor::getImplementationName()
{
    return OUString("SwAutoTextEventDescriptor");
}

void SwAutoTextEventDescriptor::replaceByName(
    const SvMacroItemId nEvent,
    const SvxMacro& rMacro)
{
    OSL_ENSURE( nullptr != rAutoTextEntry.GetGlossaries(),
                "Strangely enough, the AutoText vanished!" );
    OSL_ENSURE( (nEvent == SvMacroItemId::SwEndInsGlossary) ||
                (nEvent == SvMacroItemId::SwStartInsGlossary) ,
                "Unknown event ID" );

    SwGlossaries *const pGlossaries =
        const_cast<SwGlossaries*>(rAutoTextEntry.GetGlossaries());
    std::unique_ptr<SwTextBlocks> pBlocks(
        pGlossaries->GetGroupDoc( rAutoTextEntry.GetGroupName() ));
    OSL_ENSURE( pBlocks,
                "can't get autotext group; SwAutoTextEntry has illegal name?");

    if( pBlocks && !pBlocks->GetError())
    {
        sal_uInt16 nIndex = pBlocks->GetIndex( rAutoTextEntry.GetEntryName() );
        if( nIndex != USHRT_MAX )
        {
            SvxMacroTableDtor aMacroTable;
            if( pBlocks->GetMacroTable( nIndex, aMacroTable ) )
            {
                aMacroTable.Insert( nEvent, rMacro );
                pBlocks->SetMacroTable( nIndex, aMacroTable );
            }
        }
    }
    // else: ignore
}

void SwAutoTextEventDescriptor::getByName(
    SvxMacro& rMacro,
    const SvMacroItemId nEvent )
{
    OSL_ENSURE( nullptr != rAutoTextEntry.GetGlossaries(), "no AutoText" );
    OSL_ENSURE( (nEvent == SvMacroItemId::SwEndInsGlossary) ||
                (nEvent == SvMacroItemId::SwStartInsGlossary) ,
                "Unknown event ID" );

    SwGlossaries *const pGlossaries =
        const_cast<SwGlossaries*>(rAutoTextEntry.GetGlossaries());
    std::unique_ptr<SwTextBlocks> pBlocks(
        pGlossaries->GetGroupDoc( rAutoTextEntry.GetGroupName() ));
    OSL_ENSURE( pBlocks,
                "can't get autotext group; SwAutoTextEntry has illegal name?");

    // return empty macro, unless macro is found
    OUString sEmptyStr;
    SvxMacro aEmptyMacro(sEmptyStr, sEmptyStr);
    rMacro = aEmptyMacro;

    if ( pBlocks &&  !pBlocks->GetError())
    {
        sal_uInt16 nIndex = pBlocks->GetIndex( rAutoTextEntry.GetEntryName() );
        if( nIndex != USHRT_MAX )
        {
            SvxMacroTableDtor aMacroTable;
            if( pBlocks->GetMacroTable( nIndex, aMacroTable ) )
            {
                SvxMacro *pMacro = aMacroTable.Get( nEvent );
                if( pMacro )
                    rMacro = *pMacro;
            }
        }
    }
}

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface*
SwXAutoTextContainer_get_implementation(css::uno::XComponentContext*,
        css::uno::Sequence<css::uno::Any> const &)
{
    //the module may not be loaded
    SolarMutexGuard aGuard;
    SwGlobals::ensure();
    return cppu::acquire(new SwXAutoTextContainer());
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
