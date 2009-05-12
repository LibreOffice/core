/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: binaryreadhandler.cxx,v $
 * $Revision: 1.12 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_configmgr.hxx"
#include "binaryreadhandler.hxx"
#include "binarycache.hxx"
#include "binarytype.hxx"
#include "simpletypehelper.hxx"
#include "typeconverter.hxx"
#include "treenodefactory.hxx"

#include "tracer.hxx"
#include <com/sun/star/io/WrongFormatException.hpp>
#include <com/sun/star/util/XTimeStamped.hpp>


#define ASCII(x) rtl::OUString::createFromAscii(x)

namespace configmgr
{
    // -----------------------------------------------------------------------------
    namespace backend
    {

        namespace css = com::sun::star;
        namespace io   = css::io;
        namespace util = css::util;

        // -----------------------------------------------------------------------------
        BinaryReadHandler::BinaryReadHandler(rtl::OUString const & _aFileURL,
                                             rtl::OUString const & _aComponentName,
                                             uno::Reference<lang::XMultiServiceFactory> const & )
        : m_BinaryReader(_aFileURL)
        , m_aNodeFactory()
        , m_aComponentName(_aComponentName)
        {
        }
        // -----------------------------------------------------------------------------
        BinaryReadHandler::~BinaryReadHandler()
        {
        }
        // -----------------------------------------------------------------------------

        static inline
        bool isRequestingNoLocale(com::sun::star::lang::Locale const & aRequestedLocale)
        {
            rtl::OUString const & aLanguage = aRequestedLocale.Language;
            return aLanguage.getLength() == 0 || localehelper::isDefaultLanguage(aLanguage);
        }
        // -----------------------------------------------------------------------------

        static
        bool supportsLocale(uno::Sequence< rtl::OUString > const & aStoredLocales, rtl::OUString const & aRequestedLocale)
        {
            for (sal_Int32 ix=0; ix<aStoredLocales.getLength(); ++ix)
                if (aStoredLocales[ix].equalsIgnoreAsciiCase(aRequestedLocale))
                    return true;

            return false;
        }
        // -----------------------------------------------------------------------------

        static
        bool supportsAll(uno::Sequence< rtl::OUString > const & aStoredLocales, uno::Sequence< rtl::OUString > const & aRequestedLocales)
        {
            for (sal_Int32 jx=0; jx<aRequestedLocales.getLength(); ++jx)
                if (!supportsLocale(aStoredLocales,aRequestedLocales[jx]))
                    return false;

            return true;
        }
        // -----------------------------------------------------------------------------

