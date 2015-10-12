/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_SAX_TOOLS_DOCUMENTHANDLERADAPTER_HXX
#define INCLUDED_SAX_TOOLS_DOCUMENTHANDLERADAPTER_HXX

#include <com/sun/star/xml/sax/SAXException.hpp>
#include <com/sun/star/xml/sax/XDocumentHandler.hpp>
#include <com/sun/star/xml/sax/XExtendedDocumentHandler.hpp>

namespace sax
{
    /**
     * DocumentHandlerAdapter provides a base class for simple decorators to XDocumentHandlers.
     * It forwards all method calls to a delegate. An inheriting class only needs to override the
     * methods it actually wants to modify.
     *
     * See filters/source/odfflatxml/FlatXml.cxx for an example.
     */
    class DocumentHandlerAdapter : public ::com::sun::star::xml::sax::XDocumentHandler
    {
    public:
        // XDocumentHandler
        virtual void SAL_CALL
        startDocument() throw (::com::sun::star::xml::sax::SAXException, ::com::sun::star::uno::RuntimeException, std::exception) override
        {
            m_handler->startDocument();
        }

        virtual void SAL_CALL
        endDocument() throw (::com::sun::star::xml::sax::SAXException, ::com::sun::star::uno::RuntimeException, std::exception) override
        {
            m_handler->endDocument();
        }

        virtual void SAL_CALL
        startElement(const OUString& aName,
                const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XAttributeList > & xAttribs)
                throw (::com::sun::star::xml::sax::SAXException, ::com::sun::star::uno::RuntimeException, std::exception) override
        {
            m_handler->startElement(aName, xAttribs);
        }

        virtual void SAL_CALL
        endElement(const OUString& aName) throw (::com::sun::star::xml::sax::SAXException,
                ::com::sun::star::uno::RuntimeException, std::exception) override
        {
            m_handler->endElement(aName);
        }

        virtual void SAL_CALL
        characters(const OUString& aChars) throw (::com::sun::star::xml::sax::SAXException,
                ::com::sun::star::uno::RuntimeException, std::exception) override
        {
            m_handler->characters(aChars);
        }

        virtual void SAL_CALL
        ignorableWhitespace(const OUString& aWhitespaces) throw (::com::sun::star::xml::sax::SAXException,
                ::com::sun::star::uno::RuntimeException, std::exception) override
        {
            m_handler->ignorableWhitespace(aWhitespaces);
        }
        virtual void SAL_CALL
        processingInstruction(const OUString& aTarget, const OUString& aData)
                throw (::com::sun::star::xml::sax::SAXException, ::com::sun::star::uno::RuntimeException, std::exception) override
        {
            m_handler->processingInstruction(aTarget, aData);
        }
        virtual void SAL_CALL
        setDocumentLocator(const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XLocator > & xLocator)
                throw (::com::sun::star::xml::sax::SAXException, ::com::sun::star::uno::RuntimeException, std::exception) override
        {
            m_handler->setDocumentLocator(xLocator);
        }
        DocumentHandlerAdapter(const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XDocumentHandler >& delegate);
        DocumentHandlerAdapter() :
            m_handler(::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XDocumentHandler > (0, ::com::sun::star::uno::UNO_QUERY))
        {
        }
        ;

    protected:
        virtual void SAL_CALL
        setDelegate(const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XDocumentHandler >& delegate)
        {
            m_handler = delegate;
        }
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XDocumentHandler > SAL_CALL
        getDelegate()
        {
            return m_handler;
        }
        virtual
        ~DocumentHandlerAdapter()
        {

        }

    private:
        ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XDocumentHandler > m_handler;

    };

    /**
     * ExtendedDocumentHandlerAdapter provides a base class for simple decorators to XExtendedDocumentHandlers.
     * It forwards all method calls to a delegate. An inheriting class only needs to override the
     * methods it actually wants to modify.
     */
    class ExtendedDocumentHandlerAdapter : public ::com::sun::star::xml::sax::XExtendedDocumentHandler

    {

