/*************************************************************************
 *
 *  $RCSfile: binaryreadhandler.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2003-10-06 14:45:16 $
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
        }
        // -----------------------------------------------------------------------------
        BinaryReadHandler::~BinaryReadHandler()
        {
        }
        // -----------------------------------------------------------------------------

        bool BinaryReadHandler::verifyFileHeader(rtl::OUString const & _aOwnerEntity,
                                                 rtl::OUString const & _aLocale)
        {
            try
            {
                //Open the reader
                sal_Int16 nMagic, nVersion;
                rtl::OUString aOwnerEntity;
                rtl::OUString aLocale;

                m_BinaryReader.read(aOwnerEntity);
                if (!aOwnerEntity.equals(_aOwnerEntity))
                    return false;

                m_BinaryReader.read(aLocale);
                if (!aLocale.equals(_aLocale))
                    return false;

                m_BinaryReader.read(nMagic);
                m_BinaryReader.read(nVersion);
                if (nMagic !=CFG_BINARY_MAGIC || nVersion != CFG_BINARY_VERSION )
                    return false;

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
            readString(_aName);
        }
        // -----------------------------------------------------------------------------
        void BinaryReadHandler::readSet(rtl::OUString &_aName, node::Attributes &_aAttributes,
                        rtl::OUString &_sInstanceName, rtl::OUString &_sInstanceModule)
            SAL_THROW( (io::IOException, uno::RuntimeException) )
        {
            readAttributes(_aAttributes);
            readString(_aName);
            readString(_sInstanceName);
            readString(_sInstanceModule);
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
            readString(_aName);

            if (!bSeq && (bHasValue || bHasDefault))
            {
                switch(eBasicType)
                {
                    case ValueFlags::val_any:
                    {
                        OSL_ENSURE(false,"Node of type Any cannot have value");
                        throw io::WrongFormatException();
                        break;
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
                                                const OUString& _aOwnerEntity,
                                                const OUString& _aLocale)
            SAL_THROW( (io::IOException, uno::RuntimeException) )
        {

            //Check here if the file exists
            if (!m_BinaryReader.open())
                return false;

            if(!this->verifyFileHeader(_aOwnerEntity, _aLocale))
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
