#include "ChartController.hxx"

#include "dlg_RotateDiagram.hxx"
#include "macros.hxx"
#include "ChartWindow.hxx"
#include "Rotation.hxx"
#include "ChartModelHelper.hxx"

#ifndef _DRAFTS_COM_SUN_STAR_CHART2_SCENEDESCRIPTOR_HPP_
#include <drafts/com/sun/star/chart2/SceneDescriptor.hpp>
#endif

// header for define RET_OK
#ifndef _SV_MSGBOX_HXX
#include <vcl/msgbox.hxx>
#endif

//.............................................................................
namespace chart
{
//.............................................................................
using namespace ::com::sun::star;
using namespace ::drafts::com::sun::star::chart2;

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

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
