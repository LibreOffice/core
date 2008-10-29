/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: binaryreadhandler.hxx,v $
 * $Revision: 1.11 $
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
#ifndef CONFIGMGR_BINARYREADHANDLER_HXX
#define CONFIGMGR_BINARYREADHANDLER_HXX

#include "binaryreader.hxx"
#include "componentdatahelper.hxx"
#include "valuenode.hxx"
#include "binarytype.hxx"
#include "attributes.hxx"
#include "matchlocale.hxx"
#include <com/sun/star/configuration/backend/XLayer.hpp>
#include <com/sun/star/io/IOException.hpp>

#ifndef INCLUDED_MEMORY
#include <memory>
#define INCLUDED_MEMORY
#endif // INCLUDED_MEMORY

#ifndef INCLUDED_VECTOR
#include <vector>
#define INCLUDED_VECTOR
#endif // INCLUDED_VECTOR


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

        class BinaryReadHandler
        {
            BinaryReader         m_BinaryReader;
            ComponentDataFactory m_aNodeFactory;
            rtl::OUString        m_aComponentName;

        public:
            BinaryReadHandler(rtl::OUString const & _aFileURL, rtl::OUString const & _aComponentName, uno::Reference<lang::XMultiServiceFactory> const & _aFactory);
            ~BinaryReadHandler();

            bool validateHeader(    const uno::Reference<backenduno::XLayer> * pLayers,
                                    sal_Int32 nNumLayers,
                                    const rtl::OUString& _aSchemaVersion,
                                    com::sun::star::lang::Locale const & aRequestedLocale,
                                    std::vector< com::sun::star::lang::Locale > & outKnownLocales)
                SAL_THROW( (io::IOException, uno::RuntimeException) );

            std::auto_ptr<ISubtree> readComponentTree()
                SAL_THROW( (io::IOException, uno::RuntimeException) );

            std::auto_ptr<ISubtree> readTemplatesTree()
                SAL_THROW( (io::IOException, uno::RuntimeException) );

        private:
            std::auto_ptr<ISubtree> readToplevelTree()
                SAL_THROW( (io::IOException, uno::RuntimeException) );

            void readChildren(ISubtree & rTree)
                SAL_THROW( (io::IOException, uno::RuntimeException) );

        private:
            bool verifyFileHeader(  const uno::Reference<backenduno::XLayer> * pLayers,
                                    sal_Int32 nNumLayers,
                                    const rtl::OUString& _aSchemaVersion,
                                    com::sun::star::lang::Locale const & aRequestedLocale,
                                    std::vector< com::sun::star::lang::Locale > & outKnownLocales);

            bool isUptodate(const std::vector<rtl::OUString> & _timeStamps)
                SAL_THROW( (io::IOException, uno::RuntimeException) );

            void readChildNodes(ISubtree & rSubTree)
                SAL_THROW( (io::IOException, uno::RuntimeException) );

            binary::NodeType::Type readNodeType()
                SAL_THROW( (io::IOException, uno::RuntimeException) );

            binary::ValueFlags::Type readValueFlags(bool& bSeq, bool& hasValue, bool& hasDefault )
                SAL_THROW( (io::IOException, uno::RuntimeException) );

            void readAttributes(node::Attributes  &_aAttributes)
                SAL_THROW( (io::IOException, uno::RuntimeException) );

            void readGroup(rtl::OUString &_aName, node::Attributes &_aAttributes)
                SAL_THROW( (io::IOException, uno::RuntimeException) );

            void readSet(rtl::OUString &_aName, node::Attributes &_aAttributes,
                                            rtl::OUString &_sInstanceName, rtl::OUString &_sInstanceModule)
                SAL_THROW( (io::IOException, uno::RuntimeException) );

            void readValue(rtl::OUString &_aName, node::Attributes &_aAttributes,
                                              uno::Any& _aValue, uno::Any& _aDefaultValue,uno::Type& _aType)
                SAL_THROW( (io::IOException, uno::RuntimeException) );

            void readName(rtl::OUString &_aString)
                SAL_THROW( (io::IOException, uno::RuntimeException) );
        };
    // ---------------------------------------------------------------------------
    }
 // -----------------------------------------------------------------------------
}// namespace configmgr
#endif
