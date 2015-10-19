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
#ifndef INCLUDED_I18NPOOL_INC_CHAPTERCOLLATOR_HXX
#define INCLUDED_I18NPOOL_INC_CHAPTERCOLLATOR_HXX

#include <collatorImpl.hxx>
#include <com/sun/star/i18n/XCharacterClassification.hpp>

namespace com { namespace sun { namespace star { namespace i18n {

//      ----------------------------------------------------
//      class ChapterCollator
//      ----------------------------------------------------
class ChapterCollator : public CollatorImpl
{
public:
    // Constructors
    ChapterCollator( const css::uno::Reference < css::uno::XComponentContext >& rxContext );
    // Destructor
    virtual ~ChapterCollator();

    sal_Int32 SAL_CALL compareSubstring( const OUString& s1, sal_Int32 off1, sal_Int32 len1,
        const OUString& s2, sal_Int32 off2, sal_Int32 len2) throw(css::uno::RuntimeException, std::exception) override;
    sal_Int32 SAL_CALL compareString( const OUString& s1, const OUString& s2) throw(css::uno::RuntimeException, std::exception) override;

    //XServiceInfo
    virtual OUString SAL_CALL getImplementationName() throw( css::uno::RuntimeException, std::exception ) override;
    virtual sal_Bool SAL_CALL supportsService(const OUString& ServiceName) throw( css::uno::RuntimeException, std::exception ) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() throw( css::uno::RuntimeException, std::exception ) override;

private:
    // CharacterClassification Implementation
    css::uno::Reference< XCharacterClassification > cclass;
};

} } } }

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
