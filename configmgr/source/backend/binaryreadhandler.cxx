/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: binaryreadhandler.cxx,v $
 *
 *  $Revision: 1.9 $
 *
 *  last change: $Author: kz $ $Date: 2006-11-06 14:46:33 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_configmgr.hxx"
#include "binaryreadhandler.hxx"
#include "binarycache.hxx"
#include "binarytype.hxx"

#ifndef CONFIGMGR_SIMPLETYPEHELPER_HXX
#include "simpletypehelper.hxx"
#endif

#ifndef CONFIGMGR_TYPECONVERTER_HXX
#include "typeconverter.hxx"
#endif
#ifndef CONFIGMGR_TREE_NODEFACTORY_HXX
#include "treenodefactory.hxx"
#endif

#include "tracer.hxx"

#ifndef _COM_SUN_STAR_IO_WRONGFORMATEXCEPTION_HPP_
#include <com/sun/star/io/WrongFormatException.hpp>
#endif
#ifndef _COM_SUN_STAR_UTIL_XTIMESTAMPED_HPP_
#include <com/sun/star/util/XTimeStamped.hpp>
#endif


#define ASCII(x) rtl::OUString::createFromAscii(x)

namespace configmgr
{
    using namespace binary;
    // -----------------------------------------------------------------------------
    namespace backend
    {

        namespace css = com::sun::star;
        namespace io   = css::io;
        namespace util = css::util;

        // -----------------------------------------------------------------------------
        BinaryReadHandler::BinaryReadHandler(rtl::OUString const & _aFileURL,
                                             rtl::OUString const & _aComponentName,
                                             MultiServiceFactory const & )
        : m_BinaryReader(_aFileURL)
        , m_aNodeFactory()
        , m_aComponentName(_aComponentName)
        {
            for (int i = 0; i < LastEntry; i++)
                m_nInsert[i] = 0;
        }
        // -----------------------------------------------------------------------------
        BinaryReadHandler::~BinaryReadHandler()
        {
        }
        // -----------------------------------------------------------------------------

        static inline
        bool isRequestingNoLocale(localehelper::Locale const & aRequestedLocale)
        {
            OUString const & aLanguage = aRequestedLocale.Language;
            return aLanguage.getLength() == 0 || localehelper::isDefaultLanguage(aLanguage);
        }
        // -----------------------------------------------------------------------------

        static
        bool supportsLocale(uno::Sequence< OUString > const & aStoredLocales, OUString const & aRequestedLocale)
        {
            for (sal_Int32 ix=0; ix<aStoredLocales.getLength(); ++ix)
                if (aStoredLocales[ix].equalsIgnoreAsciiCase(aRequestedLocale))
                    return true;

            return false;
        }
        // -----------------------------------------------------------------------------

        static
        bool supportsAll(uno::Sequence< OUString > const & aStoredLocales, uno::Sequence< OUString > const & aRequestedLocales)
        {
            for (sal_Int32 jx=0; jx<aRequestedLocales.getLength(); ++jx)
                if (!supportsLocale(aStoredLocales,aRequestedLocales[jx]))
                    return false;

            return true;
        }
        // -----------------------------------------------------------------------------

