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



#include "PropertyMap.hxx"
#include <com/sun/star/beans/PropertyValues.hpp>

namespace writerfilter
{
namespace dmapper
{

XMLTag::Pointer_t lcl_TableColumnSeparatorsToTag(const uno::Any & rTableColumnSeparators);
XMLTag::Pointer_t lcl_PropertyValuesToTag(beans::PropertyValues & rValues);

typedef uno::Sequence<beans::PropertyValues> PropertyValueSeq_t;
XMLTag::Pointer_t lcl_PropertyValueSeqToTag(PropertyValueSeq_t & rPropValSeq);

typedef uno::Sequence<PropertyValueSeq_t> PropertyValueSeqSeq_t;
XMLTag::Pointer_t lcl_PropertyValueSeqSeqToTag(PropertyValueSeqSeq_t & rPropValSeqSeq);
}
}
