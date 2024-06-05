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
#include <com/sun/star/container/ElementExistException.hpp>
#include <o3tl/safeint.hxx>
#include <osl/diagnose.h>
#include <rtl/character.hxx>
#include <utility>
#include <vcl/svapp.hxx>
#include <svtools/unoevent.hxx>
#include <sfx2/event.hxx>
#include <glosdoc.hxx>
#include <shellio.hxx>
#include <initui.hxx>
#include <gloslst.hxx>
#include <unoatxt.hxx>
#include <unomap.hxx>
#include <unotextbodyhf.hxx>
#include <unotextrange.hxx>
#include <TextCursorHelper.hxx>
#include <doc.hxx>
#include <IDocumentContentOperations.hxx>
#include <IDocumentRedlineAccess.hxx>
#include <IDocumentFieldsAccess.hxx>
#include <IDocumentState.hxx>
#include <docsh.hxx>
#include <swdll.hxx>
#include <svl/hint.hxx>
#include <tools/urlobj.hxx>
#include <svl/macitem.hxx>
#include <editeng/acorrcfg.hxx>
#include <comphelper/servicehelper.hxx>
#include <cppuhelper/exc_hlp.hxx>
#include <cppuhelper/supportsservice.hxx>

#include <memory>

using namespace ::com::sun::star;

SwXAutoTextContainer::SwXAutoTextContainer()
{
    m_pGlossaries = ::GetGlossaries();

}

SwXAutoTextContainer::~SwXAutoTextContainer()
{

}

sal_Int32 SwXAutoTextContainer::getCount()
{
    OSL_ENSURE(m_pGlossaries->GetGroupCnt() < o3tl::make_unsigned(SAL_MAX_INT32),
               "SwXAutoTextContainer::getCount: too many items");
    return static_cast<sal_Int32>(m_pGlossaries->GetGroupCnt());
}

uno::Any SwXAutoTextContainer::getByIndex(sal_Int32 nIndex)
{
    SolarMutexGuard aGuard;
    const size_t nCount = m_pGlossaries->GetGroupCnt();
    if ( nIndex < 0 || o3tl::make_unsigned(nIndex) >= nCount )
        throw lang::IndexOutOfBoundsException();
    return getByName(m_pGlossaries->GetGroupName( static_cast<size_t>(nIndex) ));
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
    if ( m_pGlossaries && hasByName( GroupName ) )    // group name already known?
        // true = create group if not already available
        xGroup = m_pGlossaries->GetAutoTextGroup( GroupName );

    if ( !xGroup.is() )
        throw container::NoSuchElementException();

    return css::uno::Any( xGroup );
}

uno::Sequence< OUString > SwXAutoTextContainer::getElementNames()
{
    SolarMutexGuard aGuard;
    const size_t nCount = m_pGlossaries->GetGroupCnt();
    OSL_ENSURE(nCount < o3tl::make_unsigned(SAL_MAX_INT32),
               "SwXAutoTextContainer::getElementNames: too many groups");

    uno::Sequence< OUString > aGroupNames(static_cast<sal_Int32>(nCount));
    OUString *pArr = aGroupNames.getArray();

    for ( size_t i = 0; i < nCount; ++i )
    {
        // The names will be passed without a path extension.
        pArr[i] = m_pGlossaries->GetGroupName(i).getToken(0, GLOS_DELIM);
    }
    return aGroupNames;
}
// Finds group names with or without path index.
sal_Bool SwXAutoTextContainer::hasByName(const OUString& Name)
{
    SolarMutexGuard aGuard;
    OUString sGroupName( m_pGlossaries->GetCompleteGroupName( Name ) );
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
        throw lang::IllegalArgumentException(u"group name must not be empty"_ustr, nullptr, 0);
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
        throw lang::IllegalArgumentException(u"group name must contain a-z, A-z, '_', ' ' only"_ustr, nullptr, 0);
    }
    OUString sGroup(aGroupName);
    if (sGroup.indexOf(GLOS_DELIM)<0)
    {
        sGroup += OUStringChar(GLOS_DELIM) + "0";
    }
    m_pGlossaries->NewGroupDoc(sGroup, sGroup.getToken(0, GLOS_DELIM));

    uno::Reference< text::XAutoTextGroup > xGroup = m_pGlossaries->GetAutoTextGroup( sGroup );
    OSL_ENSURE( xGroup.is(), "SwXAutoTextContainer::insertNewByName: no UNO object created? How this?" );
        // We just inserted the group into the glossaries, so why doesn't it exist?

    return xGroup;
}

