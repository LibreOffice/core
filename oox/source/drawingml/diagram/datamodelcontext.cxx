/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: datamodelcontext.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: kz $ $Date: 2008-03-05 18:37:38 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#include "oox/drawingml/diagram/datamodelcontext.hxx"
#include "oox/helper/attributelist.hxx"
#include "oox/core/skipcontext.hxx"
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
            case NMSP_DIAGRAM|XML_extLst:
                xRet.set( new SkipContext( *this ) );
                break;
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
            case NMSP_DIAGRAM|XML_cxn:
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
            case NMSP_DIAGRAM|XML_extLst:
                xRet.set( new SkipContext( *this ) );
                break;
            case NMSP_DIAGRAM|XML_prSet:
                // TODO
                // CT_ElemPropSet
                break;
            case NMSP_DIAGRAM|XML_spPr:
                OSL_TRACE( "shape props for point");
                xRet = new ShapePropertiesContext( *this, *mpPoint->getShape() );
                break;
            case NMSP_DIAGRAM|XML_t:
                OSL_TRACE( "shape text body for point");
                xRet = new TextBodyContext( *this, *mpPoint->getShape() );
                break;
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
            case NMSP_DIAGRAM|XML_pt:
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
            case NMSP_DRAWINGML|XML_blipFill:
            case NMSP_DRAWINGML|XML_gradFill:
            case NMSP_DRAWINGML|XML_grpFill:
            case NMSP_DRAWINGML|XML_noFill:
            case NMSP_DRAWINGML|XML_pattFill:
            case NMSP_DRAWINGML|XML_solidFill:
                // EG_FillProperties
                xRet.set( FillPropertiesGroupContext::StaticCreateContext(
                    *this, aElementToken, xAttribs, *mpDataModel->getFillProperties() ) );
                break;
            case NMSP_DRAWINGML|XML_effectDag:
            case NMSP_DRAWINGML|XML_effectLst:
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
    case NMSP_DIAGRAM|XML_cxnLst:
        // CT_CxnList
        xRet.set( new CxnListContext( *this, mpDataModel->getConnections() ) );
        break;
    case NMSP_DIAGRAM|XML_ptLst:
        // CT_PtList
        xRet.set( new PtListContext( *this, mpDataModel->getPoints() ) );
        break;
    case NMSP_DIAGRAM|XML_bg:
        // CT_BackgroundFormatting
        xRet.set( new BackgroundFormattingContext( *this, mpDataModel ) );
        break;
    case NMSP_DIAGRAM|XML_whole:
        // CT_WholeE2oFormatting
        // TODO
        xRet.set( new SkipContext( *this ) );
        break;
    case NMSP_DIAGRAM|XML_extLst:
        xRet.set( new SkipContext( *this ) );
        break;
    default:
        break;
    }

    if( !xRet.is() )
        xRet.set( this );

    return xRet;
}

} }
