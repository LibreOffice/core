#ifndef _OSL_FILE_HXX_
#include <osl/file.hxx>
#endif
#ifndef _VOS_MODULE_HXX_
#include <vos/module.hxx>
#endif

#ifndef _COM_SUN_STAR_FRAME_XMODEL_HPP_
#include <com/sun/star/frame/XModel.hpp>
#endif
#ifndef _COM_SUN_STAR_DOCUMENT_XVIEWDATASUPPLIER_HPP_
#include <com/sun/star/document/XViewDataSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XINDEXACCESS_HPP_
#include <com/sun/star/container/XIndexAccess.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XSERVICEINFO_HPP_
#include <com/sun/star/lang/XServiceInfo.hpp>
#endif
#ifndef _COM_SUN_STAR_UNO_SEQUENCE_H_
#include <com/sun/star/uno/Sequence.h>
#endif
#ifndef _COM_SUN_STAR_UNO_ANY_H_
#include <com/sun/star/uno/Any.h>
#endif
#ifndef _COM_SUN_STAR_LANG_XINITIALIZATION_HPP_
#include <com/sun/star/lang/XInitialization.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XSERVICEINFO_HPP_
#include <com/sun/star/lang/XServiceInfo.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_XPROPERTYACCESS_HPP_
#include <com/sun/star/beans/XPropertyAccess.hpp>
#endif
#ifndef _COM_SUN_STAR_UI_DIALOGS_XEXECUTABLEDIALOG_HPP_
#include <com/sun/star/ui/dialogs/XExecutableDialog.hpp>
#endif
#ifndef _COM_SUN_STAR_UI_DIALOGS_EXECUTABLEDIALOGRESULTS_HPP_
#include <com/sun/star/ui/dialogs/ExecutableDialogResults.hpp>
#endif
#ifndef _COM_SUN_STAR_DOCUMENT_XEXPORTER_HPP_
#include <com/sun/star/document/XExporter.hpp>
#endif

#ifndef _CPPUHELPER_IMPLBASE5_HXX_
#include <cppuhelper/implbase5.hxx>
#endif

#ifndef _SV_SVAPP_HXX
#include <vcl/svapp.hxx>
#endif

using namespace rtl;
using namespace com::sun::star::uno;
using namespace com::sun::star::lang;
using namespace com::sun::star::document;
using namespace com::sun::star::beans;
using namespace com::sun::star::container;
using namespace com::sun::star::frame;
using namespace com::sun::star::ui::dialogs;

#include "pres.hxx"
#include "pubdlg.hxx"

class SdHtmlOptionsDialog : public cppu::WeakImplHelper5
<
    XExporter,
    XExecutableDialog,
    XPropertyAccess,
    XInitialization,
    XServiceInfo
>
{
    const Reference< XMultiServiceFactory > &mrxMgr;
    Sequence< PropertyValue > maMediaDescriptor;
    Sequence< PropertyValue > maFilterDataSequence;
    OUString aDialogTitle;
    DocumentType meDocType;

public:

    SdHtmlOptionsDialog( const Reference< XMultiServiceFactory >& _rxORB );
    ~SdHtmlOptionsDialog();

    // XInterface
    virtual void SAL_CALL acquire() throw();
    virtual void SAL_CALL release() throw();

    // XInitialization
    virtual void SAL_CALL initialize( const Sequence< Any > & aArguments ) throw ( Exception, RuntimeException );

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName() throw ( RuntimeException );
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) throw ( RuntimeException );
    virtual Sequence< OUString > SAL_CALL getSupportedServiceNames() throw ( RuntimeException );

    // XPropertyAccess
    virtual Sequence< PropertyValue > SAL_CALL getPropertyValues() throw ( RuntimeException );
    virtual void SAL_CALL setPropertyValues( const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue > & aProps )
        throw ( ::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::beans::PropertyVetoException,
                ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::lang::WrappedTargetException,
                ::com::sun::star::uno::RuntimeException );

    // XExecuteDialog
    virtual sal_Int16 SAL_CALL execute()
        throw ( com::sun::star::uno::RuntimeException );
    virtual void SAL_CALL setTitle( const ::rtl::OUString& aTitle )
        throw ( ::com::sun::star::uno::RuntimeException );

    // XExporter
    virtual void SAL_CALL setSourceDocument( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XComponent >& xDoc )
        throw ( ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException );

};

// -------------------------
// - SdHtmlOptionsDialog -
// -------------------------

Reference< XInterface >
    SAL_CALL SdHtmlOptionsDialog_CreateInstance(
        const Reference< XMultiServiceFactory > & _rxFactory )
{
    return static_cast< ::cppu::OWeakObject* > ( new SdHtmlOptionsDialog( _rxFactory ) );
}

OUString SdHtmlOptionsDialog_getImplementationName()
    throw( RuntimeException )
{
    return OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.comp.draw.SdHtmlOptionsDialog" ) );
}
#define SERVICE_NAME "com.sun.star.ui.dialog.FilterOptionsDialog"
sal_Bool SAL_CALL SdHtmlOptionsDialog_supportsService( const OUString& ServiceName )
    throw( RuntimeException )
{
    return ServiceName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( SERVICE_NAME ) );
}

