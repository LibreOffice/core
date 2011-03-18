/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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
#include "precompiled_svx.hxx"

#include "fmdocumentclassification.hxx"
#include "svx/dbtoolsclient.hxx"

/** === begin UNO includes === **/
#include <com/sun/star/container/XChild.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/xforms/XFormsSupplier.hpp>
#include <com/sun/star/sdbc/XConnection.hpp>
#include <com/sun/star/frame/XModule.hpp>
/** === end UNO includes === **/

#include <tools/diagnose_ex.h>

//........................................................................
namespace svxform
{
//........................................................................

    namespace
    {
        using ::com::sun::star::uno::Reference;
        using ::com::sun::star::uno::XInterface;
        using ::com::sun::star::container::XChild;
        using ::com::sun::star::frame::XModel;
        using ::com::sun::star::uno::UNO_QUERY;
        using ::com::sun::star::frame::XModule;

        //....................................................................
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

        //....................................................................
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

    //====================================================================
    //====================================================================
    namespace
    {
        //----------------------------------------------------------------
        struct ModuleInfo
        {
            const sal_Char* pAsciiModuleOrServiceName;
            DocumentType    eType;
        };

        //----------------------------------------------------------------
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
                { NULL, eUnknownDocumentType }
            };
            return aModuleInfo;
        }
    }

    //====================================================================
    //= DocumentClassification
    //====================================================================
    //--------------------------------------------------------------------
    DocumentType DocumentClassification::classifyDocument( const Reference< XModel >& _rxDocumentModel ) SAL_THROW(())
    {
        DocumentType eType( eUnknownDocumentType );

        OSL_ENSURE( _rxDocumentModel.is(), "DocumentClassification::classifyDocument: invalid document!" );
        if ( !_rxDocumentModel.is() )
            return eType;

        try
        {
            // first, check whether the document has a ModuleIdentifier which we know
            ::rtl::OUString sModuleIdentifier;
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
                if ( xSI->supportsService( ::rtl::OUString::createFromAscii( pModuleInfo->pAsciiModuleOrServiceName ) ) )
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

    //--------------------------------------------------------------------
    DocumentType DocumentClassification::classifyHostDocument( const Reference< XInterface >& _rxFormComponent ) SAL_THROW(())
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
            OSL_ENSURE( sal_False, "DocumentClassification::classifyHostDocument: caught an exception!" );
        }

        return eType;
    }

    //--------------------------------------------------------------------
    DocumentType DocumentClassification::getDocumentTypeForModuleIdentifier( const ::rtl::OUString& _rModuleIdentifier )
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

    //--------------------------------------------------------------------
    ::rtl::OUString DocumentClassification::getModuleIdentifierForDocumentType( DocumentType _eType )
    {
        const ModuleInfo* pModuleInfo = lcl_getModuleInfo();
        while ( pModuleInfo->pAsciiModuleOrServiceName )
        {
            if ( pModuleInfo->eType == _eType )
                return ::rtl::OUString::createFromAscii( pModuleInfo->pAsciiModuleOrServiceName );
            ++pModuleInfo;
        }
        return ::rtl::OUString();
    }

//........................................................................
} // namespace svxform
//........................................................................

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
