/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */


#include "fmdocumentclassification.hxx"

#include <com/sun/star/container/XChild.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/xforms/XFormsSupplier.hpp>
#include <com/sun/star/sdbc/XConnection.hpp>
#include <com/sun/star/frame/XModule.hpp>

#include <tools/diagnose_ex.h>


namespace svxform
{


    namespace
    {
        using ::com::sun::star::uno::Reference;
        using ::com::sun::star::uno::XInterface;
        using ::com::sun::star::container::XChild;
        using ::com::sun::star::frame::XModel;
        using ::com::sun::star::uno::UNO_QUERY;
        using ::com::sun::star::frame::XModule;


        template< class TYPE >
        Reference< TYPE > getTypedModelNode( const Reference< XInterface >& _rxModelNode )
        {
            Reference< TYPE > xTypedNode( _rxModelNode, UNO_QUERY );
            if ( xTypedNode.is() )
                return xTypedNode;
            else
            {
                Reference< XChild > xChild( _rxModelNode, UNO_QUERY );
                if ( xChild.is() )
                    return getTypedModelNode< TYPE >( xChild->getParent() );
                else
                    return Reference< TYPE >();
            }
        }


        Reference< XModel > getDocument( const Reference< XInterface >& _rxModelNode )
        {
            return getTypedModelNode< XModel >( _rxModelNode );
        }
    }

    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::frame;
    using namespace ::com::sun::star::lang;
    using namespace ::com::sun::star::xforms;
    using namespace ::com::sun::star::container;
    using namespace ::com::sun::star::sdbc;



    namespace
    {

        struct ModuleInfo
        {
            const sal_Char* pAsciiModuleOrServiceName;
            DocumentType    eType;
        };


        const ModuleInfo* lcl_getModuleInfo()
        {
            static const ModuleInfo aModuleInfo[] =
            {
                { "com.sun.star.text.TextDocument", eTextDocument },
                { "com.sun.star.text.WebDocument", eWebDocument },
                { "com.sun.star.sheet.SpreadsheetDocument", eSpreadsheetDocument },
                { "com.sun.star.drawing.DrawingDocument", eDrawingDocument },
                { "com.sun.star.presentation.PresentationDocument", ePresentationDocument },
                { "com.sun.star.xforms.XMLFormDocument", eEnhancedForm },
                { "com.sun.star.sdb.FormDesign", eDatabaseForm },
                { "com.sun.star.sdb.TextReportDesign", eDatabaseReport },
                { "com.sun.star.text.GlobalDocument", eTextDocument },
                { nullptr, eUnknownDocumentType }
            };
            return aModuleInfo;
        }
    }


    //= DocumentClassification


    DocumentType DocumentClassification::classifyDocument( const Reference< XModel >& _rxDocumentModel )
    {
        DocumentType eType( eUnknownDocumentType );

        OSL_ENSURE( _rxDocumentModel.is(), "DocumentClassification::classifyDocument: invalid document!" );
        if ( !_rxDocumentModel.is() )
            return eType;

        try
        {
            // first, check whether the document has a ModuleIdentifier which we know
            Reference< XModule > xModule( _rxDocumentModel, UNO_QUERY );
            if ( xModule.is() )
                eType = getDocumentTypeForModuleIdentifier( xModule->getIdentifier() );
            if ( eType != eUnknownDocumentType )
                return eType;

            // second, check whether it supports one of the services we know
            Reference< XServiceInfo > xSI( _rxDocumentModel, UNO_QUERY_THROW );
            const ModuleInfo* pModuleInfo = lcl_getModuleInfo();
            while ( pModuleInfo->pAsciiModuleOrServiceName )
            {
                if ( xSI->supportsService( OUString::createFromAscii( pModuleInfo->pAsciiModuleOrServiceName ) ) )
                    return pModuleInfo->eType;
                ++pModuleInfo;
            }

            // last: uhm, there is no last resort
            OSL_FAIL( "DocumentClassification::classifyDocument: unknown document!" );
        }
        catch( const Exception& )
        {
            DBG_UNHANDLED_EXCEPTION();
        }

        return eType;
    }


    DocumentType DocumentClassification::classifyHostDocument( const Reference< XInterface >& _rxFormComponent )
    {
        DocumentType eType( eUnknownDocumentType );

        try
        {
            Reference< XModel > xDocument( getDocument( _rxFormComponent.get() ) );
            if ( !xDocument.is() )
                return eUnknownDocumentType;
            eType = classifyDocument( xDocument );
        }
        catch( const Exception& )
        {
            OSL_FAIL( "DocumentClassification::classifyHostDocument: caught an exception!" );
        }

        return eType;
    }


    DocumentType DocumentClassification::getDocumentTypeForModuleIdentifier( const OUString& _rModuleIdentifier )
    {
        const ModuleInfo* pModuleInfo = lcl_getModuleInfo();
        while ( pModuleInfo->pAsciiModuleOrServiceName )
        {
            if ( _rModuleIdentifier.equalsAscii( pModuleInfo->pAsciiModuleOrServiceName ) )
                return pModuleInfo->eType;
            ++pModuleInfo;
        }
        return eUnknownDocumentType;
    }


    OUString DocumentClassification::getModuleIdentifierForDocumentType( DocumentType _eType )
    {
        const ModuleInfo* pModuleInfo = lcl_getModuleInfo();
        while ( pModuleInfo->pAsciiModuleOrServiceName )
        {
            if ( pModuleInfo->eType == _eType )
                return OUString::createFromAscii( pModuleInfo->pAsciiModuleOrServiceName );
            ++pModuleInfo;
        }
        return OUString();
    }


}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
