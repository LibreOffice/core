
#include "SdUnoDrawView.hxx"

#ifndef _COM_SUN_STAR_BEANS_PROPERTYATTRIBUTE_HPP_
#include <com/sun/star/beans/PropertyAttribute.hpp>
#endif

#ifndef _SVX_UNOSHAPE_HXX
#include <svx/unoshape.hxx>
#endif

#include <svx/svdobj.hxx>
#include <svx/svdpagv.hxx>
#include <svx/unoshape.hxx>
#include <svx/unoshcol.hxx>

#include "unohelp.hxx"
#include "unopage.hxx"
#include "unomodel.hxx"
#include "sdview.hxx"
#include "drviewsh.hxx"
#include "drawdoc.hxx"
#include "docshell.hxx"
#include "grviewsh.hxx"
#include "presvish.hxx"
#include "prvwshll.hxx"

using namespace ::rtl;
using namespace ::vos;
using namespace ::cppu;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;

//----------------------------------------------------------------------
//------ SdUnoDrawView--------------------------------------------------
//----------------------------------------------------------------------

SdUnoDrawView::SdUnoDrawView(SdView* pSdView, SdDrawViewShell* pSdViewSh) throw()
:   SdUnoDrawViewBase(),
    SfxBaseController((SfxViewShell*)pSdViewSh),
    OBroadcastHelper( ((SdUnoDrawViewBase *)this)->aMutex ),
    OPropertySetHelper( *static_cast<OBroadcastHelperVar<OMultiTypeInterfaceContainerHelper, OMultiTypeInterfaceContainerHelper::keyType> *>(this)),
    mpView(pSdView),
    mpViewSh(pSdViewSh),
    mbDisposing(sal_False),
    mbOldMasterPageMode(sal_False),
    mbOldLayerMode(sal_False)
{
    if( pSdViewSh->ISA( SdGraphicViewShell ) )
    {
        meKind = SdUnoDrawView::drawing;
    }
    else if( pSdViewSh->ISA( SdPresViewShell ) )
    {
        meKind = SdUnoDrawView::slideshow;
    }
    else if( pSdViewSh->ISA( SdPreviewViewShell ) )
    {
        meKind = SdUnoDrawView::preview;
    }
    else
    {
        switch( pSdViewSh->GetPageKind() )
        {
        case PK_NOTES:
            meKind = SdUnoDrawView::notes;
            break;
        case PK_HANDOUT:
            meKind = SdUnoDrawView::handout;
            break;
        default:
            meKind = SdUnoDrawView::presentation;
            break;
        }
    }
}

SdUnoDrawView::~SdUnoDrawView() throw()
{
}

sal_Bool SdUnoDrawView::getMasterPageMode(void) const throw()
{
    return mpViewSh && mpViewSh->GetEditMode() == EM_MASTERPAGE;
}

//----------------------------------------------------------------------

void SdUnoDrawView::setMasterPageMode(sal_Bool MasterPageMode_) throw()
{
    if(mpViewSh && ((mpViewSh->GetEditMode() == EM_MASTERPAGE) != MasterPageMode_))
        mpViewSh->ChangeEditMode(MasterPageMode_?EM_MASTERPAGE:EM_PAGE,mpViewSh->GetLayerMode());
}

//----------------------------------------------------------------------

sal_Bool SdUnoDrawView::getLayerMode(void) const throw()
{
    return mpViewSh && mpViewSh->GetLayerMode();
}

//----------------------------------------------------------------------

void SdUnoDrawView::setLayerMode(sal_Bool LayerMode_) throw()
{
    if(mpViewSh && (mpViewSh->GetLayerMode() != LayerMode_))
        mpViewSh->ChangeEditMode(mpViewSh->GetEditMode(),LayerMode_);
}

//----------------------------------------------------------------------

SdXImpressDocument* SdUnoDrawView::getModel() const throw()
{
    if(mpView && mpView->GetDocSh())
    {
        Reference< frame::XModel > xModel( mpView->GetDocSh()->GetModel() );
        return SdXImpressDocument::getImplementation( xModel );
    }
    else
        return NULL;
}

//----------------------------------------------------------------------
//------ XInterface ----------------------------------------------------
//----------------------------------------------------------------------

Any SAL_CALL SdUnoDrawView::queryInterface( const Type & rType )
    throw(RuntimeException)
{
    Any aAny;

    QUERYINT( view::XSelectionSupplier );
    else QUERYINT( drawing::XDrawView );
    else QUERYINT( lang::XServiceInfo );
    else QUERYINT( beans::XPropertySet );
    else QUERYINT( lang::XComponent );
    else
        return SfxBaseController::queryInterface(rType);

    return aAny;
}

