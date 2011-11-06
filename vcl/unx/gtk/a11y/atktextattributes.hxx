/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



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
