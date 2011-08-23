/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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

#ifndef __ATK_ATKTEXTATTRIBUTES_HXX__
#define __ATK_ATKTEXTATTRIBUTES_HXX__

#include <com/sun/star/uno/Sequence.hxx>
#include <com/sun/star/beans/PropertyValue.hpp>

#include <atk/atk.h>

AtkAttributeSet*
attribute_set_new_from_property_values(
    const com::sun::star::uno::Sequence< com::sun::star::beans::PropertyValue >& rAttributeList,
    bool run_attributes_only,
    AtkText *text);

bool
attribute_set_map_to_property_values(
    AtkAttributeSet* attribute_set,
    com::sun::star::uno::Sequence< com::sun::star::beans::PropertyValue >& rValueList );

AtkAttributeSet* attribute_set_prepend_misspelled( AtkAttributeSet* attribute_set );
// --> OD 2010-03-01 #i92232#
AtkAttributeSet* attribute_set_prepend_tracked_change_insertion( AtkAttributeSet* attribute_set );
AtkAttributeSet* attribute_set_prepend_tracked_change_deletion( AtkAttributeSet* attribute_set );
AtkAttributeSet* attribute_set_prepend_tracked_change_formatchange( AtkAttributeSet* attribute_set );
// <--

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
