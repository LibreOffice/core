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

#ifndef INCLUDED_XMLOFF_ATTRLIST_HXX
#define INCLUDED_XMLOFF_ATTRLIST_HXX

#include <sal/config.h>
#include <xmloff/dllapi.h>
#include <com/sun/star/util/XCloneable.hpp>
#include <com/sun/star/xml/sax/SAXParseException.hpp>
#include <com/sun/star/xml/sax/XExtendedDocumentHandler.hpp>
#include <com/sun/star/xml/sax/SAXException.hpp>
#include <com/sun/star/xml/sax/XDocumentHandler.hpp>
#include <com/sun/star/xml/sax/XAttributeList.hpp>
#include <com/sun/star/xml/sax/XLocator.hpp>
#include <com/sun/star/lang/XUnoTunnel.hpp>

#include <cppuhelper/implbase3.hxx>
#include <memory>

struct SvXMLAttributeList_Impl;

class XMLOFF_DLLPUBLIC SvXMLAttributeList : public ::cppu::WeakImplHelper3<
        css::xml::sax::XAttributeList,
        css::util::XCloneable,
        css::lang::XUnoTunnel>
{
    std::unique_ptr<SvXMLAttributeList_Impl> m_pImpl;

public:
    SvXMLAttributeList();
    SvXMLAttributeList( const SvXMLAttributeList& );
    SvXMLAttributeList( const css::uno::Reference<
        css::xml::sax::XAttributeList> & rAttrList );
    virtual ~SvXMLAttributeList();

    static const css::uno::Sequence< sal_Int8 > & getUnoTunnelId() throw();
    static SvXMLAttributeList* getImplementation( css::uno::Reference< css::uno::XInterface > ) throw();

    // XUnoTunnel
    virtual sal_Int64 SAL_CALL getSomething( const css::uno::Sequence< sal_Int8 >& aIdentifier ) throw(css::uno::RuntimeException, std::exception) override;

    // css::xml::sax::XAttributeList
    virtual sal_Int16 SAL_CALL getLength() throw( css::uno::RuntimeException, std::exception ) override;
    virtual OUString SAL_CALL getNameByIndex(sal_Int16 i) throw( css::uno::RuntimeException, std::exception ) override;
    virtual OUString SAL_CALL getTypeByIndex(sal_Int16 i) throw( css::uno::RuntimeException, std::exception ) override;
    virtual OUString SAL_CALL getTypeByName(const OUString& aName) throw( css::uno::RuntimeException, std::exception ) override;
    virtual OUString SAL_CALL getValueByIndex(sal_Int16 i) throw( css::uno::RuntimeException, std::exception ) override;
    virtual OUString SAL_CALL getValueByName(const OUString& aName) throw( css::uno::RuntimeException, std::exception ) override;

    // css::util::XCloneable
    virtual css::uno::Reference< css::util::XCloneable > SAL_CALL createClone()   throw( css::uno::RuntimeException, std::exception ) override;

    // methods that are not contained in any interface
    void AddAttribute( const OUString &sName , const OUString &sValue );
    void Clear();
    void RemoveAttribute( const OUString& sName );
    void AppendAttributeList( const css::uno::Reference< css::xml::sax::XAttributeList > & );
    void SetValueByIndex( sal_Int16 i, const OUString& rValue );
    void RemoveAttributeByIndex( sal_Int16 i );
    void RenameAttributeByIndex( sal_Int16 i, const OUString& rNewName );
    sal_Int16 GetIndexByName( const OUString& rName ) const;

 private:
    const OUString sType; // "CDATA"
};


#endif // INCLUDED_XMLOFF_ATTRLIST_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
