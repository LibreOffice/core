/*************************************************************************
 *
 *  $RCSfile: xdialogcreator.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: kz $ $Date: 2004-10-04 19:56:12 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef _COM_SUN_STAR_EMBED_XEMBEDOBJECTCREATOR_HPP_
#include <com/sun/star/embed/XEmbedObjectCreator.hpp>
#endif
#ifndef _COM_SUN_STAR_EMBED_XEMBEDDEDOBJECT_HPP_
#include <com/sun/star/embed/XEmbeddedObject.hpp>
#endif
#ifndef _COM_SUN_STAR_EMBED_ENTRYINITMODES_HPP_
#include <com/sun/star/embed/EntryInitModes.hpp>
#endif

#ifndef _COM_SUN_STAR_BEANS_PROPERTYVALUE_HPP_
#include <com/sun/star/beans/PropertyValue.hpp>
#endif

#ifndef _COM_SUN_STAR_DATATRANSFER_DATAFLAVOR_HPP_
#include <com/sun/star/datatransfer/DataFlavor.hpp>
#endif

#ifndef _COM_SUN_STAR_UCB_COMMANDABORTEDEXCEPTION_HPP_
#include <com/sun/star/ucb/CommandAbortedException.hpp>
#endif


#include <osl/thread.h>
#include <vos/module.hxx>

#include "platform.h"
#include "xdialogcreator.hxx"


#ifdef WNT

#include <oledlg.h>

class InitializedOleGuard
{
public:
    InitializedOleGuard()
    {
        if ( !SUCCEEDED( OleInitialize( NULL ) ) )
            throw ::com::sun::star::uno::RuntimeException();
    }

    ~InitializedOleGuard()
    {
        OleUninitialize();
    }
};

extern "C" {
typedef UINT STDAPICALLTYPE OleUIInsertObjectA_Type(LPOLEUIINSERTOBJECTA);
}

#endif


using namespace ::com::sun::star;

uno::Sequence< sal_Int8 > GetSequenceClassID( sal_uInt32 n1, sal_uInt16 n2, sal_uInt16 n3,
                                                sal_uInt8 b8, sal_uInt8 b9, sal_uInt8 b10, sal_uInt8 b11,
                                                sal_uInt8 b12, sal_uInt8 b13, sal_uInt8 b14, sal_uInt8 b15 );

//-------------------------------------------------------------------------
sal_Bool ClassIDsEqual( const uno::Sequence< sal_Int8 >& aClassID1, const uno::Sequence< sal_Int8 >& aClassID2 )
{
    // TODO/LATER: move this method and other methods like this to a standalone library
    if ( aClassID1.getLength() != aClassID2.getLength() )
        return sal_False;

    for ( sal_Int32 nInd = 0; nInd < aClassID1.getLength(); nInd++ )
        if ( aClassID1[nInd] != aClassID2[nInd] )
            return sal_False;

    return sal_True;
}

//-------------------------------------------------------------------------
uno::Sequence< sal_Int8 > GetRelatedInternalID_Impl( const uno::Sequence< sal_Int8 >& aClassID )
{
    // TODO/LATER: The ClassIDs must be moved to a general place or to configuration so that this workaround can be removed

    // Writer
    if ( ClassIDsEqual( aClassID,
                        GetSequenceClassID( 0x30a2652a, 0xddf7, 0x45e7, 0xac, 0xa6, 0x3e, 0xab, 0x26, 0xfc, 0x8a, 0x4e ) ) )
        return GetSequenceClassID( 0x8BC6B165, 0xB1B2, 0x4EDD, 0xAA, 0x47, 0xDA, 0xE2, 0xEE, 0x68, 0x9D, 0xD6 );

    // Calc
    if ( ClassIDsEqual( aClassID,
                        GetSequenceClassID( 0x7b342dc4, 0x139a, 0x4a46, 0x8a, 0x93, 0xdb, 0x8, 0x27, 0xcc, 0xee, 0x9c ) ) )
        return GetSequenceClassID( 0x47BBB4CB, 0xCE4C, 0x4E80, 0xA5, 0x91, 0x42, 0xD9, 0xAE, 0x74, 0x95, 0x0F );

    // Impress
    if ( ClassIDsEqual( aClassID,
                        GetSequenceClassID( 0xe5a0b632, 0xdfba, 0x4549, 0x93, 0x46, 0xe4, 0x14, 0xda, 0x6, 0xe6, 0xf8 ) ) )
        return GetSequenceClassID( 0x9176E48A, 0x637A, 0x4D1F, 0x80, 0x3B, 0x99, 0xD9, 0xBF, 0xAC, 0x10, 0x47 );

    // Draw
    if ( ClassIDsEqual( aClassID,
                        GetSequenceClassID( 0x41662fc2, 0xd57, 0x4aff, 0xab, 0x27, 0xad, 0x2e, 0x12, 0xe7, 0xc2, 0x73 ) ) )
        return GetSequenceClassID( 0x4BAB8970, 0x8A3B, 0x45B3, 0x99, 0x1C, 0xCB, 0xEE, 0xAC, 0x6B, 0xD5, 0xE3 );

    // Chart
    if ( ClassIDsEqual( aClassID,
                        GetSequenceClassID( 0xd415cd93, 0x35c4, 0x4c6f, 0x81, 0x9d, 0xa6, 0x64, 0xa1, 0xc8, 0x13, 0xae ) ) )
        return GetSequenceClassID( 0x12DCAE26, 0x281F, 0x416F, 0xA2, 0x34, 0xC3, 0x08, 0x61, 0x27, 0x38, 0x2E );

    // Math
    if ( ClassIDsEqual( aClassID,
                        GetSequenceClassID( 0xd0484de6, 0xaaee, 0x468a, 0x99, 0x1f, 0x8d, 0x4b, 0x7, 0x37, 0xb5, 0x7a ) ) )
        return GetSequenceClassID( 0x078B7ABA, 0x54FC, 0x457F, 0x85, 0x51, 0x61, 0x47, 0xE7, 0x76, 0xA9, 0x97 );

    return aClassID;
}

//-------------------------------------------------------------------------
uno::Sequence< ::rtl::OUString > SAL_CALL MSOLEDialogObjectCreator::impl_staticGetSupportedServiceNames()
{
    uno::Sequence< ::rtl::OUString > aRet(2);
    aRet[0] = ::rtl::OUString::createFromAscii("com.sun.star.embed.MSOLEDialogObjectCreator");
    aRet[1] = ::rtl::OUString::createFromAscii("com.sun.star.comp.embed.MSOLEDialogObjectCreator");
    return aRet;
}

//-------------------------------------------------------------------------
::rtl::OUString SAL_CALL MSOLEDialogObjectCreator::impl_staticGetImplementationName()
{
    return ::rtl::OUString::createFromAscii("com.sun.star.comp.embed.MSOLEDialogObjectCreator");
}

//-------------------------------------------------------------------------
uno::Reference< uno::XInterface > SAL_CALL MSOLEDialogObjectCreator::impl_staticCreateSelfInstance(
            const uno::Reference< lang::XMultiServiceFactory >& xServiceManager )
{
    return uno::Reference< uno::XInterface >( *new MSOLEDialogObjectCreator( xServiceManager ) );
}

//-------------------------------------------------------------------------
embed::InsertedObjectInfo SAL_CALL MSOLEDialogObjectCreator::createInstanceByDialog(
            const uno::Reference< embed::XStorage >& xStorage,
            const ::rtl::OUString& sEntName,
            const uno::Sequence< beans::PropertyValue >& lObjArgs )
    throw ( lang::IllegalArgumentException,
            io::IOException,
            uno::Exception,
            uno::RuntimeException )
{
    embed::InsertedObjectInfo aObjectInfo;

#ifdef WNT

    if ( !xStorage.is() )
        throw lang::IllegalArgumentException( ::rtl::OUString::createFromAscii( "No parent storage is provided!\n" ),
                                            uno::Reference< uno::XInterface >( reinterpret_cast< ::cppu::OWeakObject* >(this) ),
                                            1 );

    if ( !sEntName.getLength() )
        throw lang::IllegalArgumentException( ::rtl::OUString::createFromAscii( "Empty element name is provided!\n" ),
                                            uno::Reference< uno::XInterface >( reinterpret_cast< ::cppu::OWeakObject* >(this) ),
                                            2 );

    InitializedOleGuard aGuard;

    OLEUIINSERTOBJECT io;
    char szFile[MAX_PATH];
    UINT uTemp;

    memset(&io, 0, sizeof(io));

    io.cbStruct = sizeof(io);
    io.hWndOwner = GetActiveWindow();

    szFile[0] = 0;
    io.lpszFile = szFile;
    io.cchFile = MAX_PATH;

    io.dwFlags = IOF_SELECTCREATENEW | IOF_DISABLELINK;


    ::vos::OModule aOleDlgLib;
    if( !aOleDlgLib.load( ::rtl::OUString::createFromAscii( "oledlg" ) ) )
        throw uno::RuntimeException();

    OleUIInsertObjectA_Type * pInsertFct = (OleUIInsertObjectA_Type *)
                                aOleDlgLib.getSymbol( ::rtl::OUString::createFromAscii( "OleUIInsertObjectA" ) );
    if( !pInsertFct )
        throw uno::RuntimeException();

    uTemp=pInsertFct(&io);

    if ( OLEUI_OK == uTemp )
    {
        uno::Reference< embed::XEmbedObjectCreator > xEmbCreator(
                    m_xFactory->createInstance(
                            ::rtl::OUString::createFromAscii( "com.sun.star.embed.EmbeddedObjectCreator" ) ),
                    uno::UNO_QUERY );
        if ( !xEmbCreator.is() )
            throw uno::RuntimeException();


        if (io.dwFlags & IOF_SELECTCREATENEW)
        {
            uno::Sequence< sal_Int8 > aClassID = GetSequenceClassID( io.clsid.Data1,
                                                                     io.clsid.Data2,
                                                                     io.clsid.Data3,
                                                                     io.clsid.Data4[0],
                                                                     io.clsid.Data4[1],
                                                                     io.clsid.Data4[2],
                                                                     io.clsid.Data4[3],
                                                                     io.clsid.Data4[4],
                                                                     io.clsid.Data4[5],
                                                                     io.clsid.Data4[6],
                                                                     io.clsid.Data4[7] );

            aClassID = GetRelatedInternalID_Impl( aClassID );

            //TODO: retrieve ClassName
            ::rtl::OUString aClassName;
            aObjectInfo.Object = uno::Reference< embed::XEmbeddedObject >(
                            xEmbCreator->createInstanceInitNew( aClassID, aClassName, xStorage, sEntName, lObjArgs ),
                            uno::UNO_QUERY );
        }
        else
        {
            ::rtl::OUString aFileName = ::rtl::OStringToOUString( ::rtl::OString( szFile ), osl_getThreadTextEncoding() );
            uno::Sequence< beans::PropertyValue > aMediaDescr( 1 );
            aMediaDescr[0].Name = ::rtl::OUString::createFromAscii( "URL" );
            aMediaDescr[0].Value <<= aFileName;

            aObjectInfo.Object = uno::Reference< embed::XEmbeddedObject >(
                            xEmbCreator->createInstanceInitFromMediaDescriptor( xStorage, sEntName, aMediaDescr, lObjArgs ),
                            uno::UNO_QUERY );
        }

        if ((io.dwFlags & IOF_CHECKDISPLAYASICON) && io.hMetaPict != NULL )
        {
            METAFILEPICT* pMF = ( METAFILEPICT* )GlobalLock( io.hMetaPict );
            if ( pMF )
            {
                sal_uInt32 nBufSize = GetMetaFileBitsEx( pMF->hMF, 0, NULL );
                uno::Sequence< sal_Int8 > aMetafile( nBufSize + 22 );
                sal_uInt8* pBuf = (sal_uInt8*)( aMetafile.getArray() );
                *( (long* )pBuf ) = 0x9ac6cdd7L;
                *( (short* )( pBuf+6 )) = ( SHORT ) 0;
                *( (short* )( pBuf+8 )) = ( SHORT ) 0;
                *( (short* )( pBuf+10 )) = ( SHORT ) pMF->xExt;
                *( (short* )( pBuf+12 )) = ( SHORT ) pMF->yExt;
                *( (short* )( pBuf+14 )) = ( USHORT ) 2540;

                if ( nBufSize && nBufSize == GetMetaFileBitsEx( pMF->hMF, nBufSize, pBuf+22 ) )
                {
                    datatransfer::DataFlavor aFlavor(
                        ::rtl::OUString::createFromAscii( "application/x-openoffice-wmf;windows_formatname=\"Image WMF\"" ),
                        ::rtl::OUString::createFromAscii( "Image WMF" ),
                        getCppuType( ( const uno::Sequence< sal_Int8 >* ) 0 ) );

                    aObjectInfo.Options.realloc( 2 );
                    aObjectInfo.Options[0].Name = ::rtl::OUString::createFromAscii( "Icon" );
                    aObjectInfo.Options[0].Value <<= aMetafile;
                    aObjectInfo.Options[1].Name = ::rtl::OUString::createFromAscii( "IconFormat" );
                    aObjectInfo.Options[1].Value <<= aFlavor;
                }

                GlobalUnlock( io.hMetaPict );
            }
        }
    }
    else
        throw ucb::CommandAbortedException();

#else
    throw lang::NoSupportException(); // TODO:
#endif

    OSL_ENSURE( aObjectInfo.Object.is(), "No object was created!\n" );
    if ( !aObjectInfo.Object.is() )
        throw uno::RuntimeException();

    return aObjectInfo;
}

//-------------------------------------------------------------------------
::rtl::OUString SAL_CALL MSOLEDialogObjectCreator::getImplementationName()
    throw ( uno::RuntimeException )
{
    return impl_staticGetImplementationName();
}

//-------------------------------------------------------------------------
sal_Bool SAL_CALL MSOLEDialogObjectCreator::supportsService( const ::rtl::OUString& ServiceName )
    throw ( uno::RuntimeException )
{
    uno::Sequence< ::rtl::OUString > aSeq = impl_staticGetSupportedServiceNames();

    for ( sal_Int32 nInd = 0; nInd < aSeq.getLength(); nInd++ )
        if ( ServiceName.compareTo( aSeq[nInd] ) == 0 )
            return sal_True;

    return sal_False;
}

//-------------------------------------------------------------------------
uno::Sequence< ::rtl::OUString > SAL_CALL MSOLEDialogObjectCreator::getSupportedServiceNames()
    throw ( uno::RuntimeException )
{
    return impl_staticGetSupportedServiceNames();
}