//----------------------------------------------------------------------

void SAL_CALL SdUnoDrawView::acquire()
    throw ( )
{
    SfxBaseController::acquire();
}

//----------------------------------------------------------------------

void SAL_CALL SdUnoDrawView::release()
    throw ( )
{
    SfxBaseController::release();
}

//----------------------------------------------------------------------
//------ XTypeProvider -------------------------------------------------
//----------------------------------------------------------------------

Sequence< Type > SAL_CALL SdUnoDrawView::getTypes()
    throw(RuntimeException)
{
    static Sequence< Type > aTypeSequence;
    if( 0 == aTypeSequence.getLength() )
    {
        ::osl::MutexGuard guard( ::osl::Mutex::getGlobalMutex() );
        if( 0 == aTypeSequence.getLength() )
        {
            const Sequence< Type > aBaseTypes( SfxBaseController::getTypes() );
            const sal_Int32 nBaseTypes = aBaseTypes.getLength();
            const Type* pBaseTypes = aBaseTypes.getConstArray();

            const sal_Int32 nOwnTypes = 5;      // !DANGER! Keep this updated!

            aTypeSequence.realloc(  nBaseTypes + nOwnTypes );
            Type* pTypes = aTypeSequence.getArray();

            *pTypes++ = ITYPE(view::XSelectionSupplier);
            *pTypes++ = ITYPE(drawing::XDrawView);
            *pTypes++ = ITYPE(lang::XServiceInfo);
            *pTypes++ = ITYPE(beans::XPropertySet);
            *pTypes++ = ITYPE(lang::XComponent);

            for( sal_Int32 nType = 0; nType < nBaseTypes; nType++ )
                *pTypes++ = *pBaseTypes++;
        }
    }

    return aTypeSequence;
}

//----------------------------------------------------------------------

Sequence< sal_Int8 > SAL_CALL SdUnoDrawView::getImplementationId()
    throw(RuntimeException)
{
    static Sequence< sal_Int8 > aId;
    if( aId.getLength() == 0 )
    {
        aId.realloc( 16 );
        rtl_createUuid( (sal_uInt8 *)aId.getArray(), 0, sal_True );
    }
    return aId;
}

//----------------------------------------------------------------------
//------ XComponent ----------------------------------------------------
//----------------------------------------------------------------------

void SAL_CALL SdUnoDrawView::dispose()
    throw( RuntimeException )
{
    if( !mbDisposing )
    {
        OGuard aGuard( Application::GetSolarMutex() );

        if( !mbDisposing )
        {
            mbDisposing = sal_True;
            mpView = NULL,
            mpViewSh = NULL;

            SfxBaseController::dispose();
        }
    }
}

void SAL_CALL SdUnoDrawView::addEventListener( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XEventListener >& xListener ) throw (::com::sun::star::uno::RuntimeException)
{
    SfxBaseController::addEventListener( xListener );
}

void SAL_CALL SdUnoDrawView::removeEventListener( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XEventListener >& aListener ) throw (::com::sun::star::uno::RuntimeException)
{
    SfxBaseController::removeEventListener( aListener );
}

//----------------------------------------------------------------------
//------ XServiceInfo --------------------------------------------------
//----------------------------------------------------------------------

sal_Char pImplSdUnoDrawViewService[sizeof("com.sun.star.drawing.DrawingDocumentDrawView")] = "com.sun.star.drawing.DrawingDocumentDrawView";

OUString SAL_CALL SdUnoDrawView::getImplementationName(  ) throw(RuntimeException)
{
    return OUString( RTL_CONSTASCII_USTRINGPARAM( "SdUnoDrawView" ) );
}

//----------------------------------------------------------------------

static const char* pImplSdUnoDrawViewServices[6] =
{
    "com.sun.star.presentation.PresentationDocumentView",
    "com.sun.star.drawing.DrawingDocumentDrawView",
    "com.sun.star.presentation.SlideShowView",
    "com.sun.star.presentation.DrawingDocumentPreviewView",
    "com.sun.star.presentation.NotesDocumentPreviewView",
    "com.sun.star.presentation.HandoutDocumentPreviewView"
};

sal_Bool SAL_CALL SdUnoDrawView::supportsService( const OUString& ServiceName ) throw(RuntimeException)
{
    const char* pService = pImplSdUnoDrawViewServices[meKind];
    return ServiceName.equalsAscii( pService );
}

