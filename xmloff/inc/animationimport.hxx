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



#ifndef _XMLOFF_ANIMATIONIMPORT_HXX
#define _XMLOFF_ANIMATIONIMPORT_HXX

#include <xmloff/xmlictxt.hxx>
#include <com/sun/star/animations/XAnimationNode.hpp>

//////////////////////////////////////////////////////////////////////////////
// presentations:animations

namespace xmloff
{
class AnimationsImportHelperImpl;

class AnimationNodeContext : public SvXMLImportContext
{
    AnimationsImportHelperImpl* mpHelper;
    bool mbRootContext;
    ::com::sun::star::uno::Reference< ::com::sun::star::animations::XAnimationNode > mxNode;

    void init_node( const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XAttributeList >& xAttrList );

public:
    AnimationNodeContext(
        const ::com::sun::star::uno::Reference< ::com::sun::star::animations::XAnimationNode >& xParentNode,
        SvXMLImport& rImport,
        sal_uInt16 nPrfx,
        const rtl::OUString& rLocalName,
        const com::sun::star::uno::Reference< com::sun::star::xml::sax::XAttributeList>& xAttrList,
        AnimationsImportHelperImpl* mpImpl = NULL );
    virtual ~AnimationNodeContext();

    virtual void StartElement( const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XAttributeList >& xAttrList );

    virtual SvXMLImportContext * CreateChildContext( sal_uInt16 nPrefix, const ::rtl::OUString& rLocalName,
        const com::sun::star::uno::Reference< com::sun::star::xml::sax::XAttributeList>& xAttrList );

    static void postProcessRootNode( SvXMLImport& rImport, const ::com::sun::star::uno::Reference< ::com::sun::star::animations::XAnimationNode >& xNode, ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& xPageProps );
};

}

#endif  //  _XMLOFF_ANIMATIONIMPORT_HXX

