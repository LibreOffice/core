#include "ChartController.hxx"

#include "dlg_PositionAndSize.hxx"
#include "dlg_RotateDiagram.hxx"
#include "macros.hxx"
#include "ChartWindow.hxx"
#include "Rotation.hxx"
#include "ChartModelHelper.hxx"
#include "DrawViewWrapper.hxx"
#include "PositionAndSizeHelper.hxx"

#ifndef _DRAFTS_COM_SUN_STAR_CHART2_SCENEDESCRIPTOR_HPP_
#include <drafts/com/sun/star/chart2/SceneDescriptor.hpp>
#endif

// header for define RET_OK
#ifndef _SV_MSGBOX_HXX
#include <vcl/msgbox.hxx>
#endif

#ifndef _SVX_SVXIDS_HRC
#include <svx/svxids.hrc>
#endif

//.............................................................................
namespace chart
{
//.............................................................................
using namespace ::com::sun::star;
using namespace ::drafts::com::sun::star::chart2;

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

void lcl_getPositionAndSizeFromItemSet( const SfxItemSet& rItemSet, Rectangle& rPosAndSize )
{
    long nPosX(0);
    long nPosY(0);
    long nSizX(0);
    long nSizY(0);

    const SfxPoolItem* pPoolItem=NULL;
    //read position
    if (SFX_ITEM_SET==rItemSet.GetItemState(SID_ATTR_TRANSFORM_POS_X,TRUE,&pPoolItem))
        nPosX=((const SfxInt32Item*)pPoolItem)->GetValue();
    if (SFX_ITEM_SET==rItemSet.GetItemState(SID_ATTR_TRANSFORM_POS_Y,TRUE,&pPoolItem))
        nPosY=((const SfxInt32Item*)pPoolItem)->GetValue();
    //read size
    if (SFX_ITEM_SET==rItemSet.GetItemState(SID_ATTR_TRANSFORM_WIDTH,TRUE,&pPoolItem))
        nSizX=((const SfxUInt32Item*)pPoolItem)->GetValue();
    if (SFX_ITEM_SET==rItemSet.GetItemState(SID_ATTR_TRANSFORM_HEIGHT,TRUE,&pPoolItem))
        nSizY=((const SfxUInt32Item*)pPoolItem)->GetValue();

    rPosAndSize = Rectangle(Point(nPosX,nPosY),Size(nSizX,nSizY));
}

void SAL_CALL ChartController::executeDispatch_PositionAndSize( const ::rtl::OUString& rObjectCID )
{
    if( !rObjectCID.getLength() )
        return;

    bool bChanged = false;
    try
    {
        SfxItemSet aItemSet = m_pDrawViewWrapper->getPositionAndSizeItemSetFromMarkedObject();

        //prepare and open dialog
        Window* pParent( NULL );
        SdrView* pSdrView = m_pDrawViewWrapper;
        bool bResizePossible=true;
        PositionAndSizeDialog aDlg( pParent, &aItemSet, pSdrView, bResizePossible );
        if( aDlg.Execute() == RET_OK )
        {
            const SfxItemSet* pOutItemSet = aDlg.GetOutputItemSet();
            if(pOutItemSet)
            {
                Rectangle aObjectRect;
                aItemSet.Put(*pOutItemSet);//overwrite old values with new values (-> all items are set)
                lcl_getPositionAndSizeFromItemSet( aItemSet, aObjectRect );
                Rectangle aPageRect( Point(0,0),m_pChartWindow->GetOutputSize() );

                bChanged = PositionAndSizeHelper::moveObject( m_aSelectedObjectCID
                            , m_aModel->getModel()
                            , awt::Rectangle(aObjectRect.getX(),aObjectRect.getY(),aObjectRect.getWidth(),aObjectRect.getHeight())
                            , awt::Rectangle(aPageRect.getX(),aPageRect.getY(),aPageRect.getWidth(),aPageRect.getHeight())
                            );
            }
        }
    }
    catch( uno::RuntimeException& e)
    {
        ASSERT_EXCEPTION( e );
    }
    //make sure that all objects using  m_pChartView are already deleted
    if(bChanged)
    {
        m_bViewDirty = true;
        m_pChartWindow->Invalidate();
    }
}

void SAL_CALL ChartController::executeDispatch_RotateDiagram()
{
    bool bChanged = false;
    try
    {
        Window* pParent( NULL );
        double fXAngle, fYAngle, fZAngle;
        fXAngle=fYAngle=fZAngle=0.0;

        //get angle from model
        SceneDescriptor aSceneDescriptor;
        uno::Reference< beans::XPropertySet > xProp( ChartModelHelper::findDiagram( m_aModel->getModel() ), uno::UNO_QUERY );
        if( xProp.is() && (xProp->getPropertyValue( C2U( "SceneProperties" ) )>>=aSceneDescriptor) )
        {
            Rotation::getEulerFromAxisAngleRepresentation( aSceneDescriptor.aDirection, aSceneDescriptor.fRotationAngle
                , fXAngle, fYAngle, fZAngle );
        }

        //open dialog
        RotateDiagramDialog aDlg( pParent, fXAngle, fYAngle, fZAngle );
        if( aDlg.Execute() == RET_OK )
        {
            aDlg.getAngle( fXAngle, fYAngle, fZAngle );
            Rotation::getAxisAngleFromEulerRepresentation( aSceneDescriptor.aDirection, aSceneDescriptor.fRotationAngle
                , fXAngle, fYAngle, fZAngle );
            xProp->setPropertyValue( C2U( "SceneProperties" ), uno::makeAny(aSceneDescriptor) );
            bChanged = true;
        }
    }
    catch( uno::RuntimeException& e)
    {
        ASSERT_EXCEPTION( e );
    }
    //make sure that all objects using  m_pChartView are already deleted
    if(bChanged)
    {
        m_bViewDirty = true;
        m_pChartWindow->Invalidate();
    }
}

//.............................................................................
} //namespace chart
//.............................................................................
