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


#ifndef OOX_PPT_COMMONBEHAVIORCONTEXT
#define OOX_PPT_COMMONBEHAVIORCONTEXT

#include <rtl/ustring.hxx>
#include "oox/ppt/timenodelistcontext.hxx"
#include "oox/ppt/animationspersist.hxx"
#include "conditioncontext.hxx"
#include "pptfilterhelpers.hxx"

namespace oox { namespace ppt {

    struct Attribute
    {
        OUString   name;
        MS_AttributeNames type;
    };


    /** CT_TLCommonBehaviorData */
    class CommonBehaviorContext
        : public TimeNodeContext
    {
    public:
        CommonBehaviorContext( ::oox::core::FragmentHandler2& rParent,
             const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XFastAttributeList >& xAttribs,
             const TimeNodePtr & pNode );
        ~CommonBehaviorContext( )
            throw( );

        virtual void onEndElement();

        virtual void onCharacters( const OUString& aChars );

        virtual ::oox::core::ContextHandlerRef onCreateContext( sal_Int32 aElementToken, const AttributeList& rAttribs );

    private:
        bool              mbInAttrList;
        bool              mbIsInAttrName;
        std::list< Attribute > maAttributes;
        OUString   msCurrentAttribute;
    };


} }


#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