        bool BinaryReadHandler::verifyFileHeader(  const uno::Reference<backenduno::XLayer> * pLayers,
                                    sal_Int32 nNumLayers,
                                    const OUString& _aSchemaVersion,
                                    localehelper::Locale const & aRequestedLocale,
                                    localehelper::LocaleSequence & outKnownLocales)
        {
            try
            {
                //Open the reader
                sal_Int16 nMagic, nVersion;
                m_BinaryReader.read(nMagic);
                m_BinaryReader.read(nVersion);
                if (nMagic !=CFG_BINARY_MAGIC || nVersion != CFG_BINARY_VERSION )
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
                    OUString const aIsoLocale = localehelper::makeIsoLocale(aRequestedLocale);
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
        NodeType::Type BinaryReadHandler::readNodeType()
            SAL_THROW( (io::IOException, uno::RuntimeException) )
        {
            NodeType::Type eType = NodeType::invalid;

            sal_Int8 nValue;
            m_BinaryReader.read(nValue);

            sal_uInt8 const nRightValue( nValue );
            if ( (nRightValue & NodeType::nodetype_mask) == nRightValue )
                eType = NodeType::Type(nRightValue);

            if (eType == NodeType::invalid)
            {
                OSL_ENSURE(false, "unknown or unhandled node type");
                throw io::WrongFormatException();
            }

            return eType;
        }

        // -----------------------------------------------------------------------------
        ValueFlags::Type BinaryReadHandler::readValueFlags(bool& bSeq, bool& bHasValue, bool& bHasDefault )
            SAL_THROW( (io::IOException, uno::RuntimeException) )
        {
            sal_Int8 nValue;
            m_BinaryReader.read(nValue);

            sal_uInt8 const nRightValue( nValue );
            ValueFlags::Type eType = ValueFlags::Type(nRightValue & ValueFlags::basictype_mask);
            if ( eType >= ValueFlags::val_invalid)
            {
                OSL_ENSURE(false, "unknown or unhandled value type");
                throw io::WrongFormatException();
            }


            if (nRightValue & ValueFlags::seq)
                bSeq = true;
            else
                bSeq = false;

            if (nRightValue & ValueFlags::first_value_NULL)
                bHasValue = false;
            else
                bHasValue = true;

            if (nRightValue & ValueFlags::second_value_NULL)
                bHasDefault = false;
            else
                bHasDefault = true;

            return eType;
        }

        // -----------------------------------------------------------------------------

        void BinaryReadHandler::readString(rtl::OUString &_aString)
            SAL_THROW( (io::IOException, uno::RuntimeException) )
        {
            m_BinaryReader.read (_aString);
        }

        void BinaryReadHandler::readName(rtl::OUString &_aString, NamePool ePool)
            SAL_THROW( (io::IOException, uno::RuntimeException) )
        {
            m_BinaryReader.read (_aString);
            const int nElems = (sizeof (m_aPreviousName[0])
                                / sizeof (m_aPreviousName[0][0]));
            for (int i = 0; i < nElems; i++)
            {
                if (m_aPreviousName[ePool][i] == _aString)
                    _aString = m_aPreviousName[ePool][i];
            }
            m_aPreviousName[ePool][m_nInsert[ePool]++] = _aString;
            m_nInsert[ePool] %= nElems;
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
            readName(_aName, GroupName);
        }
        // -----------------------------------------------------------------------------
        void BinaryReadHandler::readSet(rtl::OUString &_aName, node::Attributes &_aAttributes,
                        rtl::OUString &_sInstanceName, rtl::OUString &_sInstanceModule)
            SAL_THROW( (io::IOException, uno::RuntimeException) )
        {
            readAttributes(_aAttributes);
            readName(_aName, SetName);
            readName(_sInstanceName, InstanceName);
            readName(_sInstanceModule, InstanceModule);
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
        uno::Type convertValueTypeToType(ValueFlags::Type eBasicValueType, bool bSeq)
        {
           uno::Type aType;

           if (!bSeq)
           {
                switch(eBasicValueType)
                {
                // --------------- Simple types ---------------

                case ValueFlags::val_boolean:
                    aType = SimpleTypeHelper::getBooleanType();
                    break;
                case ValueFlags::val_int16:
                    aType = SimpleTypeHelper::getShortType();
                    break;
                case ValueFlags::val_int32:
                    aType = SimpleTypeHelper::getIntType();
                    break;
                case ValueFlags::val_int64:
                    aType = SimpleTypeHelper::getLongType();
                    break;
                case ValueFlags::val_double:
                    aType = SimpleTypeHelper::getDoubleType();
                    break;
                case ValueFlags::val_string:
                    aType = SimpleTypeHelper::getStringType();
                    break;
                case ValueFlags::val_binary:
                    aType = SimpleTypeHelper::getBinaryType();
                    break;
                case ValueFlags::val_any:
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
                case ValueFlags::val_boolean:
                    aType = ::getCppuType(static_cast<uno::Sequence<sal_Bool> const*>(0));
                    break;
                case ValueFlags::val_int16:
                    aType = ::getCppuType(static_cast<uno::Sequence<sal_Int16> const*>(0));
                    break;
                case ValueFlags::val_int32:
                    aType = ::getCppuType(static_cast<uno::Sequence<sal_Int32> const*>(0));
                    break;
                case ValueFlags::val_int64:
                    aType = ::getCppuType(static_cast<uno::Sequence<sal_Int64> const*>(0));
                    break;
                case ValueFlags::val_double:
                    aType = ::getCppuType(static_cast<uno::Sequence<double> const*>(0));
                    break;
                case ValueFlags::val_string:
                    aType = ::getCppuType(static_cast<uno::Sequence<rtl::OUString> const*>(0));
                    break;
                case ValueFlags::val_binary:
                    aType = ::getCppuType(static_cast<uno::Sequence<BinaryReader::Binary> const*>(0));
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

            ValueFlags::Type eBasicType = readValueFlags(bSeq, bHasValue, bHasDefault);
            readAttributes(_aAttributes);
            readName(_aName, ValueName);

            if (!bSeq && (bHasValue || bHasDefault))
            {
                switch(eBasicType)
                {
                    case ValueFlags::val_any:
                    {
                        OSL_ENSURE(false,"Node of type Any cannot have value");
                                         throw io::WrongFormatException();
                    }
                    case ValueFlags::val_string:
                    {
                        rtl::OUString aStr;
                        if (bHasValue)
                            readAsAny(m_BinaryReader, _aValue, _aType, aStr);
                        if (bHasDefault)
                            readAsAny(m_BinaryReader,  _aDefaultValue, _aType, aStr);
                        break;
                    }
                    case ValueFlags::val_boolean:
                    {
                        sal_Bool nValue;
                        if (bHasValue)
                            readAsAny(m_BinaryReader,_aValue, _aType, nValue);
                        if (bHasDefault)
                            readAsAny(m_BinaryReader,_aDefaultValue, _aType, nValue);
                        break;
                    }
                    case ValueFlags::val_int16:
                    {
                        sal_Int16 nValue;
                        if (bHasValue)
                            readAsAny(m_BinaryReader,_aValue, _aType, nValue);
                        if (bHasDefault)
                            readAsAny(m_BinaryReader,_aDefaultValue, _aType, nValue);
                        break;
                    }
                    case ValueFlags::val_int32:
                    {
                        sal_Int32 nValue;
                        if (bHasValue)
                            readAsAny(m_BinaryReader,_aValue, _aType, nValue);
                        if (bHasDefault)
                            readAsAny(m_BinaryReader,_aDefaultValue, _aType, nValue);
                        break;
                    }
                    case ValueFlags::val_int64:
                    {
                        sal_Int64 nValue;
                        if (bHasValue)
                            readAsAny(m_BinaryReader,_aValue, _aType, nValue);
                        if (bHasDefault)
                            readAsAny(m_BinaryReader,_aDefaultValue, _aType, nValue);
                        break;
                    }
                    case ValueFlags::val_double:
                    {
                        double nValue;
                        if (bHasValue)
                            readAsAny(m_BinaryReader,_aValue, _aType, nValue);
                        if (bHasDefault)
                            readAsAny(m_BinaryReader, _aDefaultValue, _aType, nValue);
                        break;
                    }
                    case ValueFlags::val_binary:
                    {
                        BinaryReader::Binary aValue;
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
            NodeType::Type eType = this->readNodeType();

            if (eType != NodeType::component)
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
            NodeType::Type eType = this->readNodeType();

            if (eType != NodeType::templates)
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

            NodeType::Type eType = this->readNodeType();
            switch (eType)
            {
                case NodeType::groupnode:
                {
                    this->readGroup(aName, aAttributes);
                    pTree  = m_aNodeFactory.createGroup(aName,not_extensible,aAttributes);
                    break;
                }
                case NodeType::setnode:
                {
                    backenduno::TemplateIdentifier aTemplate;
                    this->readSet(aName, aAttributes,aTemplate.Name, aTemplate.Component);
                    pTree  = m_aNodeFactory.createSet(aName,aTemplate,not_extensible,aAttributes);
                    break;
                }
                case NodeType::nodata:
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
                OSL_ENSURE(NodeType::stop == eType, "Missing stop node to mark end of tree");
            }
            return pTree;
        }


    // -----------------------------------------------------------------------------
        bool BinaryReadHandler::validateHeader( const uno::Reference<backenduno::XLayer> * pLayers,
                                                sal_Int32 nNumLayers,
                                                const OUString& _aSchemaVersion,
                                                localehelper::Locale const & aRequestedLocale,
                                                localehelper::LocaleSequence & outKnownLocales)
            SAL_THROW( (io::IOException, uno::RuntimeException) )
        {

            //Check here if the file exists
            if (!m_BinaryReader.open())
                return false;

            if(!this->verifyFileHeader(pLayers, nNumLayers, _aSchemaVersion, aRequestedLocale, outKnownLocales))
                return false;

            //Check if layers are uptodate
            std::vector <OUString> timeStamps(nNumLayers);
            for (sal_Int32 i = 0 ; i < nNumLayers ; ++ i)
            {
                uno::Reference<util::XTimeStamped> xTimeStamp = uno::Reference<util::XTimeStamped>(pLayers[i], uno::UNO_QUERY);
                if (xTimeStamp.is())
                {
                    OUString aTimeStamp = xTimeStamp->getTimestamp();
                    timeStamps[i] = aTimeStamp;
                }
            }

            return this->isUptodate(timeStamps);
        }
        // -----------------------------------------------------------------------------
        void BinaryReadHandler::readChildren(ISubtree & rTree )
        {
            OSL_ASSERT(!NodeType::stop); // loop stops at stop node
            while (NodeType::Type eType = this->readNodeType())
            {
                rtl::OUString aName;
                node::Attributes aAttributes;
                const bool not_extensible = false;
                switch (eType)
                {
                    case NodeType::groupnode:
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
                    case NodeType::setnode:
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
                    case NodeType::valuenode:
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
