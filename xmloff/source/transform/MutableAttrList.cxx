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

#include <xmloff/attrlist.hxx>
#include <comphelper/sequence.hxx>
#include <comphelper/servicehelper.hxx>
#include "MutableAttrList.hxx"


using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::util;

struct XMLMutableAttributeList::MyMutableAttributeList : public ::cppu::WeakImplHelper<
        css::xml::sax::XFastAttributeList,
        css::util::XCloneable>
{
    std::vector<css::xml::FastAttribute> maAttributes;
    std::vector<css::xml::Attribute> maUnknownAttributes;

    MyMutableAttributeList(const css::uno::Reference< css::xml::sax::XFastAttributeList > & rOther)
    {
        AppendAttributeList(rOther);
    }
    MyMutableAttributeList() {}

    // css::xml::sax::XFastAttributeList
    virtual sal_Bool SAL_CALL hasAttribute( sal_Int32 Token ) override
    {
        for (auto const & i : maAttributes)
            if (i.Token == Token)
                return true;
        return false;
    }
    virtual sal_Int32 SAL_CALL getValueToken( sal_Int32 Token ) override
    {
        for (auto const & i : maAttributes)
            if (i.Token == Token)
                return i.Value.toInt32();
        throw Exception();
    }
    virtual sal_Int32 SAL_CALL getOptionalValueToken( sal_Int32 Token, sal_Int32 Default ) override
    {
        for (auto const & i : maAttributes)
            if (i.Token == Token)
                return i.Value.toInt32();
        return Default;
    }
    virtual OUString SAL_CALL getValue( sal_Int32 Token ) override
    {
        for (auto const & i : maAttributes)
            if (i.Token == Token)
                return i.Value;
        throw Exception();
    }
    virtual OUString SAL_CALL getOptionalValue( sal_Int32 Token ) override
    {
        for (auto const & i : maAttributes)
            if (i.Token == Token)
                return i.Value;
        return OUString();
    }
    virtual css::uno::Sequence< ::css::xml::Attribute > SAL_CALL getUnknownAttributes() override
    {
        return comphelper::containerToSequence(maUnknownAttributes);
    }
    virtual css::uno::Sequence< ::css::xml::FastAttribute > SAL_CALL getFastAttributes() override
    {
        return comphelper::containerToSequence(maAttributes);
    }
    virtual sal_Int16 SAL_CALL getLength() override { return maAttributes.size(); }
    virtual sal_Int32 SAL_CALL getTokenByIndex( sal_Int16 i ) override { return maAttributes[i].Token; }
    virtual OUString SAL_CALL getValueByIndex( sal_Int16 i ) override { return maAttributes[i].Value; }

    // css::util::XCloneable
    virtual css::uno::Reference< css::util::XCloneable > SAL_CALL createClone() override
    {
        css::uno::Reference< css::util::XCloneable >  r = new MyMutableAttributeList( *this );
        return r;
    }

    void SetValueByToken( sal_Int32 nAttributeToken, const OUString& rValue )
    {
        for (auto & i : maAttributes)
            if (i.Token == nAttributeToken)
            {
                i.Value = rValue;
                return;
            }
        throw Exception();
    }
    void SetValueByIndex( sal_Int16 i, const OUString& rValue ) { maAttributes[i].Value = rValue; }
    void AddAttribute( sal_Int32 nAttributeToken , const OUString &sValue )
    {
        maAttributes.push_back(css::xml::FastAttribute(nAttributeToken, sValue));
    }
    void RemoveAttributeByIndex( sal_Int16 i ) { maAttributes.erase(maAttributes.begin() + i); }
    void RenameAttributeByIndex( sal_Int16 i, sal_Int32 nNewAttributeToken )
    {
        maAttributes[i].Token = nNewAttributeToken;
    }
    void AppendAttributeList( const css::uno::Reference< css::xml::sax::XFastAttributeList > & rList)
    {
        const auto attrs = rList->getFastAttributes();
        for (const auto & rAttr : attrs)
            maAttributes.push_back(rAttr);
        const auto unknownAttrs = rList->getUnknownAttributes();
        for (const auto & rAttr : unknownAttrs)
            maUnknownAttributes.push_back(rAttr);
    }
    sal_Int16 GetIndexByToken( sal_Int32 nAttributeToken ) const
    {
        sal_Int16 idx = 0;
        for (auto & i : maAttributes)
        {
            if (i.Token == nAttributeToken)
                return idx;
            ++idx;
        }
        throw Exception();
    }
};

XMLMutableAttributeList::MyMutableAttributeList *XMLMutableAttributeList::GetMutableAttrList()
{
    if( !m_pMutableAttrList )
    {
        m_pMutableAttrList = new MyMutableAttributeList( m_xAttrList );
        m_xAttrList = m_pMutableAttrList;
    }

    return m_pMutableAttrList;
}

XMLMutableAttributeList::XMLMutableAttributeList() :
    m_pMutableAttrList( new MyMutableAttributeList )
{
    m_xAttrList = m_pMutableAttrList;
}

