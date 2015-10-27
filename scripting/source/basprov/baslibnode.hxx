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

#ifndef INCLUDED_SCRIPTING_SOURCE_BASPROV_BASLIBNODE_HXX
#define INCLUDED_SCRIPTING_SOURCE_BASPROV_BASLIBNODE_HXX

#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/script/XLibraryContainer.hpp>
#include <com/sun/star/script/browse/XBrowseNode.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <cppuhelper/implbase.hxx>

class BasicManager;



namespace basprov
{



    //  class BasicLibraryNodeImpl


    typedef ::cppu::WeakImplHelper<
        css::script::browse::XBrowseNode > BasicLibraryNodeImpl_BASE;


    class BasicLibraryNodeImpl : public BasicLibraryNodeImpl_BASE
    {
    private:
        css::uno::Reference< css::uno::XComponentContext >    m_xContext;
        OUString                                              m_sScriptingContext;
        BasicManager*                                         m_pBasicManager;
        css::uno::Reference< css::script::XLibraryContainer > m_xLibContainer;
        css::uno::Reference< css::container::XNameContainer > m_xLibrary;
        OUString                                              m_sLibName;
        bool                                                  m_bIsAppScript;

    public:
        BasicLibraryNodeImpl( const css::uno::Reference< css::uno::XComponentContext >& rxContext,
        const OUString& sScriptingContext,
            BasicManager* pBasicManager,
            const css::uno::Reference< css::script::XLibraryContainer >& xLibContainer,
            const OUString& sLibName, bool isAppScript=true );
        virtual ~BasicLibraryNodeImpl();

        // XBrowseNode
        virtual OUString SAL_CALL getName(  )
            throw (css::uno::RuntimeException, std::exception) override;
        virtual css::uno::Sequence< css::uno::Reference< css::script::browse::XBrowseNode > > SAL_CALL getChildNodes(  )
            throw (css::uno::RuntimeException, std::exception) override;
        virtual sal_Bool SAL_CALL hasChildNodes(  )
            throw (css::uno::RuntimeException, std::exception) override;
        virtual sal_Int16 SAL_CALL getType(  )
            throw (css::uno::RuntimeException, std::exception) override;
    };


}   // namespace basprov


#endif // INCLUDED_SCRIPTING_SOURCE_BASPROV_BASLIBNODE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