void SwXAutoTextContainer::removeByName(const OUString& aGroupName)
{
    SolarMutexGuard aGuard;
    // At first find the name with path extension
    OUString sGroupName = m_pGlossaries->GetCompleteGroupName( aGroupName );
    if(sGroupName.isEmpty())
        throw container::NoSuchElementException();
    m_pGlossaries->DelGroupDoc(sGroupName);
}

OUString SwXAutoTextContainer::getImplementationName()
{
    return u"SwXAutoTextContainer"_ustr;
}

sal_Bool SwXAutoTextContainer::supportsService(const OUString& rServiceName)
{
    return cppu::supportsService(this, rServiceName);
}

uno::Sequence< OUString > SwXAutoTextContainer::getSupportedServiceNames()
{
    return { u"com.sun.star.text.AutoTextContainer"_ustr };
}

SwXAutoTextGroup::SwXAutoTextGroup(const OUString& rName,
            SwGlossaries*   pGlos) :
    m_pPropSet(aSwMapProvider.GetPropertySet(PROPERTY_MAP_AUTO_TEXT_GROUP)),
    m_pGlossaries(pGlos),
    m_sName(rName),
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
    std::unique_ptr<SwTextBlocks> pGlosGroup(m_pGlossaries ? m_pGlossaries->GetGroupDoc(m_sGroupName) : nullptr);
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
    std::unique_ptr<SwTextBlocks> pGlosGroup(m_pGlossaries ? m_pGlossaries->GetGroupDoc(m_sGroupName) : nullptr);
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

static bool lcl_CopySelToDoc(SwDoc& rInsDoc, OTextCursorHelper* pxCursor, SwXTextRange* pxRange)
{
    SwNodes& rNds = rInsDoc.GetNodes();

    SwNodeIndex aIdx( rNds.GetEndOfContent(), -1 );
    SwContentNode * pNd = aIdx.GetNode().GetContentNode();
    SwPosition aPos(aIdx, pNd, pNd ? pNd->Len() : 0);

    bool bRet = false;
    rInsDoc.getIDocumentFieldsAccess().LockExpFields();
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
        bRet = pDoc->getIDocumentContentOperations().CopyRange(*pPam, aPos, SwCopyFlags::CheckPosInFly)
            || bRet;
    }

    rInsDoc.getIDocumentFieldsAccess().UnlockExpFields();
    if( !rInsDoc.getIDocumentFieldsAccess().IsExpFieldsLocked() )
        rInsDoc.getIDocumentFieldsAccess().UpdateExpFields(nullptr, true);

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
    if (m_pGlossaries)
        pGlosGroup = m_pGlossaries->GetGroupDoc(m_sGroupName);
    const OUString& sShortName(aName);
    const OUString& sLongName(aTitle);
    if (pGlosGroup && !pGlosGroup->GetError())
    {
        SwXTextRange* pxRange = dynamic_cast<SwXTextRange*>(xTextRange.get());
        OTextCursorHelper* pxCursor = dynamic_cast<OTextCursorHelper*>(xTextRange.get());

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
                lcl_CopySelToDoc(*pGDoc, pxCursor, pxRange);
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
        xEntry = m_pGlossaries ?
            m_pGlossaries->GetAutoTextEntry( m_sGroupName, m_sName, sShortName ) :
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
               u"Error Getting AutoText!"_ustr,
               getXWeak(),
               anyEx );
    }

    return xEntry;
}

void SwXAutoTextGroup::removeByName(const OUString& aEntryName)
{
    SolarMutexGuard aGuard;
    std::unique_ptr<SwTextBlocks> pGlosGroup(m_pGlossaries ? m_pGlossaries->GetGroupDoc(m_sGroupName) : nullptr);
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
    return m_sName;
}

