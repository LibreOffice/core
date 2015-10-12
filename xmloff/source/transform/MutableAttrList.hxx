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
#include <com/sun/star/xml/sax/XAttributeList.hpp>
#include <com/sun/star/lang/XUnoTunnel.hpp>

#include <cppuhelper/implbase.hxx>

class SvXMLAttributeList;

class XMLMutableAttributeList : public ::cppu::WeakImplHelper<
        ::com::sun::star::xml::sax::XAttributeList,
        ::com::sun::star::util::XCloneable,
        ::com::sun::star::lang::XUnoTunnel>
{
    ::com::sun::star::uno::Reference<
        ::com::sun::star::xml::sax::XAttributeList> m_xAttrList;

    SvXMLAttributeList *m_pMutableAttrList;

    SvXMLAttributeList *GetMutableAttrList();

public:
    XMLMutableAttributeList();
    XMLMutableAttributeList( const ::com::sun::star::uno::Reference<
        ::com::sun::star::xml::sax::XAttributeList> & rAttrList,
           bool bClone=false );
    virtual ~XMLMutableAttributeList();

    static const ::com::sun::star::uno::Sequence< sal_Int8 > & getUnoTunnelId() throw();

    // XUnoTunnel
    virtual sal_Int64 SAL_CALL getSomething( const ::com::sun::star::uno::Sequence< sal_Int8 >& aIdentifier ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;

    // ::com::sun::star::xml::sax::XAttributeList
    virtual sal_Int16 SAL_CALL getLength() throw( ::com::sun::star::uno::RuntimeException, std::exception ) override;
    virtual OUString SAL_CALL getNameByIndex(sal_Int16 i) throw( ::com::sun::star::uno::RuntimeException, std::exception ) override;
    virtual OUString SAL_CALL getTypeByIndex(sal_Int16 i) throw( ::com::sun::star::uno::RuntimeException, std::exception ) override;
    virtual OUString SAL_CALL getTypeByName(const OUString& aName) throw( ::com::sun::star::uno::RuntimeException, std::exception ) override;
    virtual OUString SAL_CALL getValueByIndex(sal_Int16 i) throw( ::com::sun::star::uno::RuntimeException, std::exception ) override;
    virtual OUString SAL_CALL getValueByName(const OUString& aName) throw( ::com::sun::star::uno::RuntimeException, std::exception ) override;

    // ::com::sun::star::util::XCloneable
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::util::XCloneable > SAL_CALL createClone()   throw( ::com::sun::star::uno::RuntimeException, std::exception ) override;

    // methods that are not contained in any interface
    void SetValueByIndex( sal_Int16 i, const OUString& rValue );
    void AddAttribute( const OUString &sName , const OUString &sValue );
//  void Clear();
    void RemoveAttributeByIndex( sal_Int16 i );
    void RenameAttributeByIndex( sal_Int16 i, const OUString& rNewName );
    void AppendAttributeList( const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XAttributeList > & );

    sal_Int16 GetIndexByName( const OUString& rName ) const;
};


#endif // INCLUDED_XMLOFF_SOURCE_TRANSFORM_MUTABLEATTRLIST_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
