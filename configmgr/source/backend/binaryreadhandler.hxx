/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: binaryreadhandler.hxx,v $
 *
 *  $Revision: 1.8 $
 *
 *  last change: $Author: kz $ $Date: 2006-11-06 14:46:59 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/
#ifndef CONFIGMGR_BINARYREADHANDLER_HXX
#define CONFIGMGR_BINARYREADHANDLER_HXX

#ifndef CONFIGMGR_BINARYREADER_HXX
#include "binaryreader.hxx"
#endif

#ifndef CONFIGMGR_BACKEND_COMPONENTDATAHELPER_HXX
#include "componentdatahelper.hxx"
#endif
#ifndef _CONFIGMGR_TREE_VALUENODE_HXX
#include "valuenode.hxx"
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

            // pool recent strings for re-use
            enum NamePool {
                    GroupName,
                    SetName,
                    InstanceName,
                    InstanceModule,
                    ValueName,
                    LastEntry
            };
            int           m_nInsert[LastEntry];
            rtl::OUString m_aPreviousName[LastEntry][8];
        public:
            BinaryReadHandler(rtl::OUString const & _aFileURL, rtl::OUString const & _aComponentName, MultiServiceFactory const & _aFactory);
            ~BinaryReadHandler();

            bool validateHeader(    const uno::Reference<backenduno::XLayer> * pLayers,
                                    sal_Int32 nNumLayers,
                                    const OUString& _aSchemaVersion,
                                    localehelper::Locale const & aRequestedLocale,
                                    localehelper::LocaleSequence & outKnownLocales)
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
                                    const OUString& _aSchemaVersion,
                                    localehelper::Locale const & aRequestedLocale,
                                    localehelper::LocaleSequence & outKnownLocales);

            bool isUptodate(const std::vector<rtl::OUString> & _timeStamps)
                SAL_THROW( (io::IOException, uno::RuntimeException) );

            void readChildNodes(ISubtree & rSubTree)
                SAL_THROW( (io::IOException, uno::RuntimeException) );

            binary::NodeType::Type readNodeType()
                SAL_THROW( (io::IOException, uno::RuntimeException) );

            binary::ValueFlags::Type readValueFlags(bool& bSeq, bool& hasValue, bool& hasDefault )
                SAL_THROW( (io::IOException, uno::RuntimeException) );

            void readString(rtl::OUString &_aString)
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

            void readName(rtl::OUString &_aString, NamePool ePool)
                SAL_THROW( (io::IOException, uno::RuntimeException) );
        };
    // ---------------------------------------------------------------------------
    }
 // -----------------------------------------------------------------------------
}// namespace configmgr
#endif
