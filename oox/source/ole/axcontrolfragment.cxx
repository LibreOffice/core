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

#include <oox/ole/axcontrolfragment.hxx>

#include <com/sun/star/io/XInputStream.hpp>
#include <oox/core/xmlfilterbase.hxx>
#include <oox/helper/attributelist.hxx>
#include <oox/helper/binaryinputstream.hxx>
#include <oox/ole/axcontrol.hxx>
#include <oox/ole/olehelper.hxx>
#include <oox/ole/olestorage.hxx>
#include <oox/token/namespaces.hxx>
#include <oox/token/tokens.hxx>

#include <osl/diagnose.h>

namespace oox::ole {

using namespace ::com::sun::star::io;
using namespace ::com::sun::star::uno;

using ::oox::core::ContextHandler2;
using ::oox::core::ContextHandlerRef;
using ::oox::core::FragmentHandler2;
using ::oox::core::XmlFilterBase;

AxControlPropertyContext::AxControlPropertyContext( FragmentHandler2 const & rFragment, ControlModelBase& rModel ) :
    ContextHandler2( rFragment ),
    mrModel( rModel ),
    mnPropId( XML_TOKEN_INVALID )
{
}

ContextHandlerRef AxControlPropertyContext::onCreateContext( sal_Int32 nElement, const AttributeList& rAttribs )
{
    switch( getCurrentElement() )
    {
        case AX_TOKEN( ocx ):
            if( nElement == AX_TOKEN( ocxPr ) )
            {
                mnPropId = rAttribs.getToken( AX_TOKEN( name ), XML_TOKEN_INVALID );
                switch( mnPropId )
                {
                    case XML_TOKEN_INVALID:
                        return nullptr;
                    case XML_Picture:
                    case XML_MouseIcon:
                        return this;        // import picture path from ax:picture child element
                    default:
                        mrModel.importProperty( mnPropId, rAttribs.getStringDefaulted( AX_TOKEN( value )) );
                }
            }
        break;

        case AX_TOKEN( ocxPr ):
            if( nElement == AX_TOKEN( picture ) )
            {
                OUString aPicturePath = getFragmentPathFromRelId( rAttribs.getStringDefaulted( R_TOKEN( id )) );
                if( !aPicturePath.isEmpty() )
                {
                    BinaryXInputStream aInStrm( getFilter().openInputStream( aPicturePath ), true );
                    mrModel.importPictureData( mnPropId, aInStrm );
                }
            }
        break;
    }
    return nullptr;
}

AxControlFragment::AxControlFragment( XmlFilterBase& rFilter, const OUString& rFragmentPath, EmbeddedControl& rControl ) :
    FragmentHandler2( rFilter, rFragmentPath, true ),
    mrControl( rControl )
{
}

ContextHandlerRef AxControlFragment::onCreateContext( sal_Int32 nElement, const AttributeList& rAttribs )
{
    if( isRootElement() && (nElement == AX_TOKEN( ocx )) )
    {
        OUString aClassId = rAttribs.getStringDefaulted( AX_TOKEN( classid ));
        switch( rAttribs.getToken( AX_TOKEN( persistence ), XML_TOKEN_INVALID ) )
        {
            case XML_persistPropertyBag:
                if( ControlModelBase* pModel = mrControl.createModelFromGuid( aClassId ) )
                    return new AxControlPropertyContext( *this, *pModel );
            break;

            case XML_persistStreamInit:
            {
                OUString aFragmentPath = getFragmentPathFromRelId( rAttribs.getStringDefaulted( R_TOKEN( id )) );
                if( !aFragmentPath.isEmpty() )
                {
                    BinaryXInputStream aInStrm( getFilter().openInputStream( aFragmentPath ), true );
                    if( !aInStrm.isEof() )
                    {
                        // binary stream contains a copy of the class ID, must be equal to attribute value
                        OUString aStrmClassId = OleHelper::importGuid( aInStrm );
                        OSL_ENSURE( aClassId.equalsIgnoreAsciiCase( aStrmClassId ),
                            "AxControlFragment::importBinaryControl - form control class ID mismatch" );
                        if( ControlModelBase* pModel = mrControl.createModelFromGuid( aStrmClassId ) )
                            pModel->importBinaryModel( aInStrm );
                    }
                }
            }
            break;

            case XML_persistStorage:
            {
                OUString aFragmentPath = getFragmentPathFromRelId( rAttribs.getStringDefaulted( R_TOKEN( id )) );
                if( !aFragmentPath.isEmpty() )
                {
                    Reference< XInputStream > xStrgStrm = getFilter().openInputStream( aFragmentPath );
                    if( xStrgStrm.is() )
                    {
                        // Try to import as a parent control
                        bool bImportedAsParent = false;
                        OleStorage aStorage( getFilter().getComponentContext(), xStrgStrm, false );
                        BinaryXInputStream aInStrm( aStorage.openInputStream( u"f"_ustr ), true );
                        if( !aInStrm.isEof() )
                        {
                            if( AxContainerModelBase* pModel = dynamic_cast< AxContainerModelBase* >( mrControl.createModelFromGuid( aClassId ) ) )
                            {
                                pModel->importBinaryModel( aInStrm );
                                bImportedAsParent = true;
                            }
                        }
                        // Import it as a non-parent control
                        if(!bImportedAsParent)
                        {
                            BinaryXInputStream aInStrm2(aStorage.openInputStream(u"contents"_ustr), true);
                            if (!aInStrm2.isEof())
                            {
                                if (ControlModelBase* pModel = mrControl.createModelFromGuid(aClassId))
                                {
                                    pModel->importBinaryModel(aInStrm2);
                                }
                            }
                        }
                    }
                }
            }
            break;
        }
    }
    return nullptr;
}

} // namespace oox::ole

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
