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

#ifndef INCLUDED_XMLOFF_SOURCE_TRANSFORM_MUTABLEATTRLIST_HXX
#define INCLUDED_XMLOFF_SOURCE_TRANSFORM_MUTABLEATTRLIST_HXX

#include <com/sun/star/util/XCloneable.hpp>
#include <com/sun/star/xml/sax/XFastAttributeList.hpp>
#include <com/sun/star/lang/XUnoTunnel.hpp>

#include <cppuhelper/implbase.hxx>

class XMLMutableAttributeList : public ::cppu::WeakImplHelper<
        css::xml::sax::XFastAttributeList,
        css::util::XCloneable,
        css::lang::XUnoTunnel>
{
    struct MyMutableAttributeList;

    css::uno::Reference< css::xml::sax::XFastAttributeList> m_xAttrList;

    MyMutableAttributeList *m_pMutableAttrList;

    MyMutableAttributeList *GetMutableAttrList();

public:
    XMLMutableAttributeList();
    XMLMutableAttributeList( const css::uno::Reference<
        css::xml::sax::XFastAttributeList> & rAttrList,
           bool bClone=false );
    virtual ~XMLMutableAttributeList() override;

    // XUnoTunnel
    static const css::uno::Sequence<sal_Int8>& getUnoTunnelId() throw();
    virtual sal_Int64 SAL_CALL getSomething( const css::uno::Sequence< sal_Int8 >& aIdentifier ) override;

    // css::xml::sax::XFastAttributeList
    virtual sal_Bool SAL_CALL hasAttribute( sal_Int32 Token ) override;
    virtual sal_Int32 SAL_CALL getValueToken( sal_Int32 Token ) override;
    virtual sal_Int32 SAL_CALL getOptionalValueToken( sal_Int32 Token, sal_Int32 Default ) override;
    virtual OUString SAL_CALL getValue( sal_Int32 Token ) override;
    virtual OUString SAL_CALL getOptionalValue( sal_Int32 Token ) override;
    virtual css::uno::Sequence< ::css::xml::Attribute > SAL_CALL getUnknownAttributes() override;
    virtual css::uno::Sequence< ::css::xml::FastAttribute > SAL_CALL getFastAttributes() override;
    virtual sal_Int16 SAL_CALL getLength() override;
    virtual sal_Int32 SAL_CALL getTokenByIndex( sal_Int16 i ) override;
    virtual OUString SAL_CALL getValueByIndex( sal_Int16 i ) override;

    // css::util::XCloneable
    virtual css::uno::Reference< css::util::XCloneable > SAL_CALL createClone() override;

    // methods that are not contained in any interface
    void SetValueByToken( sal_Int32 nAttributeToken, const OUString& rValue );
    void SetValueByIndex( sal_Int16 i, const OUString& rValue );
    void AddAttribute( sal_Int32 nAttributeToken , const OUString &sValue );
//  void Clear();
    void RemoveAttributeByIndex( sal_Int16 i );
    void RenameAttributeByIndex( sal_Int16 i, sal_Int32 nNewAttributeToken );
    void AppendAttributeList( const css::uno::Reference< css::xml::sax::XFastAttributeList > & );
    sal_Int16 GetIndexByToken( sal_Int32 nAttributeToken ) const;
};


#endif // INCLUDED_XMLOFF_SOURCE_TRANSFORM_MUTABLEATTRLIST_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