        bool BinaryReadHandler::verifyFileHeader(  const uno::Reference<backenduno::XLayer> * pLayers,
                                    sal_Int32 nNumLayers,
                                    const rtl::OUString& _aSchemaVersion,
                                    com::sun::star::lang::Locale const & aRequestedLocale,
                                    std::vector< com::sun::star::lang::Locale > & outKnownLocales)
        {
            try
            {
                //Open the reader
                sal_Int16 nMagic, nVersion;
                m_BinaryReader.read(nMagic);
                m_BinaryReader.read(nVersion);
                if (nMagic !=binary::CFG_BINARY_MAGIC || nVersion != binary::CFG_BINARY_VERSION )
                    return false;

                rtl::OUString aSchemaVersion;
                m_BinaryReader.read(aSchemaVersion);
                if (!aSchemaVersion.equals(_aSchemaVersion))
                    return false;

                uno::Sequence< rtl::OUString > aAvailableLocales;
                uno::Sequence< rtl::OUString > aKnownLocales;
                m_BinaryReader.read(aKnownLocales);
                m_BinaryReader.read(aAvailableLocales);
                outKnownLocales = localehelper::makeLocaleSequence(aKnownLocales);

                if (isRequestingNoLocale(aRequestedLocale))
                {
                    // any existing combination of locales (including none) is valid
                }
                else if (!localehelper::designatesAllLocales(aRequestedLocale))
                {
                    // one particular locale requested
                    rtl::OUString const aIsoLocale = localehelper::makeIsoLocale(aRequestedLocale);
                    if (!supportsLocale(aKnownLocales,aIsoLocale))
                    {
                        // a locale we didn't load previously
                        return false;
                    }
                    else if (! supportsLocale(aAvailableLocales,aIsoLocale))
                    {
                        // a locale we handled by fallback previously
                        uno::Sequence< rtl::OUString > aNewlyAvailableLocales =
                            getAvailableLocales(pLayers,nNumLayers);

                        // ... and that has become available now
                        if (supportsLocale(aNewlyAvailableLocales,aIsoLocale))
                            return false;

                        // ... or other new locales have been added
                        if (!supportsAll(aAvailableLocales,aNewlyAvailableLocales))
                            return false;
                    }
                }
                else
                {
                    uno::Sequence< rtl::OUString > aNewlyAvailableLocales =
                        getAvailableLocales(pLayers,nNumLayers);

                    // not all locales were requested yet
                    if (!supportsAll(aKnownLocales,aNewlyAvailableLocales))
                        return false;

                    // new locales have been added
                    if (!supportsAll(aAvailableLocales,aNewlyAvailableLocales))
                        return false;
                }

                rtl::OUString aComponentName;
                m_BinaryReader.read(aComponentName);
                if (!aComponentName.equals(m_aComponentName))
                    return false;

                return true;
            }
            catch (uno::Exception &)
            {
                CFG_TRACE_INFO("Binary Cache can't be used");
                return false;
            }
        }
        // -----------------------------------------------------------------------------
        bool BinaryReadHandler::isUptodate(const std::vector<rtl::OUString> & _timeStamps)
            SAL_THROW( (io::IOException, uno::RuntimeException) )
        {
            sal_Int32 nNumBinaryLayers;
            m_BinaryReader.read(nNumBinaryLayers);
            if (nNumBinaryLayers < 0 || sal_uInt32(nNumBinaryLayers) != _timeStamps.size())
                return false;

            for ( std::vector<rtl::OUString>::const_iterator it = _timeStamps.begin();
                  it != _timeStamps.end();
                  ++it)
            {
                rtl::OUString binaryTimeStamp;
                m_BinaryReader.read(binaryTimeStamp);
                if ( ! it->equals(binaryTimeStamp) )
                {
                    return false;
                }
            }

            return true;
        }
        // -----------------------------------------------------------------------------
        binary::NodeType::Type BinaryReadHandler::readNodeType()
            SAL_THROW( (io::IOException, uno::RuntimeException) )
        {
            binary::NodeType::Type eType = binary::NodeType::invalid;

            sal_Int8 nValue;
            m_BinaryReader.read(nValue);

            sal_uInt8 const nRightValue( nValue );
            if ( (nRightValue & binary::NodeType::nodetype_mask) == nRightValue )
                eType = binary::NodeType::Type(nRightValue);

            if (eType == binary::NodeType::invalid)
            {
                OSL_ENSURE(false, "unknown or unhandled node type");
                throw io::WrongFormatException();
            }

            return eType;
        }

        // -----------------------------------------------------------------------------
        binary::ValueFlags::Type BinaryReadHandler::readValueFlags(bool& bSeq, bool& bHasValue, bool& bHasDefault )
            SAL_THROW( (io::IOException, uno::RuntimeException) )
        {
            sal_Int8 nValue;
            m_BinaryReader.read(nValue);

            sal_uInt8 const nRightValue( nValue );
            binary::ValueFlags::Type eType = binary::ValueFlags::Type(nRightValue & binary::ValueFlags::basictype_mask);
            if ( eType >= binary::ValueFlags::val_invalid)
            {
                OSL_ENSURE(false, "unknown or unhandled value type");
                throw io::WrongFormatException();
            }


            if (nRightValue & binary::ValueFlags::seq)
                bSeq = true;
            else
                bSeq = false;

            if (nRightValue & binary::ValueFlags::first_value_NULL)
                bHasValue = false;
            else
                bHasValue = true;

            if (nRightValue & binary::ValueFlags::second_value_NULL)
                bHasDefault = false;
            else
                bHasDefault = true;

            return eType;
        }

