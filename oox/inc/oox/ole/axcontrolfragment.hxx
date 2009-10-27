/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: axcontrolfragment.hxx,v $
 * $Revision: 1.1 $
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#ifndef OOX_OLE_AXCONTROLFRAGMENT_HXX
#define OOX_OLE_AXCONTROLFRAGMENT_HXX

#include "oox/core/fragmenthandler2.hxx"

namespace oox {
namespace ole {

class AxControl;
class AxControlModelBase;

// ============================================================================

/** Context handler for ActiveX form control model properties. */
class AxControlPropertyContext : public ::oox::core::ContextHandler2
{
public:
    explicit            AxControlPropertyContext( ::oox::core::FragmentHandler2& rFragment, AxControlModelBase& rModel );

    virtual ::oox::core::ContextHandlerRef
                        onCreateContext( sal_Int32 nElement, const AttributeList& rAttribs );

private:
    AxControlModelBase& mrModel;
    sal_Int32           mnPropId;           /// Identifier of currently processed property.
};

// ============================================================================

/** Fragment handler for an ActiveX form control fragment. */
class AxControlFragment : public ::oox::core::FragmentHandler2
{
public:
    explicit            AxControlFragment(
                            ::oox::core::XmlFilterBase& rFilter,
                            const ::rtl::OUString& rFragmentPath,
                            AxControl& rControl );

    virtual ::oox::core::ContextHandlerRef
                        onCreateContext( sal_Int32 nElement, const AttributeList& rAttribs );

private:
    AxControl&          mrControl;
};

// ============================================================================

} // namespace ole
} // namespace oox

#endif

