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

#ifndef INCLUDED_XMLOFF_SOURCE_FORMS_ATTRIBLISTMERGE_HXX
#define INCLUDED_XMLOFF_SOURCE_FORMS_ATTRIBLISTMERGE_HXX

#include <sal/config.h>

#include <vector>

#include <cppuhelper/implbase.hxx>
#include <osl/mutex.hxx>
#include <com/sun/star/xml/sax/XAttributeList.hpp>

namespace xmloff
{

    //= OAttribListMerger
    typedef ::cppu::WeakImplHelper <   css::xml::sax::XAttributeList
                                    >   OAttribListMerger_Base;
    /** implements the XAttributeList list by merging different source attribute lists

        <p>Currently, the time behavious is O(n), though it would be possible to change it to O(log n).</p>
    */
    class OAttribListMerger : public OAttribListMerger_Base
    {
    protected:
        ::osl::Mutex        m_aMutex;
        typedef std::vector< css::uno::Reference< css::xml::sax::XAttributeList > > AttributeListArray;
        AttributeListArray  m_aLists;

        virtual ~OAttribListMerger() { }

    public:
        OAttribListMerger() { }

        // attribute list handling
        // (very thin at the moment... only adding lists is allowed... add more if you need it :)
        void addList(const css::uno::Reference< css::xml::sax::XAttributeList >& _rList);

        // XAttributeList
        virtual sal_Int16 SAL_CALL getLength(  ) throw(css::uno::RuntimeException, std::exception) override;
        virtual OUString SAL_CALL getNameByIndex( sal_Int16 i ) throw(css::uno::RuntimeException, std::exception) override;
        virtual OUString SAL_CALL getTypeByIndex( sal_Int16 i ) throw(css::uno::RuntimeException, std::exception) override;
        virtual OUString SAL_CALL getTypeByName( const OUString& aName ) throw(css::uno::RuntimeException, std::exception) override;
        virtual OUString SAL_CALL getValueByIndex( sal_Int16 i ) throw(css::uno::RuntimeException, std::exception) override;
        virtual OUString SAL_CALL getValueByName( const OUString& aName ) throw(css::uno::RuntimeException, std::exception) override;

    protected:
        bool seekToIndex(sal_Int16 _nGlobalIndex, css::uno::Reference< css::xml::sax::XAttributeList >& _rSubList, sal_Int16& _rLocalIndex);
        bool seekToName(const OUString& _rName, css::uno::Reference< css::xml::sax::XAttributeList >& _rSubList, sal_Int16& _rLocalIndex);
    };

}   // namespace xmloff

#endif // INCLUDED_XMLOFF_SOURCE_FORMS_ATTRIBLISTMERGE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