//----------------------------------------------------------------------

Sequence< OUString > SAL_CALL SdUnoDrawView::getSupportedServiceNames(  ) throw(RuntimeException)
{
    const char* pService = pImplSdUnoDrawViewServices[meKind];
    OUString aService( OUString::createFromAscii( pService ) );
    Sequence< OUString > aSeq( &aService, 1 );
    return aSeq;
}

//----------------------------------------------------------------------
//------ XSelectionSupplier --------------------------------------------
//----------------------------------------------------------------------

sal_Bool SAL_CALL SdUnoDrawView::select( const Any& aSelection )
    throw(lang::IllegalArgumentException, RuntimeException)
{
    OGuard aGuard( Application::GetSolarMutex() );

    SdrPageView *pPV = mpView->GetPageViewPvNum(0);

    if(pPV == NULL)
        return sal_False;

    // first deselect all
    mpView->UnmarkAllObj( pPV );

    // if the any is empty, just deselect all
    if( !aSelection.hasValue() )
        return sal_True;

    Reference< drawing::XShape > xShape;
    aSelection >>= xShape;

    // if the any is a shape, select it
    if(xShape.is())
    {
        SvxShape* pShape = SvxShape::getImplementation( xShape );
        if( pShape && pShape->GetSdrObject() )
        {
            SdrObject *pObj = pShape->GetSdrObject();
            if(pObj && pObj->GetPage() == pPV->GetPage())
            {
                mpView->MarkObj( pObj, pPV );
                return sal_True;
            }
        }
    }
    // else it must be a XShapes collection
    else
    {
        Reference< drawing::XShapes > xShapes;
        aSelection >>= xShapes;
        if( xShapes.is() )
        {
            const sal_uInt32 nCount = xShapes->getCount();
            for( sal_uInt32 i = 0; i < nCount; i++ )
            {
                xShapes->getByIndex(i) >>= xShape;
                if( xShape.is() )
                {
                    SvxShape* pShape = SvxShape::getImplementation(xShape);
                    if( pShape )
                    {
                        SdrObject *pObj = pShape->GetSdrObject();
                        if(pObj && pObj->GetPage() == pPV->GetPage())
                        {
                            mpView->MarkObj( pObj, pPV );
                            return sal_True;
                        }
                    }
                }
            }
        }
    }

    // todo: add selections for text ranges
    return sal_False;
}

//----------------------------------------------------------------------

Any SAL_CALL SdUnoDrawView::getSelection()
    throw(RuntimeException)
{
    OGuard aGuard( Application::GetSolarMutex() );

    SdXImpressDocument* pModel = getModel();

    Reference< drawing::XShapes > xShapes( SvxShapeCollection_NewInstance(), UNO_QUERY );

    if(mpView)
    {
        const SdrMarkList& rMarkList = mpView->GetMarkList();
        sal_uInt32 nCount = rMarkList.GetMarkCount();
        for( sal_uInt32 nNum = 0; nNum < nCount; nNum++)
        {
            SdrMark *pMark = rMarkList.GetMark(nNum);
            if(pMark==NULL)
                continue;

            SdrObject *pObj = pMark->GetObj();
            if(pObj==NULL || pObj->GetPage() == NULL)
                continue;

            Reference< drawing::XDrawPage > xPage( pObj->GetPage()->getUnoPage(), UNO_QUERY);

            if(!xPage.is())
                continue;

            SvxDrawPage* pDrawPage = SvxDrawPage::getImplementation( xPage );

            if(pDrawPage==NULL)
                continue;

            Reference< drawing::XShape > xShape( pObj->getUnoShape(), UNO_QUERY );

            if(xShape.is())
                xShapes->add(xShape);
        }
    }

    return Any( &xShapes, ITYPE(drawing::XShapes) );
}

//----------------------------------------------------------------------

inline const ::com::sun::star::uno::Type & getSelectionTypeIdentifier()
{
    return ::getCppuType( (Reference< view::XSelectionChangeListener > *)0 );
}

//----------------------------------------------------------------------

void SAL_CALL SdUnoDrawView::addSelectionChangeListener( const Reference< view::XSelectionChangeListener >& xListener )
    throw(RuntimeException)
{
    addListener( getSelectionTypeIdentifier(), xListener );
}

//----------------------------------------------------------------------

void SAL_CALL SdUnoDrawView::removeSelectionChangeListener( const Reference< view::XSelectionChangeListener >& xListener ) throw(RuntimeException)
{
    removeListener( getSelectionTypeIdentifier( ), xListener );
}

