/*************************************************************************
 *
 *  $RCSfile: binarywritehandler.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2003-10-06 14:46:00 $
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
#include "binarywritehandler.hxx"
#include "binarycache.hxx"

#ifndef _COM_SUN_STAR_UTIL_XTIMESTAMPED_HPP_
#include <com/sun/star/util/XTimeStamped.hpp>
#endif

#ifndef CONFIGMGR_TYPECONVERTER_HXX
#include "typeconverter.hxx"
#endif
#ifndef CONFIGMGR_SIMPLETYPEHELPER_HXX
#include "simpletypehelper.hxx"
#endif

#define ASCII(x) rtl::OUString::createFromAscii(x)
namespace configmgr
{
    // -----------------------------------------------------------------------------
    namespace backend
    {

        using ::rtl::OUString;
        namespace css = com::sun::star;
        namespace util = css::util ;

        using namespace binary;
        // -----------------------------------------------------------------------------

        BinaryWriteHandler::BinaryWriteHandler( rtl::OUString const & _aFileURL,
                                                rtl::OUString const & _aComponentName,
                                                MultiServiceFactory const & _aFactory)
        : m_BinaryWriter(_aFileURL,_aFactory)
        , m_aComponentName(_aComponentName)
        {

        }
        // -----------------------------------------------------------------------------
        static
        ValueFlags::Type convertTypeToValueType(uno::Type const& _aType)
        {
            ValueFlags::Type eType = ValueFlags::val_invalid;
            uno::TypeClass const aClass = _aType.getTypeClass();
            switch(aClass)
            {
            case uno::TypeClass_ANY:
                eType = ValueFlags::val_any;
                break;
            case uno::TypeClass_BOOLEAN:
                eType = ValueFlags::val_boolean;
                break;
            case uno::TypeClass_SHORT:
                eType = ValueFlags::val_int16;
                break;
            case uno::TypeClass_LONG:
                eType = ValueFlags::val_int32;
                break;
            case uno::TypeClass_HYPER:
                eType = ValueFlags::val_int64;
                break;
            case uno::TypeClass_DOUBLE:
                eType = ValueFlags::val_double;
                break;
            case uno::TypeClass_STRING:
                eType = ValueFlags::val_string;
                break;
            case uno::TypeClass_SEQUENCE:
                if (_aType == SimpleTypeHelper::getBinaryType())
                {
                    eType = ValueFlags::val_binary;
                }
                else
                {
                    uno::Type aType = configmgr::getSequenceElementType(_aType);
                    eType = convertTypeToValueType(aType);

                    OSL_ENSURE(!(eType & ValueFlags::seq), "Binary Writer - Invalid value type: Multiple nesting of sequences");
                    eType = ValueFlags::Type( eType | ValueFlags::seq );
                }
                break;
            default:
                OSL_ENSURE(false, "Binary Writer - Invalid value type: not supported");
                break;
            }
            return eType;
        }

        // -----------------------------------------------------------------------------
        template <class T>
        inline /* make performance crew happy ;-) */
        void writeDirectly(BinaryWriter& _rWriter, T const& _aVal)
            SAL_THROW( (io::IOException, uno::RuntimeException) )
        {
            _rWriter.write(_aVal);
        }
        // -----------------------------------------------------------------------------
        static
        inline
        void writeDirectly(BinaryWriter& _rWriter, uno::Sequence<sal_Int8> const& _aBinary);
        // -----------------------------------------------------------------------------
        template <class T>
        inline /* make performance crew happy ;-) */
        void writeFromAny(BinaryWriter& _rWriter,uno::Any const& _aValue, T& _aVar)
            SAL_THROW( (io::IOException, uno::RuntimeException) )
        {
            OSL_VERIFY(_aValue >>= _aVar ); // "Invalid Any for value"
            writeDirectly(_rWriter,_aVar);
        }
        // -----------------------------------------------------------------------------

        template <class Element>
        void writeSequence(BinaryWriter& _rWriter, uno::Sequence< Element > const& aSequence)
            SAL_THROW( (io::IOException, uno::RuntimeException) )
        {
            sal_Int32 const nLength = aSequence.getLength();
            _rWriter.write(nLength);

            for(sal_Int32 i=0; i<nLength; ++i)
            {
                writeDirectly(_rWriter, aSequence[i]);
            }
        }
        // -----------------------------------------------------------------------------
        static
        inline
        void writeDirectly(BinaryWriter& _rWriter, uno::Sequence<sal_Int8> const& _aBinary)
        {
            writeSequence(_rWriter,_aBinary);
        }
        // -----------------------------------------------------------------------------
        #define CASE_WRITE_SEQUENCE(TYPE_CLASS, DATA_TYPE)  \
        case TYPE_CLASS:                                \
        {                                               \
            uno::Sequence< DATA_TYPE > aData;               \
            OSL_ASSERT( ::getCppuType(aData.getConstArray()).getTypeClass() == (TYPE_CLASS) );  \
            OSL_VERIFY( _aValue >>= aData );                                        \
            writeSequence(_rWriter,aData);                      \
        }   break

        // -----------------------------------------------------------------------------
        static
        void  writeSequenceValue(BinaryWriter& _rWriter,  uno::Any const& _aValue, uno::Type const& aElementType)
        SAL_THROW( (io::IOException, uno::RuntimeException) )
        {
            switch(aElementType.getTypeClass())
            {
                CASE_WRITE_SEQUENCE( uno::TypeClass_BOOLEAN, sal_Bool );

                CASE_WRITE_SEQUENCE( uno::TypeClass_SHORT, sal_Int16 );

                CASE_WRITE_SEQUENCE( uno::TypeClass_LONG, sal_Int32 );

                CASE_WRITE_SEQUENCE( uno::TypeClass_HYPER, sal_Int64 );

                CASE_WRITE_SEQUENCE( uno::TypeClass_DOUBLE, double );

                CASE_WRITE_SEQUENCE( uno::TypeClass_STRING, OUString );

                CASE_WRITE_SEQUENCE( uno::TypeClass_SEQUENCE, uno::Sequence<sal_Int8> );

                default:
                    OSL_ENSURE(false, "Unexpected typeclass for sequence elements");
                    break;
            }
        }
        #undef CASE_WRITE_SEQUENCE

        // -----------------------------------------------------------------------------
        static
        void  writeSimpleValue(BinaryWriter& _rWriter, uno::Any const& _aValue, uno::Type const& _aType)
            SAL_THROW( (io::IOException, uno::RuntimeException) )
        {
            // PRE: Header must be written
            uno::TypeClass aDestinationClass = _aType.getTypeClass();
            switch (aDestinationClass)
            {
                case uno::TypeClass_BOOLEAN:
                {
                    sal_Bool bValue;
                    writeFromAny(_rWriter, _aValue, bValue);
                    break;
                }
                case uno::TypeClass_BYTE:
                {
                    sal_Int8 nValue;
                    writeFromAny(_rWriter, _aValue, nValue);
                    break;
                }
                case uno::TypeClass_SHORT:
                {
                    sal_Int16 nValue;
                    writeFromAny(_rWriter, _aValue, nValue);
                    break;
                }
                case uno::TypeClass_LONG:
                {
                    sal_Int32 nValue;
                    writeFromAny(_rWriter, _aValue, nValue);
                    break;
                }
                case uno::TypeClass_HYPER:
                {
                    sal_Int64 nValue;
                    writeFromAny(_rWriter, _aValue, nValue);
                    break;
                }
                case uno::TypeClass_DOUBLE:
                {
                    double nValue;
                    writeFromAny(_rWriter, _aValue, nValue);
                    break;
                }
                case uno::TypeClass_STRING:
                {
                    OUString aStr;
                    writeFromAny(_rWriter, _aValue, aStr);
                    break;
                }
                case uno::TypeClass_SEQUENCE:
                {
                    OSL_ENSURE (_aType == SimpleTypeHelper::getBinaryType(),"Unexpected sequence as simple type");
                    uno::Sequence<sal_Int8> aBinary;
                    writeFromAny(_rWriter, _aValue, aBinary);
                    break;
                }
                default:
                    OSL_ENSURE(false, "Unexpected typeclass for simple value");
                    break;
            }
        }

        // -----------------------------------------------------------------------------

        void BinaryWriteHandler::writeValue(uno::Any const& _aValue)
        {
            bool bSeq;
            uno::Type aTargetType = getBasicType(_aValue.getValueType(), bSeq);

            if (!bSeq)
            {
                writeSimpleValue(m_BinaryWriter,_aValue, aTargetType);
            }
            else
            {
                writeSequenceValue(m_BinaryWriter,_aValue, aTargetType);
            }
        }
        // -----------------------------------------------------------------------------

        void BinaryWriteHandler::writeFileHeader(rtl::OUString const & _aOwnerEntity,
                                                 rtl::OUString const & _aLocale)
            SAL_THROW( (io::IOException, uno::RuntimeException) )
        {
            m_BinaryWriter.write(_aOwnerEntity);
            m_BinaryWriter.write(_aLocale);
            m_BinaryWriter.write(CFG_BINARY_MAGIC);
            m_BinaryWriter.write(CFG_BINARY_VERSION);
            m_BinaryWriter.write(m_aComponentName);
        }
        // -----------------------------------------------------------------------------

        void BinaryWriteHandler::writeGroupNode(rtl::OUString const& _aName,node::Attributes const& _aAttributes )
            SAL_THROW( (io::IOException, uno::RuntimeException) )
        {
            writeAttributes(_aAttributes );
            m_BinaryWriter.write(_aName);
        }

        // -----------------------------------------------------------------------------

        void BinaryWriteHandler::writeSetNode(rtl::OUString const& _aName,
                                              rtl::OUString const& _aTemplateName,
                                              rtl::OUString const& _aTemplateModule,
                                              node::Attributes const& _aAttributes)
            SAL_THROW( (io::IOException, uno::RuntimeException) )
        {
            writeAttributes(_aAttributes );
            m_BinaryWriter.write(_aName);
            m_BinaryWriter.write(_aTemplateName);
            m_BinaryWriter.write(_aTemplateModule);
        }
        // -----------------------------------------------------------------------------

        void BinaryWriteHandler::writeLayerInfoList(uno::Reference<backenduno::XLayer> const * pLayers, sal_Int32 nNumLayers)
            SAL_THROW( (io::IOException, uno::RuntimeException) )
        {

            m_BinaryWriter.write(nNumLayers);
            for (sal_Int32 i = 0 ; i < nNumLayers ; ++ i)
            {
                uno::Reference<util::XTimeStamped> xTimeStamp = uno::Reference<util::XTimeStamped>(pLayers[i], uno::UNO_QUERY);
                OSL_ENSURE(xTimeStamp.is(), "BackendAccess::writeMergedTree: No xTimeStamped Interface for layer");

                OUString aTimeStamp = xTimeStamp.is() ? xTimeStamp->getTimestamp() : OUString();
                m_BinaryWriter.write(aTimeStamp);
            }

        }
        // -----------------------------------------------------------------------------

        void BinaryWriteHandler::writeNodeType(NodeType::Type _eType)
            SAL_THROW( (io::IOException, uno::RuntimeException) )
        {
            sal_Int8 nValue = _eType;
            m_BinaryWriter.write(nValue);

        }
        // -----------------------------------------------------------------------------

        void BinaryWriteHandler::writeAttributes(node::Attributes const& _aAttributes)
            SAL_THROW( (io::IOException, uno::RuntimeException) )
        {
            sal_Int8 nValue = _aAttributes.state();

            OSL_ASSERT(0 <= nValue && nValue <= 3);

            OSL_ENSURE(!_aAttributes.isReadonly() || !_aAttributes.isFinalized(),"Unexpected attribute mix: node is both read-only and finalized");

            nValue |= (_aAttributes.isReadonly()     ? 1 : 0) << 2;
            nValue |= (_aAttributes.isFinalized()    ? 1 : 0) << 3;

            nValue |= (_aAttributes.isNullable()      ? 1 : 0) << 4;
            nValue |= (_aAttributes.isLocalized()     ? 1 : 0) << 5;

            nValue |= (_aAttributes.isMandatory()     ? 1 : 0) << 6;
            nValue |= (_aAttributes.isRemovable()     ? 1 : 0) << 7;

            m_BinaryWriter.write(nValue);

        }
        // -----------------------------------------------------------------------------

        void BinaryWriteHandler::writeType(uno::Type const& _aType)
            SAL_THROW( (io::IOException, uno::RuntimeException) )
        {
            ValueFlags::Type eType = convertTypeToValueType(_aType);
            sal_Int8 nValueType = sal_Int8(eType);
            m_BinaryWriter.write(nValueType);
        }
        // -----------------------------------------------------------------------------

        void BinaryWriteHandler::writeValueNode(rtl::OUString const& _aName,
                                                node::Attributes const& _aAttributes,
                                                uno::Type const& _aType,
                                                uno::Any const& _aUserValue,
                                                uno::Any const& _aDefaultValue)
        SAL_THROW( (io::IOException, uno::RuntimeException) )
        {
            //write value flags
            ValueFlags::Type eType = convertTypeToValueType(_aType);
            sal_Int8 nValueType = sal_Int8(eType);

            bool hasUserValue   = _aUserValue.hasValue();
            bool hasDefault     = _aDefaultValue.hasValue();

            if (!hasUserValue)  nValueType |=  ValueFlags::first_value_NULL;
            if (!hasDefault)    nValueType |=  ValueFlags::second_value_NULL;

            m_BinaryWriter.write(nValueType);
            writeAttributes(_aAttributes );
            m_BinaryWriter.write(_aName);

            if(hasUserValue)
            {
                OSL_ENSURE(_aUserValue.getValueType() == _aType, "Type mismatch in value node");
                writeValue(_aUserValue);
            }
            if(hasDefault)
            {
                OSL_ENSURE(_aDefaultValue.getValueType() == _aType, "Type mismatch in value node");
                writeValue(_aDefaultValue);
            }
        }
        // -----------------------------------------------------------------------------

        void BinaryWriteHandler::writeStop()
            SAL_THROW( (io::IOException, uno::RuntimeException) )
        {
            sal_Int8 nStopValue = 0;
            m_BinaryWriter.write(nStopValue);
        }
         // -----------------------------------------------------------------------------

        void BinaryWriteHandler::writeComponentTree(const ISubtree * _pComponentTree)
            SAL_THROW( (io::IOException, uno::RuntimeException) )
        {
            this->writeNodeType(NodeType::component);
            if (_pComponentTree)
            {
                this->writeTree(*_pComponentTree);
                this->writeStop();
            }
            else
            {
                this->writeNodeType(NodeType::nodata);
            }
        }
         // -----------------------------------------------------------------------------

        void BinaryWriteHandler::writeTemplatesTree(const ISubtree * _pTemplatesTree)
            SAL_THROW( (io::IOException, uno::RuntimeException) )
        {
            this->writeNodeType(NodeType::templates);
            if (_pTemplatesTree)
            {
                this->writeTree(*_pTemplatesTree);
                this->writeStop();
            }
            else
            {
                this->writeNodeType(NodeType::nodata);
            }
        }
         // -----------------------------------------------------------------------------

        void BinaryWriteHandler::writeTree(const ISubtree & rTree)
            SAL_THROW( (io::IOException, uno::RuntimeException) )
        {
            if ( rTree.isSetNode() )
            {
                this->writeNodeType(NodeType::setnode);
                this->writeSetNode( rTree.getName(),
                                    rTree.getElementTemplateName(),
                                    rTree.getElementTemplateModule(),
                                    rTree.getAttributes());
            }
            else
            {
                this->writeNodeType(NodeType::groupnode);
                this->writeGroupNode( rTree.getName(), rTree.getAttributes() );
            }

            // process children
            this->applyToChildren(rTree);

            this->writeStop();
        }
        // -----------------------------------------------------------------------------

        void BinaryWriteHandler::handle(const ISubtree & rTree)
        {
            writeTree(rTree);
        }
        // -----------------------------------------------------------------------------

        void BinaryWriteHandler::handle(const ValueNode & rValue)
        {
            this->writeNodeType(NodeType::valuenode);

            this->writeValueNode(   rValue.getName(),
                                    rValue.getAttributes(),
                                    rValue.getValueType(),
                                    rValue.getUserValue(),
                                    rValue.getDefault() );
        }
        // -----------------------------------------------------------------------------

        bool BinaryWriteHandler::generateHeader(const uno::Reference<backenduno::XLayer> * pLayers,
                                                 sal_Int32 nNumLayers,
                                                const OUString& aEntity,
                                                const OUString& aLocale )
            SAL_THROW( (io::IOException, uno::RuntimeException) )
        {
            //Open the writer
            if (!m_BinaryWriter.open())
                return false;

            this->writeFileHeader(aEntity, aLocale);
            this->writeLayerInfoList(pLayers, nNumLayers);
            return true;
        }
        // -----------------------------------------------------------------------------

    }
   // -----------------------------------------------------------------------------
}
