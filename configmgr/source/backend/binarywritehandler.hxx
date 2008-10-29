/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: binarywritehandler.hxx,v $
 * $Revision: 1.8 $
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
#ifndef CONFIGMGR_BINARYWRITEHANDLER_HXX
#define CONFIGMGR_BINARYWRITEHANDLER_HXX

#include "valuenode.hxx"
#include "binarywriter.hxx"
#include "binarytype.hxx"
#include "attributes.hxx"
#include "matchlocale.hxx"
#include <com/sun/star/configuration/backend/XLayer.hpp>
#include <com/sun/star/io/IOException.hpp>

namespace configmgr
{
    // -----------------------------------------------------------------------------
    namespace backend
    {
        namespace css = com::sun::star;

        namespace io   = css::io;
        namespace uno  = css::uno;
        namespace backenduno = css::configuration::backend  ;

        // -----------------------------------------------------------------------------
        class BinaryWriteHandler : private NodeAction
        {
            BinaryWriter    m_BinaryWriter;
            rtl::OUString   m_aComponentName;

        public:
            BinaryWriteHandler(rtl::OUString const & _aFileURL, rtl::OUString const & _aComponentName, uno::Reference<lang::XMultiServiceFactory> const & _aFactory);

            bool generateHeader(    const uno::Reference<backenduno::XLayer> * pLayers,
                                     sal_Int32 nNumLayers,
                                    const rtl::OUString& aEntity,
                                    const std::vector< com::sun::star::lang::Locale > & aKnownLocales )
                SAL_THROW( (io::IOException, uno::RuntimeException) );

            void writeComponentTree(const ISubtree * _pComponentTree)
                SAL_THROW( (io::IOException, uno::RuntimeException) );
            void writeTemplatesTree(const ISubtree * _pTemplatesTree)
                SAL_THROW( (io::IOException, uno::RuntimeException) );
        private:
            void writeTree(ISubtree const & rTree)
                SAL_THROW( (io::IOException, uno::RuntimeException) );

            // Node Action
            virtual void handle(ISubtree  const & aSubtree);
            virtual void handle(ValueNode const & aValue);
        private:
            void writeFileHeader(   rtl::OUString const & _aSchemaVersion,
                                    const uno::Sequence<rtl::OUString> & aKnownLocales,
                                    const uno::Sequence<rtl::OUString> & aDataLocales  )
                SAL_THROW( (io::IOException, uno::RuntimeException) );

            void writeLayerInfoList(uno::Reference<backenduno::XLayer> const * pLayers, sal_Int32 nNumlayers)
                SAL_THROW( (io::IOException, uno::RuntimeException) );

            void writeGroupNode(rtl::OUString const& _aName,node::Attributes const& _aAttributes)
                SAL_THROW( (io::IOException, uno::RuntimeException) );

            void writeValueNode(rtl::OUString const& _aName,
                                node::Attributes const& _aAttributes,
                                uno::Type const& _aType,
                                uno::Any const& _aUserValue,
                                uno::Any const& _aDefaultValue)
                SAL_THROW( (io::IOException, uno::RuntimeException) );

            void writeSetNode(rtl::OUString const& _aName,
                              rtl::OUString const& _aTemplateName,
                              rtl::OUString const& _aTemplateModule,
                              node::Attributes const& _aAttributes)
                SAL_THROW( (io::IOException, uno::RuntimeException) );

            void writeAttributes(node::Attributes const& _aAttributes)
                SAL_THROW( (io::IOException, uno::RuntimeException) );

            void writeNodeType(binary::NodeType::Type _eType)
                SAL_THROW( (io::IOException, uno::RuntimeException) );

            void writeStop()
                SAL_THROW( (io::IOException, uno::RuntimeException) );

            void writeValue( uno::Any const& _aValue)
                SAL_THROW( (io::IOException, uno::RuntimeException) );

        };
    // ---------------------------------------------------------------------------
    }
 // -----------------------------------------------------------------------------
}// namespace configmgr
#endif