void SwXAutoTextGroup::setName(const OUString& rName)
{
    SolarMutexGuard aGuard;
    if( !m_pGlossaries )
        throw uno::RuntimeException();

    sal_Int32 nNewDelimPos = rName.lastIndexOf( GLOS_DELIM );
    sal_Int32 nOldDelimPos = m_sName.lastIndexOf( GLOS_DELIM );

    OUString aNewSuffix;
    if (nNewDelimPos > -1)
        aNewSuffix = rName.copy( nNewDelimPos + 1 );
    OUString aOldSuffix;
    if (nOldDelimPos > -1)
        aOldSuffix = m_sName.copy( nOldDelimPos + 1 );

    sal_Int32 nNewNumeric = aNewSuffix.toInt32();
    sal_Int32 nOldNumeric = aOldSuffix.toInt32();

    OUString aNewPrefix( (nNewDelimPos > 1) ? rName.copy( 0, nNewDelimPos ) : rName );
    OUString aOldPrefix( (nOldDelimPos > 1) ? m_sName.copy( 0, nOldDelimPos ) : m_sName );

    if ( m_sName == rName ||
       ( nNewNumeric == nOldNumeric && aNewPrefix == aOldPrefix ) )
        return;
    OUString sNewGroup(rName);
    if (sNewGroup.indexOf(GLOS_DELIM)<0)
    {
        sNewGroup += OUStringChar(GLOS_DELIM) + "0";
    }

    //the name must be saved, the group may be invalidated while in RenameGroupDoc()
    SwGlossaries* pTempGlossaries = m_pGlossaries;

    OUString sPreserveTitle( m_pGlossaries->GetGroupTitle( m_sName ) );
    if ( !m_pGlossaries->RenameGroupDoc( m_sName, sNewGroup, sPreserveTitle ) )
        throw uno::RuntimeException();
    m_sName = rName;
    m_sGroupName = sNewGroup;
    m_pGlossaries = pTempGlossaries;
}

sal_Int32 SwXAutoTextGroup::getCount()
{
    SolarMutexGuard aGuard;
    std::unique_ptr<SwTextBlocks> pGlosGroup(m_pGlossaries ? m_pGlossaries->GetGroupDoc(m_sGroupName) : nullptr);
    if (!pGlosGroup || pGlosGroup->GetError())
        throw uno::RuntimeException();
    return static_cast<sal_Int32>(pGlosGroup->GetCount());
}

uno::Any SwXAutoTextGroup::getByIndex(sal_Int32 nIndex)
{
    SolarMutexGuard aGuard;
    if (!m_pGlossaries)
        throw uno::RuntimeException();
    std::unique_ptr<SwTextBlocks> pGlosGroup(m_pGlossaries->GetGroupDoc(m_sGroupName));
    if (!pGlosGroup || pGlosGroup->GetError())
        throw uno::RuntimeException();
    const sal_uInt16 nCount = pGlosGroup->GetCount();
    if (nIndex < 0 || o3tl::make_unsigned(nIndex) >= nCount)
        throw lang::IndexOutOfBoundsException();
    return getByName(pGlosGroup->GetShortName(o3tl::narrowing<sal_uInt16>(nIndex)));
}

uno::Type SwXAutoTextGroup::getElementType()
{
    return cppu::UnoType<text::XAutoTextEntry>::get();

}

sal_Bool SwXAutoTextGroup::hasElements()
{
    SolarMutexGuard aGuard;
    std::unique_ptr<SwTextBlocks> pGlosGroup(m_pGlossaries ? m_pGlossaries->GetGroupDoc(m_sGroupName) : nullptr);
    if (!pGlosGroup || pGlosGroup->GetError())
        throw uno::RuntimeException();
    return pGlosGroup->GetCount() > 0;

}

uno::Any SwXAutoTextGroup::getByName(const OUString& _rName)
{
    SolarMutexGuard aGuard;
    uno::Reference< text::XAutoTextEntry > xEntry = m_pGlossaries->GetAutoTextEntry( m_sGroupName, m_sName, _rName );
    OSL_ENSURE( xEntry.is(), "SwXAutoTextGroup::getByName: GetAutoTextEntry is fractious!" );
        // we told it to create the object, so why didn't it?
    return css::uno::Any( xEntry );
}

uno::Sequence< OUString > SwXAutoTextGroup::getElementNames()
{
    SolarMutexGuard aGuard;
    std::unique_ptr<SwTextBlocks> pGlosGroup(m_pGlossaries ? m_pGlossaries->GetGroupDoc(m_sGroupName) : nullptr);
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
    std::unique_ptr<SwTextBlocks> pGlosGroup(m_pGlossaries ? m_pGlossaries->GetGroupDoc(m_sGroupName) : nullptr);
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
    static uno::Reference< beans::XPropertySetInfo >  xRet = m_pPropSet->getPropertySetInfo();
    return xRet;
}