        // -----------------------------------------------------------------------------

        void BinaryReadHandler::readName(rtl::OUString &_aString)
            SAL_THROW( (io::IOException, uno::RuntimeException) )
        {
            m_BinaryReader.read (_aString);
        }

        // -----------------------------------------------------------------------------
        void BinaryReadHandler::readAttributes(node::Attributes  &_aAttributes)
            SAL_THROW( (io::IOException, uno::RuntimeException) )
        {
            sal_Int8 nValue;
            m_BinaryReader.read(nValue);

            _aAttributes.setState( node::State(nValue & 0x03) );

            _aAttributes.setAccess( nValue & 0x04 ? true : false, nValue & 0x08 ? true : false);

            _aAttributes.setNullable(nValue & 0x10 ? true : false);
            _aAttributes.setLocalized(nValue & 0x20 ? true : false);
            if(nValue & 0x40 )
            {
                _aAttributes.markMandatory();
            }
            if(nValue & 0x80)
            {
                _aAttributes.markRemovable();
            }
        }
        // -----------------------------------------------------------------------------

        void BinaryReadHandler::readGroup(rtl::OUString &_aName, node::Attributes &_aAttributes)
            SAL_THROW( (io::IOException, uno::RuntimeException) )
        {
            readAttributes(_aAttributes);
            readName(_aName);
        }
        // -----------------------------------------------------------------------------
        void BinaryReadHandler::readSet(rtl::OUString &_aName, node::Attributes &_aAttributes,
                        rtl::OUString &_sInstanceName, rtl::OUString &_sInstanceModule)
            SAL_THROW( (io::IOException, uno::RuntimeException) )
        {
            readAttributes(_aAttributes);
            readName(_aName);
            readName(_sInstanceName);
            readName(_sInstanceModule);
        }

        // -----------------------------------------------------------------------------
        template <class T>
        inline
        void readAsAny(BinaryReader& rBinaryReader, uno::Any & _aValue, uno::Type& _aType, T& _aVar)
            SAL_THROW( (io::IOException, uno::RuntimeException) )
        {
            rBinaryReader.read(_aVar);
            _aValue <<= _aVar;
            _aType = _aValue.getValueType();
        }

        // -----------------------------------------------------------------------------
        uno::Type convertValueTypeToType(binary::ValueFlags::Type eBasicValueType, bool bSeq)
        {
           uno::Type aType;

           if (!bSeq)
           {
                switch(eBasicValueType)
                {
                // --------------- Simple types ---------------

                case binary::ValueFlags::val_boolean:
                    aType = SimpleTypeHelper::getBooleanType();
                    break;
                case binary::ValueFlags::val_int16:
                    aType = SimpleTypeHelper::getShortType();
                    break;
                case binary::ValueFlags::val_int32:
                    aType = SimpleTypeHelper::getIntType();
                    break;
                case binary::ValueFlags::val_int64:
                    aType = SimpleTypeHelper::getLongType();
                    break;
                case binary::ValueFlags::val_double:
                    aType = SimpleTypeHelper::getDoubleType();
                    break;
                case binary::ValueFlags::val_string:
                    aType = SimpleTypeHelper::getStringType();
                    break;
                case binary::ValueFlags::val_binary:
                    aType = SimpleTypeHelper::getBinaryType();
                    break;
                case binary::ValueFlags::val_any:
                    aType = SimpleTypeHelper::getAnyType();
                    break;
                default:
                    OSL_ENSURE(false,"Invalid value type");
                    break;
                }
           }
           else
           {
                // ------------ Sequences ------------
                switch(eBasicValueType)
                {
                case binary::ValueFlags::val_boolean:
                    aType = ::getCppuType(static_cast<uno::Sequence<sal_Bool> const*>(0));
                    break;
                case binary::ValueFlags::val_int16:
                    aType = ::getCppuType(static_cast<uno::Sequence<sal_Int16> const*>(0));
                    break;
                case binary::ValueFlags::val_int32:
                    aType = ::getCppuType(static_cast<uno::Sequence<sal_Int32> const*>(0));
                    break;
                case binary::ValueFlags::val_int64:
                    aType = ::getCppuType(static_cast<uno::Sequence<sal_Int64> const*>(0));
                    break;
                case binary::ValueFlags::val_double:
                    aType = ::getCppuType(static_cast<uno::Sequence<double> const*>(0));
                    break;
                case binary::ValueFlags::val_string:
                    aType = ::getCppuType(static_cast<uno::Sequence<rtl::OUString> const*>(0));
                    break;
                case binary::ValueFlags::val_binary:
                    aType = ::getCppuType(static_cast<uno::Sequence< uno::Sequence< sal_Int8 > > const*>(0));
                    break;
                default:
                    OSL_ENSURE(false,"Invalid sequence value type");
                    break;
                }
            }

            return aType;
        }

