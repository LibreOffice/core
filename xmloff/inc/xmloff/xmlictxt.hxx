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



#ifndef _XMLOFF_XMLICTXT_HXX
#define _XMLOFF_XMLICTXT_HXX

#include "sal/config.h"
#include "xmloff/dllapi.h"
#include "sal/types.h"
#include <com/sun/star/xml/sax/XAttributeList.hpp>
#include <tools/solar.h>
#include <tools/ref.hxx>
#include <rtl/ustring.hxx>

class SvXMLNamespaceMap;
class SvXMLImport;

class XMLOFF_DLLPUBLIC SvXMLImportContext : public SvRefBase
{
    friend class SvXMLImport;

    SvXMLImport& mrImport;

    sal_uInt16       mnPrefix;
    ::rtl::OUString maLocalName;

    SvXMLNamespaceMap   *mpRewindMap;

    SAL_DLLPRIVATE SvXMLNamespaceMap *GetRewindMap() const
    { return mpRewindMap; }
    SAL_DLLPRIVATE void SetRewindMap( SvXMLNamespaceMap *p ) { mpRewindMap = p; }

protected:

    SvXMLImport& GetImport() { return mrImport; }
    const SvXMLImport& GetImport() const { return mrImport; }

public:
    sal_uInt16 GetPrefix() const { return mnPrefix; }
    const ::rtl::OUString& GetLocalName() const { return maLocalName; }

    // A contexts constructor does anything that is required if an element
    // starts. Namespace processing has been done already.
    // Note that virtual methods cannot be used inside constructors. Use
    // StartElement instead if this is required.
    SvXMLImportContext( SvXMLImport& rImport, sal_uInt16 nPrfx,
                        const ::rtl::OUString& rLName );

    // A contexts destructor does anything that is required if an element
    // ends. By default, nothing is done.
    // Note that virtual methods cannot be used inside destructors. Use
    // EndElement instead if this is required.
    virtual ~SvXMLImportContext();

    // Create a childs element context. By default, the import's
    // CreateContext method is called to create a new default context.
    virtual SvXMLImportContext *CreateChildContext( sal_uInt16 nPrefix,
                                   const ::rtl::OUString& rLocalName,
                                   const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XAttributeList >& xAttrList );

    // StartElement is called after a context has been constructed and
    // before a elements context is parsed. It may be used for actions that
    // require virtual methods. The default is to do nothing.
    virtual void StartElement( const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XAttributeList >& xAttrList );

    // EndElement is called before a context will be destructed, but
    // after a elements context has been parsed. It may be used for actions
    // that require virtual methods. The default is to do nothing.
    virtual void EndElement();

    // This method is called for all characters that are contained in the
    // current element. The default is to ignore them.
    virtual void Characters( const ::rtl::OUString& rChars );

    // #124143# allow to copy evtl. useful data from another temporary import context, e.g. used to
    // support multiple images and to rescue evtl. GluePoints imported with one of the
    // to be deprecated contents
    virtual void onDemandRescueUsefulDataFromTemporary( const SvXMLImportContext& rCandidate );
};

SV_DECL_REF( SvXMLImportContext )
SV_IMPL_REF( SvXMLImportContext )


#endif  //  _XMLOFF_XMLICTXT_HXX

