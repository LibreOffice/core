/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: datamodelcontext.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2008-01-17 08:05:57 $
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
    : public Context
{
public:
    CxnContext( const FragmentHandlerRef& xParent,
                const Reference< XFastAttributeList >& xAttribs,
                const dgm::ConnectionPtr & pConnection )
        : Context( xParent )
        , mpConnection( pConnection )
        {
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
                xRet.set( new SkipContext( getHandler() ) );
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
    : public Context
{
public:
    CxnListContext( const FragmentHandlerRef& xParent,  dgm::Connections & aConnections )
        : Context( xParent )
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
                xRet.set( new CxnContext( getHandler( ), xAttribs, pConnection ) );
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
    : public Context
{
public:
    PtContext( const FragmentHandlerRef& xParent,
               const Reference< XFastAttributeList >& xAttribs,
               const dgm::PointPtr & pPoint)
        : Context( xParent )
        , mpPoint( pPoint )
        {
            // both can be either an int or a uuid
            mpPoint->setCnxId( xAttribs->getOptionalValue( XML_cxnId ) );
            mpPoint->setModelId( xAttribs->getOptionalValue( XML_modelId ) );
            //
            mpPoint->setType( xAttribs->getOptionalValueToken( XML_type, 0 ) );
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
                xRet.set( new SkipContext( getHandler() ) );
                break;
            case NMSP_DIAGRAM|XML_prSet:
                // TODO
                // CT_ElemPropSet
                break;
            case NMSP_DIAGRAM|XML_spPr:
                xRet = new ShapePropertiesContext( this, *(mpPoint->getShape().get()) );
                break;
            case NMSP_DIAGRAM|XML_t:
                xRet = new TextBodyContext( getHandler(), *(mpPoint->getShape().get()) );
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
    : public Context
{
public:
    PtListContext( const FragmentHandlerRef& xParent,  dgm::Points & aPoints)
        : Context( xParent )
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
                xRet.set( new PtContext( getHandler( ), xAttribs, pPoint ) );
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
    : public Context
{
public:
    BackgroundFormattingContext( const FragmentHandlerRef& xParent, DiagramDataPtr & pModel )
        : Context( xParent )
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
                xRet.set( FillPropertiesGroupContext::StaticCreateContext( getHandler( ),
                                                                           aElementToken,
                                                                           xAttribs,
                                                                           *(mpDataModel->getFillProperties().get()) ) );
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



DataModelContext::DataModelContext( const FragmentHandlerRef& xHandler,
                                    const DiagramDataPtr & pDataModel )
    : Context( xHandler )
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
        xRet.set( new CxnListContext( getHandler( ), mpDataModel->getConnections() ) );
        break;
    case NMSP_DIAGRAM|XML_ptLst:
        // CT_PtList
        xRet.set( new PtListContext( getHandler( ), mpDataModel->getPoints() ) );
        break;
    case NMSP_DIAGRAM|XML_bg:
        // CT_BackgroundFormatting
        xRet.set( new BackgroundFormattingContext( getHandler(), mpDataModel ) );
        break;
    case NMSP_DIAGRAM|XML_whole:
        // CT_WholeE2oFormatting
        // TODO
        xRet.set( new SkipContext( getHandler() ) );
        break;
    case NMSP_DIAGRAM|XML_extLst:
        xRet.set( new SkipContext( getHandler() ) );
        break;
    default:
        break;
    }

    if( !xRet.is() )
        xRet.set( this );

    return xRet;
}

} }