Sequence< OUString > SAL_CALL SdHtmlOptionsDialog_getSupportedServiceNames()
    throw( RuntimeException )
{
    Sequence< OUString > aRet(1);
    OUString* pArray = aRet.getArray();
    pArray[0] = OUString( RTL_CONSTASCII_USTRINGPARAM( SERVICE_NAME ) );
    return aRet;
}
#undef SERVICE_NAME

// -----------------------------------------------------------------------------

SdHtmlOptionsDialog::SdHtmlOptionsDialog( const Reference< XMultiServiceFactory > & xMgr ) :
    mrxMgr      ( xMgr ),
    meDocType   ( DOCUMENT_TYPE_DRAW )
{
}

// -----------------------------------------------------------------------------

SdHtmlOptionsDialog::~SdHtmlOptionsDialog()
{
}

// -----------------------------------------------------------------------------

void SAL_CALL SdHtmlOptionsDialog::acquire() throw()
{
    OWeakObject::acquire();
}

// -----------------------------------------------------------------------------

void SAL_CALL SdHtmlOptionsDialog::release() throw()
{
    OWeakObject::release();
}

// XInitialization
void SAL_CALL SdHtmlOptionsDialog::initialize( const Sequence< Any > & aArguments )
    throw ( Exception, RuntimeException )
{
}

// XServiceInfo
OUString SAL_CALL SdHtmlOptionsDialog::getImplementationName()
    throw( RuntimeException )
{
    return SdHtmlOptionsDialog_getImplementationName();
}
sal_Bool SAL_CALL SdHtmlOptionsDialog::supportsService( const OUString& rServiceName )
    throw( RuntimeException )
{
    return SdHtmlOptionsDialog_supportsService( rServiceName );
}
Sequence< OUString > SAL_CALL SdHtmlOptionsDialog::getSupportedServiceNames()
    throw ( RuntimeException )
{
    return SdHtmlOptionsDialog_getSupportedServiceNames();
}


// XPropertyAccess
Sequence< PropertyValue > SdHtmlOptionsDialog::getPropertyValues()
        throw ( RuntimeException )
{
    sal_Int32 i, nCount;
    for ( i = 0, nCount = maMediaDescriptor.getLength(); i < nCount; i++ )
    {
        if ( maMediaDescriptor[ i ].Name.equalsAscii( "FilterData" ) )
            break;
    }
    if ( i == nCount )
        maMediaDescriptor.realloc( ++nCount );

    // the "FilterData" Property is an Any that will contain our PropertySequence of Values
    maMediaDescriptor[ i ].Name = OUString( RTL_CONSTASCII_USTRINGPARAM( "FilterData" ) );
    maMediaDescriptor[ i ].Value <<= maFilterDataSequence;
    return maMediaDescriptor;
}

void SdHtmlOptionsDialog::setPropertyValues( const Sequence< PropertyValue > & aProps )
        throw ( UnknownPropertyException, PropertyVetoException,
                IllegalArgumentException, WrappedTargetException,
                RuntimeException )
{
    maMediaDescriptor = aProps;

    sal_Int32 i, nCount;
    for ( i = 0, nCount = maMediaDescriptor.getLength(); i < nCount; i++ )
    {
        if ( maMediaDescriptor[ i ].Name.equalsAscii( "FilterData" ) )
        {
            maMediaDescriptor[ i ].Value >>= maFilterDataSequence;
            break;
        }
    }
}

// XExecutableDialog
void SdHtmlOptionsDialog::setTitle( const OUString& aTitle )
    throw ( RuntimeException )
{
    aDialogTitle = aTitle;
}

sal_Int16 SdHtmlOptionsDialog::execute()
    throw ( RuntimeException )
{
    SdPublishingDlg aDlg( Application::GetDefDialogParent(), meDocType );
    if( aDlg.Execute() )
    {
        aDlg.GetParameterSequence( maFilterDataSequence );
        return ExecutableDialogResults::OK;
    }
    else
    {
        return ExecutableDialogResults::CANCEL;
    }

}

// XEmporter
void SdHtmlOptionsDialog::setSourceDocument( const Reference< XComponent >& xDoc )
        throw ( IllegalArgumentException, RuntimeException )
{
    // try to set the corresponding metric unit
    String aConfigPath;
    Reference< XServiceInfo > xServiceInfo
            ( xDoc, UNO_QUERY );
    if ( xServiceInfo.is() )
    {
        if ( xServiceInfo->supportsService( OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.presentation.PresentationDocument" ) ) ) )
        {
            meDocType = DOCUMENT_TYPE_IMPRESS;
            return;
        }
        else if ( xServiceInfo->supportsService( OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.drawing.DrawingDocument" ) ) ) )
        {
            meDocType = DOCUMENT_TYPE_DRAW;
            return;
        }
    }
    throw IllegalArgumentException();
}

