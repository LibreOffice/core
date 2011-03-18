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

#include "oox/drawingml/diagram/datamodelcontext.hxx"
#include "oox/helper/attributelist.hxx"
#include "oox/drawingml/fillpropertiesgroupcontext.hxx"
#include "oox/drawingml/shapepropertiescontext.hxx"
#include "oox/drawingml/textbodycontext.hxx"

using namespace ::oox::core;
using namespace ::com::sun::star::xml::sax;
using namespace ::com::sun::star::uno;
using ::rtl::OUString;

namespace oox { namespace drawingml {

// CL_Cxn
class CxnContext
    : public ContextHandler
{
public:
    CxnContext( ContextHandler& rParent,
                const Reference< XFastAttributeList >& xAttribs,
                const dgm::ConnectionPtr & pConnection )
        : ContextHandler( rParent )
        , mpConnection( pConnection )
        {
            sal_Int32 nType = xAttribs->getOptionalValueToken( XML_type, XML_parOf );
            pConnection->mnType = nType;
            pConnection->msModelId = xAttribs->getOptionalValue( XML_modelId );
            pConnection->msSourceId = xAttribs->getOptionalValue( XML_srcId );
            pConnection->msDestId  = xAttribs->getOptionalValue( XML_destId );
            pConnection->msPresId  = xAttribs->getOptionalValue( XML_presId );
            pConnection->msSibTransId  = xAttribs->getOptionalValue( XML_sibTransId );
            AttributeList attribs( xAttribs );
            pConnection->mnSourceOrder = attribs.getInteger( XML_srcOrd, 0 );
            pConnection->mnDestOrder = attribs.getInteger( XML_destOrd, 0 );
        }

    virtual Reference< XFastContextHandler > SAL_CALL
    createFastChildContext( sal_Int32 aElementToken,
                            const Reference< XFastAttributeList >& /*xAttribs*/ )
        throw (SAXException, RuntimeException)
        {
            Reference< XFastContextHandler > xRet;

            switch( aElementToken )
            {
            case DGM_TOKEN( extLst ):
                return xRet;
            default:
                break;
            }
            if( !xRet.is() )
                xRet.set( this );
            return xRet;
        }
private:
    dgm::ConnectionPtr mpConnection;
};


// CT_CxnList
class CxnListContext
    : public ContextHandler
{
public:
    CxnListContext( ContextHandler& rParent,  dgm::Connections & aConnections )
        : ContextHandler( rParent )
        , maConnections( aConnections )
        {
        }
    virtual Reference< XFastContextHandler > SAL_CALL
    createFastChildContext( sal_Int32 aElementToken,
                            const Reference< XFastAttributeList >& xAttribs )
        throw (SAXException, RuntimeException)
        {
            Reference< XFastContextHandler > xRet;

            switch( aElementToken )
            {
            case DGM_TOKEN( cxn ):
            {
                dgm::ConnectionPtr pConnection( new dgm::Connection() );
                maConnections.push_back( pConnection );
                xRet.set( new CxnContext( *this, xAttribs, pConnection ) );
                break;
            }
            default:
                break;
            }
            if( !xRet.is() )
                xRet.set( this );
            return xRet;
        }

private:
    dgm::Connections  & maConnections;
};



// CL_Pt
class PtContext
    : public ContextHandler
{
public:
    PtContext( ContextHandler& rParent,
               const Reference< XFastAttributeList >& xAttribs,
               const dgm::PointPtr & pPoint)
        : ContextHandler( rParent )
        , mpPoint( pPoint )
        {
            mpPoint->setModelId( xAttribs->getOptionalValue( XML_modelId ) );
            //
            // the default type is XML_node
            sal_Int32 nType  = xAttribs->getOptionalValueToken( XML_type, XML_node );
            mpPoint->setType( nType );

            // ignore the cxnId unless it is this type. See 5.15.3.1.3 in Primer
            if( ( nType == XML_parTrans ) || ( nType == XML_sibTrans ) )
            {
                mpPoint->setCnxId( xAttribs->getOptionalValue( XML_cxnId ) );
            }
        }


    virtual Reference< XFastContextHandler > SAL_CALL
    createFastChildContext( sal_Int32 aElementToken,
                            const Reference< XFastAttributeList >& /*xAttribs*/ )
        throw (SAXException, RuntimeException)
        {
            Reference< XFastContextHandler > xRet;

            switch( aElementToken )
            {
            case DGM_TOKEN( extLst ):
                return xRet;
            case DGM_TOKEN( prSet ):
                // TODO
                // CT_ElemPropSet
                break;
            case DGM_TOKEN( spPr ):
                OSL_TRACE( "shape props for point");
                xRet = new ShapePropertiesContext( *this, *mpPoint->getShape() );
                break;
            case DGM_TOKEN( t ):
            {
                OSL_TRACE( "shape text body for point");
                TextBodyPtr xTextBody( new TextBody );
                mpPoint->getShape()->setTextBody( xTextBody );
                xRet = new TextBodyContext( *this, *xTextBody );
                break;
            }
            default:
                break;
            }
            if( !xRet.is() )
                xRet.set( this );
            return xRet;
        }

private:
    dgm::PointPtr mpPoint;
};



// CT_PtList
class PtListContext
    : public ContextHandler
{
public:
    PtListContext( ContextHandler& rParent,  dgm::Points & aPoints)
        : ContextHandler( rParent )
        , maPoints( aPoints )
        {
        }
    virtual Reference< XFastContextHandler > SAL_CALL
    createFastChildContext( sal_Int32 aElementToken,
                            const Reference< XFastAttributeList >& xAttribs )
        throw (SAXException, RuntimeException)
        {
            Reference< XFastContextHandler > xRet;

            switch( aElementToken )
            {
            case DGM_TOKEN( pt ):
            {
                // CT_Pt
                dgm::PointPtr pPoint( new dgm::Point() );
                maPoints.push_back( pPoint );
                xRet.set( new PtContext( *this, xAttribs, pPoint ) );
                break;
            }
            default:
                break;
            }
            if( !xRet.is() )
                xRet.set( this );
            return xRet;
        }

private:
    dgm::Points  & maPoints;
};

// CT_BackgroundFormatting
class BackgroundFormattingContext
    : public ContextHandler
{
public:
    BackgroundFormattingContext( ContextHandler& rParent, DiagramDataPtr & pModel )
        : ContextHandler( rParent )
        , mpDataModel( pModel )
        {
            OSL_ENSURE( pModel, "the data model MUST NOT be NULL" );
        }

    virtual Reference< XFastContextHandler > SAL_CALL
    createFastChildContext( sal_Int32 aElementToken,
                            const Reference< XFastAttributeList >& xAttribs )
        throw (SAXException, RuntimeException)
        {
            Reference< XFastContextHandler > xRet;

            switch( aElementToken )
            {
            case A_TOKEN( blipFill ):
            case A_TOKEN( gradFill ):
            case A_TOKEN( grpFill ):
            case A_TOKEN( noFill ):
            case A_TOKEN( pattFill ):
            case A_TOKEN( solidFill ):
                // EG_FillProperties
                xRet.set( FillPropertiesContext::createFillContext(
                    *this, aElementToken, xAttribs, *mpDataModel->getFillProperties() ) );
                break;
            case A_TOKEN( effectDag ):
            case A_TOKEN( effectLst ):
                // TODO
                // EG_EffectProperties
                break;
            default:
                break;
            }
            if( !xRet.is() )
                xRet.set( this );
            return xRet;
        }
private:
    DiagramDataPtr mpDataModel;
};



DataModelContext::DataModelContext( ContextHandler& rParent,
                                    const DiagramDataPtr & pDataModel )
    : ContextHandler( rParent )
    , mpDataModel( pDataModel )
{
    OSL_ENSURE( pDataModel, "Data Model must not be NULL" );
}


DataModelContext::~DataModelContext()
{
    // some debug
    mpDataModel->dump();
}


Reference< XFastContextHandler > SAL_CALL
DataModelContext::createFastChildContext( ::sal_Int32 aElement,
                                          const Reference< XFastAttributeList >& /*xAttribs*/ )
    throw ( SAXException, RuntimeException)
{
    Reference< XFastContextHandler > xRet;

    switch( aElement )
    {
    case DGM_TOKEN( cxnLst ):
        // CT_CxnList
        xRet.set( new CxnListContext( *this, mpDataModel->getConnections() ) );
        break;
    case DGM_TOKEN( ptLst ):
        // CT_PtList
        xRet.set( new PtListContext( *this, mpDataModel->getPoints() ) );
        break;
    case DGM_TOKEN( bg ):
        // CT_BackgroundFormatting
        xRet.set( new BackgroundFormattingContext( *this, mpDataModel ) );
        break;
    case DGM_TOKEN( whole ):
        // CT_WholeE2oFormatting
        // TODO
        return xRet;
    case DGM_TOKEN( extLst ):
        return xRet;
    default:
        break;
    }

    if( !xRet.is() )
        xRet.set( this );

    return xRet;
}

} }

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