//----------------------------------------------------------------------

void SdUnoDrawView::fireSelectionChangeListener() throw()
{
    OInterfaceContainerHelper * pLC = getContainer( getSelectionTypeIdentifier() );
    if( pLC )
    {
        Reference< XInterface > xSource( (XWeak*)this );
        const lang::EventObject aEvent( xSource );

        // Ueber alle Listener iterieren und Events senden
        OInterfaceIteratorHelper aIt( *pLC);
        while( aIt.hasMoreElements() )
        {
            view::XSelectionChangeListener * pL = (view::XSelectionChangeListener *)aIt.next();
            pL->selectionChanged( aEvent );
        }
    }
}

//----------------------------------------------------------------------
//------ The Properties of this implementation -------------------------
//----------------------------------------------------------------------

// Id must be the index into the array
enum properties
{
    PROPERTY_MASTERPAGEMODE = 0,
    PROPERTY_LAYERMODE,
    PROPERTY_WORKAREA,

    PROPERTY_COUNT
};

/**
 * All Properties of this implementation. Must be sorted by name.
 */
static beans::Property * getBasicProps()
{
    static beans::Property *pTable = 0;

    if( ! pTable )
    {
        ::osl::MutexGuard guard( ::osl::Mutex::getGlobalMutex() );
        if( ! pTable )
        {

            static beans::Property aBasicProps[PROPERTY_COUNT] =
            {
                beans::Property( OUString( RTL_CONSTASCII_USTRINGPARAM("IsLayerMode") ),        PROPERTY_LAYERMODE,      ::getCppuBooleanType(),    beans::PropertyAttribute::BOUND ),
                beans::Property( OUString( RTL_CONSTASCII_USTRINGPARAM("IsMasterPageMode") ),   PROPERTY_MASTERPAGEMODE,     ::getCppuBooleanType(),    beans::PropertyAttribute::BOUND ),
                beans::Property( OUString( RTL_CONSTASCII_USTRINGPARAM("VisibleArea") ),        PROPERTY_WORKAREA,          ::getCppuType((const ::com::sun::star::awt::Rectangle*)0), beans::PropertyAttribute::BOUND | beans::PropertyAttribute::READONLY )
            };
            pTable = aBasicProps;
        }
    }
    return pTable;
}

//----------------------------------------------------------------------
//------ XPropertySet & OPropertySetHelper -----------------------------
//----------------------------------------------------------------------

/**
 * Create a table that map names to index values.
 */
IPropertyArrayHelper & SdUnoDrawView::getInfoHelper()
{
    OGuard aGuard( Application::GetSolarMutex() );

    static OPropertyArrayHelper aInfo( getBasicProps(), PROPERTY_COUNT );
    return aInfo;
}

//----------------------------------------------------------------------

Reference < beans::XPropertySetInfo >  SdUnoDrawView::getPropertySetInfo()
{
    OGuard aGuard( Application::GetSolarMutex() );

    static Reference < beans::XPropertySetInfo >  xInfo( createPropertySetInfo( getInfoHelper() ) );
    return xInfo;
}

//----------------------------------------------------------------------

// Return sal_True, value changed
sal_Bool SdUnoDrawView::convertFastPropertyValue
(
    Any & rConvertedValue,
    Any & rOldValue,
    sal_Int32 nHandle,
    const Any& rValue
)
{
    switch( nHandle )
    {
        case PROPERTY_MASTERPAGEMODE:
            {
            sal_Bool bOldValue = getMasterPageMode();
            sal_Bool b;
            convertPropertyValue( b , rValue );
            if( b != bOldValue )
            {

                rConvertedValue.setValue( &b , ::getCppuBooleanType()  );
                rOldValue.setValue( & bOldValue , ::getCppuBooleanType() );
                return sal_True;
            }
            else
                return sal_False;
            }

        case PROPERTY_LAYERMODE:
            {
            sal_Bool bOldValue = getLayerMode();
            sal_Bool b;
            convertPropertyValue( b , rValue );
            if( b != bOldValue )
            {
                rConvertedValue.setValue( &b , ::getCppuBooleanType()  );
                rOldValue.setValue( & bOldValue , ::getCppuBooleanType() );
                return sal_True;
            }
            else
                return sal_False;
            }
        default:
            return sal_False;
    }
}

//----------------------------------------------------------------------

/**
 * only set the value.
 */