        // -----------------------------------------------------------------------------
        void BinaryReadHandler::readValue(rtl::OUString &_aName, node::Attributes &_aAttributes,
                                          uno::Any& _aValue, uno::Any& _aDefaultValue,uno::Type& _aType)
            SAL_THROW( (io::IOException, uno::RuntimeException) )
        {
            bool bSeq = false;
            bool bHasValue = false;
            bool bHasDefault = false;

            binary::ValueFlags::Type eBasicType = readValueFlags(bSeq, bHasValue, bHasDefault);
            readAttributes(_aAttributes);
            readName(_aName);

            if (!bSeq && (bHasValue || bHasDefault))
            {
                switch(eBasicType)
                {
                    case binary::ValueFlags::val_any:
                    {
                        OSL_ENSURE(false,"Node of type Any cannot have value");
                                         throw io::WrongFormatException();
                    }
                    case binary::ValueFlags::val_string:
                    {
                        rtl::OUString aStr;
                        if (bHasValue)
                            readAsAny(m_BinaryReader, _aValue, _aType, aStr);
                        if (bHasDefault)
                            readAsAny(m_BinaryReader,  _aDefaultValue, _aType, aStr);
                        break;
                    }
                    case binary::ValueFlags::val_boolean:
                    {
                        sal_Bool nValue;
                        if (bHasValue)
                            readAsAny(m_BinaryReader,_aValue, _aType, nValue);
                        if (bHasDefault)
                            readAsAny(m_BinaryReader,_aDefaultValue, _aType, nValue);
                        break;
                    }
                    case binary::ValueFlags::val_int16:
                    {
                        sal_Int16 nValue;
                        if (bHasValue)
                            readAsAny(m_BinaryReader,_aValue, _aType, nValue);
                        if (bHasDefault)
                            readAsAny(m_BinaryReader,_aDefaultValue, _aType, nValue);
                        break;
                    }
                    case binary::ValueFlags::val_int32:
                    {
                        sal_Int32 nValue;
                        if (bHasValue)
                            readAsAny(m_BinaryReader,_aValue, _aType, nValue);
                        if (bHasDefault)
                            readAsAny(m_BinaryReader,_aDefaultValue, _aType, nValue);
                        break;
                    }
                    case binary::ValueFlags::val_int64:
                    {
                        sal_Int64 nValue;
                        if (bHasValue)
                            readAsAny(m_BinaryReader,_aValue, _aType, nValue);
                        if (bHasDefault)
                            readAsAny(m_BinaryReader,_aDefaultValue, _aType, nValue);
                        break;
                    }
                    case binary::ValueFlags::val_double:
                    {
                        double nValue;
                        if (bHasValue)
                            readAsAny(m_BinaryReader,_aValue, _aType, nValue);
                        if (bHasDefault)
                            readAsAny(m_BinaryReader, _aDefaultValue, _aType, nValue);
                        break;
                    }
                    case binary::ValueFlags::val_binary:
                    {
                        uno::Sequence< sal_Int8 > aValue;
                        if (bHasValue)
                            readAsAny(m_BinaryReader,_aValue, _aType, aValue);
                        if (bHasDefault)
                            readAsAny(m_BinaryReader,_aDefaultValue, _aType, aValue);
                        break;
                    }
                    default:
                        OSL_ENSURE(false, "Invalid value type");
                        throw io::WrongFormatException();
                }
            }
            else
            {
                _aType = convertValueTypeToType(eBasicType, bSeq);
                if (_aType == uno::Type())
                    throw io::WrongFormatException();
            }

            if(bSeq)
            {
                OSL_ASSERT(_aType.getTypeClass() == uno::TypeClass_SEQUENCE);
                uno::Type const aElementType = getSequenceElementType(_aType);
                if (bHasValue)
                    readSequenceValue(m_BinaryReader, _aValue, aElementType );

                if(bHasDefault)
                    readSequenceValue(m_BinaryReader, _aDefaultValue, aElementType );
            }

        }
        // -----------------------------------------------------------------------------
        std::auto_ptr<ISubtree> BinaryReadHandler::readComponentTree()
            SAL_THROW( (io::IOException, uno::RuntimeException) )
        {
            binary::NodeType::Type eType = this->readNodeType();

            if (eType != binary::NodeType::component)
            {
                // TODO: search for component tree
                OSL_ENSURE(false, "binary Cache: unexpected tree type for component data");
                throw io::WrongFormatException();
            }

            return readToplevelTree();
        }
        // -----------------------------------------------------------------------------
        std::auto_ptr<ISubtree> BinaryReadHandler::readTemplatesTree()
            SAL_THROW( (io::IOException, uno::RuntimeException) )
        {
            binary::NodeType::Type eType = this->readNodeType();

            if (eType != binary::NodeType::templates)
            {
                // TODO: search for templates tree
                OSL_ENSURE(false, "binary Cache: unexpected tree type for template data");
                throw io::WrongFormatException();
            }

            return readToplevelTree();
        }
        // -----------------------------------------------------------------------------

