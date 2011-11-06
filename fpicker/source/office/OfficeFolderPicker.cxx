/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_fpicker.hxx"

#include "OfficeFolderPicker.hxx"

#include "iodlg.hxx"

#include <list>
#include <tools/urlobj.hxx>

#define _SVSTDARR_STRINGSDTOR
#include "svl/svstdarr.hxx"
#include <com/sun/star/container/XContentEnumerationAccess.hpp>
#include <com/sun/star/container/XSet.hpp>
#include <com/sun/star/uno/Any.hxx>
#include <cppuhelper/factory.hxx>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <unotools/pathoptions.hxx>

// using ----------------------------------------------------------------

using namespace     ::com::sun::star::container;
using namespace     ::com::sun::star::lang;
using namespace     ::com::sun::star::uno;
using namespace     ::com::sun::star::beans;

//------------------------------------------------------------------------------------
// class SvtFolderPicker
//------------------------------------------------------------------------------------

//------------------------------------------------------------------------------------
SvtFolderPicker::SvtFolderPicker( const Reference < XMultiServiceFactory >& _rxFactory )
    :OCommonPicker( _rxFactory )
{
}

//------------------------------------------------------------------------------------
SvtFolderPicker::~SvtFolderPicker()
{
}

//------------------------------------------------------------------------------------
// disambiguate XInterface
//------------------------------------------------------------------------------------
IMPLEMENT_FORWARD_XINTERFACE2( SvtFolderPicker, OCommonPicker, SvtFolderPicker_Base )

//------------------------------------------------------------------------------------
// disambiguate XTypeProvider
//------------------------------------------------------------------------------------
IMPLEMENT_FORWARD_XTYPEPROVIDER2( SvtFolderPicker, OCommonPicker, SvtFolderPicker_Base )

//------------------------------------------------------------------------------------
// XExecutableDialog functions
//------------------------------------------------------------------------------------

//------------------------------------------------------------------------------------
void SAL_CALL SvtFolderPicker::setTitle( const ::rtl::OUString& _rTitle ) throw (RuntimeException)
{
    OCommonPicker::setTitle( _rTitle );
}

//------------------------------------------------------------------------------------
sal_Int16 SAL_CALL SvtFolderPicker::execute(  ) throw (RuntimeException)
{
    return OCommonPicker::execute();
}

//------------------------------------------------------------------------------------
// XAsynchronousExecutableDialog functions
//------------------------------------------------------------------------------------

//------------------------------------------------------------------------------------
void SAL_CALL SvtFolderPicker::setDialogTitle( const ::rtl::OUString& _rTitle) throw (RuntimeException)
{
    setTitle( _rTitle );
}

//------------------------------------------------------------------------------------
void SAL_CALL SvtFolderPicker::startExecuteModal( const Reference< ::com::sun::star::ui::dialogs::XDialogClosedListener >& xListener ) throw (RuntimeException)
{
    m_xListener = xListener;
    prepareDialog();
    prepareExecute();
    getDialog()->EnableAutocompletion( sal_True );
    getDialog()->StartExecuteModal( LINK( this, SvtFolderPicker, DialogClosedHdl ) );
}

//------------------------------------------------------------------------------------
SvtFileDialog* SvtFolderPicker::implCreateDialog( Window* _pParent )
{
    return new SvtFileDialog( _pParent, SFXWB_PATHDIALOG );
}

//------------------------------------------------------------------------------------
sal_Int16 SvtFolderPicker::implExecutePicker( )
{
    prepareExecute();

    // now we are ready to execute the dialog
    getDialog()->EnableAutocompletion( sal_False );
    sal_Int16 nRet = getDialog()->Execute();

    return nRet;
}

//------------------------------------------------------------------------------------
void SvtFolderPicker::prepareExecute()
{
    // set the default directory
    if ( m_aDisplayDirectory.getLength() > 0 )
        getDialog()->SetPath( m_aDisplayDirectory );
    else
    {
        // Default-Standard-Dir setzen
        INetURLObject aStdDirObj( SvtPathOptions().GetWorkPath() );
        getDialog()->SetPath( aStdDirObj.GetMainURL( INetURLObject::NO_DECODE) );
    }
}

