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
#ifndef INCLUDED_EXTENSIONS_SOURCE_PROPCTRLR_LINEDESCRIPTOR_HXX
#define INCLUDED_EXTENSIONS_SOURCE_PROPCTRLR_LINEDESCRIPTOR_HXX

#include <vcl/image.hxx>
#include <com/sun/star/inspection/LineDescriptor.hpp>


namespace pcr
{



    //= OLineDescriptor

    struct OLineDescriptor : public css::inspection::LineDescriptor
    {
        OUString                    sName;              // the name of the property
        css::uno::Reference< css::inspection::XPropertyHandler >
                                    xPropertyHandler;   // the handler for this property
        css::uno::Any               aValue;             // the current value of the property

        bool                        bUnknownValue    : 1;   // is the property value currently "unknown"? (PropertyState_AMBIGUOUS)
        bool                        bReadOnly        : 1;

        OLineDescriptor()
            :bUnknownValue( false )
            ,bReadOnly( false )
        {
        }

        void assignFrom( const css::inspection::LineDescriptor& _rhs )
        {
            css::inspection::LineDescriptor::operator=( _rhs );
        }
    };


} // namespace pcr


#endif // INCLUDED_EXTENSIONS_SOURCE_PROPCTRLR_LINEDESCRIPTOR_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