        std::auto_ptr<ISubtree> BinaryReadHandler::readToplevelTree()
            SAL_THROW( (io::IOException, uno::RuntimeException) )
        {
            std::auto_ptr<ISubtree> pTree;

            rtl::OUString aName;
            node::Attributes aAttributes;
            const bool not_extensible = false;

            binary::NodeType::Type eType = this->readNodeType();
            switch (eType)
            {
                case binary::NodeType::groupnode:
                {
                    this->readGroup(aName, aAttributes);
                    pTree  = m_aNodeFactory.createGroup(aName,not_extensible,aAttributes);
                    break;
                }
                case binary::NodeType::setnode:
                {
                    backenduno::TemplateIdentifier aTemplate;
                    this->readSet(aName, aAttributes,aTemplate.Name, aTemplate.Component);
                    pTree  = m_aNodeFactory.createSet(aName,aTemplate,not_extensible,aAttributes);
                    break;
                }
                case binary::NodeType::nodata:
                    break;

                default:
                    OSL_ENSURE(false, "binary Cache - cannot read data: unexpected node type for tree");
                    throw io::WrongFormatException();

            }

            if (pTree.get() != NULL)
            {
                readChildren(*pTree);

                //read terminating stop node
                eType = this->readNodeType();
                OSL_ENSURE(binary::NodeType::stop == eType, "Missing stop node to mark end of tree");
            }
            return pTree;
        }


