/*************************************************************************
 *
 *  $RCSfile: binarywritehandler.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: kz $ $Date: 2005-01-18 13:30:23 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/
#ifndef CONFIGMGR_BINARYWRITEHANDLER_HXX
#define CONFIGMGR_BINARYWRITEHANDLER_HXX

#ifndef _CONFIGMGR_TREE_VALUENODE_HXX
#include "valuenode.hxx"
#endif

#ifndef CONFIGMGR_BINARYWRITER_HXX
#include "binarywriter.hxx"
#endif

#ifndef CONFIGMGR_BINARYTYPE_HXX
#include "binarytype.hxx"
#endif

#ifndef CONFIGMGR_CONFIGURATION_ATTRIBUTES_HXX_
#include "attributes.hxx"
#endif

#ifndef CONFIGMGR_MATCHLOCALE_HXX
#include "matchlocale.hxx"
#endif

#ifndef _COM_SUN_STAR_CONFIGURATION_BACKEND_XLAYER_HPP_
#include <com/sun/star/configuration/backend/XLayer.hpp>
#endif // _COM_SUN_STAR_CONFIGURATION_BACKEND_XLAYER_HPP_

#ifndef _COM_SUN_STAR_IO_IOEXCEPTION_HPP_
#include <com/sun/star/io/IOException.hpp>
#endif

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
            BinaryWriteHandler(rtl::OUString const & _aFileURL, rtl::OUString const & _aComponentName, MultiServiceFactory const & _aFactory);

            bool generateHeader(    const uno::Reference<backenduno::XLayer> * pLayers,
                                     sal_Int32 nNumLayers,
                                    const OUString& aEntity,
                                    const localehelper::LocaleSequence & aKnownLocales )
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
            void writeFileHeader(   rtl::OUString const & _aOwnerEntity,
                                    const uno::Sequence<OUString> & aKnownLocales,
                                    const uno::Sequence<OUString> & aDataLocales  )
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

            void BinaryWriteHandler::writeNodeType(binary::NodeType::Type _eType)
                SAL_THROW( (io::IOException, uno::RuntimeException) );

            void BinaryWriteHandler::writeStop()
                SAL_THROW( (io::IOException, uno::RuntimeException) );

            void writeType(uno::Type const& _aType)
                SAL_THROW( (io::IOException, uno::RuntimeException) );

            void BinaryWriteHandler::writeValue( uno::Any const& _aValue)
                SAL_THROW( (io::IOException, uno::RuntimeException) );

        };
    // ---------------------------------------------------------------------------
    }
 // -----------------------------------------------------------------------------
}// namespace configmgr
#endif
