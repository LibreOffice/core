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



#ifndef OOX_OLE_VBAPROJECTFILTER_HXX
#define OOX_OLE_VBAPROJECTFILTER_HXX

#include "oox/core/binaryfilterbase.hxx"

namespace oox {
namespace ole {

// ============================================================================

class VbaProjectFilterBase : public ::oox::core::BinaryFilterBase
{
public:
    explicit            VbaProjectFilterBase(
                            const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >& rxContext,
                            const ::rtl::OUString& rAppName,
                            const ::rtl::OUString& rStorageName )
                            throw( ::com::sun::star::uno::RuntimeException );

    virtual bool        importDocument() throw();
    virtual bool        exportDocument() throw();

private:
    virtual VbaProject* implCreateVbaProject() const;

private:
    ::rtl::OUString     maAppName;
    ::rtl::OUString     maStorageName;
};

// ============================================================================

class WordVbaProjectFilter : public VbaProjectFilterBase
{
public:
    explicit            WordVbaProjectFilter(
                            const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >& rxContext )
                            throw( ::com::sun::star::uno::RuntimeException );

private:
    virtual ::rtl::OUString implGetImplementationName() const;
};

// ============================================================================

} // namespace ole
} // namespace oox

#endif
