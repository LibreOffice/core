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

#include "oox/ole/axcontrolfragment.hxx"
#include "oox/helper/binaryinputstream.hxx"
#include "oox/helper/binaryoutputstream.hxx"
#include "oox/core/xmlfilterbase.hxx"
#include "oox/ole/axcontrol.hxx"
#include "oox/ole/olehelper.hxx"
#include "oox/ole/olestorage.hxx"

using ::rtl::OUString;
using ::com::sun::star::io::XInputStream;
using ::com::sun::star::uno::Reference;
using ::oox::core::ContextHandler2;
using ::oox::core::ContextHandlerRef;
using ::oox::core::FragmentHandler2;
using ::oox::core::XmlFilterBase;

namespace oox {
namespace ole {

// ============================================================================

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
                        return 0;
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
                if( aPicturePath.getLength() > 0 )
                {
                    BinaryXInputStream aInStrm( getFilter().openInputStream( aPicturePath ), true );
                    mrModel.importPictureData( mnPropId, aInStrm );
                }
            }
        break;
    }
    return 0;
}

// ============================================================================

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
                if( ControlModelBase* pModel = mrControl.createModel( aClassId ).get() )
                    return new AxControlPropertyContext( *this, *pModel );
            break;

            case XML_persistStreamInit:
            {
                OUString aFragmentPath = getFragmentPathFromRelId( rAttribs.getString( R_TOKEN( id ), OUString() ) );
                if( aFragmentPath.getLength() > 0 )
                {
                    BinaryXInputStream aInStrm( getFilter().openInputStream( aFragmentPath ), true );
                    if( !aInStrm.isEof() )
                    {
                        // binary stream contains a copy of the class ID, must be equal to attribute value
                        OUString aStrmClassId = OleHelper::importGuid( aInStrm );
                        OSL_ENSURE( aClassId.equalsIgnoreAsciiCase( aStrmClassId ),
                            "AxControlFragment::importBinaryControl - form control class ID mismatch" );
                        if( ControlModelBase* pModel = mrControl.createModel( aStrmClassId ).get() )
                            pModel->importBinaryModel( aInStrm );
                    }
                }
            }
            break;

            case XML_persistStorage:
            {
                OUString aFragmentPath = getFragmentPathFromRelId( rAttribs.getString( R_TOKEN( id ), OUString() ) );
                if( aFragmentPath.getLength() > 0 )
                {
                    Reference< XInputStream > xStrgStrm = getFilter().openInputStream( aFragmentPath );
                    if( xStrgStrm.is() )
                    {
                        OleStorage aStorage( getFilter().getGlobalFactory(), xStrgStrm, false );
                        BinaryXInputStream aInStrm( aStorage.openInputStream( CREATE_OUSTRING( "f" ) ), true );
                        if( !aInStrm.isEof() )
                            if( AxContainerModelBase* pModel = dynamic_cast< AxContainerModelBase* >( mrControl.createModel( aClassId ).get() ) )
                                pModel->importBinaryModel( aInStrm );
                    }
                }
            }
            break;
        }
    }
    return 0;
}

// ============================================================================

} // namespace ole
} // namespace oox