XMLMutableAttributeList::XMLMutableAttributeList( const Reference<
        XFastAttributeList> & rAttrList, bool bClone ) :
    m_xAttrList( rAttrList.is() ? rAttrList : new MyMutableAttributeList ),
    m_pMutableAttrList( nullptr )
{
    if( bClone )
        GetMutableAttrList();
}


XMLMutableAttributeList::~XMLMutableAttributeList()
{
    m_xAttrList = nullptr;
}

namespace
{
    class theXMLMutableAttributeListUnoTunnelId : public rtl::Static< UnoTunnelIdInit, theXMLMutableAttributeListUnoTunnelId> {};
}

const css::uno::Sequence<sal_Int8>& XMLMutableAttributeList::getUnoTunnelId() throw()
{
    return theXMLMutableAttributeListUnoTunnelId::get().getSeq();
}

// XUnoTunnel
sal_Int64 SAL_CALL XMLMutableAttributeList::getSomething(
        const Sequence< sal_Int8 >& rId )
{
    if( isUnoTunnelId<XMLMutableAttributeList>(rId) )
    {
        return sal::static_int_cast<sal_Int64>(reinterpret_cast<sal_uIntPtr>(this));
    }
    return 0;
}

sal_Bool SAL_CALL XMLMutableAttributeList::hasAttribute(sal_Int32 Token)
{
    return m_xAttrList->hasAttribute(Token);
}

sal_Int32 SAL_CALL XMLMutableAttributeList::getValueToken(sal_Int32 Token)
{
    return m_xAttrList->getValueToken(Token);
}

sal_Int32 SAL_CALL XMLMutableAttributeList::getOptionalValueToken(sal_Int32 Token, sal_Int32 Default)
{
    return m_xAttrList->getOptionalValueToken(Token, Default);
}

OUString SAL_CALL XMLMutableAttributeList::getValue(sal_Int32 Token)
{
    return m_xAttrList->getValue(Token);
}

OUString SAL_CALL XMLMutableAttributeList::getOptionalValue(sal_Int32 Token)
{
    return m_xAttrList->getOptionalValue(Token);
}

css::uno::Sequence< ::css::xml::Attribute > SAL_CALL XMLMutableAttributeList::getUnknownAttributes()
{
    return m_xAttrList->getUnknownAttributes();
}

css::uno::Sequence< ::css::xml::FastAttribute > SAL_CALL XMLMutableAttributeList::getFastAttributes()
{
    return m_xAttrList->getFastAttributes();
}

sal_Int16 SAL_CALL XMLMutableAttributeList::getLength()
{
    return m_xAttrList->getLength();
}


sal_Int32 SAL_CALL XMLMutableAttributeList::getTokenByIndex(sal_Int16 i)
{
    return m_xAttrList->getTokenByIndex( i );
}


OUString SAL_CALL  XMLMutableAttributeList::getValueByIndex(sal_Int16 i)
{
    return m_xAttrList->getValueByIndex( i );
}

Reference< XCloneable > XMLMutableAttributeList::createClone()
{
    // A cloned list will be a read only list!
    Reference< XCloneable >  r = new MyMutableAttributeList( m_xAttrList );
    return r;
}

void XMLMutableAttributeList::SetValueByToken( sal_Int32 nToken,
                                               const OUString& rValue )
{
    GetMutableAttrList()->SetValueByToken( nToken, rValue );
}

void XMLMutableAttributeList::SetValueByIndex( sal_Int16 i,
                                               const OUString& rValue )
{
    GetMutableAttrList()->SetValueByIndex( i, rValue );
}

void XMLMutableAttributeList::AddAttribute( sal_Int32 rName ,
                                            const OUString &rValue )
{
    GetMutableAttrList()->AddAttribute( rName, rValue );
}

void XMLMutableAttributeList::RemoveAttributeByIndex( sal_Int16 i )
{
    GetMutableAttrList()->RemoveAttributeByIndex( i );
}

void XMLMutableAttributeList::RenameAttributeByIndex( sal_Int16 i,
                                                      sal_Int32 rNewName )
{
    GetMutableAttrList()->RenameAttributeByIndex( i, rNewName );
}

void XMLMutableAttributeList::AppendAttributeList(
        const Reference< css::xml::sax::XFastAttributeList >& r )
{
    GetMutableAttrList()->AppendAttributeList( r );
}

sal_Int16 XMLMutableAttributeList::GetIndexByToken( sal_Int32 nAttributeToken ) const
{
    sal_Int16 nIndex = -1;
    if( m_pMutableAttrList )
    {
        nIndex = m_pMutableAttrList->GetIndexByToken( nAttributeToken );
    }
    else
    {
        sal_Int16 nCount = m_xAttrList->getLength();
        for( sal_Int16 i=0; nIndex==-1 && i<nCount ; ++i )
        {
            if( m_xAttrList->getTokenByIndex(i) == nAttributeToken )
                nIndex = i;
        }
    }
    return nIndex;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