//-----------------------------------------------------------------------------
IMPL_LINK( SvtFolderPicker, DialogClosedHdl, Dialog*, pDlg )
{
    if ( m_xListener.is() )
    {
        sal_Int16 nRet = static_cast< sal_Int16 >( pDlg->GetResult() );
        ::com::sun::star::ui::dialogs::DialogClosedEvent aEvent( *this, nRet );
        m_xListener->dialogClosed( aEvent );
        m_xListener.clear();
    }
    return 0;
  }

//------------------------------------------------------------------------------------
// XFolderPicker functions
//------------------------------------------------------------------------------------

void SAL_CALL SvtFolderPicker::setDisplayDirectory( const ::rtl::OUString& aDirectory )
    throw( IllegalArgumentException, RuntimeException )
{
    m_aDisplayDirectory = aDirectory;
}

//------------------------------------------------------------------------------------
::rtl::OUString SAL_CALL SvtFolderPicker::getDisplayDirectory() throw( RuntimeException )
{
    ::rtl::OUString aResult;

    if ( ! getDialog() )
        return m_aDisplayDirectory;

    SvStringsDtor* pPathList = getDialog()->GetPathList();

    if ( pPathList->Count() )
        aResult = ::rtl::OUString( *pPathList->GetObject( 0 ) );

    delete pPathList;

    return aResult;
}

//------------------------------------------------------------------------------------
::rtl::OUString SAL_CALL SvtFolderPicker::getDirectory() throw( RuntimeException )
{
    ::rtl::OUString aResult;

    if ( ! getDialog() )
        return m_aDisplayDirectory;

    SvStringsDtor* pPathList = getDialog()->GetPathList();

    if ( pPathList->Count() )
        aResult = ::rtl::OUString( *pPathList->GetObject( 0 ) );

    delete pPathList;

    return aResult;
}

//------------------------------------------------------------------------------------
void SAL_CALL SvtFolderPicker::setDescription( const ::rtl::OUString& aDescription )
    throw( RuntimeException )
{
    m_aDescription = aDescription;
}

//------------------------------------------------------------------------------------
// XServiceInfo
//------------------------------------------------------------------------------------

/* XServiceInfo */
::rtl::OUString SAL_CALL SvtFolderPicker::getImplementationName() throw( RuntimeException )
{
    return impl_getStaticImplementationName();
}

/* XServiceInfo */
sal_Bool SAL_CALL SvtFolderPicker::supportsService( const ::rtl::OUString& sServiceName ) throw( RuntimeException )
{
    Sequence< ::rtl::OUString > seqServiceNames = getSupportedServiceNames();
    const ::rtl::OUString* pArray = seqServiceNames.getConstArray();
    for ( sal_Int32 i = 0; i < seqServiceNames.getLength(); i++ )
    {
        if ( sServiceName == pArray[i] )
        {
            return sal_True ;
        }
    }
    return sal_False ;
}

/* XServiceInfo */
Sequence< ::rtl::OUString > SAL_CALL SvtFolderPicker::getSupportedServiceNames() throw( RuntimeException )
{
    return impl_getStaticSupportedServiceNames();
}

/* Helper for XServiceInfo */
Sequence< ::rtl::OUString > SvtFolderPicker::impl_getStaticSupportedServiceNames()
{
    Sequence< ::rtl::OUString > seqServiceNames(1);
    seqServiceNames[0] = ::rtl::OUString::createFromAscii( "com.sun.star.ui.dialogs.OfficeFolderPicker" );
    return seqServiceNames ;
}

/* Helper for XServiceInfo */
::rtl::OUString SvtFolderPicker::impl_getStaticImplementationName()
{
    return ::rtl::OUString::createFromAscii( "com.sun.star.svtools.OfficeFolderPicker" );
}

/* Helper for registry */
Reference< XInterface > SAL_CALL SvtFolderPicker::impl_createInstance( const Reference< XComponentContext >& rxContext )
    throw( Exception )
{
    Reference< XMultiServiceFactory > xServiceManager (rxContext->getServiceManager(), UNO_QUERY_THROW);
    return Reference< XInterface >( *new SvtFolderPicker( xServiceManager ) );
}

