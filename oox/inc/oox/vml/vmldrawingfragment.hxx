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



#ifndef OOX_VML_VMLDRAWINGFRAGMENT_HXX
#define OOX_VML_VMLDRAWINGFRAGMENT_HXX

#include "oox/core/fragmenthandler2.hxx"

namespace oox {
namespace vml {

class Drawing;

// ============================================================================

class DrawingFragment : public ::oox::core::FragmentHandler2
{
public:
    explicit            DrawingFragment(
                            ::oox::core::XmlFilterBase& rFilter,
                            const ::rtl::OUString& rFragmentPath,
                            Drawing& rDrawing );

    virtual ::com::sun::star::uno::Reference< ::com::sun::star::io::XInputStream >
                        openFragmentStream() const;

    virtual ::oox::core::ContextHandlerRef
                        onCreateContext( sal_Int32 nElement, const AttributeList& rAttribs );

    virtual void        finalizeImport();

private:
    Drawing&            mrDrawing;
};

// ============================================================================

} // namespace vml
} // namespace oox

#endif
