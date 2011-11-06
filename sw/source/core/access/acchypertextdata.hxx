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


#ifndef _ACCHYPERTEXTDATA_HXX
#define _ACCHYPERTEXTDATA_HXX

#include <cppuhelper/weakref.hxx>

#include <map>

class SwTxtAttr;

namespace com { namespace sun { namespace star {
    namespace accessibility { class XAccessibleHyperlink; }
} } }

typedef ::std::less< const SwTxtAttr * > SwTxtAttrPtrLess;
typedef ::std::map < const SwTxtAttr *, ::com::sun::star::uno::WeakReference < com::sun::star::accessibility::XAccessibleHyperlink >, SwTxtAttrPtrLess > _SwAccessibleHyperlinkMap_Impl;

class SwAccessibleHyperTextData : public _SwAccessibleHyperlinkMap_Impl
{
public:
    SwAccessibleHyperTextData();
    ~SwAccessibleHyperTextData();
};

#endif

