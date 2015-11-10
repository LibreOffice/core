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

#include "oox/ole/axcontrolfragment.hxx"

#include "oox/core/xmlfilterbase.hxx"
#include "oox/helper/binaryinputstream.hxx"
#include "oox/helper/binaryoutputstream.hxx"
#include "oox/ole/axcontrol.hxx"
#include "oox/ole/olehelper.hxx"
#include "oox/ole/olestorage.hxx"

#include <osl/diagnose.h>

namespace oox {
namespace ole {

using namespace ::com::sun::star::io;
using namespace ::com::sun::star::uno;

using ::oox::core::ContextHandler2;
using ::oox::core::ContextHandlerRef;
using ::oox::core::FragmentHandler2;
using ::oox::core::XmlFilterBase;

AxControlPropertyContext::AxControlPropertyContext( FragmentHandler2& rFragment, ControlModelBase& rModel ) :
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
                        mrModel.importProperty( mnPropId, rAttribs.getString( AX_TOKEN( value ), OUString() ) );
                }
            }
        break;

        case AX_TOKEN( ocxPr ):
            if( nElement == AX_TOKEN( picture ) )
            {
                OUString aPicturePath = getFragmentPathFromRelId( rAttribs.getString( R_TOKEN( id ), OUString() ) );
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
        OUString aClassId = rAttribs.getString( AX_TOKEN( classid ), OUString() );
        switch( rAttribs.getToken( AX_TOKEN( persistence ), XML_TOKEN_INVALID ) )
        {
            case XML_persistPropertyBag:
                if( ControlModelBase* pModel = mrControl.createModelFromGuid( aClassId ) )
                    return new AxControlPropertyContext( *this, *pModel );
            break;

            case XML_persistStreamInit:
            {
                OUString aFragmentPath = getFragmentPathFromRelId( rAttribs.getString( R_TOKEN( id ), OUString() ) );
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
                OUString aFragmentPath = getFragmentPathFromRelId( rAttribs.getString( R_TOKEN( id ), OUString() ) );
                if( !aFragmentPath.isEmpty() )
                {
                    Reference< XInputStream > xStrgStrm = getFilter().openInputStream( aFragmentPath );
                    if( xStrgStrm.is() )
                    {
                        OleStorage aStorage( getFilter().getComponentContext(), xStrgStrm, false );
                        BinaryXInputStream aInStrm( aStorage.openInputStream( "f" ), true );
                        if( !aInStrm.isEof() )
                            if( AxContainerModelBase* pModel = dynamic_cast< AxContainerModelBase* >( mrControl.createModelFromGuid( aClassId ) ) )
                                pModel->importBinaryModel( aInStrm );
                    }
                }
            }
            break;
        }
    }
    return nullptr;
}

} // namespace ole
} // namespace oox

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