void SwXAutoTextGroup::setPropertyValue(
    const OUString& rPropertyName, const uno::Any& aValue)
{
    SolarMutexGuard aGuard;
    const SfxItemPropertyMapEntry*   pEntry = m_pPropSet->getPropertyMap().getByName( rPropertyName );

    if(!pEntry)
        throw beans::UnknownPropertyException(rPropertyName);

    std::unique_ptr<SwTextBlocks> pGlosGroup(m_pGlossaries ? m_pGlossaries->GetGroupDoc(m_sGroupName) : nullptr);
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
    const SfxItemPropertyMapEntry*   pEntry = m_pPropSet->getPropertyMap().getByName( rPropertyName);

    if(!pEntry)
        throw beans::UnknownPropertyException(rPropertyName);
    std::unique_ptr<SwTextBlocks> pGlosGroup(m_pGlossaries ? m_pGlossaries->GetGroupDoc(m_sGroupName) : nullptr);
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
    m_pGlossaries = nullptr;
    m_sName.clear();
    m_sGroupName.clear();
}

OUString SwXAutoTextGroup::getImplementationName()
{
    return u"SwXAutoTextGroup"_ustr;
}

sal_Bool SwXAutoTextGroup::supportsService(const OUString& rServiceName)
{
    return cppu::supportsService(this, rServiceName);
}

uno::Sequence< OUString > SwXAutoTextGroup::getSupportedServiceNames()
{
    uno::Sequence<OUString> aRet { u"com.sun.star.text.AutoTextGroup"_ustr };
    return aRet;
}

SwXAutoTextEntry::SwXAutoTextEntry(SwGlossaries* pGlss, OUString aGroupName,
                                            OUString aEntryName) :
    m_pGlossaries(pGlss),
    m_sGroupName(std::move(aGroupName)),
    m_sEntryName(std::move(aEntryName))
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
    if ( !m_xDocSh.is() )
        return;

    if ( m_xDocSh->GetDoc()->getIDocumentState().IsModified () )
        m_xDocSh->Save();

    if ( _bCloseDoc )
    {
        // stop listening at the document
        EndListening( *m_xDocSh );

        m_xDocSh->DoClose();
        m_xDocSh.clear();
    }
}

void SwXAutoTextEntry::Notify( SfxBroadcaster& _rBC, const SfxHint& _rHint )
{
    if ( &_rBC != m_xDocSh.get() )
        return;

// it's our document
    if (_rHint.GetId() == SfxHintId::ThisIsAnSfxEventHint)
    {
        const SfxEventHint& rEventHint = static_cast<const SfxEventHint&>(_rHint);
        if (SfxEventHintId::PrepareCloseDoc == rEventHint.GetEventId())
        {
            implFlushDocument();
            mxBodyText.clear();
            EndListening( *m_xDocSh );
            m_xDocSh.clear();
        }
    }
    else
    {
        if ( SfxHintId::Deinitializing == _rHint.GetId() )
        {
            // our document is dying (possibly because we're shutting down, and the document was notified
            // earlier than we are?)
            // stop listening at the docu
            EndListening( *m_xDocSh );
            // and release our reference
            m_xDocSh.clear();
        }
    }
}

void SwXAutoTextEntry::GetBodyText ()
{
    SolarMutexGuard aGuard;

    m_xDocSh = m_pGlossaries->EditGroupDoc ( m_sGroupName, m_sEntryName, false );
    OSL_ENSURE( m_xDocSh.is(), "SwXAutoTextEntry::GetBodyText: unexpected: no doc returned by EditGroupDoc!" );

    // start listening at the document
    StartListening( *m_xDocSh );

    mxBodyText = new SwXBodyText ( m_xDocSh->GetDoc() );
}

