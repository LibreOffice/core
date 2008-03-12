/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: ReportUndoFactory.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2008-03-12 10:16:07 $
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
#include "ReportUndoFactory.hxx"
#include "RptObject.hxx"
#include "UndoActions.hxx"
#include "RptResId.hrc"

namespace rptui
{
    using namespace ::com::sun::star;
// -----------------------------------------------------------------------------
SdrUndoAction* lcl_createUndo(SdrObject& rObject,Action _eAction,USHORT _nCommentId)
{
    OObjectBase* pObj = dynamic_cast<OObjectBase*>(&rObject);
    if ( !pObj )
        return NULL;
    uno::Reference< report::XReportComponent> xReportComponent = pObj->getReportComponent();
    uno::Reference< report::XSection> xSection = pObj->getSection();
    uno::Reference< report::XGroup> xGroup = xSection->getGroup();
    SdrUndoAction* pUndo = NULL;
    if ( xGroup.is() )
        pUndo = new OUndoGroupSectionAction(*rObject.GetModel(),_eAction,OGroupHelper::getMemberFunction(xSection),xGroup,xReportComponent,_nCommentId);
    else
        pUndo = new OUndoReportSectionAction(*rObject.GetModel(),_eAction,OReportHelper::getMemberFunction(xSection),xSection->getReportDefinition(),xReportComponent,_nCommentId);
    return pUndo;
}
// -----------------------------------------------------------------------------
DBG_NAME( rpt_OReportUndoFactory )
// -----------------------------------------------------------------------------
OReportUndoFactory::OReportUndoFactory() : m_pUndoFactory(new SdrUndoFactory)
{
    DBG_CTOR( rpt_OReportUndoFactory,NULL);
}
// -----------------------------------------------------------------------------
OReportUndoFactory::~OReportUndoFactory()
{
    DBG_DTOR( rpt_OReportUndoFactory,NULL);
}
///////////////////////////////////////////////////////////////////////
// shapes
SdrUndoAction* OReportUndoFactory::CreateUndoMoveObject( SdrObject& rObject )
{
    return m_pUndoFactory->CreateUndoMoveObject( rObject );
}

SdrUndoAction* OReportUndoFactory::CreateUndoMoveObject( SdrObject& rObject, const Size& rDist )
{
    return m_pUndoFactory->CreateUndoMoveObject( rObject, rDist );
}

SdrUndoAction* OReportUndoFactory::CreateUndoGeoObject( SdrObject& rObject )
{
    return m_pUndoFactory->CreateUndoGeoObject( rObject );
}

SdrUndoAction* OReportUndoFactory::CreateUndoAttrObject( SdrObject& rObject, bool bStyleSheet1, bool bSaveText )
{
    return m_pUndoFactory->CreateUndoAttrObject( rObject, bStyleSheet1 ? TRUE : FALSE, bSaveText ? TRUE : FALSE );
}

SdrUndoAction* OReportUndoFactory::CreateUndoRemoveObject( SdrObject& rObject, bool bOrdNumDirect )
{
    return m_pUndoFactory->CreateUndoRemoveObject( rObject, bOrdNumDirect ? TRUE : FALSE );
}

SdrUndoAction* OReportUndoFactory::CreateUndoInsertObject( SdrObject& rObject, bool /*bOrdNumDirect*/ )
{
    return lcl_createUndo(rObject,rptui::Inserted,RID_STR_UNDO_INSERT_CONTROL);
}

SdrUndoAction* OReportUndoFactory::CreateUndoDeleteObject( SdrObject& rObject, bool /*bOrdNumDirect*/ )
{
    return lcl_createUndo(rObject,rptui::Removed,RID_STR_UNDO_DELETE_CONTROL);
    //return m_pUndoFactory->CreateUndoDeleteObject( rObject, bOrdNumDirect ? TRUE : FALSE );
}

SdrUndoAction* OReportUndoFactory::CreateUndoNewObject( SdrObject& rObject, bool /*bOrdNumDirect*/ )
{
    return lcl_createUndo(rObject,rptui::Inserted,RID_STR_UNDO_INSERT_CONTROL);
}

SdrUndoAction* OReportUndoFactory::CreateUndoCopyObject( SdrObject& rObject, bool bOrdNumDirect )
{
    return m_pUndoFactory->CreateUndoCopyObject( rObject, bOrdNumDirect ? TRUE : FALSE );
}

SdrUndoAction* OReportUndoFactory::CreateUndoObjectOrdNum( SdrObject& rObject, sal_uInt32 nOldOrdNum1, sal_uInt32 nNewOrdNum1)
{
    return m_pUndoFactory->CreateUndoObjectOrdNum( rObject, nOldOrdNum1, nNewOrdNum1 );
}

SdrUndoAction* OReportUndoFactory::CreateUndoReplaceObject( SdrObject& rOldObject, SdrObject& rNewObject, bool bOrdNumDirect )
{
    return m_pUndoFactory->CreateUndoReplaceObject( rOldObject, rNewObject, bOrdNumDirect ? TRUE : FALSE );
}

SdrUndoAction* OReportUndoFactory::CreateUndoObjectLayerChange( SdrObject& rObject, SdrLayerID aOldLayer, SdrLayerID aNewLayer )
{
    return m_pUndoFactory->CreateUndoObjectLayerChange( rObject, aOldLayer, aNewLayer );
}

SdrUndoAction* OReportUndoFactory::CreateUndoObjectSetText( SdrObject& rNewObj, sal_Int32 nText )
{
    return m_pUndoFactory->CreateUndoObjectSetText( rNewObj, nText );
}

// layer
SdrUndoAction* OReportUndoFactory::CreateUndoNewLayer(sal_uInt16 nLayerNum, SdrLayerAdmin& rNewLayerAdmin, SdrModel& rNewModel)
{
    return m_pUndoFactory->CreateUndoNewLayer( nLayerNum, rNewLayerAdmin, rNewModel );
}

SdrUndoAction* OReportUndoFactory::CreateUndoDeleteLayer(sal_uInt16 nLayerNum, SdrLayerAdmin& rNewLayerAdmin, SdrModel& rNewModel)
{
    return m_pUndoFactory->CreateUndoDeleteLayer( nLayerNum, rNewLayerAdmin, rNewModel );
}

SdrUndoAction* OReportUndoFactory::CreateUndoMoveLayer(sal_uInt16 nLayerNum, SdrLayerAdmin& rNewLayerAdmin, SdrModel& rNewModel, sal_uInt16 nNeuPos1)
{
    return m_pUndoFactory->CreateUndoMoveLayer( nLayerNum, rNewLayerAdmin, rNewModel, nNeuPos1 );
}

// page
SdrUndoAction*  OReportUndoFactory::CreateUndoDeletePage(SdrPage& rPage)
{
    return m_pUndoFactory->CreateUndoDeletePage( rPage );
}

SdrUndoAction* OReportUndoFactory::CreateUndoNewPage(SdrPage& rPage)
{
    return m_pUndoFactory->CreateUndoNewPage( rPage );
}

SdrUndoAction* OReportUndoFactory::CreateUndoCopyPage(SdrPage& rPage)
{
    return m_pUndoFactory->CreateUndoCopyPage( rPage );
}

SdrUndoAction* OReportUndoFactory::CreateUndoSetPageNum(SdrPage& rNewPg, sal_uInt16 nOldPageNum1, sal_uInt16 nNewPageNum1)
{
    return m_pUndoFactory->CreateUndoSetPageNum( rNewPg, nOldPageNum1, nNewPageNum1 );
}
    // master page
SdrUndoAction* OReportUndoFactory::CreateUndoPageRemoveMasterPage(SdrPage& rChangedPage)
{
    return m_pUndoFactory->CreateUndoPageRemoveMasterPage( rChangedPage );
}

SdrUndoAction* OReportUndoFactory::CreateUndoPageChangeMasterPage(SdrPage& rChangedPage)
{
    return m_pUndoFactory->CreateUndoPageChangeMasterPage(rChangedPage);
}

//==================================================================
}   //rptui
//==================================================================

