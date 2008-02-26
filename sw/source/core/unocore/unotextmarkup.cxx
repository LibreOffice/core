/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: unotextmarkup.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: obo $ $Date: 2008-02-26 09:48:36 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sw.hxx"

#ifndef _UNOTEXTMARKUP_HXX
#include <unotextmarkup.hxx>
#endif

#include <vos/mutex.hxx>
#include <vcl/svapp.hxx>
#include <SwSmartTagMgr.hxx>
#include <com/sun/star/text/TextMarkupType.hpp>
#include <com/sun/star/container/XStringKeyMap.hpp>
#include <ndtxt.hxx>
#include <wrong.hxx>

using namespace ::com::sun::star;

/*
 * SwXTextMarkup
 */
SwXTextMarkup::SwXTextMarkup( SwTxtNode& rTxtNode, const ModelToViewHelper::ConversionMap* pMap )
    : mpTxtNode( &rTxtNode ), mpConversionMap( pMap )
{
     mpTxtNode->Add(this);
}

SwXTextMarkup::~SwXTextMarkup()
{
     delete mpConversionMap;
}

uno::Reference< container::XStringKeyMap > SAL_CALL SwXTextMarkup::getMarkupInfoContainer() throw (uno::RuntimeException)
{
    vos::OGuard aGuard(Application::GetSolarMutex());

    uno::Reference< container::XStringKeyMap > xProp = new SwXStringKeyMap;
    return xProp;
}

void SAL_CALL SwXTextMarkup::commitTextMarkup(
    ::sal_Int32 nType,
    const ::rtl::OUString & rIdentifier,
    ::sal_Int32 nStart,
    ::sal_Int32 nLength,
    const uno::Reference< container::XStringKeyMap > & xMarkupInfoContainer)
    throw (uno::RuntimeException)
{
    vos::OGuard aGuard(Application::GetSolarMutex());

    // paragraph already dead or modified?
    if ( !mpTxtNode )
        return;

    if ( nType == text::TextMarkupType::SMARTTAG &&
        !SwSmartTagMgr::Get().IsSmartTagTypeEnabled( rIdentifier ) )
        return;

    // get appropriate list to use...
    SwWrongList* pWList = 0;
    if ( nType == text::TextMarkupType::SPELLCHECK && nLength > 0 )
    {
        pWList = mpTxtNode->GetWrong();
        if ( !pWList )
        {
            pWList = new SwWrongList;
            mpTxtNode->SetWrong( pWList );
        }
    }
    else if ( nType == text::TextMarkupType::GRAMMAR && nLength > 0 )
    {
        pWList = mpTxtNode->GetGrammarCheck();
        if ( !pWList )
        {
            pWList = new SwWrongList;
            mpTxtNode->SetGrammarCheck( pWList );
        }
    }
    else if ( nType == text::TextMarkupType::SMARTTAG && nLength > 0 )
    {
        pWList = mpTxtNode->GetSmartTags();
        if ( !pWList )
        {
            pWList = new SwWrongList;
            mpTxtNode->SetSmartTags( pWList );
        }
    }


    const ModelToViewHelper::ModelPosition aStartPos =
            ModelToViewHelper::ConvertToModelPosition( mpConversionMap, nStart );
    const ModelToViewHelper::ModelPosition aEndPos   =
            ModelToViewHelper::ConvertToModelPosition( mpConversionMap, nStart + nLength - 1);

    const bool bStartInField = aStartPos.mbIsField;
    const bool bEndInField   = aEndPos.mbIsField;
    bool bCommit = false;

    if ( bStartInField && bEndInField && aStartPos.mnPos == aEndPos.mnPos )
    {
        const xub_StrLen nFieldPosModel = static_cast< xub_StrLen >(aStartPos.mnPos);
        const USHORT nInsertPos = pWList->GetWrongPos( nFieldPosModel );

        SwWrongList* pSubList = pWList->SubList( nInsertPos );
        if ( !pSubList )
        {
            pSubList = new SwWrongList;
            pWList->InsertSubList( nFieldPosModel, 1, nInsertPos, pSubList );
        }

        pWList = pSubList;
        nStart = aStartPos.mnSubPos;
        bCommit = true;
    }
    else if ( !bStartInField && !bEndInField )
    {
        nStart = aStartPos.mnPos;
        bCommit = true;
    }

    if ( bCommit )
    {
        pWList->Insert( rIdentifier, xMarkupInfoContainer,
                static_cast< xub_StrLen >(nStart), static_cast< xub_StrLen >(nLength) );
    }
}

void SwXTextMarkup::Modify( SfxPoolItem* /*pOld*/, SfxPoolItem* /*pNew*/ )
{
    // FME 2007-07-16 #i79641# In my opinion this is perfectly legal,
    // therefore I remove the assertion in SwModify::_Remove()
    if ( pRegisteredIn )
        pRegisteredIn->Remove( this );
    // <--

    mpTxtNode = 0;
}

/*
 * SwXStringKeyMap
 */
SwXStringKeyMap::SwXStringKeyMap()
{
}

uno::Any SAL_CALL SwXStringKeyMap::getValue(const ::rtl::OUString & aKey) throw (uno::RuntimeException, container::NoSuchElementException)
{
    std::map< rtl::OUString, uno::Any >::const_iterator aIter = maMap.find( aKey );
    if ( aIter == maMap.end() )
        throw container::NoSuchElementException();

    return (*aIter).second;
}

::sal_Bool SAL_CALL SwXStringKeyMap::hasValue(const ::rtl::OUString & aKey) throw (uno::RuntimeException)
{
    return maMap.find( aKey ) != maMap.end();
}

void SAL_CALL SwXStringKeyMap::insertValue(const ::rtl::OUString & aKey, const uno::Any & aValue) throw (uno::RuntimeException, lang::IllegalArgumentException, container::ElementExistException)
{
    std::map< rtl::OUString, uno::Any >::const_iterator aIter = maMap.find( aKey );
    if ( aIter != maMap.end() )
        throw container::ElementExistException();

    maMap[ aKey ] = aValue;
}

::sal_Int32 SAL_CALL SwXStringKeyMap::getCount() throw (uno::RuntimeException)
{
    return maMap.size();
}

::rtl::OUString SAL_CALL SwXStringKeyMap::getKeyByIndex(::sal_Int32 nIndex) throw (uno::RuntimeException, lang::IndexOutOfBoundsException)
{
    if ( (sal_uInt32)nIndex >= maMap.size() )
        throw lang::IndexOutOfBoundsException();

    return ::rtl::OUString();
}

uno::Any SAL_CALL SwXStringKeyMap::getValueByIndex(::sal_Int32 nIndex) throw (uno::RuntimeException, lang::IndexOutOfBoundsException)
{
    if ( (sal_uInt32)nIndex >= maMap.size() )
        throw lang::IndexOutOfBoundsException();

    return uno::Any();
}

