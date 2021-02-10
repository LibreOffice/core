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

#ifndef INCLUDED_OOX_PPT_TIMENODELISTCONTEXT_HXX
#define INCLUDED_OOX_PPT_TIMENODELISTCONTEXT_HXX

#include <com/sun/star/uno/Reference.hxx>
#include <oox/core/contexthandler.hxx>
#include <oox/core/fragmenthandler2.hxx>
#include <oox/ppt/timenode.hxx>
#include <sal/types.h>

namespace com::sun::star {
    namespace xml::sax { class XFastAttributeList; }
}

namespace oox { class AttributeList; }

namespace oox::ppt {


    class TimeNodeContext : public ::oox::core::FragmentHandler2
    {
    public:
        virtual ~TimeNodeContext() throw() override;

        static rtl::Reference<TimeNodeContext> makeContext( ::oox::core::FragmentHandler2 const & rParent, sal_Int32  aElement, const css::uno::Reference< css::xml::sax::XFastAttributeList >& xAttribs, const TimeNodePtr & pNode );

    protected:
        TimeNodeContext( ::oox::core::FragmentHandler2 const & rParent, sal_Int32  aElement, const TimeNodePtr & pNode ) throw();

        sal_Int32 mnElement;
        TimeNodePtr mpNode;
    };


/** FastParser context for XML_tnLst, XML_subTnLst and XML_childTnLst */
class TimeNodeListContext final : public ::oox::core::FragmentHandler2
{
public:
    TimeNodeListContext( ::oox::core::FragmentHandler2 const & rParent, TimeNodePtrList & aList ) throw();

    virtual ~TimeNodeListContext() throw() override;
    virtual ::oox::core::ContextHandlerRef onCreateContext( sal_Int32 aElementToken, const AttributeList& rAttribs ) override;


private:
    TimeNodePtrList & maList;
};

}

#endif // INCLUDED_OOX_PPT_TIMENODELISTCONTEXT_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