    // -----------------------------------------------------------------------------
        bool BinaryReadHandler::validateHeader( const uno::Reference<backenduno::XLayer> * pLayers,
                                                sal_Int32 nNumLayers,
                                                const rtl::OUString& _aSchemaVersion,
                                                com::sun::star::lang::Locale const & aRequestedLocale,
                                                std::vector< com::sun::star::lang::Locale > & outKnownLocales)
            SAL_THROW( (io::IOException, uno::RuntimeException) )
        {

            //Check here if the file exists
            if (!m_BinaryReader.open())
                return false;

            if(!this->verifyFileHeader(pLayers, nNumLayers, _aSchemaVersion, aRequestedLocale, outKnownLocales))
                return false;

            //Check if layers are uptodate
            std::vector <rtl::OUString> timeStamps(nNumLayers);
            for (sal_Int32 i = 0 ; i < nNumLayers ; ++ i)
            {
                uno::Reference<util::XTimeStamped> xTimeStamp = uno::Reference<util::XTimeStamped>(pLayers[i], uno::UNO_QUERY);
                if (xTimeStamp.is())
                {
                    rtl::OUString aTimeStamp = xTimeStamp->getTimestamp();
                    timeStamps[i] = aTimeStamp;
                }
            }

            return this->isUptodate(timeStamps);
        }
        // -----------------------------------------------------------------------------
        void BinaryReadHandler::readChildren(ISubtree & rTree )
        {
            OSL_ASSERT(!binary::NodeType::stop); // loop stops at stop node
            while (binary::NodeType::Type eType = this->readNodeType())
            {
                rtl::OUString aName;
                node::Attributes aAttributes;
                const bool not_extensible = false;
                switch (eType)
                {
                    case binary::NodeType::groupnode:
                    {
                        this->readGroup(aName, aAttributes);

                        std::auto_ptr<ISubtree> pNewNode = m_aNodeFactory.createGroup(aName, not_extensible, aAttributes);
                        if (pNewNode.get() == NULL)
                        {
                            OSL_ENSURE(false, "Error: factory returned NULL group node - skipping");
                            continue;
                        }

                        readChildren( *pNewNode );
                        rTree.addChild(base_ptr(pNewNode));
                        break;
                    }
                    case binary::NodeType::setnode:
                    {
                        backenduno::TemplateIdentifier aTemplate;
                        this->readSet(aName, aAttributes,aTemplate.Name, aTemplate.Component);

                        std::auto_ptr<ISubtree> pNewSetNode = m_aNodeFactory.createSet(aName,aTemplate,not_extensible,aAttributes);
                        if (pNewSetNode.get() == NULL)
                        {
                            OSL_ENSURE(false, "Error: factory returned NULL set node - skipping");
                            continue;
                        }

                        readChildren( *pNewSetNode );
                        rTree.addChild(base_ptr(pNewSetNode));
                        break;
                    }
                    case binary::NodeType::valuenode:
                    {
                        uno::Any aValue;
                        uno::Any aDefaultValue;
                        uno::Type aType;

                        this->readValue(aName, aAttributes, aValue, aDefaultValue, aType);

                        OTreeNodeFactory & rValueFactory = m_aNodeFactory.getNodeFactory();

                        std::auto_ptr<ValueNode> pValueNode;
                        if (aValue.hasValue() || aDefaultValue.hasValue())
                        {
                            pValueNode  = rValueFactory.createValueNode(aName,aValue, aDefaultValue, aAttributes);
                        }
                        else
                        {
                            pValueNode  = rValueFactory.createNullValueNode(aName,aType, aAttributes);
                        }

                        rTree.addChild(base_ptr(pValueNode));
                        break;
                    }
                    default:
                        OSL_ENSURE(false, "Binary cache: Invalid node type");
                        throw io::WrongFormatException();
                }
           }
        }
    // -----------------------------------------------------------------------------

        // -----------------------------------------------------------------------------
    }
   // -----------------------------------------------------------------------------
}
