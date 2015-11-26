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

#ifndef INCLUDED_SC_SOURCE_FILTER_INC_RICHSTRINGCONTEXT_HXX
#define INCLUDED_SC_SOURCE_FILTER_INC_RICHSTRINGCONTEXT_HXX

#include "excelhandlers.hxx"
#include "richstring.hxx"
#include <osl/diagnose.h>

namespace oox {
namespace xls {

class RichStringContext : public WorkbookContextBase
{
public:
    template< typename ParentType >
    explicit            RichStringContext( ParentType& rParent, RichStringRef xString );

protected:
    virtual ::oox::core::ContextHandlerRef onCreateContext( sal_Int32 nElement, const AttributeList& rAttribs ) override;
    virtual void        onCharacters( const OUString& rChars ) override;

private:
    RichStringRef       mxString;       /// Processed string.
    RichStringPortionRef mxPortion;     /// Processed portion in the string.
    RichStringPhoneticRef mxPhonetic;   /// Processed phonetic text portion.
};

template< typename ParentType >
RichStringContext::RichStringContext( ParentType& rParent, RichStringRef xString ) :
    WorkbookContextBase( rParent ),
    mxString( xString )
{
    mbEnableTrimSpace = false;
    OSL_ENSURE( mxString.get(), "RichStringContext::RichStringContext - missing string object" );
}

} // namespace xls
} // namespace oox

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