void SdUnoDrawView::setFastPropertyValue_NoBroadcast
(
    sal_Int32 nHandle,
    const Any& rValue
)
{
    switch( nHandle )
    {
        case PROPERTY_MASTERPAGEMODE:
            {
                sal_Bool bValue;
                rValue >>= bValue;
                setMasterPageMode( bValue );
            }
            break;

        case PROPERTY_LAYERMODE:
            {
                sal_Bool bValue;
                rValue >>= bValue;
                setLayerMode( bValue );
            }
            break;
    }
}

//----------------------------------------------------------------------

void SdUnoDrawView::getFastPropertyValue( Any & rRet, sal_Int32 nHandle ) const
{
    switch( nHandle )
    {
        case PROPERTY_MASTERPAGEMODE:
            rRet <<= getMasterPageMode();
        break;

        case PROPERTY_LAYERMODE:
            rRet <<= getLayerMode();
        break;
        case PROPERTY_WORKAREA:
            rRet <<= awt::Rectangle( maLastVisArea.Left(), maLastVisArea.Top(), maLastVisArea.GetWidth(), maLastVisArea.GetHeight() );
            break;
    }
}

//----------------------------------------------------------------------

void SdUnoDrawView::fireChangeEditMode( sal_Bool bMasterPageMode ) throw()
{
    if( bMasterPageMode != mbOldMasterPageMode )
    {
        sal_Int32 nHandles = PROPERTY_MASTERPAGEMODE;
        Any aNewValue;
        aNewValue <<= bMasterPageMode;
        Any aOldValue;
        aOldValue <<= mbOldMasterPageMode;

        fire( &nHandles, &aNewValue, &aOldValue, 1, sal_False );

        mbOldMasterPageMode = bMasterPageMode;
    }
}

//----------------------------------------------------------------------

void SdUnoDrawView::fireChangeLayerMode( sal_Bool bLayerMode ) throw()
{
    if( bLayerMode != mbOldLayerMode )
    {
        sal_Int32 nHandles = PROPERTY_LAYERMODE;
        Any aNewValue;
        aNewValue <<= bLayerMode;
        Any aOldValue;
        aOldValue <<= mbOldLayerMode;

        fire( &nHandles, &aNewValue, &aOldValue, 1, sal_False );

        mbOldLayerMode = bLayerMode;
    }
}

//----------------------------------------------------------------------
//------ XDrawView -----------------------------------------------------
//----------------------------------------------------------------------

void SAL_CALL SdUnoDrawView::setCurrentPage( const Reference< drawing::XDrawPage >& xPage )
    throw(RuntimeException)
{
    OGuard aGuard( Application::GetSolarMutex() );

    if(mpView)
    {
        SvxDrawPage* pDrawPage = SvxDrawPage::getImplementation( xPage );
        SdrPage *pSdrPage = pDrawPage ? pDrawPage->GetSdrPage() : NULL;

        if(pSdrPage)
        {
            setMasterPageMode( pSdrPage->IsMasterPage() );
            mpViewSh->SwitchPage( (pSdrPage->GetPageNum() - 1) >> 1 );
        }
    }
}

//----------------------------------------------------------------------

Reference< drawing::XDrawPage > SAL_CALL SdUnoDrawView::getCurrentPage()
    throw(RuntimeException)
{
    OGuard aGuard( Application::GetSolarMutex() );

    Reference< drawing::XDrawPage >  xPage;

    if(mpView)
    {
        SdXImpressDocument* pModel = getModel();
        SdrPageView *pPV = mpView->GetPageViewPvNum(0);
        SdrPage* pPage = pPV ? pPV->GetPage() : NULL;

        if(pPage)
            xPage = Reference< drawing::XDrawPage >::query( pPage->getUnoPage() );
    }

    return xPage;
}

//----------------------------------------------------------------------

void SdUnoDrawView::fireVisAreaChanged( const Rectangle& rVisArea ) throw()
{
    if( maLastVisArea != rVisArea )
    {
        sal_Int32 nHandles = PROPERTY_WORKAREA;
        Any aNewValue;
        aNewValue <<= awt::Rectangle( rVisArea.Left(), rVisArea.Top(), rVisArea.GetWidth(), rVisArea.GetHeight() );

        Any aOldValue;
        aOldValue <<= awt::Rectangle( maLastVisArea.Left(), maLastVisArea.Top(), maLastVisArea.GetWidth(), maLastVisArea.GetHeight() );

        fire( &nHandles, &aNewValue, &aOldValue, 1, sal_False );

        maLastVisArea = rVisArea;
    }
}
