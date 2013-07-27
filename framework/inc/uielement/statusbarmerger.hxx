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

#ifndef __FRAMEWORK_UIELEMENT_STATUSBARMERGER_HXX_
#define __FRAMEWORK_UIELEMENT_STATUSBARMERGER_HXX_

#include <com/sun/star/beans/PropertyValue.hpp>
#include <rtl/ustring.hxx>
#include <uielement/statusbar.hxx>

namespace framework
{

struct AddonStatusbarItemData
{
    rtl::OUString aLabel;
    sal_uInt16    nItemBits;
};

struct AddonStatusbarItem
{
    rtl::OUString aCommandURL;
    rtl::OUString aLabel;
    rtl::OUString aContext;
    sal_uInt16    nItemBits;
    sal_Int16     nWidth;
};

typedef ::std::vector< AddonStatusbarItem > AddonStatusbarItemContainer;

class StatusbarMerger
{
public:
    static bool IsCorrectContext( const ::rtl::OUString& aContext,
                                  const ::rtl::OUString& aModuleIdentifier );

    static bool ConvertSeqSeqToVector( const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue > > &rSequence,
                                       AddonStatusbarItemContainer& rContainer );

    static sal_uInt16 FindReferencePos( StatusBar* pStatusbar,
                                        const ::rtl::OUString& rReferencePoint );

    static bool ProcessMergeOperation( StatusBar* pStatusbar,
                                       sal_uInt16 nPos,
                                       sal_uInt16& rItemId,
                                       const ::rtl::OUString& rModuleIdentifier,
                                       const ::rtl::OUString& rMergeCommand,
                                       const ::rtl::OUString& rMergeCommandParameter,
                                       const AddonStatusbarItemContainer& rItems );

    static bool ProcessMergeFallback( StatusBar* pStatusbar,
                                      sal_uInt16 nPos,
                                      sal_uInt16& rItemId,
                                      const ::rtl::OUString& rModuleIdentifier,
                                      const ::rtl::OUString& rMergeCommand,
                                      const ::rtl::OUString& rMergeFallback,
                                      const AddonStatusbarItemContainer& rItems );

private:
    StatusbarMerger();
    StatusbarMerger( const StatusbarMerger& );
    StatusbarMerger& operator=( const StatusbarMerger& );
};

}

#endif
