/*************************************************************************
 *
 *  $RCSfile: saxbuilder.hxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: lo $ $Date: 2004-01-28 16:31:44 $
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
 *  Copyright: 2004 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef _SAXBUILDER_HXX
#define _SAXBUILDER_HXX

#include <stack>
#include <map>

#include <sal/types.h>
#include <cppuhelper/implbase3.hxx>
#include <com/sun/star/uno/Reference.h>
#include <com/sun/star/uno/Sequence.h>

#include <com/sun/star/uno/XInterface.hpp>
#include <com/sun/star/uno/Exception.hpp>
#include <com/sun/star/xml/dom/XSAXDocumentBuilder.hpp>
#include <com/sun/star/xml/dom/SAXDocumentBuilderState.hpp>
#include <com/sun/star/xml/dom/XDocument.hpp>
#include <com/sun/star/xml/dom/XDocumentFragment.hpp>
#include <com/sun/star/xml/sax/XDocumentHandler.hpp>
#include <com/sun/star/xml/sax/XLocator.hpp>
#include <com/sun/star/xml/sax/XAttributeList.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/lang/XSingleServiceFactory.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>

#include "libxml/tree.h"

using namespace rtl;
using namespace com::sun::star::uno;
using namespace com::sun::star::lang;
using namespace com::sun::star::xml::dom;
using namespace com::sun::star::xml::sax;

namespace DOM
{

    typedef std::stack< Reference< XNode > > NodeStack;
    typedef std::map< OUString, OUString > NSMap;
    typedef std::map< OUString, OUString > AttrMap;
    typedef std::stack< NSMap > NSStack;
    
    class  CSAXDocumentBuilder
        : public ::cppu::WeakImplHelper3< XDocumentHandler, XSAXDocumentBuilder, XServiceInfo >
    {

    private:
        SAXDocumentBuilderState m_aState;
        NodeStack m_aNodeStack;
        NSStack m_aNSStack;

        OUString resolvePrefix(const OUString& aPrefix);

        Reference< XDocument > m_aDocument;
        Reference< XDocumentFragment > m_aFragment;
        Reference< XLocator > m_aLocator;
        const Reference< XMultiServiceFactory > m_aServiceManager;
        
    public:

        // call for factory
        static Reference< XInterface > getInstance(const Reference < XMultiServiceFactory >& xFactory);

        // static helpers for service info and component management
        static const char* aImplementationName;
        static const char* aSupportedServiceNames[];
        static OUString _getImplementationName();
        static Sequence< OUString > _getSupportedServiceNames();
        static Reference< XInterface > _getInstance(const Reference< XMultiServiceFactory >& rSMgr);

        CSAXDocumentBuilder(const Reference< XMultiServiceFactory >& mgr);
        
        // XServiceInfo
        virtual OUString SAL_CALL getImplementationName()
            throw (RuntimeException);
        virtual sal_Bool SAL_CALL supportsService(const OUString& ServiceName)
            throw (RuntimeException);
        virtual Sequence< OUString > SAL_CALL getSupportedServiceNames ()
            throw (RuntimeException);

        // XDocumentHandler
        virtual void SAL_CALL startDocument()
            throw( com::sun::star::xml::sax::SAXException );
        virtual void SAL_CALL endDocument()
            throw( com::sun::star::xml::sax::SAXException );
        virtual void SAL_CALL startElement( const OUString& aName,
             const Reference< XAttributeList >& xAttribs )
            throw( com::sun::star::xml::sax::SAXException );
        virtual void SAL_CALL endElement( const OUString& aName )
            throw( com::sun::star::xml::sax::SAXException );
        virtual void SAL_CALL characters( const OUString& aChars )
            throw( com::sun::star::xml::sax::SAXException );
        virtual void SAL_CALL ignorableWhitespace( const OUString& aWhitespaces )
            throw( com::sun::star::xml::sax::SAXException );
        virtual void SAL_CALL processingInstruction( const OUString& aTarget,
             const OUString& aData )
            throw( com::sun::star::xml::sax::SAXException );
        virtual void SAL_CALL setDocumentLocator( const Reference< XLocator >& xLocator )
            throw( com::sun::star::xml::sax::SAXException );
    
    
        // XSAXDocumentBuilder
        virtual SAXDocumentBuilderState SAL_CALL getState() 
            throw (RuntimeException);
        virtual void SAL_CALL reset() 
            throw (RuntimeException);
        virtual Reference< XDocument > SAL_CALL getDocument()
            throw (RuntimeException);
        virtual Reference< XDocumentFragment > SAL_CALL getDocumentFragment()
            throw (RuntimeException);
        virtual void SAL_CALL startDocumentFragment(const Reference< XDocument >& ownerDoc)
            throw (RuntimeException);
        virtual void SAL_CALL endDocumentFragment()
            throw (RuntimeException);
            

    };
}

#endif
