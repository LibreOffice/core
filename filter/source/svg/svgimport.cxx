 /*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: svgimport.cxx,v $
 * $Revision: 1.9 $
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_filter.hxx"

#include "svgfilter.hxx"
#include "rtl/ref.hxx"
#include "jvmaccess/virtualmachine.hxx"
// -------------
// - SVGFilter -
// -------------

sal_Bool SVGFilter::implImport( const Sequence< PropertyValue >& rDescriptor )
    throw (RuntimeException)
{
    Reference< XMultiServiceFactory >   xServiceFactory( ::comphelper::getProcessServiceFactory() ) ;
    rtl::OUString                           aTmpFileName;
    String                              aFileName;
    sal_Int32                           nLength = rDescriptor.getLength();
    const PropertyValue*                pValue = rDescriptor.getConstArray();
    sal_Bool                            bRet = sal_False;

    for( sal_Int32 i = 0 ; ( i < nLength ) && !aTmpFileName.getLength(); i++)
        if( pValue[ i ].Name.equalsAscii( "FileName" ) )
            pValue[ i ].Value >>= aTmpFileName;

    if( aTmpFileName.getLength() && xServiceFactory.is() )
    {

        Reference< XJavaVM >    xJavaVM( xServiceFactory->createInstance( rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.java.JavaVirtualMachine") ) ), UNO_QUERY );
        Sequence< sal_Int8 >    aProcessID( 17 );
        String                  aLocalFile;

        if( ::utl::LocalFileHelper::ConvertURLToPhysicalName( aTmpFileName, aLocalFile ) && aLocalFile.Len() )
        {
            rtl_getGlobalProcessId( (sal_uInt8 *) aProcessID.getArray() );
            aProcessID[16] = 0;

            OSL_ENSURE(sizeof (sal_Int64)
                       >= sizeof (jvmaccess::VirtualMachine *),
                       "Pointer cannot be represented as sal_Int64");
            sal_Int64 nPointer = reinterpret_cast< sal_Int64 >(
                static_cast< jvmaccess::VirtualMachine * >(0));
            xJavaVM->getJavaVM(aProcessID) >>= nPointer;
            rtl::Reference<jvmaccess::VirtualMachine> _virtualMachine =
                reinterpret_cast< jvmaccess::VirtualMachine * >(nPointer);
            if (!_virtualMachine.is())
                return bRet;

            jobjectArray    aArgs;
            jclass          aClass;
            jmethodID       aMId;
            jstring         aJStr;

            try
            {
                jvmaccess::VirtualMachine::AttachGuard vmGuard(_virtualMachine);

                JNIEnv * pEnv = vmGuard.getEnvironment();

                aClass = pEnv->FindClass( "SOTranscoder" );

                if( aClass )
                {
                    aMId = pEnv->GetStaticMethodID( aClass, "main", "([Ljava/lang/String;)V" );
                    if ( aMId )
                    {

                        ::utl::TempFile aTempFile;
                        String          aOutputURL( aTempFile.GetURL() );
                        String          aOutputFile;

                        aTempFile.EnableKillingFile();

                        if( ::utl::LocalFileHelper::ConvertURLToPhysicalName( aOutputURL, aOutputFile ) && aOutputFile.Len() )
                        {
                            aJStr = pEnv->NewStringUTF( ByteString( aLocalFile.GetBuffer(), RTL_TEXTENCODING_UTF8 ).GetBuffer() );
                            aArgs = static_cast<jobjectArray>(pEnv->NewObjectArray( 2, pEnv->FindClass( "java/lang/String" ), aJStr ));
                            aJStr = pEnv->NewStringUTF( ByteString( aOutputFile.GetBuffer(), RTL_TEXTENCODING_UTF8 ).GetBuffer() );
                            pEnv->SetObjectArrayElement( aArgs, 1, aJStr );
                            pEnv->CallStaticVoidMethod( aClass, aMId, aArgs );

                            Graphic     aGraphic;
                            SvStream*   pIStm = ::utl::UcbStreamHelper::CreateStream( aOutputURL, STREAM_READ );

                            if( pIStm )
                            {
                                GraphicConverter::Import( *pIStm, aGraphic );
                                delete pIStm;
                            }

                            Reference< XDrawPagesSupplier > xDrawPagesSupplier( mxDstDoc, UNO_QUERY );

                            if( xDrawPagesSupplier.is() && ( aGraphic.GetType() != GRAPHIC_NONE ) )
                            {
                                Reference< XDrawPages > xDrawPages( xDrawPagesSupplier->getDrawPages() );

                                if( xDrawPages.is() && xDrawPages->getCount() )
                                {
                                    Reference< XDrawPage >  xDrawPage;

                                    if( xDrawPages->getByIndex( 0 ) >>= xDrawPage )
                                    {
                                        Reference< XShapes >        xShapes( xDrawPage, UNO_QUERY );
                                        Reference< XPropertySet>    xPagePropSet( xDrawPage, UNO_QUERY );
                                        Reference< XShape >         xShape( Reference< XMultiServiceFactory >( mxDstDoc, UNO_QUERY )->createInstance( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.drawing.GraphicObjectShape" ) ) ), UNO_QUERY );

                                        if( xPagePropSet.is() && xShapes.is() && xShape.is() )
                                        {
                                            Reference< XPropertySet >   xPropSet( xShape, UNO_QUERY );
                                            sal_Int32                   nPageWidth = 0, nPageHeight = 0;

                                            xPagePropSet->getPropertyValue( rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( "Width" ) ) ) >>= nPageWidth;
                                            xPagePropSet->getPropertyValue( rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( "Height" ) ) ) >>= nPageHeight;

                                            if( xPropSet.is() && nPageWidth && nPageHeight )
                                            {
                                                xShapes->add( xShape );

                                                ::com::sun::star::awt::Point    aPos;
                                                ::com::sun::star::awt::Size     aSize;
                                                GraphicObject                   aGraphObj( aGraphic );
                                                String                          aGraphURL( RTL_CONSTASCII_USTRINGPARAM( "vnd.sun.star.GraphicObject:" ) );
                                                Any                             aValue;
                                                Size                            aGraphicSize;
                                                const MapMode                   aTargetMapMode( MAP_100TH_MM );

                                                if( aGraphObj.GetPrefMapMode().GetMapUnit() == MAP_PIXEL )
                                                    aGraphicSize = Application::GetDefaultDevice()->PixelToLogic( aGraphObj.GetPrefSize(), aTargetMapMode );
                                                else
                                                    aGraphicSize = OutputDevice::LogicToLogic( aGraphObj.GetPrefSize(), aGraphObj.GetPrefMapMode(), aTargetMapMode );

                                                aGraphURL += String( aGraphObj.GetUniqueID(), RTL_TEXTENCODING_ASCII_US );
                                                aValue <<= rtl::OUString( aGraphURL );
                                                xPropSet->setPropertyValue( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "GraphicURL" ) ), aValue );

                                                aPos.X = ( nPageWidth - aGraphicSize.Width() ) >> 1;
                                                aPos.Y = ( nPageHeight - aGraphicSize.Height() ) >> 1;

                                                aSize.Width = aGraphicSize.Width();
                                                aSize.Height = aGraphicSize.Height();

                                                xShape->setPosition( aPos );
                                                xShape->setSize( aSize );

                                                bRet = sal_True;
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
            catch (jvmaccess::VirtualMachine::AttachGuard::CreationException &)
            {
            }
        }
    }
    return bRet;
}