    public:
        // XDocumentHandler
        virtual void SAL_CALL
        startDocument() throw (::com::sun::star::xml::sax::SAXException, ::com::sun::star::uno::RuntimeException, std::exception) override
        {
            m_handler->startDocument();
        }

        virtual void SAL_CALL
        endDocument() throw (::com::sun::star::xml::sax::SAXException, ::com::sun::star::uno::RuntimeException, std::exception) override
        {
            m_handler->endDocument();
        }

        virtual void SAL_CALL
        startElement(const OUString& aName,
                const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XAttributeList > & xAttribs)
                throw (::com::sun::star::xml::sax::SAXException, ::com::sun::star::uno::RuntimeException, std::exception) override
        {
            m_handler->startElement(aName, xAttribs);
        }

        virtual void SAL_CALL
        endElement(const OUString& aName) throw (::com::sun::star::xml::sax::SAXException,
                ::com::sun::star::uno::RuntimeException, std::exception) override
        {
            m_handler->endElement(aName);
        }

        virtual void SAL_CALL
        characters(const OUString& aChars) throw (::com::sun::star::xml::sax::SAXException,
                ::com::sun::star::uno::RuntimeException, std::exception) override
        {
            m_handler->characters(aChars);
        }

        virtual void SAL_CALL
        ignorableWhitespace(const OUString& aWhitespaces) throw (::com::sun::star::xml::sax::SAXException,
                ::com::sun::star::uno::RuntimeException, std::exception) override
        {
            m_handler->ignorableWhitespace(aWhitespaces);
        }
        virtual void SAL_CALL
        processingInstruction(const OUString& aTarget, const OUString& aData)
                throw (::com::sun::star::xml::sax::SAXException, ::com::sun::star::uno::RuntimeException, std::exception) override
        {
            m_handler->processingInstruction(aTarget, aData);
        }
        virtual void SAL_CALL
        setDocumentLocator(const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XLocator > & xLocator)
                throw (::com::sun::star::xml::sax::SAXException, ::com::sun::star::uno::RuntimeException, std::exception) override
        {
            m_handler->setDocumentLocator(xLocator);
        }
        // XExtendedDocumentHandler
        virtual void SAL_CALL
        startCDATA() throw (::com::sun::star::xml::sax::SAXException, ::com::sun::star::uno::RuntimeException, std::exception) override
        {
            m_handler->startCDATA();
        }
        virtual void SAL_CALL
        endCDATA() throw (::com::sun::star::uno::RuntimeException, std::exception) override
        {
            m_handler->endCDATA();
        }
        virtual void SAL_CALL
        comment(const OUString& sComment) throw (::com::sun::star::xml::sax::SAXException,
                ::com::sun::star::uno::RuntimeException, std::exception) override
        {
            m_handler->comment(sComment);
        }
        virtual void SAL_CALL
        unknown(const OUString& sString) throw (::com::sun::star::xml::sax::SAXException, ::com::sun::star::uno::RuntimeException, std::exception) override
        {
            m_handler->unknown(sString);
        }
        virtual void SAL_CALL
        allowLineBreak() throw (::com::sun::star::xml::sax::SAXException, ::com::sun::star::uno::RuntimeException, std::exception) override
        {
            m_handler->allowLineBreak();
        }
    protected:
        ExtendedDocumentHandlerAdapter() :
            m_handler(::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XExtendedDocumentHandler > (0, ::com::sun::star::uno::UNO_QUERY))
        {
        }
        ExtendedDocumentHandlerAdapter(
                const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XExtendedDocumentHandler >& delegate) :
            m_handler(delegate)
        {
        }

        virtual void SAL_CALL
        setDelegate(const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XExtendedDocumentHandler >& delegate)
        {
            m_handler = delegate;
        }
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XExtendedDocumentHandler > SAL_CALL
        getDelegate()
        {
            return m_handler;
        }
        virtual
        ~ExtendedDocumentHandlerAdapter()
        {

        }

    private:
        ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XExtendedDocumentHandler > m_handler;
    };
}
#endif // INCLUDED_SAX_TOOLS_DOCUMENTHANDLERADAPTER_HXX
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
