/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 * Version: MPL 1.1 / GPLv3+ / LGPLv3+
 *
 * The contents of this file are subject to the Mozilla Public License Version
 * 1.1 (the "License"); you may not use this file except in compliance with
 * the License. You may obtain a copy of the License at
 * http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * The Initial Developer of the Original Code is
 *       [ Peter Jentsch <pjotr@guineapics.de> ]
 *
 * Portions created by the Initial Developer are Copyright (C) 2010 the
 * Initial Developer. All Rights Reserved.
 *
 * Contributor(s): Peter Jentsch <pjotr@guineapics.de>
 *
 * Alternatively, the contents of this file may be used under the terms of
 * either the GNU General Public License Version 3 or later (the "GPLv3+"), or
 * the GNU Lesser General Public License Version 3 or later (the "LGPLv3+"),
 * in which case the provisions of the GPLv3+ or the LGPLv3+ are applicable
 * instead of those above.
 */

#ifndef _DOCUMENTHANDLERADAPTER_H_
#define _DOCUMENTHANDLERADAPTER_H_

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
        startDocument(void) throw (::com::sun::star::xml::sax::SAXException, ::com::sun::star::uno::RuntimeException)
        {
            m_handler->startDocument();
        }

        virtual void SAL_CALL
        endDocument(void) throw (::com::sun::star::xml::sax::SAXException, ::com::sun::star::uno::RuntimeException)
        {
            m_handler->endDocument();
        }

        virtual void SAL_CALL
        startElement(const OUString& aName,
                const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XAttributeList > & xAttribs)
                throw (::com::sun::star::xml::sax::SAXException, ::com::sun::star::uno::RuntimeException)
        {
            m_handler->startElement(aName, xAttribs);
        }

        virtual void SAL_CALL
        endElement(const OUString& aName) throw (::com::sun::star::xml::sax::SAXException,
                ::com::sun::star::uno::RuntimeException)
        {
            m_handler->endElement(aName);
        }

        virtual void SAL_CALL
        characters(const OUString& aChars) throw (::com::sun::star::xml::sax::SAXException,
                ::com::sun::star::uno::RuntimeException)
        {
            m_handler->characters(aChars);
        }

        virtual void SAL_CALL
        ignorableWhitespace(const OUString& aWhitespaces) throw (::com::sun::star::xml::sax::SAXException,
                ::com::sun::star::uno::RuntimeException)
        {
            m_handler->ignorableWhitespace(aWhitespaces);
        }
        virtual void SAL_CALL
        processingInstruction(const OUString& aTarget, const OUString& aData)
                throw (::com::sun::star::xml::sax::SAXException, ::com::sun::star::uno::RuntimeException)
        {
            m_handler->processingInstruction(aTarget, aData);
        }
        virtual void SAL_CALL
        setDocumentLocator(const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XLocator > & xLocator)
                throw (::com::sun::star::xml::sax::SAXException, ::com::sun::star::uno::RuntimeException)
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
        startDocument(void) throw (::com::sun::star::xml::sax::SAXException, ::com::sun::star::uno::RuntimeException)
        {
            m_handler->startDocument();
        }

        virtual void SAL_CALL
        endDocument(void) throw (::com::sun::star::xml::sax::SAXException, ::com::sun::star::uno::RuntimeException)
        {
            m_handler->endDocument();
        }

        virtual void SAL_CALL
        startElement(const OUString& aName,
                const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XAttributeList > & xAttribs)
                throw (::com::sun::star::xml::sax::SAXException, ::com::sun::star::uno::RuntimeException)
        {
            m_handler->startElement(aName, xAttribs);
        }

        virtual void SAL_CALL
        endElement(const OUString& aName) throw (::com::sun::star::xml::sax::SAXException,
                ::com::sun::star::uno::RuntimeException)
        {
            m_handler->endElement(aName);
        }

        virtual void SAL_CALL
        characters(const OUString& aChars) throw (::com::sun::star::xml::sax::SAXException,
                ::com::sun::star::uno::RuntimeException)
        {
            m_handler->characters(aChars);
        }

        virtual void SAL_CALL
        ignorableWhitespace(const OUString& aWhitespaces) throw (::com::sun::star::xml::sax::SAXException,
                ::com::sun::star::uno::RuntimeException)
        {
            m_handler->ignorableWhitespace(aWhitespaces);
        }
        virtual void SAL_CALL
        processingInstruction(const OUString& aTarget, const OUString& aData)
                throw (::com::sun::star::xml::sax::SAXException, ::com::sun::star::uno::RuntimeException)
        {
            m_handler->processingInstruction(aTarget, aData);
        }
        virtual void SAL_CALL
        setDocumentLocator(const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XLocator > & xLocator)
                throw (::com::sun::star::xml::sax::SAXException, ::com::sun::star::uno::RuntimeException)
        {
            m_handler->setDocumentLocator(xLocator);
        }
        // XExtendedDocumentHandler
        virtual void SAL_CALL
        startCDATA(void) throw (::com::sun::star::xml::sax::SAXException, ::com::sun::star::uno::RuntimeException)
        {
            m_handler->startCDATA();
        }
        virtual void SAL_CALL
        endCDATA(void) throw (::com::sun::star::uno::RuntimeException)
        {
            m_handler->endCDATA();
        }
        virtual void SAL_CALL
        comment(const OUString& sComment) throw (::com::sun::star::xml::sax::SAXException,
                ::com::sun::star::uno::RuntimeException)
        {
            m_handler->comment(sComment);
        }
        virtual void SAL_CALL
        unknown(const OUString& sString) throw (::com::sun::star::xml::sax::SAXException, ::com::sun::star::uno::RuntimeException)
        {
            m_handler->unknown(sString);
        }
        virtual void SAL_CALL
        allowLineBreak(void) throw (::com::sun::star::xml::sax::SAXException, ::com::sun::star::uno::RuntimeException)
        {
            m_handler->allowLineBreak();
        }
    protected:
        ExtendedDocumentHandlerAdapter() :
            m_handler(::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XExtendedDocumentHandler > (0, ::com::sun::star::uno::UNO_QUERY))
        {
        }
        ExtendedDocumentHandlerAdapter(
                const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XExtendedDocumentHandler > delegate) :
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
#endif /* _DOCUMENTHANDLERADAPTER_H_ */
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
