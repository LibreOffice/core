
#include "SdUnoPresView.hxx"

#ifndef _COM_SUN_STAR_BEANS_PROPERTYATTRIBUTE_HPP_
#include <com/sun/star/beans/PropertyAttribute.hpp>
#endif

#ifndef _VOS_MUTEX_HXX_
#include <vos/mutex.hxx>
#endif

#ifndef _SV_SVAPP_HXX
#include <vcl/svapp.hxx>
#endif

using namespace ::rtl;
using namespace ::vos;
using namespace ::cppu;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;

#include "unohelp.hxx"
#include "presvish.hxx"
#include "prvwshll.hxx"

//----------------------------------------------------------------------
//------ SdUnoPresView--------------------------------------------------
//----------------------------------------------------------------------

SdUnoPresView::SdUnoPresView(SdView* pSdView, SdDrawViewShell* pSdViewSh) throw()
:   SdUnoDrawView( pSdView, pSdViewSh )
{
}

SdUnoPresView::~SdUnoPresView() throw()
{
}

//----------------------------------------------------------------------
//------ XInterface ----------------------------------------------------
//----------------------------------------------------------------------

Any SAL_CALL SdUnoPresView::queryInterface( const Type & rType )
    throw(RuntimeException)
{
    Any aAny;

    QUERYINT( drawing::XDrawView );
    else QUERYINT( lang::XServiceInfo );
    else QUERYINT( beans::XPropertySet );
    else QUERYINT( lang::XComponent );
    else
        return SfxBaseController::queryInterface(rType);

    return aAny;
}

//----------------------------------------------------------------------
//------ XTypeProvider -------------------------------------------------
//----------------------------------------------------------------------

Sequence< Type > SAL_CALL SdUnoPresView::getTypes()
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

            const sal_Int32 nOwnTypes = 4;      // !DANGER! Keep this updated!

            aTypeSequence.realloc(  nBaseTypes + nOwnTypes );
            Type* pTypes = aTypeSequence.getArray();

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

Sequence< sal_Int8 > SAL_CALL SdUnoPresView::getImplementationId()
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
//------ XServiceInfo --------------------------------------------------
//----------------------------------------------------------------------


OUString SAL_CALL SdUnoPresView::getImplementationName(  ) throw(RuntimeException)
{
    return OUString( RTL_CONSTASCII_USTRINGPARAM( "SdUnoPresView" ) );
}

//----------------------------------------------------------------------
//------ The Properties of this implementation -------------------------
//----------------------------------------------------------------------

// Id must be the index into the array
enum properties
{
    PROPERTY_CURRENTPAGE = 0,
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
                beans::Property( OUString( RTL_CONSTASCII_USTRINGPARAM("CurrentPage") ),        PROPERTY_CURRENTPAGE,   ::getCppuType((const Reference< drawing::XDrawPage > *)0), beans::PropertyAttribute::BOUND ),
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
IPropertyArrayHelper & SdUnoPresView::getInfoHelper()
{
    OGuard aGuard( Application::GetSolarMutex() );

    static OPropertyArrayHelper aInfo( getBasicProps(), PROPERTY_COUNT );
    return aInfo;
}

//----------------------------------------------------------------------

Reference < beans::XPropertySetInfo >  SdUnoPresView::getPropertySetInfo()
{
    OGuard aGuard( Application::GetSolarMutex() );

    static Reference < beans::XPropertySetInfo >  xInfo( createPropertySetInfo( getInfoHelper() ) );
    return xInfo;
}

