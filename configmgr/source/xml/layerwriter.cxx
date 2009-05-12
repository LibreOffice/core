/*************************************************************************
*
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: layerwriter.cxx,v $
 * $Revision: 1.15 $
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

#include "layerwriter.hxx"

#ifndef CONFIGMGR_API_FACTORY_HXX_
#include "confapifactory.hxx"
#endif
#include "valueformatter.hxx"
#include <com/sun/star/beans/IllegalTypeException.hpp>
#include <com/sun/star/configuration/backend/BackendAccessException.hpp>
#include <com/sun/star/configuration/backend/ConnectionLostException.hpp>
#include <com/sun/star/configuration/backend/NodeAttribute.hpp>
// -----------------------------------------------------------------------------

namespace configmgr
{
    // -----------------------------------------------------------------------------
    namespace xml
    {
        // -----------------------------------------------------------------------------
        namespace uno       = ::com::sun::star::uno;
        namespace io        = ::com::sun::star::io;
        namespace sax       = ::com::sun::star::xml::sax;
        // -----------------------------------------------------------------------------

        uno::Reference< uno::XInterface > SAL_CALL instantiateLayerWriter
            ( uno::Reference< uno::XComponentContext > const& xContext )
        {
            return * new LayerWriter(xContext);
        }
        // -----------------------------------------------------------------------------

        namespace
        {
            static inline
                uno::Reference< script::XTypeConverter > createTCV(uno::Reference< lang::XMultiServiceFactory > const & _xSvcFactory)
            {
                OSL_ENSURE(_xSvcFactory.is(),"Cannot create Write Formatter without a ServiceManager");

                static const rtl::OUString k_sTCVService(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.script.Converter"));

                return uno::Reference< script::XTypeConverter >::query(_xSvcFactory->createInstance(k_sTCVService));
            }
        // -----------------------------------------------------------------------------
            static
            void translateSAXException( sax::SAXException & aSAXException,
                                        backenduno::XLayerHandler * context)
            {
                rtl::OUString sMessage = aSAXException.Message;

                uno::Any const & aWrappedException = aSAXException.WrappedException;
                if (aWrappedException.hasValue())
                {
                    if (aWrappedException.getValueTypeClass() != uno::TypeClass_EXCEPTION)
                        OSL_ENSURE(false, "ERROR: WrappedException is not an exception");

                    else if (sMessage.getLength() == 0)
                        sMessage = static_cast<uno::Exception const *>(aWrappedException.getValue())->Message;

                    // assume that a SAXException with WrappedException indicates a storage access error
                    throw backenduno::BackendAccessException(sMessage,context,aWrappedException);
                }
                else
                {
                    // assume that a SAXException without WrappedException indicates non-well-formed data
                    throw backenduno::MalformedDataException(sMessage,context,uno::makeAny(aSAXException));
                }
            }
        // -----------------------------------------------------------------------------
            static inline uno::Type getIOExceptionType()
            {
                io::IOException const * const ioexception = 0;
                return ::getCppuType(ioexception);
            }
            static inline uno::Type getNotConnectedExceptionType()
            {
                io::NotConnectedException const * const ncexception = 0;
                return ::getCppuType(ncexception);
            }
            static
            void translateIOException(uno::Any const & anIOException,
                                      backenduno::XLayerHandler * context)
            {
                OSL_ASSERT(anIOException.isExtractableTo(getIOExceptionType()));
                io::IOException const * pException = static_cast<io::IOException const *>(anIOException.getValue());
                rtl::OUString sMessage = pException->Message;

                if (anIOException.isExtractableTo(getNotConnectedExceptionType()))
                {
                    throw backenduno::ConnectionLostException(sMessage,context,anIOException);
                }
                else
                {
                    throw backenduno::BackendAccessException(sMessage,context,anIOException);
                }
            }
        // -----------------------------------------------------------------------------
        }
        // -----------------------------------------------------------------------------

        LayerWriter::LayerWriter(uno::Reference< uno::XComponentContext > const & _xContext)
            : WriterService< ::com::sun::star::configuration::backend::XLayerHandler >(_xContext)
            , m_xTCV( createTCV( WriterService< ::com::sun::star::configuration::backend::XLayerHandler >::getServiceFactory() ) )
            , m_bInProperty(false)
            , m_bStartedDocument(false)
        {
        }
        // -----------------------------------------------------------------------------

        LayerWriter::~LayerWriter()
        {
        }
        // -----------------------------------------------------------------------------

        void SAL_CALL LayerWriter::startLayer(  )
            throw (backenduno::MalformedDataException, lang::WrappedTargetException,
                   uno::RuntimeException)
        {
            m_aFormatter.reset();
            m_bInProperty = false;
            m_bStartedDocument = false;

            checkInElement(false);
        }
        // -----------------------------------------------------------------------------

        void SAL_CALL LayerWriter::endLayer(  )
            throw (backenduno::MalformedDataException, lang::WrappedTargetException,
                   uno::RuntimeException)
        {
            checkInElement(false);
            try
            {
                if (!m_bStartedDocument)
                {
                    uno::Reference< io::XOutputStream > aStream = this->getOutputStream(  );
                    aStream->closeOutput();
                }
                else
                {
                    getWriteHandler()->endDocument();
                    m_aFormatter.reset();
                    m_bStartedDocument = false;
                }
            }
            catch (sax::SAXException & xe)                  { translateSAXException(xe,this); }
            catch (io::NotConnectedException & ioe)         { translateIOException(uno::makeAny(ioe),this); }
            catch (io::BufferSizeExceededException & ioe)   { translateIOException(uno::makeAny(ioe),this); }
            catch (io::IOException & ioe)                   { translateIOException(uno::makeAny(ioe),this); }
        }
        // -----------------------------------------------------------------------------

        void SAL_CALL LayerWriter::overrideNode( const rtl::OUString& aName, sal_Int16 aAttributes, sal_Bool bClear )
            throw (backenduno::MalformedDataException, lang::WrappedTargetException,
                   uno::RuntimeException)
        {
            try
            {
                if (!m_bStartedDocument)
                {
                    getWriteHandler()->startDocument();
                    m_bStartedDocument = true;
                }
                ElementInfo aInfo(aName, this->isInElement() ? ElementType::node : ElementType::layer);
                aInfo.flags = aAttributes;
                aInfo.op = bClear ? Operation::clear : Operation::modify;

                m_aFormatter.prepareElement(aInfo);

                this->startNode();
            }
            catch (sax::SAXException & xe)
            {
                translateSAXException(xe,this);
            }
        }
        // -----------------------------------------------------------------------------

        void LayerWriter::prepareAddOrReplaceElement(
            rtl::OUString const & name, sal_Int16 attributes)
        {
            ElementInfo info(name, ElementType::node);
            info.flags = attributes &
                ~com::sun::star::configuration::backend::NodeAttribute::FUSE;
            info.op =
                (attributes &
                 com::sun::star::configuration::backend::NodeAttribute::FUSE)
                == 0
                ? Operation::replace : Operation::fuse;
            m_aFormatter.prepareElement(info);
        }

        void SAL_CALL LayerWriter::addOrReplaceNode( const rtl::OUString& aName, sal_Int16 aAttributes )
            throw (backenduno::MalformedDataException, lang::WrappedTargetException,
                   uno::RuntimeException)
        {
            checkInElement(true);

            try
            {
                prepareAddOrReplaceElement(aName, aAttributes);

                this->startNode();
            }
            catch (sax::SAXException & xe)
            {
                translateSAXException(xe,this);
            }
        }
        // -----------------------------------------------------------------------------

        void SAL_CALL LayerWriter::addOrReplaceNodeFromTemplate( const rtl::OUString& aName, const backenduno::TemplateIdentifier& aTemplate, sal_Int16 aAttributes )
            throw (backenduno::MalformedDataException, lang::WrappedTargetException,
                   uno::RuntimeException)
        {
            checkInElement(true);

            try
            {
                prepareAddOrReplaceElement(aName, aAttributes);
                m_aFormatter.addInstanceType(aTemplate.Name,aTemplate.Component);

                this->startNode();
            }
            catch (sax::SAXException & xe)
            {
                translateSAXException(xe,this);
            }
        }
        // -----------------------------------------------------------------------------

        void SAL_CALL LayerWriter::endNode(  )
            throw (backenduno::MalformedDataException, lang::WrappedTargetException,
                   uno::RuntimeException)
        {
            checkInElement(true);
            try
            {
                this->endElement();
            }
            catch (sax::SAXException & xe)
            {
                translateSAXException(xe,this);
            }
        }
        // -----------------------------------------------------------------------------

        void SAL_CALL LayerWriter::dropNode( const rtl::OUString& aName )
            throw (backenduno::MalformedDataException, lang::WrappedTargetException,
                   uno::RuntimeException)
        {
            checkInElement(true);

            try
            {
                ElementInfo aInfo(aName, ElementType::node);
                aInfo.flags = 0;
                aInfo.op = Operation::remove;

                m_aFormatter.prepareElement(aInfo);

                this->startNode();
                this->endElement();
            }
            catch (sax::SAXException & xe)
            {
                translateSAXException(xe,this);
            }
       }
        // -----------------------------------------------------------------------------

        void SAL_CALL LayerWriter::addProperty( const rtl::OUString& aName, sal_Int16 aAttributes, const uno::Type& aType )
            throw (backenduno::MalformedDataException, lang::WrappedTargetException,
                   uno::RuntimeException)
        {
            checkInElement(true);

            try
            {
                ElementInfo aInfo(aName, ElementType::property);
                aInfo.flags = aAttributes;
                aInfo.op = Operation::replace;

                m_aFormatter.prepareElement(aInfo);

                this->startProp(aType, true);
                this->writeValue(uno::Any());
                this->endElement();
            }
            catch (sax::SAXException & xe)
            {
                translateSAXException(xe,this);
            }
        }
        // -----------------------------------------------------------------------------

        void SAL_CALL LayerWriter::addPropertyWithValue( const rtl::OUString& aName, sal_Int16 aAttributes, const uno::Any& aValue )
            throw (backenduno::MalformedDataException, lang::WrappedTargetException,
                   uno::RuntimeException)
        {
            checkInElement(true);

            try
            {
                ElementInfo aInfo(aName, ElementType::property);
                aInfo.flags = aAttributes;
                aInfo.op = Operation::replace;

                m_aFormatter.prepareElement(aInfo);

                this->startProp(aValue.getValueType(), true);
                this->writeValue(aValue);
                this->endElement();
            }
            catch (sax::SAXException & xe)
            {
                translateSAXException(xe,this);
            }
        }
        // -----------------------------------------------------------------------------

        void SAL_CALL LayerWriter::overrideProperty( const rtl::OUString& aName, sal_Int16 aAttributes, const uno::Type& aType, sal_Bool bClear )
            throw (backenduno::MalformedDataException, lang::WrappedTargetException,
                   uno::RuntimeException)
        {
            checkInElement(true);

            try
            {
                ElementInfo aInfo(aName, ElementType::property);
                aInfo.flags = aAttributes;
                aInfo.op = bClear ? Operation::modify : Operation::modify;

                m_aFormatter.prepareElement(aInfo);

                this->startProp(aType, aType.getTypeClass() != uno::TypeClass_VOID);
            }
            catch (sax::SAXException & xe)
            {
                translateSAXException(xe,this);
            }
        }
        // -----------------------------------------------------------------------------

        void SAL_CALL LayerWriter::endProperty(  )
            throw (backenduno::MalformedDataException, lang::WrappedTargetException,
                   uno::RuntimeException)
        {
            try
            {
                checkInElement(true,true);
                this->endElement();
            }
            catch (sax::SAXException & xe)
            {
                translateSAXException(xe,this);
            }
        }
        // -----------------------------------------------------------------------------

        void SAL_CALL LayerWriter::setPropertyValue( const uno::Any& aValue )
            throw (backenduno::MalformedDataException, lang::WrappedTargetException,
                   uno::RuntimeException)
        {
            try
            {
                checkInElement(true,true);
                this->writeValue(aValue);
            }
            catch (sax::SAXException & xe)
            {
                translateSAXException(xe,this);
            }
        }
        // -----------------------------------------------------------------------------

        void SAL_CALL LayerWriter::setPropertyValueForLocale( const uno::Any& aValue, const rtl::OUString& aLocale )
            throw (backenduno::MalformedDataException, lang::WrappedTargetException,
                   uno::RuntimeException)
        {
            try
            {
                checkInElement(true,true);
                this->writeValue(aValue,aLocale);
            }
            catch (sax::SAXException & xe)
            {
                translateSAXException(xe,this);
            }
        }
        // -----------------------------------------------------------------------------

        void LayerWriter::raiseMalformedDataException(sal_Char const * pMsg)
        {
            OSL_ASSERT(pMsg);
            rtl::OUString sMsg = rtl::OUString::createFromAscii(pMsg);

            throw backenduno::MalformedDataException( sMsg, *this, uno::Any() );
        }
        // -----------------------------------------------------------------------------

        void LayerWriter::raiseIllegalTypeException(sal_Char const * pMsg)
        {
            OSL_ASSERT(pMsg);
            rtl::OUString sMsg = rtl::OUString::createFromAscii(pMsg);

            com::sun::star::beans::IllegalTypeException e( sMsg, *this );
            throw backenduno::MalformedDataException( sMsg, *this, uno::makeAny(e) );
        }
        // -----------------------------------------------------------------------------

        bool LayerWriter::isInElement() const
        {
            return !m_aTagStack.empty();
        }
        // -----------------------------------------------------------------------------

        void LayerWriter::checkInElement(bool bInElement, bool bInProperty)
        {
            if (bInElement != this->isInElement())
            {
                sal_Char const * pMsg = bInElement ?
                    "LayerWriter: Illegal Data: Operation requires a started node" :
                "LayerWriter: Illegal Data: There is a started node already" ;
                raiseMalformedDataException(pMsg);
            }

            if (bInProperty != m_bInProperty)
            {
                sal_Char const * pMsg = bInProperty ?
                    "LayerWriter: Illegal Data: Operation requires a started property" :
                "LayerWriter: Illegal Data: There is a started property already" ;
                raiseMalformedDataException(pMsg);
            }
        }
        // -----------------------------------------------------------------------------

        void LayerWriter::startNode()
        {
            rtl::OUString sTag = m_aFormatter.getElementTag();
            getWriteHandler()->startElement(sTag,m_aFormatter.getElementAttributes());
            getWriteHandler()->ignorableWhitespace(rtl::OUString());
            m_aTagStack.push(sTag);
        }
        // -----------------------------------------------------------------------------

        void LayerWriter::startProp(uno::Type const & _aType, bool bNeedType)
        {
            if (bNeedType && _aType == uno::Type())
                raiseIllegalTypeException("LayerWriter: Illegal Data: Cannot add VOID property");

            m_aFormatter.addPropertyValueType(_aType);

            startNode();

            m_aPropertyType = _aType;
            m_bInProperty = true;
        }
        // -----------------------------------------------------------------------------

        void LayerWriter::endElement()
        {
            OSL_ASSERT(!m_aTagStack.empty()); // checks done elsewhere

            getWriteHandler()->endElement(m_aTagStack.top());
            getWriteHandler()->ignorableWhitespace(rtl::OUString());

            m_aTagStack.pop();
            m_bInProperty = false;
        }
        // -----------------------------------------------------------------------------

        void LayerWriter::writeValue(uno::Any const & _aValue)
        {
            m_aFormatter.prepareSimpleElement( ElementType::value );
            outputValue(_aValue);
        }
        // -----------------------------------------------------------------------------

        void LayerWriter::writeValue(uno::Any const & _aValue, rtl::OUString const & _aLocale)
        {
            m_aFormatter.prepareSimpleElement( ElementType::value );
            m_aFormatter.addLanguage(_aLocale);
            outputValue(_aValue);
        }
        // -----------------------------------------------------------------------------

        void LayerWriter::outputValue(uno::Any const & _aValue)
        {
            ValueFormatter aValueFormatter(_aValue);

            aValueFormatter.addValueAttributes(m_aFormatter);

            rtl::OUString sTag       = m_aFormatter.getElementTag();
            rtl::OUString sContent   = aValueFormatter.getContent( this->m_xTCV );

            uno::Reference< sax::XDocumentHandler > xOut = getWriteHandler();
            xOut->startElement(sTag, m_aFormatter.getElementAttributes());

            if (sContent.getLength()) xOut->characters(sContent);

            xOut->endElement(sTag);
            xOut->ignorableWhitespace(rtl::OUString());
        }
        // -----------------------------------------------------------------------------

        // -----------------------------------------------------------------------------
        // -----------------------------------------------------------------------------
    } // namespace

    // -----------------------------------------------------------------------------
} // namespace