void SwXAutoTextEntry::disposing(std::unique_lock<std::mutex>&)
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

    // The reference to xKeepAlive is needed during the whole call, likely because it could be a
    // different object, not xTextRange itself, and the reference guards it from preliminary death
    auto xKeepAlive( xTextRange );
    SwXTextRange* pRange = dynamic_cast<SwXTextRange*>(xTextRange.get());
    OTextCursorHelper* pCursor = dynamic_cast<OTextCursorHelper*>(xTextRange.get());
    SwXText *pText = dynamic_cast<SwXText*>(xTextRange.get());

    SwDoc* pDoc = nullptr;
    if (pRange)
        pDoc = &pRange->GetDoc();
    else if ( pCursor )
        pDoc = pCursor->GetDoc();
    else if ( pText && pText->GetDoc() )
    {
        xKeepAlive = pText->getStart();
        pCursor = dynamic_cast<OTextCursorHelper*>(xKeepAlive.get());
        if (pCursor)
            pDoc = pText->GetDoc();
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

    std::unique_ptr<SwTextBlocks> pBlock(m_pGlossaries->GetGroupDoc(m_sGroupName));
    const bool bResult = pBlock && !pBlock->GetError()
                    && pDoc->InsertGlossary( *pBlock, m_sEntryName, InsertPaM);

    if(!bResult)
        throw uno::RuntimeException();
}

OUString SwXAutoTextEntry::getImplementationName()
{
    return u"SwXAutoTextEntry"_ustr;
}

sal_Bool SwXAutoTextEntry::supportsService(const OUString& rServiceName)
{
    return cppu::supportsService(this, rServiceName);
}

uno::Sequence< OUString > SwXAutoTextEntry::getSupportedServiceNames()
{
    uno::Sequence<OUString> aRet { u"com.sun.star.text.AutoTextEntry"_ustr };
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
        m_rAutoTextEntry(rAutoText)
{
}

SwAutoTextEventDescriptor::~SwAutoTextEventDescriptor()
{
}

OUString SwAutoTextEventDescriptor::getImplementationName()
{
    return u"SwAutoTextEventDescriptor"_ustr;
}

void SwAutoTextEventDescriptor::replaceByName(
    const SvMacroItemId nEvent,
    const SvxMacro& rMacro)
{
    OSL_ENSURE( nullptr != m_rAutoTextEntry.GetGlossaries(),
                "Strangely enough, the AutoText vanished!" );
    OSL_ENSURE( (nEvent == SvMacroItemId::SwEndInsGlossary) ||
                (nEvent == SvMacroItemId::SwStartInsGlossary) ,
                "Unknown event ID" );

    SwGlossaries *const pGlossaries =
        const_cast<SwGlossaries*>(m_rAutoTextEntry.GetGlossaries());
    std::unique_ptr<SwTextBlocks> pBlocks(
        pGlossaries->GetGroupDoc( m_rAutoTextEntry.GetGroupName() ));
    OSL_ENSURE( pBlocks,
                "can't get autotext group; SwAutoTextEntry has illegal name?");

    if( !pBlocks || pBlocks->GetError())
        return;

    sal_uInt16 nIndex = pBlocks->GetIndex( m_rAutoTextEntry.GetEntryName() );
    if( nIndex != USHRT_MAX )
    {
        SvxMacroTableDtor aMacroTable;
        if( pBlocks->GetMacroTable( nIndex, aMacroTable ) )
        {
            aMacroTable.Insert( nEvent, rMacro );
            pBlocks->SetMacroTable( nIndex, aMacroTable );
        }
    }
    // else: ignore
}

void SwAutoTextEventDescriptor::getByName(
    SvxMacro& rMacro,
    const SvMacroItemId nEvent )
{
    OSL_ENSURE( nullptr != m_rAutoTextEntry.GetGlossaries(), "no AutoText" );
    OSL_ENSURE( (nEvent == SvMacroItemId::SwEndInsGlossary) ||
                (nEvent == SvMacroItemId::SwStartInsGlossary) ,
                "Unknown event ID" );

    SwGlossaries *const pGlossaries =
        const_cast<SwGlossaries*>(m_rAutoTextEntry.GetGlossaries());
    std::unique_ptr<SwTextBlocks> pBlocks(
        pGlossaries->GetGroupDoc( m_rAutoTextEntry.GetGroupName() ));
    OSL_ENSURE( pBlocks,
                "can't get autotext group; SwAutoTextEntry has illegal name?");

    // return empty macro, unless macro is found
    OUString sEmptyStr;
    rMacro = SvxMacro(sEmptyStr, sEmptyStr);

    if ( !pBlocks || pBlocks->GetError())
        return;

    sal_uInt16 nIndex = pBlocks->GetIndex( m_rAutoTextEntry.GetEntryName() );
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
